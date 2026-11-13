//-----------------------------------------------------------------------
// author: O. Moeller
// created: 03.2026
// last modified: 
//-----------------------------------------------------------------------


#include "TQClamAnalysis.h"

/** Go4-Klassen */
#include "TGo4Version.h"
#include "TGo4StepFactory.h"
#include "TGo4FileStoreParameter.h"
#include "TGo4RuntimeException.h"
#include "TGo4UserException.h"
#include "TGo4MbsFileParameter.h"
#include "TGo4Picture.h"
#include "TGo4UserSourceParameter.h"
#include "TGo4MbsStreamParameter.h"
//#include "TGo4RevServParameter.h"

/** ROOT-includes */
#include "TSystem.h"
#include "TH1.h"
#include "TCanvas.h"

/** yaml-cpp fuer configs */
#include <yaml-cpp/yaml.h>

/** sources */
#ifdef QCLAM_SINGLE_ROOT
#include "source/TQClamOfflineRootSource.h"
#endif

/** processors */
//#include "processor/TQClamHitProc.h"

/** events */
//#include "evt/TQClamUnpackEvent.h"
#include "evt/TQClamHitEvent.h"
#include "evt/TQClamClusterEvent.h"
#include "evt/TQClamTrackEvent.h"



#if defined(QCLAM_SINGLE) || defined(QCLAM_SINGLE_ROOT)

#include "evt/TQClamRawEvent.h"

#endif




#ifdef QCLAM_DAGOBERT

#include "evt/TQClamDagobertRawEvent.h"
#include "evt/TDagobertHitEvent.h"
#include "evt/TQClamDagobertCoincidenceEvent.h"
#include "processor/TDagobertHitProc.h"
#include "parameter/TQClamTcpParameter.h"
#include "parameter/TDagobertCalibrationParameter.h"

#endif


/** parameter */
#include "parameter/TQClamSourceParameter.h"
#include "parameter/TQClamMasterParameter.h"
#include "parameter/TQClamDriftchamGeomParameter.h"
#include "parameter/TQClamDriftchamCalibrationParameter.h"
#include "parameter/TQClamTriggerCalibrationParameter.h"
#include "parameter/TQClamFaradayCupCurrentCalibrationParameter.h"
#include "parameter/TQClamEdgeMappingParameter.h"
#include "parameter/TQClamHitConditionParameter.h"
#include "parameter/TQClamHitProcDebugOutputParameter.h"
#include "parameter/TQClamFaradayCupCurrentCalibrationParameter.h"
#include "parameter/TQClamDisplaySettingParameter.h"
#include "parameter/TQClamClusterConditionParameter.h"
#include "parameter/TQClamTrackConditionParameter.h"
#include "parameter/TQClamDriftToZOffsetParameter.h"
#include "parameter/TQClamFocalPlaneParameter.h"
#include "parameter/TQClamSpectrumCalibrationParameter.h"
#include "parameter/TQClamFitParameter.h"
#include "parameter/TQClamDrifttimeCalibParameter.h"
#include "parameter/TQClamUnpackParameter.h"
#include "parameter/TQClamStoreParameter.h"
#include "parameter/TQClamHistogramParameter.h"
#include "parameter/TQClamSieveSlitCalibParameter.h"
#include "parameter/TQClamImageMatrixParameter.h"

/** geometry */
#include "geometry/TQClamGeometry.h"
//#include "geometry/TQClamDriftcham.h"
//#include "geometry/TQClamWirePlane.h"
//#include "geometry/TQClamWire.h"

/** utils */
#include "utils/TQClamDriftCorrLoader.h"
#include "utils/TQClamTimeOffsetLoader.h"
#include "utils/TQClamTimeWalkLoader.h"


#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>


//ClassImp(TQClamAnalysis)

//***********************************************************
TQClamAnalysis::TQClamAnalysis()
	: TGo4Analysis()
{
	TGo4Log::Error("[TQClamAnalysis] Falscher Konstruktor TQClamAnalysis()!");
}


//***********************************************************
// Konstruktor, aufgerufen durch go4analysis
TQClamAnalysis::TQClamAnalysis(int argc, char **argv) 
	: TGo4Analysis(argc, argv)
{
	/// ====================================================
	// user-args
	/// ====================================================
	for(int i=0; i<argc; ++i)
	{
		std::string arg = argv[i];


		if(arg == "--no-histos")
		{
			batch_no_histos = kTRUE;
		}

		if(arg == "--input-list")
		{
			if(i + 1 >= argc)
			{
				TGo4Log::Error("Option --input-list benoetigt einen Dateinamen.");
				throw std::runtime_error("Missing argument for --input-list");
			}

			fInputListGiven = true;
			fInputList = argv[++i];
		}
		if(arg == "--input-file-list")
		{
			if(i + 1 >= argc)
			{
				TGo4Log::Error("Missing argument for --input-file-list");
				throw std::runtime_error("Missing argument");
			}

			fInputFileListGiven = true;
			fInputFileList = argv[++i];
		}
		if(arg == "--input-prefix")
		{
			if(i + 1 >= argc)
			{
				TGo4Log::Error("Missing argument for --input-prefix");
				throw std::runtime_error("Missing argument");
			}

			fInputPrefixGiven = true;
			fInputPrefix = argv[++i];
		}
		if(arg == "--input-file")
		{
			if(i + 1 >= argc)
			{
				TGo4Log::Error("Missing argument for --input-file");
				throw std::runtime_error("Missing argument");
			}

			fInputFileGiven = true;
			fInputFile = argv[++i];
		}
		
		// ================
		// output-dir wird aus config store-dir + filenamen gebildet
		// ================
		if(arg == "--track-store-default")
		{
			if(i + 1 >= argc)
			{
				TGo4Log::Error("Missing argument for --track-store-default");
				throw std::runtime_error("Missing argument");
			}

			fTrackStoreDefaultEnabled = true;
			fTrackStoreDefaultName = argv[++i];
		}

		// ================
		// uebergebe output-dir
		// ================
		if(arg == "--coinc-store")
		{
			fCoincStoreEnabled = true;
			fCoincStoreName = argv[++i];
		}
		if(arg == "--track-store")
		{
			fTrackStoreEnabled = true;
			fTrackStoreName = argv[++i];
		}
		
		if(arg == "--track-testing-store")
		{
			fTrackTestingStoreEnabled = true;
			fTrackTestingStoreName = argv[++i];
		}
		
		
		if(arg == "--driftlength-calib-store")
		{
			fDriftlengthRelationCalibStoreEnabled = true;
			fDriftlengthRelationCalibStoreName = argv[++i];
		}
	
		if(arg == "--cluster-store")
		{
			fClusterStoreEnabled = true;
			fClusterStoreName = argv[++i];
		}

		if(arg == "--drifttime-store")
		{
			fDrifttimeStoreEnabled = true;
			fDrifttimeStoreName = argv[++i];
		}
	}

	TGo4Log::Info("[TQClamAnalysis] Constructor: Start");
	TGo4Log::Info("[TQClamAnalysis] Create %s", GetName());
	SetAutoSave(kFALSE); /** konfigurierbar machen */


	/// pruefe Go4-Version Missmatch, bspw.:
	// Compile-Version 60499
	// Runtime-Version 60302
	if(!TGo4Version::CheckVersion(__GO4BUILDVERSION__))
	{
		TGo4Log::Error("Go4-Versions Missmatch!");
		std::cout << "Korrekte Version " << __GO4BUILDVERSION__ << std::endl;
		std::cout << "Beende nun." << std::endl;
		exit(-1);
	}


	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);

	std::tm tm{};
	localtime_r(&t, &tm);


	/** Go4Parameter erzeugen und registrieren --> in UserPreLoop konfigurieren */
	AddParameter(new TQClamMasterParameter("MasterParam"));
	AddParameter(new TQClamDriftchamGeomParameter("DriftchamGeomParam"));
	AddParameter(new TQClamDriftchamCalibrationParameter("DriftchamCalibParam"));
	AddParameter(new TQClamTriggerCalibrationParameter("TriggerCalibParam"));
	AddParameter(new TQClamFaradayCupCurrentCalibrationParameter("FaradayCupCurrentCalibParam"));
	AddParameter(new TQClamEdgeMappingParameter("EdgeMappingParam"));
	AddParameter(new TQClamHitConditionParameter("HitConditionParam"));
	AddParameter(new TQClamHitProcDebugOutputParameter("HitProcDebugOutputParam"));
	AddParameter(new TQClamDisplaySettingParameter("DisplaySettingParam"));
	AddParameter(new TQClamClusterConditionParameter("ClusterConditionParam"));
	AddParameter(new TQClamTrackConditionParameter("TrackConditionParam"));
	AddParameter(new TQClamDriftToZOffsetParameter("DriftToZOffsetParam"));
	AddParameter(new TQClamFocalPlaneParameter("FocalPlaneParam"));
	AddParameter(new TQClamSpectrumCalibrationParameter("SpecCalibParam"));
	AddParameter(new TQClamFitParameter("FitParam",nullptr));
	AddParameter(new TQClamDrifttimeCalibParameter("DrifttimeCalibParam"));
	AddParameter(new TQClamUnpackParameter("UnpackParam"));
	AddParameter(new TQClamStoreParameter("StoreParam"));
	AddParameter(new TQClamHistogramParameter("HistoParam"));
	AddParameter(new TQClamSieveSlitCalibParameter("SieveSlitCalibParam"));
	AddParameter(new TQClamImageMatrixParameter("ImageMatrixParam"));

#ifdef QCLAM_DAGOBERT
	AddParameter(new TQClamTcpParameter("TcpParam"));
	AddParameter(new TDagobertCalibrationParameter("DagoCalibParam"));
#endif

	// switch-case ueber source-klassen
	// 0 -> OfflineRootSource
	// 1 -> MbsStream
	// ... weitere?

	/** registriere parameter als UserSourceParameter */
	//TQClamSourceParameter* sourcepar = new TQClamSourceParameter("/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/unpacked_qclam_data_for_testing/4x1/pulser_031_0000.root");
	TQClamSourceParameter* sourcepar = new TQClamSourceParameter("SourceParam");
	AddParameter(sourcepar);
//	AddParameter(new TQClamSourceParameter("SourcePar"));

	/** lese configs ein und konfiguriere Go4Parameter */
	InitMasterParameter();
	InitSourceParameter();
	InitDriftchamGeomParameter();
	InitEdgeMappingParameter(); /** muss vor InitDriftchamCalibrationParameter ausgefuehrt werden */
	InitDriftchamCalibrationParameter();
	InitTriggerCalibrationParameter();
	InitHitConditionParameter();
	InitHitProcDebugOutputParameter();
	InitFaradayCupCurrentCalibrationParameter();
	InitDisplaySettingParameter();
	InitClusterConditionParameter();
	InitTrackConditionParameter();
	InitDriftToZOffsetParameter();
	InitFocalPlaneParameter();
	InitSpectrumCalibrationParameter();
	InitFitParameter();
	InitDrifttimeCalibParameter();
	InitUnpackParameter();
	InitStoreParameter();
	InitHistogramParameter();
	InitSieveSlitCalibParameter();
	InitImageMatrixParameter();

#ifdef QCLAM_DAGOBERT
	InitTcpParameter();
	InitDagobertCalibrationParameter();
#endif



	// -- erzeuge Factories & registriere Step-Eventklassen und Step-Analysisklassen -- 

	/** 1 OfflineRootSource:
	 * 1.1 OfflinRootSource -> RawEvent -> OfflineRootHitProc -> HitEvent
	 * 1.2 HitEvent -> ClusterProc -> ClusterEvent 
	 * 1.3 ClusterEvent -> TrackProc -> TrackEvent
	 * 1.4 TrackEvent -> Projector ... TODO */



#ifdef QCLAM_SINGLE_ROOT

        TGo4StepFactory* hitFactory = new TGo4StepFactory("HitFactory");

        hitFactory->DefUserEventSource("TQClamOfflineRootSource"); // user event source
        hitFactory->DefInputEvent("RawEvent", "TQClamRawEvent");
        hitFactory->DefEventProcessor("HitProc", "TQClamHitProc");
        hitFactory->DefOutputEvent("HitEvent", "TQClamHitEvent");
        auto* hitStore = new TGo4FileStoreParameter("QClamOutput.root");
        hitStore->SetOverwriteMode(kTRUE);

        auto* source = new TGo4UserSourceParameter(fSourcePar->fName);
        TGo4AnalysisStep* hitStep = new TGo4AnalysisStep("HitStep", hitFactory,
                        /*source*/ source,
                        /*store*/ nullptr,
                        /*filter*/ nullptr);
        hitStep->SetErrorStopEnabled(kFALSE);
        // ueberschrieben durch setup.C
        hitStep->SetSourceEnabled(kTRUE);
        hitStep->SetStoreEnabled(kFALSE);
        hitStep->SetEventStore(hitStore);
        hitStep->SetProcessEnabled(kTRUE);
        AddAnalysisStep(hitStep);


        // Step 3: Cluster
        TGo4StepFactory* clusterFactory = new TGo4StepFactory("ClusterFactory");
        clusterFactory->DefInputEvent("HitEvent", "TQClamHitEvent");
        clusterFactory->DefEventProcessor("ClusterProc", "TQClamClusterProc");
        clusterFactory->DefOutputEvent("ClusterEvent", "TQClamClusterEvent");
        TGo4AnalysisStep* clusterStep = new TGo4AnalysisStep("ClusterStep", clusterFactory,
                        /*source*/ nullptr,
                        /*store*/ nullptr,
                        /*filter*/ nullptr);
        clusterStep->SetErrorStopEnabled(kTRUE);
        AddAnalysisStep(clusterStep);
        // ueberschrieben durch setup.C
        clusterStep->SetSourceEnabled(kFALSE);
        clusterStep->SetStoreEnabled(kFALSE);
        clusterStep->SetProcessEnabled(kTRUE);

        // Step 4: Track
	TGo4StepFactory* trackFactory = new TGo4StepFactory("TrackFactory");
	trackFactory->DefInputEvent("ClusterEvent", "TQClamClusterEvent");
	trackFactory->DefEventProcessor("TrackProc", "TQClamTrackProc");
	trackFactory->DefOutputEvent("TrackEvent", "TQClamTrackEvent");




	if(fTrackStoreEnabled)
	{
		trackFactory->DefUserEventStore("TQClamTrackStore");
		auto* trackStore = new TGo4UserStoreParameter(fTrackStoreName.c_str());
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kTRUE);
		trackStep->SetProcessEnabled(kTRUE);
	}
	else if(fTrackTestingStoreEnabled)
	{
		trackFactory->DefUserEventStore("TQClamTrackTestingStore");
		auto* trackStore = new TGo4UserStoreParameter(fTrackTestingStoreName.c_str());
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kTRUE);
		trackStep->SetProcessEnabled(kTRUE);
	}

	else
	{
		trackFactory->DefUserEventStore("TQClamTrackStore");
		auto* trackStore = new TGo4UserStoreParameter("TrackStore");
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kFALSE);
		trackStep->SetProcessEnabled(kTRUE);
	}





#endif








#ifdef QCLAM_SINGLE
	switch(sourcepar->fSourceType)
	{
		// CASE OfflineLmdSource
		case TQClamSourceParameter::kQClamOfflineLmd:
		{
			// Step 1: Unpack
			TGo4StepFactory* unpackFactory = new TGo4StepFactory("UnpackFactory");

			unpackFactory->DefOutputEvent("RawEvent","TQClamRawEvent");
			unpackFactory->DefEventProcessor("QClamUnpackProc","TQClamUnpackProc");

			TGo4MbsFileParameter* source = new TGo4MbsFileParameter();
			for(size_t i=0; i<fSourcePar->MAX_LMD_FILES; i++)
			{
				TString name = fSourcePar->fLmdFiles[i];
				if(name == "" || !name.EndsWith(".lmd")) { continue; }
				source->AddMoreFile(name.Data());
			}
			//TGo4MbsFileParameter* source = new TGo4MbsFileParameter("/d/d02-1/ag_vnc/QCLAM/DAQ_tests_4x1_system/noise2026_030_0000.lmd");

			TGo4AnalysisStep* unpackStep = new TGo4AnalysisStep("UnpackStep",unpackFactory,
					source,
					nullptr,
					nullptr);
			AddAnalysisStep(unpackStep);
			unpackStep->SetSourceEnabled(kTRUE);
			unpackStep->SetErrorStopEnabled(kTRUE);
			unpackStep->SetProcessEnabled(kTRUE);
			unpackStep->SetStoreEnabled(kFALSE);




			break;
		}




		// CASE: MbsStream
		case TQClamSourceParameter::kQClamMbsStream:
		{
			TGo4Log::Info("[TQClamAnalsis] Aktiviere Mbs-Stream als Source.");


			// TCP STEP
			/*
			TGo4StepFactory* tcpFactory = new TGo4StepFactory("TcpFactory");

			tcpFactory->DefUserEventSource("TQClamTcpSource");
			tcpFactory->DefInputEvent("TcpEvent","TQClamTcpEvent");
			tcpFactory->DefEventProcessor("BackgroundRateProc","TQClamBackgroundRateProc");
			tcpFactory->DefOutputEvent("BackgroundRateEvent","TQClamBackgroundRateEvent");
			//auto* tcpStore = TODO;
			TGo4UserSourceParameter* tcpSource = new TGo4UserSourceParameter("TcpSource");
			tcpSource->SetPort(5050); // Tcp-Port 
			tcpSource->SetExpression("192.168.2.8");//spectrometercontrol");//"192.168.15.78"); // hostname 
			TGo4AnalysisStep* tcpStep = new TGo4AnalysisStep("TcpStep",tcpFactory,
					tcpSource,
					nullptr,
					nullptr);

			tcpStep->SetErrorStopEnabled(kFALSE);
			tcpStep->SetSourceEnabled(kTRUE);
			tcpStep->SetProcessEnabled(kTRUE);
			tcpStep->SetStoreEnabled(kFALSE);
			AddAnalysisStep(tcpStep);
			*/


			// Step 1: Unpack
			TGo4StepFactory* unpackFactory = new TGo4StepFactory("UnpackFactory");

			unpackFactory->DefOutputEvent("RawEvent","TQClamRawEvent");
			unpackFactory->DefEventProcessor("QClamUnpackProc","TQClamUnpackProc");

			auto* streampar = new TGo4MbsStreamParameter(fSourcePar->fStreamServer.Data());
			//auto* revpar = new TGo4RevServParameter(fSourcePar->fStreamServer.Data());

			TGo4AnalysisStep* unpackStep = new TGo4AnalysisStep("UnpackStep",unpackFactory,
					streampar,
					 nullptr,
					 nullptr);
			AddAnalysisStep(unpackStep);
			unpackStep->SetSourceEnabled(kTRUE);
			unpackStep->SetErrorStopEnabled(kTRUE);
			unpackStep->SetProcessEnabled(kTRUE);
			unpackStep->SetStoreEnabled(kFALSE);

			break;
		}


		default:
		{
			TGo4Log::Error("[TQClamAnalysis] Unknown SourceType: %d", sourcepar->fSourceType);
			TGo4RuntimeException();
		}

	}





	// Step 2: Hit 
	TGo4StepFactory* hitFactory = new TGo4StepFactory("HitFactory");

	hitFactory->DefInputEvent("RawEvent", "TQClamRawEvent");
	hitFactory->DefEventProcessor("HitProc", "TQClamHitProc");
	hitFactory->DefOutputEvent("HitEvent", "TQClamHitEvent");
	auto* hitStore = new TGo4FileStoreParameter("QClamOutput.root");
	hitStore->SetOverwriteMode(kTRUE);

	TGo4AnalysisStep* hitStep = new TGo4AnalysisStep("HitStep", hitFactory,
			nullptr,
			hitStore,
			nullptr);
	hitStep->SetErrorStopEnabled(kFALSE);
	// ueberschrieben durch setup.C
	hitStep->SetSourceEnabled(kFALSE);
	hitStep->SetStoreEnabled(kTRUE);
	hitStep->SetEventStore(hitStore);
	hitStep->SetProcessEnabled(kTRUE);
	AddAnalysisStep(hitStep);


	// Step 3: Cluster
	TGo4StepFactory* clusterFactory = new TGo4StepFactory("ClusterFactory");
	clusterFactory->DefInputEvent("HitEvent", "TQClamHitEvent");
	clusterFactory->DefEventProcessor("ClusterProc", "TQClamClusterProc");
	clusterFactory->DefOutputEvent("ClusterEvent", "TQClamClusterEvent");
	TGo4AnalysisStep* clusterStep = new TGo4AnalysisStep("ClusterStep", clusterFactory,
			/*source*/ nullptr,
			/*store*/ nullptr,
			/*filter*/ nullptr);
	clusterStep->SetErrorStopEnabled(kTRUE);
	AddAnalysisStep(clusterStep);
	// ueberschrieben durch setup.C
	clusterStep->SetSourceEnabled(kFALSE);
	clusterStep->SetStoreEnabled(kFALSE);
	clusterStep->SetProcessEnabled(kTRUE);

	// Step 4: Track
	TGo4StepFactory* trackFactory = new TGo4StepFactory("TrackFactory");
	trackFactory->DefInputEvent("ClusterEvent", "TQClamClusterEvent");
	trackFactory->DefEventProcessor("TrackProc", "TQClamTrackProc");
	trackFactory->DefOutputEvent("TrackEvent", "TQClamTrackEvent");
	TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
			/*source*/ nullptr,
			/*store*/ nullptr,
			/*filter*/ nullptr);
	trackStep->SetErrorStopEnabled(kTRUE);
	AddAnalysisStep(trackStep);
	// uebeschrieben durch setup.C
	trackStep->SetSourceEnabled(kFALSE);
	trackStep->SetStoreEnabled(kFALSE);
	trackStep->SetProcessEnabled(kTRUE);



