#include "processor/TQClamTrackProc.h"


// ===========================================
// Diese Klasse fuehrt das Tracking durch, Ablauf:
//
// 1) Cluster als Input
//
// 2) ExtractRawLayerData, Iterations-Zustaende vorbereiten 
// -> fuelle fuer jeden Cluster die Struktur 'LayerRawData' (z-Pos, Drahtwinkel, Driftzeiten...)
// -> bereite Struktur 'LayerTrackingState' je Cluster fuer Iteration vor
// -> pruefe, welche Ebenen Cluster haben -> jeweils verschiedenes Vorgehen
//
// 3) Startwerte (phi,theta)
// -> berechne Startwinkel aus Cluster-Schwerpunkten
//
// 4) Iterative Rekonstruktion
// -> Iterationsschleife:
// 4.1) Driftzeit -> zOffset umrechnen
// -> fuer phi aus letztem Iterationsschritt (bzw. Start-Phi): mappe (phi,driftzeit) -> Spurversatz
// 4.2) Schnittpunkt mit Ebenen bestimmen: ComputeItersection()
// -> nutze Regression oder Algorithmen (Auswahl abhaengig von Cluster-Form)
// zur Berechnung des Schnittpunkts des Tracks mit der jeweiligen Ebene
// 4.3) Residuen-Berechnung
// -> Residuum = Spurversatz_gemessen - Spurversatz_rekonstruiert (je Hit)
// 4.4) Entferne Ausreisser-Hits
// -> Pruefe: Residuum / Sigma_Residuum in Condition-Fenster
// -> Hits ausserhalb des Fensters werden in der naechsten Iteration nicht mehr verwendet
// 4.5) Neue Winkel berechnen (ComputeAngles()):
// -> berechne mittels der neu-rekonstruierten Ebenen-Schnittpunkte die Winkel
// -> phi = arctan( (z_X2 - z_X1) / (x_X2 - x_X1) )
// -> theta = TODO
// 4.6) Konvergenz testen:
// -> | phi_old - phi_new | < Epsilon
// -> falls nicht erfuellt und maximale Iterationsanzahl nicht ueberschritten: wiederholen
// -> falls erfuellt: fortfahren mit 5)
//
// 5) Fit-Qualitaet bestimmen
// -> berechne 'Fit'-Qualitaets Groessen:
// - chi2, chi2/NDF, Residuen (mean, rms, ...), outlier-anzahl, interationsanzahl
//
// 6) Track-Parameter berechnen
//
// 7) Event fuellen:
// -> FillEventFromStates(...): uebergebe alle berechneten Groessen an TrackEvent
//
// 7) Kalibrationen
// -> projiziere Track auf Fokalebene
// -> TODO: X-Phi Korrektur
//
// 8) Histogramme fuellen
// ===========================================

/** go4-includes */
#include "TGo4Fitter.h"
#include "TGo4RuntimeException.h"
#include "TGo4Log.h"
#include "TGo4EventElement.h"
#include "TGo4Picture.h"
#include "TGo4WinCond.h"

/** ROOT-includes */
#include "TClass.h"
#include "TVector3.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TGraph.h"
#include "TTree.h"
#include "TLegend.h"

/** evts */
#include "evt/TQClamClusterEvent.h"
#include "evt/TQClamTrackEvent.h"

/** parameter */
#include "parameter/TQClamTrackConditionParameter.h"
#include "parameter/TQClamDriftToZOffsetParameter.h"
#include "parameter/TQClamFocalPlaneParameter.h"
#include "parameter/TQClamSpectrumCalibrationParameter.h"
#include "parameter/TQClamFitParameter.h"
#include "parameter/TQClamClusterConditionParameter.h"
#include "parameter/TQClamHistogramParameter.h"
#include "parameter/TQClamSieveSlitCalibParameter.h"

/** conditions */


/** geometry */
#include "geometry/TQClamGeometry.h"
#include "geometry/TQClamWirePlane.h"

#include <iostream>
#include <experimental/simd> // TODO Mache schleifen mit simd schneller???




/** Kandidaten fuer Outlier
 * -> sortiere diese anschliessend und entferne die n groessten hits,
 *  wobei n = konfigurierbar */
struct OutlierCandidate
{
	size_t hitIndex;
	size_t residualIndex;
	double pull;
};




ClassImp(TQClamTrackProc)





TQClamTrackProc::TQClamTrackProc()
	: TGo4EventProcessor(),
	fDriftcham(TQClamGeometry::Instance().GetDriftcham())
{}


