//-----------------------------------------------------------------------
// author: O. Moeller
// created: 03.2026
// last modified:
//
// processor: 
// - nehme Rohdaten entgegen und erzeuge daraus Driftkammer- sowie Trigger-Hits
// - input: TQClamRawEvent (--> output von TQClamOfflineRootSource)
// - output: TQClamHitEvent
//-----------------------------------------------------------------------
#include <iostream>
#include "processor/TQClamHitProc.h"

/** go4-includes */
#include "TGo4Log.h"
#include "TGo4CondArray.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"
#include "TGo4CounterCond.h"
#include "TGo4Fitter.h"
#include "TGo4FitDataHistogram.h"

/** ROOT-includes */
#include "TClass.h"
#include "TCutG.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "THStack.h"

/** evts */
#include "evt/TQClamHitEvent.h"


#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
#include "evt/TQClamRawEvent.h"
#endif

#ifdef QCLAM_DAGOBERT
#include "evt/TQClamDagobertRawEvent.h"
#endif

/** parameter */
#include "parameter/TQClamDriftchamGeomParameter.h"
#include "parameter/TQClamEdgeMappingParameter.h"
#include "parameter/TQClamDriftchamCalibrationParameter.h"
#include "parameter/TQClamTriggerCalibrationParameter.h"
#include "parameter/TQClamFaradayCupCurrentCalibrationParameter.h"
#include "parameter/TQClamHitConditionParameter.h"
#include "parameter/TQClamHitProcDebugOutputParameter.h"
#include "parameter/TQClamDisplaySettingParameter.h"
#include "parameter/TQClamDrifttimeCalibParameter.h"
#include "parameter/TQClamHistogramParameter.h"

/** Go4-Klassen */
#include "TGo4RollingGraph.h"
#include "TGo4Picture.h"

/** geometry */
#include "geometry/TQClamGeometry.h"
#include "geometry/TQClamDriftcham.h"

#include <fstream>
#include <vector>




ClassImp(TQClamHitProc)

//***********************************************************
TQClamHitProc::TQClamHitProc()
	: TGo4EventProcessor()
//	fDriftcham(TQClamGeometry::Instance().GetDriftcham())
{}

//***********************************************************
TQClamHitProc::TQClamHitProc(const char* name)
	: TGo4EventProcessor(name)