#endif




#ifdef QCLAM_DAGOBERT

	/**  --- Ablauf: ---
	 *
	 *
	 *				   ---> QClamHitEvent ---> QClamClusterEvent ---> QClamTrackEvent --->
	 *				  |								       |
	 * LmdFile/MbsStream -> RawEvent -|								       |-> CoincidenceEvent
	 *				  |								       |
	 * 				   ---> 		   DagobertHitEvent			  --->
	 */


	// OfflineLmdSource oder MbsStream

	SetStepChecking(kFALSE);

	switch(sourcepar->fSourceType)
	{
		// Case QClamDagobertLmd-Source
		case TQClamSourceParameter::kQClamDagobertOfflineLmd:
		{
			TGo4StepFactory* unpackFactory = new TGo4StepFactory("UnpackFactory");

			unpackFactory->DefOutputEvent("RawEvent","TQClamDagobertRawEvent");
			unpackFactory->DefEventProcessor("UnpackProc","TQClamDagobertUnpackProc");

			TGo4MbsFileParameter* source = new TGo4MbsFileParameter();
			for(size_t i=0; i<fSourcePar->MAX_LMD_FILES; i++)
			{
				TString name = fSourcePar->fLmdFiles[i];
				if(name == "" || !name.EndsWith(".lmd")) { continue; }
				source->AddMoreFile(name.Data());
			}
			//TGo4MbsFileParameter* source = new TGo4MbsFileParameter("/d/d02-1/ag_vnc/QCLAM/DAQ_tests_4x1_system/noise2026_030_0000.lmd");

			TGo4AnalysisStep* unpackStep = new TGo4AnalysisStep("UnpackStep",unpackFactory,
					source,
					nullptr,
					nullptr);
			AddAnalysisStep(unpackStep);
			unpackStep->SetSourceEnabled(kTRUE);
			unpackStep->SetErrorStopEnabled(kTRUE);
			unpackStep->SetProcessEnabled(kTRUE);
			unpackStep->SetStoreEnabled(kFALSE);


			break;
		}



		// CASE: MbsStream
		case TQClamSourceParameter::kQClamDagobertMbsStream:
		{
			TGo4Log::Info("[TQClamAnalsis] Aktiviere Mbs-Stream als Source.");


			// Step 1: Unpack
			TGo4StepFactory* unpackFactory = new TGo4StepFactory("UnpackFactory");

			unpackFactory->DefOutputEvent("RawEvent","TQClamDagobertRawEvent");
			unpackFactory->DefEventProcessor("QClamDagobertUnpackProc","TQClamDagobertUnpackProc");

			auto* streampar = new TGo4MbsStreamParameter(fSourcePar->fStreamServer.Data());
			//auto* revpar = new TGo4RevServParameter(fSourcePar->fStreamServer.Data());

			TGo4AnalysisStep* unpackStep = new TGo4AnalysisStep("UnpackStep",unpackFactory,
					streampar,
					 nullptr,
					 nullptr);
			AddAnalysisStep(unpackStep);
			unpackStep->SetSourceEnabled(kTRUE);
			unpackStep->SetErrorStopEnabled(kTRUE);
			unpackStep->SetProcessEnabled(kTRUE);
			unpackStep->SetStoreEnabled(kFALSE);

			break;
		}




		default:
		{
			TGo4Log::Error("[TQClamAnalysis] Unknown SourceType: %d", sourcepar->fSourceType);
			TGo4RuntimeException();
		}

	}

	// Provider fuer QClamHit
	TGo4StepFactory* provFactoryQClam = new TGo4StepFactory("QClamRawProviderFactory");
	provFactoryQClam->DefInputEvent("RawEvent","TQClamDagobertRawEvent");
	provFactoryQClam->DefEventProcessor("RawEvent_Q","TQClamDagobertProviderProc");
	provFactoryQClam->DefOutputEvent("Dummy","TQClamDagobertDummyEvent");
	auto* provStepQClam = new TGo4AnalysisStep("QClamRawProvider",provFactoryQClam,nullptr,nullptr,nullptr);
	AddAnalysisStep(provStepQClam);
	provStepQClam->SetSourceEnabled(kFALSE);
	provStepQClam->SetProcessEnabled(kTRUE);
	provStepQClam->SetStoreEnabled(kFALSE);

	// Provider fuer DagobertHit
	TGo4StepFactory* provFactoryDag = new TGo4StepFactory("DagRawProviderFactory");
	provFactoryDag->DefInputEvent("RawEvent","TQClamDagobertRawEvent");
	provFactoryDag->DefEventProcessor("RawEvent_D","TQClamDagobertProviderProc");
	provFactoryDag->DefOutputEvent("Dummy","TQClamDagobertDummyEvent");
	auto* provStepDag = new TGo4AnalysisStep("DagRawProvider",provFactoryDag,nullptr,nullptr,nullptr);
	AddAnalysisStep(provStepDag);
	provStepDag->SetSourceEnabled(kFALSE);
	provStepDag->SetProcessEnabled(kTRUE);
	provStepDag->SetStoreEnabled(kFALSE);


	// Step 2.1: Dagobert Hit
	TGo4StepFactory* dagHitFactory = new TGo4StepFactory("DagobertHitFactory");

	//dagHitFactory->DefInputEvent("RawEvent", "TQClamDagobertRawEvent");
	dagHitFactory->DefEventProcessor("DagobertHitProc", "TDagobertHitProc");
	dagHitFactory->DefOutputEvent("DagobertHitEvent", "TDagobertHitEvent");
	auto* dagHitStore = new TGo4FileStoreParameter("DagobertOutput.root");
	dagHitStore->SetOverwriteMode(kTRUE);

	TGo4AnalysisStep* dagHitStep = new TGo4AnalysisStep("DagobertHitStep", dagHitFactory,
			nullptr,
			dagHitStore,
			nullptr);
	dagHitStep->SetErrorStopEnabled(kFALSE);
	// ueberschrieben durch setup.C
	dagHitStep->SetSourceEnabled(kFALSE);
	dagHitStep->SetStoreEnabled(kFALSE);
	dagHitStep->SetEventStore(dagHitStore);
	dagHitStep->SetProcessEnabled(kTRUE);
	AddAnalysisStep(dagHitStep);



	// Step 2.2: QClam Hit
	TGo4StepFactory* qclamHitFactory = new TGo4StepFactory("QClamHitFactory");

	//qclamHitFactory->DefInputEvent("RawEvent", "TQClamDagobertRawEvent");
	qclamHitFactory->DefEventProcessor("QClamHitProc", "TQClamHitProc");
	qclamHitFactory->DefOutputEvent("QClamHitEvent", "TQClamHitEvent");
	//qclamHitFactory->DefUserEventStore("TQClamDrifttimeCalibStore");
	//auto* qclamHitStore = new TGo4UserStoreParameter("/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run591_to_601/TQClamDrifttimeCalibStore");
	//auto* qclamHitStore = new TGo4UserStoreParameter("DrifttimeCalibStore");
	//qclamHitStore->SetOverwriteMode(kTRUE);

	TGo4AnalysisStep* qclamHitStep = new TGo4AnalysisStep("QClamHitStep", qclamHitFactory,
			nullptr,
			nullptr,//qclamHitStore,
			nullptr);
	qclamHitStep->SetErrorStopEnabled(kTRUE);
	AddAnalysisStep(qclamHitStep);
	// ueberschrieben durch setup.C
	qclamHitStep->SetSourceEnabled(kFALSE);
	//qclamHitStep->SetEventStore(qclamHitStore);
	qclamHitStep->SetStoreEnabled(kFALSE); //TRUE);
	qclamHitStep->SetProcessEnabled(kTRUE);

	// Step 3: Cluster
	TGo4StepFactory* clusterFactory = new TGo4StepFactory("ClusterFactory");
	clusterFactory->DefInputEvent("HitEvent", "TQClamHitEvent");
	clusterFactory->DefEventProcessor("ClusterProc", "TQClamClusterProc");
	clusterFactory->DefOutputEvent("ClusterEvent", "TQClamClusterEvent");
	clusterFactory->DefUserEventStore("TQClamDrifttimeCalibStore");

	if(fDrifttimeStoreEnabled)
	{
		auto* qclamHitStore = new TGo4UserStoreParameter(fDrifttimeStoreName.c_str());
		TGo4AnalysisStep* clusterStep = new TGo4AnalysisStep("ClusterStep", clusterFactory,
				/*source*/ nullptr,
				/*store*/ qclamHitStore, // nullptr,
				/*filter*/ nullptr);
		clusterStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(clusterStep);
		// ueberschrieben durch setup.C
		clusterStep->SetSourceEnabled(kFALSE);
		clusterStep->SetStoreEnabled(kTRUE);
		clusterStep->SetProcessEnabled(kTRUE);
	}
	else
	{
		std::ostringstream ossDrifttime;
		ossDrifttime << "DrifttimeCalibStore" << std::put_time(&tm, "_%Y%m%d_%H%M%S");
		//std::string drifttimeStoreName = ossDrifttime.str();
		std::string drifttimeStoreName = "drifttime_calib_store";

		auto* qclamHitStore = new TGo4UserStoreParameter(drifttimeStoreName.c_str()); //"/d/d01-3/ag_pi/QCLAM_COMM_2026/go4_data/DrifttimeCalibStore_Run_80_to_91_white_noise_fast"); //drifttimeStoreName.c_str()); //"DrifttimeCalibStore");
		TGo4AnalysisStep* clusterStep = new TGo4AnalysisStep("ClusterStep", clusterFactory,
				/*source*/ nullptr,
				/*store*/ qclamHitStore, // nullptr,
				/*filter*/ nullptr);
		clusterStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(clusterStep);
		// ueberschrieben durch setup.C
		clusterStep->SetSourceEnabled(kFALSE);
		clusterStep->SetStoreEnabled(kFALSE);
		clusterStep->SetProcessEnabled(kTRUE);
	}
	
	// Step 4: Track
	TGo4StepFactory* trackFactory = new TGo4StepFactory("TrackFactory");
	trackFactory->DefInputEvent("ClusterEvent", "TQClamClusterEvent");
	trackFactory->DefEventProcessor("TrackProc", "TQClamTrackProc");
	trackFactory->DefOutputEvent("TrackEvent", "TQClamTrackEvent");

	if(fTrackStoreDefaultEnabled)
	{
		trackFactory->DefUserEventStore("TQClamTrackStore");
		auto* trackStore = new TGo4UserStoreParameter(fTrackStoreName.c_str());
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kTRUE);
		trackStep->SetProcessEnabled(kTRUE);
	}
	else if(fTrackStoreEnabled)
	{
		trackFactory->DefUserEventStore("TQClamTrackStore");
		auto* trackStore = new TGo4UserStoreParameter(fTrackStoreName.c_str());
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kTRUE);
		trackStep->SetProcessEnabled(kTRUE);
	}
	else if(fTrackTestingStoreEnabled)
	{
		trackFactory->DefUserEventStore("TQClamTrackTestingStore");
		auto* trackStore = new TGo4UserStoreParameter(fTrackTestingStoreName.c_str());
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kTRUE);
		trackStep->SetProcessEnabled(kTRUE);
	}

	else if(fDriftlengthRelationCalibStoreEnabled)
	{
		trackFactory->DefUserEventStore("TQClamDriftlengthCalibStore");
		auto* trackStore = new TGo4UserStoreParameter(fDriftlengthRelationCalibStoreName.c_str());
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kTRUE);
		trackStep->SetProcessEnabled(kTRUE);
	}


	else
	{

		trackFactory->DefUserEventStore("TQClamTrackStore");
		std::ostringstream ossTrack;
		ossTrack << "TrackStore" << std::put_time(&tm, "_%Y%m%d_%H%M%S");
		std::string trackStoreName = ossTrack.str();
		//std::string trackStoreName = "track_store";

		auto* trackStore = new TGo4UserStoreParameter(trackStoreName.c_str()); //trackStoreName.c_str()); //"/d/d01-3/ag_pi/QCLAM_COMM_2026/go4_data/TrackStore_Run_80_to_91_white_noise"); //trackStoreName.c_str());
		TGo4AnalysisStep* trackStep = new TGo4AnalysisStep("TrackStep", trackFactory,
				/*source*/ nullptr,
				/*store*/ trackStore,
				/*filter*/ nullptr);
		trackStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(trackStep);
		// uebeschrieben durch setup.C
		trackStep->SetSourceEnabled(kFALSE);
		trackStep->SetStoreEnabled(kFALSE);
		trackStep->SetProcessEnabled(kTRUE);
	}




	// Output-Provider-Steps
	
	// Provider fuer QClamDagobertCoincidence -> input: TDagobertHitEvent
	TGo4StepFactory* provFactoryCoincDag = new TGo4StepFactory("CoincDagoFactory");
	provFactoryCoincDag->DefInputEvent("DagobertHitEvent","TDagobertHitEvent");
	provFactoryCoincDag->DefEventProcessor("DagobertHitEvent_D","TQClamDagobertProviderProc");
	provFactoryCoincDag->DefOutputEvent("Dummy","TQClamDagobertDummyEvent");
	auto* provStepCoincDag = new TGo4AnalysisStep("CoincDagProvider",provFactoryCoincDag,nullptr,nullptr,nullptr);
	AddAnalysisStep(provStepCoincDag);
	provStepCoincDag->SetSourceEnabled(kFALSE);
	provStepCoincDag->SetProcessEnabled(kTRUE);
	provStepCoincDag->SetStoreEnabled(kFALSE);
	
	// Provider fuer QClamDagobertCoincidence -> input: TQClamTrackEvent
	TGo4StepFactory* provFactoryCoincQClam = new TGo4StepFactory("CoincQClamFactory");
	provFactoryCoincQClam->DefInputEvent("TrackEvent","TQClamTrackEvent");
	provFactoryCoincQClam->DefEventProcessor("TrackEvent_D","TQClamDagobertProviderProc");
	provFactoryCoincQClam->DefOutputEvent("Dummy","TQClamDagobertDummyEvent");
	auto* provStepCoincQClam = new TGo4AnalysisStep("CoincQClamProvider",provFactoryCoincQClam,nullptr,nullptr,nullptr);
	AddAnalysisStep(provStepCoincQClam);
	provStepCoincQClam->SetSourceEnabled(kFALSE);
	provStepCoincQClam->SetProcessEnabled(kTRUE);
	provStepCoincQClam->SetStoreEnabled(kFALSE);




	// Step 5: QClamDagobertCoincidence
	TGo4StepFactory* coincFactory = new TGo4StepFactory("QClamDagobertCoincidence");
	coincFactory->DefOutputEvent("CoincidenceEvent","TQClamDagobertCoincidenceEvent");
	coincFactory->DefEventProcessor("QClamDagoCoincProc","TQClamDagobertCoincidenceProc");
	//TGo4EventStoreParameter* ; TODO
	coincFactory->DefUserEventStore("TQClamDagoStore");
	if(fCoincStoreEnabled)
	{
		auto* coincStore = new TGo4UserStoreParameter(fCoincStoreName.c_str());
		TGo4AnalysisStep* coincStep = new TGo4AnalysisStep("CoincStep",coincFactory,
				nullptr,
				coincStore, //nullptr,
				nullptr);
		coincStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(coincStep);
		coincStep->SetSourceEnabled(kFALSE);
		coincStep->SetStoreEnabled(kTRUE);
		coincStep->SetProcessEnabled(kTRUE);
	}
	else
	{
		std::ostringstream ossCoinc;
		ossCoinc << "CoincStore" << std::put_time(&tm, "_%Y%m%d_%H%M%S");
		std::string coincStoreName = ossCoinc.str();
		auto* coincStore = new TGo4UserStoreParameter(coincStoreName.c_str()); //"/d/d01-3/ag_pi/QCLAM_COMM_2026/go4_data/CoincStore_Run_80_to_91_white_noise"); //coincStoreName.c_str());
		TGo4AnalysisStep* coincStep = new TGo4AnalysisStep("CoincStep",coincFactory,
				nullptr,
				coincStore, //nullptr,
				nullptr);
		coincStep->SetErrorStopEnabled(kTRUE);
		AddAnalysisStep(coincStep);
		coincStep->SetSourceEnabled(kFALSE);
		coincStep->SetStoreEnabled(kFALSE);
		coincStep->SetProcessEnabled(kTRUE);
	}


#endif





	// ConditionWindows TODO



	/** Histogramme registrieren */

	TGo4Log::Info("[TQClamAnalysis] Constructor: Finish");
}



//***********************************************************
TQClamAnalysis::~TQClamAnalysis()
{
	TGo4Log::Info("[TQClamAnalysis] Delete %s", GetName());
}




/** -------------------------------------------------------
 *  -- Go4-Methoden implementieren
 *  -------------------------------------------------------- **/

//***********************************************************
// fuehre einmalig vor EventLoop aus
// - hole pointer auf input-/outputevents
// - zaehler zuruecksetzen
// - infos ausgeben
Int_t TQClamAnalysis::UserPreLoop()
{
	TGo4Log::Info("[TQClamAnalysis] UserPreLoop: Start");
	Print();

	/** trailing-mapping aus leading + edge-mapping ableiten */
	RebuildTrailingMapping();

	/** baue Geometrie-Objekte */
	BuildDriftcham();


	/** zaehler setzen TODO */
	fEvents = 0;
	fLastEvent = 0;
	fAccumEventSizeGB = 0;
	// ...
	
	/** autosave histogramm storen */
	fHistEventSize = MakeTH1('D', "EventSize", "Event Groesse/kB", 200, 0., 200.); // evt-groesse
	fHistAccumEventSize = MakeTH1('D', "AccumEventSize", "Akkumulierte Event Groesse/GB", 1, 0., 1); // akkumulierte eventgroesse

	/** reset objects to 0 */
	ClearObjects("Histograms");
	ClearObjects("Trees");


	/** hole pointer auf in-/output-events */
	switch(fSourcePar->fSourceType)
	{
#ifdef QCLAM_SINGLE_ROOT
		case TQClamSourceParameter::kQClamOfflineRoot:
		{
			fRawEvent = dynamic_cast<TQClamRawEvent*>(GetInputEvent("HitStep"));
			fHitEvent = dynamic_cast<TQClamHitEvent*>(GetOutputEvent("HitStep"));
			fClusterEvent = dynamic_cast<TQClamClusterEvent*>(GetOutputEvent("ClusterStep"));
			fTrackEvent = dynamic_cast<TQClamTrackEvent*>(GetOutputEvent("TrackStep"));
			break;
		}
#endif
#ifdef QCLAM_SINGLE
		case TQClamSourceParameter::kQClamOfflineLmd:
		{
			fMbsEvent = dynamic_cast<TGo4MbsEvent*>(GetInputEvent("UnpackStep"));
			fRawEvent = dynamic_cast<TQClamRawEvent*>(GetOutputEvent("UnpackStep"));
			fHitEvent = dynamic_cast<TQClamHitEvent*>(GetOutputEvent("HitStep"));
			fClusterEvent = dynamic_cast<TQClamClusterEvent*>(GetOutputEvent("ClusterStep"));
			fTrackEvent = dynamic_cast<TQClamTrackEvent*>(GetOutputEvent("TrackStep"));
			break;
		}
#endif

		default: break;
	}


        //-----------------------------------------------------------
        // initialisiere Pictures
	fPEventMonitor = GetPicture("EventMon");
	if(!fPEventMonitor)
	{
		fPEventMonitor = new TGo4Picture("EventMonitor","Event Monitor");
		fPEventMonitor->SetDivision(3,3);
		fPEventMonitor->Pic(0,0)->AddObject(GetHistogram("ClusterEfficiency"));
		fPEventMonitor->Pic(1,0)->AddObject(GetHistogram("TrackEfficiency"));
#ifdef QCLAM_DAGOBERT
		fPEventMonitor->Pic(2,0)->AddObject(GetHistogram("RelLaBrCoincPerEvent"));
#endif
		fPEventMonitor->Pic(0,1)->AddObject(GetHistogram("XCorrSpectrum"));
		fPEventMonitor->Pic(1,1)->AddObject(GetHistogram("ElectronEnergySpectrum"));
#ifdef QCLAM_DAGOBERT
		fPEventMonitor->Pic(0,2)->AddObject(GetHistogram("Time_LaBr0_vs_MeanSzint"));
		fPEventMonitor->Pic(1,2)->AddObject(GetHistogram("EEPGMatrix_LaBr0"));
#endif
		fPEventMonitor->Pic(2,1)->AddObject(GetHistogram("DrifttimeDistX1TimeWalkCorr"));
		fPEventMonitor->Pic(2,1)->AddObject(GetHistogram("DrifttimeDistUTimeWalkCorr"));
		fPEventMonitor->Pic(2,1)->AddObject(GetHistogram("DrifttimeDistX2TimeWalkCorr"));

		fPEventMonitor->Pic(2,2)->AddObject(GetHistogram("HDrifttimeVsToTTimeWalkCorr"));
		/*
		fPEventMonitor->Pic(2,2)->AddObject(GetHistogram("SzintUpTrigRateGraph"));
		fPEventMonitor->Pic(2,2)->AddObject(GetHistogram("SzintDownTrigRateGraph"));
		fPEventMonitor->Pic(2,2)->AddObject(GetHistogram("CherenkovTrigRateGraph"));
		fPEventMonitor->Pic(2,2)->AddObject(GetHistogram("CoincTrigRateGraph"));
		fPEventMonitor->Pic(2,2)->AddObject(GetHistogram("AccTrigRateGraph"));

		fPEventMonitor->Pic(2,1)->AddObject(GetHistogram("RelDeadtime"));
		fPEventMonitor->Pic(2,1)->AddObject(GetHistogram("RelMbsDeadtime"));
		*/
		AddPicture(fPEventMonitor);
	}


	fPEventMonitor = GetPicture("EventMonShifts");
	if(!fPEventMonitor_Shifts)
	{
		fPEventMonitor_Shifts = new TGo4Picture("EventMonitorShifts","Event Monitor (Shifts)");
		fPEventMonitor_Shifts->SetDivision(2,2);
		fPEventMonitor_Shifts->Pic(0,0)->AddObject(GetHistogram("ClusterEfficiency"));

		fPEventMonitor_Shifts->Pic(1,0)->AddObject(GetHistogram("TrackEfficiency"));

		fPEventMonitor_Shifts->Pic(0,1)->AddObject(GetHistogram("XCorrSpectrum"));

		fPEventMonitor_Shifts->Pic(1,1)->AddObject(GetHistogram("DrifttimeDistX1TimeWalkCorr"));
		fPEventMonitor_Shifts->Pic(1,1)->AddObject(GetHistogram("DrifttimeDistUTimeWalkCorr"));
		fPEventMonitor_Shifts->Pic(1,1)->AddObject(GetHistogram("DrifttimeDistX2TimeWalkCorr"));

		AddPicture(fPEventMonitor_Shifts);
	}






	TGo4Log::Info("[TQClamAnalysis] UserPreLoop: Finish");
	return 42;
}