TQClamTrackProc::TQClamTrackProc(const char* name)
	: TGo4EventProcessor(name),
	  fDriftcham(TQClamGeometry::Instance().GetDriftcham())
{
	TGo4Log::Info("[TQClamTrackProc] Create %s", name);

        //-----------------------------------------------------------
        // Parameter, Histogramme etc initialisieren
        //-----------------------------------------------------------


	fFitCounter = 0;
	fEvtCounter = 0;
	fConvergedTrackCounter = 0;
	fNotConvergedTrackCounter = 0;

	fRejectedCounter.resize(nRejHitVecSize,0);
	fRejectedCounter_X1.resize(nRejHitVecSize,0);
	fRejectedCounter_U.resize(nRejHitVecSize,0);
	fRejectedCounter_X2.resize(nRejHitVecSize,0);

	fUsedCounter.resize(nUsedHitVecSize,0);
	fUsedCounter_X1.resize(nUsedHitVecSize,0);
	fUsedCounter_U.resize(nUsedHitVecSize,0);
	fUsedCounter_X2.resize(nUsedHitVecSize,0);


        //-----------------------------------------------------------
        // 1. initialisiere Parameter
	fTrackConditionPar = (TQClamTrackConditionParameter*) GetParameter("TrackConditionParam");
	fDriftToZOffsetPar = (TQClamDriftToZOffsetParameter*) (GetParameter("DriftToZOffsetParam"));
	fFocalPlanePar = (TQClamFocalPlaneParameter*) (GetParameter("FocalPlaneParam"));
	fSpectrumCaliPar = (TQClamSpectrumCalibrationParameter*) (GetParameter("SpecCalibParam"));
	fFitPar = (TQClamFitParameter*) (GetParameter("FitParam"));
	fClusterPar = (TQClamClusterConditionParameter*) (GetParameter("ClusterConditionParam"));
	fHistoPar = (TQClamHistogramParameter*) (GetParameter("HistoParam"));
	fSieveSlitCalibParam = (TQClamSieveSlitCalibParameter*) (GetParameter("SieveSlitCalibParam"));


        //-----------------------------------------------------------
        // 2. Conditions

	fCondPull = MakeWinCond("CondWinTrackPull",-1.0,fTrackConditionPar->UpperOutlierRejectionLimit);
	fSieveSlitGate = MakeWinCond("CondSieveSlitGate",0,1000);



        //-----------------------------------------------------------
	// 3. Histogramme

	/** DriftDistanz Vs Zeit */
	fHDriftDistanceTrackVsTime_X1 = MakeTH2('D',"DriftDistanceTrackVsTime_X1","Rekonstr. Driftdistanz vs. Driftzeit (X1)",800,0,400,2000,0,20);
	fHDriftDistanceTrackVsTime_X1->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceTrackVsTime_X1->GetXaxis()->CenterTitle();
	fHDriftDistanceTrackVsTime_X1->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceTrackVsTime_X1->GetYaxis()->CenterTitle();

	fHDriftDistanceTrackVsTime_U = MakeTH2('D',"DriftDistanceTrackVsTime_U","Rekonstr. Driftdistanz vs. Driftzeit (U)",800,0,400,2000,0,20);
	fHDriftDistanceTrackVsTime_U->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceTrackVsTime_U->GetXaxis()->CenterTitle();
	fHDriftDistanceTrackVsTime_U->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceTrackVsTime_U->GetYaxis()->CenterTitle();

	fHDriftDistanceTrackVsTime_X2 = MakeTH2('D',"DriftDistanceTrackVsTime_X2","Rekonstr. Driftdistanz vs. Driftzeit (X2)",800,0,400,2000,0,20);
	fHDriftDistanceTrackVsTime_X2->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceTrackVsTime_X2->GetXaxis()->CenterTitle();
	fHDriftDistanceTrackVsTime_X2->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceTrackVsTime_X2->GetYaxis()->CenterTitle();

	fHDriftDistanceTrackVsTime_All = MakeTH2('D',"DriftDistanceTrackVsTime_All","Rekonstr. Driftdistanz vs. Driftzeit (X1&U&X2)",800,0,400,2000,0,20);
	fHDriftDistanceTrackVsTime_All->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceTrackVsTime_All->GetXaxis()->CenterTitle();
	fHDriftDistanceTrackVsTime_All->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceTrackVsTime_All->GetYaxis()->CenterTitle();



	fHDriftDistanceMeasVsTime_X1 = MakeTH2('D',"DriftDistanceMeasVsTime_X1","Gem. Driftdistanz vs. Driftzeit (X1)",800,0,400,2000,0,20);
	fHDriftDistanceMeasVsTime_X1->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceMeasVsTime_X1->GetXaxis()->CenterTitle();
	fHDriftDistanceMeasVsTime_X1->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceMeasVsTime_X1->GetYaxis()->CenterTitle();

	fHDriftDistanceMeasVsTime_U = MakeTH2('D',"DriftDistanceMeasVsTime_U","Gem. Driftdistanz vs. Driftzeit (U)",800,0,400,2000,0,20);
	fHDriftDistanceMeasVsTime_U->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceMeasVsTime_U->GetXaxis()->CenterTitle();
	fHDriftDistanceMeasVsTime_U->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceMeasVsTime_U->GetYaxis()->CenterTitle();

	fHDriftDistanceMeasVsTime_X2 = MakeTH2('D',"DriftDistanceMeasVsTime_X2","Gem. Driftdistanz vs. Driftzeit (X2)",800,0,400,2000,0,20);
	fHDriftDistanceMeasVsTime_X2->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceMeasVsTime_X2->GetXaxis()->CenterTitle();
	fHDriftDistanceMeasVsTime_X2->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceMeasVsTime_X2->GetYaxis()->CenterTitle();

	fHDriftDistanceMeasVsTime_All = MakeTH2('D',"DriftDistanceMeasVsTime_All","Gem. Driftdistanz vs. Driftzeit (X1&U&X2)",800,0,400,2000,0,20);
	fHDriftDistanceMeasVsTime_All->GetXaxis()->SetTitle("Driftzeit / (ns)");
	fHDriftDistanceMeasVsTime_All->GetXaxis()->CenterTitle();
	fHDriftDistanceMeasVsTime_All->GetYaxis()->SetTitle("Spurversatz / (mm)");
	fHDriftDistanceMeasVsTime_All->GetYaxis()->CenterTitle();





	/** Spektrum in X1-Ebene */
	fHTrackEfficiency = MakeTH1('D',"TrackEfficiency","Track-Effizienz ( (#conv #nconv)/#evt | #conv/#evt | #conv/#nconv)",3,0,3);
	fHTrackEfficiency->GetXaxis()->SetBinLabel(1,"#epsilon_{track} (conv + nconv)");
	fHTrackEfficiency->GetXaxis()->SetBinLabel(2,"#epsilon_{track} (conv)");
	fHTrackEfficiency->GetXaxis()->SetBinLabel(3,"frac_{conv}");
	fHTrackEfficiency->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHTrackEfficiency->GetYaxis()->CenterTitle();

	fHXSpectrumX1Plane = MakeTH1('D',"XSpectrumX1Plane","X-Spektrum in X1-Ebene",1000,0,1000);
	fHXSpectrumX1Plane->GetXaxis()->SetTitle("x / mm (X1)");
	fHXSpectrumX1Plane->GetXaxis()->CenterTitle();
	fHXSpectrumX1Plane->GetYaxis()->SetTitle("Haeufigkeit / (1 mm)");
	fHXSpectrumX1Plane->GetYaxis()->CenterTitle();

	fHXSpectrumUPlane = MakeTH1('D',"XSpectrumUPlane","X-Spektrum in U-Ebene",1000,0,1000);
	fHXSpectrumUPlane->GetXaxis()->SetTitle("x / mm (U)");
	fHXSpectrumUPlane->GetXaxis()->CenterTitle();
	fHXSpectrumUPlane->GetYaxis()->SetTitle("Haeufigkeit / (1 mm)");
	fHXSpectrumUPlane->GetYaxis()->CenterTitle();

	fHXSpectrumX2Plane = MakeTH1('D',"XSpectrumX2Plane","X-Spektrum in X2-Ebene",1000,-100,900);
	fHXSpectrumX2Plane->GetXaxis()->SetTitle("x / mm (X2)");
	fHXSpectrumX2Plane->GetXaxis()->CenterTitle();
	fHXSpectrumX2Plane->GetYaxis()->SetTitle("Haeufigkeit / (1 mm)");
	fHXSpectrumX2Plane->GetYaxis()->CenterTitle();


	fHUSpectrum = MakeTH1('D',"USpectrum","U-Spektrum",1000,-100,900);
	fHUSpectrum->GetXaxis()->SetTitle("u / mm (U)");
	fHUSpectrum->GetXaxis()->CenterTitle();
	fHUSpectrum->GetYaxis()->SetTitle("Haeufigkeit / (1 mm)");
	fHUSpectrum->GetYaxis()->CenterTitle();



	fHYSpectrumUPlane = MakeTH1('D',"YSpectrumUPlane","Y-Spektrum in U-Ebene",200,-100,100);
	fHYSpectrumUPlane->GetXaxis()->SetTitle("y-Koordinate / mm");
	fHYSpectrumUPlane->GetXaxis()->CenterTitle();
	fHYSpectrumUPlane->GetYaxis()->SetTitle("Haeufigkeit / (0.1 mm)");
	fHYSpectrumUPlane->GetYaxis()->CenterTitle();

	/// WINKEL (Detektorebene)
	fHPhiSpectrum = MakeTH1('D',"PhiSpectrum","Phi-Spektrum",400,0,100);
	fHPhiSpectrum->GetXaxis()->SetTitle("#phi / deg");
	fHPhiSpectrum->GetXaxis()->CenterTitle();
	fHPhiSpectrum->GetYaxis()->SetTitle("Haeufigkeit / (0.25 deg)");
	fHPhiSpectrum->GetYaxis()->CenterTitle();

	fHPhiCorrSpectrum = MakeTH1('D',"PhiCorrSpectrum","Phi-Spektrum korrigiert",400,-50.,50.);
	fHPhiCorrSpectrum->GetXaxis()->SetTitle("#phi / deg");
	fHPhiCorrSpectrum->GetXaxis()->CenterTitle();
	fHPhiCorrSpectrum->GetYaxis()->SetTitle("Haeufigkeit / (0.25 deg)");
	fHPhiCorrSpectrum->GetYaxis()->CenterTitle();

	fHThetaSpectrum = MakeTH1('D',"ThetaSpectrum","Theta-Spektrum",800,-100,100);
	fHThetaSpectrum->GetXaxis()->SetTitle("#theta / deg");
	fHThetaSpectrum->GetXaxis()->CenterTitle();
	fHThetaSpectrum->GetYaxis()->SetTitle("Haeufigkeit / (0.25 deg)");
	fHThetaSpectrum->GetYaxis()->CenterTitle();

	/// STREUWINKEL
	fHScattPhi = MakeTH1('D',"PhiScatt","Streuwinkel #phi_\{scatt} (horiz.)",200,-10,10);
	fHScattPhi->GetXaxis()->SetTitle("#phi_\{scatt} / deg");
	fHScattPhi->GetXaxis()->CenterTitle();
	fHScattPhi->GetYaxis()->SetTitle("Haeufigkeit / (0.10 deg)");
	fHScattPhi->GetYaxis()->CenterTitle();

	fHScattTheta = MakeTH1('D',"ThetaScatt","Streuwinkel #theta_\{scatt} (vert.)",200,-10,10);
	fHScattTheta->GetXaxis()->SetTitle("#theta_\{scatt} / deg");
	fHScattTheta->GetXaxis()->CenterTitle();
	fHScattTheta->GetYaxis()->SetTitle("Haeufigkeit / (0.10 deg)");
	fHScattTheta->GetYaxis()->CenterTitle();

	/// TARGETPOSITION
	fHScattXPos = MakeTH1('D',"ScattXPosition","Target Streuposition x",1000,-1000,1000);
	fHScattYPos = MakeTH1('D',"ScattYPosition","Target Streuposition y",1000,-1000,1000);
	fHScattXYPos = MakeTH2('D',"ScattXYPosition","Target Streuposition (x,y)",1000,-1000,1000,1000,-1000,1000);



	fHBeamX_All = MakeTH1('D',"BeamX_All","Target X position;X [mm];Counts",1000,-1000,1000);

	fHBeamY_All = MakeTH1('D',"BeamY_All","Target Y position;Y [mm];Counts",1000,-1000,1000);

	fHBeamXY_All = MakeTH2('D',"BeamXY_All","Target position;X [mm];Y [mm]",1000,-1000,1000,1000,-1000,1000);


	fHBeamX_Live = MakeTH1('D',"BeamX_Live","Live Target X position;X [mm];Counts",1000,-1000,1000);

	fHBeamY_Live = MakeTH1('D',"BeamY_Live","Live Target Y position;Y [mm];Counts",1000,-1000,1000);

	fHBeamXY_Live = MakeTH2('D',"BeamXY_Live","Live Target position;X [mm];Y [mm]",1000,-1000,1000,1000,-1000,1000);

	
	fPBeamX_Mean = new TGraph();
	fPBeamX_Mean->SetName("BeamX_Mean");
	fPBeamX_Mean->SetTitle("Running mean X");
	fPBeamX_Mean->GetXaxis()->SetTitle("Zeit / Min");
	fPBeamX_Mean->GetYaxis()->SetTitle("x / mm");
	AddObject(fPBeamX_Mean);

	fPBeamY_Mean = new TGraph();
	fPBeamY_Mean->SetName("BeamY_Mean");
	fPBeamY_Mean->SetTitle("Running mean Y");
	fPBeamY_Mean->GetXaxis()->SetTitle("Zeit / Min");
	fPBeamY_Mean->GetYaxis()->SetTitle("y / mm");
	AddObject(fPBeamY_Mean);



	/// 2D  Detektorkoordinaten/-winkel
	fHPhiVsX1 = MakeTH2('D',"PhiVsX1","Phi vs X1",1000,0,1000,400,0,100);
	fHPhiVsX1->GetXaxis()->SetTitle("x / mm (X1)");
	fHPhiVsX1->GetXaxis()->CenterTitle();
	fHPhiVsX1->GetYaxis()->SetTitle("#phi / deg");
	fHPhiVsX1->GetYaxis()->CenterTitle();

	fHPhiCorrVsX1Corr = MakeTH2('D',"PhiCorrVsX1Corr","Phi vs X1 - korrigiert",1000,0,1000,400,-100,100);
	fHPhiCorrVsX1Corr->GetXaxis()->SetTitle("x / mm (X1)");
	fHPhiCorrVsX1Corr->GetXaxis()->CenterTitle();
	fHPhiCorrVsX1Corr->GetYaxis()->SetTitle("#phi / deg");
	fHPhiCorrVsX1Corr->GetYaxis()->CenterTitle();


	fHPhiVsX2 = MakeTH2('D',"PhiVsX2","Phi vs X2",10000,-100,900,1600,0,100);
	fHPhiVsX2->GetXaxis()->SetTitle("x / mm (X2)");
	fHPhiVsX2->GetXaxis()->CenterTitle();
	fHPhiVsX2->GetYaxis()->SetTitle("#phi / deg");
	fHPhiVsX2->GetYaxis()->CenterTitle();


	fHThetaVsY = MakeTH2('D',"ThetaVsY","Theta vs Y",2000,-100,100,800,-100,100);
	fHThetaVsY->GetXaxis()->SetTitle("y / mm (U)");
	fHThetaVsY->GetXaxis()->CenterTitle();
	fHThetaVsY->GetYaxis()->SetTitle("#theta / deg");
	fHThetaVsY->GetYaxis()->CenterTitle();


	fHPhiVsY = MakeTH2('D',"PhiVsY","Phi vs Y",400,-100,100,800,0,100);
	fHPhiVsY->GetXaxis()->SetTitle("y / mm (U)");
	fHPhiVsY->GetXaxis()->CenterTitle();
	fHPhiVsY->GetYaxis()->SetTitle("#phi / deg");
	fHPhiVsY->GetYaxis()->CenterTitle();


	fHPhiCorrVsY = MakeTH2('D',"PhiCorrVsY","PhiCorr vs Y",400,-100,100,1600,-100,100);
	fHPhiCorrVsY->GetXaxis()->SetTitle("y / mm (U)");
	fHPhiCorrVsY->GetXaxis()->CenterTitle();
	fHPhiCorrVsY->GetYaxis()->SetTitle("#phi / deg");
	fHPhiCorrVsY->GetYaxis()->CenterTitle();

	fHThetaVsPhi = MakeTH2('D',"ThetaVsPhi","Theta Vs PhiCorr",200,-50,50,400,-100,100);
	fHThetaVsPhi->GetXaxis()->SetTitle("#phi_{corr} / deg");
	fHThetaVsPhi->GetXaxis()->CenterTitle();
	fHThetaVsPhi->GetYaxis()->SetTitle("#theta / deg");
	fHThetaVsPhi->GetYaxis()->CenterTitle();

	fHYVsX = MakeTH2('D',"YVsX","Y vs XCorr",1000,0,1000,800,-100,100);
	fHYVsX->GetXaxis()->SetTitle("x_{corr} (X1) / mm");
	fHYVsX->GetXaxis()->CenterTitle();
	fHYVsX->GetYaxis()->SetTitle("y / mm");
	fHYVsX->GetYaxis()->CenterTitle();

	/** Spektrum in Fokalebene */
	fHProjSpectrum = MakeTH1('D',"ProjSpectrum","Projektion des X1-Spektrums in die Fokalebene", 1000,0,1000);
	fHProjSpectrum->GetXaxis()->SetTitle("Kanal");
	fHProjSpectrum->GetXaxis()->CenterTitle();
	fHProjSpectrum->GetYaxis()->SetTitle("Haeufigkeit");
	fHProjSpectrum->GetYaxis()->CenterTitle();


	/** kalibrierte Spektren */
	fHXCorrSpectrum = MakeTH1('D',"XCorrSpectrum","Korrigiertes X1-Spektrum (X-Phi Korrektur)",1000,0,1000);
	fHXCorrSpectrum->GetXaxis()->SetTitle("x / mm (korrigiert");
	fHXCorrSpectrum->GetXaxis()->CenterTitle();
	fHXCorrSpectrum->GetYaxis()->SetTitle("Haeufigeit");
	fHXCorrSpectrum->GetYaxis()->CenterTitle();

	fHExEnergySpectrum = MakeTH1('D',"ElectronEnergySpectrum","Rest-Energie Elektron",11000,-10,100);
	fHExEnergySpectrum->GetXaxis()->SetTitle("Energie / MeV");
	fHExEnergySpectrum->GetXaxis()->CenterTitle();
	fHExEnergySpectrum->GetYaxis()->SetTitle("Haeufigeit");
	fHExEnergySpectrum->GetYaxis()->CenterTitle();;


	fHDPhiVsIteration = MakeTH2('D',"DPhiVsIteration","Aenderung in #phi je Iteration",30,0,30,10000,0.,0.01);
	fHDPhiVsIteration->GetXaxis()->SetTitle("Iteration");
	fHDPhiVsIteration->GetXaxis()->CenterTitle();
	fHDPhiVsIteration->GetYaxis()->SetTitle("|#Delta#varphi| / deg");
	fHDPhiVsIteration->GetYaxis()->CenterTitle();

	fHRmsVsIteration = MakeTH2('D',"RmsVsIteration","RMS Residuum vs. Iteration",30,0,30,100,0,10.);
	fHRmsVsIteration->GetXaxis()->SetTitle("Iteration");
	fHRmsVsIteration->GetXaxis()->CenterTitle();
	fHRmsVsIteration->GetYaxis()->SetTitle("RMS Residuum");
	fHRmsVsIteration->GetYaxis()->CenterTitle();


	fHPhi_PhiX1 = MakeTH1('D',"PhiMinusPhiX1","#phi_{rekonstruiert} - #phi_\{X1}",160,-20,20);
	fHPhi_PhiX1->GetXaxis()->SetTitle("(#phi_\{rec} - #phi_\{X1}) / deg");
	fHPhi_PhiX1->GetXaxis()->CenterTitle();
	fHPhi_PhiX1->GetYaxis()->SetTitle("Haeufigkeit / (0.25 deg)");
	fHPhi_PhiX1->GetYaxis()->CenterTitle();

	fHPhi_PhiU = MakeTH1('D',"PhiMinusPhiU","#phi_{rekonstruiert} - #phi_\{U}",500,-50,50);
	fHPhi_PhiU->GetXaxis()->SetTitle("(#phi_\{rec} - #phi_\{U}) / deg");
	fHPhi_PhiU->GetXaxis()->CenterTitle();
	fHPhi_PhiU->GetYaxis()->SetTitle("Haeufigkeit / (0.25 deg)");
	fHPhi_PhiU->GetYaxis()->CenterTitle();

	fHPhi_PhiX2 = MakeTH1('D',"PhiMinusPhiX2","#phi_{rekonstruiert} - #phi_\{X2}",160,-20,20);
	fHPhi_PhiX2->GetXaxis()->SetTitle("(#phi_\{rec} - #phi_\{X2}) / deg");
	fHPhi_PhiX2->GetXaxis()->CenterTitle();
	fHPhi_PhiX2->GetYaxis()->SetTitle("Haeufigkeit / (0.25 deg)");
	fHPhi_PhiX2->GetYaxis()->CenterTitle();


	/// 2D-Hists mit Detektorwinkeln
	fHPhiVsTheta = MakeTH2('D',"Phi_Vs_Theta","#phi_\{det} vs. #theta_\{det}",800,-100,100,400,0,100);
	fHPhiVsTheta->GetXaxis()->SetTitle("#theta_\{det} / (0.25 deg)");
	fHPhiVsTheta->GetXaxis()->CenterTitle();
	fHPhiVsTheta->GetYaxis()->SetTitle("#phi_\{det} / (0.25 deg)");
	fHPhiVsTheta->GetYaxis()->CenterTitle();

	/// 2D-Hists mit Streuwinkeln
	fHScattPhiVsScattTheta = MakeTH2('D',"ScattPhi_Vs_ScattTheta","Horiz. Streuwinkel #phi_\{scatt} Vert. Streuwinkel #theta_\{scatt}",200,-10,10,200,-10,10);
	fHScattPhiVsScattTheta->GetXaxis()->SetTitle("#theta_\{scatt} / (0.10 deg)");
	fHScattPhiVsScattTheta->GetXaxis()->CenterTitle();
	fHScattPhiVsScattTheta->GetYaxis()->SetTitle("#phi_\{scatt} / (0.10 deg)");
	fHScattPhiVsScattTheta->GetYaxis()->CenterTitle();

	// Streuwinkel vs X1
	fHScattPhiVsX1 = MakeTH2('D',"ScattPhi_Vs_X1","Horiz. Streuwinkel #phi_\{scatt} vs. X1-Spektrum",1000,0,1000,200,-10,10);
	fHScattPhiVsX1->GetXaxis()->SetTitle("x / mm");
	fHScattPhiVsX1->GetXaxis()->CenterTitle();
	fHScattPhiVsX1->GetYaxis()->SetTitle("#phi_\{scatt} / (0.10 deg)");
	fHScattPhiVsX1->GetYaxis()->CenterTitle();

	fHScattThetaVsX1 = MakeTH2('D',"ScattTheta_Vs_X1","Vert. Streuwinkel #theta_\{scatt} vs. X1-Spektrum",1000,0,1000,200,-10,10);
	fHScattThetaVsX1->GetXaxis()->SetTitle("x / mm");
	fHScattThetaVsX1->GetXaxis()->CenterTitle();
	fHScattThetaVsX1->GetYaxis()->SetTitle("#theta_\{scatt} / (0.10 deg)");
	fHScattThetaVsX1->GetYaxis()->CenterTitle();


	// Streuwinkel vs XCorr
	fHScattPhiVsXCorr = MakeTH2('D',"ScattPhi_Vs_XCorr","Horiz. Streuwinkel #phi_\{scatt} vs. Korrigiertes X-Spektrum",1000,0,1000,200,-10,10);
	fHScattPhiVsXCorr->GetXaxis()->SetTitle("x / mm");
	fHScattPhiVsXCorr->GetXaxis()->CenterTitle();
	fHScattPhiVsXCorr->GetYaxis()->SetTitle("#phi_\{scatt} / (0.10 deg)");
	fHScattPhiVsXCorr->GetYaxis()->CenterTitle();

	fHScattThetaVsXCorr = MakeTH2('D',"ScattTheta_Vs_XCorr","Vert. Streuwinkel #theta_\{scatt} vs. Korrigiertes X-Spektrum",1000,0,1000,200,-10,10);
	fHScattThetaVsXCorr->GetXaxis()->SetTitle("x / mm");
	fHScattThetaVsXCorr->GetXaxis()->CenterTitle();
	fHScattThetaVsXCorr->GetYaxis()->SetTitle("#theta_\{scatt} / (0.10 deg)");
	fHScattThetaVsXCorr->GetYaxis()->CenterTitle();

	// Streuwinkel vs Y
	fHScattPhiVsY = MakeTH2('D',"ScattPhi_Vs_Y","Horiz. Streuwinkel #phi_\{scatt} vs. Y-Spektrum",200,-100,100,200,-10,10);
	fHScattPhiVsY->GetXaxis()->SetTitle("y / mm");
	fHScattPhiVsY->GetXaxis()->CenterTitle();
	fHScattPhiVsY->GetYaxis()->SetTitle("#phi_\{scatt} / (0.10 deg)");
	fHScattPhiVsY->GetYaxis()->CenterTitle();

	fHScattThetaVsY = MakeTH2('D',"ScattTheta_Vs_Y","Vert. Streuwinkel #theta_\{scatt} vs. Y-Spektrum",200,-100,100,200,-10,10);
	fHScattThetaVsY->GetXaxis()->SetTitle("y / mm");
	fHScattThetaVsY->GetXaxis()->CenterTitle();
	fHScattThetaVsY->GetYaxis()->SetTitle("#theta_\{scatt} / (0.10 deg)");
	fHScattThetaVsY->GetYaxis()->CenterTitle();


	// Streuwinkel vs. Detektorwinkel
	fHScattPhiVsPhi = MakeTH2('D',"ScattPhi_Vs_Phi","Horiz. Streuwinkel #phi_\{scatt} vs. Detektorwinkel #phi_\{det}",1000,0,100,200,-10,10);
	fHScattPhiVsPhi->GetXaxis()->SetTitle("#phi_\{det} / (0.10 deg)");
	fHScattPhiVsPhi->GetXaxis()->CenterTitle();
	fHScattPhiVsPhi->GetYaxis()->SetTitle("#phi_\{scatt} / (0.10 deg)");
	fHScattPhiVsPhi->GetYaxis()->CenterTitle();

	fHScattPhiVsPhiCorr = MakeTH2('D',"ScattPhi_Vs_PhiCorr","Horiz. Streuwinkel #phi_\{scatt} vs. Korr. Detektorwinkel #phi_\{det}",1000,-50,50,200,-10,10);
	fHScattPhiVsPhiCorr->GetXaxis()->SetTitle("Korr. #phi_\{det} / (0.10 deg)");
	fHScattPhiVsPhiCorr->GetXaxis()->CenterTitle();
	fHScattPhiVsPhiCorr->GetYaxis()->SetTitle("#phi_\{scatt} / (0.10 deg)");
	fHScattPhiVsPhiCorr->GetYaxis()->CenterTitle();

	fHScattThetaVsTheta = MakeTH2('D',"ScattTheta_Vs_Theta","Vert. Streuwinkel #theta_\{scatt} vs. Detektorwinkel #theta_\{det}",2000,-100,100,200,-10,10);
	fHScattThetaVsTheta->GetXaxis()->SetTitle("#theta_\{det} / (0.10 deg)");
	fHScattThetaVsTheta->GetXaxis()->CenterTitle();
	fHScattThetaVsTheta->GetYaxis()->SetTitle("#theta_\{scatt} / (0.10 deg)");
	fHScattThetaVsTheta->GetYaxis()->CenterTitle();






	
	/// nicht noetig....
	fFitPar->SetSpectrum(fHProjSpectrum);

	fHFitSrc = MakeTH1('D',"FitSource","Copy of fit data",1000,0.,100.);
	fHFitRes = MakeTH1('D',"FitTarget","Copy of fit result",1000,0.,100.);




	fHChi2 = MakeTH1('D',"Chi2","Chi2 Distribution",1000,0,1000);
	fHChi2->GetXaxis()->SetTitle("chi2");
	fHChi2->GetXaxis()->CenterTitle();
	fHChi2->GetYaxis()->SetTitle("Haeufigkeit");
	fHChi2->GetYaxis()->CenterTitle();

	fHChi2NDF = MakeTH1('D',"Chi2NDF","Chi2/NDF Distribution",500,0,50);
	fHChi2NDF->GetXaxis()->SetTitle("chi2/ndf");
	fHChi2NDF->GetXaxis()->CenterTitle();
	fHChi2NDF->GetYaxis()->SetTitle("Haeufigkeit");
	fHChi2NDF->GetYaxis()->CenterTitle();

	fHMaxResidualAbs = MakeTH1('D',"MaxResidual","Maximale Residuen - letzte Iteration",1000,0,10);
	fHMaxResidualAbs->GetXaxis()->SetTitle("Max. Residuum / mm");
	fHMaxResidualAbs->GetXaxis()->CenterTitle();
	fHMaxResidualAbs->GetYaxis()->SetTitle("Haeufigkeit / (0.01 mm)");
	fHMaxResidualAbs->GetYaxis()->CenterTitle();

	fHMeanResidual = MakeTH1('D',"MeanResidual","Mittlere Residuen - letzte Iteration",2000,-0.1,0.1);
	fHMeanResidual->GetXaxis()->SetTitle("Mittl. Residuum / mm");
	fHMeanResidual->GetXaxis()->CenterTitle();
	fHMeanResidual->GetYaxis()->SetTitle("Haeufigkeit / (0.01 mm)");
	fHMeanResidual->GetYaxis()->CenterTitle();

	fHConverged = MakeTH1('I',"Converged","Spurrekonstruktion konvergiert",2,0,2);
	fHConverged->GetXaxis()->SetBinLabel(1,"konvergiert");
	fHConverged->GetXaxis()->SetBinLabel(2,"nicht konvergiert");

	fHIterations = MakeTH1('I',"Iterations","Track-Iterationen",50,0,50);
	fHIterations->GetXaxis()->SetTitle("Anzahl Iterationen");
	fHIterations->GetXaxis()->CenterTitle();
	fHIterations->GetYaxis()->SetTitle("Haeufigkeit");
	fHIterations->GetYaxis()->CenterTitle();

	fHRejected = MakeTH1('D',"RejectedHits","Verworfene Hits je Track",20,0,20);
	fHRejected->GetXaxis()->SetTitle("Anzahl verworfene Hits/Track");
	fHRejected->GetXaxis()->CenterTitle();
	fHRejected->GetYaxis()->SetTitle("Rel. Haeufigkeit (norm. auf # Tracks)");
	fHRejected->GetYaxis()->CenterTitle();

	fHRejectedHits_X1 = MakeTH1('D',"RejectedHits_X1","Verworfene Hits je Track X1",20,0,20);
	fHRejectedHits_X1->GetXaxis()->SetTitle("Anzahl verworfene Hits/Track");
	fHRejectedHits_X1->GetXaxis()->CenterTitle();
	fHRejectedHits_X1->GetYaxis()->SetTitle("Rel. Haeufigkeit (norm. auf # Tracks)");
	fHRejectedHits_X1->GetYaxis()->CenterTitle();

	fHRejectedHits_U = MakeTH1('D',"RejectedHits_U","Verworfene Hits je Track U",20,0,20);
	fHRejectedHits_U->GetXaxis()->SetTitle("Anzahl verworfene Hits/Track");
	fHRejectedHits_U->GetXaxis()->CenterTitle();
	fHRejectedHits_U->GetYaxis()->SetTitle("Rel. Haeufigkeit (norm. auf # Tracks)");
	fHRejectedHits_U->GetYaxis()->CenterTitle();

	fHRejectedHits_X2 = MakeTH1('D',"RejectedHits_X2","Verworfene Hits je Track X2",20,0,20);
	fHRejectedHits_X2->GetXaxis()->SetTitle("Anzahl verworfene Hits/Track");
	fHRejectedHits_X2->GetXaxis()->CenterTitle();
	fHRejectedHits_X2->GetYaxis()->SetTitle("Rel. Haeufigkeit (norm. auf # Tracks)");
	fHRejectedHits_X2->GetYaxis()->CenterTitle();



	fHUsedHits_X1 = MakeTH1('D',"UsedHits_X1","Verwendete Hits je Track X1",50,0,50);
	fHUsedHits_X1->GetXaxis()->SetTitle("Anzahl verwendete Hits/Track");
	fHUsedHits_X1->GetXaxis()->CenterTitle();
	fHUsedHits_X1->GetYaxis()->SetTitle("Rel. Haeufigkeit (norm. auf # Tracks)");
	fHUsedHits_X1->GetYaxis()->CenterTitle();

	fHUsedHits_U = MakeTH1('D',"UsedHits_U","Verwendete Hits je Track U",50,0,50);
	fHUsedHits_U->GetXaxis()->SetTitle("Anzahl verwendete Hits/Track");
	fHUsedHits_U->GetXaxis()->CenterTitle();
	fHUsedHits_U->GetYaxis()->SetTitle("Rel. Haeufigkeit (norm. auf # Tracks)");
	fHUsedHits_U->GetYaxis()->CenterTitle();

	fHUsedHits_X2 = MakeTH1('D',"UsedHits_X2","Verwendete Hits je Track X2",50,0,50);
	fHUsedHits_X2->GetXaxis()->SetTitle("Anzahl verwendete Hits/Track");
	fHUsedHits_X2->GetXaxis()->CenterTitle();
	fHUsedHits_X2->GetYaxis()->SetTitle("Rel. Haeufigkeit (norm. auf # Tracks)");
	fHUsedHits_X2->GetYaxis()->CenterTitle();




	fHPull = MakeTH1('D', "Pull", "Kriterium zum Hit-Ausschluss", 500, -1, 19);
	fHPull->GetXaxis()->SetTitle("Residuum**2 / mm**2");
	fHPull->GetXaxis()->CenterTitle();
	fHPull->GetYaxis()->SetTitle("Haeufigkeit / (0.4 mm**2)");
	fHPull->GetYaxis()->CenterTitle();
	fCondPull->SetHistogram("Pull");

	fHResidualVsWireX1 = MakeTH2('D', "ResidualVsWireX1", "X1", 320,0,320,4000,-20,20);
	// U, X2, Fill


	fHMeanResidual_X1 = MakeTH1('D',"MeanResiduals_X1", "Mittlere Residuen X1 - letzte Iteration", 2000,-0.01,0.01);
	fHMeanResidual_X1->GetXaxis()->SetTitle("Mittl. Residuum X1 / mm");
	fHMeanResidual_X1->GetXaxis()->CenterTitle();
	fHMeanResidual_X1->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHMeanResidual_X1->GetYaxis()->CenterTitle();

	fHMeanResidual_U = MakeTH1('D',"MeanResiduals_U", "Mittlere Residuen U - letzte Iteration", 2000,-0.01,0.01);
	fHMeanResidual_U->GetXaxis()->SetTitle("Mittl. Residuum U / mm");
	fHMeanResidual_U->GetXaxis()->CenterTitle();
	fHMeanResidual_U->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHMeanResidual_U->GetYaxis()->CenterTitle();

	fHMeanResidual_X2 = MakeTH1('D',"MeanResiduals_X2", "Mittlere Residuen X2 - letzte Iteration", 2000,-0.01,0.01);
	fHMeanResidual_X2->GetXaxis()->SetTitle("Mittl. Residuum X2 / mm");
	fHMeanResidual_X2->GetXaxis()->CenterTitle();
	fHMeanResidual_X2->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHMeanResidual_X2->GetYaxis()->CenterTitle();


	fHRmsResidual_X1 = MakeTH1('D',"RmsResiduals_X1", "Root-Mean-Square Residuen X1 - letzte Iteration", 1000,0.,8.);
	fHRmsResidual_X1->GetXaxis()->SetTitle("RMS Residuum X1 / mm");
	fHRmsResidual_X1->GetXaxis()->CenterTitle();
	fHRmsResidual_X1->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHRmsResidual_X1->GetYaxis()->CenterTitle();

	fHRmsResidual_U = MakeTH1('D',"RmsResiduals_U", "Root-Mean-Square Residuen U - letzte Iteration", 1000,0.,8.);
	fHRmsResidual_U->GetXaxis()->SetTitle("RMS Residuum U / mm");
	fHRmsResidual_U->GetXaxis()->CenterTitle();
	fHRmsResidual_U->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHRmsResidual_U->GetYaxis()->CenterTitle();

	fHRmsResidual_X2 = MakeTH1('D',"RmsResiduals_X2", "Root-Mean-Square Residuen X2 - letzte Iteration",1000,0.,8.);
	fHRmsResidual_X2->GetXaxis()->SetTitle("RMS Residuum X2 / mm");
	fHRmsResidual_X2->GetXaxis()->CenterTitle();
	fHRmsResidual_X2->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHRmsResidual_X2->GetYaxis()->CenterTitle();


	fHZResolution_X1 = MakeTH1('D',"ZResolution_X1","z-Aufloesung in X1",500,0.,5.);
	fHZResolution_X1->GetXaxis()->SetTitle("#sigma_{z} / mm");
	fHZResolution_X1->GetXaxis()->CenterTitle();
	fHZResolution_X1->GetYaxis()->SetTitle("Haeufigkeit");
	fHZResolution_X1->GetYaxis()->CenterTitle();

	fHZResolution_U = MakeTH1 ('D',"ZResolution_U","z-Aufloesung in U",500,0.,5.);
	fHZResolution_U->GetXaxis()->SetTitle("#sigma_{z} / mm");
	fHZResolution_U->GetXaxis()->CenterTitle();
	fHZResolution_U->GetYaxis()->SetTitle("Haeufigkeit");
	fHZResolution_U->GetYaxis()->CenterTitle();

	fHZResolution_X2 = MakeTH1('D',"ZResolution_X2","z-Aufloesung in X2",500,0.,5.);
	fHZResolution_X2->GetXaxis()->SetTitle("#sigma_{z} / mm");
	fHZResolution_X2->GetXaxis()->CenterTitle();
	fHZResolution_X2->GetYaxis()->SetTitle("Haeufigkeit");
	fHZResolution_X2->GetYaxis()->CenterTitle();


	fHXResolution_X1 = MakeTH1('D',"XResolution_X1","x-Aufloesung in X1",500,0.,5.);
	fHXResolution_X1->GetXaxis()->SetTitle("#sigma_{x} / mm");
	fHXResolution_X1->GetXaxis()->CenterTitle();
	fHXResolution_X1->GetYaxis()->SetTitle("Haeufigkeit");
	fHXResolution_X1->GetYaxis()->CenterTitle();

	fHXResolution_X2 = MakeTH1('D',"XResolution_X2","x-Aufloesung in X2",500,0.,5.);
	fHXResolution_X2->GetXaxis()->SetTitle("#sigma_{x} / mm");
	fHXResolution_X2->GetXaxis()->CenterTitle();
	fHXResolution_X2->GetYaxis()->SetTitle("Haeufigkeit");
	fHXResolution_X2->GetYaxis()->CenterTitle();

	fHUResolution_U = MakeTH1('D',"UResolution_U","u-Aufloesung in U",500,0.,5.);
	fHUResolution_U->GetXaxis()->SetTitle("#sigma_{u} / mm");
	fHUResolution_U->GetXaxis()->CenterTitle();
	fHUResolution_U->GetYaxis()->SetTitle("Haeufigkeit");
	fHUResolution_U->GetYaxis()->CenterTitle();

	fHYResolution_U = MakeTH1('D',"YResolution_U","y-Aufloesung in U",500,0.,5.);
	fHYResolution_U->GetXaxis()->SetTitle("#sigma_{y} / mm");
	fHYResolution_U->GetXaxis()->CenterTitle();
	fHYResolution_U->GetYaxis()->SetTitle("Haeufigkeit");
	fHYResolution_U->GetYaxis()->CenterTitle();



	fHResidual_X1 = MakeTH1('D',"Residuals_X1", "Residuen X1", 1000,-5.,5.);
	fHResidual_X1->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1->GetXaxis()->CenterTitle();
	fHResidual_X1->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X1->GetYaxis()->CenterTitle();

	fHResidual_U = MakeTH1('D',"Residuals_U", "Residuen U", 1000,-5.,5.);
	fHResidual_U->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U->GetXaxis()->CenterTitle();
	fHResidual_U->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_U->GetYaxis()->CenterTitle();

	fHResidual_X2 = MakeTH1('D',"Residuals_X2", "Residuen X2", 1000,-5.,5.);
	fHResidual_X2->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2->GetXaxis()->CenterTitle();
	fHResidual_X2->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X2->GetYaxis()->CenterTitle();


	/// Residuen nach phi-bins

	fHResidual_X1_phiBin_25_35 = MakeTH1('D',"Residuals_X1_phiBin_25_35", "Residuen X1 (25#circ #leq #phi < 35#circ)", 1000,-5.,5.);
	fHResidual_X1_phiBin_25_35->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_phiBin_25_35->GetXaxis()->CenterTitle();
	fHResidual_X1_phiBin_25_35->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X1_phiBin_25_35->GetYaxis()->CenterTitle();

	fHResidual_U_phiBin_25_35 = MakeTH1('D',"Residuals_U_phiBin_25_35", "Residuen U (25#circ #leq #phi < 35#circ)", 1000,-5.,5.);
	fHResidual_U_phiBin_25_35->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_phiBin_25_35->GetXaxis()->CenterTitle();
	fHResidual_U_phiBin_25_35->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_U_phiBin_25_35->GetYaxis()->CenterTitle();

	fHResidual_X2_phiBin_25_35 = MakeTH1('D',"Residuals_X2_phiBin_25_35", "Residuen X2 (25#circ #leq #phi < 35#circ)", 1000,-5.,5.);
	fHResidual_X2_phiBin_25_35->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_phiBin_25_35->GetXaxis()->CenterTitle();
	fHResidual_X2_phiBin_25_35->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X2_phiBin_25_35->GetYaxis()->CenterTitle();




	fHResidual_X1_phiBin_35_45 = MakeTH1('D',"Residuals_X1_phiBin_35_45", "Residuen X1 (35#circ #leq #phi < 45#circ)", 1000,-5.,5.);
	fHResidual_X1_phiBin_35_45->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_phiBin_35_45->GetXaxis()->CenterTitle();
	fHResidual_X1_phiBin_35_45->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X1_phiBin_35_45->GetYaxis()->CenterTitle();

	fHResidual_U_phiBin_35_45 = MakeTH1('D',"Residuals_U_phiBin_35_45", "Residuen U (35#circ #leq #phi < 45#circ)", 4000,-5.,5.);
	fHResidual_U_phiBin_35_45->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_phiBin_35_45->GetXaxis()->CenterTitle();
	fHResidual_U_phiBin_35_45->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_U_phiBin_35_45->GetYaxis()->CenterTitle();

	fHResidual_X2_phiBin_35_45 = MakeTH1('D',"Residuals_X2_phiBin_35_45", "Residuen X2 (35#circ #leq #phi < 45#circ)", 4000,-5.,5.);
	fHResidual_X2_phiBin_35_45->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_phiBin_35_45->GetXaxis()->CenterTitle();
	fHResidual_X2_phiBin_35_45->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X2_phiBin_35_45->GetYaxis()->CenterTitle();





	fHResidual_X1_phiBin_45_55 = MakeTH1('D',"Residuals_X1_phiBin_45_55", "Residuen X1 (45#circ #leq #phi < 55#circ)", 1000,-5.,5.);
	fHResidual_X1_phiBin_45_55->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_phiBin_45_55->GetXaxis()->CenterTitle();
	fHResidual_X1_phiBin_45_55->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X1_phiBin_45_55->GetYaxis()->CenterTitle();

	fHResidual_U_phiBin_45_55 = MakeTH1('D',"Residuals_U_phiBin_45_55", "Residuen U (45#circ #leq #phi < 55#circ)", 1000,-5.,5.);
	fHResidual_U_phiBin_45_55->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_phiBin_45_55->GetXaxis()->CenterTitle();
	fHResidual_U_phiBin_45_55->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_U_phiBin_45_55->GetYaxis()->CenterTitle();

	fHResidual_X2_phiBin_45_55 = MakeTH1('D',"Residuals_X2_phiBin_45_55", "Residuen X2 (45#circ #leq #phi < 55#circ)", 1000,-5.,5.);
	fHResidual_X2_phiBin_45_55->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_phiBin_45_55->GetXaxis()->CenterTitle();
	fHResidual_X2_phiBin_45_55->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X2_phiBin_45_55->GetYaxis()->CenterTitle();





	fHResidual_X1_phiBin_55_65 = MakeTH1('D',"Residuals_X1_phiBin_55_65", "Residuen X1 (55#circ #leq #phi < 65#circ)", 1000,-5.,5.);
	fHResidual_X1_phiBin_55_65->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_phiBin_55_65->GetXaxis()->CenterTitle();
	fHResidual_X1_phiBin_55_65->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X1_phiBin_55_65->GetYaxis()->CenterTitle();

	fHResidual_U_phiBin_55_65 = MakeTH1('D',"Residuals_U_phiBin_55_65", "Residuen U (55#circ #leq #phi < 65#circ)", 1000,-5.,5.);
	fHResidual_U_phiBin_55_65->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_phiBin_55_65->GetXaxis()->CenterTitle();
	fHResidual_U_phiBin_55_65->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_U_phiBin_55_65->GetYaxis()->CenterTitle();

	fHResidual_X2_phiBin_55_65 = MakeTH1('D',"Residuals_X2_phiBin_55_65", "Residuen X2 (55#circ #leq #phi < 65#circ)", 1000,-5.,5.);
	fHResidual_X2_phiBin_55_65->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_phiBin_55_65->GetXaxis()->CenterTitle();
	fHResidual_X2_phiBin_55_65->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X2_phiBin_55_65->GetYaxis()->CenterTitle();


	/// Residuen nach driftime-bins

	fHResidual_X1_drifttimeBin_0_50 = MakeTH1('D',"Residuals_X1_drifttimeBin_0_50", "Residuen X1 (0 ns #leq t_\{D} < 50 ns)", 1000,-5.,5.);
	fHResidual_X1_drifttimeBin_0_50->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_drifttimeBin_0_50->GetXaxis()->CenterTitle();
	fHResidual_X1_drifttimeBin_0_50->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X1_drifttimeBin_0_50->GetYaxis()->CenterTitle();

	fHResidual_U_drifttimeBin_0_50 = MakeTH1('D',"Residuals_U_drifttimeBin_0_50", "Residuen U (0 ns #leq t_\{D} < 50 ns)", 1000,-5.,5.);
	fHResidual_U_drifttimeBin_0_50->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_drifttimeBin_0_50->GetXaxis()->CenterTitle();
	fHResidual_U_drifttimeBin_0_50->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_U_drifttimeBin_0_50->GetYaxis()->CenterTitle();

	fHResidual_X2_drifttimeBin_0_50 = MakeTH1('D',"Residuals_X2_drifttimeBin_0_50", "Residuen X2 (0 ns #leq t_\{D} < 50 ns)", 1000,-5.,5.);
	fHResidual_X2_drifttimeBin_0_50->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_drifttimeBin_0_50->GetXaxis()->CenterTitle();
	fHResidual_X2_drifttimeBin_0_50->GetYaxis()->SetTitle("Haeufigkeit / mm");
	fHResidual_X2_drifttimeBin_0_50->GetYaxis()->CenterTitle();



	fHResidual_X1_drifttimeBin_50_100 = MakeTH1('D',"Residuals_X1_drifttimeBin_50_100", "Residuen X1 (50 ns #leq t_\{D} < 100 ns)", 4000,-20.,20.);
	fHResidual_X1_drifttimeBin_50_100->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_drifttimeBin_50_100->GetXaxis()->CenterTitle();
	fHResidual_X1_drifttimeBin_50_100->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X1_drifttimeBin_50_100->GetYaxis()->CenterTitle();

	fHResidual_U_drifttimeBin_50_100 = MakeTH1('D',"Residuals_U_drifttimeBin_50_100", "Residuen U (50 ns #leq t_\{D} < 100 ns)", 4000,-20.,20.);
	fHResidual_U_drifttimeBin_50_100->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_drifttimeBin_50_100->GetXaxis()->CenterTitle();
	fHResidual_U_drifttimeBin_50_100->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_U_drifttimeBin_50_100->GetYaxis()->CenterTitle();

	fHResidual_X2_drifttimeBin_50_100 = MakeTH1('D',"Residuals_X2_drifttimeBin_50_100", "Residuen X2 (50 ns #leq t_\{D} < 100 ns)", 4000,-20.,20.);
	fHResidual_X2_drifttimeBin_50_100->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_drifttimeBin_50_100->GetXaxis()->CenterTitle();
	fHResidual_X2_drifttimeBin_50_100->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X2_drifttimeBin_50_100->GetYaxis()->CenterTitle();




	fHResidual_X1_drifttimeBin_100_150 = MakeTH1('D',"Residuals_X1_drifttimeBin_100_150", "Residuen X1 (100 ns #leq t_\{D} < 150 ns)", 4000,-20.,20.);
	fHResidual_X1_drifttimeBin_100_150->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_drifttimeBin_100_150->GetXaxis()->CenterTitle();
	fHResidual_X1_drifttimeBin_100_150->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X1_drifttimeBin_100_150->GetYaxis()->CenterTitle();

	fHResidual_U_drifttimeBin_100_150 = MakeTH1('D',"Residuals_U_drifttimeBin_100_150", "Residuen U (100 ns #leq t_\{D} < 150 ns)", 4000,-20.,20.);
	fHResidual_U_drifttimeBin_100_150->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_drifttimeBin_100_150->GetXaxis()->CenterTitle();
	fHResidual_U_drifttimeBin_100_150->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_U_drifttimeBin_100_150->GetYaxis()->CenterTitle();

	fHResidual_X2_drifttimeBin_100_150 = MakeTH1('D',"Residuals_X2_drifttimeBin_100_150", "Residuen X2 (100 ns #leq t_\{D} < 150 ns)", 4000,-20.,20.);
	fHResidual_X2_drifttimeBin_100_150->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_drifttimeBin_100_150->GetXaxis()->CenterTitle();
	fHResidual_X2_drifttimeBin_100_150->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X2_drifttimeBin_100_150->GetYaxis()->CenterTitle();



	fHResidual_X1_drifttimeBin_150_200 = MakeTH1('D',"Residuals_X1_drifttimeBin_150_200", "Residuen X1 (150 ns #leq t_\{D} < 200 ns)", 4000,-20.,20.);
	fHResidual_X1_drifttimeBin_150_200->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_drifttimeBin_150_200->GetXaxis()->CenterTitle();
	fHResidual_X1_drifttimeBin_150_200->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X1_drifttimeBin_150_200->GetYaxis()->CenterTitle();

	fHResidual_U_drifttimeBin_150_200 = MakeTH1('D',"Residuals_U_drifttimeBin_150_100", "Residuen U (150 ns #leq t_\{D} < 200 ns)", 4000,-20.,20.);
	fHResidual_U_drifttimeBin_150_200->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_drifttimeBin_150_200->GetXaxis()->CenterTitle();
	fHResidual_U_drifttimeBin_150_200->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_U_drifttimeBin_150_200->GetYaxis()->CenterTitle();

	fHResidual_X2_drifttimeBin_150_200 = MakeTH1('D',"Residuals_X2_drifttimeBin_150_100", "Residuen X2 (150 ns #leq t_\{D} < 200 ns)", 4000,-20.,20.);
	fHResidual_X2_drifttimeBin_150_200->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_drifttimeBin_150_200->GetXaxis()->CenterTitle();
	fHResidual_X2_drifttimeBin_150_200->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X2_drifttimeBin_150_200->GetYaxis()->CenterTitle();




	fHResidual_X1_drifttimeBin_200_250 = MakeTH1('D',"Residuals_X1_drifttimeBin_200_250", "Residuen X1 (200 ns #leq t_\{D} < 250 ns)", 4000,-20.,20.);
	fHResidual_X1_drifttimeBin_200_250->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_drifttimeBin_200_250->GetXaxis()->CenterTitle();
	fHResidual_X1_drifttimeBin_200_250->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X1_drifttimeBin_200_250->GetYaxis()->CenterTitle();

	fHResidual_U_drifttimeBin_200_250 = MakeTH1('D',"Residuals_U_drifttimeBin_200_250", "Residuen U (200 ns #leq t_\{D} < 250 ns)", 4000,-20.,20.);
	fHResidual_U_drifttimeBin_200_250->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_drifttimeBin_200_250->GetXaxis()->CenterTitle();
	fHResidual_U_drifttimeBin_200_250->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_U_drifttimeBin_200_250->GetYaxis()->CenterTitle();

	fHResidual_X2_drifttimeBin_200_250 = MakeTH1('D',"Residuals_X2_drifttimeBin_200_250", "Residuen X2 (200 ns #leq t_\{D} < 250 ns)", 4000,-20.,20.);
	fHResidual_X2_drifttimeBin_200_250->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_drifttimeBin_200_250->GetXaxis()->CenterTitle();
	fHResidual_X2_drifttimeBin_200_250->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X2_drifttimeBin_200_250->GetYaxis()->CenterTitle();



	fHResidual_X1_drifttimeBin_250_300 = MakeTH1('D',"Residuals_X1_drifttimeBin_250_300", "Residuen X1 (250 ns #leq t_\{D} < 300 ns)", 4000,-20.,20.);
	fHResidual_X1_drifttimeBin_250_300->GetXaxis()->SetTitle("Residuum X1 / mm");
	fHResidual_X1_drifttimeBin_250_300->GetXaxis()->CenterTitle();
	fHResidual_X1_drifttimeBin_250_300->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X1_drifttimeBin_250_300->GetYaxis()->CenterTitle();

	fHResidual_U_drifttimeBin_250_300 = MakeTH1('D',"Residuals_U_drifttimeBin_250_300", "Residuen U (250 ns #leq t_\{D} < 300 ns)", 4000,-20.,20.);
	fHResidual_U_drifttimeBin_250_300->GetXaxis()->SetTitle("Residuum U / mm");
	fHResidual_U_drifttimeBin_250_300->GetXaxis()->CenterTitle();
	fHResidual_U_drifttimeBin_250_300->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_U_drifttimeBin_250_300->GetYaxis()->CenterTitle();

	fHResidual_X2_drifttimeBin_250_300 = MakeTH1('D',"Residuals_X2_drifttimeBin_250_300", "Residuen X2 (250 ns #leq t_\{D} < 300 ns)", 4000,-20.,20.);
	fHResidual_X2_drifttimeBin_250_300->GetXaxis()->SetTitle("Residuum X2 / mm");
	fHResidual_X2_drifttimeBin_250_300->GetXaxis()->CenterTitle();
	fHResidual_X2_drifttimeBin_250_300->GetYaxis()->SetTitle("Haeufigkeit / (0.05 mm)");
	fHResidual_X2_drifttimeBin_250_300->GetYaxis()->CenterTitle();







        //-----------------------------------------------------------
	// TTrees


        //-----------------------------------------------------------
	// 4. Pictures 
	fPDriftDistanceVsTime = GetPicture("DriftDistVsTime");
	if(!fPDriftDistanceVsTime)
	{
		fPDriftDistanceVsTime = new TGo4Picture("DriftDistVsTime","Driftdistanz Vs. Driftzeit");
		fPDriftDistanceVsTime->SetDivision(2,4);

		fPDriftDistanceVsTime->Pic(0,0)->AddObject(fHDriftDistanceMeasVsTime_All);
		fPDriftDistanceVsTime->Pic(0,1)->AddObject(fHDriftDistanceMeasVsTime_X1);
		fPDriftDistanceVsTime->Pic(0,2)->AddObject(fHDriftDistanceMeasVsTime_U);
		fPDriftDistanceVsTime->Pic(0,3)->AddObject(fHDriftDistanceMeasVsTime_X2);

		fPDriftDistanceVsTime->Pic(1,0)->AddObject(fHDriftDistanceTrackVsTime_All);
		fPDriftDistanceVsTime->Pic(1,1)->AddObject(fHDriftDistanceTrackVsTime_X1);
		fPDriftDistanceVsTime->Pic(1,2)->AddObject(fHDriftDistanceTrackVsTime_U);
		fPDriftDistanceVsTime->Pic(1,3)->AddObject(fHDriftDistanceTrackVsTime_X2);


		AddPicture(fPDriftDistanceVsTime);
	}

	fPScattPosMon = GetPicture("BeamPositonMonitor");
	if(!fPScattPosMon)
	{
		fPScattPosMon = new TGo4Picture("BeamPositonMonitor","Streukoordinaten-Monitor");
		fPScattPosMon->SetDivision(3,3);

		fPScattPosMon->Pic(0,0)->AddObject(fHBeamX_All);
		fPScattPosMon->Pic(0,1)->AddObject(fHBeamY_All);
		fPScattPosMon->Pic(0,2)->AddObject(fHBeamXY_All);

		fPScattPosMon->Pic(1,0)->AddObject(fHBeamX_Live);
		fPScattPosMon->Pic(1,1)->AddObject(fHBeamY_Live);
		fPScattPosMon->Pic(1,2)->AddObject(fHBeamXY_Live);

		fPScattPosMon->Pic(2,0)->AddObject(fPBeamX_Mean);
		fPScattPosMon->Pic(2,1)->AddObject(fPBeamY_Mean);
		fPScattPosMon->Pic(2,2)->AddObject(fPBeamX_RMS);


		AddPicture(fPScattPosMon);
	}


	fPFocal1D = GetPicture("FocalPlane1D");
	if(!fPFocal1D)
	{
		fPFocal1D = new TGo4Picture("FocalPlane1D","1D-Spektren in Detektorkoordinatensystem");
		fPFocal1D->SetDivision(3,4);

		fPFocal1D->Pic(0,0)->AddObject(fHXSpectrumX1Plane);
		fPFocal1D->Pic(0,1)->AddObject(fHXSpectrumUPlane);
		fPFocal1D->Pic(0,2)->AddObject(fHXSpectrumX2Plane);
		fPFocal1D->Pic(0,3)->AddObject(fHProjSpectrum);

		fPFocal1D->Pic(1,0)->AddObject(fHUSpectrum);
		fPFocal1D->Pic(1,1)->AddObject(fHYSpectrumUPlane);
		fPFocal1D->Pic(1,2)->AddObject(fHXCorrSpectrum);
		fPFocal1D->Pic(1,3)->AddObject(fHExEnergySpectrum);

		fPFocal1D->Pic(2,0)->AddObject(fHPhiSpectrum);
		fPFocal1D->Pic(2,1)->AddObject(fHThetaSpectrum);
		fPFocal1D->Pic(2,2)->AddObject(fHPhiCorrSpectrum);
		//fPFocal1D->Pic(2,3)->AddObject(/** TODO Theta-Corr? */);

		AddPicture(fPFocal1D);
	}

	fPMatrixSpectra = GetPicture("FocalPlaneCorrelations");
	if(!fPMatrixSpectra)
	{
		fPMatrixSpectra = new TGo4Picture("FocalPlaneCorrelations","2D-Spektren in Detektorkoordinatensystem");
		fPMatrixSpectra->SetDivision(3,3);

		fPMatrixSpectra->Pic(0,0)->AddObject(fHPhiVsX1);
		fPMatrixSpectra->Pic(0,1)->AddObject(fHPhiVsX2);
		fPMatrixSpectra->Pic(0,2)->AddObject(fHPhiVsY);
		
		fPMatrixSpectra->Pic(1,0)->AddObject(fHPhiCorrVsX1Corr);
		fPMatrixSpectra->Pic(1,1)->AddObject(fHPhiCorrVsY);
		fPMatrixSpectra->Pic(1,2)->AddObject(fHYVsX);

		fPMatrixSpectra->Pic(2,0)->AddObject(fHThetaVsY);
		fPMatrixSpectra->Pic(2,1)->AddObject(fHThetaVsPhi);
		//fPMatrixSpectra->Pic(2,2)->AddObject();
		//fPMatrixSpectra->Pic()->AddObject();

		AddPicture(fPMatrixSpectra);
	}

	fPSieveSlitSpectra = GetPicture("SieveSlitSpectra");
	if(!fPSieveSlitSpectra)
	{
		fPSieveSlitSpectra = new TGo4Picture("SieveSlitSpectra","Sieve-Slit Spektren");
		fPSieveSlitSpectra->SetDivision(5,4);

		fPSieveSlitSpectra->Pic(0,0)->AddObject(fHXSpectrumX1Plane);
		fPSieveSlitSpectra->Pic(0,0)->AddCondition(fSieveSlitGate);
		//fPSieveSlitSpectra->Pic(1,0)->AddObject(fHXCorrSpectrum);
		fPSieveSlitSpectra->Pic(1,0)->AddObject(fHYSpectrumUPlane);
		fPSieveSlitSpectra->Pic(2,0)->AddObject(fHPhiSpectrum);
		fPSieveSlitSpectra->Pic(3,0)->AddObject(fHPhiCorrSpectrum);
		fPSieveSlitSpectra->Pic(4,0)->AddObject(fHThetaSpectrum);

		fPSieveSlitSpectra->Pic(0,1)->AddObject(fHScattPhi);
		fPSieveSlitSpectra->Pic(1,1)->AddObject(fHScattTheta);
		fPSieveSlitSpectra->Pic(2,1)->AddObject(fHPhiVsTheta);
		fPSieveSlitSpectra->Pic(3,1)->AddObject(fHPhiVsY);
		fPSieveSlitSpectra->Pic(4,1)->AddObject(fHPhiCorrVsY);



		fPSieveSlitSpectra->Pic(0,2)->AddObject(fHScattPhiVsX1);
		fPSieveSlitSpectra->Pic(1,2)->AddObject(fHScattPhiVsXCorr);
		fPSieveSlitSpectra->Pic(2,2)->AddObject(fHScattPhiVsY);
		fPSieveSlitSpectra->Pic(3,2)->AddObject(fHScattPhiVsPhi);
		fPSieveSlitSpectra->Pic(4,2)->AddObject(fHScattPhiVsPhiCorr);

		fPSieveSlitSpectra->Pic(0,3)->AddObject(fHScattThetaVsX1);
		fPSieveSlitSpectra->Pic(1,3)->AddObject(fHScattThetaVsXCorr);
		fPSieveSlitSpectra->Pic(2,3)->AddObject(fHScattThetaVsY);
		fPSieveSlitSpectra->Pic(3,3)->AddObject(fHScattThetaVsTheta);
		fPSieveSlitSpectra->Pic(4,3)->AddObject(fHScattPhiVsScattTheta);
		//fPSieveSlitSpectra->Pic(3,4)->AddObject();


		AddPicture(fPSieveSlitSpectra);
	}


	fPDispSpectra = GetPicture("DispersiveSpectra");
	if(!fPDispSpectra)
	{
		fPDispSpectra = new TGo4Picture("DispersiveSpectra","Dispersive Spektren");
		fPDispSpectra->SetDivision(3,4);

		fPDispSpectra->Pic(0,0)->AddObject(fHProjSpectrum);
		fPDispSpectra->Pic(0,1)->AddObject(fHXSpectrumX1Plane);
		fPDispSpectra->Pic(0,1)->AddCondition(fSieveSlitGate);

		fPDispSpectra->Pic(0,2)->AddObject(fHXSpectrumUPlane);
		fPDispSpectra->Pic(0,3)->AddObject(fHXSpectrumX2Plane);
		//fPDispSpectra->Pic(1,1)->AddObject(fHFitSrc);
		//fPDispSpectra->Pic(0,1)->AddObject(fHFitRes);

		fPDispSpectra->Pic(1,0)->AddObject(fHXCorrSpectrum);
		fPDispSpectra->Pic(1,1)->AddObject(fHPhiSpectrum);
		fPDispSpectra->Pic(1,2)->AddObject(fHPhiVsX1);
		fPDispSpectra->Pic(1,3)->AddObject(fHPhiCorrVsY);
		//fPDispSpectra->Pic(1,3)->AddObject(fHPhiVsX2);

		fPDispSpectra->Pic(2,0)->AddObject(fHExEnergySpectrum);
		fPDispSpectra->Pic(2,1)->AddObject(fHPhiCorrSpectrum);
		fPDispSpectra->Pic(2,2)->AddObject(fHPhiCorrVsX1Corr);
		fPDispSpectra->Pic(2,3)->AddObject(fHPhiVsY);

		AddPicture(fPDispSpectra);
	}

	fPNonDispSpectra = GetPicture("NonDispersiveSpectra");
	if(!fPNonDispSpectra)
	{
		fPNonDispSpectra = new TGo4Picture("NonDispersiveSpectra","Nicht-dispersive Spektren");
		fPNonDispSpectra->SetDivision(2,3);

		fPNonDispSpectra->Pic(0,0)->AddObject(fHUSpectrum);
		fPNonDispSpectra->Pic(0,1)->AddObject(fHYSpectrumUPlane);

		fPNonDispSpectra->Pic(1,0)->AddObject(fHThetaSpectrum);
		fPNonDispSpectra->Pic(1,1)->AddObject(fHThetaVsY);

		fPNonDispSpectra->Pic(0,2)->AddObject(fHYVsX);
		fPNonDispSpectra->Pic(1,2)->AddObject(fHThetaVsPhi);


		AddPicture(fPNonDispSpectra);
	}


	fPTrackResiduals = GetPicture("TrackQuality");
	if(!fPTrackResiduals)
	{
		fPTrackResiduals = new TGo4Picture("TrackQuality", "Track-Qualitaet");
		fPTrackResiduals->SetDivision(3,4);

		fPTrackResiduals->Pic(0,0)->AddObject(fHTrackEfficiency);
		//fPTrackResiduals->Pic(0,1)->AddObject(fHConverged);
		fPTrackResiduals->Pic(0,1)->AddObject(fHRmsVsIteration);
		//fPTrackResiduals->Pic(0,2)->AddObject(fHChi2);
		//fPTrackResiduals->Pic(0,3)->AddObject(fHChi2NDF);

		fPTrackResiduals->Pic(0,2)->AddObject(fHUsedHits_X1);
		fPTrackResiduals->Pic(0,2)->AddObject(fHUsedHits_U);
		fPTrackResiduals->Pic(0,2)->AddObject(fHUsedHits_X2);

		fPTrackResiduals->Pic(0,3)->AddObject(fHRejectedHits_X1);
		fPTrackResiduals->Pic(0,3)->AddObject(fHRejectedHits_U);
		fPTrackResiduals->Pic(0,3)->AddObject(fHRejectedHits_X2);
		

		fPTrackResiduals->Pic(1,0)->AddObject(fHMaxResidualAbs);
		//fPTrackResiduals->Pic(1,1)->AddObject(fHMeanResidual);
		fPTrackResiduals->Pic(1,1)->AddObject(fHDPhiVsIteration);
		fPTrackResiduals->Pic(1,2)->AddObject(fHPull);
		fPTrackResiduals->Pic(1,2)->AddCondition(fCondPull);
		fPTrackResiduals->Pic(1,3)->AddObject(fHRejected);

		fPTrackResiduals->Pic(2,0)->AddObject(fHIterations);
		fPTrackResiduals->Pic(2,1)->AddObject(fHPhi_PhiX1);
		fPTrackResiduals->Pic(2,2)->AddObject(fHPhi_PhiX2);
		fPTrackResiduals->Pic(2,3)->AddObject(fHPhi_PhiU);

		AddPicture(fPTrackResiduals);
	}

	fPTrackResidualsLayer = GetPicture("TrackResidualsLayer");
	if(!fPTrackResidualsLayer)
	{
		fPTrackResidualsLayer = new TGo4Picture("TrackResidualsLayer","Track-Residuen je Ebene");
		fPTrackResidualsLayer->SetDivision(3,3);

		fPTrackResidualsLayer->Pic(0,0)->AddObject(fHResidual_X1);
		fPTrackResidualsLayer->Pic(1,0)->AddObject(fHResidual_U);
		fPTrackResidualsLayer->Pic(2,0)->AddObject(fHResidual_X2);

		fPTrackResidualsLayer->Pic(0,1)->AddObject(fHRmsResidual_X1);
		fPTrackResidualsLayer->Pic(1,1)->AddObject(fHRmsResidual_U);
		fPTrackResidualsLayer->Pic(2,1)->AddObject(fHRmsResidual_X2);

		fPTrackResidualsLayer->Pic(0,2)->AddObject(fHMeanResidual_X1);
		fPTrackResidualsLayer->Pic(1,2)->AddObject(fHMeanResidual_U);
		fPTrackResidualsLayer->Pic(2,2)->AddObject(fHMeanResidual_X2);

		AddPicture(fPTrackResidualsLayer);
	}



	fPTrackResidualsPhiBins = GetPicture("TrackResidualsPhiBins");
	if(!fPTrackResidualsPhiBins)
	{
		fPTrackResidualsPhiBins = new TGo4Picture("TrackResidualsPhiBins","Track-Residuen je Ebene in #phi-Bins unterteilt");

		fPTrackResidualsPhiBins->SetDivision(2,2);

		fPTrackResidualsPhiBins->Pic(0,0)->AddObject(fHResidual_X1_phiBin_25_35);
		fPTrackResidualsPhiBins->Pic(0,0)->AddObject(fHResidual_U_phiBin_25_35);
		fPTrackResidualsPhiBins->Pic(0,0)->AddObject(fHResidual_X2_phiBin_25_35);

		fPTrackResidualsPhiBins->Pic(1,0)->AddObject(fHResidual_X1_phiBin_35_45);
		fPTrackResidualsPhiBins->Pic(1,0)->AddObject(fHResidual_U_phiBin_35_45);
		fPTrackResidualsPhiBins->Pic(1,0)->AddObject(fHResidual_X2_phiBin_35_45);

		fPTrackResidualsPhiBins->Pic(0,1)->AddObject(fHResidual_X1_phiBin_45_55);
		fPTrackResidualsPhiBins->Pic(0,1)->AddObject(fHResidual_U_phiBin_45_55);
		fPTrackResidualsPhiBins->Pic(0,1)->AddObject(fHResidual_X2_phiBin_45_55);

		fPTrackResidualsPhiBins->Pic(1,1)->AddObject(fHResidual_X1_phiBin_55_65);
		fPTrackResidualsPhiBins->Pic(1,1)->AddObject(fHResidual_U_phiBin_55_65);
		fPTrackResidualsPhiBins->Pic(1,1)->AddObject(fHResidual_X2_phiBin_55_65);

		AddPicture(fPTrackResidualsPhiBins);
	}




	fPTrackResidualsDrifttimeBins = GetPicture("TrackResidualsDrifttimeBins");
	if(!fPTrackResidualsDrifttimeBins)
	{
		fPTrackResidualsDrifttimeBins = new TGo4Picture("TrackResidualsDrifttimeBins","Track-Residuen je Ebene in Driftzeit-Bins unterteilt");

		fPTrackResidualsDrifttimeBins->SetDivision(2,3);

		fPTrackResidualsDrifttimeBins->Pic(0,0)->AddObject(fHResidual_X1_drifttimeBin_0_50);
		fPTrackResidualsDrifttimeBins->Pic(0,0)->AddObject(fHResidual_U_drifttimeBin_0_50);
		fPTrackResidualsDrifttimeBins->Pic(0,0)->AddObject(fHResidual_X2_drifttimeBin_0_50);

		fPTrackResidualsDrifttimeBins->Pic(1,0)->AddObject(fHResidual_X1_drifttimeBin_50_100);
		fPTrackResidualsDrifttimeBins->Pic(1,0)->AddObject(fHResidual_U_drifttimeBin_50_100);
		fPTrackResidualsDrifttimeBins->Pic(1,0)->AddObject(fHResidual_X2_drifttimeBin_50_100);

		fPTrackResidualsDrifttimeBins->Pic(0,1)->AddObject(fHResidual_X1_drifttimeBin_100_150);
		fPTrackResidualsDrifttimeBins->Pic(0,1)->AddObject(fHResidual_U_drifttimeBin_100_150);
		fPTrackResidualsDrifttimeBins->Pic(0,1)->AddObject(fHResidual_X2_drifttimeBin_100_150);

		fPTrackResidualsDrifttimeBins->Pic(1,1)->AddObject(fHResidual_X1_drifttimeBin_150_200);
		fPTrackResidualsDrifttimeBins->Pic(1,1)->AddObject(fHResidual_U_drifttimeBin_150_200);
		fPTrackResidualsDrifttimeBins->Pic(1,1)->AddObject(fHResidual_X2_drifttimeBin_150_200);

		fPTrackResidualsDrifttimeBins->Pic(0,2)->AddObject(fHResidual_X1_drifttimeBin_200_250);
		fPTrackResidualsDrifttimeBins->Pic(0,2)->AddObject(fHResidual_U_drifttimeBin_200_250);
		fPTrackResidualsDrifttimeBins->Pic(0,2)->AddObject(fHResidual_X2_drifttimeBin_200_250);

		fPTrackResidualsDrifttimeBins->Pic(1,2)->AddObject(fHResidual_X1_drifttimeBin_250_300);
		fPTrackResidualsDrifttimeBins->Pic(1,2)->AddObject(fHResidual_U_drifttimeBin_250_300);
		fPTrackResidualsDrifttimeBins->Pic(1,2)->AddObject(fHResidual_X2_drifttimeBin_250_300);


		AddPicture(fPTrackResidualsDrifttimeBins);
	}

	fPTrackResolution = GetPicture("TrackResolution");
	if(!fPTrackResolution)
	{
		fPTrackResolution = new TGo4Picture("TrackResolution","Rekonstruktionsgenauigkeit");
		fPTrackResolution->SetDivision(3,3);

		fPTrackResolution->Pic(0,0)->AddObject(fHZResolution_X1);
		fPTrackResolution->Pic(1,0)->AddObject(fHZResolution_U);
		fPTrackResolution->Pic(2,0)->AddObject(fHZResolution_X2);

		fPTrackResolution->Pic(0,1)->AddObject(fHXResolution_X1);
		fPTrackResolution->Pic(1,1)->AddObject(fHXResolution_X2);

		fPTrackResolution->Pic(0,2)->AddObject(fHUResolution_U);
		fPTrackResolution->Pic(1,2)->AddObject(fHYResolution_U);

		AddPicture(fPTrackResolution);
	}







        //-----------------------------------------------------------
	// 5. Fitter

	//fFitter = (TGo4Fitter*) GetObject("Fitter");

	fFitter = fFitPar->GetFitter();
	/*
	if(!fFitter)
	{
		fFitter = new TGo4Fitter("Fitter", TGo4Fitter::ff_least_squares, kTRUE);
		fFitter->AddH1(__DATANAME__, nullptr, kFALSE, 0., 100.);
		fFitter->AddGauss1(__DATANAME__,"G", 50., 5.);
		AddObject(fFitter);
	}
	*/



}


