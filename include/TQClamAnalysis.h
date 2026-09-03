#ifndef TQCLAMANALYSIS_h
#define TQCLAMANALYSIS_h

#include "TGo4Analysis.h"
#include "Rtypes.h"

class TGo4EventSource;

/** ROOT-Klassen */
class TH1;

/** evts */
class TGo4MbsEvent;
//class TQClamUnpackEvent;

#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
class TQClamRawEvent;
#endif
#ifdef QCLAM_DAGOBERT
class TQClamDagobertRawEvent;
#endif

class TQClamHitEvent;
class TQClamClusterEvent;
class TQClamTrackEvent;
//class TQClamDriftcham;

class TQClamHitProc;

/** Go4-Klassen */
class TGo4Picture;

/** parameter */
class TQClamSourceParameter;




class TQClamAnalysis : public TGo4Analysis
{
	public:
		TQClamAnalysis();
		TQClamAnalysis(int argc, char **argv);
		virtual ~TQClamAnalysis();

//		virtual TGo4EventSource* CreateEventSource();
		Int_t UserPreLoop() override;
		Int_t UserEventFunc() override;
		Int_t UserPostLoop() override;



	private:
		Bool_t fTrackStoreDefaultEnabled = kFALSE;
		std::string fTrackStoreDefaultName = "";

		Bool_t fTrackStoreEnabled = kFALSE;
		Bool_t fTrackTestingStoreEnabled = kFALSE;
		Bool_t fDriftlengthRelationCalibStoreEnabled = kFALSE;
		Bool_t fCoincStoreEnabled = kFALSE;
		Bool_t fClusterStoreEnabled = kFALSE;
		Bool_t fDrifttimeStoreEnabled = kFALSE;


		Bool_t fInputListGiven = kFALSE;
		std::string fInputList = "";

		Bool_t fInputFileListGiven = kFALSE;
		std::string fInputFileList = "";

		Bool_t fInputFileGiven = kFALSE;
		std::string fInputFile = "";



		Bool_t fInputPrefixGiven = kFALSE;
		std::string fInputPrefix = "";

		std::string fTrackStoreName = "";
		std::string fTrackTestingStoreName = "";
		std::string fDriftlengthRelationCalibStoreName = "";
		std::string fCoincStoreName = "";
		std::string fClusterStoreName = "";
		std::string fDrifttimeStoreName = "";


		Bool_t batch_no_histos = kFALSE;

		TString fCmdLineFile;

		void AppendRunFiles(const TString& prefix, size_t& nextIndex);
		void AppendFileList(const std::string& filename,size_t& nextIndex);

		/** Parameter konfigurieren */
		void InitMasterParameter();
		void InitSourceParameter();
		void InitEdgeMappingParameter();
		void InitDriftchamGeomParameter();
		void InitDriftchamCalibrationParameter();
		void InitTriggerCalibrationParameter();
		void InitFaradayCupCurrentCalibrationParameter();
		void InitHitConditionParameter();
		void InitHitProcDebugOutputParameter();
		void InitDisplaySettingParameter();
		void InitClusterConditionParameter();
		void InitTrackConditionParameter();
		void InitDriftToZOffsetParameter();
		void InitFocalPlaneParameter();
		void InitSpectrumCalibrationParameter();
		void InitFitParameter();
		void InitDrifttimeCalibParameter();
		void InitUnpackParameter();
		void InitStoreParameter();
		void InitHistogramParameter();
		void InitSieveSlitCalibParameter();

#ifdef QCLAM_DAGOBERT
		void InitTcpParameter();
		void InitDagobertCalibrationParameter();
#endif

		void RebuildTrailingMapping();



		/** Geometrie-Objekte bauen */
		void BuildDriftcham();



                //-----------------------------------------------------------
                // Picture
		TGo4Picture* fPEventMonitor{nullptr};
		TGo4Picture* fPEventMonitor_Shifts{nullptr};


	private:
		/** pointer auf geometrieobjekte */
		//TQClamDriftcham* fDriftcham{nullptr};

		/** TQClamDriftchamGeomParameter veraendert?
		  * --> falls kTRUE wird die Driftkammer neu gebaut. */

		/** pointer auf eventklassen */
		TQClamSourceParameter* fSourcePar{nullptr};

		TGo4MbsEvent* fMbsEvent{nullptr};

#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)
		TQClamRawEvent* fRawEvent{nullptr};
#endif
#ifdef QCLAM_DAGOBERT
		TQClamDagobertRawEvent* fRawEvent{nullptr};
#endif
		TQClamHitEvent* fHitEvent{nullptr};
		TQClamClusterEvent* fClusterEvent{nullptr};
		TQClamTrackEvent* fTrackEvent{nullptr};
//		TQClamUnpackEvent* fUnpackEvent{nullptr};
//		TQClamHitEvent* fHitEvent{nullptr};

		TQClamHitProc* fHitProc{nullptr};

		//		TODO
//		TQClamClusterEvent* fClusterEvent{nullptr};
//		TQClamTrackEvent* fTrackEvent{nullptr};

		/** TODO: Projector etc. */
		/** TODO: DAGOBERT */

		// pointer auf histogramme
		TH1* fHistEventSize{nullptr};
		TH1* fHistAccumEventSize{nullptr};

		/** TODO Windows */

		// zaehler
		Int_t fEvents{0};
		Int_t fLastEvent{0}; // irrefuehrend... -> aendern TODO
		Double_t fAccumEventSizeGB{0};

		/** parameter */


	ClassDefOverride(TQClamAnalysis, 1);
};

#endif