//***********************************************************
// - infos ausgeben
// - evtl. statistiken ausgeben
// - letztes event inspizieren
Int_t TQClamAnalysis::UserPostLoop()
{
	//TODO
	TGo4Log::Info("[TQClamAnalysis] UserPostLoop: Start");
	TGo4Log::Info("Letzes Event: %d, Anzahl analysierte Events: %d", fLastEvent, fEvents);

	if(fMbsEvent) // falls mbsevent --> inspiziere letztes event
	{
		TGo4Log::Info("[TQClamAnalysis] Inspiziere letztes Event:");
//		fMbsEvent->PrintEvent(); // header & data content

		fMbsEvent->PrintMbsFileHeader(); // fileheader

		fMbsEvent->PrintMbsBufferHeader(); // mbs buffer header structure
	}

	/** reset in-/outputevent-pointer */
	fMbsEvent = nullptr;
	fRawEvent = nullptr;
	fHitEvent = nullptr;
	fClusterEvent = nullptr;
	fTrackEvent = nullptr;

	fEvents = 0;
	fLastEvent = 0;

	TGo4Log::Info("[TQClamAnalysis] UserPostLoop: Finish");
	return 42;
}



//***********************************************************
// wird fuer jedes event einmal aufgerufen
// - histogramme fuellen
// - zaehler inkrementieren
// - ...
Int_t TQClamAnalysis::UserEventFunc()
{
//	TGo4Log::Info("Event %d verarbeitet", fEvents);


	auto ana = TGo4Analysis::Instance();


	/** eventgroesse bestimmen */
	Int_t kilobytes = 0;
	if(fMbsEvent)
	{
		// ermittle Mbs-Eventgroesse in MB (32 bit)
		// GetDlen --> eventgroesse in bytes
		// +4 --> eventheader 4 bytes gross
		// /(1024) --> umrechnung byte -> kilobyte
		kilobytes = (fMbsEvent->GetDlen()+4)/1024;
	}

	/** akkumulierte eventgroesse in GB */
	fAccumEventSizeGB += (kilobytes/(1024*1024));

	// histogramme fuellen
	fHistEventSize->Fill(kilobytes);
	fHistAccumEventSize->SetBinContent(1, fAccumEventSizeGB);

	/** eventzaehler */
	fEvents++;
//	TGo4Log::Info("fEvents=%d",fEvents);
	
	// LastEvent irrefuehrend ... aendern TODO
	if(fEvents == 10 || IsNewInputFile())
	{
		if(fMbsEvent)
		{
			fLastEvent = fMbsEvent->GetCount();
//			TGo4Log::Info("Erstes Event #: %d", count);
			fMbsEvent->PrintMbsBufferHeader();
		}
	}


	switch(fSourcePar->fSourceType)
	{
		case TQClamSourceParameter::kQClamOfflineRoot:
		{
			int eventnumber = fRawEvent->eventNumber;
			size_t nHits = fHitEvent->hits.size();
		}

		case TQClamSourceParameter::kQClamOfflineLmd:
		{
			size_t nHits = fHitEvent->hits.size();
		}
	}


	/** TODO: hierueber alle Events holen
	 * --> EventViewer bauen
	 * --> GetOutputEvent("step_name")
	 * --> GetInputEvent("step_name") */


	//int triggerID = fRawEvent->triggerID;

	return 42;
}







/** -------------------------------------------------------
 *  -- Configs einlesen und Go4Parameter fuellen
 *  -------------------------------------------------------- **/

//***********************************************************
// static Hilfsmethode zum Bauen von absoluten Pfaden
static std::string ResolvePath(const std::string& rel)
{
	/** in CMakeLists.txt ist der Projektpfad definiert: 
	  * PROJECT_SOURCE_DIR
	  * --> erhalte Pfade (von configs) relativ zu Projektpfad
	  * und baue absoluten Pfad */
	return std::string(PROJECT_SOURCE_DIR) + "/" + rel;
}





//***********************************************************
// gebe nur runnummer & prefix, baue daraus komplette filenamen
void TQClamAnalysis::AppendRunFiles(const TString& prefix,size_t& nextIndex)
{
	for(int file = 0;; ++file)
	{
		TString filename = Form("%s_%04d.lmd", prefix.Data(), file);

		if(gSystem->AccessPathName(filename)) { break; }

		TGo4Log::Info("[TQClamAnalysis]    %ld: %s", nextIndex, filename.Data());

		fSourcePar->fLmdFiles[nextIndex++] = filename;
	}
}




//***********************************************************
// gebe runnummer & filenummer & prefix, baue daraus komplette filenamen
void TQClamAnalysis::AppendFileList(const std::string& filename,size_t& nextIndex)
{
	TGo4Log::Info("[TQClamAnalysis] Verarbeite File-Liste: %s",filename.c_str());


	//------------------------------------------------------------------
        // File-Liste öffnen
        //------------------------------------------------------------------

        std::ifstream fin(filename);

        if(!fin.is_open())
        {
		TGo4Log::Error("[TQClamAnalysis] Kann File-Liste %s nicht öffnen.",filename.c_str());

		TGo4RuntimeException();
        }


        //------------------------------------------------------------------
        // Zeilenweise lesen
        //------------------------------------------------------------------

        std::string line;

        while(std::getline(fin, line))
        {
                //------------------------------------------------------------------
                // Leere Zeilen überspringen
                //------------------------------------------------------------------

                if(line.empty())
                {
                        continue;
                }


                //------------------------------------------------------------------
                // Kommentare überspringen
                //------------------------------------------------------------------

                if(line[0] == '#')
                {
                        continue;
                }


                //------------------------------------------------------------------
                // Maximale Anzahl Dateien prüfen
                //------------------------------------------------------------------

                if(nextIndex >= fSourcePar->MAX_LMD_FILES)
                {
                        TGo4Log::Error("[TQClamAnalysis] Maximale Anzahl von %d LMD-Dateien erreicht.",fSourcePar->MAX_LMD_FILES);

                        break;
                }


                //------------------------------------------------------------------
                // Zeile parsen
                //
                // Format:
                //
                // eepg_2026_commissioning_beamtime qclam_dago 146 0006
                //
                //        experiment                prefix    run file
                //------------------------------------------------------------------

                std::istringstream iss(line);

                std::string experiment;
                std::string prefix;

                int runNumber = -1;
                int fileNumber = -1;


                if(!(iss >> experiment >> prefix >> runNumber >> fileNumber))
                {
                        TGo4Log::Error("[TQClamAnalysis] Ungültige Zeile in File-Liste: %s",line.c_str());

                        continue;
                }


                //------------------------------------------------------------------
                // Dateiname erzeugen
                //
                // qclam_dago_146_0006.lmd
                //------------------------------------------------------------------

                TString fileName = Form("%s_%03d_%04d.lmd",prefix.c_str(),runNumber,fileNumber);


                //------------------------------------------------------------------
                // Vollständigen Pfad erzeugen
                //------------------------------------------------------------------

                TString fullPath;

                if(fSourcePar->file_directory.empty())
                {
                        fullPath = fileName;
                }
                else
                {
                        fullPath = Form("%s/%s",fSourcePar->file_directory.c_str(),fileName.Data());
                }


                //------------------------------------------------------------------
                // Prüfen, ob Datei existiert
                //------------------------------------------------------------------

                if(gSystem->AccessPathName(fullPath.Data()))
                {
                        TGo4Log::Error("[TQClamAnalysis] Datei existiert nicht: %s",fullPath.Data());

                        continue;
                }


                //------------------------------------------------------------------
                // Datei in SourceParameter eintragen
                //------------------------------------------------------------------

                fSourcePar->fLmdFiles[nextIndex] = fullPath;


                //------------------------------------------------------------------
                // Logging
                //------------------------------------------------------------------

                TGo4Log::Info("[TQClamAnalysis] File %ld:",nextIndex);

                TGo4Log::Info("[TQClamAnalysis]   Experiment : %s",experiment.c_str());

                TGo4Log::Info("[TQClamAnalysis]   Prefix     : %s",prefix.c_str());

                TGo4Log::Info("[TQClamAnalysis]   Run        : %03d",runNumber);

                TGo4Log::Info("[TQClamAnalysis]   File       : %04d",fileNumber);

                TGo4Log::Info("[TQClamAnalysis]   Full path  : %s",fullPath.Data());


                //------------------------------------------------------------------
                // Index erhöhen
                //------------------------------------------------------------------

                ++nextIndex;
        }


        fin.close();
}




//***********************************************************
// Konfirguriere master-parameter (pfade zu allen configs)
void TQClamAnalysis::InitMasterParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamMasterParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));

	/** pfad zu master-config bauen */
	std::string path = ResolvePath(master->fMasterConfigFile.Data() /** hard-coded in Parameter */);

	YAML::Node cfg_master = YAML::LoadFile(path);
	auto configs = cfg_master["configs"];

	/** fuelle Master-Parameter */
	master->fSourceConfigFile = ResolvePath(configs["source"].as<std::string>());
	master->fDriftchamConfigFile = ResolvePath(configs["geometry"].as<std::string>());
	master->fTdcMappingConfigFile = ResolvePath(configs["tdc_mapping"].as<std::string>());
	master->fTriggerMappingConfigFile = ResolvePath(configs["trigger_mapping"].as<std::string>());
	master->fTriggerConditionConfigFile = ResolvePath(configs["trigger_condition"].as<std::string>());
	master->fSignalEdgeConfigFile = ResolvePath(configs["signal_edge"].as<std::string>());
	master->fDrifttimeConfigFile = ResolvePath(configs["drifttime"].as<std::string>());
	master->fDrifttimeTimeWalkCorrFile = ResolvePath(configs["time_walk_corr"].as<std::string>());
	master->fHitConditionConfigFile = ResolvePath(configs["hit_condition"].as<std::string>());
	master->fHitProcDebugOutputConfigFile = ResolvePath(configs["hitproc_debug_output"].as<std::string>());
	master->fFaradayCupCurrentCalibrationFile = ResolvePath(configs["faraday_cup_current_calibration"].as<std::string>());
	master->fDisplaySettingConfigFile = ResolvePath(configs["display_settings"].as<std::string>());
	master->fClusterConditionConfigFile = ResolvePath(configs["cluster_condition"].as<std::string>());
	master->fTrackConditionConfigFile = ResolvePath(configs["track_condition"].as<std::string>());
	master->fDriftToZOffsetTable = ResolvePath(configs["drift_to_zOffset_table"].as<std::string>());
	master->fFocalPlaneConfigFile = ResolvePath(configs["focal_plane"].as<std::string>());
	master->fSpecCalibFile = ResolvePath(configs["spectrum_calibration"].as<std::string>());
	master->fFitConfigFile = ResolvePath(configs["spectrum_fit"].as<std::string>());
	master->fSpecCalibFile = ResolvePath(configs["spectrum_calibration"].as<std::string>());
	master->fDrifttimeCalibConfigFile = ResolvePath(configs["drifttime_calibration"].as<std::string>());
	master->fUnpackConfigFile = ResolvePath(configs["unpack_config"].as<std::string>());

#ifdef QCLAM_DAGOBERT
	master->fTcpConnectionFile = ResolvePath(configs["tcp_connection"].as<std::string>());
	master->fDagoCalibrationFile = ResolvePath(configs["dagobert_calibration"].as<std::string>());
#endif

	master->fStoreConfigFile = ResolvePath(configs["store"].as<std::string>());
	master->fHistoFillingConfigFile = ResolvePath(configs["histogram_filling"].as<std::string>());

	TGo4Log::Info("[TQClamAnalysis] TQClamMasterParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere SourceParameter
void TQClamAnalysis::InitSourceParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamSourceParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	fSourcePar = dynamic_cast<TQClamSourceParameter*>(GetParameter("SourceParam", "TQClamSourceParameter"));

	fSourcePar->fFiles.clear();

	for(size_t i=0; i<fSourcePar->MAX_LMD_FILES; i++)
	{
		fSourcePar->fLmdFiles[i] = "";
	}
	/*
	for(size_t n=0; n<source->MAX_FILES; n++)
	{
		source->fFiles[n] = "";
	}
	*/

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fSourceConfigFile.Data());
	auto src = cfg["source"];
	auto filesrc = src["filesource"];
	auto filesrc_lmd = src["filesource_lmd"];

	fSourcePar->file_directory = src["file_directory"].as<std::string>();	//-- directory von rohfiles des jew. exp
	fSourcePar->store_directory = src["store_directory"].as<std::string>();	//-- directory fuer output des jew. exp
	fSourcePar->fStreamServer = src["StreamServer"].as<std::string>();	//-- name stream-server fuer online analyse
	Int_t source_type = src["source_type"].as<Int_t>();			//-- art der analyse online/offline ...




	if(fTrackStoreDefaultEnabled)
	{
		TString storeDirectory = fSourcePar->store_directory.c_str();

		if(!storeDirectory.EndsWith("/"))
		{
			storeDirectory += "/";
		}

		fTrackStoreName = storeDirectory + fTrackStoreDefaultName;
		fTrackStoreEnabled = true;

		TGo4Log::Info("[TQClamAnalysis] Track-Store: %s",fTrackStoreName.data());
	}


#ifdef QCLAM_SINGLE_ROOT
	if(source_type == 0)
	{
		TGo4Log::Info("[TQClamAnalysis] Modus: QCLAM_SINGLE_ROOT. Aktiviere Offline-Root-Source.");
		fSourcePar->fSourceType = TQClamSourceParameter::kQClamOfflineRoot;



		// offline-Modus und Source-list uebergeben
		if(fInputListGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende Input-Liste: %s",fInputList.c_str());

			std::ifstream fin(fInputList);

			if(!fin.is_open())
			{
				TGo4Log::Error("Kann Input-Liste %s nicht öffnen.",fInputList.c_str());
				TGo4RuntimeException();
			}

			TString prefix;
			size_t nextIndex = 0;

			while(fin >> prefix)
			{
				AppendRunFiles(prefix,nextIndex);
				//fSourcePar->fLmdFiles[idx++] = filename;

			}

			return;
		}

		// ---------------------------------------------------------
		// Eine einzelne LMD-Datei wurde übergeben
		// ---------------------------------------------------------
		if(fInputFileGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende einzelne Input-Datei: %s",fInputFile.c_str());

			size_t nextIndex = 0;

			fSourcePar->fLmdFiles[nextIndex++] = fInputFile.c_str();

			return;
		}


		if(fInputFileListGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende File-Liste: %s",fInputFileList.c_str());

			size_t nextIndex = 0;

			AppendFileList(fInputFileList,nextIndex);

			return;
		}



		// ---------------------------------------------------------
		// Ein kompletter Run wurde übergeben
		// ---------------------------------------------------------
		if(fInputPrefixGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende Input-Prefix: %s",fInputPrefix.c_str());

			size_t nextIndex = 0;
			AppendRunFiles(fInputPrefix.c_str(), nextIndex);

			return;
		}




		std::vector<TString> selectedFiles;
		for (const auto& entry : filesrc)
		{
			if(!entry || !entry.IsMap()) { continue; }

			bool use = entry["use"] && entry["use"].IsScalar() ? entry["use"].as<bool>() : false;
			if(!use) { continue; }

			TString folder = "";
			if(entry["folder"] && entry["folder"].IsScalar())
			{
				folder = entry["folder"].as<std::string>();
			}

			if(!entry["files"] || !entry["files"].IsMap()) { continue; }

			auto files = entry["files"];
			for (const auto& file : files)
			{
				if(!file.second || !file.second.IsScalar()) { continue; }

				TString filename = file.second.as<std::string>();
				TString fullpath = folder.Length() > 0 ? folder + filename : filename;

				selectedFiles.push_back(fullpath);
				TGo4Log::Info("[TQClamAnalysis]   -> %s", fullpath.Data());
			}
		}

		// Kommagetrennten String erzeugen
		TString filelist;
		for (size_t i = 0; i < selectedFiles.size(); i++)
		{
			filelist += selectedFiles[i];
			if(i + 1 < selectedFiles.size()) { filelist += ","; }
		}

		// In SourceParameter speichern
		fSourcePar->fName = filelist;
	}

	else
	{
		TGo4Log::Error("[TQClamAnalysis] Source-Typ falsch oder nicht moeglich. Modus: QCLAM_SINGLE_ROOT!");
		TGo4RuntimeException();
	}


#endif
#ifdef QCLAM_SINGLE
	if(source_type == 1)
	{
		TGo4Log::Info("[TQClamAnalysis] Modus: QCLAM_SINGLE. Aktiviere Offline-Lmd-Source.");
		fSourcePar->fSourceType = TQClamSourceParameter::kQClamOfflineLmd;


		// offline-Modus und Source-list uebergeben
		if(fInputListGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende Input-Liste: %s",fInputList.c_str());

			std::ifstream fin(fInputList);

			if(!fin.is_open())
			{
				TGo4Log::Error("Kann Input-Liste %s nicht öffnen.",fInputList.c_str());
				TGo4RuntimeException();
			}

			TString prefix; //filename;
			size_t nextIndex = 0; //idx = 0;

			while(fin >> prefix)
			{
				AppendRunFiles(prefix,nextIndex);
				fSourcePar->fLmdFiles[idx++] = filename;

			}

			return;
		}


		// ---------------------------------------------------------
		// Eine einzelne LMD-Datei wurde übergeben
		// ---------------------------------------------------------
		if(fInputFileGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende einzelne Input-Datei: %s",fInputFile.c_str());

			size_t nextIndex = 0;

			fSourcePar->fLmdFiles[nextIndex++] = fInputFile.c_str();

			return;
		}



		// ---------------------------------------------------------
		// Ein kompletter Run wurde übergeben
		// ---------------------------------------------------------
		if(fInputPrefixGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende Input-Prefix: %s",fInputPrefix.c_str());

			size_t nextIndex = 0;
			AppendRunFiles(fInputPrefix.c_str(), nextIndex);

			return;
		}


		if(fInputFileListGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende File-Liste: %s",fInputFileList.c_str());

			size_t nextIndex = 0;

			AppendFileList(fInputFileList,nextIndex);

			return;
		}





		TGo4Log::Info("[TQClamAnalysis] Input-Files:");

		if(!filesrc_lmd || !filesrc_lmd.IsSequence())
		{
			TGo4Log::Error("filesource_lmd ist kein gültiges YAML-Array! Korrigiere Source-Config!");
			return;
		}


		size_t nextIndex = 0;

		for(const auto& entry : filesrc_lmd)
		{
			if(!entry || !entry.IsMap()) { continue; }

			bool use = false;
			if(entry["use"] && entry["use"].IsScalar()) { use = entry["use"].as<bool>(); }
			if(!use) { continue; }

			TString folder;
			if(entry["folder"] && entry["folder"].IsScalar())
			{
				folder = entry["folder"].as<std::string>();
			}
			else { folder = ""; }
	
			if(!entry["files"] || !entry["files"].IsMap()) { continue; }
	
			auto files = entry["files"];
			for(const auto& file : files)
			{
				if(!file.second || !file.second.IsScalar()) { continue; }
				int index = file.first.as<int>();
				TString filename = file.second.as<std::string>();
	
				TString fullpath;
				if(folder.Length() > 0) { fullpath = folder + filename; }
				else { fullpath = filename; }
	
				TGo4Log::Info("[TQClamAnalysis]    %ld: %s", nextIndex, fullpath.Data());
				//source->fFiles[index] = fullpath;
				fSourcePar->fLmdFiles[nextIndex] = fullpath;

				nextIndex++;
			}
		}


	}
	else if(source_type == 2)
	{
		TGo4Log::Info("[TQClamAnalysis] Modus: QCLAM_SINGLE. Aktiviere Mbs-Stream als Source.");
		fSourcePar->fSourceType = TQClamSourceParameter::kQClamMbsStream;
	}
	else
	{
		TGo4Log::Error("[TQClamAnalysis] Source-Typ falsch oder nicht moeglich. Modus: QCLAM_SINGLE!");
		TGo4RuntimeException();
	}

