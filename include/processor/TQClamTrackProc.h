#ifndef TQCLAMTRACKPROC_H
#define TQCLAMTRACKPROC_H

#include "TGo4EventProcessor.h"

/** geometry */
#include "geometry/TQClamDriftcham.h"
#include "geometry/TQClamWire.h"

/** Go4 */
class TGo4WinCond;

/** ROOT */
#include "Rtypes.h"
class TClass;
class TH1;
class TH2;
class TProfile;
class TGraph;

/** parameter */
class TQClamTrackConditionParameter;
class TQClamDriftToZOffsetParameter;
class TQClamFocalPlaneParameter;
class TQClamSpectrumCalibrationParameter;
class TQClamFitParameter;
class TQClamClusterConditionParameter;
class TQClamHistogramParameter;
class TQClamSieveSlitCalibParameter;

class TGo4Fitter;

/** evts */
struct TQClamCluster;
class TQClamTrackEvent;

/** hilfsstrukturen */
struct ZOffsetInput;
struct ZOffsetResult;

/** benoetigte Struktur aus TrackEvent */
struct OutlierRejectionSnapshot;
struct IterationSummary;


#include "TTree.h"


#include <vector>
#include <utility>
#include <bitset>



enum class ETrackingMode
{
	None,
	X1_U_X2,
	X1_X2,
	X1Only,
	X2Only
};


/** -- interne Strukturen --
 *
 * LayerRawData
 *  |
 * LayerTrackingState
 *  |
 * IterationSnapshot (pro Iteration)
 *  |
 * ResidualData (pro Hit)
 *  |
 * FitResiduals (global)
 */





/**
 * Von ClusterProc und Drikftkammer-Geometrie uebernommene Rohdaten
 */
struct LayerRawData /** Daten zu Ebene aus entsprechendem Cluster */
{
	int layerID; /** Ebenen-ID */
	WirePlaneType type; /** X-/U-Ebene */
	double zPosLayer; /** z-Pos Ebene */

	/** Cluster-Eigenschaften */
	bool hasGaps; /** Cluster hat luecken? */
	bool hasVShape; /** Cluster V-Foem? */
	bool isAscending; /** Driftzeiten vs. Drahtnummer steigend? */
	bool isDescending; /** Driftzeiten vs. Drahtnummer fallend? */

	size_t nHits; /** anzahl hits in cluster */

	/** Eigenschaften Ebene */
	double normalWireDist; /** normaler Abstand zwischen Draehten */
	double wireAngle; /** Drahtwinkel in x-y ebene zu x-achse */

	/** Rohdaten hits in cluster */
	std::vector<int> wireNumbers; /** Drahtnummer */
	std::vector<double> normalCoords; /** Normalkoordinaten der Draehte (x/u) sortiert nach Drahtnummer */
	std::vector<double> drifttimes; /** Driftzeiten/ns sortiert nach Drahtnumer */
	std::vector<double> tot; /** tot/ns sortiert nach Drahtnumer */

	/** stdw Rohdaten */
	std::vector<double> sigmaNormalCoords; /** Stdw Normalkoordinaten */
	std::vector<double> sigmaDrifttimes; /** Stdw Driftzeiten */

	/** erster Guess */
//	double xMidFirstGuess; /** Cluster x-Mittelpunkt - Falls X-Ebene */
//	double uMidFirstGuess; /** Cluster y-Mittelpunkt - Falls U-Ebene */

	double normalCoordIntersecFirstGuess = 0.;
};


struct HitResidual /** Residuen */
{
	double wireCoord = 0.; /** drahtposition */
	double measuredDrift = 0.; /** zOffset aus Driftzeit */
	double trackDrift = 0.; /** abstand track -> draht (spurversatz) */
	double residual = 0.; /** trackDrift - measuredDrift --> signed */

	/** stdws */
	double sigmaMeasured = 0.;
	double sigmaTrack = 0.;
	double sigmaResidual = 0.;
};

/**
 * komplette fit-unsicherheit je Hit
 */