TQClamTrackProc::~TQClamTrackProc()
{

	/** Conditions printen */
	TGo4Log::Info("[TQClamTrackProc] GEBE ALLE CONDITION-STATISTIKEN AUS:");


	TGo4Log::Info("[TQClamTrackProc] Delete %s", GetName());
}





//***********************************************************
// pruefe klasse von output-eventobjekt
Bool_t TQClamTrackProc::CheckEventClass(TClass* cl)
{
	return cl->InheritsFrom(TQClamTrackEvent::Class());
}








Bool_t TQClamTrackProc::BuildEvent(TGo4EventElement* dest)
{
	auto* evt = dynamic_cast<TQClamTrackEvent*>(dest);
	auto* i_evt = dynamic_cast<TQClamClusterEvent*>(GetInputEvent());

	if(!evt || !i_evt)
	{
		return kFALSE;
	}



	/// HEADER
	evt->header = i_evt->header;


	/// SCALER uebergeben
	evt->scaler = i_evt->scaler;
	/// Trigger-Detektor Zeiten uebergeben
	evt->trigDet = i_evt->trigDet;


	double eventTime = evt->header.mbs_time_sec + evt->header.mbs_time_subsec * 1e-9;

        if(fFirstTime)
        {
                fStartTime = eventTime;
                fFirstTime = false;
        }

        fGraphTime = (eventTime - fStartTime) / 60.; // in Min



	evt->fEvtCounter = i_evt->fEvtCounter;
	fEvtCounter = i_evt->fEvtCounter;

	std::vector<const TQClamCluster*> cluster_ptrs;
	cluster_ptrs.reserve(i_evt->clusters.size());

	for(auto& c : i_evt->clusters)
	{
		if(fTrackConditionPar->AllowedLayersForTrack[c.layerID])
		{
			cluster_ptrs.push_back(&c);
		}
	}

	// -------------------------
	// Daten aus ClusterStep uebernehmen
	// -------------------------
	auto layers = ExtractLayerRawData(cluster_ptrs);
	auto states = PrepareTrackingStates(layers);

	// -------------------------
	// Tracking-Modus bestimmen
	// -------------------------
	auto mode = DetermineTrackingMode(i_evt->layerHasCluster);

	if(mode == ETrackingMode::None)
	{
		evt->SetValid(kFALSE);
		return kFALSE;
	}

	// -------------------------
	// Tracking
	// -------------------------
	bool ok = ProcessTrack(mode,evt,layers,states);

	evt->fConvergedTrackCounter = fConvergedTrackCounter;
	evt->fNotConvergedTrackCounter = fNotConvergedTrackCounter;

	if(!ok)
	{
		evt->SetValid(kFALSE);
		return kFALSE;
	}


	// -----------------------------------------
	// Track Conditions prüfen
	// -----------------------------------------
	if(!PassTrackConditions(evt))
	{
		evt->SetValid(kFALSE);
		return kFALSE;
	}



	// -------------------------
	// Nachbereitung, Output & Histogramme fuellen
	// -------------------------
	DoProjection(evt);
	CalibrateSpectrum(evt);
		
	if(fHistoPar->fBFillHistos) { FillHistograms(evt); }



	evt->SetValid(kTRUE);
	return kTRUE;
}