#endif // QCLAM_SINGLE

#ifdef QCLAM_DAGOBERT
	if(source_type == 3)
	{
		TGo4Log::Info("[TQClamAnalysis] Modus: QCLAM_DAGOBERT. Aktiviere Offline-Lmd-Source.");
		fSourcePar->fSourceType = TQClamSourceParameter::kQClamDagobertOfflineLmd;




		// offline-Modus und Source-list uebergeben
		if(fInputListGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende Input-Liste: %s",fInputList.c_str());

			std::ifstream fin(fInputList);

			if(!fin.is_open())
			{
				TGo4Log::Error("Kann Input-Liste %s nicht öffnen.",fInputList.c_str());
				TGo4RuntimeException();
			}



			TString prefix; //filename;
			size_t nextIndex = 0; //idx = 0;

			while(fin >> prefix)
			{
				AppendRunFiles(prefix,nextIndex);
				//fSourcePar->fLmdFiles[idx++] = filename;

			}

			return;
		}



		// ---------------------------------------------------------
		// Eine einzelne LMD-Datei wurde übergeben
		// ---------------------------------------------------------
		if(fInputFileGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende einzelne Input-Datei: %s",fInputFile.c_str());

			size_t nextIndex = 0;

			TString fullpath;

			if(fSourcePar->file_directory.length() > 0)
			{
				fullpath = fSourcePar->file_directory + fInputFile.c_str();
			}
			else
			{
				fullpath = fInputFile.c_str();
			}

			TGo4Log::Info("[TQClamAnalysis] Vollständiger Input-Pfad: %s",fullpath.Data());

			fSourcePar->fLmdFiles[nextIndex++] = fullpath;

			return;
		}

		/*
		if(fInputFileGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende einzelne Input-Datei: %s",fInputFile.c_str());

			size_t nextIndex = 0;

			fSourcePar->fLmdFiles[nextIndex++] = fInputFile.c_str();

			return;
		}
		*/



		// ---------------------------------------------------------
		// Ein kompletter Run wurde übergeben
		// ---------------------------------------------------------
		if(fInputPrefixGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende Input-Prefix: %s",fInputPrefix.c_str());

			size_t nextIndex = 0;
			AppendRunFiles(fInputPrefix.c_str(), nextIndex);

			return;
		}


		if(fInputFileListGiven)
		{
			TGo4Log::Info("[TQClamAnalysis] Verwende File-Liste: %s",fInputFileList.c_str());

			size_t nextIndex = 0;

			AppendFileList(fInputFileList,nextIndex);

			return;
		}





		TGo4Log::Info("[TQClamAnalysis] Input-Files:");

		if(!filesrc_lmd || !filesrc_lmd.IsSequence())
		{
			TGo4Log::Error("filesource_lmd ist kein gültiges YAML-Array! Korrigiere Source-Config!");
			return;
		}

		size_t nextIndex = 0;

		for(const auto& entry : filesrc_lmd)
		{
			if(!entry || !entry.IsMap()) { continue; }

			bool use = false;
			if(entry["use"] && entry["use"].IsScalar()) { use = entry["use"].as<bool>(); }
			if(!use) { continue; }

			TString folder;
			if(entry["folder"] && entry["folder"].IsScalar())
			{
				folder = entry["folder"].as<std::string>();
			}
			else { folder = ""; }
	
			// ------------------------------------------------------------------------
			// Variante 1: Dateien explizit angegeben
			// ------------------------------------------------------------------------
			if(entry["files"] && entry["files"].IsMap())
			{
	
				auto files = entry["files"];
				for(const auto& file : files)
				{
					if(!file.second || !file.second.IsScalar()) { continue; }
					int index = file.first.as<int>();
					TString filename = file.second.as<std::string>();
	
					TString fullpath;
					if(folder.Length() > 0) { fullpath = folder + filename; }
					else { fullpath = filename; }
	
					TGo4Log::Info("[TQClamAnalysis]    %ld: %s", nextIndex, fullpath.Data());
					//source->fFiles[index] = fullpath;
					fSourcePar->fLmdFiles[nextIndex] = fullpath;

					nextIndex++;
				}
			}



			// ------------------------------------------------------------------------
			// Variante 2: automatische Dateisuche anhand der Runnummern
			// ------------------------------------------------------------------------
			else if(entry["runs"] && entry["runs"].IsSequence()
					&& entry["prefix"] && entry["prefix"].IsScalar())
			{
				TString prefix = entry["prefix"].as<std::string>();

				for(const auto& runNode : entry["runs"])
				{
					int run = runNode.as<int>();

					for(int file = 0; ; ++file)
					{
						TString filename =
							Form("%s_%03d_%04d.lmd",prefix.Data(),run,file);

						TString fullpath;
						if(folder.Length() > 0)
						{
							fullpath = folder + filename;
						}
						else { fullpath = filename; }

						// Datei existiert nicht -> nächster Run
						if(gSystem->AccessPathName(fullpath)) { break; }

						TGo4Log::Info("[TQClamAnalysis]    %ld: %s",
								nextIndex, fullpath.Data());

						fSourcePar->fLmdFiles[nextIndex++] = fullpath;
					}
				}
			}



		}


	}
	else if(source_type == 4)
	{
		TGo4Log::Info("[TQClamAnalysis] QCLAM_DAGOBERT. Aktiviere Mbs-Stream als Source.");
		fSourcePar->fSourceType = TQClamSourceParameter::kQClamDagobertMbsStream;
	}
	else
	{
		TGo4Log::Error("[TQClamAnalysis] Source-Typ falsch oder nicht moeglich. Modus: QCLAM_DAGOBERT!");
		TGo4RuntimeException();
	}
#endif



		


//	source->fFiles.push_back("/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/unpacked_qclam_data_for_testing/4x1/noise2026_032_0000.root");

	// TODO

	TGo4Log::Info("[TQClamAnalysis] TQClamSourceParameter erfolgreich konfiguriert.");
}


//***********************************************************
// Konfiguriere DriftkammerParameter
void TQClamAnalysis::InitDriftchamGeomParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamDriftchamGeomParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	auto* driftcham = dynamic_cast<TQClamDriftchamGeomParameter*>(GetParameter("DriftchamGeomParam", "TQClamDriftchamGeomParameter"));

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fDriftchamConfigFile.Data());
	auto cfg_driftcham = cfg["driftcham"];
	auto cfg_planes = cfg_driftcham["planes"];

	driftcham->driftchamID = cfg_driftcham["id"].as<int>();
	driftcham->generation = cfg_driftcham["generation"].as<std::string>();
	driftcham->nPlanes = cfg_planes.size();

	/** sicherheitshalber vectoren in TQClamDriftchamGeomParameter leeren */
	for(Int_t layer=0; layer<TQClamDriftchamGeomParameter::MAX_LAYER; layer++)
	{
		driftcham->planeName[layer] = "";
		driftcham->planeType[layer] = TQClamDriftchamGeomParameter::NONE;
		driftcham->nWires[layer] = 0;
		driftcham->zPos[layer] = 0;
		driftcham->xOffset[layer] = 0;
		driftcham->yOffset[layer] = 0;
		driftcham->xLength[layer] = 0;
		driftcham->yLength[layer] = 0;
		driftcham->wireDist[layer] = 0;
		driftcham->angleDeg[layer] = 0;
	}

	/*
	driftcham->planeName.clear();
//	driftcham->planeName.Expand(nPlanes);
	driftcham->planeType.clear();
//	driftcham->zPos.Set(driftcham->nPlanes);
	driftcham->xOffset.clear();
	driftcham->yOffset.clear();
	driftcham->xLength.clear();
	driftcham->yLength.clear();
	driftcham->wireDist.clear();
	driftcham->angleDeg.clear();
	driftcham->nWires.clear();
	*/

	/** pruefe */

	/** MAX_CH ueberschritten? */
	if(driftcham->nPlanes < 0 || driftcham->nPlanes > TQClamDriftchamGeomParameter::MAX_LAYER)
	{
		TGo4Log::Error("[TQClamAnalysis::InitDriftchamGeomParameter] Channel index %d out of range (0..%d)",
				driftcham->nPlanes, TQClamDriftchamGeomParameter::MAX_LAYER-1);
		throw TGo4RuntimeException();
	}
	/** lese Config-Parameter fuer Ebenen ein */
	for(int i=0; i<driftcham->nPlanes; i++)
	{
		auto cfg_p = cfg_planes[i];

		if(i < 0 || i >= TQClamDriftchamGeomParameter::MAX_LAYER)
		{
			TGo4Log::Error("[TQClamAnalysis::InitDriftchamGeomParameter] Layer index %d out of range (0..%d)",
					i, TQClamDriftchamGeomParameter::MAX_LAYER-1);
			throw TGo4RuntimeException();
		}
		auto name = cfg_p["name"].as<std::string>();
		driftcham->planeName[i] = name;
		//driftcham->planeName.AddAt(new TObjString(name), i);

		if(name[0] == 'X' || name[0] == 'x')
		{
//			driftcham->planeType.push_back(TQClamDriftchamGeomParameter::X);
			driftcham->planeType[i] = TQClamDriftchamGeomParameter::X;
		}
		else if(name[0] == 'U' || name[0] == 'u')
		{
//			driftcham->planeType.push_back(TQClamDriftchamGeomParameter::U);
			driftcham->planeType[i] = TQClamDriftchamGeomParameter::U;
		}
		else
		{
			driftcham->planeType[i] = TQClamDriftchamGeomParameter::NONE;

			TGo4Log::Error("[TQClamAnalysis] Unbekannter 'name' (%s) in %s. Brauche 'X' oder 'x' fuer X-Ebenen an erster Stelle und 'U' oder 'u' fuer U-Ebenen an erster Stelle!", name.c_str(), master->fDriftchamConfigFile.Data());
			TGo4RuntimeException();
		}

		//driftcham->zPos.push_back(cfg_p["z_pos"].as<double>());
		driftcham->zPos[i] = cfg_p["z_pos"].as<Double_t>();
		driftcham->xOffset[i] = cfg_p["x_offset"].as<Double_t>();
		driftcham->yOffset[i] = cfg_p["y_offset"].as<Double_t>();
		driftcham->xLength[i] = cfg_p["x_length"].as<Double_t>();
		driftcham->yLength[i] = cfg_p["y_length"].as<Double_t>();
		driftcham->wireDist[i] = cfg_p["wire_dist"].as<Double_t>();
		driftcham->angleDeg[i] = cfg_p["angle_deg"].as<Double_t>();
		driftcham->nWires[i] = cfg_p["n_wires"].as<Int_t>();

	TGo4Log::Info("GEOM-LOAD: layer=%d name=%s | zPos=%f | xOffset=%f | wireDist=%f",
              i,
              driftcham->planeName[i].Data(),
              driftcham->zPos[i],
              driftcham->xOffset[i],
              driftcham->wireDist[i]);
	}


	TGo4Log::Info("[TQClamAnalysis] TQClamDriftchamGeomParameter erfolgreich konfiguriert.");
}



//***********************************************************
// Konfiguriere EdgeMappingParameter
void TQClamAnalysis::InitEdgeMappingParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamEdgeMappingParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	auto* edge = dynamic_cast<TQClamEdgeMappingParameter*>(GetParameter("EdgeMappingParam", "TQClamEdgeMappingParameter"));

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fSignalEdgeConfigFile.Data());
	auto node = cfg["signalEdge"];

	/*
	edge->needTdcTrigger = node["needTdcTrigger"].as<Bool_t>();
	edge->invertEdges = node["invertSignalEdges"].as<Bool_t>();
	*/

	auto trig = node["trig"];
	auto lead = node["lead"];
	auto trail = node["trail"];
	auto pairs = node["pairs"];


	/** trigger channel */
	for(auto tdcEntry : trig)
	{
		std::string tdcName = tdcEntry.first.as<std::string>();
		int tdc = std::stoi(tdcName.substr(3));
		
		int ch = tdcEntry.second["channel"].as<int>();

		edge->tdcTriggerChannel[tdc] = ch;
	}



	/** leading channel */
	for(auto tdcEntry : lead)
	{
		std::string tdcName = tdcEntry.first.as<std::string>();
		int tdc = std::stoi(tdcName.substr(3));

		int chFirst = tdcEntry.second["first"].as<int>();
		int chLast = tdcEntry.second["last"].as<int>();

		for(int i=chFirst; i<=chLast; i++)
		{
			edge->edgeType[tdc][i] = EdgeType::LEADING;
		}
	
	}



	/** trailing channel */
	for(auto tdcEntry : trail)
	{
		std::string tdcName = tdcEntry.first.as<std::string>();
		int tdc = std::stoi(tdcName.substr(3));

		int chFirst = tdcEntry.second["first"].as<int>();
		int chLast = tdcEntry.second["last"].as<int>();

		for(int i=chFirst; i<=chLast; i++)
		{
			edge->edgeType[tdc][i] = EdgeType::TRAILING;
		}

	}



	/** pairs: leading <-> trailing */
	for(auto tdcEntry : pairs)
	{
		std::string tdcName = tdcEntry.first.as<std::string>();
		int tdc = std::stoi(tdcName.substr(3));

		auto leadNode = tdcEntry.second["leading"];
		auto trailNode = tdcEntry.second["trailing"];

		int leadFirst = leadNode["first"].as<int>();
		int leadLast = leadNode["last"].as<int>();

		int trailFirst = trailNode["first"].as<int>();
		int trailLast = trailNode["last"].as<int>();

		int nLead = leadLast - leadFirst + 1;
		int nTrail = trailLast - trailFirst + 1;
		if(nLead != nTrail)
		{
			TGo4Log::Error("[TQClamAnalysis] Error in InitEdgeMappingParameter, look at pairing (leading <-> trailing) in %s", master->fSignalEdgeConfigFile.Data());
			/** RuntimeException? TODO */
		}

		for(int i=0; i<nLead; i++)
		{
			int leadCh = leadFirst + i;
			int trailCh = trailFirst + i;

			edge->leadingToTrailing[tdc][leadCh] = trailCh;
			edge->trailingToLeading[tdc][trailCh] = leadCh;
		}
	}


	TGo4Log::Info("[TQClamAnalysis] TQClamEdgeMappingParameter erfolgreich konfiguriert.");
}






