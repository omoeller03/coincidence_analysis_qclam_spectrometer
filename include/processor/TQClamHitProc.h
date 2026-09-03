#ifndef TQCLAMHITPROC_H
#define TQCLAMHITPROC_H

#include "TGo4EventProcessor.h"
#include "TQClamConstants.h"

#include <vector>
#include <deque>

/** evts */

#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
class TQClamRawEvent;
#endif
#ifdef QCLAM_DAGOBERT
class TQClamDagobertRawEvent;
#endif

class TQClamHitEvent;
struct TQClamHit;
struct TQClamScaler;
struct TQClamTrigger;


/** parameter */
class TQClamDriftchamGeomParameter;
class TQClamTriggerCalibrationParameter;
class TQClamDriftchamCalibrationParameter;
class TQClamEdgeMappingParameter;
class TQClamHitConditionParameter;
class TQClamHitProcDebugOutputParameter;
class TQClamFaradayCupCurrentCalibrationParameter;
class TQClamDisplaySettingParameter;
class TQClamDrifttimeCalibParameter;
class TQClamHistogramParameter;
//class TQClamHitHistogramParameter;
//class TQClamHitTreeParameter;

/** Go4-Klassen */
class TGo4RollingGraph;
class TGo4Picture;
class TGo4CondArray;
class TGo4CounterCond;

/** ROOT Klassen */
class TH1;
class TH2;
class TGraph;
class THStack;



/** Struktur fuer Subevents
 * je TdcTrigger ein Tdc-ubergreifendes SubEvent
 *
 * tdc0:
 *  - tdcTrigger 0 -> SubEvent 0
 *  - tdcTrigger 1 -> SubEvent 1
 *  ...
 * tdc1:
 *  - tdcTrigger 0 -> SubEvent 0
 *  - tdcTrigger 1 -> SubEvent 1
 *  ...
 * ...*/
struct SubEvent
{
        double tdcTriggerTime[QCLAM_MAX_TDC];
        bool hasTdcTrigger[QCLAM_MAX_TDC];

        bool isSzintPmtUp = false;
        bool isSzintPmtDown = false;
        bool isCherenkovPmt = false;
        bool isCoincTrig = false;
        bool isAccTrig = false;
        bool isClock = false;
        bool isLivetime = false;
        bool isMbsLivetime = false;
        bool isFaradayCupChargeCounter = false;

        std::vector<TQClamHit> hits;
};







class TQClamHitProc : public TGo4EventProcessor
{
	public:
		/** konstruktor */
		TQClamHitProc();
		TQClamHitProc(const char* name);
		/** destruktor */
		virtual ~TQClamHitProc();

		/** setze Driftkammer */
//		void SetDriftcham(const TQClamDriftcham* driftcham);

		/** pruefe Klasse von output-Eventobjekt */
		Bool_t CheckEventClass(TClass* cl) override;
		
		/** baue output-Eventobjekt */
		Bool_t BuildEvent(TGo4EventElement* dest) override;



	private:
		bool ProofHitConditions(TQClamHit& hit);
		bool ProofScalerConditions(TQClamScaler& scaler);
#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
		void WriteRawDataToFile(const TQClamRawEvent* raw);
#endif
#ifdef QCLAM_DAGOBERT
		void WriteRawDataToFile(const TQClamDagobertRawEvent* raw);
#endif


#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
		void ProcessScaler(TQClamRawEvent* raw, TQClamHitEvent* evt);
		void ProcessSingleEvent(TQClamRawEvent* raw, TQClamHitEvent* evt);
#endif
#ifdef QCLAM_DAGOBERT
		void ProcessScaler(TQClamDagobertRawEvent* raw, TQClamHitEvent* evt);
		void ProcessSingleEvent(TQClamDagobertRawEvent* raw, TQClamHitEvent* evt);
#endif
//		void ProcessEvent(TQClamRawEvent* raw);

		/** erzeuge trigger/scaler (szintPmtUp/cherenkovPmt/.../clock...) hit */
		void AddTriggerHit(TQClamTrigger& trg, int tdc, int ch, double tLeadCorr, double tot);