ETrackingMode TQClamTrackProc::DetermineTrackingMode(const std::bitset<3>& hasCluster) const
{
	bool x1 = hasCluster[0];
	bool u = hasCluster[1];
	bool x2 = hasCluster[2];

	if(x1 && u && x2) { return ETrackingMode::X1_U_X2; }
	if(x1 && x2) { return ETrackingMode::X1_X2; }
	if(x1) { return ETrackingMode::X1Only; }
	if(x2) { return ETrackingMode::X2Only; }

	return ETrackingMode::None;
}



Bool_t TQClamTrackProc::ProcessTrack(
		ETrackingMode mode,
		TQClamTrackEvent* evt,
		const std::vector<LayerRawData>& layers,
		std::vector<LayerTrackingState>& states)
{
	FitResults R{};

	auto result = EstimateStartAngles(layers);

	auto [phi,sigmaPhi] = result.first;
	auto [theta,sigmaTheta] = result.second;

	R.phi = phi;
	R.sigmaPhi = sigmaPhi;

	// nur wenn U existiert
	if(mode == ETrackingMode::X1_U_X2)
	{
		R.theta = theta;
		R.sigmaTheta = sigmaTheta;
	}
	else
	{
		R.theta = NAN;
		R.sigmaTheta = NAN;
	}

	IteratePhiTheta(R,states,mode,evt->outlierRejectionSnapshots);

	// Track valid?
	FillFitQualityHistograms(R); // DIES NICHT VON fBFillHistograms abhaengig machen, da relevante Counter darin gesetzt werden!!!
	if(!R.valid) { return kFALSE; }


	FillEventFromStates(evt,R,states);

	FillRejectedHitHistograms(states);

	FillFinalTrackParameters(evt,R,states,mode);


	return kTRUE;
}