struct ResidualData
{
	size_t hitIndex = 0; // hit-index in cluster, geht mit drahtnummer

	int layerID = -1;
	int wireNumber = -1;

	double wireCoord = NAN;
	double drifttime = NAN;
	double tot = NAN;
	double measuredZOffset = NAN;
	double trackDistance = NAN;
	double sigma_measuredZOffset = NAN;
	double sigma_trackDistance = NAN;

	double residual = 0.;
	double sigmaResidual = 0.;

	double pull = 0.; // residual/sigma_residual
	bool isOutlier = false; // hit ein outliert? --> ignoriere ihn fuer track-fitting
};

/**
 * Zustand einer Iteration pro Ebene
 */
struct IterationSnapshot
{
	std::vector<double> zOffsets;
	std::vector<double> sigmaZOffsets;

	std::vector<ResidualData> residuals; // Hits dieser Iteration

	double chi2 = NAN;
	double chi2ndf = NAN; /** chi2 / ndf */
	int nRejected = 0;
};


/**
 * komplette historie
 */
struct LayerIterationData
{
	std::vector<IterationSnapshot> history;
	/*
	std::vector<double> zOffsets;
	std::vector<double> sigmaZOffsets;

	std::vector<HitResidual> residuals;

	double chi2 = NAN;
	double residualMean = NAN;
	*/
};

/**
 * Input fuer jeweils naechste Iteration der Schnittpunkt-Bestimmung: ComputeIntersection()
 */
struct IntersectionInput 
{
	int layerID;
	WirePlaneType type;
	size_t nHits;

	bool hasGaps;
	bool hasVShape;
	bool isAscending;
	bool isDescending;

	double normalWireDist;
	std::vector<double> normalCoords;
	std::vector<double> drifttimes;
	std::vector<double> zOffsets;

	double phi;
	double theta;

	/** optional stdws */
	std::vector<double> sigmaNormalCoords;
	std::vector<double> sigmaDrifttimes;
	std::vector<double> sigmaZOffsets;

	double sigmaPhi;
	double sigmaTheta;


	double previousIntersection = NAN;
	bool hasPreviousIntersection = false;
};



/**
 * Ergebnis der Schnittpunktsberechnung -> TrackFit
 */
struct IntersectionResult
{
	bool valid = false;

	double intersection = NAN; /** normale Koordinate */
	double sigmaIntersection = NAN;

	double phi = NAN; /** x-z Winkel/rad */
	double sigmaPhi = NAN;

	double quality = NAN;
};

/**
 * 'Live'-Zustand eines Cluster (in einer Ebene) waehrend Fit
 * - LayerRawData -> speichert die uebergebenen 'Roh'-Daten
 * - LayerIterationData -> Fit-Historie
 * - IntersectionResult -> aktuelles Fit-Ergebnis
 * - hitIsOutlier -> Hit mit ID = Index im vector ist Outlier (-> schlechter hit ausgeschlossen)
 */
struct LayerTrackingState
{
	const LayerRawData* raw = nullptr;

	LayerIterationData iter;

	IntersectionResult result;

	std::vector<Bool_t> hitIsOutlier;
};




/**
 * globale (ueber alle Ebenen gesehen) Fit-Statistik
 */
struct FitQuality
{
	double chi2 = NAN;
	double chi2ndf = NAN; // chi/ndf
	
	int nIterations = 0;
	int totalRejected = 0;
	bool converged = false;

	double maxResidualSigned = NAN;
	double maxResidualAbs = NAN;
	double meanResidual = NAN;
	double rmsResidual = NAN;

	double meanResidual_layer[3] = {NAN,NAN,NAN};
	double rmsResidual_layer[3] = {NAN,NAN,NAN};
	double sigmaResidual_layer[3] = {NAN,NAN,NAN};
	int nResiduals_layer[3] = {0,0,0};
};

/**
 * Endergebnis des Trackfits
 * -> alle berechneten Schnittpunkte, Fitqualitaet (struct FitQuality)
 */