		void FitDrifttimeSpectra();
		TH2* GetDrifttimeHistForLayer(int layer);

	private:
		/** -- Parameter, Windows, Histogramme & Variablen speichern -- **/

	        //-----------------------------------------------------------
		// 1. Parameter
		TQClamDriftchamGeomParameter* fDriftchamGeomPar{nullptr};
		TQClamTriggerCalibrationParameter* fTriggerCaliPar{nullptr};
		TQClamDriftchamCalibrationParameter* fDriftchamCaliPar{nullptr};
		TQClamEdgeMappingParameter* fEdgeMappingPar{nullptr};
		TQClamHitConditionParameter* fHitConditionPar{nullptr};
		TQClamHitProcDebugOutputParameter* fHitProcDebugOutputPar{nullptr};
		TQClamFaradayCupCurrentCalibrationParameter* fFaradayCupCurrentCaliPar{nullptr};
		TQClamDisplaySettingParameter* fDisplaySettingPar{nullptr};
		TQClamDrifttimeCalibParameter* fDrifttimeCalibPar{nullptr};
		TQClamHistogramParameter* fHistoPar{nullptr};




	        //-----------------------------------------------------------
		// 2. Conditions
		TGo4WinCond* fWinCondToT{nullptr};
		TGo4WinCond* fWinCondDrifttime{nullptr};
		TGo4PolyCond* fPolyCondDrifttimeVsToT{nullptr};

		TGo4CondArray* fCondArrToT{nullptr};
		TGo4CondArray* fCondArrDrifttime{nullptr};
		TGo4CondArray* fCondArrDrifttimeVsToT{nullptr};

		/** zaehle, wie viele Events HitProc passieren */
		TGo4CounterCond* fCondCountHitProcPassed{nullptr};
		TGo4CounterCond* fCondCountHitProcPassedX1{nullptr};
		TGo4CounterCond* fCondCountHitProcPassedU{nullptr};
		TGo4CounterCond* fCondCountHitProcPassedX2{nullptr};




	        //-----------------------------------------------------------
       		// 3. initialisiere Histogramme

		/** Histogramme */
		TH1* fHScaler{nullptr};

		/** Triggerdetektor-zeiten */
		TH1* fHSzintUpTime{nullptr};
		TH1* fHSzintDownTime{nullptr};
		TH1* fHCherenkovTime{nullptr};
		TH1* fHMeanSzintTime{nullptr};
		TH1* fHDiffSzintTime{nullptr};
		TH2* fHSzintUpVsSzintDown{nullptr};


		/** Statistik/Rate ganze Ebene */
//		TH1* fHHitsInLayer{nullptr};

		TH1* fHHitsInX1{nullptr};
		TH1* fHHitsInU{nullptr};
		TH1* fHHitsInX2{nullptr};
		TH1* fHHitsInX1Cut{nullptr};
		TH1* fHHitsInUCut{nullptr};
		TH1* fHHitsInX2Cut{nullptr};

		TH1* fHHitRateEvtInX1{nullptr};
		TH1* fHHitRateEvtInU{nullptr};
		TH1* fHHitRateEvtInX2{nullptr};
		TH1* fHHitRateEvtInX1Cut{nullptr};
		TH1* fHHitRateEvtInUCut{nullptr};
		TH1* fHHitRateEvtInX2Cut{nullptr};

		TH1* fHHitRateTimeInX1{nullptr};
		TH1* fHHitRateTimeInU{nullptr};
		TH1* fHHitRateTimeInX2{nullptr};
		TH1* fHHitRateTimeInX1Cut{nullptr};
		TH1* fHHitRateTimeInUCut{nullptr};
		TH1* fHHitRateTimeInX2Cut{nullptr};




		/** Hitpattern plots */
		TH1* fHHitPatternX1{nullptr};
		TH1* fHHitPatternU{nullptr};
		TH1* fHHitPatternX2{nullptr};
		TH1* fHHitPatternX1Cut{nullptr};
		TH1* fHHitPatternUCut{nullptr};
		TH1* fHHitPatternX2Cut{nullptr};