//***********************************************************
// 1. X1- und X2-Cluster liegen vor
//***********************************************************





//***********************************************************
// Extrahiere fuer Tracking benoetigte Rohdaten
std::vector<LayerRawData> TQClamTrackProc::ExtractLayerRawData(const std::vector<const TQClamCluster*>& clusters) const
{
	std::vector<LayerRawData> out;

	for(const auto* c : clusters)
	{
		LayerRawData L;

		const TQClamWirePlane* plane = fDriftcham.GetWirePlane(c->layerID);

		L.layerID = c->layerID;
		L.type = plane->GetType();
		L.zPosLayer = c->zPos;

		L.hasGaps = c->hasGaps;
		L.hasVShape = c->hasVShape;
		L.isAscending = c->ascending;
		L.isDescending = c->descending;

		L.nHits = c->numHits;

		L.normalWireDist = plane->GetNormalWireDist();
		L.wireAngle = plane->GetWireAngleRad();

		L.wireNumbers = c->wireNumbers;
		L.drifttimes = c->drifttimes;
		L.tot = c->tot;
		L.normalCoords = c->normalCoords;

//		TGo4Log::Info("LayerRawData: layerID=%d, %ld hits.", L.layerID, L.nHits);
//		for(size_t i=0; i<L.wireNumbers.size(); i++)
//		{
//			TGo4Log::Info("LayerRawData: i=%ld, Drahtnummer=%d, drifttime=%f, normalCoord=%f.",i,L.wireNumbers[i],L.drifttimes[i],L.normalCoords[i]);
//		}

		/** TODO: stdws */

		/** TODO: xMid, uMid First Guess --> evtl nicht noetig... */

		L.normalCoordIntersecFirstGuess = c->meanNormalCoordinate;




		out.push_back(L);
	}

	return out;
}




//***********************************************************
// Berechne den Startwinkel phi fuer Iteration
std::pair<std::pair<double,double>,std::pair<double,double>> TQClamTrackProc::EstimateStartAngles(const std::vector<LayerRawData>& layers) const
{
	double x1 = NAN;
	double z1 = NAN;
	double x2 = NAN;
	double z2 = NAN;
	double u = NAN;
	double uWireAngle = NAN;

	for(const auto& L : layers)
	{
		if(L.type == WirePlaneType::X)
		{
			if(L.layerID == 0)
			{
				x1 = L.normalCoordIntersecFirstGuess;
				z1 = L.zPosLayer;
			}
	
			else if(L.layerID == 2)
			{
				x2 = L.normalCoordIntersecFirstGuess;
				z2 = L.zPosLayer;
			}
		}

		else if(L.layerID == 1)
		{
			u = L.normalCoordIntersecFirstGuess;
			uWireAngle = L.wireAngle;
		}
	}

	if(std::isnan(x1) || std::isnan(x2)) { return {{NAN,NAN},{NAN,NAN}}; }

	double phi = M_PI - std::atan2((z2-z1),(x2-x1));
//	double phi2 = -std::atan((z2-z1)/(x2-x1));
//	TGo4Log::Info("EstimateStartAngles: x1=%f,x2=%f, z1=%f,z2=%f",x1,x2,z1,z2);
//	TGo4Log::Info("EstimateStartAngles: phi=%f",phi);
//	TGo4Log::Info("EstimateStartAngles: phi2=%f",phi2);
	double sigmaPhi = NAN; /** TODO */

	if(std::isnan(u) || std::isnan(uWireAngle)) { return { {phi,sigmaPhi},{NAN,NAN} }; }

	double theta = std::atan2( std::tan(phi) , std::cos(uWireAngle)); // TODO!!
	double sigmaTheta = NAN; /** TODO */



	return {{phi,sigmaPhi},{theta,sigmaTheta}};
}







//***********************************************************
// bereite schnittpunktsberechnung vor
std::vector<LayerTrackingState> TQClamTrackProc::PrepareTrackingStates(const std::vector<LayerRawData>& layers) const
{
	std::vector<LayerTrackingState> states;
	states.reserve(layers.size());

	for(const auto& L : layers)
	{
		LayerTrackingState S;
		S.raw = &L;
		S.hitIsOutlier.assign(S.raw->nHits, false);
		states.push_back(S);
	}

	return states;
}




//***********************************************************
// Iteration zur phi-Winkel Optimierung
// i) Z-Offsets neu berechnen (neue Winkel) --> ComputeZOffsets()
// ii) Schnittpunkte mit Ebenen berechnen --> ComputeIntersection()
// iii) Winkel neu berechnen --> ComputeAngles()
// iv) Konvergenz pruefen: |phi_i - phi_{i-1}| < Konvergenzkriterium (ggf. andere Konvergenzkriterien)
//void TQClamTrackProc::IteratePhiTheta(FitResults& R, std::vector<LayerTrackingState>& states) const
void TQClamTrackProc::IteratePhiTheta(FitResults& R, std::vector<LayerTrackingState>& states, ETrackingMode mode, std::vector<OutlierRejectionSnapshot>& outlierRejectionSnapshots) const
{
	/** iteriere bis max_iter erreicht oder |phi_i - phi_{i-1} < fTrackConditionPar->EpsilonPhi
	 * --> ergebnise valid, wenn epsilon-schranke unterschritten */

	//int max_iter = 100;

	int iter_not_converged = 0;

//	TGo4Log::Info("\n\n\nStart new iteration");
	for(int iter=0; iter<fTrackConditionPar->fMaxIter; iter++)
	{

		double phi_iter = R.phi;
		double theta_iter = R.theta;

		/** 1. zOffsets neu berechnen */
		for(auto& S : states)
		{
			IterationSnapshot snapshot;

			//-------------------------------------------------
			// zOffsets berechnen
			//-------------------------------------------------

			for(double drifttime : S.raw->drifttimes)
			{
				ZOffsetInput in; // in TQClamDriftToZOffsetParameter definiert
				in.layerID = S.raw->layerID;
				in.drifttime = drifttime;
				//in.phi = R.phi;
				in.phi = phi_iter;

				auto out = fDriftToZOffsetPar->ComputeZOffset(in);

				/*
				std::cout
					<< "ZOFFSET: "
					<< "layer=" << in.layerID
					<< " dt=" << in.drifttime
					<< " phi=" << in.phi
					<< " zOffset=" << out.zOffset
					<< " sigma=" << out.sigma_zOffset
					<< std::endl;
				*/

				snapshot.zOffsets.push_back(out.zOffset);
				snapshot.sigmaZOffsets.push_back(out.sigma_zOffset);
			}

			//-------------------------------------------------
			// Snapshot speichern
			//-------------------------------------------------

			S.iter.history.push_back(snapshot);
		}


		/** schnittpunkte berechnen */
		for(auto& S : states)
		{
			//auto in = MakeInput(S,R);
			auto in = MakeInput(S,phi_iter, theta_iter);

			if(in.nHits == 0)
			{
				// Track invalid setzen und IteratePhiTheta beenden
				// Anschliessend wird dann BuildEvent abgebrochen wg. R.valid = false;
				R.valid = false;
				return;
			}

			auto res = ComputeIntersection(in);

//			TGo4Log::Info(
//    "INTERSECTION: value=%f sigma=%f valid=%d",
//    res.intersection,
//    res.sigmaIntersection,
//   res.valid
//);

			S.result.valid = res.valid;
			S.result.intersection = res.intersection;
			S.result.sigmaIntersection = res.sigmaIntersection;

			// TODO?
			S.result.phi = res.phi;
			S.result.sigmaPhi = res.sigmaPhi;
			S.result.quality = res.quality;

			auto& current = S.iter.history.back();

			double chi2 = 0.0;
			int nRejected = 0;

			/*
			double phi_plane = phi_iter;
			if(S.raw->type == WirePlaneType::U)
			{
				phi_plane = std::abs(res.phi);
			}
			*/
			double phi_plane = std::abs(res.phi);



			/** sammle Outlier-Hit Kandidaten,
			 * verwerfe die n (konfigurierbar) schlechtesten */
			std::vector<OutlierCandidate> outlierCandidates;
			for(size_t i=0; i<S.raw->nHits; i++)
			{
				/** berechne abstand track <-> draht (Spurversatz) zur Residuenberechnung */
				if(S.hitIsOutlier[i]) { continue; }

				/*
				std::cout
					<< "wire=" << S.raw->normalCoords[i]
					<< " intersection=" << res.intersection
					<< " res.phi=" << res.phi
					<< " phi_plane=" << phi_plane
					<< " zOffset=" << current.zOffsets[i]
					<< std::endl;
				*/

				double trackDistance = ComputeSignedDistanceToWire(
						S.raw->normalCoords[i],
						res.intersection,
						phi_plane);
				/// TODO entscheide, was am sinnvollsten...
				// phi_plane bewirkt natuerlich super MeanResiduum (wahres Track-Fit Residuum),
				// aber vielleicht interessiert eher, wie gut der Track-Fit eines Clusters zum Track-Winkel phi aus X1 und X2 Schnittpunkt passt?
				//		phi_iter);
						//R.phi);




				double measuredZOffset = current.zOffsets[i];

				// Vorzeichen aus Drahtposition relativ zum Schnittpunkt
				double sign = (S.raw->normalCoords[i] < S.result.intersection) ? -1.0 : +1.0;

				double signedMeasured = sign * measuredZOffset;

				// echtes Residuum
				double residual = signedMeasured - trackDistance;
				double res_2 = signedMeasured + trackDistance;
				//double residual = measuredZOffset - trackDistance;

				//std::cout << "phi = " << phi_iter*180./M_PI << " phi_ebene = " << phi_plane * 180./M_PI << std::endl;
				//std::cout << "residual = " << residual << " res2 = " << res_2 << " measuredZOffset = " << measuredZOffset << " signedMeasured = " << signedMeasured << " trackDistance = " << trackDistance << std::endl;

				/** speichere Residuum-Daten */
				ResidualData RD;

				RD.hitIndex = i;
				RD.wireCoord = S.raw->normalCoords[i];
				RD.measuredZOffset = measuredZOffset;
				RD.trackDistance = trackDistance;
				RD.residual = residual;
				RD.drifttime = S.raw->drifttimes[i];
				RD.tot = S.raw->tot[i];
				RD.layerID = S.raw->layerID;
				RD.wireNumber = S.raw->wireNumbers[i];

				double sigmaMeasured = current.sigmaZOffsets[i];
				RD.sigma_measuredZOffset = sigmaMeasured;

				// Berechnung sigma-track
				double dx = res.intersection - S.raw->normalCoords[i];

				double sigmaX = res.sigmaIntersection;
				double sigmaPhi = R.sigmaPhi;

				if(!std::isfinite(sigmaX)) { sigmaX = 0.0; }
				if(!std::isfinite(sigmaPhi)) { sigmaPhi = 0.0; }

				//double term1 = std::cos(R.phi) * sigmaX;
				double term1 = std::cos(phi_iter) * sigmaX;
				//double term2 = dx * std::sin(R.phi) * sigmaPhi;
				double term2 = dx * std::sin(phi_iter) * sigmaPhi;

				double sigmaTrack = std::sqrt(term1*term1 + term2*term2);
				RD.sigma_trackDistance = sigmaTrack;

				RD.sigmaResidual = std::sqrt(sigmaMeasured*sigmaMeasured + sigmaTrack*sigmaTrack);



				// -------------------
				// chi2 + outlier
				// -------------------

				constexpr double eps = 1e-12;
				if(RD.sigmaResidual > eps) /** quasi-0 */
				{
					double pull = residual*residual; // / RD.sigmaResidual;
					RD.pull = pull;
					fHPull->Fill(pull);
					if(fTrackConditionPar->UseOutlierRejection)
					{
						bool isInside = true;
						if(fCondPull)
						{
							isInside = fCondPull->Test(pull);
						}
						// erst auf false setzen, da spaeter nur die groessten ausgewaehlt werden
						RD.isOutlier = false; //  !isInside;
						if(!isInside)
						{
							/*
							// optional zaehlen ...
							nRejected++;
							// hit als outlier kennzeichnen
							S.hitIsOutlier[i] = true;
							*/

							size_t residualIndex = current.residuals.size();
							outlierCandidates.push_back({i,residualIndex,pull});
						}
					}

					if(!RD.isOutlier)
					{
						double sigma2 = RD.sigmaResidual * RD.sigmaResidual;
						chi2 += residual * residual / sigma2;
					}
				}

				// ResidualData (RD) gefuellt
				current.residuals.push_back(RD);



			}



			// nun: verwerfe Outlier
			std::sort(outlierCandidates.begin(),outlierCandidates.end(),
					[](const OutlierCandidate& a, const OutlierCandidate& b)
					{ return a.pull > b.pull; });

			int nToReject = 0;
			switch(S.raw->layerID)
			{
				case 0: // X1
					nToReject = fTrackConditionPar->fMaxRejectedHitsX1PerIter;
					break;
				case 1: // U
					nToReject = fTrackConditionPar->fMaxRejectedHitsUPerIter;
					break;
				case 2: // X2
					nToReject = fTrackConditionPar->fMaxRejectedHitsX2PerIter;
					break;
			}
	
			nToReject = std::min(nToReject,(int)outlierCandidates.size());
			/*
			for(int k=0; k<nToReject; k++)
			{
				auto& cand = outlierCandidates[k];
				//size_t idx = outlierCandidates[k].hitIndex;
				S.hitIsOutlier[cand.hitIndex] = true;

				current.residuals[cand.residualIndex].isOutlier = true;
	
				nRejected++;
			}
			*/




			for(int k = 0; k < nToReject; k++)
			{
				auto& cand = outlierCandidates[k];

				// Hit als Outlier markieren
				S.hitIsOutlier[cand.hitIndex] = true;

				current.residuals[cand.residualIndex].isOutlier = true;

				nRejected++;


				// =========================================================
				// Outlier-Rejection dokumentieren
				// =========================================================

				const auto& RD = current.residuals[cand.residualIndex];

				OutlierRejectionSnapshot snapshot;

				snapshot.iteration = iter;
				snapshot.layerID = S.raw->layerID;

				snapshot.phi_global = phi_iter;
				snapshot.theta_global = theta_iter;
				snapshot.phi_plane = phi_plane;

				snapshot.intersection = res.intersection;


				OutlierHitEntry hit;

				hit.layerID = RD.layerID;
				hit.wireNumber = RD.wireNumber;
				hit.hitIndex = RD.hitIndex;

				hit.drifttime = RD.drifttime;
				hit.tot = RD.tot;

				hit.normalCoord = RD.wireCoord;

				hit.measuredZOffset = RD.measuredZOffset;
				hit.sigmaMeasuredZOffset = RD.sigma_measuredZOffset;

				hit.trackDistance = RD.trackDistance;

				hit.residual = RD.residual;
				hit.pull = RD.pull;

				hit.phi_global = phi_iter;
				hit.phi_plane = phi_plane;
				hit.theta_global = theta_iter;

				hit.iteration = iter;

				snapshot.hits.push_back(hit);

				outlierRejectionSnapshots.push_back(snapshot);
			}







			// chi2 ueberarbeiten, hier muss summe neu berechnet werden

			current.chi2 = chi2; // TODO global aggregieren
			current.nRejected = nRejected;

			int nUsed = current.residuals.size() - current.nRejected;
			if(nUsed > 2)
			{
				current.chi2ndf = chi2 / (nUsed - 2);
			}

		}




		// ===============================
		// FINAL RESIDUAL STATISTICS
		// ===============================

		double maxResSigned = -1e30;
		double maxResAbs = 0.0;
		double sumRes = 0.0;
		size_t nRes = 0;
		double sumRes2 = 0.; // optimiergroesse: RMS der Residuen

		for(const auto& S : states)
		{
			if(S.iter.history.empty()) continue;

			const auto& lastIter = S.iter.history.back();

			for(const auto& RD : lastIter.residuals) // iteriere ueber Hits
			{
				if(RD.isOutlier) { continue; } // Hit nicht nutzen, wenn Outlier
				double r = RD.residual;

				sumRes += r;
				sumRes2 += r*r;
				nRes++;

				if(r > maxResSigned) { maxResSigned = r; }

				if(std::abs(r) > maxResAbs) { maxResAbs = std::abs(r); }
			}
		}


		if(nRes > 0)
		{
			R.quality.meanResidual = sumRes / nRes;
			R.quality.maxResidualSigned = maxResSigned;
			R.quality.maxResidualAbs = maxResAbs;
			R.quality.rmsResidual = std::sqrt(sumRes2/nRes);
		}
		else
		{
			R.quality.meanResidual = NAN;
			R.quality.maxResidualSigned = NAN;
			R.quality.maxResidualAbs = NAN;
			R.quality.rmsResidual = NAN;
		}

		// Debug
//		TGo4Log::Info("FINAL: mean=%f maxSigned=%f maxAbs=%f nRes=%zu",
//				R.quality.meanResidual,
//				R.quality.maxResidualSigned,
//				R.quality.maxResidualAbs,
//				nRes);



		/** 4. neue winkel */
		auto result = ComputeAngles(states,mode);
		auto [phi_new,sigmaPhi_new] = result.first;
		auto [theta_new,sigmaTheta_new] = result.second;

		/** Konvergenz */
		//double totalChi2_prev = R.quality.chi2; // gesetzt???
		//double dChi2 = std::abs(

		double phi_old = R.phi;
		double dPhi = std::abs(phi_new - phi_old);

		double theta_old = R.theta;


		double rms = R.quality.rmsResidual;

		// ============================
		// IterationSummary fuellen
		// ============================
		IterationSummary summary;

		summary.iteration = iter;

		summary.phi = phi_new;
		summary.theta = theta_new;

		summary.dPhi = dPhi;
		summary.rmsResidual = rms;

		summary.meanResidual = R.quality.meanResidual;
		summary.maxResidualAbs = R.quality.maxResidualAbs;

		R.iterationSummaries.push_back(summary);


		bool converged = false;
		if(iter < fTrackConditionPar->fMinIter) { converged = false; }
		else
		{
			switch(fTrackConditionPar->ConvergenceCriterion)
			{
				case EConvergenceCriterion::Phi:
					converged = (dPhi < fTrackConditionPar->EpsilonPhi);
					break;
				case EConvergenceCriterion::RMS:
					converged = (rms < fTrackConditionPar->EpsilonRMS);
					break;
				case EConvergenceCriterion::PhiAndRMS:
					converged = (dPhi < fTrackConditionPar->EpsilonPhi) && (rms < fTrackConditionPar->EpsilonRMS);
					break;
				case EConvergenceCriterion::PhiOrRMS:
					converged = (dPhi < fTrackConditionPar->EpsilonPhi) || (rms < fTrackConditionPar->EpsilonRMS);
					break;
			}
		}


		/**
		 * falls noch nicht konvergiert, aber dPhi stagniert
		 * --> verwerfe Track
		 */
		bool stagnated = std::isfinite(dPhi) && dPhi < fTrackConditionPar->EpsilonPhiStagnation;

		if(stagnated && !converged)
		{
			R.valid = false;
			R.quality.converged = false;
			R.quality.nIterations = iter + 1;

			return;
		}


		//if(fabs(phi_new - phi_old) < fTrackConditionPar->EpsilonPhi) // TODO: Auch && Theta pruefen????, && chi2???? mit &&
		//if(rms < fTrackConditionPar->EpsilonRMS)
		if(converged)
		{
			//-------------------------------------------------
			// globale Fit-Qualität berechnen
			//-------------------------------------------------

			double totalChi2 = 0.0;
			double maxResidualSigned = 0.0;
			double maxResidualAbs = 0.0;
			size_t nResiduals = 0;
			double residualSum2 = 0.0;
			int totalRejected = 0;

			double sum_layer[3] = {0.,0.,0.};
			double sum2_layer[3] = {0.,0.,0.};
			int count_layer[3] = {0,0,0};
	
			for(const auto& S : states)
			{
				const auto& current = S.iter.history.back();

				totalChi2 += current.chi2;
				/**
				 * Alle Hits zaehlen, die waehrend des gesamten
				 * Iterationsprozesses als Outlier verworfen wurden.
				 */
				for(const auto& isOutlier : S.hitIsOutlier)
				{
					if(isOutlier) { totalRejected++; }
				}
				//totalRejected += current.nRejected;


				for(const auto& RD : current.residuals)
				{
					double r = RD.residual;
					double absRes = std::abs(r);

					//residualSum += absRes;
					residualSum2 += r*r;

					if(absRes > maxResidualAbs)
					{
						maxResidualAbs = absRes;
						maxResidualSigned = r;
					}

					int L = S.raw->layerID;

					sum_layer[L] += r;
					sum2_layer[L] += r*r;
					count_layer[L]++;

					nResiduals++;
				}
			}


			//-------------------------------------------------
			// FitQuality füllen
			//-------------------------------------------------

			double mean_layer[3] = {0.,0.,0.};
			for(int L=0; L<3; L++)
			{
				if(count_layer[L] > 0)
				{
					mean_layer[L] = sum_layer[L] / count_layer[L];
				}
				else
				{
					mean_layer[L] = NAN;
				}
			}
			
			double var_layer[3] = {0.,0.,0.};
			for(const auto& S : states)
			{
				const auto& current = S.iter.history.back();

				for(const auto& RD : current.residuals)
				{
					double r = RD.residual;

					int L = S.raw->layerID;

					if(count_layer[L] > 0)
					{
						double d = r - mean_layer[L];
						var_layer[L] += d*d;
					}
				}
			}


			R.quality.chi2 = totalChi2;
			R.quality.totalRejected = totalRejected;

			for(int L=0; L<3; L++)
			{
				if(count_layer[L] > 0)
				{
					R.quality.meanResidual_layer[L] = mean_layer[L]; //sum_layer[L] / count_layer[L];
					R.quality.sigmaResidual_layer[L] = sqrt(var_layer[L] / count_layer[L]);
					R.quality.rmsResidual_layer[L] = std::sqrt(sum2_layer[L] / count_layer[L]);
					R.quality.nResiduals_layer[L] = count_layer[L];
				}
				else
				{
					R.quality.meanResidual_layer[L] = NAN;
					R.quality.rmsResidual_layer[L] = NAN;
					R.quality.nResiduals_layer[L] = 0;
				}
			}


			int nFitParams = 2; /** Konfigurieren? TODO */
			if(nResiduals > nFitParams)
			{
				R.quality.chi2ndf = totalChi2 / (nResiduals - nFitParams);
			}
			else
			{
				R.quality.chi2ndf = NAN;
			}


			if(nResiduals > 0)
			{
				//R.quality.meanResidual = residualSum / nResiduals;
//				R.quality.rmsResidual = std::sqrt(residualSum2 / nResiduals);
				R.quality.maxResidualAbs = maxResidualAbs;
				R.quality.maxResidualSigned = maxResidualSigned;
			}
			else
			{
				R.quality.meanResidual = NAN;
				R.quality.rmsResidual = NAN;
				R.quality.maxResidualAbs = NAN;
				R.quality.maxResidualSigned = NAN;
			}

			R.quality.nIterations = iter + 1;
			R.quality.converged = true;

			//-------------------------------------------------
			// finale Trackparameter setzen
			//-------------------------------------------------

			R.valid = true;
			R.phi = phi_new; /** endgueltiges phi setzen*/

			if(mode == ETrackingMode::X1_U_X2 && std::isfinite(theta_new))
			{
				R.theta = theta_new; /** endguelitges theta setzen */
			}
			else
			{
				R.theta = NAN;
			}


			R.iterations = iter + 1;
//			TGo4Log::Info("converged, iter=%d",iter+1);
			return;
		}

		/** Relaxation fuer bessere Konvergenz */
		R.phi = fTrackConditionPar->RelaxationAlpha * phi_new + (1-fTrackConditionPar->RelaxationAlpha) * phi_old;

		if(mode == ETrackingMode::X1_U_X2 && std::isfinite(theta_new))
		{
			R.theta = fTrackConditionPar->RelaxationAlpha * theta_new + (1-fTrackConditionPar->RelaxationAlpha) * theta_old;
		}

		iter_not_converged++;
	}

//	TGo4Log::Info("not converged!");


	R.valid = false;
	R.quality.converged = false;
	R.quality.nIterations = iter_not_converged+1; //max_iter;
	/*
	R.quality.chi2 = NAN;
	R.quality.chi2ndf = NAN;
	R.quality.maxResidualAbs = NAN;
	R.quality.maxResidualSigned = NAN;
	R.quality.meanResidual = NAN;
	*/
}