struct FitResults
{
	double phi, theta;
	double sigmaPhi, sigmaTheta;

	double X1D, Y1D, Z1D;
	double sigmaX1D, sigmaY1D, sigmaZ1D;

	//double residual; /** abweichung rekonstruierte Trajektorie von berechneten driftweg */
	//double chi2;
	size_t iterations;
	bool valid = false;

	FitQuality quality;
	std::vector<IterationSummary> iterationSummaries;
};








class TQClamTrackProc : public TGo4EventProcessor
{
	public:
		TQClamTrackProc();
		TQClamTrackProc(const char* name);
		virtual ~TQClamTrackProc();


		/** pruefe klasse von output-eventobjekt */
		Bool_t CheckEventClass(TClass* cl) override;

		/** baue output-eventobjekt */
		Bool_t BuildEvent(TGo4EventElement* dest) override;




	private:
		/** Geometrie benoetigt? */
		const TQClamDriftcham& fDriftcham;

		Int_t fFitCounter{0};

		Int_t fEvtCounter{0};
		Int_t fConvergedTrackCounter{0};
		Int_t fNotConvergedTrackCounter{0};

		int nRejHitVecSize = 50;
		int nUsedHitVecSize = 150;

		// Counter
		Int_t fBeamMonitorCounter{0};
		Int_t fBeamMonitorWindow{10000};

		// Hilfsvariablen für Statistik
		double fBeamSumX{0.};
		double fBeamSumY{0.};

		double fBeamSumX2{0.};
		double fBeamSumY2{0.};

		Int_t fBeamStatCounter{0};

		// zaehler fuer rejected hits
		std::vector<Int_t> fRejectedCounter;
		std::vector<Int_t> fRejectedCounter_X1;
		std::vector<Int_t> fRejectedCounter_U;
		std::vector<Int_t> fRejectedCounter_X2;

		// zaehler fuer used hits
		std::vector<Int_t> fUsedCounter;
		std::vector<Int_t> fUsedCounter_X1;
		std::vector<Int_t> fUsedCounter_U;
		std::vector<Int_t> fUsedCounter_X2;


		// zeitberechnung
		ULong64_t fStartTime{0};
                Double_t fGraphTime{0.};
                Bool_t fFirstTime{kTRUE};



                //-----------------------------------------------------------
                // 1. Parameter
		TQClamTrackConditionParameter* fTrackConditionPar{nullptr};
		TQClamDriftToZOffsetParameter* fDriftToZOffsetPar{nullptr};
		TQClamFocalPlaneParameter* fFocalPlanePar{nullptr};
		TQClamSpectrumCalibrationParameter* fSpectrumCaliPar{nullptr};
		TQClamFitParameter* fFitPar{nullptr};
		TQClamHistogramParameter* fHistoPar{nullptr};
		TQClamSieveSlitCalibParameter* fSieveSlitCalibParam{nullptr};

		TQClamClusterConditionParameter* fClusterPar{nullptr}; // zum prüfen der Cluster-Groesse nach entfernen von outsider-hits

                //-----------------------------------------------------------
                // 2. Conditions
		TGo4WinCond* fCondPull{nullptr};
		TGo4WinCond* fSieveSlitGate{nullptr};

                //-----------------------------------------------------------
                // 3. Histogramme
		TH2* fHDriftDistanceTrackVsTime_X1{nullptr};
		TH2* fHDriftDistanceTrackVsTime_U{nullptr};
		TH2* fHDriftDistanceTrackVsTime_X2{nullptr};
		TH2* fHDriftDistanceTrackVsTime_All{nullptr};

		TH2* fHDriftDistanceMeasVsTime_X1{nullptr};
		TH2* fHDriftDistanceMeasVsTime_U{nullptr};
		TH2* fHDriftDistanceMeasVsTime_X2{nullptr};
		TH2* fHDriftDistanceMeasVsTime_All{nullptr};