//***********************************************************
// Konfiguriere DriftchamCalibrationParameter
void TQClamAnalysis::InitDriftchamCalibrationParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamDriftchamCalibrationParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	auto* cali = dynamic_cast<TQClamDriftchamCalibrationParameter*>(GetParameter("DriftchamCalibParam", "TQClamDriftchamCalibrationParameter"));


	/** - zu Beginn mit 0 initialisieren, um Komplikationen zu verhindern - */
        cali->numTDC = -1;

        /** alle Werte mit -1 initialisieren */
        for(Int_t tdc=0; tdc<TQClamDriftchamCalibrationParameter::MAX_TDC; tdc++)
        {
               	cali->numCh[tdc] = -1;
                for(Int_t ch=0; ch<TQClamDriftchamCalibrationParameter::MAX_CH; ch++)
                {
                        cali->layerMapLead[tdc][ch] = -1;
                        cali->wireMapLead[tdc][ch] = -1;
                        /** Standard: Kanaele deaktiviert */
                        cali->leadChannelDisabled[tdc][ch] = kTRUE;
                }
        }

        /** dito */
        for(Int_t layer=0; layer<TQClamDriftchamCalibrationParameter::MAX_LAYER; layer++)
        {
                for(Int_t wire=0; wire<TQClamDriftchamCalibrationParameter::MAX_WIRE; wire++)
                {
                        /** Standard: Draehte deaktiviert */
                        cali->wireDisabled[layer][wire] = kTRUE;

			cali->p0_spec[layer][wire] = 0.;
			cali->p1_spec[layer][wire] = 0.;
			cali->p2_spec[layer][wire] = 0.;
                }
        }

	cali->useGlobalTimeWalk = kTRUE;
	cali->useWireSpecificTimeWalk = kFALSE;
	cali->tref = 0.0;
	cali->p0 = 0.0;
	cali->p1 = 0.0;
	cali->p2 = 0.0;



	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fTdcMappingConfigFile.Data());
	auto mapping = cfg["mapping"];

	cali->numTDC = mapping.size();
	    
	/** MAX_TDC ueberschritten? */
	if(cali->numTDC > TQClamDriftchamCalibrationParameter::MAX_TDC)
	{
		TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Config defines %d TDCs, but MAX_TDC = %d",
				cali->numTDC, TQClamDriftchamCalibrationParameter::MAX_TDC);
		throw TGo4RuntimeException();
	}

	for(auto tdcEntry : mapping)
	{
		std::string tdcName = tdcEntry.first.as<std::string>();
		/** 'tdc0' -> 0 extrahieren */
		int tdc = std::stoi(tdcName.substr(3));

		if(tdc < 0 || tdc >= TQClamDriftchamCalibrationParameter::MAX_TDC)
		{
			TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] TDC index %d out of range (0..%d)",
					tdc, TQClamDriftchamCalibrationParameter::MAX_TDC-1);
			throw TGo4RuntimeException();
		}


		auto channels = tdcEntry.second;
		/** MAX_CH ueberschritten? */
		if(channels.size() > TQClamDriftchamCalibrationParameter::MAX_CH)
		{
			TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] TDC %d has %zu channels, but MAX_CH = %d",
					tdc, channels.size(), TQClamDriftchamCalibrationParameter::MAX_CH);
			throw TGo4RuntimeException();
		}

		cali->numCh[tdc] = channels.size();
		for(auto chEntry : channels)
		{
			int ch = chEntry.first.as<int>();
			
			auto data = chEntry.second;
			Int_t layer = data["layer"].as<Int_t>();
			Int_t wire = data["wire"].as<Int_t>();
			Bool_t active = data["active"].as<Bool_t>();

			/** MAX_CH ueberschritten? */
			if(ch < 0 || ch >= TQClamDriftchamCalibrationParameter::MAX_CH)
			{
				TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Channel index %d out of range (0..%d)",
						ch, TQClamDriftchamCalibrationParameter::MAX_CH-1);
				throw TGo4RuntimeException();
			}

			/** MAX_LAYER ueberschritten? */
			if(layer < 0 || layer >= TQClamDriftchamCalibrationParameter::MAX_LAYER)
			{
				TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Layer index %d out of range (0..%d)", 
						layer, TQClamDriftchamCalibrationParameter::MAX_LAYER-1);
				throw TGo4RuntimeException();
			}

			/** MAX_WIRE ueberschritten? */
			if(wire < 0 || wire >= TQClamDriftchamCalibrationParameter::MAX_WIRE)
			{
				TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Wire index %d out of range (0..%d)", 
						wire, TQClamDriftchamCalibrationParameter::MAX_WIRE-1);
				throw TGo4RuntimeException();
			}

			/** pruefe, ob Draht bereis aktiviert wurde */
			if(!cali->wireDisabled[layer][wire])
			{
				TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Wire (layer=%d, wire=%d) is assigned twice!",
						layer, wire);
				throw TGo4RuntimeException();
			}


			cali->layerMapLead[tdc][ch] = layer;
			cali->wireMapLead[tdc][ch] = wire;
			cali->leadChannelDisabled[tdc][ch] = !active;
			cali->wireDisabled[layer][wire] = !active;
		}
	}


	/** -- MAPPING der Trailing-Kanaele -- */
	auto* edge = dynamic_cast<TQClamEdgeMappingParameter*>(GetParameter("EdgeMappingParam", "TQClamEdgeMappingParameter"));

	for(Int_t tdc=0; tdc<TQClamDriftchamCalibrationParameter::MAX_TDC; tdc++)
	{
		for(Int_t leadCh=0; leadCh<cali->numCh[tdc]; leadCh++)
		{
			/** nur Leading-Kanaele beruecksichtigen */
			if(edge->IsLeading(tdc, leadCh))
			{
				Int_t trailCh = edge->GetTrailingForLeading(tdc,leadCh);

				/** kein gueltiger Trailing? */
				if(trailCh < 0) { continue; }

				/** Trailing uebernehmen */
				cali->layerMapTrail[tdc][trailCh] = cali->layerMapLead[tdc][leadCh];
				cali->wireMapTrail[tdc][trailCh] = cali->wireMapLead[tdc][leadCh];

				/** Aktivierung/Deaktivierung uebernehmen */
				cali->trailChannelDisabled[tdc][trailCh] = cali->leadChannelDisabled[tdc][leadCh];
			}
		}
	}




	/** -- Driftzeit Umrechnungsparameter einlesen -- */

	/** - auf 0 setzen - */
        cali->globOffset = 0.0;
        cali->invert = kFALSE;

	cali->useGlobOffset = kFALSE;
	cali->useLayerOffset = kFALSE;
	cali->useTdcOffset = kFALSE;
	cali->useTdcChannelOffset = kFALSE;
	cali->useWireOffset = kFALSE;

        for(Int_t tdc=0; tdc<TQClamDriftchamCalibrationParameter::MAX_TDC; tdc++)
        {
                cali->tdcOffset[tdc] = 0.0;

                for(Int_t ch=0; ch<TQClamDriftchamCalibrationParameter::MAX_CH; ch++)
                {
                        cali->tdcChannelOffset[tdc][ch] = 0.0;
                }
        }

        for(Int_t layer=0; layer<TQClamDriftchamCalibrationParameter::MAX_LAYER; layer++)
        {
                cali->layerOffset[layer] = 0.0;

                for(Int_t wire=0; wire<TQClamDriftchamCalibrationParameter::MAX_WIRE; wire++)
                {
                        cali->wireOffset[layer][wire] = 0.0;
                }
        }



	/** config einlesen */
	YAML::Node cfg2 = YAML::LoadFile(master->fDrifttimeConfigFile.Data());
	auto offsets = cfg2["offsets"];



	/** - driftzeit nach offsets invertieren - */
	cali->invert = offsets["invert"].as<Bool_t>();
	printf("invertSignalEdges: %d", cali->invert);

	/** offsets nutzen? */
	cali->useGlobOffset = offsets["use_glob_offset"].as<Bool_t>();
	cali->useLayerOffset = offsets["use_layer_offset"].as<Bool_t>();
	cali->useTdcOffset = offsets["use_tdc_offset"].as<Bool_t>();
	cali->useTdcChannelOffset = offsets["use_tdc_channel_offset"].as<Bool_t>();
	cali->useWireOffsetFiles = offsets["use_wire_offset_files"].as<Bool_t>();
	cali->useWireOffset = offsets["use_wire_offset"].as<Bool_t>();

	auto files = offsets["files"];
	for(int i=0; i<TQClamDriftchamCalibrationParameter::MAX_LAYER; i++)
	{
		//if(files.count(i));
		{
			cali->offsetFiles[i] = ResolvePath(files[i].as<std::string>());
		}
	}


	/** - globaler offset - */
	cali->globOffset = offsets["global"].as<Double_t>();



	/** - layer offsets - */
	auto layers = offsets["layer"];

	/** MAX_LAYER proof 1 */
	if(layers.size() > TQClamDriftchamCalibrationParameter::MAX_LAYER)
	{
		TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Config defines %zu Layers, but MAX_LAYER = %d",
				layers.size(), TQClamDriftchamCalibrationParameter::MAX_LAYER-1);
		throw TGo4RuntimeException();
	}

	for(auto layerEntry : layers)
	{
		int layer = layerEntry.first.as<int>();

		/** MAX_LAYER proof 2*/
		if(layer > TQClamDriftchamCalibrationParameter::MAX_LAYER)
		{
			TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Layer index %d out of range (0..%d)",
					layer, TQClamDriftchamCalibrationParameter::MAX_LAYER-1);
			throw TGo4RuntimeException();
		}

		cali->layerOffset[layer] = layerEntry.second.as<Double_t>();
	}




	/** - tdc offsets - */
	auto tdcs = offsets["tdc"];

	/** MAX_TDC proof 1 */
	if(tdcs.size() > TQClamDriftchamCalibrationParameter::MAX_TDC)
	{
		TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Config defines %zu TDCs, but MAX_TDC = %d",
				tdcs.size(), TQClamDriftchamCalibrationParameter::MAX_TDC-1);
		throw TGo4RuntimeException();
	}
	for(auto tdcEntry : tdcs)
	{
		int tdc = tdcEntry.first.as<int>();
		/** MAX_TDC proof 2 */
		if(tdc > TQClamDriftchamCalibrationParameter::MAX_TDC)
		{
			TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] TDC index %d out of range (0..%d)",
					tdc, TQClamDriftchamCalibrationParameter::MAX_TDC-1);
			throw TGo4RuntimeException();
		}

		cali->tdcOffset[tdc] = tdcEntry.second.as<Double_t>();
	}




	/** - tdc-channel offsets - */
	for(auto tdcEntry : offsets["tdcChannel"])
	{
		/** tdc bereits geprueft */
		int tdc = tdcEntry.first.as<int>();

		auto tdcEntries = tdcEntry.second;
		/** MAX_CH proof 1*/
		if(tdcEntries.size() > TQClamDriftchamCalibrationParameter::MAX_CH)
		{
			TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Config defines %zu Channel, but MAX_CH = %d",
					tdcEntries.size(), TQClamDriftchamCalibrationParameter::MAX_CH-1);
			throw TGo4RuntimeException();
		}
		for(auto chEntry : tdcEntries)
		{
			int ch = chEntry.first.as<int>();

			/** MAX_CH proof 2 */
			if(ch > TQClamDriftchamCalibrationParameter::MAX_CH)
			{
				TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Channel index %d out of range (0..%d)",
						ch, TQClamDriftchamCalibrationParameter::MAX_CH-1);
				throw TGo4RuntimeException();
			}

			cali->tdcChannelOffset[tdc][ch] = chEntry.second.as<Double_t>();
		}
	}




	/** - wire offsets - */


	// -------------------------
	// VERWENDE INPUT-FILES FUER DRAHT-OFFSETS
	// -------------------------
	if(cali->useWireOffsetFiles)
	{
		auto files = offsets["time_offsets"]["files"];

		// -------------------------
		// X1
		// -------------------------
		//if(files.count(0))
		{
			auto x1 = TQClamTimeOffsetLoader::LoadSingleFile(cali->offsetFiles[0].Data(),TQClamDriftchamCalibrationParameter::MAX_WIRE);

			for(int wire=0; wire<TQClamDriftchamCalibrationParameter::MAX_WIRE; wire++)
			{
				cali->wireOffset[0][wire] = x1[wire];
			}
		}

		// -------------------------
		// U (256 wires)
		// -------------------------
		//if(files.count(1))
		{
			constexpr int U_WIRES = 256;

			auto u = TQClamTimeOffsetLoader::LoadSingleFile(cali->offsetFiles[1].Data(), U_WIRES);

			for(int wire=0; wire<U_WIRES; wire++)
			{
				cali->wireOffset[1][wire] = u[wire];
			}
		}

		// -------------------------
		// X2
		// -------------------------
		//if(files.count(2))
		{
			auto x2 = TQClamTimeOffsetLoader::LoadSingleFile(cali->offsetFiles[2].Data(), TQClamDriftchamCalibrationParameter::MAX_WIRE);

			for(int wire=0; wire<TQClamDriftchamCalibrationParameter::MAX_WIRE; wire++)
			{
				cali->wireOffset[2][wire] = x2[wire];
			}
		}
	}

	// -------------------------
	// VERWENDE KEINE INPUT-FILES FUER DRAHT-OFFSETS (default -> alte offsets...)
	// -------------------------
	else
	{
		for(auto layerEntry : offsets["wire"])
		{
			/** layer bereits geprueft */
			int layer = layerEntry.first.as<int>();
	
			auto layerEntries = layerEntry.second;
			/** MAX_WIRE proof 1*/
			if(layerEntries.size() > TQClamDriftchamCalibrationParameter::MAX_WIRE)
			{
				TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Config defines %zu Wire, but MAX_WIRE = %d",
						layerEntries.size(), TQClamDriftchamCalibrationParameter::MAX_WIRE-1);
				throw TGo4RuntimeException();
			}
			for(auto wireEntry : layerEntries)
			{
				int wire = wireEntry.first.as<int>();
	
				/** MAX_WIRE proof 2 */
				if(wire > TQClamDriftchamCalibrationParameter::MAX_WIRE)
				{
					TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Wire index %d out of range (0..%d)",
							wire, TQClamDriftchamCalibrationParameter::MAX_WIRE-1);
					throw TGo4RuntimeException();
				}
	
				cali->wireOffset[layer][wire] = wireEntry.second.as<Double_t>();
			}
		}
	}



	// -------------------------
	// TIME-WALK Korrektur
	// -------------------------
	/** -- Time-Walk Korrekturen -- */
	YAML::Node cfg3 = YAML::LoadFile(master->fDrifttimeTimeWalkCorrFile.Data());
	auto timewalk = cfg3["correction"];
	cali->useWireSpecificTimeWalk = timewalk["use_wire_specific_timewalk"].as<Bool_t>();

        auto files_ = timewalk["files"];
        for(int i=0; i<TQClamDriftchamCalibrationParameter::MAX_LAYER; i++)
        {
                {
                        cali->timewalkFiles[i] = ResolvePath(files_[i].as<std::string>());
                }
        }


	if(cali->useWireSpecificTimeWalk)
	{
		auto files_ = timewalk["files"];

		// -------------------------
		// X1
		// -------------------------
		//if(files.count(0))
		{
			auto x1 = TQClamTimeWalkLoader::LoadSingleFile(cali->timewalkFiles[0].Data(),TQClamDriftchamCalibrationParameter::MAX_WIRE);

			for(int wire=0; wire<TQClamDriftchamCalibrationParameter::MAX_WIRE; wire++)
			{
				cali->p0_spec[0][wire] = x1[wire].p0;
				cali->p1_spec[0][wire] = x1[wire].p1;
				cali->p2_spec[0][wire] = x1[wire].p2;
			}
		}

		// -------------------------
		// U (256 wires)
		// -------------------------
		//if(files.count(1))
		{

			constexpr int U_WIRES = 256;
			auto u = TQClamTimeWalkLoader::LoadSingleFile(cali->timewalkFiles[1].Data(), U_WIRES);

			for(int wire=0; wire<U_WIRES; wire++)
			{
				cali->p0_spec[1][wire] = u[wire].p0;
				cali->p1_spec[1][wire] = u[wire].p1;
				cali->p2_spec[1][wire] = u[wire].p2;
			}
		}


		// -------------------------
		// X2
		// -------------------------
		//if(files.count(2))
		{
			auto x2 = TQClamTimeWalkLoader::LoadSingleFile(cali->timewalkFiles[2].Data(),TQClamDriftchamCalibrationParameter::MAX_WIRE);

			for(int wire=0; wire<TQClamDriftchamCalibrationParameter::MAX_WIRE; wire++)
			{
				cali->p0_spec[2][wire] = x2[wire].p0;
				cali->p1_spec[2][wire] = x2[wire].p1;
				cali->p2_spec[2][wire] = x2[wire].p2;
			}
		}

	}

	// -------------------------
	// VERWENDE KEINE INPUT-FILES FUER DRAHT-OFFSETS (default -> alte offsets...)
	// -------------------------
	else
	{
		for(auto layerEntry : offsets["wire"])
		{
			/** layer bereits geprueft */
			int layer = layerEntry.first.as<int>();
	
			auto layerEntries = layerEntry.second;
			/** MAX_WIRE proof 1*/
			if(layerEntries.size() > TQClamDriftchamCalibrationParameter::MAX_WIRE)
			{
				TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Config defines %zu Wire, but MAX_WIRE = %d",
						layerEntries.size(), TQClamDriftchamCalibrationParameter::MAX_WIRE-1);
				throw TGo4RuntimeException();
			}
			for(auto wireEntry : layerEntries)
			{
				int wire = wireEntry.first.as<int>();
	
				/** MAX_WIRE proof 2 */
				if(wire > TQClamDriftchamCalibrationParameter::MAX_WIRE)
				{
					TGo4Log::Error("[TQClamAnalysis::InitDriftchamCalibrationParameter] Wire index %d out of range (0..%d)",
							wire, TQClamDriftchamCalibrationParameter::MAX_WIRE-1);
					throw TGo4RuntimeException();
				}
	
				cali->wireOffset[layer][wire] = wireEntry.second.as<Double_t>();
			}
		}
	}


	cali->useTimeWalkCorrectedDrifttime = timewalk["use_timewalk_corrected_drifttime"].as<Bool_t>();
	cali->tref = timewalk["tref"].as<Double_t>();
	cali->p0 = timewalk["p0"].as<Double_t>();
	cali->p1 = timewalk["p1"].as<Double_t>();
	cali->p2 = timewalk["p2"].as<Double_t>();





	TGo4Log::Info("[TQClamAnalysis] TQClamDriftchamCalibrationParameter erfolgreich konfiguriert.");
}



//***********************************************************
// Konfiguriere TriggerCalibrationParameter
void TQClamAnalysis::InitTriggerCalibrationParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamTriggerCalibrationParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	auto* sc = dynamic_cast<TQClamTriggerCalibrationParameter*>(GetParameter("TriggerCalibParam", "TQClamTriggerCalibrationParameter"));

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fTriggerMappingConfigFile.Data());
	auto node = cfg["trigger"];


	/** Mapping der Leading Kanaele */
	auto mapping = node["mapping"];

	std::string sPU = "szintPmtUp";
	std::string sPD = "szintPmtDown";
	std::string cP = "cherenkovPmt";
	std::string cT = "coincTrig";
	std::string aT = "accTrig";
	std::string c = "clock";
	std::string l = "livetime";
	std::string mL = "mbsLivetime";
	std::string fCCC = "faradayCupChargeCounter";

	std::string t = "tdc";
	std::string ch = "channel";

	sc->szintPmtUp_ch = mapping[sPU][ch].as<Int_t>();
	sc->szintPmtUp_tdc = mapping[sPU][t].as<Int_t>();

	sc->szintPmtDown_ch = mapping[sPD][ch].as<Int_t>();
	sc->szintPmtDown_tdc = mapping[sPD][t].as<Int_t>();

	sc->cherenkovPmt_ch =  mapping[cP][ch].as<Int_t>();
	sc->cherenkovPmt_tdc = mapping[cP][t].as<Int_t>();

	sc->coincTrig_ch = mapping[cT][ch].as<Int_t>();
	sc->coincTrig_tdc = mapping[cT][t].as<Int_t>();

	sc->accTrig_ch = mapping[aT][ch].as<Int_t>();
	sc->accTrig_tdc = mapping[aT][t].as<Int_t>();

	sc->clock_ch = mapping[c][ch].as<Int_t>();
	sc->clock_tdc = mapping[c][t].as<Int_t>();

	sc->livetime_ch = mapping[l][ch].as<Int_t>();
	sc->livetime_tdc = mapping[l][t].as<Int_t>();

	sc->mbsLivetime_ch = mapping[mL][ch].as<Int_t>();
	sc->mbsLivetime_tdc = mapping[mL][t].as<Int_t>();

	sc->faradayCupChargeCounter_ch = mapping[fCCC][ch].as<int>();
	sc->faradayCupChargeCounter_tdc = mapping[fCCC][t].as<int>();


	/** Bedingungen an Event */
//	auto cond = node["condition"];
	YAML::Node cfg2 = YAML::LoadFile(master->fTriggerConditionConfigFile.Data());
	auto cond = cfg2["condition"];


	sc->needTdcTrigger = cond["needTdcTrigger"].as<Bool_t>();
	sc->invertEdges = cond["invertSignalEdges"].as<Bool_t>();


	std::string nSPU = "needSzintPmtUp";
	std::string nSPD = "needSzintPmtDown";
	std::string nCP = "needCherenkovPmt";
	std::string nCT = "needCoincidenceTrigger";
	std::string nAT = "needAcceptedTrigger";
	std::string nC = "needClockCounter";
	std::string nL = "needLivetime";
	std::string nML = "needMbsLivetime";
	std::string nFCCC = "needFaradayCupChargeCounter";

	sc->needSzintPmtUp = cond[nSPU].as<Bool_t>();
	sc->needSzintPmtDown = cond[nSPD].as<Bool_t>();
	sc->needCherenkovPmt = cond[nCP].as<Bool_t>();
	sc->needCoincTrig = cond[nCT].as<Bool_t>();
	sc->needAccTrig = cond[nAT].as<Bool_t>();
	sc->needClock = cond[nC].as<Bool_t>();
	sc->needLivetime = cond[nL].as<Bool_t>();
	sc->needMbsLivetime = cond[nML].as<Bool_t>();
	sc->needFaradayCupChargeCounter = cond[nFCCC].as<Bool_t>();


	TGo4Log::Info("[TQClamAnalysis] TQClamTriggerCalibrationParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere FaradayCupCurrentCalibrationParameter
void TQClamAnalysis::InitFaradayCupCurrentCalibrationParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamFaradayCupCurrentCalibrationParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	auto* cali = dynamic_cast<TQClamFaradayCupCurrentCalibrationParameter*>(GetParameter("FaradayCupCurrentCalibParam","TQClamFaradayCupCurrentCalibrationParameter"));

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fFaradayCupCurrentCalibrationFile.Data());
	auto cup = cfg["faraday_cup"];

	cali->invert = cup["invert"].as<Bool_t>();
	cali->scalefactorCurrent = cup["scalefactorCurrent"].as<Double_t>();
	cali->offsetCurrent = cup["offsetCurrent"].as<Double_t>();

	TGo4Log::Info("[TQClamAnalysis] TQClamFaradayCupCurrentCalibrationParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Settings von Histogrammen/Graphen/RollingGraps/...
void TQClamAnalysis::InitDisplaySettingParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamDisplaySettingParameter.");
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	auto* display = dynamic_cast<TQClamDisplaySettingParameter*>(GetParameter("DisplaySettingParam","TQClamDisplaySettingParameter"));

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fDisplaySettingConfigFile.Data());
	auto set = cfg["display_settings"];

	display->RollGraphTriggerIntegrationWindow = set["RollingGraph_TriggerIntegrationWindow"].as<Int_t>();


	TGo4Log::Info("[TQClamAnalysis] TQClamDisplaySettingParameter erfolgreich konfiguriert.");
}



//***********************************************************
// Konfiguriere HitConditionParameter
void TQClamAnalysis::InitHitConditionParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamHitConditionParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam", "TQClamMasterParameter"));
	auto* hit = dynamic_cast<TQClamHitConditionParameter*>(GetParameter("HitConditionParam", "TQClamHitConditionParameter"));

	/** alles nullen */
	for(size_t i=0; i<hit->MAX_LAYER; i++)
	{
		hit->minToT_ns[i] = -1e9;
		hit->maxToT_ns[i] = +1e9;

		hit->minDrifttime_ns[i] = -1e9;
		hit->maxDrifttime_ns[i] = 1e9;
	}


	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fHitConditionConfigFile.Data());
	auto cond = cfg["condition"];

	for(const auto entry : cond["minToT_ns"])
	{
		hit->minToT_ns[entry.first.as<Int_t>()] = entry.second.as<double>();
	}

	for(const auto entry : cond["maxToT_ns"])
	{
		hit->maxToT_ns[entry.first.as<Int_t>()] = entry.second.as<double>();
	}

	for(const auto entry : cond["minDrifttime_ns"])
	{
		hit->minDrifttime_ns[entry.first.as<Int_t>()] = entry.second.as<double>();
	}

	for(const auto entry : cond["maxDrifttime_ns"])
	{
		hit->maxDrifttime_ns[entry.first.as<Int_t>()] = entry.second.as<double>();
	}


	if(cond["ToTCondMode_Off"].as<bool>()) { hit->ToTCondMode = TQClamHitConditionParameter::kOff; }
	if(cond["ToTCondMode_FlagOnly"].as<bool>()) { hit->ToTCondMode = TQClamHitConditionParameter::kFlagOnly; }
	if(cond["ToTCondMode_Filter"].as<bool>()) { hit->ToTCondMode = TQClamHitConditionParameter::kFilter; }

	if(cond["DrifttimeCondMode_Off"].as<bool>()) { hit->DrifttimeCondMode = TQClamHitConditionParameter::kOff; }
	if(cond["DrifttimeCondMode_FlagOnly"].as<bool>()) { hit->DrifttimeCondMode = TQClamHitConditionParameter::kFlagOnly; }
	if(cond["DrifttimeCondMode_Filter"].as<bool>()) { hit->DrifttimeCondMode = TQClamHitConditionParameter::kFilter; }

	hit->requireTdcTrig = cond["requireTdcTrig"].as<bool>();
	hit->requireLead = cond["requireLead"].as<bool>();
	hit->requireTrail = cond["requireTrail"].as<bool>();


	TGo4Log::Info("[TQClamAnalysis] TQClamHitConditionParameter erfolgreich konfiguriert.");
}