//***********************************************************
// Berechne signierten senkrechten Abstand
// zwischen Track und Draht
//
// @param:
// - wireCoord: Normalkoordinate des Drahts
// - trackIntersection: rekonstruierter Ebenenschnittpunkt
// - phi: Trackwinkel in x-z bzw u-z Ebene
//
// @return:
// - signierter Driftabstand
double TQClamTrackProc::ComputeSignedDistanceToWire(double wireCoord,double trackIntersection,double phi) const
{
	//---------------------------------------------------------
	// Abstand entlang Ebenennormalen
	//---------------------------------------------------------
//TGo4Log::Info(
//    "DIST FUNC: wire=%f intersec=%f phi=%f",
//    wireCoord,
//    trackIntersection,
//   phi
//);
	double dx = wireCoord - trackIntersection;

	//---------------------------------------------------------
	// Abstand von Draht zum Track in z-Richtung (orhtogonal auf der Drahtebene)
	//
	// --> Spurversatz (fitted zOffset)
	//---------------------------------------------------------

	if(std::abs(std::cos(phi)) < 1e-6) { return NAN; } // vorsicht: phi = 90 °
	double driftDistance = dx * std::tan(phi);

	//---------------------------------------------------------
	// signed distance
	//---------------------------------------------------------

	return driftDistance;
}






std::pair<std::pair<double,double>,std::pair<double,double>> TQClamTrackProc::ComputeAngles(
		const std::vector<LayerTrackingState>& states,
		ETrackingMode mode) const
{
	const LayerTrackingState* s_x1 = nullptr;
	const LayerTrackingState* s_u  = nullptr;
	const LayerTrackingState* s_x2 = nullptr;

	for(const auto& S : states)
	{
		if(S.raw->layerID == 0) { s_x1 = &S; }
		else if(S.raw->layerID == 1) { s_u  = &S; }
		else if(S.raw->layerID == 2) { s_x2 = &S; }
	}

	//--------------------------------------------------
	// X1 und X2 müssen immer vorhanden sein
	//--------------------------------------------------

	if(!s_x1 || !s_x2)
	{
		return {{NAN,NAN},{NAN,NAN}};
	}

	double x1D = s_x1->result.intersection;
	double x2D = s_x2->result.intersection;

	double z1 = s_x1->raw->zPosLayer;
	double z2 = s_x2->raw->zPosLayer;

	double dz = z2 - z1;
	double dx = x2D - x1D;

	// ==========================================
	// 1. phi aus X-Ebenen
	// --> nutze Schnittpunkte mit X-Ebenen und z-Abstand der X-Ebenen
	double phi = M_PI - std::atan2(dz,dx);

	double sigmaPhi = NAN;

	//--------------------------------------------------
	// X1+X2 Modus
	//--------------------------------------------------

	if(mode == ETrackingMode::X1_X2)
	{
		return {{phi,sigmaPhi},{NAN,NAN}};
	}

	//--------------------------------------------------
	// U-Ebene erforderlich
	//--------------------------------------------------

	if(!s_u)
	{
		return {{phi,sigmaPhi},{NAN,NAN}};
	}

	double gamma = s_u->raw->wireAngle;

	// ==========================================
	// 2. phi_u aus U-Ebene holen
	double phi_u = std::abs(s_u->result.phi);

	// Korrigiere phi_u.
	// Problem: Es gibt immer einen generellen Offset zwischen den Cluster-Winkeln
	// und dem rekonstruierten Trackwinkel
	// -> versuche diesen Offset zu naehern, indem ich den Offset aus den X-Ebenen mittele:
	double phi_x1 = std::abs(s_x1->result.phi);
	double phi_x2 = std::abs(s_x2->result.phi);

	double phi_offset = phi - (phi_x1 + phi_x2)/2;
	phi_u = phi_u + phi_offset;

	if(!std::isfinite(phi_u))
	{
		return {{phi,sigmaPhi},{NAN,NAN}};
	}

	double tan_phi  = std::tan(phi);
	double tan_phiu = std::tan(phi_u);

	if(std::abs(tan_phi)  < 1e-6 || std::abs(tan_phiu) < 1e-6 || std::abs(std::sin(gamma)) < 1e-6)
	{
		return {{phi,sigmaPhi},{NAN,NAN}};
	}


	// ==========================================
	// 3. theta berechnen aus Steigungen
	// u = cos(gamma) x - sin(gamma) y
	// du/dz = cos(gamma) dx/dz - sin(gamma) dy/dz
	// 1/tan(phi_u) = cos(gamma) 1/tan(phi) - sin(gamma) tan(theta)
	//
	// -> tan(theta) = [ cos(gamma)/tan(phi) - 1/tan(phi_u) ] / sin(gamma)
	//double inv_tan_theta = (std::cos(gamma)/tan_phi - 1.0/tan_phiu) / std::sin(gamma);
	double tan_theta = (std::cos(gamma)/tan_phi - 1.0/tan_phiu) / std::sin(gamma);

	//double theta = std::atan(inv_tan_theta);
	double theta = std::atan(tan_theta);

	/*
	std::cout << "Gamma = " << gamma
		<< " phi_offset = " << phi_offset
		<< " tan_phi_u = " << tan_phiu
		<< " phi_u = " << phi_u
		<< " tan_phi = " << tan_phi 
		<< " phi = " << phi 
		<< " tan_theta = " << tan_theta
		<< " theta = " << theta
		<< " theta_deg = " << theta*180./M_PI << std::endl;
	*/



	double sigmaTheta = NAN; // TODO

	return {{phi,sigmaPhi},{theta,sigmaTheta}};
}







//***********************************************************
// hole U-Ebenen LayerTrackingState
const LayerTrackingState& TQClamTrackProc::GetULayer(const std::vector<LayerTrackingState>& states) const
{
	for(const auto& S : states)
	{
		if(S.raw->type == WirePlaneType::U) { return S; }
	}

	/** ... TODO ...*/
	return states[0];
}













//-------------------------------------------------------------//
//              Berechnung von Schnittpunkt mit Ebene
//-------------------------------------------------------------//




//***********************************************************
// Bilde Input fuer naechse Iteration in Schnittpunktsberechnung
IntersectionInput TQClamTrackProc::MakeInput(const LayerTrackingState& S, double phi, double theta) const//const FitResults& R) const
{
	IntersectionInput in;


	// --------------------------------
	// meta daten
	// --------------------------------
	in.layerID = S.raw->layerID;
	in.type = S.raw->type;

	in.hasVShape = S.raw->hasVShape;
	in.hasGaps = S.raw->hasGaps;
	in.isAscending = S.raw->isAscending;
	in.isDescending = S.raw->isDescending;
	
	in.normalWireDist = S.raw->normalWireDist;

	// --------------------------------
	// aktuelle iterationsdaten holen
	// --------------------------------
	const auto& current = S.iter.history.back();

	in.previousIntersection = S.result.intersection;
	in.hasPreviousIntersection = std::isfinite(S.result.intersection);


	// --------------------------------
	// hit-container clearen
	// --------------------------------
	in.normalCoords.clear();
	in.drifttimes.clear();
	in.zOffsets.clear();

	in.sigmaNormalCoords.clear();
	in.sigmaDrifttimes.clear();
	in.sigmaZOffsets.clear();

	// --------------------------------
	// outlier-hits filtern
	// --------------------------------
	for(size_t i=0; i<S.raw->nHits; i++)
	{
		if(S.hitIsOutlier[i]) { continue; } // outliert nicht fuer trackfitting nutzen

		// -- rohdaten uebernehmen --
		in.normalCoords.push_back(S.raw->normalCoords[i]);
		in.drifttimes.push_back(S.raw->drifttimes[i]);

		// -- berechnete groessen --
		in.zOffsets.push_back(current.zOffsets[i]);

		// -- unsicherheiten --
		if(i < S.raw->sigmaNormalCoords.size())
		{
			in.sigmaNormalCoords.push_back(S.raw->sigmaNormalCoords[i]);
		}
		else
		{
			in.sigmaNormalCoords.push_back(0.);
		}

		if(i < S.raw->sigmaDrifttimes.size())
		{
			in.sigmaDrifttimes.push_back(S.raw->sigmaDrifttimes[i]);
		}
		else
		{
			in.sigmaDrifttimes.push_back(0.);
		}

		if(i< current.sigmaZOffsets.size())
		{
			in.sigmaZOffsets.push_back(current.sigmaZOffsets[i]);
		}
		else
		{
			in.sigmaZOffsets.push_back(0.);
		}
	}

	// -- anzahl gueltiger hits setzen --
	in.nHits = in.normalCoords.size();

	// -- winkel uebernehmen --
	//in.phi = R.phi;
	in.phi = phi;
	//in.theta = R.theta;
	in.theta = theta;

	/** TODO */

	in.sigmaPhi = 0.;//R.sigmaPhi;
	in.sigmaTheta = 0.;//R.sigmaTheta;

	if(in.nHits < fClusterPar->minWireSpanInCluster[in.layerID])
	{
		//TGo4Log::Warn("[TQClamTrackProc] Es stehen nur noch weniger als 2 Hits fuer das Track-Fitting zur Verfuegung.");
		in.nHits = 0; // --> nicht weiter processen
		return in;
	}

	return in;
}




IntersectionResult TQClamTrackProc::ComputeIntersection(const IntersectionInput& in) const
{
//	std::cout << "ComputeIntersection phi IN = " << in.phi << std::endl;
	/** weniger als 2 Hits in Cluster --> TODO */
	if(in.nHits < 2)
	{
		TGo4Log::Error("[TQClamTrackProc] Versuche Schnittpunkt eines Clusters mit < 2 Hits zu bestimmen. FEHLER!");
		TGo4RuntimeException();
	}

	/** Cluster ohne Drahtluecken */
	if(!in.hasGaps)
	{
		if(in.nHits == 2)
		{
			return ComputeRegression(in);
			//return ComputeCross2(in);
		}

		else if(in.nHits == 3)
		{
			return ComputeRegression(in);
			//return ComputeMIT(in);
		}

		/** 3,4 Hits --> TODO */


		/** 4 Hits oder mehr benachbarte Hits TODO  */



		/** sonst: lineare Regression */
		return ComputeRegression(in);
	}

	/** falls luecken --> falle auf lin regression zurueck */
	return ComputeRegression(in);



	/* TODO: evtl hits rausschmeissen aus cluster etc... */
}