		TH1* fHTrackEfficiency{nullptr};
		TH1* fHXSpectrumX1Plane{nullptr};
		TH1* fHYSpectrumX1Plane{nullptr};
		TH1* fHXSpectrumUPlane{nullptr};
		TH1* fHYSpectrumUPlane{nullptr};
		TH1* fHXSpectrumX2Plane{nullptr};
		TH1* fHUSpectrum{nullptr};

		TH1* fHProjSpectrum{nullptr}; // in Fokalebene projiziert

		/** kalibrierte Spektren */
		TH1* fHXCorrSpectrum{nullptr};
		TH1* fHExEnergySpectrum{nullptr};


		/** Winkel Detektorsystem */
		TH1* fHPhiSpectrum{nullptr};
		TH1* fHThetaSpectrum{nullptr};
		TH1* fHPhiCorrSpectrum{nullptr};

		TH2* fHPhiVsTheta{nullptr};

		/** Streuwinkel */
		TH1* fHScattTheta{nullptr};
		TH1* fHScattPhi{nullptr};



		// -----------------------------------------------------------
		// Beam position monitor
		// -----------------------------------------------------------

		TH1* fHBeamX_All{nullptr};
		TH1* fHBeamY_All{nullptr};
		TH2* fHBeamXY_All{nullptr};

		TH1* fHBeamX_Live{nullptr};
		TH1* fHBeamY_Live{nullptr};
		TH2* fHBeamXY_Live{nullptr};

		// running mean/std
		TGraph* fPBeamX_Mean{nullptr};
		TGraph* fPBeamY_Mean{nullptr};

		TProfile* fPBeamX_RMS{nullptr};
		TProfile* fPBeamY_RMS{nullptr};


		TH1* fHScattXPos{nullptr};
		TH1* fHScattYPos{nullptr};
		TH2* fHScattXYPos{nullptr};

		/** 2D: Winkel Detektorsystem vs. Detektorkoordinaten */
		TH2* fHPhiVsX1{nullptr};
		TH2* fHPhiVsX2{nullptr};
		TH2* fHThetaVsY{nullptr};
		TH2* fHPhiVsY{nullptr};
		TH2* fHPhiCorrVsY{nullptr};

		TH2* fHThetaVsPhi{nullptr};
		TH2* fHYVsX{nullptr};

		TH2* fHPhiCorrVsX1Corr{nullptr};

		/** 2D: Streuwinkel vs. ...*/
		TH2* fHScattPhiVsScattTheta{nullptr};

		TH2* fHScattPhiVsX1{nullptr};
		TH2* fHScattPhiVsXCorr{nullptr};
		TH2* fHScattPhiVsY{nullptr};

		TH2* fHScattThetaVsX1{nullptr};
		TH2* fHScattThetaVsXCorr{nullptr};
		TH2* fHScattThetaVsY{nullptr};

		TH2* fHScattPhiVsPhi{nullptr};
		TH2* fHScattPhiVsPhiCorr{nullptr};
		TH2* fHScattThetaVsTheta{nullptr};



		// Konvergenz via Iteration beobachten
		TH2* fHDPhiVsIteration{nullptr};
		TH2* fHRmsVsIteration{nullptr};

		TH1* fHPhi_PhiX1{nullptr};
		TH1* fHPhi_PhiU{nullptr};
		TH1* fHPhi_PhiX2{nullptr};

		TH1* fHFitSrc{nullptr};
		TH1* fHFitRes{nullptr};

		TH1* fHChi2{nullptr};
		TH1* fHChi2NDF{nullptr};
		TH1* fHMaxResidualSigned{nullptr};
		TH1* fHMaxResidualAbs{nullptr};
		TH1* fHMeanResidual{nullptr};

		TH1* fHConverged{nullptr};
		TH1* fHIterations{nullptr};
		TH1* fHRejected{nullptr};

		TH1* fHRejectedHits_X1{nullptr};
		TH1* fHRejectedHits_U{nullptr};
		TH1* fHRejectedHits_X2{nullptr};