//	fDriftcham(TQClamGeometry::Instance().GetDriftcham())
{
	TGo4Log::Info("[TQClamHitProc] Create %s", name);




	//-----------------------------------------------------------
	// initialisiere interne Variablen/Parameter
	fFaradayCupChargeCounter_LastEvent = 0;
	fSzintPmtUpScaler_LastEvent = 0;
	fSzintPmtDownScaler_LastEvent = 0;
	fCherenkovPmtScaler_LastEvent = 0;
	fCoincidenceScaler_LastEvent = 0;
	fAcceptedScaler_LastEvent = 0;
	fEvtCounter = 0;
	fTimeInMicroSeconds = 0.0;

	fHitCounterX1 = 0;
	fHitCounterU = 0;
	fHitCounterX2 = 0;
	fHitCounterX1Cut = 0;
	fHitCounterUCut = 0;
	fHitCounterX2Cut = 0;

	for(size_t i=0; i<QCLAM_MAX_WIRE;i++)
	{
		fHitCounterX1Wire[i] = 0;
		fHitCounterUWire[i] = 0;
		fHitCounterX2Wire[i] = 0;
		fHitCounterX1WireCut[i] = 0;
		fHitCounterUWireCut[i] = 0;
		fHitCounterX2WireCut[i] = 0;
	}




	//-----------------------------------------------------------
	// Parameter, Histogramme etc initialisieren
	//-----------------------------------------------------------

	//-----------------------------------------------------------
	// 1. initialisiere Parameter
	fDriftchamGeomPar = (TQClamDriftchamGeomParameter*) GetParameter("DriftchamGeomParam");
	fTriggerCaliPar = (TQClamTriggerCalibrationParameter*) GetParameter("TriggerCalibParam");
	fDriftchamCaliPar = (TQClamDriftchamCalibrationParameter*) GetParameter("DriftchamCalibParam");
	fEdgeMappingPar = (TQClamEdgeMappingParameter*) GetParameter("EdgeMappingParam");
	fHitConditionPar = (TQClamHitConditionParameter*) GetParameter("HitConditionParam");
	fHitProcDebugOutputPar = (TQClamHitProcDebugOutputParameter*) GetParameter("HitProcDebugOutputParam");
	fFaradayCupCurrentCaliPar = (TQClamFaradayCupCurrentCalibrationParameter*) GetParameter("FaradayCupCurrentCalibParam");
	fDisplaySettingPar = (TQClamDisplaySettingParameter*) GetParameter("DisplaySettingParam");
	fDrifttimeCalibPar = (TQClamDrifttimeCalibParameter*) GetParameter("DrifttimeCalibParam");
	fHistoPar = (TQClamHistogramParameter*) GetParameter("HistoParam");



	//-----------------------------------------------------------
	// 2. intialisiere Conditions
	/*
	fWinCondToT = MakeWinCond("ToTCondition", fHitConditionPar->minToT_ns, fHitConditionPar->maxToT_ns);
	fWinCondDrifttime = MakeWinCond("DrifttimeCondition", fHitConditionPar->minDrifttime_ns, fHitConditionPar->maxDrifttime_ns);


	Double_t xmin = fWinCondToT->GetXLow();
	Double_t xmax = fWinCondToT->GetXUp();
	Double_t ymin = fWinCondDrifttime->GetXLow();
	Double_t ymax = fWinCondDrifttime->GetXUp();
	Double_t cutpnts[5][2] = { {xmin,ymin},{xmax,ymin},{xmax,ymax},{xmin,ymax},{xmin,ymin} };
	fPolyCondDrifttimeVsToT = MakePolyCond("DrifttimeVsToTCondition",5,cutpnts);
	*/

	/*
	Double_t x[5] = {xmin,xmax,xmax,xmin,xmin};
	Double_t y[5] = {ymin,ymin,ymax,ymax,ymin};
	fPolyCondDrifttimeVsToT->SetValues(x,y,5);
	*/

	/** ToT-Cut */
	fCondArrToT = (TGo4CondArray*)GetAnalysisCondition("CondArrToT");
	if(!fCondArrToT)
	{
		fCondArrToT = new TGo4CondArray("CondArrToT",fHitConditionPar->MAX_LAYER,"TGo4WinCond");
		fCondArrToT->SetValues(-1e9,1e9); /** default */
		fCondArrToT->Enable();
		for(int i=0; i<fHitConditionPar->MAX_LAYER; i++)
		{
			((*fCondArrToT)[i])->SetValues(fHitConditionPar->minToT_ns[i],fHitConditionPar->maxToT_ns[i]);
			AddAnalysisCondition((*fCondArrToT)[i]);
		}
		//AddAnalysisCondition(fCondArrToT);
	}
	else
	{
		TGo4Log::Info("[TQClamHitProc] Lade CondArrToT aus Autosave-Datei.");
		fCondArrToT->ResetCounts();
	}



	/** Drifttime-Cut */
	fCondArrDrifttime = (TGo4CondArray*)GetAnalysisCondition("CondArrDrifttime");
	if(!fCondArrDrifttime)
	{
		fCondArrDrifttime = new TGo4CondArray("CondArrDrifttime",fHitConditionPar->MAX_LAYER,"TGo4WinCond");
		fCondArrDrifttime->SetValues(-1e9,1e9); /** default */
		fCondArrDrifttime->Enable();
		for(size_t i=0; i<fHitConditionPar->MAX_LAYER; i++)
		{
			((*fCondArrDrifttime)[i])->SetValues(fHitConditionPar->minDrifttime_ns[i],fHitConditionPar->maxDrifttime_ns[i]);
			AddAnalysisCondition((*fCondArrDrifttime)[i]);
		}
		//AddAnalysisCondition(fCondArrDrifttime);
	}
	else
	{
		TGo4Log::Info("[TQClamHitProc] Lade CondArrDrifttime aus Autosave-Datei.");
		fCondArrDrifttime->ResetCounts();
	}


	/** Drifttime-ToT-Cut */
	fCondArrDrifttimeVsToT = (TGo4CondArray*)GetAnalysisCondition("CondArrDrifttimeVsToT");
	if(!fCondArrDrifttimeVsToT)
	{
		fCondArrDrifttimeVsToT = new TGo4CondArray("CondArrDrifttimeVsToT",fHitConditionPar->MAX_LAYER,"TGo4PolyCond");
		Double_t xvalDefault[5] = { -1e9,1e9,1e9,-1e9,-1e9 };
		Double_t yvalDefault[5] = { -1e9,-1e9,1e9,1e9,-1e9 };
		TCutG* cutDefault = new TCutG("cut",5,xvalDefault,yvalDefault);
		fCondArrDrifttimeVsToT->SetValues(cutDefault); /** default */
		fCondArrDrifttimeVsToT->Enable();
		delete cutDefault;

		for(size_t i=0; i<fHitConditionPar->MAX_LAYER; i++)
		{
			double xmin = fHitConditionPar->minToT_ns[i];
			double xmax = fHitConditionPar->maxToT_ns[i];
			double ymin = fHitConditionPar->minDrifttime_ns[i];
			double ymax = fHitConditionPar->maxDrifttime_ns[i];

			Double_t xval[5] = {xmin,xmax,xmax,xmin,xmin};
			Double_t yval[5] = {ymin,ymin,ymax,ymax,ymin};

			TCutG* cut = new TCutG("cut",5,xval,yval);
			((*fCondArrDrifttimeVsToT)[i])->SetValues(cut);
			AddAnalysisCondition((*fCondArrDrifttimeVsToT)[i]);
			delete cut;
		}
		//AddAnalysisCondition(fCondArrDrifttimeVsToT);
	}
	else
	{
		TGo4Log::Info("[TQClamHitProc] Lade CondArrDrifttimeVsToT aus Autosave-Datei.");
		fCondArrDrifttimeVsToT->ResetCounts();
	}


	/** zaehle, wie viele Hits HitProc passieren */
	fCondCountHitProcPassed = (TGo4CounterCond*)GetAnalysisCondition("CondCountHitProcPassedHits");
	if(!fCondCountHitProcPassed)
	{
		fCondCountHitProcPassed = new TGo4CounterCond("CondCountHitProcPassedHits");
		AddAnalysisCondition(fCondCountHitProcPassed);
	}
	else
	{
		TGo4Log::Info("[TQClamHitProc] Lade CondCountHitProcPassedHits aus Autosave-Datei");
		fCondCountHitProcPassed->ResetCounts();
	}

	/** nun fuer X1 */
	fCondCountHitProcPassedX1 = (TGo4CounterCond*)GetAnalysisCondition("CondCountHitProcPassedX1Hits");
	if(!fCondCountHitProcPassedX1)
	{
		fCondCountHitProcPassedX1 = new TGo4CounterCond("CondCountHitProcPassedX1Hits");
		AddAnalysisCondition(fCondCountHitProcPassedX1);
	}
	else
	{
		TGo4Log::Info("[TQClamHitProc] Lade CondCountHitProcPassedX1Hits aus Autosave-Datei");
		fCondCountHitProcPassedX1->ResetCounts();
	}

	/** nun fuer U */
	fCondCountHitProcPassedU = (TGo4CounterCond*)GetAnalysisCondition("CondCountHitProcPassedUHits");
	if(!fCondCountHitProcPassedU)
	{
		fCondCountHitProcPassedU = new TGo4CounterCond("CondCountHitProcPassedUHits");
		AddAnalysisCondition(fCondCountHitProcPassedU);
	}
	else
	{
		TGo4Log::Info("[TQClamHitProc] Lade CondCountHitProcPassedUHits aus Autosave-Datei");
		fCondCountHitProcPassedU->ResetCounts();
	}

	/** nun fuer X2 */
	fCondCountHitProcPassedX2 = (TGo4CounterCond*)GetAnalysisCondition("CondCountHitProcPassedX2Hits");
	if(!fCondCountHitProcPassedX2)
	{
		fCondCountHitProcPassedX2 = new TGo4CounterCond("CondCountHitProcPassedX2Hits");
		AddAnalysisCondition(fCondCountHitProcPassedX2);
	}
	else
	{
		TGo4Log::Info("[TQClamHitProc] Lade CondCountHitProcPassedX2Hits aus Autosave-Datei");
		fCondCountHitProcPassedX2->ResetCounts();
	}






	//-----------------------------------------------------------
	// 3. initialisiere Histogramme



	/** grenzen histogramme nach anwendung condition */
	/* driftzeit */
	double minDrifttime_0 = fHitConditionPar->minDrifttime_ns[0];
	double maxDrifttime_0 = fHitConditionPar->maxDrifttime_ns[0];
	int nBinsDrifttime_0 = int(maxDrifttime_0 - minDrifttime_0);

	double minDrifttime_1 = fHitConditionPar->minDrifttime_ns[1];
	double maxDrifttime_1 = fHitConditionPar->maxDrifttime_ns[1];
	int nBinsDrifttime_1 = int(maxDrifttime_1 - minDrifttime_1);

	double minDrifttime_2 = fHitConditionPar->minDrifttime_ns[2];
	double maxDrifttime_2 = fHitConditionPar->maxDrifttime_ns[2];
	int nBinsDrifttime_2 = int(maxDrifttime_2 - minDrifttime_2);

	double minDrifttime = std::min({minDrifttime_0,minDrifttime_1,minDrifttime_2});
	double maxDrifttime = std::max({maxDrifttime_0,maxDrifttime_1,maxDrifttime_2});
	int nBinsDrifttime = int(maxDrifttime - minDrifttime);

	/* tot */
	double minToT_0 = fHitConditionPar->minToT_ns[0];
	double maxToT_0 = fHitConditionPar->maxToT_ns[0];
	int nBinsToT_0 = int(maxToT_0 - minToT_0);

	double minToT_1 = fHitConditionPar->minToT_ns[1];
	double maxToT_1 = fHitConditionPar->maxToT_ns[1];
	int nBinsToT_1 = int(maxToT_1 - minToT_1);

	double minToT_2 = fHitConditionPar->minToT_ns[2];
	double maxToT_2 = fHitConditionPar->maxToT_ns[2];
	int nBinsToT_2 = int(maxToT_2 - minToT_2);

	double minToT = std::min({minToT_0,minToT_1,minToT_2});
	double maxToT = std::max({maxToT_0,maxToT_1,maxToT_2});
	int nBinsToT = int(maxToT - minToT);

	/* drahtnummer */
	int minWire_0 = 0;
	int maxWire_0 = fDriftchamGeomPar->nWires[0];
	int nBinsWire_0 = maxWire_0 - minWire_0;

	int minWire_1 = 0;
	int maxWire_1 = fDriftchamGeomPar->nWires[1];
	int nBinsWire_1 = maxWire_1 - minWire_1;

	int minWire_2 = 0;
	int maxWire_2 = fDriftchamGeomPar->nWires[2];
	int nBinsWire_2 = maxWire_2 - minWire_2;

	/** Ebenen */
	int minLayer = 0;
	int maxLayer = fDriftchamGeomPar->nPlanes;
	int nBinsLayer = maxLayer - minLayer;





	/** TODO! */
	fHScaler = MakeTH1('I', "Scaler", "Scaler: SzintPmtUp / SzintPmtDown / CherenkovPmt / CoincTrig / AccTrig / ClockCounter / Livetime / MbsLivetime / FaradayCupChargeCounter", 9, 0, 9);




	/** Zeiten Trigger-Detektoren */
	fHSzintUpTime = MakeTH1('D',"SzintUpTime","Triggerzeit Szintillator Pmt Up",4800,0,1200);
	fHSzintUpTime->GetXaxis()->SetTitle("Zeit/ns");
	fHSzintUpTime->GetXaxis()->CenterTitle();
	fHSzintUpTime->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHSzintUpTime->GetYaxis()->CenterTitle();

	fHSzintDownTime = MakeTH1('D',"SzintDownTime","Triggerzeit Szintillator Pmt Down",4800,0,1200);
	fHSzintDownTime->GetXaxis()->SetTitle("Zeit/ns");
	fHSzintDownTime->GetXaxis()->CenterTitle();
	fHSzintDownTime->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHSzintDownTime->GetYaxis()->CenterTitle();

	fHMeanSzintTime = MakeTH1('D',"MeanSzintUpDownTime","Triggerzeit (SzintPmtUp + SzintPmtDown)/2",4800,0,1200);
	fHMeanSzintTime->GetXaxis()->SetTitle("Zeit/ns");
	fHMeanSzintTime->GetXaxis()->CenterTitle();
	fHMeanSzintTime->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHMeanSzintTime->GetYaxis()->CenterTitle();

	fHDiffSzintTime = MakeTH1('D',"DiffSzintUpDownTime","Triggerzeit (SzintPmtUp-SzintPmtDown)",4800,-600,600);
	fHDiffSzintTime->GetXaxis()->SetTitle("Zeit/ns");
	fHDiffSzintTime->GetXaxis()->CenterTitle();
	fHDiffSzintTime->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDiffSzintTime->GetYaxis()->CenterTitle();

	fHCherenkovTime = MakeTH1('D',"CherenkovTime","Triggerzeit Cherenkov",4800,0,1200);
	fHCherenkovTime->GetXaxis()->SetTitle("Zeit/ns");
	fHCherenkovTime->GetXaxis()->CenterTitle();
	fHCherenkovTime->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHCherenkovTime->GetYaxis()->CenterTitle();

	fHSzintUpVsSzintDown = MakeTH2('D',"TimeSzintUpVsSzintDown","Triggerzeit Szintillator Pmt Up vs. Szintillator Pmt Down",
			4800,0,1200,4800,0,1200);
	fHSzintUpVsSzintDown->GetXaxis()->SetTitle("Szint Down Zeit/ns");
	fHSzintUpVsSzintDown->GetXaxis()->CenterTitle();
        fHSzintUpVsSzintDown->GetYaxis()->SetTitle("Szint Up Zeit/ns");
        fHSzintUpVsSzintDown->GetYaxis()->CenterTitle();











	/** - Statistik/Rate ganze Ebene - */

	/** hit-anzahl */
	fHHitsInX1 = MakeTH1('I', "HitsInLayer","Hit-Verteilung auf Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	//fHHitsInX1->SetLineColor(kGreen);
	fHHitsInX1->SetFillColor(3);
	fHHitsInX1->SetFillStyle(3031);
//	fHHitsInX1->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	fHHitsInX1->GetXaxis()->SetBinLabel(1,"X1");
	fHHitsInX1->GetXaxis()->SetBinLabel(2,"U");
	fHHitsInX1->GetXaxis()->SetBinLabel(3,"X2");
	//fHHitsInX1->GetXaxis()->CenterTitle();
	fHHitsInX1->GetYaxis()->SetTitle("Anzahl Hits");
	fHHitsInX1->GetYaxis()->CenterTitle();

	fHHitsInU = MakeTH1('I', "HitsInLayer1","Hit-Verteilung auf Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	//fHHitsInU->SetLineColor(kMagenta);
	fHHitsInU->SetFillColor(6);
	fHHitsInU->SetFillStyle(3031);
//	fHHitsInU->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitsInU->GetXaxis()->SetBinLabel(1,"X1");
	//fHHitsInU->GetXaxis()->SetBinLabel(1,"U");
	//fHHitsInU->GetXaxis()->SetBinLabel(3,"X2");
	fHHitsInU->GetXaxis()->CenterTitle();
	fHHitsInU->GetYaxis()->SetTitle("Anzahl Hits");
	fHHitsInU->GetYaxis()->CenterTitle();

	fHHitsInX2 = MakeTH1('I', "HitsInLayer2","Hit-Verteilung auf Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	//fHHitsInX2->SetLineColor(kCyan);
	fHHitsInX2->SetFillColor(7);
	fHHitsInX2->SetFillStyle(3031);
	//fHHitsInX2->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitsInX2->GetXaxis()->CenterTitle();
	fHHitsInX2->GetYaxis()->SetTitle("Anzahl Hits");
	fHHitsInX2->GetYaxis()->CenterTitle();

	fHStackHitsInLayer = new THStack("HitsInLayerStack","Hit-Verteilung auf Driftkammer-Ebenen");
	fHStackHitsInLayer->Add(fHHitsInX1);
	fHStackHitsInLayer->Add(fHHitsInU);
	fHStackHitsInLayer->Add(fHHitsInX2);

	AddObject(fHStackHitsInLayer);



	/** evt-rate */
	fHHitRateEvtInX1 = MakeTH1('D', "HitRateEvtInLayer0","Hit-Raten (Evt) in Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	fHHitRateEvtInX1->SetFillColor(3);
	fHHitRateEvtInX1->SetFillStyle(3031);
	//fHHitRateEvtInX1->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	fHHitRateEvtInX1->GetXaxis()->SetBinLabel(1,"X1");
	fHHitRateEvtInX1->GetXaxis()->SetBinLabel(2,"U");
	fHHitRateEvtInX1->GetXaxis()->SetBinLabel(3,"X2");
	//fHHitRateEvtInX1->GetXaxis()->CenterTitle();
	fHHitRateEvtInX1->GetYaxis()->SetTitle("Rate: Hits/Evts");
	fHHitRateEvtInX1->GetYaxis()->CenterTitle();

	fHHitRateEvtInU = MakeTH1('D', "HitRateEvtInLayer1","Hit-Raten (Evt) in Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	fHHitRateEvtInU->SetFillColor(6);
	fHHitRateEvtInU->SetFillStyle(3031);
	//fHHitRateEvtInU->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateEvtInU->GetXaxis()->CenterTitle();
	fHHitRateEvtInU->GetYaxis()->SetTitle("Rate: Hits/Evts");
	fHHitRateEvtInU->GetYaxis()->CenterTitle();

	fHHitRateEvtInX2 = MakeTH1('D', "HitRateEvtInLayer2","Hit-Raten (Evt) in Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	fHHitRateEvtInX2->SetFillColor(7);
	fHHitRateEvtInX2->SetFillStyle(3031);
	//fHHitRateEvtInX2->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateEvtInX2->GetXaxis()->CenterTitle();
	fHHitRateEvtInX2->GetYaxis()->SetTitle("Rate: Hits/Evts");
	fHHitRateEvtInX2->GetYaxis()->CenterTitle();

	fHStackHitRateEvtInLayer = new THStack("HitRateEvtInLayerStack","Hit-Raten (Evt) Driftkammer-Ebenen");
	fHStackHitRateEvtInLayer->Add(fHHitRateEvtInX1);
	fHStackHitRateEvtInLayer->Add(fHHitRateEvtInU);
	fHStackHitRateEvtInLayer->Add(fHHitRateEvtInX2);

	AddObject(fHStackHitRateEvtInLayer);



	/** zeitl. rate */
	fHHitRateTimeInX1 = MakeTH1('D', "HitRateTimeInLayer0","Hit-Raten (Zeit) in Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	fHHitRateTimeInX1->SetFillColor(3);
	fHHitRateTimeInX1->SetFillStyle(3031);
	//fHHitRateTimeInX1->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	fHHitRateTimeInX1->GetXaxis()->SetBinLabel(1,"X1");
	fHHitRateTimeInX1->GetXaxis()->SetBinLabel(2,"U");
	fHHitRateTimeInX1->GetXaxis()->SetBinLabel(3,"X2");
	//fHHitRateTimeInX1->GetXaxis()->CenterTitle();
	fHHitRateTimeInX1->GetYaxis()->SetTitle("Rate: Hits/Zeit (1/s)");
	fHHitRateTimeInX1->GetYaxis()->CenterTitle();

	fHHitRateTimeInU = MakeTH1('D', "HitRateTimeInLayer1","Hit-Raten (Zeit) in Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	fHHitRateTimeInU->SetFillColor(6);
	fHHitRateTimeInU->SetFillStyle(3031);
	//fHHitRateTimeInU->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateTimeInU->GetXaxis()->CenterTitle();
	fHHitRateTimeInU->GetYaxis()->SetTitle("Rate: Hits/Zeit (1/s)");
	fHHitRateTimeInU->GetYaxis()->CenterTitle();

	fHHitRateTimeInX2 = MakeTH1('D', "HitRateTimeInLayer2","Hit-Raten (Zeit) in Driftkammer-Ebenen", nBinsLayer,minLayer,maxLayer);
	fHHitRateTimeInX2->SetFillColor(7);
	fHHitRateTimeInX2->SetFillStyle(3031);
	//fHHitRateTimeInX2->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateTimeInX2->GetXaxis()->CenterTitle();
	fHHitRateTimeInX2->GetYaxis()->SetTitle("Rate: Hits/Zeit (1/s)");
	fHHitRateTimeInX2->GetYaxis()->CenterTitle();

	fHStackHitRateTimeInLayer = new THStack("HitRateTimeInLayerStack","Hit-Raten (Zeit) Driftkammer-Ebenen");
	fHStackHitRateTimeInLayer->Add(fHHitRateTimeInX1);
	fHStackHitRateTimeInLayer->Add(fHHitRateTimeInU);
	fHStackHitRateTimeInLayer->Add(fHHitRateTimeInX2);

	AddObject(fHStackHitRateTimeInLayer);




	/** hit-anzahl condition */
	fHHitsInX1Cut = MakeTH1('I', "HitsInLayer0Cut","Hit-Verteilung auf Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitsInX1Cut->SetFillColor(3);
	fHHitsInX1Cut->SetFillStyle(3031);
	//fHHitsInX1Cut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitsInX1Cut->GetXaxis()->CenterTitle();
	fHHitsInX1Cut->GetXaxis()->SetBinLabel(1,"X1");
	fHHitsInX1Cut->GetXaxis()->SetBinLabel(2,"U");
	fHHitsInX1Cut->GetXaxis()->SetBinLabel(3,"X2");
	fHHitsInX1Cut->GetYaxis()->SetTitle("Anzahl Hits");
	fHHitsInX1Cut->GetYaxis()->CenterTitle();

	fHHitsInUCut = MakeTH1('I', "HitsInLayer1Cut","Hit-Verteilung auf Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitsInUCut->SetFillColor(6);
	fHHitsInUCut->SetFillStyle(3031);
	//fHHitsInUCut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitsInUCut->GetXaxis()->CenterTitle();
	fHHitsInUCut->GetYaxis()->SetTitle("Anzahl Hits");
	fHHitsInUCut->GetYaxis()->CenterTitle();

	fHHitsInX2Cut = MakeTH1('I', "HitsInLayer2Cut","Hit-Verteilung auf Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitsInX2Cut->SetFillColor(7);
	fHHitsInX2Cut->SetFillStyle(3031);
	//fHHitsInX2Cut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitsInX2Cut->GetXaxis()->CenterTitle();
	fHHitsInX2Cut->GetYaxis()->SetTitle("Anzahl Hits");
	fHHitsInX2Cut->GetYaxis()->CenterTitle();

	fHStackHitsInLayerCut = new THStack("HitsInLayerStackCut","Hit-Verteilung auf Driftkammer-Ebenen - Condition angewendet");
	fHStackHitsInLayerCut->Add(fHHitsInX1Cut);
	fHStackHitsInLayerCut->Add(fHHitsInUCut);
	fHStackHitsInLayerCut->Add(fHHitsInX2Cut);

	AddObject(fHStackHitsInLayerCut);



	/** evt-rate condition */
	fHHitRateEvtInX1Cut = MakeTH1('D', "HitRateEvtInLayer0Cut","Hit-Raten (Evt) in Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitRateEvtInX1Cut->SetFillColor(3);
	fHHitRateEvtInX1Cut->SetFillStyle(3031);
	//fHHitRateEvtInX1Cut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateEvtInX1Cut->GetXaxis()->CenterTitle();
	fHHitRateEvtInX1Cut->GetXaxis()->SetBinLabel(1,"X1");
	fHHitRateEvtInX1Cut->GetXaxis()->SetBinLabel(2,"U");
	fHHitRateEvtInX1Cut->GetXaxis()->SetBinLabel(3,"X2");
	fHHitRateEvtInX1Cut->GetYaxis()->SetTitle("Rate: Hits/Evts");
	fHHitRateEvtInX1Cut->GetYaxis()->CenterTitle();

	fHHitRateEvtInUCut = MakeTH1('D', "HitRateEvtInLayer1Cut","Hit-Raten (Evt) in Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitRateEvtInUCut->SetFillColor(6);
	fHHitRateEvtInUCut->SetFillStyle(3031);
	//fHHitRateEvtInUCut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateEvtInUCut->GetXaxis()->CenterTitle();
	fHHitRateEvtInUCut->GetYaxis()->SetTitle("Rate: Hits/Evts");
	fHHitRateEvtInUCut->GetYaxis()->CenterTitle();

	fHHitRateEvtInX2Cut = MakeTH1('D', "HitRateEvtInLayer2Cut","Hit-Raten (Evt) in Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitRateEvtInX2Cut->SetFillColor(7);
	fHHitRateEvtInX2Cut->SetFillStyle(3031);
	//fHHitRateEvtInX2Cut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateEvtInX2Cut->GetXaxis()->CenterTitle();
	fHHitRateEvtInX2Cut->GetYaxis()->SetTitle("Rate: Hits/Evts");
	fHHitRateEvtInX2Cut->GetYaxis()->CenterTitle();

	fHStackHitRateEvtInLayerCut = new THStack("HitRateEvtInLayerStackCut","Hit-Raten (Evt) Driftkammer-Ebenen - Condition angewendet");
	fHStackHitRateEvtInLayerCut->Add(fHHitRateEvtInX1Cut);
	fHStackHitRateEvtInLayerCut->Add(fHHitRateEvtInUCut);
	fHStackHitRateEvtInLayerCut->Add(fHHitRateEvtInX2Cut);

	AddObject(fHStackHitRateEvtInLayerCut);


	/** zeitl. rate condition */
	fHHitRateTimeInX1Cut = MakeTH1('D', "HitRateTimeInLayer0Cut","Hit-Raten (Zeit) in Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitRateTimeInX1Cut->SetFillColor(3);
	fHHitRateTimeInX1Cut->SetFillStyle(3031);
	//fHHitRateTimeInX1Cut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateTimeInX1Cut->GetXaxis()->CenterTitle();
	fHHitRateTimeInX1Cut->GetXaxis()->SetBinLabel(1,"X1");
	fHHitRateTimeInX1Cut->GetXaxis()->SetBinLabel(2,"U");
	fHHitRateTimeInX1Cut->GetXaxis()->SetBinLabel(3,"X2");
	fHHitRateTimeInX1Cut->GetYaxis()->SetTitle("Rate: Hits/Zeit (1/s)");
	fHHitRateTimeInX1Cut->GetYaxis()->CenterTitle();

	fHHitRateTimeInUCut = MakeTH1('D', "HitRateTimeInLayer1Cut","Hit-Raten (Zeit) in Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitRateTimeInUCut->SetFillColor(6);
	fHHitRateTimeInUCut->SetFillStyle(3031);
	//fHHitRateTimeInUCut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateTimeInUCut->GetXaxis()->CenterTitle();
	fHHitRateTimeInUCut->GetYaxis()->SetTitle("Rate: Hits/Zeit (1/s)");
	fHHitRateTimeInUCut->GetYaxis()->CenterTitle();

	fHHitRateTimeInX2Cut = MakeTH1('D', "HitRateTimeInLayer2Cut","Hit-Raten (Zeit) in Driftkammer-Ebenen - Condition angewendet",
			nBinsLayer,minLayer,maxLayer);
	fHHitRateTimeInX2Cut->SetFillColor(7);
	fHHitRateTimeInX2Cut->SetFillStyle(3031);
	//fHHitRateTimeInX2Cut->GetXaxis()->SetTitle("Drahtebenen-Nummer (0->X1, 1->U, 2->X2)");
	//fHHitRateTimeInX2Cut->GetXaxis()->CenterTitle();
	fHHitRateTimeInX2Cut->GetYaxis()->SetTitle("Rate: Hits/Zeit (1/s)");
	fHHitRateTimeInX2Cut->GetYaxis()->CenterTitle();

	fHStackHitRateTimeInLayerCut = new THStack("HitRateTimeInLayerStackCut","Hit-Raten (Zeit) Driftkammer-Ebenen - Condition angewendet");
	fHStackHitRateTimeInLayerCut->Add(fHHitRateTimeInX1Cut);
	fHStackHitRateTimeInLayerCut->Add(fHHitRateTimeInUCut);
	fHStackHitRateTimeInLayerCut->Add(fHHitRateTimeInX2Cut);

	AddObject(fHStackHitRateTimeInLayerCut);






	/** - Hitpattern - */
	fHHitPatternX1 = MakeTH1('I', "HitPatternX1", "Hit-Verteilung - X1", nBinsWire_0,minWire_0,maxWire_0);
	fHHitPatternX1->GetXaxis()->SetTitle("Drahtnummer");
	fHHitPatternX1->GetXaxis()->CenterTitle();
	fHHitPatternX1->GetYaxis()->SetTitle("Anzahl Hits/Draht");
	fHHitPatternX1->GetYaxis()->CenterTitle();

	fHHitPatternU = MakeTH1('I', "HitPatternU", "Hit-Verteilung - U", nBinsWire_1,minWire_1,maxWire_1);
	fHHitPatternU->GetXaxis()->SetTitle("Drahtnummer");
	fHHitPatternU->GetXaxis()->CenterTitle();
	fHHitPatternU->GetYaxis()->SetTitle("Anzahl Hits/Draht");
	fHHitPatternU->GetYaxis()->CenterTitle();

	fHHitPatternX2 = MakeTH1('I', "HitPatternX2", "Hit-Verteilung - X2", nBinsWire_2,minWire_2,maxWire_2);
	fHHitPatternX2->GetXaxis()->SetTitle("Drahtnummer");
	fHHitPatternX2->GetXaxis()->CenterTitle();
	fHHitPatternX2->GetYaxis()->SetTitle("Anzahl Hits/Draht");
	fHHitPatternX2->GetYaxis()->CenterTitle();


	fHHitPatternX1Cut = MakeTH1('I', "HitPatternX1Cut", "Hit-Verteilung - X1 - Condition angwendet", nBinsWire_0,minWire_0,maxWire_0);
	fHHitPatternX1Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitPatternX1Cut->GetXaxis()->CenterTitle();
	fHHitPatternX1Cut->GetYaxis()->SetTitle("Anzahl Hits/Draht");
	fHHitPatternX1Cut->GetYaxis()->CenterTitle();

	fHHitPatternUCut = MakeTH1('I', "HitPatternUCut", "Hit-Verteilung - U - Condition angewendet", nBinsWire_1,minWire_1,maxWire_1);
	fHHitPatternUCut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitPatternUCut->GetXaxis()->CenterTitle();
	fHHitPatternUCut->GetYaxis()->SetTitle("Anzahl Hits/Draht");
	fHHitPatternUCut->GetYaxis()->CenterTitle();

	fHHitPatternX2Cut = MakeTH1('I', "HitPatternX2Cut", "Hit-Verteilung - X2 - Condition angewendet", nBinsWire_2,minWire_2,maxWire_2);
	fHHitPatternX2Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitPatternX2Cut->GetXaxis()->CenterTitle();
	fHHitPatternX2Cut->GetYaxis()->SetTitle("Anzahl Hits/Draht");
	fHHitPatternX2Cut->GetYaxis()->CenterTitle();


	/** - Hitraten-Evt - */
	fHHitRateEvtX1 = MakeTH1('D', "HitRateEvtX1", "Hit-Rate (Evt) - X1", nBinsWire_0,minWire_0,maxWire_0);
	fHHitRateEvtX1->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateEvtX1->GetXaxis()->CenterTitle();
	fHHitRateEvtX1->GetYaxis()->SetTitle("Rate: Hits/Evts/Draht");
	fHHitRateEvtX1->GetYaxis()->CenterTitle();

	fHHitRateEvtU = MakeTH1('D', "HitRateEvtU", "Hit-Rate (Evt) - U", nBinsWire_1,minWire_1,maxWire_1);
	fHHitRateEvtU->GetXaxis()->SetTitle("Rate: Hits/Evts/Draht");
	fHHitRateEvtU->GetXaxis()->CenterTitle();
	fHHitRateEvtU->GetYaxis()->SetTitle("Anzahl");
	fHHitRateEvtU->GetYaxis()->CenterTitle();

	fHHitRateEvtX2 = MakeTH1('D', "HitRateEvtX2", "Hit-Rate (Evt) - X2", nBinsWire_2,minWire_2,maxWire_2);
	fHHitRateEvtX2->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateEvtX2->GetXaxis()->CenterTitle();
	fHHitRateEvtX2->GetYaxis()->SetTitle("Rate: Hits/Evts/Draht");
	fHHitRateEvtX2->GetYaxis()->CenterTitle();


	fHHitRateEvtX1Cut = MakeTH1('D', "HitRateEvtX1Cut", "Hit-Rate (Evt) - X1, Condition angwendet", nBinsWire_0,minWire_0,maxWire_0);
	fHHitRateEvtX1Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateEvtX1Cut->GetXaxis()->CenterTitle();
	fHHitRateEvtX1Cut->GetYaxis()->SetTitle("Rate: Hits/Evts/Draht");
	fHHitRateEvtX1Cut->GetYaxis()->CenterTitle();

	fHHitRateEvtUCut = MakeTH1('D', "HitRateEvtUCut", "Hit-Rate (Evt) - U, Condition angewendet", nBinsWire_1,minWire_1,maxWire_1);
	fHHitRateEvtUCut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateEvtUCut->GetXaxis()->CenterTitle();
	fHHitRateEvtUCut->GetYaxis()->SetTitle("Rate: Hits/Evts/Draht");
	fHHitRateEvtUCut->GetYaxis()->CenterTitle();

	fHHitRateEvtX2Cut = MakeTH1('D', "HitRateEvtX2Cut", "Hit-Rate (Evt) - X2, Condition angewendet", nBinsWire_2,minWire_2,maxWire_2);
	fHHitRateEvtX2Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateEvtX2Cut->GetXaxis()->CenterTitle();
	fHHitRateEvtX2Cut->GetYaxis()->SetTitle("Rate: Hits/Evts/Draht");
	fHHitRateEvtX2Cut->GetYaxis()->CenterTitle();


	/** - Hitraten-Zeit - */
	fHHitRateTimeX1 = MakeTH1('D', "HitRateTimeX1", "Hit-Rate (Zeit) - X1", nBinsWire_0,minWire_0,maxWire_0);
	fHHitRateTimeX1->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateTimeX1->GetXaxis()->CenterTitle();
	fHHitRateTimeX1->GetYaxis()->SetTitle("Rate: Hits/Zeit/Draht (1/s)");
	fHHitRateTimeX1->GetYaxis()->CenterTitle();

	fHHitRateTimeU = MakeTH1('D', "HitRateTimeU", "Hit-Rate (Zeit) - U", nBinsWire_1,minWire_1,maxWire_1);
	fHHitRateTimeU->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateTimeU->GetXaxis()->CenterTitle();
	fHHitRateTimeU->GetYaxis()->SetTitle("Rate: Hits/Zeit/Draht (1/s)");
	fHHitRateTimeU->GetYaxis()->CenterTitle();

	fHHitRateTimeX2 = MakeTH1('D', "HitRateTimeX2", "Hit-Rate (Zeit) - X2", nBinsWire_2,minWire_2,maxWire_2);
	fHHitRateTimeX2->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateTimeX2->GetXaxis()->CenterTitle();
	fHHitRateTimeX2->GetYaxis()->SetTitle("Rate: Hits/Zeit/Draht (1/s)");
	fHHitRateTimeX2->GetYaxis()->CenterTitle();


	fHHitRateTimeX1Cut = MakeTH1('D', "HitRateTimeX1Cut", "Hit-Rate (Zeit) - X1, Condition angwendet", nBinsWire_0,minWire_0,maxWire_0);
	fHHitRateTimeX1Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateTimeX1Cut->GetXaxis()->CenterTitle();
	fHHitRateTimeX1Cut->GetYaxis()->SetTitle("Rate: Hits/Zeit/Draht (1/s)");
	fHHitRateTimeX1Cut->GetYaxis()->CenterTitle();

	fHHitRateTimeUCut = MakeTH1('D', "HitRateTimeUCut", "Hit-Rate (Zeit) - U, Condition angewendet", nBinsWire_1,minWire_1,maxWire_1);
	fHHitRateTimeUCut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateTimeUCut->GetXaxis()->CenterTitle();
	fHHitRateTimeUCut->GetYaxis()->SetTitle("Rate: Hits/Zeit/Draht (1/s)");
	fHHitRateTimeUCut->GetYaxis()->CenterTitle();

	fHHitRateTimeX2Cut = MakeTH1('D', "HitRateTimeX2Cut", "Hit-Rate (Zeit) - X2, Condition angewendet", nBinsWire_2,minWire_2,maxWire_2);
	fHHitRateTimeX2Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHHitRateTimeX2Cut->GetXaxis()->CenterTitle();
	fHHitRateTimeX2Cut->GetYaxis()->SetTitle("Rate: Hits/Zeit/Draht (1/s)");
	fHHitRateTimeX2Cut->GetYaxis()->CenterTitle();





	/** Drifttime-Dist */
	fHDrifttimeDist = MakeTH1('D', "DrifttimeDist", "Driftzeitverteilung  - Alle Ebenen", 1300, -100, 1200);
	fHDrifttimeDist->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDist->GetXaxis()->CenterTitle();
	fHDrifttimeDist->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDist->GetYaxis()->CenterTitle();

	fHDrifttimeDistX1 = MakeTH1('D', "DrifttimeDistX1", "Driftzeitverteilung - X1", 1300, -100, 1200);
	fHDrifttimeDistX1->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistX1->GetXaxis()->CenterTitle();
	fHDrifttimeDistX1->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistX1->GetYaxis()->CenterTitle();

	fHDrifttimeDistU = MakeTH1('D', "DrifttimeDistU", "Driftzeitverteilung - U", 1300, -100, 1200);
	fHDrifttimeDistU->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistU->GetXaxis()->CenterTitle();
	fHDrifttimeDistU->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistU->GetYaxis()->CenterTitle();

	fHDrifttimeDistX2 = MakeTH1('D', "DrifttimeDistX2", "Driftzeitverteilung - X2", 1300, -100, 1200);
	fHDrifttimeDistX2->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistX2->GetXaxis()->CenterTitle();
	fHDrifttimeDistX2->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistX2->GetYaxis()->CenterTitle();


	((*fCondArrDrifttime)[0])->SetHistogram("DrifttimeDistX1");
	((*fCondArrDrifttime)[1])->SetHistogram("DrifttimeDistU");
	((*fCondArrDrifttime)[2])->SetHistogram("DrifttimeDistX2");
	/*
	fWinCondDrifttime->SetHistogram("DrifttimeDistX1");
	fWinCondDrifttime->SetHistogram("DrifttimeDistU");
	fWinCondDrifttime->SetHistogram("DrifttimeDistX2");
	*/



	/** ToT-Dist */
	fHToTDist = MakeTH1('D', "ToTDist", "Time-over-Threshold Verteilung - Alle Ebenen", 1300, -100, 1200);
	fHToTDist->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDist->GetXaxis()->CenterTitle();
	fHToTDist->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDist->GetYaxis()->CenterTitle();

	fHToTDistX1 = MakeTH1('D', "ToTDistX1", "Time-over-Threshold Verteilung - X1", 1300, -100, 1200);
	fHToTDistX1->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDistX1->GetXaxis()->CenterTitle();
	fHToTDistX1->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDistX1->GetYaxis()->CenterTitle();

	fHToTDistU = MakeTH1('D', "ToTDistU", "Time-over-Threshold Verteilung - U", 1300, -100, 1200);
	fHToTDistU->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDistU->GetXaxis()->CenterTitle();
	fHToTDistU->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDistU->GetYaxis()->CenterTitle();

	fHToTDistX2 = MakeTH1('D', "ToTDistX2", "Time-over-Threshold Verteilung - X2", 1300, -100, 1200);
	fHToTDistX2->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDistX2->GetXaxis()->CenterTitle();
	fHToTDistX2->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDistX2->GetYaxis()->CenterTitle();


	((*fCondArrToT)[0])->SetHistogram("ToTDistX1");
	((*fCondArrToT)[1])->SetHistogram("ToTDistU");
	((*fCondArrToT)[2])->SetHistogram("ToTDistX2");
	/*
	fWinCondToT->SetHistogram("ToTDistX1");
	fWinCondToT->SetHistogram("ToTDistU");
	fWinCondToT->SetHistogram("ToTDistX2");
	*/


	/** Drifttime-Dist nach Cuts */
	fHDrifttimeDistCut = MakeTH1('D', "DrifttimeDistCut", "Driftzeitverteilung - Alle Ebenen, Condition angewendet",
			nBinsDrifttime,minDrifttime,maxDrifttime);
	fHDrifttimeDistCut->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistCut->GetXaxis()->CenterTitle();
	fHDrifttimeDistCut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistCut->GetYaxis()->CenterTitle();

	fHDrifttimeDistX1Cut = MakeTH1('D', "DrifttimeDistX1Cut", "Driftzeitverteilung - X1, Condition angewendet",
			nBinsDrifttime_0,minDrifttime_0,maxDrifttime_0);
	fHDrifttimeDistX1Cut->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistX1Cut->GetXaxis()->CenterTitle();
	fHDrifttimeDistX1Cut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistX1Cut->GetYaxis()->CenterTitle();

	fHDrifttimeDistUCut = MakeTH1('D', "DrifttimeDistUCut", "Driftzeitverteilung - U, Condition angewendet",
			nBinsDrifttime_1,minDrifttime_1,maxDrifttime_1);
	fHDrifttimeDistUCut->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistUCut->GetXaxis()->CenterTitle();
	fHDrifttimeDistUCut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistUCut->GetYaxis()->CenterTitle();

	fHDrifttimeDistX2Cut = MakeTH1('D', "DrifttimeDistX2Cut", "Driftzeitverteilung - X2, Condition angewendet",
			nBinsDrifttime_2,minDrifttime_2,maxDrifttime_2);
	fHDrifttimeDistX2Cut->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistX2Cut->GetXaxis()->CenterTitle();
	fHDrifttimeDistX2Cut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistX2Cut->GetYaxis()->CenterTitle();



	/** Drifttime-Dist nach Time-Walk Korrektur */
	fHDrifttimeDistTimeWalkCorr = MakeTH1('D', "DrifttimeDistTimeWalkCorr", "Driftzeitverteilung - Alle Ebenen, Time-Walk korrigiert",
			nBinsDrifttime,minDrifttime,maxDrifttime);
	fHDrifttimeDistTimeWalkCorr->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistTimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeDistTimeWalkCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistTimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeDistX1TimeWalkCorr = MakeTH1('D', "DrifttimeDistX1TimeWalkCorr", "Driftzeitverteilung - X1, Time-Walk korrigiert",
			nBinsDrifttime_0,minDrifttime_0,maxDrifttime_0);
	fHDrifttimeDistX1TimeWalkCorr->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistX1TimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeDistX1TimeWalkCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistX1TimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeDistUTimeWalkCorr = MakeTH1('D', "DrifttimeDistUTimeWalkCorr", "Driftzeitverteilung - U, Time-Walk korrigiert",
			nBinsDrifttime_1,minDrifttime_1,maxDrifttime_1);
	fHDrifttimeDistUTimeWalkCorr->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistUTimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeDistUTimeWalkCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistUTimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeDistX2TimeWalkCorr = MakeTH1('D', "DrifttimeDistX2TimeWalkCorr", "Driftzeitverteilung - X2, Time-Walk korrigiert",
			nBinsDrifttime_2,minDrifttime_2,maxDrifttime_2);
	fHDrifttimeDistX2TimeWalkCorr->GetXaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeDistX2TimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeDistX2TimeWalkCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHDrifttimeDistX2TimeWalkCorr->GetYaxis()->CenterTitle();



	/** ToT-Dist nach Cuts */
	fHToTDistCut = MakeTH1('D', "ToTDistCut", "Time-over-Threshold Verteilung - Alle Ebenen, Condition angewendet",
			nBinsToT,minToT,maxToT);
	fHToTDistCut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDistCut->GetXaxis()->CenterTitle();
	fHToTDistCut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDistCut->GetYaxis()->CenterTitle();

	fHToTDistX1Cut = MakeTH1('D', "ToTDistX1Cut", "Time-over-Threshold Verteilung - X1, Condition angewendet",
			nBinsToT_0,minToT_0,maxToT_0);
	fHToTDistX1Cut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDistX1Cut->GetXaxis()->CenterTitle();
	fHToTDistX1Cut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDistX1Cut->GetYaxis()->CenterTitle();

	fHToTDistUCut = MakeTH1('D', "ToTDistUCut", "Time-over-Threshold Verteilung - U, Condition angewendet",
			nBinsToT_1,minToT_1,maxToT_1);
	fHToTDistUCut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDistUCut->GetXaxis()->CenterTitle();
	fHToTDistUCut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDistUCut->GetYaxis()->CenterTitle();

	fHToTDistX2Cut = MakeTH1('D', "ToTDistX2Cut", "Time-over-Threshold Verteilung - X2, Condition angewendet",
			nBinsToT_2,minToT_2,maxToT_2);
	fHToTDistX2Cut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTDistX2Cut->GetXaxis()->CenterTitle();
	fHToTDistX2Cut->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHToTDistX2Cut->GetYaxis()->CenterTitle();



	/** drifttime vs wire */
	fHDrifttimeVsWireX1 = MakeTH2('D',"DrifttimeVsWireX1","Driftzeitverteilung je Draht - X1",
			nBinsWire_0,minWire_0,maxWire_0,1300,-100,1200);
	fHDrifttimeVsWireX1->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireX1->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireX1->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireX1->GetYaxis()->CenterTitle();

	fHDrifttimeVsWireU = MakeTH2('D',"DrifttimeVsWireU","Driftzeitverteilung je Draht  - U",
			nBinsWire_1,minWire_1,maxWire_1,1300,-100,1200);
	fHDrifttimeVsWireU->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireU->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireU->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireU->GetYaxis()->CenterTitle();

	fHDrifttimeVsWireX2 = MakeTH2('D',"DrifttimeVsWireX2","Driftzeitverteilung je Draht - X2",
			nBinsWire_2,minWire_2,maxWire_2,1300,-100,1200);
	fHDrifttimeVsWireX2->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireX2->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireX2->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireX2->GetYaxis()->CenterTitle();


	/*
	fWinCondDrifttime->SetHistogram("DrifttimeVsWireX1");
	fWinCondDrifttime->SetHistogram("DrifttimeVsWireU");
	fWinCondDrifttime->SetHistogram("DrifttimeVsWireX2");
	*/



	/** drifttime vs wire nach Cuts */
	fHDrifttimeVsWireX1Cut = MakeTH2('D',"DrifttimeVsWireX1Cut","Driftzeitverteilung je Draht - X1, Condition angewendet",
			nBinsWire_0,minWire_0,maxWire_0,nBinsDrifttime_0,minDrifttime_0,maxDrifttime_0);
	fHDrifttimeVsWireX1Cut->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireX1Cut->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireX1Cut->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireX1Cut->GetYaxis()->CenterTitle();

	fHDrifttimeVsWireUCut = MakeTH2('D',"DrifttimeVsWireUCut","Driftzeitverteilung je Draht - U, Condition angewendet",
			nBinsWire_1,minWire_1,maxWire_1,nBinsDrifttime_1,minDrifttime_1,maxDrifttime_1);
	fHDrifttimeVsWireUCut->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireUCut->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireUCut->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireUCut->GetYaxis()->CenterTitle();

	fHDrifttimeVsWireX2Cut = MakeTH2('D',"DrifttimeVsWireX2Cut","Driftzeitverteilung je Draht - X2, Condition angewendet",
			nBinsWire_2,minWire_2,maxWire_2,nBinsDrifttime_2,minDrifttime_2,maxDrifttime_2);
	fHDrifttimeVsWireX2Cut->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireX2Cut->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireX2Cut->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireX2Cut->GetYaxis()->CenterTitle();



	/** drifttime vs wire nach timewalk-korrektur */
	fHDrifttimeVsWireX1TimeWalkCorr = MakeTH2('D',"DrifttimeVsWireX1TimeWalkCorr","Driftzeitverteilung je Draht - X1, Time-Walk korrigiert",
			nBinsWire_0,minWire_0,maxWire_0,nBinsDrifttime_0,minDrifttime_0,maxDrifttime_0);
	fHDrifttimeVsWireX1TimeWalkCorr->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireX1TimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireX1TimeWalkCorr->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireX1TimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeVsWireUTimeWalkCorr = MakeTH2('D',"DrifttimeVsWireUTimeWalkCorr","Driftzeitverteilung je Draht - U, Time-Walk korrigiert",
			nBinsWire_1,minWire_1,maxWire_1,nBinsDrifttime_1,minDrifttime_1,maxDrifttime_1);
	fHDrifttimeVsWireUTimeWalkCorr->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireUTimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireUTimeWalkCorr->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireUTimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeVsWireX2TimeWalkCorr = MakeTH2('D',"DrifttimeVsWireX2TimeWalkCorr","Driftzeitverteilung je Draht - X2, Time-Walk korrigiert",
			nBinsWire_2,minWire_2,maxWire_2,nBinsDrifttime_2,minDrifttime_2,maxDrifttime_2);
	fHDrifttimeVsWireX2TimeWalkCorr->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHDrifttimeVsWireX2TimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeVsWireX2TimeWalkCorr->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsWireX2TimeWalkCorr->GetYaxis()->CenterTitle();



	/** tot vs wire */
	fHToTVsWireX1 = MakeTH2('D',"ToTVsWireX1","ToT-Verteilung je Draht - X1",
			nBinsWire_0,minWire_0,maxWire_0,1300,-100,1200);
	fHToTVsWireX1->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHToTVsWireX1->GetXaxis()->CenterTitle();
	fHToTVsWireX1->GetYaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTVsWireX1->GetYaxis()->CenterTitle();

	fHToTVsWireU = MakeTH2('D',"ToTVsWireU","ToT-Verteilung je Draht - U",
			nBinsWire_1,minWire_1,maxWire_1,1300,-100,1200);
	fHToTVsWireU->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHToTVsWireU->GetXaxis()->CenterTitle();
	fHToTVsWireU->GetYaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTVsWireU->GetYaxis()->CenterTitle();

	fHToTVsWireX2 = MakeTH2('D',"ToTVsWireX2","ToT-Verteilung je Draht - X2",
			nBinsWire_2,minWire_2,maxWire_2,1300,-100,1200);
	fHToTVsWireX2->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHToTVsWireX2->GetXaxis()->CenterTitle();
	fHToTVsWireX2->GetYaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTVsWireX2->GetYaxis()->CenterTitle();


	/*
	fWinCondToT->SetHistogram("ToTVsWireX1");
	fWinCondToT->SetHistogram("ToTVsWireU");
	fWinCondToT->SetHistogram("ToTVsWireX2");
	*/



	/** tot vs wire nach Cuts */
	fHToTVsWireX1Cut = MakeTH2('D',"ToTVsWireX1Cut","ToT-Verteilung je Draht - X1, Condition angewendet",
			nBinsWire_0,minWire_0,maxWire_0,nBinsToT_0,minToT_0,maxToT_0);
	fHToTVsWireX1Cut->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHToTVsWireX1Cut->GetXaxis()->CenterTitle();
	fHToTVsWireX1Cut->GetYaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTVsWireX1Cut->GetYaxis()->CenterTitle();

	fHToTVsWireUCut = MakeTH2('D',"ToTVsWireUCut","ToT-Verteilung je Draht - U, Condition angewendet",
			nBinsWire_1,minWire_1,maxWire_1,nBinsToT_1,minToT_1,maxToT_1);
	fHToTVsWireUCut->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHToTVsWireUCut->GetXaxis()->CenterTitle();
	fHToTVsWireUCut->GetYaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTVsWireUCut->GetYaxis()->CenterTitle();

	fHToTVsWireX2Cut = MakeTH2('D',"ToTVsWireX2Cut","ToT-Verteilung je Draht - X2, Condition angewendet",
			nBinsWire_2,minWire_2,maxWire_2,nBinsToT_2,minToT_2,maxToT_2);
	fHToTVsWireX2Cut->GetXaxis()->SetTitle("Drahtnummer/(1ns)");
	fHToTVsWireX2Cut->GetXaxis()->CenterTitle();
	fHToTVsWireX2Cut->GetYaxis()->SetTitle("Time-over-Threshold/ns");
	fHToTVsWireX2Cut->GetYaxis()->CenterTitle();



	/** drifttime vs tot */
	fHDrifttimeVsToT = MakeTH2('D', "DrifttimeVsToT","Driftzeit versus Time-over-Threshold - Alle Ebenen",1300,-100,1200,1300,-100,1200);
	fHDrifttimeVsToT->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToT->GetXaxis()->CenterTitle();
	fHDrifttimeVsToT->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToT->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTX1 = MakeTH2('D', "DrifttimeVsToTX1","Driftzeit versus Time-over-Threshold - X1",1300,-100,1200,1300,-100,1200);
	fHDrifttimeVsToTX1->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTX1->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTX1->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTX1->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTU = MakeTH2('D', "DrifttimeVsToTU","Driftzeit versus Time-over-Threshold - U",1300,-100,1200,1300,-100,1200);
	fHDrifttimeVsToTU->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTU->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTU->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTU->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTX2 = MakeTH2('D', "DrifttimeVsToTX2","Driftzeit versus Time-over-Threshold - X2",1300,-100,1200,1300,-100,1200);
	fHDrifttimeVsToTX2->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTX2->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTX2->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTX2->GetYaxis()->CenterTitle();


	((*fCondArrDrifttimeVsToT)[0])->SetHistogram("DrifttimeVsToTX1");
	((*fCondArrDrifttimeVsToT)[1])->SetHistogram("DrifttimeVsToTU");
	((*fCondArrDrifttimeVsToT)[2])->SetHistogram("DrifttimeVsToTX2");



	/** tot vs drifttime nach Cuts */
	fHDrifttimeVsToTCut = MakeTH2('D', "DrifttimeVsToTCut","Driftzeit versus Time-over-Threshold - AlleEbenen, Condition angewendet",
			nBinsToT,minToT,maxToT,nBinsDrifttime,minDrifttime,maxDrifttime);
	fHDrifttimeVsToTCut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTCut->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTCut->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTCut->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTX1Cut = MakeTH2('D', "DrifttimeVsToTX1Cut","Driftzeit versus Time-over-Threshold - X1, Condition angewendet",
			nBinsToT_0,minToT_0,maxToT_0,nBinsDrifttime_0,minDrifttime_0,maxDrifttime_0);
	fHDrifttimeVsToTX1Cut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTX1Cut->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTX1Cut->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTX1Cut->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTUCut = MakeTH2('D', "DrifttimeVsToTUCut","Driftzeit versus Time-over-Threshold - U, Condition angewendet",
			nBinsToT_1,minToT_1,maxToT_1,nBinsDrifttime_1,minDrifttime_1,maxDrifttime_1);
	fHDrifttimeVsToTUCut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTUCut->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTUCut->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTUCut->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTX2Cut = MakeTH2('D', "DrifttimeVsToTX2Cut","Driftzeit versus Time-over-Threshold - X2, Condition angewendet",
			nBinsToT_2,minToT_2,maxToT_2,nBinsDrifttime_2,minDrifttime_2,maxDrifttime_2);
	fHDrifttimeVsToTX2Cut->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTX2Cut->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTX2Cut->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTX2Cut->GetYaxis()->CenterTitle();



	/** tot vs drifttime nach Time-Walk Korrektur */
	fHDrifttimeVsToTTimeWalkCorr = MakeTH2('D', "HDrifttimeVsToTTimeWalkCorr","Driftzeit versus Time-over-Threshold - Alle Ebenen, Time-Walk korrigiert",
			nBinsToT,minToT,maxToT,nBinsDrifttime,minDrifttime,maxDrifttime);
	fHDrifttimeVsToTTimeWalkCorr->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTTimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTTimeWalkCorr->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTTimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTX1TimeWalkCorr = MakeTH2('D', "DrifttimeVsToTX1TimeWalkCorr","Driftzeit versus Time-over-Threshold - X1, Time-Walk korrigiert",
			nBinsToT_0,minToT_0,maxToT_0,nBinsDrifttime_0,minDrifttime_0,maxDrifttime_0);
	fHDrifttimeVsToTX1TimeWalkCorr->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTX1TimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTX1TimeWalkCorr->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTX1TimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTUTimeWalkCorr = MakeTH2('D', "DrifttimeVsToTUTimeWalkCorr","Driftzeit versus Time-over-Threshold - U, Time-Walk korrigiert",
			nBinsToT_1,minToT_1,maxToT_1,nBinsDrifttime_1,minDrifttime_1,maxDrifttime_1);
	fHDrifttimeVsToTUTimeWalkCorr->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTUTimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTUTimeWalkCorr->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTUTimeWalkCorr->GetYaxis()->CenterTitle();

	fHDrifttimeVsToTX2TimeWalkCorr = MakeTH2('D', "DrifttimeVsToTX2TimeWalkCorr","Driftzeit versus Time-over-Threshold - X2, Time-Walk korrigiert",
			nBinsToT_2,minToT_2,maxToT_2,nBinsDrifttime_2,minDrifttime_2,maxDrifttime_2);
	fHDrifttimeVsToTX2TimeWalkCorr->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	fHDrifttimeVsToTX2TimeWalkCorr->GetXaxis()->CenterTitle();
	fHDrifttimeVsToTX2TimeWalkCorr->GetYaxis()->SetTitle("Driftzeit/ns");
	fHDrifttimeVsToTX2TimeWalkCorr->GetYaxis()->CenterTitle();



	//-----------------------------------------------------------
	// 4. initialisiere RollingGraph
	fGFaradayCupCurrent = MakeRollingGraph("FaradayCupCurrent", "Faraday Cup-Strom (Zoom)", 1000);//, 100);
	fGFaradayCupCurrent->SetLineColor(kGray);
	fGFaradayCupCurrent->SetLineWidth(2);
	fGFaradayCupCurrent->SetLineStyle(1);
	fGFaradayCupCurrent->SetMarkerStyle(0);
	fGFaradayCupCurrent->GetXaxis()->SetTitle("a.u.");
	fGFaradayCupCurrent->GetXaxis()->CenterTitle();
	fGFaradayCupCurrent->GetYaxis()->SetTitle("Strom/nA");
	fGFaradayCupCurrent->GetYaxis()->CenterTitle();

	fGSzintUpTrigRate = MakeRollingGraph("SzintillatorPmtUp", "QClam Trigger-Raten (Zoom)", 1000);
	fGSzintUpTrigRate->SetLineColor(kCyan);
	fGSzintUpTrigRate->SetLineWidth(2);
	fGSzintUpTrigRate->SetLineStyle(1);
	fGSzintUpTrigRate->SetMarkerStyle(0);
	fGSzintUpTrigRate->GetXaxis()->SetTitle("a.u.");
	fGSzintUpTrigRate->GetXaxis()->CenterTitle();
	fGSzintUpTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGSzintUpTrigRate->GetYaxis()->CenterTitle();

	fGSzintDownTrigRate = MakeRollingGraph("SzintillatorPmtDown", "QClam Trigger-Raten (Zoom)", 1000);
	fGSzintDownTrigRate->SetLineColor(kBlue);
	fGSzintDownTrigRate->SetLineWidth(2);
	fGSzintDownTrigRate->SetLineStyle(1);
	fGSzintDownTrigRate->SetMarkerStyle(0);
	fGSzintDownTrigRate->GetXaxis()->SetTitle("a.u.");
	fGSzintDownTrigRate->GetXaxis()->CenterTitle();
	fGSzintDownTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGSzintDownTrigRate->GetYaxis()->CenterTitle();

	fGCherenkovTrigRate = MakeRollingGraph("CherenkovPmtTrigger", "QClam Trigger-Raten (Zoom)", 1000);
	fGCherenkovTrigRate->SetLineColor(kOrange);
	fGCherenkovTrigRate->SetLineWidth(2);
	fGCherenkovTrigRate->SetLineStyle(1);
	fGCherenkovTrigRate->SetMarkerStyle(0);
	fGCherenkovTrigRate->GetXaxis()->SetTitle("a.u.");
	fGCherenkovTrigRate->GetXaxis()->CenterTitle();
	fGCherenkovTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGCherenkovTrigRate->GetYaxis()->CenterTitle();

	fGCoincTrigRate = MakeRollingGraph("CoincidenceTrigger", "QClam Trigger-Raten (Zoom)", 1000);
	fGCoincTrigRate->SetLineColor(kViolet);
	fGCoincTrigRate->SetLineWidth(2);
	fGCoincTrigRate->SetLineStyle(1);
	fGCoincTrigRate->SetMarkerStyle(0);
	fGCoincTrigRate->GetXaxis()->SetTitle("a.u.");
	fGCoincTrigRate->GetXaxis()->CenterTitle();
	fGCoincTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGCoincTrigRate->GetYaxis()->CenterTitle();

	fGAccTrigRate = MakeRollingGraph("AcceptedTrigger", "QClam Trigger-Raten (Zoom)", 1000);
	fGAccTrigRate->SetLineColor(kGreen);
	fGAccTrigRate->SetLineWidth(2);
	fGAccTrigRate->SetLineStyle(1);
	fGAccTrigRate->SetMarkerStyle(0);
	fGAccTrigRate->GetXaxis()->SetTitle("a.u.");
	fGAccTrigRate->GetXaxis()->CenterTitle();
	fGAccTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGAccTrigRate->GetYaxis()->CenterTitle();

	fGRelDeadtime = MakeRollingGraph("RelDeadtime", "QClam Relative Totzeit (Zoom)", 1000);
	fGRelDeadtime->SetLineColor(kGreen);
	fGRelDeadtime->SetLineWidth(2);
	fGRelDeadtime->SetLineStyle(1);
	fGRelDeadtime->SetMarkerStyle(0);
	fGRelDeadtime->GetXaxis()->SetTitle("a.u.");
	fGRelDeadtime->GetXaxis()->CenterTitle();
	fGRelDeadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRelDeadtime->GetYaxis()->CenterTitle();

	fGRelMbsDeadtime = MakeRollingGraph("RelMbsDeadtime", "QClam Mbs-Totzeit (Zoom)", 1000);
	fGRelMbsDeadtime->SetLineColor(kGreen);
	fGRelMbsDeadtime->SetLineWidth(2);
	fGRelMbsDeadtime->SetLineStyle(1);
	fGRelMbsDeadtime->SetMarkerStyle(0);
	fGRelMbsDeadtime->GetXaxis()->SetTitle("a.u.");
	fGRelMbsDeadtime->GetXaxis()->CenterTitle();
	fGRelMbsDeadtime->GetYaxis()->SetTitle("Relative Mbs-Totzeit: 1 - (Live/Total)");
	fGRelMbsDeadtime->GetYaxis()->CenterTitle();


	/// GRAPHEN

	fGraphFaradayCupCurrent = new TGraph();
	fGraphFaradayCupCurrent->SetName("FaradayCupCurrentGraph");
	fGraphFaradayCupCurrent->SetTitle("Faraday Cup Current");
	fGraphFaradayCupCurrent->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphFaradayCupCurrent->GetYaxis()->SetTitle("Strom / nA");
	fGraphFaradayCupCurrent->SetLineColor(kGray);
	fGraphFaradayCupCurrent->SetLineWidth(2);
	fGraphFaradayCupCurrent->SetLineStyle(1);
	fGraphFaradayCupCurrent->SetMarkerStyle(0);
	AddObject(fGraphFaradayCupCurrent);

	fGraphScalerTriggerRate = new TGraph();
	fGraphScalerTriggerRate->SetName("ScalerTriggerRateGraph");
	fGraphScalerTriggerRate->SetTitle("ScalerTriggerRate");
	fGraphScalerTriggerRate->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphScalerTriggerRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGraphScalerTriggerRate->SetLineColor(kRed);
	fGraphScalerTriggerRate->SetLineWidth(2);
	fGraphScalerTriggerRate->SetLineStyle(1);
	fGraphScalerTriggerRate->SetMarkerStyle(0);
	AddObject(fGraphScalerTriggerRate);

	fGraphSzintUpTrigRate = new TGraph();
	fGraphSzintUpTrigRate->SetName("SzintUpTrigRateGraph");
	fGraphSzintUpTrigRate->SetTitle("SzintUp Trigger Rate");
	fGraphSzintUpTrigRate->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphSzintUpTrigRate->GetXaxis()->CenterTitle();
	fGraphSzintUpTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGraphSzintUpTrigRate->GetYaxis()->CenterTitle();
	fGraphSzintUpTrigRate->SetLineColor(kCyan);
	fGraphSzintUpTrigRate->SetLineWidth(2);
	fGraphSzintUpTrigRate->SetLineStyle(1);
	fGraphSzintUpTrigRate->SetMarkerStyle(0);
	AddObject(fGraphSzintUpTrigRate);

	fGraphSzintDownTrigRate = new TGraph();
	fGraphSzintDownTrigRate->SetName("SzintDownTrigRateGraph");
	fGraphSzintDownTrigRate->SetTitle("SzintDown Trigger Rate");
	fGraphSzintDownTrigRate->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphSzintDownTrigRate->GetXaxis()->CenterTitle();
	fGraphSzintDownTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGraphSzintDownTrigRate->GetYaxis()->CenterTitle();
	fGraphSzintDownTrigRate->SetLineColor(kBlue);
	fGraphSzintDownTrigRate->SetLineWidth(2);
	fGraphSzintDownTrigRate->SetLineStyle(1);
	fGraphSzintDownTrigRate->SetMarkerStyle(0);
	AddObject(fGraphSzintDownTrigRate);

	fGraphCherenkovTrigRate = new TGraph();
	fGraphCherenkovTrigRate->SetName("CherenkovTrigRateGraph");
	fGraphCherenkovTrigRate->SetTitle("Cherenkov Trigger Rate");
	fGraphCherenkovTrigRate->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphCherenkovTrigRate->GetXaxis()->CenterTitle();
	fGraphCherenkovTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGraphCherenkovTrigRate->GetYaxis()->CenterTitle();
	fGraphCherenkovTrigRate->SetLineColor(kOrange);
	fGraphCherenkovTrigRate->SetLineWidth(2);
	fGraphCherenkovTrigRate->SetLineStyle(1);
	fGraphCherenkovTrigRate->SetMarkerStyle(0);
	AddObject(fGraphCherenkovTrigRate);

	fGraphCoincTrigRate = new TGraph();
	fGraphCoincTrigRate->SetName("CoincTrigRateGraph");
	fGraphCoincTrigRate->SetTitle("Coinc Trigger Rate");
	fGraphCoincTrigRate->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphCoincTrigRate->GetXaxis()->CenterTitle();
	fGraphCoincTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGraphCoincTrigRate->GetYaxis()->CenterTitle();
	fGraphCoincTrigRate->SetLineColor(kViolet);
	fGraphCoincTrigRate->SetLineWidth(2);
	fGraphCoincTrigRate->SetLineStyle(1);
	fGraphCoincTrigRate->SetMarkerStyle(0);
	AddObject(fGraphCoincTrigRate);

	fGraphAccTrigRate = new TGraph();
	fGraphAccTrigRate->SetName("AccTrigRateGraph");
	fGraphAccTrigRate->SetTitle("AccTrigger Rate");
	fGraphAccTrigRate->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphAccTrigRate->GetXaxis()->CenterTitle();
	fGraphAccTrigRate->GetYaxis()->SetTitle("Rate/(1/s)");
	fGraphAccTrigRate->GetYaxis()->CenterTitle();
	fGraphAccTrigRate->SetLineColor(kGreen);
	fGraphAccTrigRate->SetLineWidth(2);
	fGraphAccTrigRate->SetLineStyle(1);
	fGraphAccTrigRate->SetMarkerStyle(0);
	AddObject(fGraphAccTrigRate);

	fGraphRelDeadtime = new TGraph();
	fGraphRelDeadtime->SetName("RelDeadtimeGraph");
	fGraphRelDeadtime->SetTitle("Relative Totzeit");
	fGraphRelDeadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphRelDeadtime->GetXaxis()->CenterTitle();
	fGraphRelDeadtime->GetYaxis()->SetTitle("Rel. Totzeit: 1 - (Live/Total)");
	fGraphRelDeadtime->GetYaxis()->CenterTitle();
	fGraphRelDeadtime->SetLineColor(kGreen);
	fGraphRelDeadtime->SetLineWidth(2);
	fGraphRelDeadtime->SetLineStyle(1);
	fGraphRelDeadtime->SetMarkerStyle(0);
	AddObject(fGraphRelDeadtime);

	fGraphRelMbsDeadtime = new TGraph();
	fGraphRelMbsDeadtime->SetName("RelMbsDeadtimeGraph");
	fGraphRelMbsDeadtime->SetTitle("Relative Mbs Totzeit");
	fGraphRelMbsDeadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGraphRelMbsDeadtime->GetXaxis()->CenterTitle();
	fGraphRelMbsDeadtime->GetYaxis()->SetTitle("Rel. Totzeit: 1 - (Live/Total)");
	fGraphRelMbsDeadtime->GetYaxis()->CenterTitle();
	fGraphRelMbsDeadtime->SetLineColor(kGreen);
	fGraphRelMbsDeadtime->SetLineWidth(2);
	fGraphRelMbsDeadtime->SetLineStyle(1);
	fGraphRelMbsDeadtime->SetMarkerStyle(0);
	AddObject(fGraphRelMbsDeadtime);






	/** TODO? */
	fGScalerTriggerRate = MakeRollingGraph("ScalerTriggerRate", "Scaler Trigger Rate/(1/ms)", 100000);//, 100);
	



	//-----------------------------------------------------------
	// 5. initialisiere Pictures
	fPRateMon = GetPicture("QClamRateMonitor");
	if(!fPRateMon) // TODO
	{
		fPRateMon = new TGo4Picture("QClamRateMonitor", "QClam Raten Monitor");
		fPRateMon->SetDivision(2,3);
		fPRateMon->Pic(0,0)->AddObject(fGFaradayCupCurrent);
		fPRateMon->Pic(1,0)->AddObject(fGraphFaradayCupCurrent);
		
		fPRateMon->Pic(0,1)->AddObject(fGSzintUpTrigRate);
		fPRateMon->Pic(0,1)->AddObject(fGSzintDownTrigRate);
		fPRateMon->Pic(0,1)->AddObject(fGCherenkovTrigRate);
		fPRateMon->Pic(0,1)->AddObject(fGCoincTrigRate);
		fPRateMon->Pic(0,1)->AddObject(fGAccTrigRate);
		
		fPRateMon->Pic(1,1)->AddObject(fGraphSzintUpTrigRate);
		fPRateMon->Pic(1,1)->AddObject(fGraphSzintDownTrigRate);
		fPRateMon->Pic(1,1)->AddObject(fGraphCherenkovTrigRate);
		fPRateMon->Pic(1,1)->AddObject(fGraphCoincTrigRate);
		fPRateMon->Pic(1,1)->AddObject(fGraphAccTrigRate);


		fPRateMon->Pic(0,2)->AddObject(fGRelDeadtime);
		fPRateMon->Pic(0,2)->AddObject(fGRelMbsDeadtime);

		fPRateMon->Pic(1,2)->AddObject(fGraphRelDeadtime);
		fPRateMon->Pic(1,2)->AddObject(fGraphRelMbsDeadtime);
		/*
		fPScalerMon->Pic(0,0)->AddText(0.05,0.95,"Scaler Values");
		fPScalerMon->Pic(0,1)->AddText(0.05,0.95,"Mean Trigger Values");
		*/

		AddPicture(fPRateMon);
	}


	fPTriggerDetectors = GetPicture("TriggerDetectorTime");
	if(!fPTriggerDetectors)
	{
		fPTriggerDetectors = new TGo4Picture("TriggerDetectorTime","Zeiten Trigger Detektoren");
		fPTriggerDetectors->SetDivision(2,3);
		fPTriggerDetectors->Pic(0,0)->AddObject(fHSzintUpTime);
		fPTriggerDetectors->Pic(1,0)->AddObject(fHSzintDownTime);
		fPTriggerDetectors->Pic(0,1)->AddObject(fHMeanSzintTime);
		fPTriggerDetectors->Pic(1,1)->AddObject(fHDiffSzintTime);
		fPTriggerDetectors->Pic(0,2)->AddObject(fHCherenkovTime);
		fPTriggerDetectors->Pic(1,2)->AddObject(fHSzintUpVsSzintDown);

		AddPicture(fPTriggerDetectors);
	}

	/** Statistik/Raten ganze Ebene */
	fPLayer_Rates_Statistics = GetPicture("Layer_Rates_Statistics");
	if(!fPLayer_Rates_Statistics)
	{
		fPLayer_Rates_Statistics = new TGo4Picture("Layer_Rates_Statistics", "Statistiken & Raten gesamte Driftkammerebenen");
		fPLayer_Rates_Statistics->SetDivision(2,3);

		/*
		fPLayer_Rates_Statistics->Pic(0,0)->AddObject(fHHitsInX1);
		fPLayer_Rates_Statistics->Pic(0,0)->SetFillAtt(3,3031);
//		fPLayer_Rates_Statistics->Pic(0,0)->SetLineAtt(3,1,1);
		fPLayer_Rates_Statistics->Pic(0,0)->AddObject(fHHitsInU);
		fPLayer_Rates_Statistics->Pic(0,0)->SetFillAtt(6,3031);
//		fPLayer_Rates_Statistics->Pic(0,0)->SetLineAtt(6,1,1);
		fPLayer_Rates_Statistics->Pic(0,0)->AddObject(fHHitsInX2);
		fPLayer_Rates_Statistics->Pic(0,0)->SetFillAtt(7,3031);
//		fPLayer_Rates_Statistics->Pic(0,0)->SetLineAtt(7,1,1);
//		*/

		fPLayer_Rates_Statistics->Pic(0,0)->AddHStack(fHStackHitsInLayer,"hist");
		fPLayer_Rates_Statistics->Pic(0,1)->AddHStack(fHStackHitRateTimeInLayer,"hist");
		fPLayer_Rates_Statistics->Pic(0,2)->AddHStack(fHStackHitRateEvtInLayer,"hist");
		fPLayer_Rates_Statistics->Pic(1,0)->AddHStack(fHStackHitsInLayerCut,"hist");
		fPLayer_Rates_Statistics->Pic(1,1)->AddHStack(fHStackHitRateTimeInLayerCut,"hist");
		fPLayer_Rates_Statistics->Pic(1,2)->AddHStack(fHStackHitRateEvtInLayerCut,"hist");

		/*
		fPLayer_Rates_Statistics->Pic(0,1)->AddObject(fHHitRateEvtInLayer);
//		fPLayer_Rates_Statistics->Pic(0,1)->SetFillAtt(6,3031);
		fPLayer_Rates_Statistics->Pic(0,2)->AddObject(fHHitRateTimeInLayer);
//		fPLayer_Rates_Statistics->Pic(0,2)->SetFillAtt(7,3031);

		fPLayer_Rates_Statistics->Pic(1,0)->AddObject(fHHitsInLayerCut);
//		fPLayer_Rates_Statistics->Pic(1,0)->SetFillAtt(3,3031);
		fPLayer_Rates_Statistics->Pic(1,1)->AddObject(fHHitRateEvtInLayerCut);
//		fPLayer_Rates_Statistics->Pic(1,1)->SetFillAtt(6,3031);
		fPLayer_Rates_Statistics->Pic(1,2)->AddObject(fHHitRateTimeInLayerCut);
//		fPLayer_Rates_Statistics->Pic(1,2)->SetFillAtt(7,3031);
//		*/

		AddPicture(fPLayer_Rates_Statistics);
	}


	/** normale Hitpattern */
	fPHitPatternCond = GetPicture("HitPatternCond");
	if(!fPHitPatternCond)
	{
		fPHitPatternCond = new TGo4Picture("HitPattern", "Hit-Pattern - Anwendung Condition");
		fPHitPatternCond->SetDivision(2,3);
		fPHitPatternCond->Pic(0,0)->AddObject(fHHitPatternX1);
		fPHitPatternCond->Pic(0,0)->SetFillAtt(3,3031);
		fPHitPatternCond->Pic(0,1)->AddObject(fHHitPatternU);
		fPHitPatternCond->Pic(0,1)->SetFillAtt(6,3031);
		fPHitPatternCond->Pic(0,2)->AddObject(fHHitPatternX2);
		fPHitPatternCond->Pic(0,2)->SetFillAtt(7,3031);

		fPHitPatternCond->Pic(1,0)->AddObject(fHHitPatternX1Cut);
		fPHitPatternCond->Pic(1,0)->SetFillAtt(3,3031);
		fPHitPatternCond->Pic(1,1)->AddObject(fHHitPatternUCut);
		fPHitPatternCond->Pic(1,1)->SetFillAtt(6,3031);
		fPHitPatternCond->Pic(1,2)->AddObject(fHHitPatternX2Cut);
		fPHitPatternCond->Pic(1,2)->SetFillAtt(7,3031);

		AddPicture(fPHitPatternCond);
	}

	/** HitRateEvt */
	fPHitRateEvtCond = GetPicture("HitRateEvtCond");
	if(!fPHitRateEvtCond)
	{
		fPHitRateEvtCond = new TGo4Picture("HitRateEvt", "Hit-Raten (Evt) - Anwendung Condition");
		fPHitRateEvtCond->SetDivision(2,3);
		fPHitRateEvtCond->Pic(0,0)->AddObject(fHHitRateEvtX1);
		fPHitRateEvtCond->Pic(0,0)->SetFillAtt(3,3031);
		fPHitRateEvtCond->Pic(0,1)->AddObject(fHHitRateEvtU);
		fPHitRateEvtCond->Pic(0,1)->SetFillAtt(6,3031);
		fPHitRateEvtCond->Pic(0,2)->AddObject(fHHitRateEvtX2);
		fPHitRateEvtCond->Pic(0,2)->SetFillAtt(7,3031);

		fPHitRateEvtCond->Pic(1,0)->AddObject(fHHitRateEvtX1Cut);
		fPHitRateEvtCond->Pic(1,0)->SetFillAtt(3,3031);
		fPHitRateEvtCond->Pic(1,1)->AddObject(fHHitRateEvtUCut);
		fPHitRateEvtCond->Pic(1,1)->SetFillAtt(6,3031);
		fPHitRateEvtCond->Pic(1,2)->AddObject(fHHitRateEvtX2Cut);
		fPHitRateEvtCond->Pic(1,2)->SetFillAtt(7,3031);

		AddPicture(fPHitRateEvtCond);
	}


	/** HitRateTime */
	fPHitRateTimeCond = GetPicture("HitRateTimeCond");
	if(!fPHitRateTimeCond)
	{
		fPHitRateTimeCond = new TGo4Picture("HitRateTime", "Hit-Raten (Zeit) - Anwendung Condition");
		fPHitRateTimeCond->SetDivision(2,3);
		fPHitRateTimeCond->Pic(0,0)->AddObject(fHHitRateTimeX1);
		fPHitRateTimeCond->Pic(0,0)->SetFillAtt(3,3031);
		fPHitRateTimeCond->Pic(0,1)->AddObject(fHHitRateTimeU);
		fPHitRateTimeCond->Pic(0,1)->SetFillAtt(6,3031);
		fPHitRateTimeCond->Pic(0,2)->AddObject(fHHitRateTimeX2);
		fPHitRateTimeCond->Pic(0,2)->SetFillAtt(7,3031);

		fPHitRateTimeCond->Pic(1,0)->AddObject(fHHitRateTimeX1Cut);
		fPHitRateTimeCond->Pic(1,0)->SetFillAtt(3,3031);
		fPHitRateTimeCond->Pic(1,1)->AddObject(fHHitRateTimeUCut);
		fPHitRateTimeCond->Pic(1,1)->SetFillAtt(6,3031);
		fPHitRateTimeCond->Pic(1,2)->AddObject(fHHitRateTimeX2Cut);
		fPHitRateTimeCond->Pic(1,2)->SetFillAtt(7,3031);

		AddPicture(fPHitRateTimeCond);
	}



	/** -- ab hier Driftzeit und ToT Pitures -- */
	fPDrifttimeCond = GetPicture("DrifttimeCond");
	if(!fPDrifttimeCond)
	{
		fPDrifttimeCond = new TGo4Picture("DrifttimeCond", "Driftzeit-Spektren - Anwendung Condition");
		fPDrifttimeCond->SetDivision(2,4);
		fPDrifttimeCond->Pic(0,0)->AddObject(fHDrifttimeDist);
		fPDrifttimeCond->Pic(0,0)->SetFillAtt(4,3031);
		fPDrifttimeCond->Pic(0,1)->AddObject(fHDrifttimeDistX1);
		fPDrifttimeCond->Pic(0,1)->SetFillAtt(3,3031);
		fPDrifttimeCond->Pic(0,1)->AddCondition((*fCondArrDrifttime)[0]); /** Cond einzeichnen */
		fPDrifttimeCond->Pic(0,2)->AddObject(fHDrifttimeDistU);
		fPDrifttimeCond->Pic(0,2)->SetFillAtt(6,3031);
		fPDrifttimeCond->Pic(0,2)->AddCondition((*fCondArrDrifttime)[1]); /** Cond einzeichnen */
		fPDrifttimeCond->Pic(0,3)->AddObject(fHDrifttimeDistX2);
		fPDrifttimeCond->Pic(0,3)->SetFillAtt(7,3031);
		fPDrifttimeCond->Pic(0,3)->AddCondition((*fCondArrDrifttime)[2]); /** Cond einzeichnen */

		/** TODO Condition fuer jede Ebene einzeln... */
		fPDrifttimeCond->Pic(1,0)->AddObject(fHDrifttimeDistCut);
		fPDrifttimeCond->Pic(1,0)->SetFillAtt(4,3031);
		fPDrifttimeCond->Pic(1,1)->AddObject(fHDrifttimeDistX1Cut);
		fPDrifttimeCond->Pic(1,1)->SetFillAtt(3,3031);
		fPDrifttimeCond->Pic(1,2)->AddObject(fHDrifttimeDistUCut);
		fPDrifttimeCond->Pic(1,2)->SetFillAtt(6,3031);
		fPDrifttimeCond->Pic(1,3)->AddObject(fHDrifttimeDistX2Cut);
		fPDrifttimeCond->Pic(1,3)->SetFillAtt(7,3031);

		AddPicture(fPDrifttimeCond);
	}


	fPDrifttimeVsWireCond = GetPicture("DrifttimeVsWireCond");
	if(!fPDrifttimeVsWireCond)
	{
		fPDrifttimeVsWireCond = new TGo4Picture("DrifttimeVsWireCond", "Driftzeit-vs-Draht Spektren - Anwendung Condition");
		fPDrifttimeVsWireCond->SetDivision(2,3);
		fPDrifttimeVsWireCond->Pic(0,0)->AddObject(fHDrifttimeVsWireX1);
		fPDrifttimeVsWireCond->Pic(0,1)->AddObject(fHDrifttimeVsWireU);
		fPDrifttimeVsWireCond->Pic(0,2)->AddObject(fHDrifttimeVsWireX2);
		fPDrifttimeVsWireCond->Pic(1,0)->AddObject(fHDrifttimeVsWireX1Cut);
		fPDrifttimeVsWireCond->Pic(1,1)->AddObject(fHDrifttimeVsWireUCut);
		fPDrifttimeVsWireCond->Pic(1,2)->AddObject(fHDrifttimeVsWireX2Cut);

		AddPicture(fPDrifttimeVsWireCond);
	}


	fPToTCond = GetPicture("ToTCond");
	if(!fPToTCond)
	{
		fPToTCond = new TGo4Picture("ToTCond", "Time-over-Threshold Spektren - Anwendung Condition");
		fPToTCond->SetDivision(2,4);
		fPToTCond->Pic(0,0)->AddObject(fHToTDist);
		fPToTCond->Pic(0,0)->SetFillAtt(4,3031);
		fPToTCond->Pic(0,1)->AddObject(fHToTDistX1);
		fPToTCond->Pic(0,1)->SetFillAtt(3,3031);
		fPToTCond->Pic(0,1)->AddCondition((*fCondArrToT)[0]); /** Cond einzeichnen */
		fPToTCond->Pic(0,2)->AddObject(fHToTDistU);
		fPToTCond->Pic(0,2)->SetFillAtt(6,3031);
		fPToTCond->Pic(0,2)->AddCondition((*fCondArrToT)[1]); /** Cond einzeichnen */
		fPToTCond->Pic(0,3)->AddObject(fHToTDistX2);
		fPToTCond->Pic(0,3)->SetFillAtt(7,3031);
		fPToTCond->Pic(0,3)->AddCondition((*fCondArrToT)[2]); /** Cond einzeichnen */

		/** TODO Condition fuer jede Ebene einzeln... */
		fPToTCond->Pic(1,0)->AddObject(fHToTDistCut);
		fPToTCond->Pic(1,0)->SetFillAtt(4,3031);
		fPToTCond->Pic(1,1)->AddObject(fHToTDistX1Cut);
		fPToTCond->Pic(1,1)->SetFillAtt(3,3031);
		fPToTCond->Pic(1,2)->AddObject(fHToTDistUCut);
		fPToTCond->Pic(1,2)->SetFillAtt(6,3031);
		fPToTCond->Pic(1,3)->AddObject(fHToTDistX2Cut);
		fPToTCond->Pic(1,3)->SetFillAtt(7,3031);

		AddPicture(fPToTCond);
	}

	fPToTVsWireCond = GetPicture("ToTVsWireCond");
	if(!fPToTVsWireCond)
	{
		fPToTVsWireCond = new TGo4Picture("ToTVsWireCond", "Time-over-Threshold versus Draht Spektren - Anwendung Condition");
		fPToTVsWireCond->SetDivision(2,3);
		fPToTVsWireCond->Pic(0,0)->AddObject(fHToTVsWireX1);
		fPToTVsWireCond->Pic(0,1)->AddObject(fHToTVsWireU);
		fPToTVsWireCond->Pic(0,2)->AddObject(fHToTVsWireX2);
		fPToTVsWireCond->Pic(1,0)->AddObject(fHToTVsWireX1Cut);
		fPToTVsWireCond->Pic(1,1)->AddObject(fHToTVsWireUCut);
		fPToTVsWireCond->Pic(1,2)->AddObject(fHToTVsWireX2Cut);

		AddPicture(fPToTVsWireCond);
	}

	fPDrifttimeVsToTCond = GetPicture("DrifttimeVsToTCond");
	if(!fPDrifttimeVsToTCond)
	{
		fPDrifttimeVsToTCond = new TGo4Picture("DrifttimeVsToTCond", "Time-over-Threshold versus Driftzeit Spektren - Anwendung Condition");
		fPDrifttimeVsToTCond->SetDivision(2,4);
		fPDrifttimeVsToTCond->Pic(0,0)->AddObject(fHDrifttimeVsToT);
		fPDrifttimeVsToTCond->Pic(0,1)->AddObject(fHDrifttimeVsToTX1);
		fPDrifttimeVsToTCond->Pic(0,1)->AddCondition((*fCondArrDrifttimeVsToT)[0]); /** Cond einzeichnen */
		fPDrifttimeVsToTCond->Pic(0,2)->AddObject(fHDrifttimeVsToTU);
		fPDrifttimeVsToTCond->Pic(0,2)->AddCondition((*fCondArrDrifttimeVsToT)[1]); /** Cond einzeichnen */
		fPDrifttimeVsToTCond->Pic(0,3)->AddObject(fHDrifttimeVsToTX2);
		fPDrifttimeVsToTCond->Pic(0,3)->AddCondition((*fCondArrDrifttimeVsToT)[2]); /** Cond einzeichnen */

		fPDrifttimeVsToTCond->Pic(1,0)->AddObject(fHDrifttimeVsToTCut);
		fPDrifttimeVsToTCond->Pic(1,1)->AddObject(fHDrifttimeVsToTX1Cut);
		fPDrifttimeVsToTCond->Pic(1,2)->AddObject(fHDrifttimeVsToTUCut);
		fPDrifttimeVsToTCond->Pic(1,3)->AddObject(fHDrifttimeVsToTX2Cut);

		AddPicture(fPDrifttimeVsToTCond);
	}

	/** Time-Walk Korrekturen */
	fPDrifttimeTimeWalkCorr = GetPicture("DrifttimeTimeWalkCorr");
	if(!fPDrifttimeTimeWalkCorr)
	{
		fPDrifttimeTimeWalkCorr = new TGo4Picture("DrifttimeTimeWalkCorr", "Driftzeit-Spektren - Time-Walk Korrektur");
		fPDrifttimeTimeWalkCorr->SetDivision(2,4);

		fPDrifttimeTimeWalkCorr->Pic(0,0)->AddObject(fHDrifttimeDistCut);
		fPDrifttimeTimeWalkCorr->Pic(0,0)->SetFillAtt(4,3031);
		fPDrifttimeTimeWalkCorr->Pic(0,1)->AddObject(fHDrifttimeDistX1Cut);
		fPDrifttimeTimeWalkCorr->Pic(0,1)->SetFillAtt(3,3031);
		fPDrifttimeTimeWalkCorr->Pic(0,2)->AddObject(fHDrifttimeDistUCut);
		fPDrifttimeTimeWalkCorr->Pic(0,2)->SetFillAtt(6,3031);
		fPDrifttimeTimeWalkCorr->Pic(0,3)->AddObject(fHDrifttimeDistX2Cut);
		fPDrifttimeTimeWalkCorr->Pic(0,3)->SetFillAtt(7,3031);

		fPDrifttimeTimeWalkCorr->Pic(1,0)->AddObject(fHDrifttimeDistTimeWalkCorr);
		fPDrifttimeTimeWalkCorr->Pic(1,0)->SetFillAtt(4,3031);
		fPDrifttimeTimeWalkCorr->Pic(1,1)->AddObject(fHDrifttimeDistX1TimeWalkCorr);
		fPDrifttimeTimeWalkCorr->Pic(1,1)->SetFillAtt(3,3031);
		fPDrifttimeTimeWalkCorr->Pic(1,2)->AddObject(fHDrifttimeDistUTimeWalkCorr);
		fPDrifttimeTimeWalkCorr->Pic(1,2)->SetFillAtt(6,3031);
		fPDrifttimeTimeWalkCorr->Pic(1,3)->AddObject(fHDrifttimeDistX2TimeWalkCorr);
		fPDrifttimeTimeWalkCorr->Pic(1,3)->SetFillAtt(7,3031);


		AddPicture(fPDrifttimeTimeWalkCorr);
	}


	fPDrifttimeVsWireTimeWalkCorr = GetPicture("DrifttimeVsWireTimeWalkCorr");
	if(!fPDrifttimeVsWireTimeWalkCorr)
	{
		fPDrifttimeVsWireTimeWalkCorr = new TGo4Picture("DrifttimeVsWireTimeWalkCorr", "Driftzeit-vs-Draht Spektren - Time-Walk Korrektur");
		fPDrifttimeVsWireTimeWalkCorr->SetDivision(2,3);
		fPDrifttimeVsWireTimeWalkCorr->Pic(0,0)->AddObject(fHDrifttimeVsWireX1Cut);
		fPDrifttimeVsWireTimeWalkCorr->Pic(0,1)->AddObject(fHDrifttimeVsWireUCut);
		fPDrifttimeVsWireTimeWalkCorr->Pic(0,2)->AddObject(fHDrifttimeVsWireX2Cut);
		fPDrifttimeVsWireTimeWalkCorr->Pic(1,0)->AddObject(fHDrifttimeVsWireX1TimeWalkCorr);
		fPDrifttimeVsWireTimeWalkCorr->Pic(1,1)->AddObject(fHDrifttimeVsWireUTimeWalkCorr);
		fPDrifttimeVsWireTimeWalkCorr->Pic(1,2)->AddObject(fHDrifttimeVsWireX2TimeWalkCorr);

		AddPicture(fPDrifttimeVsWireTimeWalkCorr);
	}

	fPDrifttimeVsToTTimeWalkCorr = GetPicture("DrifttimeVsToTTimeWalkCorr");
	if(!fPDrifttimeVsToTTimeWalkCorr)
	{
		fPDrifttimeVsToTTimeWalkCorr = new TGo4Picture("DrifttimeVsToTTimeWalkCorr", "Time-over-Threshold versus Driftzeit Spektren - Time-Walk Korrektur");
		fPDrifttimeVsToTTimeWalkCorr->SetDivision(2,4);

		fPDrifttimeVsToTTimeWalkCorr->Pic(0,0)->AddObject(fHDrifttimeVsToTCut);
		fPDrifttimeVsToTTimeWalkCorr->Pic(0,1)->AddObject(fHDrifttimeVsToTX1Cut);
		fPDrifttimeVsToTTimeWalkCorr->Pic(0,2)->AddObject(fHDrifttimeVsToTUCut);
		fPDrifttimeVsToTTimeWalkCorr->Pic(0,3)->AddObject(fHDrifttimeVsToTX2Cut);

		fPDrifttimeVsToTTimeWalkCorr->Pic(1,0)->AddObject(fHDrifttimeVsToTTimeWalkCorr);
		fPDrifttimeVsToTTimeWalkCorr->Pic(1,1)->AddObject(fHDrifttimeVsToTX1TimeWalkCorr);
		fPDrifttimeVsToTTimeWalkCorr->Pic(1,2)->AddObject(fHDrifttimeVsToTUTimeWalkCorr);
		fPDrifttimeVsToTTimeWalkCorr->Pic(1,3)->AddObject(fHDrifttimeVsToTX2TimeWalkCorr);

		AddPicture(fPDrifttimeVsToTTimeWalkCorr);
	}




}

//***********************************************************
TQClamHitProc::~TQClamHitProc()
{

	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("===================================================================================\n");
	printf("===================================================================================\n");
	TGo4Log::Info("[TQClamHitProc] GEBE ALLE CONDITION-STATISTIKEN AUS:");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");

	TGo4Log::Info("[TQClamHitProc]          --- Condition: Time-over-Threshold -----------------------");
	printf("\n");
	fCondArrToT->PrintCondition(true);
	//TGo4Log::Info("...");
	printf("\n");
	printf("...\n");
	TGo4Log::Info("[TQClamHitProc]          --- Condition: Driftzeit -----------------------");
	printf("\n");
	fCondArrDrifttime->PrintCondition(true);
	printf("\n");
	printf("...\n");
	//TGo4Log::Info("...");
	TGo4Log::Info("[TQClamHitProc]          --- Condition: Driftzeit versus Time-over-Threshold -----------------------");
	printf("\n");
	fCondArrDrifttimeVsToT->PrintCondition(true);
	printf("\n");
	printf("...\n");
	printf("...\n");
	printf("...\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");

	printf("\n");
	printf("===================================================================================\n");
	printf("===================================================================================\n");
	TGo4Log::Info("[TQClamHitProc] GEBE CONDITION-STATISTIK ALLER HITPROZESSOR PASSIERTEN HITS AUS:");
	printf("...\n");
	printf("...\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");

	TGo4Log::Info("[TQClamHitProc]          --- Gesamtanzahl passierte Hits -----------------------");
	printf("\n");
	fCondCountHitProcPassed->PrintCondition(true);
	printf("\n");
	printf("...\n");
	//TGo4Log::Info("...");
	TGo4Log::Info("[TQClamHitProc]          --- Anzahl passierter Hits aus X1 -----------------------");
	printf("\n");
	fCondCountHitProcPassedX1->PrintCondition(true);
	printf("\n");
	printf("...\n");
	//TGo4Log::Info("...");
	TGo4Log::Info("[TQClamHitProc]          --- Anzahl passierter Hits aus U -----------------------");
	printf("\n");
	fCondCountHitProcPassedU->PrintCondition(true);
	printf("\n");
	printf("...\n");
	//TGo4Log::Info("...");
	TGo4Log::Info("[TQClamHitProc]          --- Anzahl passierter Hits aus X2 -----------------------");
	printf("\n");
	fCondCountHitProcPassedX2->PrintCondition(true);
	printf("\n");
	printf("...\n");
	printf("...\n");
	printf("===================================================================================\n");
	printf("===================================================================================\n");
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
//
	/** ... */
	TGo4Log::Info("[TQClamHitProc] Delete %s", GetName());
}





//***********************************************************
Bool_t TQClamHitProc::CheckEventClass(TClass* cl)
{
	return cl->InheritsFrom(TQClamHitEvent::Class());
}


//***********************************************************
Bool_t TQClamHitProc::BuildEvent(TGo4EventElement* dest)
{
//	TGo4Log::Info("new event\n\n");
	/** output event */
	TQClamHitEvent* evt = dynamic_cast<TQClamHitEvent*>(dest);

	/** input event */
#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
	auto* raw = dynamic_cast<TQClamRawEvent*>(GetInputEvent());
#endif

#ifdef QCLAM_DAGOBERT
	auto* raw = dynamic_cast<TQClamDagobertRawEvent*>(GetInputEvent("QClamRawProvider"));
#endif


	if(!evt || !raw)
	{
		return kFALSE;
	}

	/// HEADER
	evt->header = raw->header;
	/*
	evt->header.file_name = raw->header.file_name;
	evt->header.file_number = raw->header.file_number;
	evt->header.run_number = raw->header.run_number;

	evt->header.user = raw->header.user;
	evt->header.time = raw->header.time;
	evt->header.label = raw->header.label;

	evt->header.mbs_event_count = raw->header.mbs_event_count;
	evt->header.mbs_trigger = raw->header.mbs_trigger;
	evt->header.mbs_buffer_number = raw->header.mbs_buffer_number;
	evt->header.mbs_evts_in_buffer = raw->header.mbs_evts_in_buffer;
	evt->header.mbs_time_sec = raw->header.mbs_time_sec;
	evt->header.mbs_time_subsec = raw->header.mbs_time_subsec;
	*/

	double eventTime = evt->header.mbs_time_sec + evt->header.mbs_time_subsec * 1e-9;

	if(fFirstTime)
	{
		fStartTime = eventTime;
		fFirstTime = false;
	}

	fGraphTime = (eventTime - fStartTime) / 60.; // in Min

	evt->fEvtCounter = raw->fEvtCounter;

	fEvtCounter++;

	evt->fEventNumber = raw->eventNumber;

#ifdef QCLAM_SINGLE_ROOT
	evt->fEvtCounter = fEvtCounter;
#endif

	//if(!raw->scalers.empty())
	if(true)
	{
		ProcessScaler(raw,evt);

		//auto& sc = raw->scalers[0];
		auto& sc = raw->scaler;


		/** absicherung: falls integrations-fenster online veraendert wurde */
		int currentWindow = fDisplaySettingPar->RollGraphTriggerIntegrationWindow;
		if(currentWindow != fLastRollGraphWindow)
		{
    
			TGo4Log::Info("Rollgraph integration window changed: %d -> %d", fLastRollGraphWindow, currentWindow);

			// Integrationsfenster resetten
			fIntegratedTime = 0.;

			fChSzintUp.dScalerSum = 0.;
			fChSzintDown.dScalerSum = 0.;
			fChCherenkov.dScalerSum = 0.;
			fChCoincidence.dScalerSum = 0.;
			fChAccepted.dScalerSum = 0.;
			fChClock.dScalerSum = 0.;
			fChLivetime.dScalerSum = 0.;
			fChMbsLivetime.dScalerSum = 0.;
			fChFaraday.dScalerSum = 0.;

			fFirstEvent = true;

			fChSzintUp.first = true;
			fChSzintDown.first = true;
			fChCherenkov.first = true;
			fChCoincidence.first = true;
			fChAccepted.first = true;
			fChClock.first = true;
			fChLivetime.first = true;
			fChMbsLivetime.first = true;
			fChFaraday.first = true;

			fLastRollGraphWindow = currentWindow;

		}





		// Zentrale Zeitintegration fuer alle Ratenberechnungen
		bool validTime = IntegrateTime(sc.clockCounter);//timeMicroSeconds);

		// Jeden Scaler integrieren
		UpdateRateChannel(fChSzintUp,sc.szintillatorPmtUp);
		UpdateRateChannel(fChSzintDown,sc.szintillatorPmtDown);
		UpdateRateChannel(fChCherenkov,sc.cherenkovPmt);
		UpdateRateChannel(fChCoincidence,sc.coincidenceTrigger);
		UpdateRateChannel(fChAccepted,sc.acceptedTrigger);
		UpdateRateChannel(fChClock,sc.clockCounter);
		UpdateRateChannel(fChLivetime,sc.livetimeCounter);
		UpdateRateChannel(fChMbsLivetime,sc.mbsLivetimeCounter);
		UpdateRateChannel(fChFaraday,sc.faradayCupChargeCounter);




		if(validTime)
		{
			bool doDisplay = (fEvtCounter % fDisplaySettingPar->RollGraphTriggerIntegrationWindow == 0);



			if(doDisplay)
			{

				double dtInt = fIntegratedTime;

				FlushDeadtime(fChClock,fChLivetime,fGRelDeadtime,fGraphRelDeadtime,nullptr);
				FlushDeadtime(fChClock,fChMbsLivetime,fGRelMbsDeadtime,fGraphRelMbsDeadtime,nullptr);
				FlushRateChannel(fChSzintUp,dtInt,fGSzintUpTrigRate,fGraphSzintUpTrigRate,nullptr);
				FlushRateChannel(fChSzintDown,dtInt,fGSzintDownTrigRate,fGraphSzintDownTrigRate,nullptr);
				FlushRateChannel(fChCherenkov,dtInt,fGCherenkovTrigRate,fGraphCherenkovTrigRate,nullptr);
				FlushRateChannel(fChCoincidence,dtInt,fGCoincTrigRate,fGraphCoincTrigRate,nullptr);
				FlushRateChannel(fChAccepted,dtInt,fGAccTrigRate,fGraphAccTrigRate,nullptr);
	
				FlushFaradayChannel(fChFaraday,dtInt);

				// Clock zuruecksetzen
				fChClock.dScalerSum = 0.;

				// Zeitfenster zuruecksetzen
				fIntegratedTime = 0.;

			}
		}
	}
//	else
//	{
//		TGo4Log::Warn("[TQClamHitProc] Keine Scaler aus TQClamRawEvent uebergeben worden! Ueberspringe Auswertung der Scaler.");
//		return kFALSE;
//	}

	if(!raw->hits.empty())
	{
		ProcessSingleEvent(raw, evt);

		/** fuelle trigger-zeit histogramme */

		if(fHistoPar->fBFillHistos)
		{
			fHSzintUpTime->Fill(evt->trigSzUp.time);
			fHSzintDownTime->Fill(evt->trigSzDown.time);
			fHCherenkovTime->Fill(evt->trigCher.time);
			fHMeanSzintTime->Fill((evt->trigSzUp.time + evt->trigSzDown.time)/2);
			fHDiffSzintTime->Fill((evt->trigSzUp.time - evt->trigSzDown.time));
			fHSzintUpVsSzintDown->Fill(evt->trigSzDown.time,evt->trigSzUp.time);
		}

		evt->trigDet.timeSzintDown = evt->trigSzDown.time;
		evt->trigDet.timeSzintUp = evt->trigSzUp.time;
		evt->trigDet.timeCherenkov = evt->trigCher.time;

	}
	else
	{
		TGo4Log::Info("[TQClamHitProc Keine Hits aus TQClamRawEvent uebergeben worden! Ueberspringe Auswertung der Hits!");
		return kFALSE;
	}

	/** Rohdaten fuer Debug in ein file schreiben */
	if(fHitProcDebugOutputPar->WriteRaw)
	{
		WriteRawDataToFile(raw);
	}

	//TQClamHit& hit = evt->hits[0];
	//if(!ProofHitConditions(hit)) { return kFALSE; }






	/** Interne Parameter updaten */
	//if(!raw->scalers.empty() && fEvtCounter%fDisplaySettingPar->RollGraphTriggerIntegrationWindow == 0)
	if(fEvtCounter%fDisplaySettingPar->RollGraphTriggerIntegrationWindow == 0)
	{
		//fTimeInMicroSeconds_LastEvent = raw->scalers[0].timeMicroSeconds;
		fTimeInMicroSeconds_LastEvent = raw->scaler.timeMicroSeconds;
	}



	if(fDrifttimeCalibPar->fDoFitAny && fEvtCounter%fDrifttimeCalibPar->fUpdateEventInterval == 0)
	{
		FitDrifttimeSpectra();
	}



	/** alle noetigen trigger vorhanden? */
	bool eventValid = fTriggerCaliPar->ProofEvent(
			evt->trigSzUp.fired,
			evt->trigSzDown.fired,
			evt->trigCher.fired,
			evt->trigCoinc.fired,
			evt->trigAcc.fired,
			evt->trigClock.fired,
			evt->trigLive.fired,
			evt->trigMbsLive.fired,
			evt->trigFaraday.fired);

	/** evt nicht ok */
	if(!eventValid)
	{
		evt->SetValid(kFALSE);
		return kFALSE;
	}


	/** TODO Daten in output-evt schreiben */


	/** -- baue hits -- */
	/** alle Fenster ok */
	evt->SetValid(kTRUE);
	return kTRUE;

}










//***********************************************************
// lese Scalerwerte
#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
void TQClamHitProc::ProcessScaler(TQClamRawEvent* raw, TQClamHitEvent* evt)
#endif
#ifdef QCLAM_DAGOBERT
void TQClamHitProc::ProcessScaler(TQClamDagobertRawEvent* raw, TQClamHitEvent* evt)
#endif
{
	//evt->scaler.clear();

	evt->scaler.accepted = raw->scaler.accepted;
	evt->scaler.timeMicroSeconds = raw->scaler.timeMicroSeconds;

	evt->scaler.tdcTrigger = raw->scaler.tdcTrigger;
	evt->scaler.szintillatorPmtUp = raw->scaler.szintillatorPmtUp;
	evt->scaler.szintillatorPmtDown = raw->scaler.szintillatorPmtDown;
	evt->scaler.cherenkovPmt = raw->scaler.cherenkovPmt;
	evt->scaler.coincidenceTrigger = raw->scaler.coincidenceTrigger;
	evt->scaler.acceptedTrigger = raw->scaler.acceptedTrigger;
	evt->scaler.clockCounter = raw->scaler.clockCounter;
	evt->scaler.livetimeCounter = raw->scaler.livetimeCounter;
	evt->scaler.mbsLivetimeCounter = raw->scaler.mbsLivetimeCounter;
	evt->scaler.faradayCupChargeCounter = raw->scaler.faradayCupChargeCounter;

	evt->scaler.errorTdcTrigger = raw->scaler.errorTdcTrigger;
	evt->scaler.errorSzintillatorPmtUp = raw->scaler.errorSzintillatorPmtUp;
	evt->scaler.errorSzintillatorPmtDown = raw->scaler.errorSzintillatorPmtDown;
	evt->scaler.errorCherenkovPmt = raw->scaler.errorCherenkovPmt;
	evt->scaler.errorCoincidenceTrigger = raw->scaler.errorCoincidenceTrigger;
	evt->scaler.errorAcceptedTrigger = raw->scaler.errorAcceptedTrigger;
	evt->scaler.errorClockCounter = raw->scaler.errorClockCounter;
	evt->scaler.errorLivetimeCounter = raw->scaler.errorLivetimeCounter;
	evt->scaler.errorMbsLivetimeCounter = raw->scaler.errorMbsLivetimeCounter;
	evt->scaler.errorFaradayCupChargeCounter = raw->scaler.errorFaradayCupChargeCounter;


	fTimeInMicroSeconds = evt->scaler.timeMicroSeconds;



	/*
	for(const auto& s : raw->scalers)
	{
		TQClamScaler sc;
		sc.accepted = s.accepted;
		sc.timeMicroSeconds = s.timeMicroSeconds;

		sc.tdcTrigger = s.TDCTrigger;
		sc.szintillatorPmtUp = s.scintillatorPMTUp;
		sc.szintillatorPmtDown = s.scintillatorPMTDown;
		sc.cherenkovPmt = s.cherenkovPMT;
		sc.coincidenceTrigger = s.coincidenceTrigger;
		sc.acceptedTrigger = s.acceptedTrigger;
		sc.clockCounter = s.clockCounter;
		sc.livetimeCounter = s.livetimeCounter;
		sc.mbsLivetimeCounter = s.mbsLivetimeCounter;
		sc.faradayCupChargeCounter = s.faradayCupChargeCounter;

		sc.errorTdcTrigger = s.errorTDCTrigger;
		sc.errorSzintillatorPmtUp = s.errorScintillatorPMTUp;
		sc.errorSzintillatorPmtDown = s.errorScintillatorPMTDown;
		sc.errorCherenkovPmt = s.errorCherenkovPMT;
		sc.errorCoincidenceTrigger = s.errorCoincidenceTrigger;
		sc.errorAcceptedTrigger = s.errorAcceptedTrigger;
		sc.errorClockCounter = s.errorClockCounter;
		sc.errorLivetimeCounter = s.errorLivetimeCounter;
		sc.errorMbsLivetimeCounter = s.errorMBSLivetimeCounter;
		sc.errorFaradayCupChargeCounter = s.errorFaradayCupChargeCounter;

		evt->scaler.push_back(sc);

		fTimeInMicroSeconds = sc.timeMicroSeconds;
	}
	*/
}






//***********************************************************
// erzeuge Driftkammerhits und Triggerhits (Szint, Cherenkov, Clock, ...)
// --> keine Beruecksichtigung von moeglichen (?) SubEvents --> mehrere Haupt- und Tdc-Trigger?
#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
void TQClamHitProc::ProcessSingleEvent(TQClamRawEvent* raw, TQClamHitEvent* evt)
#endif
#ifdef QCLAM_DAGOBERT
void TQClamHitProc::ProcessSingleEvent(TQClamDagobertRawEvent* raw, TQClamHitEvent* evt)
#endif
{
	/** reset */

	for(int t=0; t<QCLAM_MAX_TDC; t++)
	{
		hasTdcTrigger[t] = false;
		tdcTriggerTime[t] = -1;
		for(int ch=0; ch<QCLAM_MAX_CH; ch++)
		{
//			hasLeading[t][ch] = false;
//			leadingTime[t][ch] = -1;
			leadingTimes[t][ch].clear();
			trailingTimes[t][ch].clear();
		}
	}




	/** iteriere ueber hits */
	for(const auto& hit : raw->hits)
	{
		if(!hit.accepted) { continue; }

		int tdc = hit.tdcID;
		int ch = hit.channel;
		double t = hit.rawTimeInNanoSeconds;

		if(tdc < 0 || ch < 0 || tdc >= QCLAM_MAX_TDC || ch >= QCLAM_MAX_CH)
		{
			TGo4Log::Warn("[TQClamHitProc] Ungueltiger Kanal: tdc=%d ch=%d.", tdc, ch);
			continue;
		}


		/** stelle fest, ob eine leading- oder trailing-edge vorliegt.
		 * beruecksichtige das invertieren beider
		 * --> alte firmware vertauschte: leading <-> trailing */
		bool isLead = fTriggerCaliPar->invertEdges ? fEdgeMappingPar->IsTrailing(tdc,ch)
			: fEdgeMappingPar->IsLeading(tdc,ch);
		bool isTrail = fTriggerCaliPar->invertEdges ? fEdgeMappingPar->IsLeading(tdc,ch)
			: fEdgeMappingPar->IsTrailing(tdc,ch);


		/** TDC-TRIGGER */
		if(fEdgeMappingPar->IsTdcTrigger(tdc,ch))
		{
			tdcTriggerTime[tdc] = t;
			hasTdcTrigger[tdc] = true;
			continue;
		}

		TriggerType tt = fTriggerCaliPar->GetTriggerType(tdc,ch);
		if(tt != TriggerType::None)
		{
			/** kein Tdc-Trigger benoetigt fuer Trigger-Detektoren! */

			double tTdc = hasTdcTrigger[tdc] ? tdcTriggerTime[tdc] : 0;
			double tRel = t - tTdc;
			double tCorr = fTriggerCaliPar->CorrectTime(tRel,tdc,ch);

			switch(tt)
			{
				case TriggerType::SzintPmtUp:
				{
					AddTriggerHit(evt->trigSzUp,tdc,ch,tCorr,0. /** kein tot */);
					break;
				}
				case TriggerType::SzintPmtDown:
				{
					AddTriggerHit(evt->trigSzDown,tdc,ch,tCorr,0.);
					break;
				}
				case TriggerType::CherenkovPmt:
				{
					AddTriggerHit(evt->trigCher,tdc,ch,tCorr,0.);
					break;
				}
				case TriggerType::CoincTrig:
				{
					AddTriggerHit(evt->trigCoinc,tdc,ch,tCorr,0.);
					break;
				}
				case TriggerType::AccTrig:
				{
					AddTriggerHit(evt->trigAcc,tdc,ch,tCorr,0.);
					break;
				}
				case TriggerType::Clock:
				{
					AddTriggerHit(evt->trigClock,tdc,ch,tCorr,0.);
					break;
				}
				case TriggerType::Livetime:
				{
					AddTriggerHit(evt->trigLive,tdc,ch,tCorr,0.);
					break;
				}
				case TriggerType::MbsLivetime:
				{
					AddTriggerHit(evt->trigMbsLive,tdc,ch,tCorr,0.);
					break;
				}
				case TriggerType::FaradayCupChargeCounter:
				{
					AddTriggerHit(evt->trigFaraday,tdc,ch,tCorr,0.);
					break;
				}
				default: break;
			}

			/** naechsten hit lesen */
			continue;
		}

		/** TRAILING */
		else if(isTrail)
		{

			/** */
			int leadCh = fTriggerCaliPar->invertEdges ? ch
				: fEdgeMappingPar->GetLeadingForTrailing(tdc,ch);


			if(leadCh < 0 || leadCh >= QCLAM_MAX_CH)
			{
				TGo4Log::Warn("[TQClamHitProc] Ungueltiger leadCh=%d fuer tdc=%d ch=%d.",leadCh,tdc,ch);
				continue;
			}
			
			/** falls ungueltiger leading Kanal || kein Leading || kein tdcTrigger */
			/*
			if(!hasLeading[tdc][leadCh] || !fTriggerCaliPar->ProofHit(hasTdcTrigger[tdc]))
			{
				continue;
			}
			*/

			trailingTimes[tdc][leadCh].push_back(t);
			if(leadingTimes[tdc][leadCh].empty()) { continue; }

			double tLead = leadingTimes[tdc][leadCh].front();
			double tTrail = trailingTimes[tdc][leadCh].front();

			leadingTimes[tdc][leadCh].pop_front();
			trailingTimes[tdc][leadCh].pop_front();


//			TGo4Log::Info("Trail time: tdc=%d,ch=%d,t=%f",tdc,ch,t);


			/** zeiten (tdc-trig, leading, trailing) holen */
			double tTdcTrigger = (hasTdcTrigger[tdc] ? tdcTriggerTime[tdc] : 0);
//			double tLead = leadingTime[tdc][leadCh];
//			double tTrail = t;

//			TGo4Log::Info("mapped to leading: tdc=%d,ch=%d,t=%f",tdc,leadCh,tLead);
//			TGo4Log::Info("");
//			TGo4Log::Info("");

			/** time-over-threshold & korrigierte leading-zeit */
			double tot = std::abs(tTrail - tLead); // TODO
			double tLeadCorr = tLead - tTdcTrigger;


			/*
			if(fTriggerCaliPar->IsSzintPmtUp(tdc,leadCh))
			{
				AddTriggerHit(evt->trigSzUp,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsSzintPmtDown(tdc,leadCh))
			{
				AddTriggerHit(evt->trigSzDown,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsCherenkovPmt(tdc,leadCh))
			{
				AddTriggerHit(evt->trigCher,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsCoincTrig(tdc,leadCh))
			{
				AddTriggerHit(evt->trigCoinc,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsAccTrig(tdc,leadCh))
			{
				AddTriggerHit(evt->trigAcc,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsClock(tdc,leadCh))
			{
				AddTriggerHit(evt->trigClock,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsLivetime(tdc,leadCh))
			{
				AddTriggerHit(evt->trigLive,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsMbsLivetime(tdc,leadCh))
			{
				AddTriggerHit(evt->trigMbsLive,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsFaradayCupChargeCounter(tdc,leadCh))
			{
				AddTriggerHit(evt->trigFaraday,tdc,leadCh,tLeadCorr,tot);
			}
			*/




			/** DRIFTKAMMER */
			if(fDriftchamCaliPar->HasMapping(tdc,leadCh))
			{
				auto [layer,wire] = fDriftchamCaliPar->GetWire(tdc,leadCh);

				if(layer >= 0 && wire >= 0 && fDriftchamCaliPar->IsWireActive(layer,wire) && hasTdcTrigger[tdc])
				{
					double drifttime = fDriftchamCaliPar->ApplyTimeOffsets(tLeadCorr,tdc,leadCh,layer,wire);


					if(fHistoPar->fBFillHistos) // START fBFillHistos
					{
						/** Histogramme fuellen --> noch keine Conditions */
						fHDrifttimeDist->Fill(drifttime);
						fHToTDist->Fill(tot);
						fHDrifttimeVsToT->Fill(tot,drifttime);
						

						if(layer == 0)
						{
							fHitCounterX1++;
							fHitCounterX1Wire[wire]++;
	
							fHHitsInX1->Fill(layer);
	
							fHHitRateEvtInX1->SetBinContent(layer+1,double(fHitCounterX1)/double(fEvtCounter));
							fHHitRateTimeInX1->SetBinContent(layer+1,
									double(fHitCounterX1)/fTimeInMicroSeconds*1000*1000);
	
							fHHitPatternX1->Fill(wire);
							fHHitRateEvtX1->SetBinContent(wire+1,double(fHitCounterX1Wire[wire])/double(fEvtCounter));
							fHHitRateTimeX1->SetBinContent(wire+1,
									double(fHitCounterX1Wire[wire])/fTimeInMicroSeconds*1000*1000);
							fHDrifttimeDistX1->Fill(drifttime);
							fHToTDistX1->Fill(tot);
							fHDrifttimeVsWireX1->Fill(wire,drifttime);
							fHToTVsWireX1->Fill(wire,tot);
							fHDrifttimeVsToTX1->Fill(tot,drifttime);
						}
						else if(layer == 1)
						{
							fHitCounterU++;
							fHitCounterUWire[wire]++;
	
							fHHitsInU->Fill(layer);
	
							fHHitRateEvtInU->SetBinContent(layer+1,double(fHitCounterU)/double(fEvtCounter));
							fHHitRateTimeInU->SetBinContent(layer+1,
									double(fHitCounterU)/fTimeInMicroSeconds*1000*1000);
	
	
							fHHitPatternU->Fill(wire);
							fHHitRateEvtU->SetBinContent(wire+1,double(fHitCounterUWire[wire])/double(fEvtCounter));
							fHHitRateTimeU->SetBinContent(wire+1,
									double(fHitCounterUWire[wire])/fTimeInMicroSeconds*1000*1000);
							fHDrifttimeDistU->Fill(drifttime);
							fHToTDistU->Fill(tot);
							fHDrifttimeVsWireU->Fill(wire,drifttime);
							fHToTVsWireU->Fill(wire,tot);
							fHDrifttimeVsToTU->Fill(tot,drifttime);
						}
						else if(layer == 2)
						{
							fHitCounterX2++;
							fHitCounterX2Wire[wire]++;
	
							fHHitsInX2->Fill(layer);
	
							fHHitRateEvtInX2->SetBinContent(layer+1,double(fHitCounterX2)/double(fEvtCounter));
							fHHitRateTimeInX2->SetBinContent(layer+1,
									double(fHitCounterX2)/fTimeInMicroSeconds*1000*1000);
	
	
							fHHitPatternX2->Fill(wire);
							fHHitRateEvtX2->SetBinContent(wire+1,double(fHitCounterX2Wire[wire])/double(fEvtCounter));
							fHHitRateTimeX2->SetBinContent(wire+1,
									double(fHitCounterX2Wire[wire])/fTimeInMicroSeconds*1000*1000);
							fHDrifttimeDistX2->Fill(drifttime);
							fHToTDistX2->Fill(tot);
							fHDrifttimeVsWireX2->Fill(wire,drifttime);
							fHToTVsWireX2->Fill(wire,tot);
							fHDrifttimeVsToTX2->Fill(tot,drifttime);
						}
					} // END fBFillHistos



					/** Hit erzeugen */
					TQClamHit h;
					h.accepted = true;
					h.tdcID = tdc;
					h.chNum = leadCh;
					h.layerID = layer;
					h.wireNum = wire;
					h.absoluteTime = tLead; /** oder tLeadCorr */
					h.tdcTrigTime = tTdcTrigger;
					h.leading = tLead;
					h.trailing = tTrail;
					h.tot = tot;
//					h.fWire = driftcham.GetWire(layer,wire);


					/** time-walk korrektur und driftzeit in Hit setzen*/
					double timewalk_corrected_drifttime = fDriftchamCaliPar->ApplyTimeWalkCorrection(layer,wire,drifttime,tot);
					if(fDriftchamCaliPar->useTimeWalkCorrectedDrifttime)
					{
						h.drifttime = timewalk_corrected_drifttime;
					}
					else { h.drifttime = drifttime; } // andernfalls unkorrigierte driftzeit




					/** Conditions angewendet */
					if(ProofHitConditions(h))
					{
						evt->hits.push_back(h);
						fCondCountHitProcPassed->Test(true); /** weiterer hit passiert */

						//fHHitsInLayerCut->Fill(layer);



						// START fBFillHistos
						if(fHistoPar->fBFillHistos)
						{

							/** Histogramme fuellen */
							fHDrifttimeDistCut->Fill(drifttime);
							fHToTDistCut->Fill(tot);
							fHDrifttimeVsToTCut->Fill(tot,drifttime);

							fHDrifttimeDistTimeWalkCorr->Fill(timewalk_corrected_drifttime);
							fHDrifttimeVsToTTimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);

							if(layer == 0)
							{
								fHitCounterX1Cut++;
								fHitCounterX1WireCut[wire]++;

								fHHitsInX1Cut->Fill(layer);
								fHHitRateEvtInX1Cut->SetBinContent(layer+1,double(fHitCounterX1Cut)/double(fEvtCounter));
								fHHitRateTimeInX1Cut->SetBinContent(layer+1,
										double(fHitCounterX1Cut)/fTimeInMicroSeconds*1000*1000);



								fHHitPatternX1Cut->Fill(wire);
								fHHitRateEvtX1Cut->SetBinContent(wire+1,double(fHitCounterX1WireCut[wire])/double(fEvtCounter));
								fHHitRateTimeX1Cut->SetBinContent(wire+1,
										double(fHitCounterX1WireCut[wire])/fTimeInMicroSeconds*1000*1000);
								fCondCountHitProcPassedX1->Test(true); /** weiterer X1-hit passiert */
								fHDrifttimeDistX1Cut->Fill(drifttime);
								fHToTDistX1Cut->Fill(tot);
								fHDrifttimeVsWireX1Cut->Fill(wire,drifttime);
								fHToTVsWireX1Cut->Fill(wire,tot);
								fHDrifttimeVsToTX1Cut->Fill(tot,drifttime);

								fHDrifttimeDistX1TimeWalkCorr->Fill(timewalk_corrected_drifttime);
								fHDrifttimeVsWireX1TimeWalkCorr->Fill(wire,timewalk_corrected_drifttime);
								fHDrifttimeVsToTX1TimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);
							}
							else if(layer == 1)
							{
								fHitCounterUCut++;
								fHitCounterUWireCut[wire]++;

								fHHitsInUCut->Fill(layer);
								fHHitRateEvtInUCut->SetBinContent(layer+1,double(fHitCounterUCut)/double(fEvtCounter));
								fHHitRateTimeInUCut->SetBinContent(layer+1,
										double(fHitCounterUCut)/fTimeInMicroSeconds*1000*1000);
	


								fHHitPatternUCut->Fill(wire);
								fHHitRateEvtUCut->SetBinContent(wire+1,double(fHitCounterUWireCut[wire])/double(fEvtCounter));
								fHHitRateTimeUCut->SetBinContent(wire+1,
										double(fHitCounterUWireCut[wire])/fTimeInMicroSeconds*1000*1000);
								fCondCountHitProcPassedU->Test(true); /** weiterer U-hit passiert */
								fHDrifttimeDistUCut->Fill(drifttime);
								fHToTDistUCut->Fill(tot);
								fHDrifttimeVsWireUCut->Fill(wire,drifttime);
								fHToTVsWireUCut->Fill(wire,tot);
								fHDrifttimeVsToTUCut->Fill(tot,drifttime);

								fHDrifttimeDistUTimeWalkCorr->Fill(timewalk_corrected_drifttime);
								fHDrifttimeVsWireUTimeWalkCorr->Fill(wire,timewalk_corrected_drifttime);
								fHDrifttimeVsToTUTimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);
							}
							else if(layer == 2)
							{
								fHitCounterX2Cut++;
								fHitCounterX2WireCut[wire]++;


								fHHitsInX2Cut->Fill(layer);
								fHHitRateEvtInX2Cut->SetBinContent(layer+1,double(fHitCounterX2Cut)/double(fEvtCounter));
								fHHitRateTimeInX2Cut->SetBinContent(layer+1,
										double(fHitCounterX2Cut)/fTimeInMicroSeconds*1000*1000);


								fHHitPatternX2Cut->Fill(wire);
								fHHitRateEvtX2Cut->SetBinContent(wire+1,double(fHitCounterX2WireCut[wire])/double(fEvtCounter));
								fHHitRateTimeX2Cut->SetBinContent(wire+1,
										double(fHitCounterX2WireCut[wire])/fTimeInMicroSeconds*1000*1000);
								fCondCountHitProcPassedX2->Test(true); /** weiterer X2-hit passiert */
								fHDrifttimeDistX2Cut->Fill(drifttime);
								fHToTDistX2Cut->Fill(tot);
								fHDrifttimeVsWireX2Cut->Fill(wire,drifttime);
								fHToTVsWireX2Cut->Fill(wire,tot);
								fHDrifttimeVsToTX2Cut->Fill(tot,drifttime);
	
								fHDrifttimeDistX2TimeWalkCorr->Fill(timewalk_corrected_drifttime);
								fHDrifttimeVsWireX2TimeWalkCorr->Fill(wire,timewalk_corrected_drifttime);
								fHDrifttimeVsToTX2TimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);
							}
	
		
						} // END fBFillHistos

					} // END if ProofConditions = true
					 
					else
					{
						if(layer == 0) { fCondCountHitProcPassedX1->Test(false); }
						if(layer == 1) { fCondCountHitProcPassedU->Test(false); }
						if(layer == 2) { fCondCountHitProcPassedX2->Test(false); }
						fCondCountHitProcPassed->Test(false);
					}



				}
			}

			/** zuruecksetzen */
			//hasLeading[tdc][leadCh] = false;
			//leadingTime[tdc][leadCh] = -1;

			//trailingTimes[tdc][leadCh].pop_front();
			continue;
		}




		/** LEADING */
		else if(isLead)
		{
			int leadCh = fTriggerCaliPar->invertEdges ? fEdgeMappingPar->GetLeadingForTrailing(tdc,ch) : ch;

			if(leadCh < 0 || leadCh >= QCLAM_MAX_CH)
			{
				TGo4Log::Warn("[TQClamHitProc] Ungueltiger leadCh=%d fuer tdc=%d ch=%d.",leadCh,tdc,ch);
				continue;
			}
//			TGo4Log::Info("leadTime: tdc=%d, ch=%d,t=%f",tdc,ch,t);

			/*
			leadingTime[tdc][leadCh] = t;
			hasLeading[tdc][leadCh] = true;
			continue;
			*/
			leadingTimes[tdc][leadCh].push_back(t);

			if(trailingTimes[tdc][leadCh].empty()) { continue; }

			double tLead = leadingTimes[tdc][leadCh].front();
			double tTrail = trailingTimes[tdc][leadCh].front();

			leadingTimes[tdc][leadCh].pop_front();
			trailingTimes[tdc][leadCh].pop_front();



//			TGo4Log::Info("Trail time: tdc=%d,ch=%d,t=%f",tdc,ch,t);


			/** zeiten (tdc-trig, leading, trailing) holen */
			double tTdcTrigger = (hasTdcTrigger[tdc] ? tdcTriggerTime[tdc] : 0);
//			double tLead = leadingTime[tdc][leadCh];
//			double tTrail = t;

//			TGo4Log::Info("mapped to leading: tdc=%d,ch=%d,t=%f",tdc,leadCh,tLead);
//			TGo4Log::Info("");
//			TGo4Log::Info("");

			/** time-over-threshold & korrigierte leading-zeit */
			double tot = std::abs(tTrail - tLead); // TODO
			double tLeadCorr = tLead - tTdcTrigger;


			/*
			if(fTriggerCaliPar->IsSzintPmtUp(tdc,leadCh))
			{
				AddTriggerHit(evt->trigSzUp,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsSzintPmtDown(tdc,leadCh))
			{
				AddTriggerHit(evt->trigSzDown,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsCherenkovPmt(tdc,leadCh))
			{
				AddTriggerHit(evt->trigCher,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsCoincTrig(tdc,leadCh))
			{
				AddTriggerHit(evt->trigCoinc,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsAccTrig(tdc,leadCh))
			{
				AddTriggerHit(evt->trigAcc,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsClock(tdc,leadCh))
			{
				AddTriggerHit(evt->trigClock,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsLivetime(tdc,leadCh))
			{
				AddTriggerHit(evt->trigLive,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsMbsLivetime(tdc,leadCh))
			{
				AddTriggerHit(evt->trigMbsLive,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsFaradayCupChargeCounter(tdc,leadCh))
			{
				AddTriggerHit(evt->trigFaraday,tdc,leadCh,tLeadCorr,tot);
			}
			*/




			/** DRIFTKAMMER */
			if(fDriftchamCaliPar->HasMapping(tdc,leadCh))
			{
				auto [layer,wire] = fDriftchamCaliPar->GetWire(tdc,leadCh);

				if(layer >= 0 && wire >= 0 && fDriftchamCaliPar->IsWireActive(layer,wire) && hasTdcTrigger[tdc])
				{
					double drifttime = fDriftchamCaliPar->ApplyTimeOffsets(tLeadCorr,tdc,leadCh,layer,wire);


	
					// START fBFillHistos
					if(fHistoPar->fBFillHistos)
					{
						/** Histogramme fuellen --> noch keine Conditions */
						fHDrifttimeDist->Fill(drifttime);
						fHToTDist->Fill(tot);
						fHDrifttimeVsToT->Fill(tot,drifttime);
						

						if(layer == 0)
						{
							fHitCounterX1++;
							fHitCounterX1Wire[wire]++;

							fHHitsInX1->Fill(layer);
	
							fHHitRateEvtInX1->SetBinContent(layer+1,double(fHitCounterX1)/double(fEvtCounter));
							fHHitRateTimeInX1->SetBinContent(layer+1,
									double(fHitCounterX1)/fTimeInMicroSeconds*1000*1000);
	
							fHHitPatternX1->Fill(wire);
							fHHitRateEvtX1->SetBinContent(wire+1,double(fHitCounterX1Wire[wire])/double(fEvtCounter));
							fHHitRateTimeX1->SetBinContent(wire+1,
									double(fHitCounterX1Wire[wire])/fTimeInMicroSeconds*1000*1000);
							fHDrifttimeDistX1->Fill(drifttime);
							fHToTDistX1->Fill(tot);
							fHDrifttimeVsWireX1->Fill(wire,drifttime);
							fHToTVsWireX1->Fill(wire,tot);
							fHDrifttimeVsToTX1->Fill(tot,drifttime);
						}
						else if(layer == 1)
						{
							fHitCounterU++;
							fHitCounterUWire[wire]++;
	
							fHHitsInU->Fill(layer);
	
							fHHitRateEvtInU->SetBinContent(layer+1,double(fHitCounterU)/double(fEvtCounter));
							fHHitRateTimeInU->SetBinContent(layer+1,
									double(fHitCounterU)/fTimeInMicroSeconds*1000*1000);


							fHHitPatternU->Fill(wire);
							fHHitRateEvtU->SetBinContent(wire+1,double(fHitCounterUWire[wire])/double(fEvtCounter));
							fHHitRateTimeU->SetBinContent(wire+1,
									double(fHitCounterUWire[wire])/fTimeInMicroSeconds*1000*1000);
							fHDrifttimeDistU->Fill(drifttime);
							fHToTDistU->Fill(tot);
							fHDrifttimeVsWireU->Fill(wire,drifttime);
							fHToTVsWireU->Fill(wire,tot);
							fHDrifttimeVsToTU->Fill(tot,drifttime);
						}
						else if(layer == 2)
						{
							fHitCounterX2++;
							fHitCounterX2Wire[wire]++;

							fHHitsInX2->Fill(layer);

							fHHitRateEvtInX2->SetBinContent(layer+1,double(fHitCounterX2)/double(fEvtCounter));
							fHHitRateTimeInX2->SetBinContent(layer+1,
									double(fHitCounterX2)/fTimeInMicroSeconds*1000*1000);


							fHHitPatternX2->Fill(wire);
							fHHitRateEvtX2->SetBinContent(wire+1,double(fHitCounterX2Wire[wire])/double(fEvtCounter));
							fHHitRateTimeX2->SetBinContent(wire+1,
									double(fHitCounterX2Wire[wire])/fTimeInMicroSeconds*1000*1000);
							fHDrifttimeDistX2->Fill(drifttime);
							fHToTDistX2->Fill(tot);
							fHDrifttimeVsWireX2->Fill(wire,drifttime);
							fHToTVsWireX2->Fill(wire,tot);
							fHDrifttimeVsToTX2->Fill(tot,drifttime);
						}
					} // END fBFillHistos



					/** Hit erzeugen */
					TQClamHit h;
					h.accepted = true;
					h.tdcID = tdc;
					h.chNum = leadCh;
					h.layerID = layer;
					h.wireNum = wire;
					h.absoluteTime = tLead; /** oder tLeadCorr */
					h.tdcTrigTime = tTdcTrigger;
					h.leading = tLead;
					h.trailing = tTrail;
					h.tot = tot;
//					h.fWire = driftcham.GetWire(layer,wire);


					/** time-walk korrektur und driftzeit in Hit setzen*/
					double timewalk_corrected_drifttime = fDriftchamCaliPar->ApplyTimeWalkCorrection(layer,wire,drifttime,tot);
					if(fDriftchamCaliPar->useTimeWalkCorrectedDrifttime)
					{
						h.drifttime = timewalk_corrected_drifttime;
					}
					else { h.drifttime = drifttime; } // andernfalls unkorrigierte driftzeit




					/** Conditions angewendet */
					if(ProofHitConditions(h))
					{
						evt->hits.push_back(h);
						fCondCountHitProcPassed->Test(true); /** weiterer hit passiert */

						//fHHitsInLayerCut->Fill(layer);



	
						// START fBFillHistos
						if(fHistoPar->fBFillHistos)
						{
							/** Histogramme fuellen */
							fHDrifttimeDistCut->Fill(drifttime);
							fHToTDistCut->Fill(tot);
							fHDrifttimeVsToTCut->Fill(tot,drifttime);

							fHDrifttimeDistTimeWalkCorr->Fill(timewalk_corrected_drifttime);
							fHDrifttimeVsToTTimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);

							if(layer == 0)
							{
								fHitCounterX1Cut++;
								fHitCounterX1WireCut[wire]++;
	
								fHHitsInX1Cut->Fill(layer);
								fHHitRateEvtInX1Cut->SetBinContent(layer+1,double(fHitCounterX1Cut)/double(fEvtCounter));
								fHHitRateTimeInX1Cut->SetBinContent(layer+1,
										double(fHitCounterX1Cut)/fTimeInMicroSeconds*1000*1000);



								fHHitPatternX1Cut->Fill(wire);
								fHHitRateEvtX1Cut->SetBinContent(wire+1,double(fHitCounterX1WireCut[wire])/double(fEvtCounter));
								fHHitRateTimeX1Cut->SetBinContent(wire+1,
										double(fHitCounterX1WireCut[wire])/fTimeInMicroSeconds*1000*1000);
								fCondCountHitProcPassedX1->Test(true); /** weiterer X1-hit passiert */
								fHDrifttimeDistX1Cut->Fill(drifttime);
								fHToTDistX1Cut->Fill(tot);
								fHDrifttimeVsWireX1Cut->Fill(wire,drifttime);
								fHToTVsWireX1Cut->Fill(wire,tot);
								fHDrifttimeVsToTX1Cut->Fill(tot,drifttime);

								fHDrifttimeDistX1TimeWalkCorr->Fill(timewalk_corrected_drifttime);
								fHDrifttimeVsWireX1TimeWalkCorr->Fill(wire,timewalk_corrected_drifttime);
								fHDrifttimeVsToTX1TimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);
							}
							else if(layer == 1)
							{
								fHitCounterUCut++;
								fHitCounterUWireCut[wire]++;

								fHHitsInUCut->Fill(layer);
								fHHitRateEvtInUCut->SetBinContent(layer+1,double(fHitCounterUCut)/double(fEvtCounter));
								fHHitRateTimeInUCut->SetBinContent(layer+1,
										double(fHitCounterUCut)/fTimeInMicroSeconds*1000*1000);


	
								fHHitPatternUCut->Fill(wire);
								fHHitRateEvtUCut->SetBinContent(wire+1,double(fHitCounterUWireCut[wire])/double(fEvtCounter));
								fHHitRateTimeUCut->SetBinContent(wire+1,
										double(fHitCounterUWireCut[wire])/fTimeInMicroSeconds*1000*1000);
								fCondCountHitProcPassedU->Test(true); /** weiterer U-hit passiert */
								fHDrifttimeDistUCut->Fill(drifttime);
								fHToTDistUCut->Fill(tot);
								fHDrifttimeVsWireUCut->Fill(wire,drifttime);
								fHToTVsWireUCut->Fill(wire,tot);
								fHDrifttimeVsToTUCut->Fill(tot,drifttime);

								fHDrifttimeDistUTimeWalkCorr->Fill(timewalk_corrected_drifttime);
								fHDrifttimeVsWireUTimeWalkCorr->Fill(wire,timewalk_corrected_drifttime);
								fHDrifttimeVsToTUTimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);
							}
							else if(layer == 2)
							{
								fHitCounterX2Cut++;
								fHitCounterX2WireCut[wire]++;


								fHHitsInX2Cut->Fill(layer);
								fHHitRateEvtInX2Cut->SetBinContent(layer+1,double(fHitCounterX2Cut)/double(fEvtCounter));
								fHHitRateTimeInX2Cut->SetBinContent(layer+1,
										double(fHitCounterX2Cut)/fTimeInMicroSeconds*1000*1000);


								fHHitPatternX2Cut->Fill(wire);
								fHHitRateEvtX2Cut->SetBinContent(wire+1,double(fHitCounterX2WireCut[wire])/double(fEvtCounter));
								fHHitRateTimeX2Cut->SetBinContent(wire+1,
										double(fHitCounterX2WireCut[wire])/fTimeInMicroSeconds*1000*1000);
								fCondCountHitProcPassedX2->Test(true); /** weiterer X2-hit passiert */
								fHDrifttimeDistX2Cut->Fill(drifttime);
								fHToTDistX2Cut->Fill(tot);
								fHDrifttimeVsWireX2Cut->Fill(wire,drifttime);
								fHToTVsWireX2Cut->Fill(wire,tot);
								fHDrifttimeVsToTX2Cut->Fill(tot,drifttime);

								fHDrifttimeDistX2TimeWalkCorr->Fill(timewalk_corrected_drifttime);
								fHDrifttimeVsWireX2TimeWalkCorr->Fill(wire,timewalk_corrected_drifttime);
								fHDrifttimeVsToTX2TimeWalkCorr->Fill(tot,timewalk_corrected_drifttime);
							}
						} // END fBFillHistos

	
					}

					else
					{
						if(layer == 0) { fCondCountHitProcPassedX1->Test(false); }
						if(layer == 1) { fCondCountHitProcPassedU->Test(false); }
						if(layer == 2) { fCondCountHitProcPassedX2->Test(false); }
						fCondCountHitProcPassed->Test(false);
					}


				}
			}

			/** zuruecksetzen */
			//hasLeading[tdc][leadCh] = false;
			//leadingTime[tdc][leadCh] = -1;

			//leadingTimes[tdc][leadCh].pop_front();
			continue;
		}

		/** weder tdcTrigger / leading / trailing */
		else
		{
			continue;
		}

	}

	
}









//***********************************************************
// fuelle TQClamTrigger fuer TQClamHitEvent
void TQClamHitProc::AddTriggerHit(TQClamTrigger& trg, int tdc, int ch, double tLeadCorr, double tot)
{
	/** hier ueberschreibe ich aktuell alles, wenn mehrfach hits auf einem kanal vorkommen... TODO */
	trg.fired = true;
	trg.tdcID = tdc;
	trg.chNum = ch;
	trg.time = tLeadCorr;
	trg.tot.push_back(tot);
	trg.multiplicity++;
}










//***********************************************************
// Fulle RollinGraphs
//***********************************************************

//***********************************************************
// Aktualisiere Zeit zur Raten-Berechnung
bool TQClamHitProc::IntegrateTime(double clockCounter)
{
	if(fFirstEvent)
	{
		fTimeLastEvent = clockCounter;
		fFirstEvent = false;
		return false;
	}

	double dClock = clockCounter - fTimeLastEvent;

	if(dClock > 0)
	{
		double dt_sec = dClock * fClockPeriod;
		fIntegratedTime += dt_sec;
	}

	fTimeLastEvent = clockCounter;

	return true;
}

//***********************************************************
// Update Scaler-Stand: lastScaler + integratedScaler
void TQClamHitProc::UpdateRateChannel(RateChannel& ch, double scaler)
{
	if(ch.first)
	{
		ch.lastScaler = scaler;
		ch.first = false;
		return;
	}

	double dScaler = scaler - ch.lastScaler;
	ch.lastScaler = scaler;

	if(dScaler >= 0) { ch.dScalerSum += dScaler; }
}



//***********************************************************
// schreibe integrierte Rate in Graphen
void TQClamHitProc::FlushRateChannel(
		RateChannel& ch,
		double integratedTime,
		TGo4RollingGraph* graph,
		TGraph* gr,
		TH1* hist)
{
	if(integratedTime <= 0) { return; }

	double rate = ch.dScalerSum / integratedTime;


	if(fHistoPar->fBFillHistos)
	{
		if(graph) { graph->Fill(rate,0,0); }

		if(gr)
		{
			int n = gr->GetN();
			gr->SetPoint(n,fGraphTime,rate);
		}

		if(hist)  { hist->Fill(rate); }
	}


	ch.dScalerSum = 0.;
}



//***********************************************************
// berechne relative Totzeit und schreibe in Graphen
void TQClamHitProc::FlushDeadtime(RateChannel& chClock, RateChannel& chLivetime,
		TGo4RollingGraph* graph,
		TGraph* gr,
		TH1* hist)
{
	double live = chLivetime.dScalerSum;
	double total = chClock.dScalerSum;

	double deadtime = (total > 0 ? 1.0 - live/total : 0);

	if(fHistoPar->fBFillHistos)
	{
		if(graph) { graph->Fill(deadtime,0,0); }
		if(gr)
		{
			int n = gr->GetN();
			gr->SetPoint(n,fGraphTime,deadtime);
		}
		if(hist) { hist->Fill(deadtime); }
	}

	chLivetime.dScalerSum = 0.;
}


//***********************************************************
// schreibe integrierten Faraday-Cup Strom in Graphen (nach Kalibration)
void TQClamHitProc::FlushFaradayChannel(
        RateChannel& ch,
        double integratedTime)
{
	if(integratedTime <= 0) { return; }

	double freqMHz = (ch.dScalerSum / integratedTime) * 1e-6;
	double curr = fFaradayCupCurrentCaliPar->CorrectCurrent(freqMHz);

	
	if(fHistoPar->fBFillHistos)
	{
		fGFaradayCupCurrent->Fill(curr,0,0);
	
		int n = fGraphFaradayCupCurrent->GetN();
		fGraphFaradayCupCurrent->SetPoint(n,fGraphTime,curr);
	}


	ch.dScalerSum = 0.;
}













//***********************************************************
// pruefe Conditions und verwerfe ggf.
bool TQClamHitProc::ProofHitConditions(TQClamHit& hit)
{
	int layer = hit.layerID;

	/** ToT-Fenster pruefen */
	if(fHitConditionPar->ToTCondMode != TQClamHitConditionParameter::kOff)
	{
		/** Condition pruefen */
		bool inToTWindow = false;
		if(layer == 0)
		{
			((*fCondArrDrifttimeVsToT)[0])->Test(hit.tot,hit.drifttime);
			inToTWindow = ((*fCondArrToT)[0])->Test(hit.tot);
		}
		else if(layer == 1)
		{
			((*fCondArrDrifttimeVsToT)[1])->Test(hit.tot,hit.drifttime);
			inToTWindow = ((*fCondArrToT)[1])->Test(hit.tot);
		}
		else if(layer == 2)
		{
			((*fCondArrDrifttimeVsToT)[2])->Test(hit.tot,hit.drifttime);
			inToTWindow = ((*fCondArrToT)[2])->Test(hit.tot);
		}

		/** FlagOnly */
		if(fHitConditionPar->ToTCondMode == TQClamHitConditionParameter::kFlagOnly)
		{
			/** falls Condition nicht erfuellt: markieren und weitergeben */
			if(!inToTWindow)
			{
				hit.accepted = false;
				return true;
		       	}
		}

		/** Filter */
		if(fHitConditionPar->ToTCondMode == TQClamHitConditionParameter::kFilter)
		{
			if(!inToTWindow)
			{
				/** Ereignis markieren und verwerfen */
				hit.accepted = false;
				return false;
			}
		}
	}



	/** Driftzeit-Fenster pruefen */
	if(fHitConditionPar->DrifttimeCondMode != TQClamHitConditionParameter::kOff)
	{
		/** Condition pruefen */
		bool inDrifttimeWindow = false;
	        if(layer == 0) { inDrifttimeWindow = ((*fCondArrDrifttime)[0])->Test(hit.drifttime); }
		else if(layer == 1) { inDrifttimeWindow = ((*fCondArrDrifttime)[1])->Test(hit.drifttime); }
		else if(layer == 2) { inDrifttimeWindow = ((*fCondArrDrifttime)[2])->Test(hit.drifttime); }

		/** FlagOnly */
		if(fHitConditionPar->DrifttimeCondMode == TQClamHitConditionParameter::kFlagOnly)
		{
			/** falls Condition nicht erfuellt: markieren und weitergeben */
			if(!inDrifttimeWindow)
			{
				hit.accepted = false;
				return true;
			}
		}

		/** Filter */
		if(fHitConditionPar->DrifttimeCondMode == TQClamHitConditionParameter::kFilter)
		{
			if(!inDrifttimeWindow)
			{
				/** Ereignis markieren und verwerfen */
				hit.accepted = false;
				return false;
			}
		}
	}

	hit.accepted = true;
	return true;
}


























//***********************************************************
// schreibe Rohdaten in textfile
#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
void TQClamHitProc::WriteRawDataToFile(const TQClamRawEvent* raw)
#endif
#ifdef QCLAM_DAGOBERT
void TQClamHitProc::WriteRawDataToFile(const TQClamDagobertRawEvent* raw)
#endif
{
	static std::ofstream out(fHitProcDebugOutputPar->OutputFileRaw, std::ios::app);

	out << "-------------------------------------------------------------------------------------------------------" << "\n";
	out << "*** HEADER NEW EVENT ***" << "\n" << "\n";
	out << "*** EvtNo " << raw->eventNumber << " - TriggerID " << raw->triggerID << " - TriggerTime " << raw->triggerTime << " ***" << "\n" << "\n";
	out << "*** SCALERS IN EVENT ***" << "\n";
	//for(const auto& scaler : raw->scalers)
	{
		out << "*** Accepted " << raw->scaler.accepted << " - ScalerID " << raw->scaler.scalerID << " - TimeInMicroSeconds " << raw->scaler.timeMicroSeconds << "\n";
		out << "*** TDCTrigger " << raw->scaler.tdcTrigger << "\n"
			<< " - SzintillatorePmtUp " << raw->scaler.szintillatorPmtUp
			<< " - SzintillatorPmtDown " << raw->scaler.szintillatorPmtDown
			<< " - CherenkovPmt " << raw->scaler.cherenkovPmt << "\n"
			<< " - CoincidenceTrigger " << raw->scaler.coincidenceTrigger
			<< " - AcceptedTrigger " << raw->scaler.acceptedTrigger << "\n"
			<< " - ClockCounter " << raw->scaler.clockCounter
			<< " - LivetimeCounter " << raw->scaler.livetimeCounter
			<< " - MbsLivetimeCounter " << raw->scaler.mbsLivetimeCounter << "\n"
			<< " - FaradayCupChargeCounter " << raw->scaler.faradayCupChargeCounter << " ***" << "\n";

		out << "*** ErrorTDCTrigger " << raw->scaler.errorTdcTrigger << "\n"
			<< " - ErrorSzintillatorPmtUp " << raw->scaler.errorSzintillatorPmtUp
			<< " - ErrorSzintillatorPmtDown " << raw->scaler.errorSzintillatorPmtDown
			<< " - ErrorCherenkovPmt " << raw->scaler.errorCherenkovPmt << "\n"
			<< " - ErrorCoincidenceTrigger " << raw->scaler.errorCoincidenceTrigger
			<< " - ErrorAcceptedTrigger " << raw->scaler.errorAcceptedTrigger << "\n"
			<< " - ErrorClockCounter " << raw->scaler.errorClockCounter
			<< " - ErrorLivetimeCounter " << raw->scaler.errorLivetimeCounter
			<< " - ErrorMbsLivetimeCounter " << raw->scaler.errorMbsLivetimeCounter << "\n"
			<< " - ErrorFaradayCupChargeCounter " << raw->scaler.errorFaradayCupChargeCounter << " ***" << "\n" << "\n";
	}

	out << "*** HITS IN EVENT ***" << "\n";
	for(const auto& hit : raw->hits)
	{
		out << "*** TDCID " << hit.tdcID
			<< " - Channel " << hit.channel
			<< " - RawTimeInNanoSeconds " << hit.rawTimeInNanoSeconds << " ***" << "\n";
	}


	out << "-------------------------------------------------------------------------------------------------------" << "\n";
	out << "\n" << "\n";
}




//***********************************************************
// hole Driftzeitplot je nach ebenen-ID
TH2* TQClamHitProc::GetDrifttimeHistForLayer(int layer)
{
	switch(layer)
	{
		case 0: return fHDrifttimeVsWireX1TimeWalkCorr;
		case 1: return fHDrifttimeVsWireUTimeWalkCorr;
		case 2: return fHDrifttimeVsWireX2TimeWalkCorr;
		default: return nullptr;
	}
}



//***********************************************************
// fitte an driftzeit-spektren zur offset-kalibrierung
void TQClamHitProc::FitDrifttimeSpectra()
{
	TGo4Log::Info("Fitte an Driftzeit-Verteilungen");
	if(!fDrifttimeCalibPar) { return; }

	for(size_t l=0; l<fDrifttimeCalibPar->MAX_LAYER; l++)
	{
		if(!fDrifttimeCalibPar->fDoFit[l]) { continue; }
		TGo4Log::Info("DoFit");

		TH2* h2 = GetDrifttimeHistForLayer(l);
		if(!h2) { continue; }

		int nWires = fDrifttimeCalibPar->fNumWires[l];

		for(size_t wire=0; wire<nWires; ++wire)
		{
			/** projektion erzeugen */
			TH1* proj = h2->ProjectionY(Form("dt_layer%ld_wire%ld",l,wire),wire,wire);

			if(!proj || proj->GetEntries() < 20) { continue; }

			if(fDrifttimeCalibPar->fUseFermiDirac)
			{
				/** nur bis Maximum der Driftzeit-Verteilung fitten --> hole Maximum-Bin */
				Int_t maxbin = proj->GetMaximumBin();
				Double_t xmax = proj->GetBinCenter(maxbin);
				/** xmin */
				Double_t xmin = proj->GetXaxis()->GetXmin();
				TGo4Log::Info("Ranges xmin: %f, xmax: %f",xmin,xmax);
	
				/** fitter auf projektion setzen */
				auto* data = fDrifttimeCalibPar->fFitter->SetH1(__DATANAME_DRIFTDIST__,proj,kFALSE);
				fDrifttimeCalibPar->fFitter->AssignModelTo("FermiDirac",__DATANAME_DRIFTDIST__);

				if(data) { data->SetRange(0,xmin,xmax); }
	
				/** startwerte setzen */
				fDrifttimeCalibPar->fFitter->SetParValue("FermiDirac.A",proj->GetMaximum());
				fDrifttimeCalibPar->fFitter->SetParValue("FermiDirac.t0",fDrifttimeCalibPar->fStartT0);
				fDrifttimeCalibPar->fFitter->SetParValue("FermiDirac.tau",fDrifttimeCalibPar->fStartTau);
				fDrifttimeCalibPar->fFitter->SetParValue("FermiDirac.T",fDrifttimeCalibPar->fStartT);
	
				/** fitten */
				fDrifttimeCalibPar->fFitter->DoActions();
	
				fDrifttimeCalibPar->fA[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("FermiDirac.A");
				fDrifttimeCalibPar->fT0[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("FermiDirac.t0");
				fDrifttimeCalibPar->fTau[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("FermiDirac.tau");
				fDrifttimeCalibPar->fT[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("FermiDirac.T");
	
				if(fDrifttimeCalibPar->fDoPlotFit)
				{
					auto* fitcurve = dynamic_cast<TH1*>(fDrifttimeCalibPar->fFitter->CreateDrawObject("FitResult",__DATANAME_DRIFTDIST__,kTRUE,"FermiDirac"));
	
					if(fitcurve)
					 {
						TCanvas c("c","c",800,600);
						fitcurve->SetLineColor(kRed);
						proj->Draw();
						fitcurve->Draw("same");
	
						c.SaveAs(Form("../data/drifttime_offset_fit/fit_fermidirac_L%ld_W%ld.png",l,wire));
					}
				}
			}
			fDrifttimeCalibPar->fFitter->ClearModelAssignmentTo("FermiDirac",__DATANAME_DRIFTDIST__);


			if(fDrifttimeCalibPar->fUseSkewed)
			{
				/** xmax */
				Double_t xmax = proj->GetXaxis()->GetXmax();
				/** xmin */
				Double_t xmin = proj->GetXaxis()->GetXmin();
				TGo4Log::Info("Ranges xmin: %f, xmax: %f",xmin,xmax);
	
				/** fitter auf projektion setzen */
				auto* data = fDrifttimeCalibPar->fFitter->SetH1(__DATANAME_DRIFTDIST__,proj,kFALSE);
				fDrifttimeCalibPar->fFitter->AssignModelTo("DrifttimeDist",__DATANAME_DRIFTDIST__);

				if(data) { data->SetRange(0,xmin,xmax); }
	
				/** startwerte setzen */
				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.A0",proj->GetMaximum());
				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.mu0",fDrifttimeCalibPar->fStartMu0);
				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.sigma0",fDrifttimeCalibPar->fStartSigma0);
				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.s0",fDrifttimeCalibPar->fStartS0);

				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.A1",proj->GetMaximum());
				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.mu1",fDrifttimeCalibPar->fStartMu1);
				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.sigma1",fDrifttimeCalibPar->fStartSigma1);
				fDrifttimeCalibPar->fFitter->SetParValue("DrifttimeDist.s1",fDrifttimeCalibPar->fStartS1);
	
				/** fitten */
				fDrifttimeCalibPar->fFitter->DoActions();
	
				fDrifttimeCalibPar->fA0[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.A0");
				fDrifttimeCalibPar->fMu0[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.mu0");
				fDrifttimeCalibPar->fSigma0[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.sigma0");
				fDrifttimeCalibPar->fS0[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.s0");

				fDrifttimeCalibPar->fA1[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.A1");
				fDrifttimeCalibPar->fMu1[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.mu1");
				fDrifttimeCalibPar->fSigma1[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.sigma1");
				fDrifttimeCalibPar->fS1[l][wire] = fDrifttimeCalibPar->fFitter->GetParValue("DrifttimeDist.s1");
	
	
				if(fDrifttimeCalibPar->fDoPlotFit)
				{
					auto* fitcurve = dynamic_cast<TH1*>(fDrifttimeCalibPar->fFitter->CreateDrawObject("FitResult",__DATANAME_DRIFTDIST__,kTRUE,"DrifttimeDist"));
	
					if(fitcurve)
					 {
						TCanvas c("c","c",800,600);
						fitcurve->SetLineColor(kRed);
						proj->Draw();
						fitcurve->Draw("same");
	
						c.SaveAs(Form("../data/drifttime_offset_fit/fit_skewed_L%ld_W%ld.png",l,wire));
					}
				}
			}

			fDrifttimeCalibPar->fFitter->ClearModelAssignmentTo("DrifttimeDist",__DATANAME_DRIFTDIST__);


			delete proj;
		}
	}

	TGo4Log::Info("Alle Driftzeit-Spektrum Fits durchgefuehrt");
}















//***********************************************************
// Erzeuge Hits

/*
void TQClamHitProc::ProcessEvent(TQClamRawEvent* raw)
{
	fSubEvents.clear();
	fCurrentSubEvent = SubEvent{};

	for(int tdc=0; tdc<QCLAM_MAX_TDC; tdc++)
	{
		fTriggerIndex[tdc] = -1;
		fCurrentSubEvent.hasTdcTrigger[tdc] = false;

		for(int ch=0; ch<QCLAM_MAX_CH; ch++)
		{
			hasLeading[tdc][ch] = true;
		}
	}

	auto ensureSubEvent = [this](int N)
	{
		if(static_cast<int>(fSubEvents.size()) <= N)
		{
			fSubEvents.resize(N+1);
		}
	};

	for(const RawHit& hit : raw->hits)
	{
		if(!hit.accepted) { continue; }

		int tdc = hit.tdcID;
		int ch = hit.channel;
		double t = hit.rawTimeInNanoSeconds;

		bool isLead = fEdgeMappingPar->invertEdges ? fEdgeMappingPar->IsTrailing(tdc,ch)
			: fEdgeMappingPar->IsLeading(tdc,ch);
		bool isTrail = fEdgeMappingPar->invertEdges ? fEdgeMappingPar->IsLeading(tdc,ch)
			: fEdgeMappingPar->IsTrailing(tdc,ch);

		if(fEdgeMappingPar->IsTdcTrigger(tdc,ch))
		{
			++fTriggerIndex[tdc];
			int N = fTriggerIndex[tdc];

			ensureSubEvent(N);


			fSubEvents[N].tdcTriggerTime[tdc] = t;
			fSubEvents[N].hasTdcTrigger[tdc] = true;

			for(int c=0; c<QCLAM_MAX_TDC; c++)
			{
				hasLeading[tdc][c] = false;
			}


			continue;
		}




		else if(isLead)
		{
			leadingTime[tdc][ch] = t;
			hasLeading[tdc][ch] = true;
			continue;
		}




		else if(isTrail)
		{
			int leadCh = fEdgeMappingPar->GetLeadingForTrailing(tdc,ch);

			if(leadCh<0 || !hasLeading[tdc][leadCh]
					|| !fTriggerCaliPar->ProofHit(hasTdcTrigger[tdc]))
			{
				continue;
			}

			int N = fTriggerIndex[tdc];
			if(N<0) { continue; }

			ensureSubEvent(N);
			SubEvent& sub = fSubEvents[N];

			// 1. brauche keinen tdc-trigger
			// --> egal ob tdc-Trigger vorliegt oder nicht
			// --> weiter
			//  2. brauche tdc-trigger
			// --> i) habe bereits tdc-trigger --> weiter
			// --> ii) habe noch keinen tdc-trigger -> continue 
			bool hitOk = fTriggerCaliPar->ProofHit(sub.hasTdcTrigger[tdc]);
			if(!hitOk) { continue; }


			double tTdcTrigger = (sub.hasTdcTrigger[tdc] ? sub.tdcTriggerTime[tdc] : 0);
			double tLead = leadingTime[tdc][leadCh];
			double tTrail = t;

			double tot = tTrail - tLead;
			double tLeadCorr = tLead - tTdcTrigger;


			if(fTriggerCaliPar->IsSzintPmtUp(tdc,leadCh))
			{
				sub.isSzintPmtUp = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsSzintPmtDown(tdc,leadCh))
			{
				isSzintPmtDown = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsCherenkovPmt(tdc,leadCh))
			{
				isCherenkovPmt = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsCoincTrig(tdc,leadCh))
			{
				isCoincTrig = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsAccTrig(tdc,leadCh))
			{
				isAccTrig = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsClock(tdc,leadCh))
			{
				isClock = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsLivetime(tdc,leadCh))
			{
				isLivetime = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsMbsLivetime(tdc,leadCh))
			{
				isMbsLivetime = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}
			else if(fTriggerCaliPar->IsFaradayCupChargeCounter(tdc,leadCh))
			{
				isFaradayCupChargeCount = true;
				AddTriggerHit(sub,tdc,leadCh,tLeadCorr,tot);
			}


			else if(fDriftchamCaliPar->HasMapping(tdc,leadCh))
			{
				auto [layer,wire] = fDriftchamCaliPar->GetWire(tdc,leadCh);

				if(layer >= 0 && wire >= 0 && fDriftchamCaliPar->IsWireActive(layer,wire))
				{
					double drifttime = fDriftchamCaliPar->ApplyTimeOffsets(tLeadCorr,tdc,leadCh,layer,wire);

					TQClamHit h;
					h.accepted = true;
					h.tdcID = tdc;
					h.chNum = leadCh;
					h.layerID = layer;
					h.wireNum = wire;
					h.absoluteTime = tLeadCorr;
					h.drifttime = drifttime;
					h.driftlength = 0.0;
					h.tot = tot;
					h.fWire = nullptr;

					if(ProofHitConditions(h)) { sub.hits.push_back(h); }
				}
			}

			hasLeading[tdc][ch] = false;
			continue;
		}

		else
		{
			continue;
		}

	}

	
}


*/