//-------------------------------------------------------------//
//              Algorithmen zur Schnittpunktsberechnung
//-------------------------------------------------------------//

/** TODO!!! RESIDUEN BESTIMMUNG !!! */


//***********************************************************
// Schnittpunkts-Berechnung: 2 x benachbarte Hits in Cluster
IntersectionResult TQClamTrackProc::ComputeCross2(const IntersectionInput& in) const
{
	IntersectionResult out;

	/** rechts-links ambiguitaet cluster ... */
	double slope_minus = (in.zOffsets[0] - in.zOffsets[1]) / in.normalWireDist;
	double slope_plus = (in.zOffsets[0] + in.zOffsets[1]) / in.normalWireDist;

	double phi_minus = std::atan(slope_minus);
	double phi_plus = std::atan(slope_plus);

	/** 1. beide Hits auf verschiedenen Seiten der Drahtebene */
	if(std::abs(slope_plus - std::tan(in.phi)) < std::abs(slope_minus - std::tan(in.phi)))
	{
		double x_plusD = in.normalCoords[0] + in.normalWireDist * in.zOffsets[0]/(in.zOffsets[0] + in.zOffsets[1]);
		
		out.intersection = x_plusD;
		out.phi = phi_plus;

		/** TODO */
		out.sigmaPhi = NAN;
		out.sigmaIntersection = NAN;
		out.quality = NAN;

		out.valid = true;

		return out;
	}

	/** 2. beide Hits auf derselben Drahtebenen-Seite */
	double x_minusD = in.normalCoords[0] + in.normalWireDist * in.zOffsets[0]/(in.zOffsets[0] - in.zOffsets[1]);

	out.intersection = x_minusD;
	out.phi = phi_minus;

	/** TODO */
	out.sigmaPhi = NAN;
	out.sigmaIntersection = NAN;
	out.quality = NAN;

	out.valid = true;

	return out;
}




//***********************************************************
// Schnittpunkts-Berechnung: 3 x benachbarte Hits in Cluster
IntersectionResult TQClamTrackProc::ComputeMIT(const IntersectionInput& in) const
{
	IntersectionResult out;

	/** 1. Hits verteilt auf 2 Seiten mit VShape => MIT-Algorithmus */
	if(in.hasVShape)
	{
                // x_d = x_i - DeltaX/2 * (z_{i+1} - z_{i-1})/(z_g - z_i)
                // z_i --> zOffset mit min Driftzeit
                // z_g --> max(z_{i-1}, z_{i+1})

                //int i = std::min_element(in.drifttimes.begin(), in.drifttimes.end()) - in.drifttimes.begin();
                int i = 1; // 3 Hits, V-Shape --> mittlere Driftzeit

                double maxZOffset = *std::max_element(in.zOffsets.begin(), in.zOffsets.end());
                double x_D = in.normalCoords[i] - in.normalWireDist/2 * (in.zOffsets[i+1] - in.zOffsets[i-1])/(maxZOffset - in.zOffsets[i]);

                // lineare Regression ueber alle MIT-Punkte und den berechneten Ebenenschnittpunkt
                // zur Berechnung der Steigung m
                // m = (N * [zx] - [z]*[x])/(N * [zz] - [z]^2), mit N = Anzahl Messwerte, [a] = sum_1^N(a_i)
                double S_xz = 0, S_z = 0, S_x = 0, S_zz = 0;
                for(int i=0; i<in.nHits; i++)
                {
                        S_z += in.zOffsets[i];
                        S_x += in.normalCoords[i];
                        S_zz += in.zOffsets[i]*in.zOffsets[i];
                        S_xz += in.normalCoords[i]*in.zOffsets[i];
                }

                double m = (in.nHits * S_xz - S_z * S_x)/(in.nHits*S_zz - S_z*S_z);
                double phi = std::atan(m);
//		TGo4Log::Info("MIT: m=%f, phi=%f",m,phi);

                out.intersection = x_D;

                // TODO --> gaussche fortpflanzung der S_i...
                out.sigmaIntersection = NAN;
                out.sigmaPhi = NAN;
                out.quality = NAN;

                out.valid = true;

//              std::cout << "Ergebnis MIT x_D=" << std::to_string(x_D) << std::endl;

                return out;
        }


        // 2. zwei weitere moeglichkeiten:
        // i) alle Hits auf einer Ebenenseite --> steigende/fallende Driftzeiten
        // ii) zwei Hits auf der einen Seite, letzte auf der anderen, aber trotzdem die kleinste.
        // => diese Faelle kann ich nicht unterscheiden => beide lin. Regression
        return ComputeRegression(in);
}





//***********************************************************
// Schnittpunkts-Berechnung: mind. 4 Driftzeiten mit symmetrischen Vorzeichenwechsel --> VShape TODO
IntersectionResult TQClamTrackProc::ComputeCross4(const IntersectionInput& in) const
{
        IntersectionResult out;

        return ComputeRegression(in);


        //return out;
}




//***********************************************************
// Falls nicht anders moeglich: Mache lineare Regression (Methode kleinster Quadrate)
// --> eher vermeiden, da am aufwendigsten

IntersectionResult TQClamTrackProc::ComputeRegression(const IntersectionInput& in) const
{
	IntersectionResult out;

	if(in.nHits < 2)
	{
		out.valid = false;
		return out;
	}

	double reference;
	if(in.hasPreviousIntersection)
	{
		reference = in.previousIntersection;
	}
	else
	{
		// 1) Grobe Näherung für x_D: Schwerpunkt der Drahtkoordinaten
		double x_mean = 0.0;
		for(int i = 0; i < in.nHits; i++)
		{
			x_mean += in.normalCoords[i];
		}
		x_mean /= in.nHits;

		reference = x_mean;
	}

	// 2) Lineare Regression in (x, z) mit einfacher Vorzeichen-Zuordnung
	double S_x  = 0.0;
	double S_z  = 0.0;
	double S_xx = 0.0;
	double S_xz = 0.0;
	int N = 0;

	for(int i = 0; i < in.nHits; i++)
	{
		double x  = in.normalCoords[i];
		double z0 = in.zOffsets[i];   // immer positiv (Driftweg)

		// einfache Seiten-Zuordnung relativ zum Schwerpunkt
		double sign = (x >= reference) ? +1.0 : -1.0;
		double z = sign * z0;

		S_x  += x;
		S_z  += z;
		S_xx += x * x;
		S_xz += x * z;
		N++;
	}

	double denom = N * S_xx - S_x * S_x;
	if(std::abs(denom) < 1e-12)
	{
		out.valid = false;
		return out;
	}

	double m = (N * S_xz - S_x * S_z) / denom;  // dz/dx
	double b = (S_z - m * S_x) / N;

	if(std::abs(m) < 1e-12)
	{
		out.valid = false;
		return out;
	}

	double x_D = -b / m;
	//double phi_layer = std::atan(m);   // lokaler Winkel in dieser Ebene
	double phi_layer = -std::atan(m);   // lokaler Winkel in dieser Ebene TODO

	out.intersection      = x_D;
	out.phi               = phi_layer;
	out.sigmaPhi          = NAN;
	out.sigmaIntersection = NAN;
	out.valid             = true;

	return out;
}





//***********************************************************
// 2. Nur X1-Cluster liegt vor
//***********************************************************










//***********************************************************
// 3. Nur X2-Cluster liegt vor
//***********************************************************




//***********************************************************
// 1. X1- und X2-Cluster liegen vor
//***********************************************************



//***********************************************************
// Berechnung Z-Offset
//***********************************************************
/*
ZOffsetResult TQClamTrackProc::ComputeZOffset(const ZOffsetInput& cfg) const
{
	return fDriftToZOffsetPar->ComputeZOffset(cfg);
}
*/


//***********************************************************
// pruefe Track-Parameter: x_X1, phi
Bool_t TQClamTrackProc::PassTrackConditions(const TQClamTrackEvent* evt) const
{
	if(!evt)
	{
		return kFALSE;
	}

	// -----------------------------------------
	// X1 position
	// -----------------------------------------

	if(evt->xD_X1 < fTrackConditionPar->min_x_X1 || evt->xD_X1 > fTrackConditionPar->max_x_X1)
	{
		return kFALSE;
	}

	// -----------------------------------------
	// phi
	// -----------------------------------------

	if(evt->phi_deg < fTrackConditionPar->min_phi_deg || evt->phi_deg > fTrackConditionPar->max_phi_deg)
	{
		return kFALSE;
	}

	return kTRUE;
}




//***********************************************************
// Projiziere auf Fokalebene
void TQClamTrackProc::DoProjection(TQClamTrackEvent* evt)
{
	evt->proj_position = fFocalPlanePar->Project(evt->posAtX1, evt->phi, evt->theta); //evt->dirAtX1);


	evt->focalPlaneX = evt->proj_position.X();
	evt->focalPlaneY = evt->proj_position.Y();
	evt->focalPlaneZ = evt->proj_position.Z();



	/*
	TGo4Log::Info("X1: x=%f, z=%f | dir=(%f,%f,%f) | proj_x=%f, proj_z=%f",
	              evt->posAtX1.X(), evt->posAtX1.Z(),
	              evt->dirAtX1.X(), evt->dirAtX1.Y(), evt->dirAtX1.Z(),
	              evt->proj_position.X(), evt->proj_position.Z());
	*/
}





//***********************************************************
// Kalibriere getrackte Daten:
// X-Phi Korrektur und Energiekalibrierung
void TQClamTrackProc::CalibrateSpectrum(TQClamTrackEvent* evt)
{
	double x_raw = evt->xD_X1;
	double sigma_x_raw = 0.; // TODO

	double phi_raw = evt->phi_deg;
	//TGo4Log::Info("phi_raw=%f",phi_raw);
	double sigma_phi_raw = 0.; // TODO
	double y = evt->yD_U_corr;

	// Kalibriere
	double phi_corr;
	double sigma_phi_corr;
	fSpectrumCaliPar->CorrectPhi(x_raw,sigma_x_raw,phi_raw,sigma_phi_raw,phi_corr,sigma_phi_corr);

	double x_corr;
	double sigma_x_corr;
	fSpectrumCaliPar->CorrectX(x_raw,sigma_x_raw,phi_corr,sigma_phi_corr,x_corr,sigma_x_corr);

	// Energiekalibrierung
	double e_MeV = NAN;
	double sigma_e_MeV = NAN;
	fSpectrumCaliPar->Calibrate(x_corr,sigma_x_corr,e_MeV,sigma_e_MeV);

	double phi_scatt = NAN;
	double theta_scatt = NAN;
	fSieveSlitCalibParam->TransformToScattCoords(
			x_raw,
			y,
			phi_raw,
			phi_scatt,
			theta_scatt);

	double x_scatt = NAN;
	double y_scatt = NAN;
	fSieveSlitCalibParam->TransformToTargetCoords(
			x_raw,
			y,
			phi_raw,
			x_scatt,
			y_scatt);


	// ergebnisse an evt uebergeben
	evt->phi_corr = phi_corr;
	evt->sigma_phi_corr = sigma_phi_corr;
	evt->x_corr = x_corr;
	evt->sigma_x_corr = sigma_x_corr;
	evt->e_MeV = e_MeV;
	evt->sigma_e_MeV = sigma_e_MeV;

	evt->scatteringPhi = phi_scatt;
	evt->scatteringTheta = theta_scatt;

	evt->scatteringX = x_scatt;
	evt->scatteringY = y_scatt;

	//TGo4Log::Info("phi_corr=%f",phi_corr);
}






//***********************************************************
// Fuelle TrackEvent
void TQClamTrackProc::FillFinalTrackParameters(TQClamTrackEvent* evt, const FitResults& R, const std::vector<LayerTrackingState>& states, ETrackingMode mode) const
{
	evt->trackAccepted = R.valid;

	evt->phi = R.phi;
	evt->phi_deg = R.phi * 180./M_PI;

	evt->xResolution_X1 = evt->zResolution_X1 / std::abs(std::tan(evt->phi));
	evt->xResolution_X2 = evt->zResolution_X2 / std::abs(std::tan(evt->phi));

	evt->theta = R.theta;
	evt->theta_deg = R.theta * 180./M_PI;

	//--------------------------------------------------
	// X-Ebenen suchen
	//--------------------------------------------------

	const LayerTrackingState* s_x1 = nullptr;
	const LayerTrackingState* s_x2 = nullptr;
	const LayerTrackingState* s_u  = nullptr;

	for(const auto& S : states)
	{
		if(S.raw->layerID == 0) { s_x1 = &S; }
		else if(S.raw->layerID == 1) { s_u = &S; }
		else if(S.raw->layerID == 2) { s_x2 = &S; }
	}

	if(s_x1)
	{
		evt->fBX1 = kTRUE;
	}
	else
	{
		evt->fBX1 = kFALSE;
	}

	if(s_x2)
	{
		evt->fBX2 = kTRUE;
	}
	else
	{
		evt->fBX2 = kFALSE;
	}


	if(!s_x1 || !s_x2)
	{
		return;
	}

	double xD = s_x1->result.intersection;
	double zD = s_x1->raw->zPosLayer;

	double xDX2 = s_x2->result.intersection;

	evt->xD_X1 = xD;
	evt->xD_X2 = xDX2;

	//--------------------------------------------------
	// X1 + X2
	//--------------------------------------------------

	if(mode == ETrackingMode::X1_X2)
	{
		evt->uD   = NAN;
		evt->xD_U = NAN;
		evt->yD_U = NAN;

		evt->fBU = kFALSE;

		evt->theta = NAN;
		evt->theta_deg = NAN;

		evt->posAtX1.SetXYZ(xD,NAN,zD);

		return;
	}

	//--------------------------------------------------
	// U-Ebene erforderlich
	//--------------------------------------------------

	if(!s_u)
	{
		evt->uD   = NAN;
		evt->xD_U = NAN;
		evt->yD_U = NAN;

		evt->fBU = kFALSE;

		return;
	}

	evt->fBX1 = kTRUE;

	double uD = s_u->result.intersection;

	double gamma = s_u->raw->wireAngle;

	evt->uResolution_U = evt->zResolution_U / std::abs(std::tan(evt->phi_U));
	// ohne Beachtung der x-Aufloesung:
	// sigma_y**2 = (dy/du)**2 * sigma_u**2 + (dy/dx)**2 * sigma_x**2
	//            = sqrt(sigma_u**2 + cos(gamma)**2 * sigma_x**2) / sin(gamma)
	// --> sigma_x nicht beachten:
	// sigma_y    = sigma_u / sin(gamma)
	evt->yResolution_U = evt->uResolution_U / std::abs(std::sin(gamma));

	double zDU  = s_u->raw->zPosLayer;
	double zDX2 = s_x2->raw->zPosLayer;

	double d_xx = zDX2 - zD;
	double d_xu = zDU - zD;

	double xDU = (1.0 - d_xu/d_xx) * xD + (d_xu/d_xx) * xDX2;


	//double y_org = 175.;
	double u_org = fSpectrumCaliPar->u_org; //-83.0;
	double yD = (xDU*std::cos(gamma) - uD) / std::sin(gamma);
	double yD_U_corr = (xDU*std::cos(gamma) - (uD-u_org)) / std::sin(gamma);
	//double yD = xDU/std::tan(gamma) - uD / std::sin(gamma);


	evt->uD = uD;
	evt->xD_U = xDU;
	evt->yD_U = yD;
	evt->yD_U_corr = yD_U_corr;
	//evt->yD_U_corr = yD - y_org;

	evt->posAtX1.SetXYZ(xD,yD,zD);
}




//***********************************************************
// Fuelle TrackEvent
void TQClamTrackProc::FillEventFromStates(TQClamTrackEvent* evt, const FitResults& R, const std::vector<LayerTrackingState>& states) const
{
	// ================================
	// 1) globale Fit-Parameter
	// ================================
	evt->iterationSummaries = R.iterationSummaries;


	evt->chi2 = R.quality.chi2;
	evt->chi2ndf = R.quality.chi2ndf;
	evt->nIterations = R.quality.nIterations;

	evt->meanResidual = R.quality.meanResidual;
	evt->rmsResidual = R.quality.rmsResidual;
	evt->maxResidualAbs = R.quality.maxResidualAbs;

	evt->meanResidual_X1 = R.quality.meanResidual_layer[0];
	evt->meanResidual_U = R.quality.meanResidual_layer[1];
	evt->meanResidual_X2 = R.quality.meanResidual_layer[2];

	evt->sigmaResidual_X1 = R.quality.sigmaResidual_layer[0];
	evt->sigmaResidual_U = R.quality.sigmaResidual_layer[1];
	evt->sigmaResidual_X2 = R.quality.sigmaResidual_layer[2];

	evt->rmsResidual_X1 = R.quality.rmsResidual_layer[0];
	evt->rmsResidual_U = R.quality.rmsResidual_layer[1];
	evt->rmsResidual_X2 = R.quality.rmsResidual_layer[2];

	evt->nResiduals_X1 = R.quality.nResiduals_layer[0];
	evt->nResiduals_U = R.quality.nResiduals_layer[1];
	evt->nResiduals_X2 = R.quality.nResiduals_layer[2];

	if(evt->nResiduals_X1 > 0)
	{
		evt->zResolution_X1 = evt->rmsResidual_X1 / std::sqrt(static_cast<double>(evt->nResiduals_X1));
	}

	if(evt->nResiduals_U > 0)
	{
		evt->zResolution_U = evt->rmsResidual_U / std::sqrt(static_cast<double>(evt->nResiduals_U));
	}

	if(evt->nResiduals_X2 > 0)
	{
		evt->zResolution_X2 = evt->rmsResidual_X2 / std::sqrt(static_cast<double>(evt->nResiduals_X2));
	}



	// ================================
	// 2) ResidualEntries (Finale Iteration)
	// ================================
	evt->residualEntries.clear();

	int totalResiduals = 0;
	int totalRejected = 0;

	for(const auto& S : states)
	{
		switch(S.raw->layerID)
		{
			case 0:
				evt->phi_X1 = -S.result.phi*180./M_PI;
				//TGo4Log::Info("phi_X1=%f",evt->phi_X1);
				break;
			case 1:
				evt->phi_U = -S.result.phi*180./M_PI;
				//TGo4Log::Info("phi_U=%f",evt->phi_U);
				break;
			case 2:
				evt->phi_X2 = -S.result.phi*180./M_PI;
				//TGo4Log::Info("phi_X2=%f",evt->phi_X2);
				break;
		}

		if(S.iter.history.empty()) { continue; }

		const auto& lastIter = S.iter.history.back();

		for(const auto& RD : lastIter.residuals)
		{
			ResidualEntry e;
			
			e.layerID = RD.layerID; //S.raw->layerID;
			e.wireNumber = RD.wireNumber; //S.raw->wireNumbers[RD.hitIndex];

			e.residual = RD.residual;
			e.sigma_residual = RD.sigmaResidual;

			e.pull = RD.pull;

			e.drifttime = RD.drifttime;
			e.tot = RD.tot;
			e.z_measured = RD.measuredZOffset;
			e.z_track = RD.trackDistance;

			e.sigma_z_measured = RD.sigma_measuredZOffset;
			e.sigma_z_track = RD.sigma_trackDistance;

			e.phi = R.phi;

			evt->residualEntries.push_back(e);
			totalResiduals++;
		}

		totalRejected += lastIter.nRejected;
	}

	evt->nResiduals = totalResiduals;
	evt->nHitsRejected = totalRejected;
	evt->nHitsUsed = totalResiduals - totalRejected;

	// ================================
	// 3) Iteration-Level Residuen 
	// ================================

	// evtl. konfigurierbarer debug?
	evt->residualIterationEntries.clear();

	for(const auto& S : states)
	{
		int iterIndex = 0;

		for(const auto& hist : S.iter.history)
		{
			for(const auto& RD : hist.residuals)
			{
				ResidualIterationEntry e;

				e.iteration = iterIndex;
				e.layerID = S.raw->layerID;
				e.wireNumber = S.raw->wireNumbers[RD.hitIndex];

				e.residual = RD.residual;
				e.phi = S.result.phi;

				evt->residualIterationEntries.push_back(e);
			}

			iterIndex++;
		}
	}


	// ================================
	// 4) Iteration Summary
	// ================================
	evt->iterationSummaries.clear();

	if(states.empty()) { return; }

	size_t nIter = states[0].iter.history.size();

	for(size_t iter=0; iter<nIter; iter++)
	{
		IterationSummary s;
		s.iteration = iter;

		double totalChi2 = 0.;
		int totalN = 0;
		
		double sumRes = 0.;
		double maxAbs = 0.;
		int count = 0;

		for(const auto& S : states)
		{
			if(iter >= S.iter.history.size()) { continue; }

			const auto& h = S.iter.history[iter];

			totalChi2 += h.chi2;
			totalN += h.residuals.size() - h.nRejected;

			for(const auto& RD : h.residuals)
			{
				sumRes += RD.residual;
				maxAbs = std::max(maxAbs, std::abs(RD.residual));
				count++;
			}
		}

		s.chi2 = totalChi2;

		if(totalN > 2) { s.chi2ndf = totalChi2 / (totalN - 2); }

		if(count > 0)
		{
			s.meanResidual = sumRes / count;
			s.maxResidualAbs = maxAbs;
		}

		evt->iterationSummaries.push_back(s);
	}
} // end FillEventFromStates