		TH1* fHUsedHits_X1{nullptr};
		TH1* fHUsedHits_U{nullptr};
		TH1* fHUsedHits_X2{nullptr};

		TH1* fHPull{nullptr};
		TH2* fHResidualVsWireX1{nullptr};
		TH2* fHResidualVsWireU{nullptr};
		TH2* fHResidualVsWireX2{nullptr};

		TH1* fHMeanResidual_X1{nullptr};
		TH1* fHMeanResidual_U{nullptr};
		TH1* fHMeanResidual_X2{nullptr};

		TH1* fHRmsResidual_X1{nullptr};
		TH1* fHRmsResidual_U{nullptr};
		TH1* fHRmsResidual_X2{nullptr};

		TH1* fHX1Resolution_X1{nullptr};
		TH1* fHX1Resolution_U{nullptr};
		TH1* fHX1Resolution_X2{nullptr};

		TH1* fHResidual_X1{nullptr};
		TH1* fHResidual_U{nullptr};
		TH1* fHResidual_X2{nullptr};


		TH1* fHResidual_X1_phiBin_25_35{nullptr};
		TH1* fHResidual_U_phiBin_25_35{nullptr};
		TH1* fHResidual_X2_phiBin_25_35{nullptr};

		TH1* fHResidual_X1_phiBin_35_45{nullptr};
		TH1* fHResidual_U_phiBin_35_45{nullptr};
		TH1* fHResidual_X2_phiBin_35_45{nullptr};

		TH1* fHResidual_X1_phiBin_45_55{nullptr};
		TH1* fHResidual_U_phiBin_45_55{nullptr};
		TH1* fHResidual_X2_phiBin_45_55{nullptr};

		TH1* fHResidual_X1_phiBin_55_65{nullptr};
		TH1* fHResidual_U_phiBin_55_65{nullptr};
		TH1* fHResidual_X2_phiBin_55_65{nullptr};



		/// Driftzeit-Bins 
		TH1* fHResidual_X1_drifttimeBin_0_50{nullptr};
		TH1* fHResidual_U_drifttimeBin_0_50{nullptr};
		TH1* fHResidual_X2_drifttimeBin_0_50{nullptr};

		TH1* fHResidual_X1_drifttimeBin_50_100{nullptr};
		TH1* fHResidual_U_drifttimeBin_50_100{nullptr};
		TH1* fHResidual_X2_drifttimeBin_50_100{nullptr};

		TH1* fHResidual_X1_drifttimeBin_100_150{nullptr};
		TH1* fHResidual_U_drifttimeBin_100_150{nullptr};
		TH1* fHResidual_X2_drifttimeBin_100_150{nullptr};

		TH1* fHResidual_X1_drifttimeBin_150_200{nullptr};
		TH1* fHResidual_U_drifttimeBin_150_200{nullptr};
		TH1* fHResidual_X2_drifttimeBin_150_200{nullptr};

		TH1* fHResidual_X1_drifttimeBin_200_250{nullptr};
		TH1* fHResidual_U_drifttimeBin_200_250{nullptr};
		TH1* fHResidual_X2_drifttimeBin_200_250{nullptr};

		TH1* fHResidual_X1_drifttimeBin_250_300{nullptr};
		TH1* fHResidual_U_drifttimeBin_250_300{nullptr};
		TH1* fHResidual_X2_drifttimeBin_250_300{nullptr};


		// Rekonstruktionsgenauigkeit
		TH1* fHZResolution_X1{nullptr};
		TH1* fHZResolution_U{nullptr};
		TH1* fHZResolution_X2{nullptr};

		TH1* fHXResolution_X1{nullptr};
		TH1* fHXResolution_X2{nullptr};

		TH1* fHUResolution_U{nullptr};
		TH1* fHYResolution_U{nullptr};





		TProfile* fProfMeanResVsIter{nullptr};  // TODO


		TGo4Fitter* fFitter{nullptr};