		/** HitRaten-Evt plots: hits/evt */
		TH1* fHHitRateEvtX1{nullptr};
		TH1* fHHitRateEvtU{nullptr};
		TH1* fHHitRateEvtX2{nullptr};
		TH1* fHHitRateEvtX1Cut{nullptr};
		TH1* fHHitRateEvtUCut{nullptr};
		TH1* fHHitRateEvtX2Cut{nullptr};

		/** HitRaten-Time plots: hits/time(µs) */
		TH1* fHHitRateTimeX1{nullptr};
		TH1* fHHitRateTimeU{nullptr};
		TH1* fHHitRateTimeX2{nullptr};
		TH1* fHHitRateTimeX1Cut{nullptr};
		TH1* fHHitRateTimeUCut{nullptr};
		TH1* fHHitRateTimeX2Cut{nullptr};



		/** Drifttime-Dist */
		TH1* fHDrifttimeDist{nullptr};
		TH1* fHDrifttimeDistX1{nullptr};
		TH1* fHDrifttimeDistU{nullptr};
		TH1* fHDrifttimeDistX2{nullptr};

		/** ToT-Dist */
		TH1* fHToTDist{nullptr};
		TH1* fHToTDistX1{nullptr};
		TH1* fHToTDistU{nullptr};
		TH1* fHToTDistX2{nullptr};

		/** Drifttime-Dist nach Cuts */
		TH1* fHDrifttimeDistCut{nullptr};
		TH1* fHDrifttimeDistX1Cut{nullptr};
		TH1* fHDrifttimeDistUCut{nullptr};
		TH1* fHDrifttimeDistX2Cut{nullptr};

		/** Drifttime-Dist nach Time-Walk Korrektur */
		TH1* fHDrifttimeDistTimeWalkCorr{nullptr};
		TH1* fHDrifttimeDistX1TimeWalkCorr{nullptr};
		TH1* fHDrifttimeDistUTimeWalkCorr{nullptr};
		TH1* fHDrifttimeDistX2TimeWalkCorr{nullptr};

		/** ToT-Dist nach Cuts */
		TH1* fHToTDistCut{nullptr};
		TH1* fHToTDistX1Cut{nullptr};
		TH1* fHToTDistUCut{nullptr};
		TH1* fHToTDistX2Cut{nullptr};

		/** Drifttime vs. Wire */
		TH2* fHDrifttimeVsWireX1{nullptr};
		TH2* fHDrifttimeVsWireU{nullptr};
		TH2* fHDrifttimeVsWireX2{nullptr};

		/** ToT vs. Wire */
		TH2* fHToTVsWireX1{nullptr};
		TH2* fHToTVsWireU{nullptr};
		TH2* fHToTVsWireX2{nullptr};

		/** Drifttime vs. Wire nach Cuts */
		TH2* fHDrifttimeVsWireX1Cut{nullptr};
		TH2* fHDrifttimeVsWireUCut{nullptr};
		TH2* fHDrifttimeVsWireX2Cut{nullptr};

		/** Drifttime vs. Wire nach Time-Walk Korrektur*/
		TH2* fHDrifttimeVsWireX1TimeWalkCorr{nullptr};
		TH2* fHDrifttimeVsWireUTimeWalkCorr{nullptr};
		TH2* fHDrifttimeVsWireX2TimeWalkCorr{nullptr};

		/** ToT vs. Wire nach Cuts */
		TH2* fHToTVsWireX1Cut{nullptr};
		TH2* fHToTVsWireUCut{nullptr};
		TH2* fHToTVsWireX2Cut{nullptr};

		/** Drifttime vs. ToT */
		TH2* fHDrifttimeVsToT{nullptr};
		TH2* fHDrifttimeVsToTX1{nullptr};
		TH2* fHDrifttimeVsToTU{nullptr};
		TH2* fHDrifttimeVsToTX2{nullptr};

		/** Drifttime vs. ToT nach Cuts */
		TH2* fHDrifttimeVsToTCut{nullptr};
		TH2* fHDrifttimeVsToTX1Cut{nullptr};
		TH2* fHDrifttimeVsToTUCut{nullptr};
		TH2* fHDrifttimeVsToTX2Cut{nullptr};