//***********************************************************
// Histogramme
//***********************************************************



void TQClamTrackProc::FillRejectedHitHistograms(const std::vector<LayerTrackingState>& states)
{
	int nRejected_X1 = 0;
	int nRejected_U  = 0;
	int nRejected_X2 = 0;

	int nUsed_X1 = 0;
	int nUsed_U = 0;
	int nUsed_X2 = 0;

	for(const auto& S : states)
	{
		if(!S.raw) { continue; }

		const int nHits = static_cast<int>(S.raw->nHits);
		const int nRejected = std::count(S.hitIsOutlier.begin(),S.hitIsOutlier.end(),true);

		const int nUsed = nHits - nRejected;

		switch(S.raw->layerID)
		{
			case 0:
				nRejected_X1 = nRejected;
				nUsed_X1 = nUsed;
				break;

			case 1:
				nRejected_U = nRejected;
				nUsed_U = nUsed;
				break;

			case 2:
				nRejected_X2 = nRejected;
				nUsed_X2 = nUsed;
				break;
		}
	}

	// -------------------------------------------------
	// Rejected
	// -------------------------------------------------

	// Anzahl Tracks mit N rejected Hits erhöhen
	if(static_cast<int>(fRejectedCounter_X1.size()) > nRejected_X1 && nRejected_X1 >= 0)
	{
		fRejectedCounter_X1[nRejected_X1]++;

		// auf Anzahl konvergierter Tracks normieren
		fHRejectedHits_X1->SetBinContent(
				nRejected_X1 + 1,
				double(fRejectedCounter_X1[nRejected_X1])
				/ double(fConvergedTrackCounter)
				);
	}

	if(static_cast<int>(fRejectedCounter_U.size()) > nRejected_U && nRejected_U >= 0)
	{
		fRejectedCounter_U[nRejected_U]++;
	
		fHRejectedHits_U->SetBinContent(
				nRejected_U + 1,
				double(fRejectedCounter_U[nRejected_U])
				/ double(fConvergedTrackCounter)
				);
	}

	if(static_cast<int>(fRejectedCounter_X2.size()) > nRejected_X2 && nRejected_X2 >= 0)
	{
		fRejectedCounter_X2[nRejected_X2]++;
	
		fHRejectedHits_X2->SetBinContent(
				nRejected_X2 + 1,
				double(fRejectedCounter_X2[nRejected_X2])
				/ double(fConvergedTrackCounter)
				);
	}


	// -------------------------------------------------
	// Used
	// -------------------------------------------------

	if(static_cast<int>(fUsedCounter_X1.size()) > nUsed_X1 && nUsed_X1 >= 0)
	{
		fUsedCounter_X1[nUsed_X1]++;

		fHUsedHits_X1->SetBinContent(
				nUsed_X1 + 1,
				double(fUsedCounter_X1[nUsed_X1])
				/ double(fConvergedTrackCounter)
				);
	}

	if(static_cast<int>(fUsedCounter_U.size()) > nUsed_U && nUsed_U >= 0)
	{
		fUsedCounter_U[nUsed_U]++;

		fHUsedHits_U->SetBinContent(
				nUsed_U + 1,
				double(fUsedCounter_U[nUsed_U])
				/ double(fConvergedTrackCounter)
				);
	}

	if(static_cast<int>(fUsedCounter_X2.size()) > nUsed_X2 && nUsed_X2 >= 0)
	{
		fUsedCounter_X2[nUsed_X2]++;

		fHUsedHits_X2->SetBinContent(
				nUsed_X2 + 1,
				double(fUsedCounter_X2[nUsed_X2])
				/ double(fConvergedTrackCounter)
				);
	}

}





void TQClamTrackProc::FillFitQualityHistograms(const FitResults& R)
{
	if(R.valid)
	{
		fConvergedTrackCounter++;
		fHConverged->Fill(0);


		for(const auto& summary : R.iterationSummaries)
		{
			double dPhiDeg = summary.dPhi * 180.0 / M_PI;

			fHDPhiVsIteration->Fill(summary.iteration,dPhiDeg);

			fHRmsVsIteration->Fill(summary.iteration,summary.rmsResidual);
		}


		fHChi2->Fill(R.quality.chi2);

		if(std::isfinite(R.quality.chi2ndf)) { fHChi2NDF->Fill(R.quality.chi2ndf); }

		if(std::isfinite(R.quality.maxResidualAbs)) { fHMaxResidualAbs->Fill(R.quality.maxResidualAbs); }

		if(std::isfinite(R.quality.meanResidual)) { fHMeanResidual->Fill(R.quality.meanResidual); }

		fHIterations->Fill(R.quality.nIterations);

		int nRejected = R.quality.totalRejected;
		if(nRejected >= 0 && nRejected < static_cast<int>(fRejectedCounter.size()))
		{
			fRejectedCounter[nRejected]++;

			fHRejected->SetBinContent(nRejected+1,double(fRejectedCounter[nRejected])/double(fConvergedTrackCounter));
		}
		//fHRejected->Fill(R.quality.totalRejected);
	}
	else
	{
		fNotConvergedTrackCounter++;
		fHConverged->Fill(1);
	}

	if(fConvergedTrackCounter != 0 && fEvtCounter != 0)
	{
		fHTrackEfficiency->SetBinContent(1,100.*double(fConvergedTrackCounter + fNotConvergedTrackCounter) / double(fEvtCounter));
		fHTrackEfficiency->SetBinContent(2,100.*double(fConvergedTrackCounter) / double(fEvtCounter));
		fHTrackEfficiency->SetBinContent(3,100.*double(fConvergedTrackCounter) / (double(fConvergedTrackCounter + fNotConvergedTrackCounter)));
	}
}

//***********************************************************
// Fuelle Histogramme
void TQClamTrackProc::FillHistograms(const TQClamTrackEvent* evt)
{
	double phi_deg = evt->phi_deg;
	double theta_deg = evt->theta_deg;


        fHXSpectrumX1Plane->Fill(evt->posAtX1.X());
        if(fSieveSlitGate && fSieveSlitGate->Test(evt->xD_X1))
        {
                fHXSpectrumUPlane->Fill(evt->xD_U);
                fHYSpectrumUPlane->Fill(evt->yD_U_corr);
                fHXSpectrumX2Plane->Fill(evt->xD_X2);
                fHUSpectrum->Fill(evt->uD);
                fHPhiSpectrum->Fill(phi_deg);
                fHThetaSpectrum->Fill(theta_deg);

                fHPhiCorrSpectrum->Fill(evt->phi_corr);

                fHPhiVsX1->Fill(evt->xD_X1,phi_deg);
                fHPhiVsX2->Fill(evt->xD_X2,phi_deg);
                fHThetaVsY->Fill(evt->yD_U_corr,evt->theta_deg);
                fHThetaVsPhi->Fill(evt->phi_corr, evt->theta_deg);
                fHYVsX->Fill(evt->x_corr,evt->yD_U_corr);

                fHPhiCorrVsX1Corr->Fill(evt->x_corr, evt->phi_corr);

                fHProjSpectrum->Fill(evt->proj_position.X());

                fHXCorrSpectrum->Fill(evt->x_corr);
                fHExEnergySpectrum->Fill(evt->e_MeV);
		fHPhiVsY->Fill(evt->yD_U_corr,evt->phi_deg);
		fHPhiCorrVsY->Fill(evt->yD_U_corr,evt->phi_corr);

		fHPhiVsTheta->Fill(theta_deg,phi_deg);

		// Streukoordinaten
		fHScattTheta->Fill(evt->scatteringTheta);
		fHScattPhi->Fill(evt->scatteringPhi);

		fHScattXPos->Fill(evt->scatteringX);
		fHScattYPos->Fill(evt->scatteringY);
		fHScattXYPos->Fill(evt->scatteringX,evt->scatteringY);

		// BeamPos fill
		FillBeamPositionMonitor(evt);


		fHScattPhiVsScattTheta->Fill(evt->scatteringTheta,evt->scatteringPhi);

		fHScattPhiVsX1->Fill(evt->xD_X1,evt->scatteringPhi);
		fHScattPhiVsXCorr->Fill(evt->x_corr,evt->scatteringPhi);
		fHScattPhiVsY->Fill(evt->yD_U_corr,evt->scatteringPhi);

		fHScattThetaVsX1->Fill(evt->xD_X1,evt->scatteringTheta);
		fHScattThetaVsXCorr->Fill(evt->x_corr,evt->scatteringTheta);
		fHScattThetaVsY->Fill(evt->yD_U_corr,evt->scatteringTheta);

		fHScattPhiVsPhi->Fill(phi_deg,evt->scatteringPhi);
		fHScattPhiVsPhiCorr->Fill(evt->phi_corr,evt->scatteringPhi);
		fHScattThetaVsTheta->Fill(evt->theta_deg,evt->scatteringTheta);

        }


	/** Fuelle Driftdistanz vs Zeit histogramme */
	for(const auto& r : evt->residualEntries)
	{
		int l = r.layerID;
		double t = r.drifttime;
		double d_meas = std::abs(r.z_measured);
		double d_track = std::abs(r.z_track);

		switch(l)
		{
			case 0:
			{
				fHDriftDistanceMeasVsTime_X1->Fill(t,d_meas);
				fHDriftDistanceTrackVsTime_X1->Fill(t,d_track);
				break;
			}

			case 1:
			{
				fHDriftDistanceMeasVsTime_U->Fill(t,d_meas);
				fHDriftDistanceTrackVsTime_U->Fill(t,d_track);
				break;
			}

			case 2:
			{
				fHDriftDistanceMeasVsTime_X2->Fill(t,d_meas);
				fHDriftDistanceTrackVsTime_X2->Fill(t,d_track);
				break;
			}
		}

			fHDriftDistanceMeasVsTime_All->Fill(t,d_meas);
			fHDriftDistanceTrackVsTime_All->Fill(t,d_track);

	}

/*
	fHXSpectrumX1Plane->Fill(evt->posAtX1.X());
	fHXSpectrumUPlane->Fill(evt->xD_U);
	fHYSpectrumUPlane->Fill(evt->yD_U_corr);
	fHXSpectrumX2Plane->Fill(evt->xD_X2);
	fHUSpectrum->Fill(evt->uD);
	fHPhiSpectrum->Fill(phi_deg);
	fHThetaSpectrum->Fill(theta_deg);

	fHPhiCorrSpectrum->Fill(evt->phi_corr);

	fHPhiVsX1->Fill(evt->xD_X1,phi_deg);
	fHPhiVsX2->Fill(evt->xD_X2,phi_deg);
	fHThetaVsY->Fill(evt->yD_U_corr,evt->theta_deg);
	fHPhiVsY->Fill(evt->yD_U_corr,evt->phi_deg);
	fHPhiCorrVsY->Fill(evt->yD_U_corr,evt->phi_corr);
	fHThetaVsPhi->Fill(evt->phi_corr, evt->theta_deg);
	fHYVsX->Fill(evt->x_corr,evt->yD_U_corr);

	fHPhiCorrVsX1Corr->Fill(evt->x_corr, evt->phi_corr);

	fHProjSpectrum->Fill(evt->proj_position.X());

	fHXCorrSpectrum->Fill(evt->x_corr);
	fHExEnergySpectrum->Fill(evt->e_MeV);
*/
	/// TODO: aufpassen: hier fuelle ich die stdw nicht das Rms in RmsResidual!!!
	fHRmsResidual_X1->Fill(evt->sigmaResidual_X1);
	fHRmsResidual_U->Fill(evt->sigmaResidual_U);
	fHRmsResidual_X2->Fill(evt->sigmaResidual_X2);

	fHMeanResidual_X1->Fill(evt->meanResidual_X1);
	fHMeanResidual_U->Fill(evt->meanResidual_U);
	fHMeanResidual_X2->Fill(evt->meanResidual_X2);

	fHPhi_PhiX1->Fill(evt->phi_deg-evt->phi_X1);
	fHPhi_PhiU->Fill(evt->phi_deg-evt->phi_U);
	fHPhi_PhiX2->Fill(evt->phi_deg-evt->phi_X2);

	/// Track-Aufloesung:
	fHZResolution_X1->Fill(evt->zResolution_X1);
	fHZResolution_U->Fill(evt->zResolution_U);
	fHZResolution_X2->Fill(evt->zResolution_X2);

	fHXResolution_X1->Fill(evt->xResolution_X1);
	fHXResolution_X2->Fill(evt->xResolution_X2);

	fHUResolution_U->Fill(evt->uResolution_U);
	fHYResolution_U->Fill(evt->yResolution_U);

	for(const auto& e : evt->residualEntries)
	{
		if(e.layerID == 0) { fHResidual_X1->Fill(e.residual); }
		else if(e.layerID == 1) { fHResidual_U->Fill(e.residual); }
		else if(e.layerID == 2) { fHResidual_X2->Fill(e.residual); }

		double phi = evt->phi_deg;

		if(25 <= phi && phi < 35)
		{
			if(e.layerID == 0) { fHResidual_X1_phiBin_25_35->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_phiBin_25_35->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_phiBin_25_35->Fill(e.residual); }
		}

		else if(35 <= phi && phi < 45)
		{
			if(e.layerID == 0) { fHResidual_X1_phiBin_35_45->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_phiBin_35_45->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_phiBin_35_45->Fill(e.residual); }
		}

		else if(45 <= phi && phi < 55)
		{
			if(e.layerID == 0) { fHResidual_X1_phiBin_45_55->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_phiBin_45_55->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_phiBin_45_55->Fill(e.residual); }
		}

		else if(55 <= phi && phi < 65)
		{
			if(e.layerID == 0) { fHResidual_X1_phiBin_55_65->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_phiBin_55_65->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_phiBin_55_65->Fill(e.residual); }
		}


		double tD = e.drifttime;

		if(0 <= tD && tD < 50)
		{
			if(e.layerID == 0) { fHResidual_X1_drifttimeBin_0_50->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_drifttimeBin_0_50->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_drifttimeBin_0_50->Fill(e.residual); }
		}

		else if(50 <= tD && tD < 100)
		{
			if(e.layerID == 0) { fHResidual_X1_drifttimeBin_50_100->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_drifttimeBin_50_100->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_drifttimeBin_50_100->Fill(e.residual); }
		}

		else if(100 <= tD && tD < 150)
		{
			if(e.layerID == 0) { fHResidual_X1_drifttimeBin_100_150->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_drifttimeBin_100_150->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_drifttimeBin_100_150->Fill(e.residual); }
		}

		else if(150 <= tD && tD < 200)
		{
			if(e.layerID == 0) { fHResidual_X1_drifttimeBin_150_200->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_drifttimeBin_150_200->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_drifttimeBin_150_200->Fill(e.residual); }
		}

		else if(200 <= tD && tD < 250)
		{
			if(e.layerID == 0) { fHResidual_X1_drifttimeBin_200_250->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_drifttimeBin_200_250->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_drifttimeBin_200_250->Fill(e.residual); }
		}

		else if(250 <= tD && tD < 300)
		{
			if(e.layerID == 0) { fHResidual_X1_drifttimeBin_250_300->Fill(e.residual); }
			else if(e.layerID == 1) { fHResidual_U_drifttimeBin_250_300->Fill(e.residual); }
			else if(e.layerID == 2) { fHResidual_X2_drifttimeBin_250_300->Fill(e.residual); }
		}







		continue;
	}

}




//***********************************************************
// BeamPosition Monitor 
void TQClamTrackProc::FillBeamPositionMonitor(const TQClamTrackEvent* evt)
{
	double x = evt->scatteringX;
	double y = evt->scatteringY;


	if(!std::isfinite(x) || !std::isfinite(y)) { return; }



	// ----------------------------------
	// all data
	// ----------------------------------

	fHBeamX_All->Fill(x);
	fHBeamY_All->Fill(y);
	fHBeamXY_All->Fill(x,y);


	// ----------------------------------
	// live window
	// ----------------------------------

	fHBeamX_Live->Fill(x);
	fHBeamY_Live->Fill(y);
	fHBeamXY_Live->Fill(x,y);



	// ----------------------------------
	// statistics
	// ----------------------------------

	fBeamSumX += x;
	fBeamSumY += y;

	fBeamSumX2 += x*x;
	fBeamSumY2 += y*y;

	fBeamStatCounter++;


	fBeamMonitorCounter++;


	if(fBeamMonitorCounter >= fBeamMonitorWindow)
	{
		UpdateBeamPositionMonitor();

		fBeamMonitorCounter=0;


		// neue Messperiode
		fHBeamX_Live->Reset();
		fHBeamY_Live->Reset();
		fHBeamXY_Live->Reset();


		fBeamSumX=0.;
		fBeamSumY=0.;
		fBeamSumX2=0.;
		fBeamSumY2=0.;
		fBeamStatCounter=0;
	}

}




void TQClamTrackProc::UpdateBeamPositionMonitor()
{
	if(fBeamStatCounter==0) { return; }


	double meanX = fBeamSumX / fBeamStatCounter;


	double meanY = fBeamSumY / fBeamStatCounter;



	double rmsX = sqrt(fBeamSumX2/fBeamStatCounter - meanX*meanX);


	double rmsY = sqrt(fBeamSumY2/fBeamStatCounter - meanY*meanY);



	int evt = fEvtCounter/fBeamMonitorWindow;


	int n_x = fPBeamX_Mean->GetN();
	int n_y = fPBeamY_Mean->GetN();

	fPBeamX_Mean->SetPoint(n_x,fGraphTime,meanX);
	fPBeamY_Mean->SetPoint(n_y,fGraphTime,meanY);



	// RMS kannst als TH1/TGraph machen.
	// TProfile mittelt, daher nicht ideal.
}



// TODO
/*
IntersectionInput MakeSimpleInput(const LayerRawData& L)
{
    IntersectionInput in;

    in.layerID = L.layerID;
    in.type = L.type;

    in.hasVShape = L.hasVShape;
    in.hasGaps = L.hasGaps;
    in.isAscending = L.isAscending;
    in.isDescending = L.isDescending;

    in.nHits = L.nHits;
    in.normalWireDist = L.normalWireDist;

    in.normalCoords = L.normalCoords;
    in.drifttimes = L.drifttimes;

    // hier FIRST GUESS nehmen:
    in.zOffsets = L.drifttimes; // oder besser: vorher ComputeZOffset!

    return in;
}
*/