                //-----------------------------------------------------------
		// 4. Picture
		TGo4Picture* fPDriftDistanceVsTime{nullptr};
		TGo4Picture* fPScattPosMon{nullptr};
		TGo4Picture* fPDispSpectra{nullptr};
		TGo4Picture* fPNonDispSpectra{nullptr};
		TGo4Picture* fPFocal1D{nullptr};
		TGo4Picture* fPSieveSlitSpectra{nullptr};
		TGo4Picture* fPMatrixSpectra{nullptr};
		TGo4Picture* fPTrackResiduals{nullptr};
		TGo4Picture* fPTrackResidualsLayer{nullptr};
		TGo4Picture* fPTrackResidualsPhiBins{nullptr};
		TGo4Picture* fPTrackResidualsDrifttimeBins{nullptr};
		TGo4Picture* fPTrackResolution{nullptr};



	private:

		/** Methoden */
		Bool_t ProcessTrack(ETrackingMode mode,TQClamTrackEvent* evt,const std::vector<LayerRawData>& layers,std::vector<LayerTrackingState>& states);

		ETrackingMode DetermineTrackingMode(const std::bitset<3>& hasCluster) const;
		std::vector<LayerRawData> ExtractLayerRawData(const std::vector<const TQClamCluster*>& clusters) const;
		std::pair<std::pair<double,double>,std::pair<double,double>> EstimateStartAngles(const std::vector<LayerRawData>& layers) const;
		std::vector<LayerTrackingState> PrepareTrackingStates(const std::vector<LayerRawData>& layers) const;

		//std::pair<std::pair<double,double>,std::pair<double,double>> ComputeAngles(const std::vector<LayerTrackingState>& states) const;
		std::pair<std::pair<double,double>,std::pair<double,double>> ComputeAngles(const std::vector<LayerTrackingState>& states, ETrackingMode mode) const;
		const LayerTrackingState& GetULayer(const std::vector<LayerTrackingState>& states) const;


		//IntersectionInput MakeInput(const LayerTrackingState& S, const FitResults& R) const;
		IntersectionInput MakeInput(const LayerTrackingState& S, double phi, double theta) const;

		//void IteratePhiTheta(FitResults& R, std::vector<LayerTrackingState>& states) const;
		//void IteratePhiTheta(FitResults& R, std::vector<LayerTrackingState>& states, ETrackingMode mode) const;
		void IteratePhiTheta(FitResults& R, std::vector<LayerTrackingState>& states, ETrackingMode mode, std::vector<OutlierRejectionSnapshot>& outlierRejectionSnapshots) const;

		double ComputeSignedDistanceToWire(double wireCoord, double trackIntersection, double phi) const;

		IntersectionResult ComputeIntersection(const IntersectionInput& in) const;

		IntersectionResult ComputeCross2(const IntersectionInput& in) const;
//		IntersectionResult ComputeCross3(const IntersectionInput& in) const;
		IntersectionResult ComputeMIT(const IntersectionInput& in) const;
		IntersectionResult ComputeCross4(const IntersectionInput& in) const;
		IntersectionResult ComputeRegression(const IntersectionInput& in) const;

		Bool_t PassTrackConditions(const TQClamTrackEvent* evt) const;

		void DoProjection(TQClamTrackEvent* evt);

		void CalibrateSpectrum(TQClamTrackEvent* evt);

		void FillEventFromStates(TQClamTrackEvent* evt, const FitResults& R, const std::vector<LayerTrackingState>& states) const;
		void FillFinalTrackParameters(TQClamTrackEvent* evt, const FitResults& R, const std::vector<LayerTrackingState>& states, ETrackingMode mode) const;
		void FillFitQualityHistograms(const FitResults& R);
		void FillRejectedHitHistograms(const std::vector<LayerTrackingState>& states);
		void FillHistograms(const TQClamTrackEvent* evt);

		void FillBeamPositionMonitor(const TQClamTrackEvent* evt);
		void UpdateBeamPositionMonitor();

	ClassDefOverride(TQClamTrackProc,1);
};

#endif