//***********************************************************
// Konfiguriere HitProcDebugOutputParameter
void TQClamAnalysis::InitHitProcDebugOutputParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamHitProcDebugOutputParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam"));
	auto* debugOut = dynamic_cast<TQClamHitProcDebugOutputParameter*>(GetParameter("HitProcDebugOutputParam"));

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fHitProcDebugOutputConfigFile.Data());
	auto debug = cfg["debug"];

	debugOut->WriteRaw = debug["WriteRaw"].as<Bool_t>();
	debugOut->WriteProcessed = debug["WriteProcessed"].as<Bool_t>();
	debugOut->OutputFileRaw = debug["OutputFileRaw"].as<std::string>();
	debugOut->OutputFileProcessed = debug["OutputFileProcessed"].as<std::string>();


	TGo4Log::Info("[TQClamAnalysis] TQClamHitProcDebugOutputParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere ClusterConditionParameter
void TQClamAnalysis::InitClusterConditionParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamClusterConditionParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam"));
	auto* cluster = dynamic_cast<TQClamClusterConditionParameter*>(GetParameter("ClusterConditionParam"));

	cluster->fCleanCluster = kFALSE;
	cluster->WireSpanCondMode = TQClamClusterConditionParameter::kFlagOnly;
	cluster->HitNumberCondMode = TQClamClusterConditionParameter::kFlagOnly;

	for(int i=0; i<cluster->MAX_LAYER; i++)
	{
		cluster->minWireSpanInCluster[i] = -1;
		cluster->maxWireSpanInCluster[i] = -1;
		cluster->minHitNumberInCluster[i] = -1;
		cluster->maxHitNumberInCluster[i] = -1;
		cluster->requireVShape[i] = false;
		cluster->requireAscending[i] = false;
		cluster->requireDescending[i] = false;
		cluster->requireClusterWithoutGaps[i] = false;

		cluster->fMaxAllowedGapInCleaning[i] = -1;
		cluster->fMaxDrifttimeJump[i] = -1.;
		cluster->fMaxRemovedHits[i] = -1;
		cluster->fMaxCleaningIterations[i] = 0;
	}

	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fClusterConditionConfigFile.Data());
	auto cond = cfg["condition"];


        if(cond["WireSpanInClusterCondMode_Off"].as<bool>()) { cluster->WireSpanCondMode = TQClamClusterConditionParameter::kOff; }
        if(cond["WireSpanInClusterCondMode_FlagOnly"].as<bool>()) { cluster->WireSpanCondMode = TQClamClusterConditionParameter::kFlagOnly; }
        if(cond["WireSpanInClusterCondMode_Filter"].as<bool>()) { cluster->WireSpanCondMode = TQClamClusterConditionParameter::kFilter; }

        if(cond["HitNumberInClusterCondMode_Off"].as<bool>()) { cluster->HitNumberCondMode = TQClamClusterConditionParameter::kOff; }
        if(cond["HitNumberInClusterCondMode_FlagOnly"].as<bool>()) { cluster->HitNumberCondMode = TQClamClusterConditionParameter::kFlagOnly; }
        if(cond["HitNumberInClusterCondMode_Filter"].as<bool>()) { cluster->HitNumberCondMode = TQClamClusterConditionParameter::kFilter; }

	for(auto entry : cond["MinWireSpanInCluster"])
	{
		int layer = entry.first.as<int>();
		int span = entry.second.as<int>();

		cluster->minWireSpanInCluster[layer] = span;
	}

	for(auto entry : cond["MaxWireSpanInCluster"])
	{
		int layer = entry.first.as<int>();
		int span = entry.second.as<int>();

		cluster->maxWireSpanInCluster[layer] = span;
	}

	for(auto entry : cond["MinHitNumberInCluster"])
	{
		int layer = entry.first.as<int>();
		int n = entry.second.as<int>();

		cluster->minHitNumberInCluster[layer] = n;
	}

	for(auto entry : cond["MaxHitNumberInCluster"])
	{
		int layer = entry.first.as<int>();
		int n = entry.second.as<int>();

		cluster->maxHitNumberInCluster[layer] = n;
	}

	for(auto entry : cond["RequireVShape"])
	{
		int layer = entry.first.as<int>();
		bool req = entry.second.as<bool>();

		cluster->requireVShape[layer] = req;
	}

	for(auto entry : cond["RequireAscending"])
	{
		int layer = entry.first.as<int>();
		bool req = entry.second.as<bool>();

		cluster->requireAscending[layer] = req;
	}

	for(auto entry : cond["RequireDescending"])
	{
		int layer = entry.first.as<int>();
		bool req = entry.second.as<bool>();

		cluster->requireDescending[layer] = req;
	}

	for(auto entry : cond["RequireClusterWithoutGaps"])
	{
		int layer = entry.first.as<int>();
		bool req = entry.second.as<bool>();

		cluster->requireClusterWithoutGaps[layer] = req;
	}


	auto cleanClusterCond = cond["CleanCluster"];
	cluster->fCleanCluster = cleanClusterCond["do"].as<Bool_t>();

	for (auto it = cleanClusterCond.begin(); it != cleanClusterCond.end(); ++it)
	{
		std::string key = it->first.as<std::string>();
    
		// skip the 'do' entry
		if (key == "do") { continue; }

		int layer = std::stoi(key); // Key ist "0", "1", "2", etc.

		auto layerCond = it->second;

		cluster->fMaxCleaningIterations[layer] = layerCond["max_cleaning_iterations"].as<int>();
		cluster->fMaxRemovedHits[layer] = layerCond["max_removed_hits"].as<int>();
		cluster->fMaxDrifttimeJump[layer] = layerCond["max_drifttime_jump_neighboring_wires"].as<double>();
		cluster->fMaxAllowedGapInCleaning[layer] = layerCond["max_allowed_gap_size"].as<int>();
	}


	TGo4Log::Info("[TQClamAnalysis] TQClamClusterConditionParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere TrackConditionParameter
void TQClamAnalysis::InitTrackConditionParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamTrackConditionParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam"));
	auto* track = dynamic_cast<TQClamTrackConditionParameter*>(GetParameter("TrackConditionParam"));

	track->EpsilonPhi = 1e-3;

	for(size_t i=0; i<TQClamTrackConditionParameter::MAX_LAYER; i++)
	{
		track->AllowedLayersForTrack[i] = kFALSE;
		track->RequiredLayersForTrack[i] = kFALSE;
	}


	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fTrackConditionConfigFile.Data());
	auto cond = cfg["condition"];



	for(auto entry : cond["allowed_layers_for_track"])
	{
		int layer = entry.first.as<int>();
		bool allowed = entry.second.as<bool>();

		track->AllowedLayersForTrack[layer] = allowed;
	}

	for(auto entry : cond["required_layers_for_track"])
	{
		int layer = entry.first.as<int>();
		bool required = entry.second.as<bool>();

		track->RequiredLayersForTrack[layer] = required;
	}

	track->min_x_X1 = cond["min_x_X1"].as<double>();
	track->max_x_X1 = cond["max_x_X1"].as<double>();

	track->min_phi_deg = cond["min_phi_deg"].as<double>();
	track->max_phi_deg = cond["max_phi_deg"].as<double>();

	track->fMinIter = cond["min_iter"].as<int>();
	track->fMaxIter = cond["max_iter"].as<int>();
	track->RelaxationAlpha = cond["relaxation_alpha"].as<double>();
	track->EpsilonPhi = cond["epsilon_phi"].as<double>();
	track->EpsilonTheta = cond["epsilon_theta"].as<double>();
	track->EpsilonChi2 = cond["epsilon_chi2"].as<double>();
	track->EpsilonRMS = cond["epsilon_rms"].as<double>();

	track->EpsilonPhiStagnation = cond["epsilon_stagnation_phi"].as<double>();

	track->UseOutlierRejection = cond["use_outlier_rejection"].as<Bool_t>();
	track->OutlierRejectionLimit = cond["outlier_rejection_limit"].as<double>();
	track->UpperOutlierRejectionLimit = cond["upper_outlier_rejection_limit"].as<double>();

	track->fMaxRejectedHitsX1PerIter = cond["max_rejected_hits_X1_per_iteration"].as<double>();
	track->fMaxRejectedHitsUPerIter = cond["max_rejected_hits_U_per_iteration"].as<double>();
	track->fMaxRejectedHitsX2PerIter = cond["max_rejected_hits_X2_per_iteration"].as<double>();

	track->ConvergenceCriterion_Phi = cond["convergence_criterion_phi"].as<Bool_t>();
	if(track->ConvergenceCriterion_Phi)
	{
		track->ConvergenceCriterion = EConvergenceCriterion::Phi;
	}
	track->ConvergenceCriterion_RMS = cond["convergence_criterion_rms"].as<Bool_t>();
	if(track->ConvergenceCriterion_RMS)
	{
		track->ConvergenceCriterion = EConvergenceCriterion::RMS;
	}
	track->ConvergenceCriterion_PhiAndRMS = cond["convergence_criterion_phi_and_rms"].as<Bool_t>();
	if(track->ConvergenceCriterion_PhiAndRMS)
	{
		track->ConvergenceCriterion = EConvergenceCriterion::PhiAndRMS;
	}
	track->ConvergenceCriterion_PhiOrRMS = cond["convergence_criterion_phi_or_rms"].as<Bool_t>();
	if(track->ConvergenceCriterion_PhiOrRMS)
	{
		track->ConvergenceCriterion = EConvergenceCriterion::PhiOrRMS;
	}


	/**
	 * pruefe epsilon_phi und epsilon_stagnation
	 */
	if(track->ConvergenceCriterion_Phi || track->ConvergenceCriterion_PhiAndRMS || track->ConvergenceCriterion_PhiOrRMS)
	{
		if(track->EpsilonPhiStagnation >= track->EpsilonPhi)
		{
			TGo4UserException ex(3,"[TQClamTrackConditionParameter] ACHTUNG!!! EpsilonPhi < EpsilonPhiStagnation. Track kann nicht konvergieren! => EpsilonPhiStagnation muss kleiner als EpsilonPhi sein!");

			ex.Handle();
			throw ex;

		}
	}

	TGo4Log::Info("[TQClamAnalysis] TQClamTrackConditionParameter erfolgreich konfiguriert.");
}









//***********************************************************
// Lade DriftToZOffsetParameter (driftzeit,phi) -> zOffset
void TQClamAnalysis::InitDriftToZOffsetParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamDriftToZOffsetParameter.");

	auto* corr = dynamic_cast<TQClamDriftToZOffsetParameter*>(GetParameter("DriftToZOffsetParam"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam"));

	if(!corr)
	{
		TGo4Log::Error("InitDriftToZOffsetParameter: corr == nullptr");
		return;
	}
	if(!master)
	{
		TGo4Log::Error("InitDriftToZOffsetParameter: master == nullptr");
		return;
	}

	corr->Folder = "";
	corr->Type   = -1;

	// alles definierter Zustand
	for(size_t l=0; l<corr->MAX_LAYER; ++l)
	{
		corr->LayerIDs[l] = -1;
		corr->Files[l]    = "";

		for(size_t p=0; p<corr->MAX_PHI_BINS; ++p)
		{
			corr->PhiBins[l][p] = -1.;

			for(size_t d=0; d<corr->MAX_DRIFTTIME_BINS; ++d)
			{
				corr->DrifttimeBins[l][p][d] = -1.;
				corr->ZOffsetTable[l][p][d]  = -1.;
			}
		}
	}

	// YAML laden
	TGo4Log::Info("DriftToZOffset: YAML = %s", master->fDriftToZOffsetTable.Data());
	YAML::Node cfg = YAML::LoadFile(master->fDriftToZOffsetTable.Data());

	auto table = cfg["table"];
	corr->Folder = table["folder"].as<std::string>();
	corr->Type = table["type"].as<Int_t>();
	corr->fInterpolationMode = table["interpolation_mode"].as<Int_t>();

	TGo4Log::Info("DriftToZOffset: folder = %s, type = %d",corr->Folder.Data(), corr->Type);

	// Hilfs-Lambda zum Kopieren mit Bounds-Checks
	auto fill_from_corrdata = [corr](size_t l, const CorrData& data)
	{
		size_t nPhi = std::min(data.phiBins.size(),(size_t)corr->MAX_PHI_BINS);

		corr->LayerIDs[l] = (Int_t)l;

		for(size_t p=0; p<nPhi; ++p)
		{
			corr->PhiBins[l][p] = data.phiBins[p];

			if(p >= data.drifttimeBins.size() || p >= data.zOffsetTable.size()) { break; }

			size_t nDT = std::min(data.drifttimeBins[p].size(),(size_t)corr->MAX_DRIFTTIME_BINS);

			for(size_t d=0; d<nDT; ++d)
			{
				corr->DrifttimeBins[l][p][d] = data.drifttimeBins[p][d];
				corr->ZOffsetTable[l][p][d]  = data.zOffsetTable[p][d];
			}
		}
	};

	// TXT
	if(corr->Type == 0)
	{
		for(auto entry : table["files_txt"])
		{
			int idx = entry.first.as<Int_t>();
			if(idx < 0 || idx >= (Int_t)corr->MAX_LAYER) { continue; }

			corr->Files[idx] = corr->Folder.Data() + entry.second.as<std::string>();
		}

		for(size_t l=0; l<corr->MAX_LAYER; ++l)
		{
			TString file = corr->Files[l];
			if(file.IsNull() || file.Length()==0) { continue; }

			TGo4Log::Info("TXT: Layer %zu, file = %s", l, file.Data());

			CorrData data = TQClamDriftCorrLoader::LoadSingleTXTFile(file.Data());
			fill_from_corrdata(l, data);
		}
	}

	// ROOT
	else if(corr->Type == 1)
	{
		for(auto entry : table["files_sim"])
		{
			int idx = entry.first.as<Int_t>();
			if(idx < 0 || idx >= (Int_t)corr->MAX_LAYER) { continue; }

			corr->Files[idx] = corr->Folder.Data() + entry.second.as<std::string>();
		}

		for(size_t l=0; l<corr->MAX_LAYER; ++l)
		{
			TString file = corr->Files[l];
			if(file.IsNull() || file.Length()==0) { continue; }

			TGo4Log::Info("ROOT: Layer %zu, file = %s", l, file.Data());

			CorrData data = TQClamDriftCorrLoader::LoadSingleROOTFile(file.Data());
			fill_from_corrdata(l, data);
		}
	}
	else
	{
		TGo4Log::Error("[TQClamAnalysis] Ungueltiger DriftToZOffset type = %d", corr->Type);
		TGo4RuntimeException();
	}

	TGo4Log::Info("[TQClamAnalysis] TQClamDriftToZOffsetParameter erfolgreich konfiguriert.");
}







//***********************************************************
// Konfiguriere TQClamFocalPlaneParameter
void TQClamAnalysis::InitFocalPlaneParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamFocalPlaneParameter.");

	auto* focal = dynamic_cast<TQClamFocalPlaneParameter*>(GetParameter("FocalPlaneParam","TQClamFocalPlaneParameter"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));

	/** nullen */
	focal->QuadraticFocalPlane = kFALSE;
	focal->a = 0.;
	focal-> b = 0.;
	focal-> c = 0.;

	YAML::Node cfg = YAML::LoadFile(master->fFocalPlaneConfigFile.Data());

	auto focalplane = cfg["focalplane"];

	focal->QuadraticFocalPlane = focalplane["use_quadratic_focal_plane"].as<Bool_t>();

	if(focal->QuadraticFocalPlane)
	{
		auto quad_pars = focalplane["quadratic_focal_plane_parameter"];
		focal->a = quad_pars["a"].as<Double_t>();
		focal->b = quad_pars["b"].as<Double_t>();
		focal->c = quad_pars["c"].as<Double_t>();
	}

	else
	{
		TGo4Log::Info("[TQClamAnalysis] InitFocalPlaneParameter(): Kein Fokalebenen-Typen ausgewaehlt!");
	}

	TGo4Log::Info("[TQClamAnalysis] TQClamFocalPlaneParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere TQClamSpectrumCalibrationParameter
void TQClamAnalysis::InitSpectrumCalibrationParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamSpectrumCalibrationParameter.");

	auto* spec = dynamic_cast<TQClamSpectrumCalibrationParameter*>(GetParameter("SpecCalibParam","TQClamSpectrumCalibrationParameter"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));

	/** nullen */
	spec->u_org = 0.;

	spec->a_0_0 = 0.;
	spec->a_1_0 = 0.;
	spec->a_2_0 = 0.;
	spec->a_0_1 = 0.;
	spec->a_0_2 = 0.;
	spec->a_1_1 = 0.;
	spec->a_1_2 = 0.;
	spec->a_2_1 = 0.;
	spec->a_2_2 = 0.;

	spec->a_phi_corr = 0.;
	spec->b_phi_corr = 0.;
	spec->c_phi_corr = 0.;

	spec->e0_corr = 0.;
	spec->eS_corr = 0.;
	spec->a_e_corr = 0.;
	spec->b_e_corr = 0.;
	spec->c_e_corr = 0.;


	/** config einlesen */
	YAML::Node cfg = YAML::LoadFile(master->fSpecCalibFile.Data());
	auto calib = cfg["calibration"];

	spec->u_org = calib["u_org"].as<double>();

	spec->a_phi_corr = calib["a_phi_corr"].as<double>();
	spec->b_phi_corr = calib["b_phi_corr"].as<double>();
	spec->c_phi_corr = calib["c_phi_corr"].as<double>();


	spec->a_0_0 = calib["a_0_0"].as<double>();

	spec->a_1_0 = calib["a_1_0"].as<double>();
	spec->a_0_1 = calib["a_0_1"].as<double>();
	spec->a_1_1 = calib["a_1_1"].as<double>();

	spec->a_2_0 = calib["a_2_0"].as<double>();
	spec->a_2_1 = calib["a_2_1"].as<double>();
	spec->a_0_2 = calib["a_0_2"].as<double>();
	spec->a_1_2 = calib["a_1_2"].as<double>();
	spec->a_2_2 = calib["a_2_2"].as<double>();


	spec->e0_corr = calib["e0_corr"].as<double>();
	spec->eS_corr = calib["eS_corr"].as<double>();
	spec->a_e_corr = calib["a_e_corr"].as<double>();
	spec->b_e_corr = calib["b_e_corr"].as<double>();
	spec->c_e_corr = calib["c_e_corr"].as<double>();


	TGo4Log::Info("[TQClamAnalysis] TQClamSpectrumCalibrationParameter erfolgreich konfiguriert.");
}







//***********************************************************
// Konfiguriere FitParameter fuer Fit an Energiespektrum in TQClamTrackProc
void TQClamAnalysis::InitFitParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamFitParameter].");
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));
	auto* fit = dynamic_cast<TQClamFitParameter*>(GetParameter("FitParam","TQClamFitParameter"));

	/** nullen */

	/** 1. Fit-Parameter nur nullen, nicht einlesen */
	fit->fGauss_Amp = 0.;
	fit->fGauss_Mean = 0.;
	fit->fGauss_Sigma = 0.;

	fit->fEMG_Amp = 0.;
	fit->fEMG_Lambda = 0.;
	fit->fEMG_Mean = 0.;
	fit->fEMG_Sigma = 0.;

	fit->fTail_Amp = 0.;
	fit->fTail_Mean = 0.;
	fit->fTail_Eps = 0.;

	fit->fGaussTail_GAmp = 0.;
	fit->fGaussTail_GMean = 0.;
	fit->fGaussTail_GSigma = 0.;
	fit->fGaussTail_TAmp = 0.;
	fit->fGaussTail_TMean = 0.;
	fit->fGaussTail_TEps = 0.;

	/** 2. startwerte fuer fit */
	fit->fStartMean = 50.;
	fit->fStartSigma = 0.5;
	fit->fStartAmp = 100.;
	fit->fStartEps = 0.5;
	fit->fStartLambda = 0.1;

	/** 3. name des spektrums, woran gefittet werden soll */
	fit->fSpectrumName = "ProjSpectrum";

	/** 4. fit ja/nein? interval? welcher fit */
	fit->fDoFit = kFALSE;
	fit->fFitUpdateEventInterval = kFALSE;
	fit->fUseGaussLineFitter = kFALSE;
	fit->fUseSimpleLineFitter = kFALSE;
	fit->fUseEMGLineFitter = kFALSE;
	fit->fUseGaussTailLineFitter = kFALSE;


	/** jetzt setzen */
	YAML::Node cfg = YAML::LoadFile(master->fFitConfigFile.Data());
	
	auto calibration = cfg["fit"];

	fit->fDoFit = calibration["do_fit"].as<Bool_t>();
	fit->fFitUpdateEventInterval = calibration["fit_interval_ntracks"].as<Int_t>();
	fit->fUseGaussLineFitter = calibration["use_gauss_fit"].as<Bool_t>();
	fit->fUseSimpleLineFitter = calibration["use_tail_fit"].as<Bool_t>();
	fit->fUseEMGLineFitter = calibration["use_emg_fit"].as<Bool_t>();
	fit->fUseGaussTailLineFitter = calibration["use_gauss_tail_fit"].as<Bool_t>();



	fit->fStartMean = calibration["start_mean"].as<Double_t>();
	fit->fStartSigma = calibration["start_sigma"].as<Double_t>();
	fit->fStartAmp = calibration["start_amp"].as<Double_t>();
	fit->fStartEps = calibration["start_eps"].as<Double_t>();
	fit->fStartLambda = calibration["start_lambda"].as<Double_t>();

	fit->fSpectrumName = calibration["spectrum_name"].as<std::string>();

	TGo4Log::Info("[TQClamAnalysis] TQClamFitParameter erfolgreich konfiguriert.");
}


	