		/** Drifttime vs. ToT nach TimeWalkCorr */
		TH2* fHDrifttimeVsToTTimeWalkCorr{nullptr};
		TH2* fHDrifttimeVsToTX1TimeWalkCorr{nullptr};
		TH2* fHDrifttimeVsToTUTimeWalkCorr{nullptr};
		TH2* fHDrifttimeVsToTX2TimeWalkCorr{nullptr};



		THStack* fHStackHitsInLayer{nullptr};
		THStack* fHStackHitRateEvtInLayer{nullptr};
		THStack* fHStackHitRateTimeInLayer{nullptr};

		THStack* fHStackHitsInLayerCut{nullptr};
		THStack* fHStackHitRateEvtInLayerCut{nullptr};
		THStack* fHStackHitRateTimeInLayerCut{nullptr};




	        //-----------------------------------------------------------
		// RollingGraph
		TGo4RollingGraph* fGFaradayCupCurrent{nullptr};
		TGo4RollingGraph* fGScalerTriggerRate{nullptr};

		TGo4RollingGraph* fGSzintUpTrigRate{nullptr};
		TGo4RollingGraph* fGSzintDownTrigRate{nullptr};
		TGo4RollingGraph* fGCherenkovTrigRate{nullptr};
		TGo4RollingGraph* fGCoincTrigRate{nullptr};
		TGo4RollingGraph* fGAccTrigRate{nullptr};

		TGo4RollingGraph* fGRelDeadtime{nullptr};
		TGo4RollingGraph* fGRelMbsDeadtime{nullptr};

		TGraph* fGraphFaradayCupCurrent{nullptr};
		TGraph* fGraphScalerTriggerRate{nullptr};

		TGraph* fGraphSzintUpTrigRate{nullptr};
		TGraph* fGraphSzintDownTrigRate{nullptr};
		TGraph* fGraphCherenkovTrigRate{nullptr};
		TGraph* fGraphCoincTrigRate{nullptr};
		TGraph* fGraphAccTrigRate{nullptr};

		TGraph* fGraphRelDeadtime{nullptr};
		TGraph* fGraphRelMbsDeadtime{nullptr};





	        //-----------------------------------------------------------
		// Picture

		TGo4Picture* fPRateMon{nullptr};

		TGo4Picture* fPTriggerDetectors{nullptr};

		/** Hit-Statistik ganze Ebene */
		TGo4Picture* fPLayer_Rates_Statistics{nullptr};
		/** Hipattern/HitRaten-Evt/HitRaten-Zeit */
		TGo4Picture* fPHitPatternCond{nullptr};
		TGo4Picture* fPHitRateEvtCond{nullptr};
		TGo4Picture* fPHitRateTimeCond{nullptr};

		/** driftzeit/tot spektren */
		TGo4Picture* fPDrifttimeCond{nullptr};
		TGo4Picture* fPDrifttimeVsWireCond{nullptr};
		TGo4Picture* fPToTCond{nullptr};
		TGo4Picture* fPToTVsWireCond{nullptr};
		TGo4Picture* fPDrifttimeVsToTCond{nullptr};

		/** time-walk korrekturen */
		TGo4Picture* fPDrifttimeTimeWalkCorr{nullptr};
		TGo4Picture* fPDrifttimeVsWireTimeWalkCorr{nullptr};
		TGo4Picture* fPDrifttimeVsToTTimeWalkCorr{nullptr};




	private:
		/** -- Variablen -- */
		const double fClockPeriod = CLOCK_PERIOD_QCLAM; // definiert in TQClamConstants

		/** evt counter */
		int fEvtCounter = 0;
		double fTimeInMicroSeconds = 0.0;

		ULong64_t fStartTime{0};
		Double_t fGraphTime{0.};
		Bool_t fFirstTime{kTRUE};


		/** speicher integrations-interval fuer Rolling-Graphen, um aenderung zu erkennen */
		int fLastRollGraphWindow = -1;

		int fHitCounterX1 = 0;
		int fHitCounterU = 0;
		int fHitCounterX2 = 0;
		int fHitCounterX1Cut = 0;
		int fHitCounterUCut = 0;
		int fHitCounterX2Cut = 0;