//***********************************************************
// Konfiguriere DrifttimeCalibParameter
void TQClamAnalysis::InitDrifttimeCalibParameter()
{
	TGo4Log::Info("[TQClamAnalysis] Konfiguriere TQClamDrifttimeCalibParameter.");

	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));
	auto* cali = dynamic_cast<TQClamDrifttimeCalibParameter*>(GetParameter("DrifttimeCalibParam","TQClamDrifttimeCalibParameter"));

	/** nullen */

	cali->fUpdateEventInterval = 100000;
	cali->fDoFitAny = kFALSE;
	cali->fDoPlotFit = kFALSE;

	for(size_t l=0; l<cali->MAX_LAYER; l++)
	{
		for(size_t w=0; w<cali->MAX_WIRE; w++)
		{
			cali->fT0[l][w] = 0.;
			cali->fTau[l][w] = 0.;
			cali->fT[l][w] = 0.;
			cali->fA[l][w] = 0.;
		}
	}

	cali->fStartAmp = 100.;
        cali->fStartT0 = 50.;
        cali->fStartTau = 2.;
        cali->fStartT = 2.;

        cali->fMinAmp = 0.;
        cali->fMaxAmp = 1e9;
        cali->fMinT0 = -200.;
        cali->fMaxT0 = 1000.;
        cali->fMinTau = -50.;
        cali->fMaxTau = 100.;
        cali->fMinT = 0.;
        cali->fMaxT = 50.;


        for(size_t l=0; l<cali->MAX_LAYER; l++)
        {
                cali->fNumWires[l] = 0;
                cali->fDoFit[l] = kFALSE;

	        for(size_t w=0; w<cali->MAX_WIRE; w++)
	        {
                	cali->fT0[l][w] = 0.;
                	cali->fTau[l][w] = 0.;
                	cali->fT[l][w] = 0.;
                	cali->fA[l][w] = 0.;
        	}
        }


	/** config lesen */
	YAML::Node cfg = YAML::LoadFile(master->fDrifttimeCalibConfigFile.Data());
	auto calib = cfg["calibration"];

	cali->fUpdateEventInterval = calib["update_event_interval"].as<Int_t>();

	cali->fDoPlotFit = calib["do_plot_fit"].as<Bool_t>();

	cali->fUseFermiDirac = calib["use_fermi_dirac_fit"].as<Bool_t>();
	cali->fUseSkewed = calib["use_skewed_fit"].as<Bool_t>();

	cali->fStartAmp = calib["start_amp"].as<Double_t>();
	cali->fStartT0 = calib["start_t0"].as<Double_t>();
	cali->fStartTau = calib["start_tau"].as<Double_t>();
	cali->fStartT = calib["start_T"].as<Double_t>();

	cali->fMinAmp = calib["min_amp"].as<Double_t>();
	cali->fMaxAmp = calib["max_amp"].as<Double_t>();
	cali->fMinT0 = calib["min_t0"].as<Double_t>();
	cali->fMaxT0 = calib["max_t0"].as<Double_t>();
	cali->fMinTau = calib["min_tau"].as<Double_t>();
	cali->fMaxTau = calib["max_tau"].as<Double_t>();
	cali->fMinT = calib["min_T"].as<Double_t>();
	cali->fMaxT = calib["max_T"].as<Double_t>();


	cali->fStartA0 = calib["start_amp0"].as<Double_t>();
	cali->fStartMu0 = calib["start_mu0"].as<Double_t>();
	cali->fStartSigma0 = calib["start_sigma0"].as<Double_t>();
	cali->fStartS0 = calib["start_s0"].as<Double_t>();
	
	cali->fStartA1 = calib["start_amp1"].as<Double_t>();
	cali->fStartMu1 = calib["start_mu1"].as<Double_t>();
	cali->fStartSigma1 = calib["start_sigma1"].as<Double_t>();
	cali->fStartS1 = calib["start_s1"].as<Double_t>();

	cali->fMinA0 = calib["min_amp0"].as<Double_t>();
	cali->fMaxA0 = calib["max_amp0"].as<Double_t>();
	cali->fMinMu0 = calib["min_mu0"].as<Double_t>();
	cali->fMaxMu0 = calib["max_mu0"].as<Double_t>();
	cali->fMinSigma0 = calib["min_sigma0"].as<Double_t>();
	cali->fMaxSigma0 = calib["max_sigma0"].as<Double_t>();
	cali->fMinS0 = calib["min_s0"].as<Double_t>();
	cali->fMaxS0 = calib["max_s0"].as<Double_t>();

	cali->fMinA1 = calib["min_amp1"].as<Double_t>();
	cali->fMaxA1 = calib["max_amp1"].as<Double_t>();
	cali->fMinMu1 = calib["min_mu1"].as<Double_t>();
	cali->fMaxMu1 = calib["max_mu1"].as<Double_t>();
	cali->fMinSigma1 = calib["min_sigma1"].as<Double_t>();
	cali->fMaxSigma1 = calib["max_sigma1"].as<Double_t>();
	cali->fMinS1 = calib["min_s1"].as<Double_t>();
	cali->fMaxS1 = calib["max_s1"].as<Double_t>();



	for(auto entry : calib["num_wires"])
	{
		cali->fNumWires[entry.first.as<Int_t>()] = entry.second.as<Int_t>();
	}

	for(auto entry : calib["do_calibration"])
	{
		if(entry.first.as<std::string>() == "any")
		{
			cali->fDoFitAny = entry.second.as<Bool_t>();
			continue;
		}
		cali->fDoFit[entry.first.as<Int_t>()] = entry.second.as<Bool_t>();
		TGo4Log::Info("DoFit: layer: %d, bool: %d",entry.first.as<Int_t>(),entry.second.as<Bool_t>());
	}


	TGo4Log::Info("[TQClamAnalysis] TQClamDrifttimeCalibParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere UnpackParameter
void TQClamAnalysis::InitUnpackParameter()
{
	auto* unpack = dynamic_cast<TQClamUnpackParameter*>(GetParameter("UnpackParam","TQClamUnpackParameter"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));

	unpack->fFolderTdcCalibration = "calibration_files/tdcCalibration";
	unpack->fTimeInterpolationElbVfb6_Random = kTRUE;
	unpack->fTimeInterpolationElbVfb6_Linear = kFALSE;
	unpack->fTimeInterpolationElbVfb6_None = kFALSE;

	/** config lesen */
	YAML::Node cfg = YAML::LoadFile(master->fUnpackConfigFile.Data());
	auto unpack_conf = cfg["unpacker"];

	unpack->fFolderTdcCalibration = unpack_conf["folder_tdc_calibration_files"].as<std::string>();

	unpack->fTimeInterpolationElbVfb6_Random = unpack_conf["elb_vfb6_time_interpolation_random"].as<Bool_t>();
	unpack->fTimeInterpolationElbVfb6_Linear = unpack_conf["elb_vfb6_time_interpolation_linear"].as<Bool_t>();
	unpack->fTimeInterpolationElbVfb6_None = unpack_conf["elb_vfb6_time_interpolation_none"].as<Bool_t>();

	TGo4Log::Info("[TQClamAnalysis] TQClamUnpackParameter erfolgreich konfiguriert.");
}





//***********************************************************
// Konfiguriere StoreParameter
void TQClamAnalysis::InitStoreParameter()
{
	auto* store = dynamic_cast<TQClamStoreParameter*>(GetParameter("StoreParam","TQClamStoreParameter"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));

	store->fBPauseUserDrifttimeCalibStore = kFALSE;
	store->fBPauseUserTrackStore = kFALSE;

	YAML::Node cfg = YAML::LoadFile(master->fStoreConfigFile.Data());
	auto store_cfg = cfg["store"];

	store->fBPauseUserDrifttimeCalibStore = store_cfg["pause_user_drifttime_calib_store"].as<Bool_t>();
	store->fBPauseUserTrackStore = store_cfg["pause_user_track_store"].as<Bool_t>();

	TGo4Log::Info("[TQClamAnalysis] TQClamStoreParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere HistogramParameter
void TQClamAnalysis::InitHistogramParameter()
{
	auto* histo = dynamic_cast<TQClamHistogramParameter*>(GetParameter("HistoParam","TQClamHistogramParameter"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));

	/** default setzen */
	// HitProc
	histo->fBFillHistos = kTRUE;


	histo->fBFillHitProc_SzintUpTime = kTRUE;
	histo->fBFillHitProc_SzintDownTime = kTRUE;
	histo->fBFillHitProc_CherenkovTime = kTRUE;
	histo->fBFillHitProc_MeanSzintTime = kTRUE;
	histo->fBFillHitProc_DiffSzintTime = kTRUE;
	histo->fBFillHitProc_SzintUpVsSzintDownTime = kTRUE;

	histo->fBFillHitProc_HitsInLayer = kTRUE;
	histo->fBFillHitProc_HitsInLayerCut = kTRUE;

	histo->fBFillHitProc_HitRateEvtInLayer = kTRUE;
	histo->fBFillHitProc_HitRateEvtInLayerCut = kTRUE;

	histo->fBFillHitProc_HitRateTimeInLayer = kTRUE;
	histo->fBFillHitProc_HitRateTimeInLayerCut = kTRUE;

	histo->fBFillHitProc_HitsPerWire = kTRUE;
	histo->fBFillHitProc_HitsPerWireCut = kTRUE;

	histo->fBFillHitProc_HitRateEvtPerWire = kTRUE;
	histo->fBFillHitProc_HitRateEvtPerWireCut = kTRUE;
	histo->fBFillHitProc_HitRateTimePerWire = kTRUE;
	histo->fBFillHitProc_HitRateTimePerWireCut = kTRUE;

	histo->fBFillHitProc_DrifttimeDist = kTRUE;
	histo->fBFillHitProc_DrifttimeDistCut = kTRUE; 
	histo->fBFillHitProc_DrifttimeDistTimeWalk = kTRUE;

	histo->fBFillHitProc_ToTDist = kTRUE;
	histo->fBFillHitProc_ToTDistCut = kTRUE;
	histo->fBFillHitProc_ToTDistTimeWalk = kTRUE;

	histo->fBFillHitProc_DrifttimeVsWire = kTRUE;
	histo->fBFillHitProc_DrifttimeVsWireCut = kTRUE;
	histo->fBFillHitProc_DrifttimeVsWireTimeWalk = kTRUE;

	histo->fBFillHitProc_ToTVsWire = kTRUE;
	histo->fBFillHitProc_ToTVsWireCut = kTRUE; 
	histo->fBFillHitProc_ToTVsWireTimeWalk = kTRUE;

	histo->fBFillHitProc_DrifttimeVsToT = kTRUE;
	histo->fBFillHitProc_DrifttimeVsToTCut = kTRUE;
	histo->fBFillHitProc_DrifttimeVsToTTimeWalk = kTRUE;

	histo->fBFillHitProc_FaradayCupCurrent = kTRUE;
	histo->fBFillHitProc_ScalerTriggerRate = kTRUE;
	histo->fBFillHitProc_SzintUpTrigRate = kTRUE;
	histo->fBFillHitProc_SzintDownTrigRate = kTRUE;
	histo->fBFillHitProc_CherenkovTrigRate = kTRUE;
	histo->fBFillHitProc_CoincTrigRate = kTRUE;
	histo->fBFillHitProc_AccTrigRate = kTRUE;
	histo->fBFillHitProc_RelDeadtime = kTRUE;
	histo->fBFillHitProc_RelMbsDeadtime = kTRUE;


	// ClusterProc
	if(batch_no_histos)
	{
		histo->fBFillHistos = kFALSE;
	}
	else
	{
		histo->fBFillHistos = kTRUE; // evtl in config TODO
	}

	histo->fBFillClusterProc_HitNumberInCluster = kTRUE;
	histo->fBFillClusterProc_WireSpanInCluster = kTRUE;
	histo->fBFillClusterProc_HitNumberVsWireSpan = kTRUE;

	histo->fBFillClusterProc_XMidCluster = kTRUE;
	histo->fBFillClusterProc_XMidClusterCut = kTRUE;
	histo->fBFillClusterProc_WireNumberWithMinDrifttimeInCluster = kFALSE;
	histo->fBFillClusterProc_WireNumberWithMinDrifttimeInClusterCut = kFALSE;

	histo->fBFillClusterProc_MinDrifttimeInCluster = kTRUE;
	histo->fBFillClusterProc_MinDrifttimeInClusterCut = kTRUE;

	histo->fBFillClusterProc_ClusterPropertiesBeforeCut = kTRUE;
	histo->fBFillClusterProc_ClusterPropertiesAfterCut = kTRUE;
	histo->fBFillClusterProc_ClusterCoincidences = kTRUE;
	histo->fBFillClusterProc_ClusterCoincidencesRel = kTRUE;
	histo->fBFillClusterProc_ClusterEfficiency = kTRUE;

	histo->fBFillClusterProc_ClusterRemovedHits = kTRUE;
	histo->fBFillClusterProc_ClusterSizeBeforeClean = kTRUE;
	histo->fBFillClusterProc_ClusterSizeAfterClean = kTRUE;
	histo->fBFillClusterProc_ClusterSizeBeforeVsAfterClean = kTRUE;
	histo->fBFillClusterProc_ClusterShapeChange = kTRUE;
	histo->fBFillClusterProc_DrifttimeJump = kTRUE;
	histo->fBFillClusterProc_DrifttimeResidualAll = kTRUE;
	histo->fBFillClusterProc_DrifttimeResidualRemoved = kTRUE;


	// TrackProc
	histo->fBFillTrackProc_XSpectrumX1Plane = kTRUE;
	histo->fBFillTrackProc_YSpectrumX1Plane = kTRUE;
	histo->fBFillTrackProc_XSpectrumUPlane = kTRUE;
	histo->fBFillTrackProc_YSpectrumUPlane = kTRUE;
	histo->fBFillTrackProc_XSpectrumX2Plane = kTRUE;
	histo->fBFillTrackProc_USpectrum = kTRUE;

	histo->fBFillTrackProc_ProjSpectrum = kTRUE;
	histo->fBFillTrackProc_XCorrSpectrum = kTRUE;

	histo->fBFillTrackProc_PhiSpectrum = kTRUE;
	histo->fBFillTrackProc_ThetaSpectrum = kTRUE; 

	histo->fBFillTrackProc_PhiVsX1 = kTRUE;
	histo->fBFillTrackProc_PhiVsX2 = kTRUE;
	histo->fBFillTrackProc_PhiVsXCorr = kTRUE;

	histo->fBFillTrackProc_ThetaVsY = kTRUE;

	histo->fBFillTrackProc_Chi2 = kTRUE;
	histo->fBFillTrackProc_Chi2NDF = kTRUE;

	histo->fBFillTrackProc_MaxResidualSigned = kTRUE;
	histo->fBFillTrackProc_MaxResidualAbs = kTRUE;
	histo->fBFillTrackProc_MeanResidual = kTRUE;

	histo->fBFillTrackProc_Converged = kTRUE;
	histo->fBFillTrackProc_Iterations = kTRUE;
	histo->fBFillTrackProc_Rejected = kTRUE;

	histo->fBFillTrackProc_Pull = kTRUE;
	histo->fBFillTrackProc_ResidualVsWire = kTRUE;

	histo->fBFillTrackProc_MeanResidualX1 = kTRUE;
	histo->fBFillTrackProc_MeanResidualU = kTRUE;
	histo->fBFillTrackProc_MeanResidualX2 = kTRUE;

	histo->fBFillTrackProc_RmsResidualX1 = kTRUE;
	histo->fBFillTrackProc_RmsResidualU = kTRUE;
	histo->fBFillTrackProc_RmsResidualX2 = kTRUE;

	histo->fBFillTrackProc_ResidualX1 = kTRUE;
	histo->fBFillTrackProc_ResidualU = kTRUE;
	histo->fBFillTrackProc_ResidualX2 = kTRUE;

	// TODO: DagoHitProc, CoincProc


	YAML::Node cfg = YAML::LoadFile(master->fHistoFillingConfigFile.Data());
	auto histo_cfg = cfg["histogram_filling"];

	// HitProc
	histo->fBFillHitProc_SzintUpTime = histo_cfg["fill_SzintUpTime"].as<Bool_t>();
	histo->fBFillHitProc_SzintDownTime = histo_cfg["fill_SzintDownTime"].as<Bool_t>();
	histo->fBFillHitProc_CherenkovTime = histo_cfg["fill_CherenkovTime"].as<Bool_t>();
	histo->fBFillHitProc_MeanSzintTime = histo_cfg["fill_MeanSzintTime"].as<Bool_t>();
	histo->fBFillHitProc_DiffSzintTime = histo_cfg["fill_DiffSzintTime"].as<Bool_t>();
	histo->fBFillHitProc_SzintUpVsSzintDownTime = histo_cfg["fill_SzintUpVsSzintDownTime"].as<Bool_t>();

	histo->fBFillHitProc_HitsInLayer = histo_cfg["fill_HitsInLayer"].as<Bool_t>();
	histo->fBFillHitProc_HitsInLayerCut = histo_cfg["fill_HitsInLayerCut"].as<Bool_t>();

	histo->fBFillHitProc_HitRateEvtInLayer = histo_cfg["fill_HitRateEvtInLayer"].as<Bool_t>();
	histo->fBFillHitProc_HitRateEvtInLayerCut = histo_cfg["fill_HitRateEvtInLayerCut"].as<Bool_t>();

	histo->fBFillHitProc_HitRateTimeInLayer = histo_cfg["fill_HitRateTimeInLayer"].as<Bool_t>();
	histo->fBFillHitProc_HitRateTimeInLayerCut = histo_cfg["fill_HitRateTimeInLayerCut"].as<Bool_t>();

	histo->fBFillHitProc_HitsPerWire = histo_cfg["fill_HitsPerWire"].as<Bool_t>();
	histo->fBFillHitProc_HitsPerWireCut = histo_cfg["fill_HitsPerWireCut"].as<Bool_t>();

	histo->fBFillHitProc_HitRateEvtPerWire = histo_cfg["fill_HitRateEvtPerWire"].as<Bool_t>();
	histo->fBFillHitProc_HitRateEvtPerWireCut = histo_cfg["fill_HitRateEvtPerWireCut"].as<Bool_t>();
	histo->fBFillHitProc_HitRateTimePerWire = histo_cfg["fill_HitRateTimePerWire"].as<Bool_t>();
	histo->fBFillHitProc_HitRateTimePerWireCut = histo_cfg["fill_HitRateTimePerWireCut"].as<Bool_t>();

	histo->fBFillHitProc_DrifttimeDist = histo_cfg["fill_DrifttimeDist"].as<Bool_t>();
	histo->fBFillHitProc_DrifttimeDistCut = histo_cfg["fill_DrifttimeDistCut"].as<Bool_t>(); 
	histo->fBFillHitProc_DrifttimeDistTimeWalk = histo_cfg["fill_DrifttimeDistTimeWalk"].as<Bool_t>();

	histo->fBFillHitProc_ToTDist = histo_cfg["fill_ToTDist"].as<Bool_t>();
	histo->fBFillHitProc_ToTDistCut = histo_cfg["fill_ToTDistCut"].as<Bool_t>();
	histo->fBFillHitProc_ToTDistTimeWalk = histo_cfg["fill_ToTDistTimeWalk"].as<Bool_t>();

	histo->fBFillHitProc_DrifttimeVsWire = histo_cfg["fill_DrifttimeVsWire"].as<Bool_t>();
	histo->fBFillHitProc_DrifttimeVsWireCut = histo_cfg["fill_DrifttimeVsWireCut"].as<Bool_t>();
	histo->fBFillHitProc_DrifttimeVsWireTimeWalk = histo_cfg["fill_DrifttimeVsWireTimeWalk"].as<Bool_t>();

	histo->fBFillHitProc_ToTVsWire = histo_cfg["fill_ToTVsWire"].as<Bool_t>();
	histo->fBFillHitProc_ToTVsWireCut = histo_cfg["fill_ToTVsWireCut"].as<Bool_t>(); 
	histo->fBFillHitProc_ToTVsWireTimeWalk = histo_cfg["fill_ToTVsWireTimeWalk"].as<Bool_t>();

	histo->fBFillHitProc_DrifttimeVsToT = histo_cfg["fill_DrifttimeVsToT"].as<Bool_t>();
	histo->fBFillHitProc_DrifttimeVsToTCut = histo_cfg["fill_DrifttimeVsToTCut"].as<Bool_t>();
	histo->fBFillHitProc_DrifttimeVsToTTimeWalk = histo_cfg["fill_DrifttimeVsToTTimeWalk"].as<Bool_t>();

	histo->fBFillHitProc_FaradayCupCurrent = histo_cfg["fill_FaradayCupCurrent"].as<Bool_t>();
	histo->fBFillHitProc_ScalerTriggerRate = histo_cfg["fill_ScalerTriggerRate"].as<Bool_t>();
	histo->fBFillHitProc_SzintUpTrigRate = histo_cfg["fill_SzintUpTrigRate"].as<Bool_t>();
	histo->fBFillHitProc_SzintDownTrigRate = histo_cfg["fill_SzintDownTrigRate"].as<Bool_t>();
	histo->fBFillHitProc_CherenkovTrigRate = histo_cfg["fill_CherenkovTrigRate"].as<Bool_t>();
	histo->fBFillHitProc_CoincTrigRate = histo_cfg["fill_CoincTrigRate"].as<Bool_t>();
	histo->fBFillHitProc_AccTrigRate = histo_cfg["fill_AccTrigRate"].as<Bool_t>();
	histo->fBFillHitProc_RelDeadtime = histo_cfg["fill_RelDeadtime"].as<Bool_t>();
	histo->fBFillHitProc_RelMbsDeadtime = histo_cfg["fill_RelMbsDeadtime"].as<Bool_t>();


	// ClusterProc
	histo->fBFillClusterProc_HitNumberInCluster = histo_cfg["fill_HitNumberInCluster"].as<Bool_t>();
	histo->fBFillClusterProc_WireSpanInCluster = histo_cfg["fill_WireSpanInCluster"].as<Bool_t>();
	histo->fBFillClusterProc_HitNumberVsWireSpan = histo_cfg["fill_HitNumberVsWireSpan"].as<Bool_t>();

	histo->fBFillClusterProc_XMidCluster = histo_cfg["fill_XMidCluster"].as<Bool_t>();
	histo->fBFillClusterProc_XMidClusterCut = histo_cfg["fill_XMidClusterCut"].as<Bool_t>();
	histo->fBFillClusterProc_WireNumberWithMinDrifttimeInCluster = histo_cfg["fill_WireNumberWithMinDrifttimeInCluster"].as<Bool_t>();
	histo->fBFillClusterProc_WireNumberWithMinDrifttimeInClusterCut = histo_cfg["fill_WireNumberWithMinDrifttimeInClusterCut"].as<Bool_t>();

	histo->fBFillClusterProc_MinDrifttimeInCluster = histo_cfg["fill_MinDrifttimeInCluster"].as<Bool_t>();
	histo->fBFillClusterProc_MinDrifttimeInClusterCut = histo_cfg["fill_MinDrifttimeInClusterCut"].as<Bool_t>();

	histo->fBFillClusterProc_ClusterPropertiesBeforeCut = histo_cfg["fill_ClusterPropertiesBeforeCut"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterPropertiesAfterCut = histo_cfg["fill_ClusterPropertiesAfterCut"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterCoincidences = histo_cfg["fill_ClusterCoincidences"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterCoincidencesRel = histo_cfg["fill_ClusterCoincidencesRel"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterEfficiency = histo_cfg["fill_ClusterEfficiency"].as<Bool_t>();

	histo->fBFillClusterProc_ClusterRemovedHits = histo_cfg["fill_ClusterRemovedHits"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterSizeBeforeClean = histo_cfg["fill_ClusterSizeBeforeClean"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterSizeAfterClean = histo_cfg["fill_ClusterSizeAfterClean"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterSizeBeforeVsAfterClean = histo_cfg["fill_ClusterSizeBeforeVsAfterClean"].as<Bool_t>();
	histo->fBFillClusterProc_ClusterShapeChange = histo_cfg["fill_ClusterShapeChange"].as<Bool_t>();
	histo->fBFillClusterProc_DrifttimeJump = histo_cfg["fill_DrifttimeJump"].as<Bool_t>();
	histo->fBFillClusterProc_DrifttimeResidualAll = histo_cfg["fill_DrifttimeResidualAll"].as<Bool_t>();
	histo->fBFillClusterProc_DrifttimeResidualRemoved = histo_cfg["fill_DrifttimeResidualRemoved"].as<Bool_t>();


	// TrackProc
	histo->fBFillTrackProc_XSpectrumX1Plane = histo_cfg["fill_XSpectrumX1Plane"].as<Bool_t>();
	histo->fBFillTrackProc_YSpectrumX1Plane = histo_cfg["fill_YSpectrumX1Plane"].as<Bool_t>();
	histo->fBFillTrackProc_XSpectrumUPlane = histo_cfg["fill_XSpectrumUPlane"].as<Bool_t>();
	histo->fBFillTrackProc_YSpectrumUPlane = histo_cfg["fill_YSpectrumUPlane"].as<Bool_t>();
	histo->fBFillTrackProc_XSpectrumX2Plane = histo_cfg["fill_XSpectrumX2Plane"].as<Bool_t>();
	histo->fBFillTrackProc_USpectrum = histo_cfg["fill_USpectrum"].as<Bool_t>();

	histo->fBFillTrackProc_ProjSpectrum = histo_cfg["fill_ProjSpectrum"].as<Bool_t>();
	histo->fBFillTrackProc_XCorrSpectrum = histo_cfg["fill_XCorrSpectrum"].as<Bool_t>();

	histo->fBFillTrackProc_PhiSpectrum = histo_cfg["fill_PhiSpectrum"].as<Bool_t>();
	histo->fBFillTrackProc_ThetaSpectrum = histo_cfg["fill_ThetaSpectrum"].as<Bool_t>(); 

	histo->fBFillTrackProc_PhiVsX1 = histo_cfg["fill_PhiVsX1"].as<Bool_t>();
	histo->fBFillTrackProc_PhiVsX2 = histo_cfg["fill_PhiVsX2"].as<Bool_t>();
	histo->fBFillTrackProc_PhiVsXCorr = histo_cfg["fill_PhiVsXCorr"].as<Bool_t>();

	histo->fBFillTrackProc_ThetaVsY = histo_cfg["fill_ThetaVsY"].as<Bool_t>();

	histo->fBFillTrackProc_Chi2 = histo_cfg["fill_Chi2"].as<Bool_t>();
	histo->fBFillTrackProc_Chi2NDF = histo_cfg["fill_Chi2NDF"].as<Bool_t>();

	histo->fBFillTrackProc_MaxResidualSigned = histo_cfg["fill_MaxResidualSigned"].as<Bool_t>();
	histo->fBFillTrackProc_MaxResidualAbs = histo_cfg["fill_MaxResidualAbs"].as<Bool_t>();
	histo->fBFillTrackProc_MeanResidual = histo_cfg["fill_MeanResidual"].as<Bool_t>();

	histo->fBFillTrackProc_Converged = histo_cfg["fill_Converged"].as<Bool_t>();
	histo->fBFillTrackProc_Iterations = histo_cfg["fill_Iterations"].as<Bool_t>();
	histo->fBFillTrackProc_Rejected = histo_cfg["fill_Rejected"].as<Bool_t>();

	histo->fBFillTrackProc_Pull = histo_cfg["fill_Pull"].as<Bool_t>();
	histo->fBFillTrackProc_ResidualVsWire = histo_cfg["fill_ResidualVsWire"].as<Bool_t>();

	histo->fBFillTrackProc_MeanResidualX1 = histo_cfg["fill_MeanResidualX1"].as<Bool_t>();
	histo->fBFillTrackProc_MeanResidualU = histo_cfg["fill_MeanResidualU"].as<Bool_t>();
	histo->fBFillTrackProc_MeanResidualX2 = histo_cfg["fill_MeanResidualX2"].as<Bool_t>();

	histo->fBFillTrackProc_RmsResidualX1 = histo_cfg["fill_RmsResidualX1"].as<Bool_t>();
	histo->fBFillTrackProc_RmsResidualU = histo_cfg["fill_RmsResidualU"].as<Bool_t>();
	histo->fBFillTrackProc_RmsResidualX2 = histo_cfg["fill_RmsResidualX2"].as<Bool_t>();

	histo->fBFillTrackProc_ResidualX1 = histo_cfg["fill_ResidualX1"].as<Bool_t>();
	histo->fBFillTrackProc_ResidualU = histo_cfg["fill_ResidualU"].as<Bool_t>();
	histo->fBFillTrackProc_ResidualX2 = histo_cfg["fill_ResidualX2"].as<Bool_t>();

	// TODO: DagoHitProc, CoincProc




	TGo4Log::Info("[TQClamAnalysis] TQClamHistogramParameter erfolgreich konfiguriert.");
}




//***********************************************************
// Konfiguriere Sieve-Slit Parameter
void TQClamAnalysis::InitSieveSlitCalibParameter()
{
	auto* sieve = dynamic_cast<TQClamSieveSlitCalibParameter*>(GetParameter("SieveSlitCalibParam","TQClamSieveSlitCalibParameter"));
	// TODO

}


//***********************************************************
// Konfiguriere Sieve-Slit Parameter
void TQClamAnalysis::InitImageMatrixParameter()
{
	auto* matrix = dynamic_cast<TQClamImageMatrixParameter*>(GetParameter("ImageMatrixParam","TQClamImageMatrixParameter"));

}






#ifdef QCLAM_DAGOBERT
//***********************************************************
// Konfiguriere TcpParameter
void TQClamAnalysis::InitTcpParameter()
{
	auto* tcp = dynamic_cast<TQClamTcpParameter*>(GetParameter("TcpParam","TQClamTcpParameter"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));

	tcp->fbOpenTcpConnection = kFALSE;
	tcp->fbPauseTcpConnection = kFALSE;
	tcp->fxTcpHost = "";
	tcp->fiTcpPort = -1;


	/** config lesen */
	YAML::Node cfg = YAML::LoadFile(master->fTcpConnectionFile.Data());
	auto tcp_con = cfg["tcp_connection"];

	tcp->fbOpenTcpConnection = tcp_con["open_tcp_connection"].as<Bool_t>();
	tcp->fbPauseTcpConnection = tcp_con["pause_tcp_connection"].as<Bool_t>();
	tcp->fxTcpHost = tcp_con["host"].as<std::string>();
	tcp->fiTcpPort = tcp_con["port"].as<Int_t>();

	TGo4Log::Info("[TQClamAnalysis] TQClamTcpConnectionParameter erfolgreich konfiguriert.");
}


//***********************************************************
// Konfiguriere DagobertCalibrationParameter
void TQClamAnalysis::InitDagobertCalibrationParameter()
{
	auto* cali = dynamic_cast<TDagobertCalibrationParameter*>(GetParameter("DagoCalibParam","TDagobertCalibrationParameter"));
	auto* master = dynamic_cast<TQClamMasterParameter*>(GetParameter("MasterParam","TQClamMasterParameter"));

	for(size_t n=0; n<cali->N_DETECTORS; n++)
	{
		cali->a[n] = 0.;
		cali->b[n] = 0.;
		cali->c[n] = 0.;
		cali->d[n] = 0.;
		cali->e[n] = 0.;
		cali->f[n] = 0.;
	}


	/** config lesen */
	YAML::Node cfg = YAML::LoadFile(master->fDagoCalibrationFile.Data());
	auto dago_cali = cfg["calibration"];


	cali->minCoincTimeWin[0] = dago_cali["min_bound_coinc_time_wind_LaBr0"].as<Double_t>();
	cali->minCoincTimeWin[1] = dago_cali["min_bound_coinc_time_wind_LaBr1"].as<Double_t>();
	cali->minCoincTimeWin[2] = dago_cali["min_bound_coinc_time_wind_LaBr2"].as<Double_t>();
	cali->minCoincTimeWin[3] = dago_cali["min_bound_coinc_time_wind_LaBr3"].as<Double_t>();
	cali->minCoincTimeWin[4] = dago_cali["min_bound_coinc_time_wind_LaBr4"].as<Double_t>();
	cali->minCoincTimeWin[5] = dago_cali["min_bound_coinc_time_wind_LaBr5"].as<Double_t>();
	cali->minCoincTimeWin[6] = dago_cali["min_bound_coinc_time_wind_LaBr6"].as<Double_t>();
	cali->minCoincTimeWin[7] = dago_cali["min_bound_coinc_time_wind_LaBr7"].as<Double_t>();
	cali->minCoincTimeWin[8] = dago_cali["min_bound_coinc_time_wind_LaBr8"].as<Double_t>();

	cali->maxCoincTimeWin[0] = dago_cali["max_bound_coinc_time_wind_LaBr0"].as<Double_t>();
	cali->maxCoincTimeWin[1] = dago_cali["max_bound_coinc_time_wind_LaBr1"].as<Double_t>();
	cali->maxCoincTimeWin[2] = dago_cali["max_bound_coinc_time_wind_LaBr2"].as<Double_t>();
	cali->maxCoincTimeWin[3] = dago_cali["max_bound_coinc_time_wind_LaBr3"].as<Double_t>();
	cali->maxCoincTimeWin[4] = dago_cali["max_bound_coinc_time_wind_LaBr4"].as<Double_t>();
	cali->maxCoincTimeWin[5] = dago_cali["max_bound_coinc_time_wind_LaBr5"].as<Double_t>();
	cali->maxCoincTimeWin[6] = dago_cali["max_bound_coinc_time_wind_LaBr6"].as<Double_t>();
	cali->maxCoincTimeWin[7] = dago_cali["max_bound_coinc_time_wind_LaBr7"].as<Double_t>();
	cali->maxCoincTimeWin[8] = dago_cali["max_bound_coinc_time_wind_LaBr8"].as<Double_t>();

	cali->fDoRateCorr = dago_cali["do_rate_corr"].as<Bool_t>();
	cali->fDoTimeWalkCorr = dago_cali["do_timewalk_corr"].as<Bool_t>();
	cali->fDoToFCorr = dago_cali["do_tof_corr"].as<Bool_t>();

	auto energy_cali = dago_cali["energy_calibration"];
	auto rate_corr = dago_cali["rate_correction"];
	auto timewalk_corr = dago_cali["timewalk_correction"];
	auto tof_corr = dago_cali["tof_correction"];

	auto entry0 = energy_cali["detector_0"];
	
	cali->a[0] = entry0["a"].as<Double_t>();
	cali->b[0] = entry0["b"].as<Double_t>();
	cali->c[0] = entry0["c"].as<Double_t>();
	cali->d[0] = entry0["d"].as<Double_t>();
	cali->e[0] = entry0["e"].as<Double_t>();
	cali->f[0] = entry0["f"].as<Double_t>();
	

	auto entry1 = energy_cali["detector_1"];

	cali->a[1] = entry1["a"].as<Double_t>();
	cali->b[1] = entry1["b"].as<Double_t>();
	cali->c[1] = entry1["c"].as<Double_t>();
	cali->d[1] = entry1["d"].as<Double_t>();
	cali->e[1] = entry1["e"].as<Double_t>();
	cali->f[1] = entry1["f"].as<Double_t>();



	auto entry2 = energy_cali["detector_2"];
	
	cali->a[2] = entry2["a"].as<Double_t>();
	cali->b[2] = entry2["b"].as<Double_t>();
	cali->c[2] = entry2["c"].as<Double_t>();
	cali->d[2] = entry2["d"].as<Double_t>();
	cali->e[2] = entry2["e"].as<Double_t>();
	cali->f[2] = entry2["f"].as<Double_t>();
	


	auto entry3 = energy_cali["detector_3"];
	
	cali->a[3] = entry3["a"].as<Double_t>();
	cali->b[3] = entry3["b"].as<Double_t>();
	cali->c[3] = entry3["c"].as<Double_t>();
	cali->d[3] = entry3["d"].as<Double_t>();
	cali->e[3] = entry3["e"].as<Double_t>();
	cali->f[3] = entry3["f"].as<Double_t>();
	


	auto entry4 = energy_cali["detector_4"];
	
	cali->a[4] = entry4["a"].as<Double_t>();
	cali->b[4] = entry4["b"].as<Double_t>();
	cali->c[4] = entry4["c"].as<Double_t>();
	cali->d[4] = entry4["d"].as<Double_t>();
	cali->e[4] = entry4["e"].as<Double_t>();
	cali->f[4] = entry4["f"].as<Double_t>();
	


	auto entry5 = energy_cali["detector_5"];
	
	cali->a[5] = entry5["a"].as<Double_t>();
	cali->b[5] = entry5["b"].as<Double_t>();
	cali->c[5] = entry5["c"].as<Double_t>();
	cali->d[5] = entry5["d"].as<Double_t>();
	cali->e[5] = entry5["e"].as<Double_t>();
	cali->f[5] = entry5["f"].as<Double_t>();
	


	auto entry6 = energy_cali["detector_6"];
	
	cali->a[6] = entry6["a"].as<Double_t>();
	cali->b[6] = entry6["b"].as<Double_t>();
	cali->c[6] = entry6["c"].as<Double_t>();
	cali->d[6] = entry6["d"].as<Double_t>();
	cali->e[6] = entry6["e"].as<Double_t>();
	cali->f[6] = entry6["f"].as<Double_t>();
	


	auto entry7 = energy_cali["detector_7"];
	
	cali->a[7] = entry7["a"].as<Double_t>();
	cali->b[7] = entry7["b"].as<Double_t>();
	cali->c[7] = entry7["c"].as<Double_t>();
	cali->d[7] = entry7["d"].as<Double_t>();
	cali->e[7] = entry7["e"].as<Double_t>();
	cali->f[7] = entry7["f"].as<Double_t>();
	


	auto entry8 = energy_cali["detector_8"];
	
	cali->a[8] = entry8["a"].as<Double_t>();
	cali->b[8] = entry8["b"].as<Double_t>();
	cali->c[8] = entry8["c"].as<Double_t>();
	cali->d[8] = entry8["d"].as<Double_t>();
	cali->e[8] = entry8["e"].as<Double_t>();
	cali->f[8] = entry8["f"].as<Double_t>();
	


	auto entry9 = energy_cali["detector_9"];

	cali->a[9] = entry9["a"].as<Double_t>();
	cali->b[9] = entry9["b"].as<Double_t>();
	cali->c[9] = entry9["c"].as<Double_t>();
	cali->d[9] = entry9["d"].as<Double_t>();
	cali->e[9] = entry9["e"].as<Double_t>();
	cali->f[9] = entry9["f"].as<Double_t>();
	


	// Raten-Korrektur
	for(size_t det = 0; det < TDagobertCalibrationParameter::N_DETECTORS; det++)
	{
		auto entry = rate_corr["detector_" + std::to_string(det)];

		cali->rate_p0[det] = entry["p0"].as<Double_t>();
		cali->rate_p1[det] = entry["p1"].as<Double_t>();
		cali->rate_p2[det] = entry["p2"].as<Double_t>();
		cali->rate_p3[det] = entry["p3"].as<Double_t>();
	}

	// timewalk korrektur
	for(size_t det = 0; det < TDagobertCalibrationParameter::N_DETECTORS; det++)
	{
		auto entry = timewalk_corr["detector_" + std::to_string(det)];

		cali->timewalk_c[det] = entry["c"].as<Double_t>();
		cali->timewalk_E1[det] = entry["E1"].as<Double_t>();
		cali->timewalk_E2[det] = entry["E2"].as<Double_t>();
	}


	// tof korrektur
	cali->tof_c       = tof_corr["c"].as<Double_t>();

	cali->tof_x1      = tof_corr["x1"].as<Double_t>();
	cali->tof_phi1    = tof_corr["phi1"].as<Double_t>();

	cali->tof_phi2    = tof_corr["phi2"].as<Double_t>();
	cali->tof_x1phi1  = tof_corr["x1phi1"].as<Double_t>();
	cali->tof_x2      = tof_corr["x2"].as<Double_t>();

	cali->tof_phi3    = tof_corr["phi3"].as<Double_t>();
	cali->tof_x1phi2  = tof_corr["x1phi2"].as<Double_t>();
	cali->tof_x2phi1  = tof_corr["x2phi1"].as<Double_t>();
	cali->tof_x3      = tof_corr["x3"].as<Double_t>();

	cali->tof_phi4    = tof_corr["phi4"].as<Double_t>();
	cali->tof_x2phi2  = tof_corr["x2phi2"].as<Double_t>();




	TGo4Log::Info("[TQClamAnalysis] TDagobertCalibrationParameter erfolgreich konfiguriert.");
}


#endif






//***********************************************************
// Bei Update von EdgeMappingParameter muss auch DriftchamCalibrationParameter geupdatet werden...
void TQClamAnalysis::RebuildTrailingMapping()
{
	auto* edge = dynamic_cast<TQClamEdgeMappingParameter*>(GetParameter("EdgeMappingParam", "TQClamEdgeMappingParameter"));
	auto* cali = dynamic_cast<TQClamDriftchamCalibrationParameter*>(GetParameter("DriftchamCalibParam", "TQClamDriftchamCalibrationParameter"));

	for(int tdc = 0; tdc < cali->numTDC; ++tdc)
	{
		for(int leadCh = 0; leadCh < cali->numCh[tdc]; ++leadCh)
		{
			if(!edge->IsLeading(tdc, leadCh)) { continue; }

			int trailCh = edge->GetTrailingForLeading(tdc, leadCh);
			if(trailCh < 0) { continue; }

			cali->layerMapTrail[tdc][trailCh] = cali->layerMapLead[tdc][leadCh];
			cali->wireMapTrail[tdc][trailCh]  = cali->wireMapLead[tdc][leadCh];
			cali->trailChannelDisabled[tdc][trailCh] = cali->leadChannelDisabled[tdc][leadCh];
		}
	}

	TGo4Log::Info("[TQClamAnalysis] Trailing-Mapping erfolgreich neu aufgebaut.");
}






/** --------------------------------------------------------
 *  -- Geometrie-Objekte bauen
 *  -------------------------------------------------------- **/



//***********************************************************
// Baue Driftkammer
void TQClamAnalysis::BuildDriftcham()
{
	TGo4Log::Info("[TQClamAnalysis] Baue Driftkammer.");

	auto* par = dynamic_cast<TQClamDriftchamGeomParameter*>(GetParameter("DriftchamGeomParam", "TQClamDriftchamGeomParameter"));

	auto& geo = TQClamGeometry::Instance();
	geo.BuildDriftchamFromParameter(par);
/*
	fDriftcham = new TQClamDriftcham(par->driftchamID, par->generation.Data());
	
	for(int i=0; i<par->nPlanes; i++)
	{
		TQClamWirePlane plane(
				(par->planeType[i] == TQClamDriftchamGeomParameter::X ? WirePlaneType::X : WirePlaneType::U),
				par->zPos[i],
				par->xOffset[i],
				par->yOffset[i],
				par->xLength[i],
				par->yLength[i],
				par->wireDist[i],
				par->angleDeg[i] * M_PI/180.0,
				par->nWires[i],
				i
				);
		fDriftcham->AddWirePlane(plane);
	}

	TGo4Log::Info("[TQClamAnalysis] Driftkammer erfolgreich gebaut!");
*/
}