		int fHitCounterX1Wire[QCLAM_MAX_WIRE];
		int fHitCounterUWire[QCLAM_MAX_WIRE];
		int fHitCounterX2Wire[QCLAM_MAX_WIRE];
		int fHitCounterX1WireCut[QCLAM_MAX_WIRE];
		int fHitCounterUWireCut[QCLAM_MAX_WIRE];
		int fHitCounterX2WireCut[QCLAM_MAX_WIRE];


		/** Berechnung von Faraday-Cup Strom */
		double fFaradayCupChargeCounter_LastEvent; /** speicher ladung letztes event --> berechne Strom */
		double fSzintPmtUpScaler_LastEvent; /** speichert scaler szintPmtUp von letzten Event */
		double fSzintPmtDownScaler_LastEvent; /** speichert scaler szintPmtDown von letzten Event */
		double fCherenkovPmtScaler_LastEvent; /** speichert scaler cherenkovPmt von letzten Event */
		double fCoincidenceScaler_LastEvent; /** speichert scaler coinc-trig von letzten Event */
		double fAcceptedScaler_LastEvent; /** speichert scaler acc-trig von letzten Event */
		double fLivetimeScaler_LastEvent; /** speichert sclaer livetime von letztem Event */
		double fMbsLivetimeScaler_LastEvent; /** speichert sclaer mbslivetime von letztem Event */

		double fTimeInMicroSeconds_LastEvent; /** speicher zeit letztes event --> berechne strom */
		/** tdcTrigger */
		std::vector<double> tdcTriggerTimes[QCLAM_MAX_TDC]; /** TdcTrigger-Zeiten in Tdc in Subevent */
		int currentTriggerIndex[QCLAM_MAX_SUBEVENTS]; /** aktueller TdcTrigger-Index (=Subevent) */
		std::vector<SubEvent> fSubEvents;
		int fTriggerIndex[QCLAM_MAX_TDC]; /** zaehlerstand: trigger-index je tdc */
		SubEvent fCurrentSubEvent;

		/** weitere tdc-Kanaele */
	//	double leadingTime[QCLAM_MAX_TDC][QCLAM_MAX_CH]; OLD VERSION
        //	bool hasLeading[QCLAM_MAX_TDC][QCLAM_MAX_CH]; OLD VERSION
	
		std::deque<double> leadingTimes[QCLAM_MAX_TDC][QCLAM_MAX_CH];
		std::deque<double> trailingTimes[QCLAM_MAX_TDC][QCLAM_MAX_CH];




		/** tdc-Trigger */

		//static double tdcTriggerTime[QCLAM_MAX_TDC][QCLAM_MAX_CH];
		double tdcTriggerTime[QCLAM_MAX_TDC];
		bool hasTdcTrigger[QCLAM_MAX_TDC];

	protected:

		double fTimeLastEvent = 0.;
		double fIntegratedTime = 0.;

		bool fFirstEvent = true;

	    
		struct RateChannel
		{
			double lastScaler = 0;
			double dScalerSum = 0;
			bool   first = true;
		};

		RateChannel fChFaraday;
		RateChannel fChSzintUp;
		RateChannel fChSzintDown;
		RateChannel fChCherenkov;
		RateChannel fChCoincidence;
		RateChannel fChAccepted;
		RateChannel fChClock;
		RateChannel fChLivetime;
		RateChannel fChMbsLivetime;

		/** hilfsmethoden */
		bool IntegrateTime(double timeMicroSeconds);
		void UpdateRateChannel(RateChannel& ch, double scaler);
		void FlushRateChannel(RateChannel& ch, double integrated,//TimeMicroSeconds,
				TGo4RollingGraph* graph, TGraph* gr, TH1* hist);
		void FlushDeadtime(RateChannel& chClock, RateChannel& chLivetime,
				TGo4RollingGraph* graph, TGraph* gr, TH1* hist);
		void FlushFaradayChannel(RateChannel& ch, double integrated);//TimeMicroSeconds);



	ClassDefOverride(TQClamHitProc, 1);
};

#endif
