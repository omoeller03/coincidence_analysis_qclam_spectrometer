#include "store/TQClamDagoStore.h"

#include "TGo4Analysis.h"

#include "parameter/TQClamStoreParameter.h"
#include "evt/TQClamDagobertCoincidenceEvent.h"

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TBranch.h"

#include "TGo4Log.h"
#include "TGo4Status.h"
#include "TGo4UserStoreParameter.h"
#include "TGo4EventElement.h"



ClassImp(TQClamDagoStore)


TQClamDagoStore::TQClamDagoStore()
	: TGo4EventStore("CoincStore")
{
	GO4TRACE((15,"TQClamDagoStore::TQClamDagoStore()",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));

}

TQClamDagoStore::TQClamDagoStore(TGo4UserStoreParameter* par)
	: TGo4EventStore(par->GetName())
{
	GO4TRACE((15,"TQClamDagoStore::TQClamDagoStore(TGo4UserStoreParameter* par)",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));
	
	/** ggf.... 2GB max size
	TTree::SetMaxTreeSize(2LL * 1024 * 1024 * 1024);
	 */
	if(!par)
	{
		TGo4Log::Error("[TQClamDagoStore] FEHLER: TGo4UserStoreParameter nicht spezifiziert!");
		return;
	}

	SetName(par->GetName());

	TString fname = par->GetName();
	if(!fname.Contains(".root")) { fname.Append(".root"); }

	fxFile = TFile::Open(fname.Data(),"RECREATE");
	TGo4Log::Info("[TQClamDagoStore] Oeffne ROOT-File %s RECREATE.",fname.Data());

	fxTree = new TTree("Coinc","Coincidence Go4 Store");
	fxTree->Write();
	fxTree->SetAutoSave(100000); // alle 100k Tracks storen

}




TQClamDagoStore::~TQClamDagoStore()
{
	if(fxFile && fxTree)
	{
		TGo4Log::Info("[TQClamDagoStore] Schliesse File %s.",fxFile->GetName());
		fxFile = fxTree->GetCurrentFile(); // wg. File-Split nach 1.8 GB!
		fxFile->cd();
		fxTree->Write(nullptr,TObject::kOverwrite);
		delete fxFile;
		fxFile = nullptr;
		fxTree = nullptr;
	}

	else
	{
		TGo4Log::Error("[TQClamDagoStore] Finde Store-File oder -Tree nicht!");
	}
}





Int_t TQClamDagoStore::Store(TGo4EventElement* event)
{
	//if(storepar->fBPauseUserDagoStore) { return 0; }


	GO4TRACE((12,"[TQClamDagoStore] Store.",__LINE__,__FILE__));

	
	TQClamDagobertCoincidenceEvent* evt = dynamic_cast<TQClamDagobertCoincidenceEvent*>(event);
	if(!evt) { return 1; } // error

	if(!fbBranchExists)
	{
		fxTree->Branch("file_number", &b_file_number);
		fxTree->Branch("run_number", &b_run_number);

		fxTree->Branch("user", &b_user);
		fxTree->Branch("time", &b_user);
		fxTree->Branch("label", &b_label);

		fxTree->Branch("mbs_event_count", &b_mbs_event_count);
		fxTree->Branch("mbs_trigger", &b_mbs_trigger);
		fxTree->Branch("mbs_buffer_number", &b_mbs_buffer_number);
		fxTree->Branch("mbs_evts_in_buffer", &b_mbs_evts_in_buffer);
		fxTree->Branch("mbs_time_sec", &b_mbs_time_sec);
		fxTree->Branch("mbs_time_subsec", &b_mbs_time_subsec);


                fxTree->Branch("scalerID", &b_scalerID);
                fxTree->Branch("scaler_timeMicroSeconds", &b_scaler_timeMicroSeconds);

                fxTree->Branch("scaler_tdcTrigger", &b_scaler_tdcTrigger);
                fxTree->Branch("scaler_szintillatorPmtUp", &b_scaler_szintillatorPmtUp);
                fxTree->Branch("scaler_szintillatorPmtDown", &b_scaler_szintillatorPmtDown);
                fxTree->Branch("scaler_cherenkovPmt", &b_scaler_cherenkovPmt);
                fxTree->Branch("scaler_coincidenceTrigger", &b_scaler_coincidenceTrigger);
                fxTree->Branch("scaler_acceptedTrigger", &b_scaler_acceptedTrigger);
                fxTree->Branch("scaler_clockCounter", &b_scaler_clockCounter);
                fxTree->Branch("scaler_livetimeCounter", &b_scaler_livetimeCounter);
                fxTree->Branch("scaler_mbsLivetimeCounter", &b_scaler_mbsLivetimeCounter);
                fxTree->Branch("scaler_faradayCupChargeCounter", &b_scaler_faradayCupChargeCounter);

                fxTree->Branch("scaler_errorTdcTrigger", &b_scaler_errorTdcTrigger);
                fxTree->Branch("scaler_errorSzintillatorPmtUp", &b_scaler_errorSzintillatorPmtUp);
                fxTree->Branch("scaler_errorSzintillatorPmtDown", &b_scaler_errorSzintillatorPmtDown);
                fxTree->Branch("scaler_errorCherenkovPmt", &b_scaler_errorCherenkovPmt);
                fxTree->Branch("scaler_errorCoincidenceTrigger", &b_scaler_errorCoincidenceTrigger);
                fxTree->Branch("scaler_errorAcceptedTrigger", &b_scaler_errorAcceptedTrigger);
                fxTree->Branch("scaler_errorClockCounter", &b_scaler_errorClockCounter);
                fxTree->Branch("scaler_errorLivetimeCounter", &b_scaler_errorLivetimeCounter);
                fxTree->Branch("scaler_errorMbsLivetimeCounter", &b_scaler_errorMbsLivetimeCounter);
                fxTree->Branch("scaler_errorFaradayCupChargeCounter", &b_scaler_errorFaradayCupChargeCounter);

		fxTree->Branch("timeSzintDown", &b_timeSzintDown);
		fxTree->Branch("timeSzintUp", &b_timeSzintUp);
		fxTree->Branch("timeCherenkov", &b_timeCherenkov);


		fxTree->Branch("phi_deg", &b_phi_deg);
		fxTree->Branch("theta_deg", &b_theta_deg);
		fxTree->Branch("phi_deg_corr", &b_phi_deg_corr);

		fxTree->Branch("xD_X1", &b_xD_X1);
		fxTree->Branch("xD_U", &b_xD_U);
		fxTree->Branch("xD_X2", &b_xD_X2);
		fxTree->Branch("uD_U", &b_uD_U);
		fxTree->Branch("yD_U", &b_yD_U);

		fxTree->Branch("xD_X1_corr", &b_xD_X1_corr);
		fxTree->Branch("yD_U_corr", &b_yD_U_corr);

		fxTree->Branch("el_channel", &b_el_channel);
		fxTree->Branch("el_energyMeVCali", &b_el_energy_MeV);

		fxTree->Branch("scattX", &b_scattX);
		fxTree->Branch("scattY", &b_scattY);
		fxTree->Branch("scattPhi", &b_scattPhi);
		fxTree->Branch("scattTheta", &b_scattTheta);

		fxTree->Branch("rateEnton", &b_rateENTON);
		fxTree->Branch("rate_SzintScatt", &b_rateSzintScatt);
		fxTree->Branch("rateLaBrOR", &b_rateLaBrOR);
		fxTree->Branch("rateLaBrORNoDead", &b_rateLaBrORNoDead);

		fxTree->Branch("coincTimeLaBr", &b_coincTimeLaBr);
		fxTree->Branch("szintPmtUpTime", &b_szintPmtUpTime);
		fxTree->Branch("szintPmtDownTime", &b_szintPmtDownTime);
		fxTree->Branch("meanSzintTime", &b_meanSzintTime);

		fxTree->Branch("energyLaBr", &b_energy_LaBr);
		fxTree->Branch("calibrated_energy_LaBr", &b_calibrated_energy_LaBr);
		fxTree->Branch("time_LaBr", &b_time_LaBr);
		fxTree->Branch("dtToMeanSzintTime_LaBr", &b_dtToMeanSzintTime_LaBr);
		fxTree->Branch("dtToLaBrCoincTime_LaBr", &b_dtToLaBrCoincTime_LaBr);
		fxTree->Branch("pileup_LaBr", &b_pileup_LaBr);
		fxTree->Branch("rateRaw_LaBr", &b_rateRaw_LaBr);
		fxTree->Branch("rateAccepted_LaBr", &b_rateAccepted_LaBr);
		fxTree->Branch("deadtime_LaBr", &b_deadtime_LaBr);
		fxTree->Branch("timewalk_LaBr", &b_timewalk_LaBr);
		fxTree->Branch("dtToMeanTime_Corr_LaBr", &b_dtToMeanTime_Corr_LaBr);

		fxTree->Branch("tof", &b_tof);



		fxCoincEvent = evt;

		fbBranchExists = kTRUE;
	}
	else if(fxCoincEvent != evt)
	{
		TGo4Log::Info("[TQClamDagoStore] Event pointer changed!");
		return 1; // error, sollte nicht passieren
	}

	/** vektoren leeren */
	b_energy_LaBr.clear();
	b_calibrated_energy_LaBr.clear();
	b_time_LaBr.clear();
	b_dtToMeanSzintTime_LaBr.clear();
	b_dtToLaBrCoincTime_LaBr.clear();
	b_pileup_LaBr.clear();
	b_rateRaw_LaBr.clear();
	b_rateAccepted_LaBr.clear();
	b_deadtime_LaBr.clear();
	b_timewalk_LaBr.clear();
	b_dtToMeanTime_Corr_LaBr.clear();

	/** reserve */
	b_energy_LaBr.reserve(9);
	b_calibrated_energy_LaBr.reserve(9);
	b_time_LaBr.reserve(9);
	b_dtToMeanSzintTime_LaBr.reserve(9);
	b_dtToLaBrCoincTime_LaBr.reserve(9);
	b_pileup_LaBr.reserve(9);
	b_rateRaw_LaBr.reserve(9);
	b_rateAccepted_LaBr.reserve(9);
	b_deadtime_LaBr.reserve(9);
	b_timewalk_LaBr.reserve(9);
	b_dtToMeanTime_Corr_LaBr.reserve(9);





	/** neue werte setzen */
	b_file_name = evt->header.file_name;
	b_run_number = evt->header.run_number;
	b_file_number = evt->header.file_number;

	b_user = evt->header.user;
	b_time = evt->header.time;
	b_label = evt->header.label;

	b_mbs_event_count = evt->header.mbs_event_count;
	b_mbs_trigger = evt->header.mbs_trigger;
	b_mbs_buffer_number = evt->header.mbs_buffer_number;
	b_mbs_evts_in_buffer = evt->header.mbs_evts_in_buffer;
	b_mbs_time_sec = evt->header.mbs_time_sec;
	b_mbs_time_subsec = evt->header.mbs_time_subsec;


        b_scalerID = evt->scaler.scalerID;
        b_scaler_timeMicroSeconds = evt->scaler.timeMicroSeconds;
        b_scaler_tdcTrigger = evt->scaler.tdcTrigger;
        b_scaler_szintillatorPmtUp = evt->scaler.szintillatorPmtUp;
        b_scaler_szintillatorPmtDown = evt->scaler.szintillatorPmtDown;
        b_scaler_cherenkovPmt = evt->scaler.cherenkovPmt;
        b_scaler_coincidenceTrigger = evt->scaler.coincidenceTrigger;
        b_scaler_acceptedTrigger = evt->scaler.acceptedTrigger;
        b_scaler_clockCounter = evt->scaler.clockCounter;
        b_scaler_livetimeCounter = evt->scaler.livetimeCounter;
        b_scaler_mbsLivetimeCounter = evt->scaler.mbsLivetimeCounter;
        b_scaler_faradayCupChargeCounter = evt->scaler.faradayCupChargeCounter;

        b_scaler_errorTdcTrigger = evt->scaler.errorTdcTrigger;
        b_scaler_errorSzintillatorPmtUp = evt->scaler.errorSzintillatorPmtUp;
        b_scaler_errorSzintillatorPmtDown = evt->scaler.errorSzintillatorPmtDown;
        b_scaler_errorCherenkovPmt = evt->scaler.errorCherenkovPmt;
        b_scaler_errorCoincidenceTrigger = evt->scaler.errorCoincidenceTrigger;
        b_scaler_errorAcceptedTrigger = evt->scaler.errorAcceptedTrigger;
        b_scaler_errorClockCounter = evt->scaler.errorClockCounter;
        b_scaler_errorLivetimeCounter = evt->scaler.errorLivetimeCounter;
        b_scaler_errorMbsLivetimeCounter = evt->scaler.errorMbsLivetimeCounter;
        b_scaler_errorFaradayCupChargeCounter = evt->scaler.errorFaradayCupChargeCounter;


	b_timeSzintDown = evt->trigDet.timeSzintDown;
	b_timeSzintUp = evt->trigDet.timeSzintUp;
	b_timeCherenkov = evt->trigDet.timeCherenkov;



	b_phi_deg = evt->phi_deg;
	b_theta_deg = evt->theta_deg;
	b_phi_deg_corr = evt->phi_deg_corr;

	b_xD_X1 = evt->xD_X1;
	b_xD_U = evt->xD_U;
	b_xD_X2 = evt->xD_X2;
	b_uD_U = evt->uD_U;
	b_yD_U = evt->yD_U;

	b_xD_X1_corr = evt->xD_X1_corr;
	b_yD_U_corr = evt->yD_U_corr;

	b_el_channel = evt->el_channel;
	b_el_energy_MeV = evt->el_energy_MeV;
	
	b_scattTheta = evt->scattTheta;
	b_scattPhi = evt->scattPhi;
	b_scattX = evt->scattX;
	b_scattY = evt->scattY;

	b_rateENTON = evt->rateEnton;
	b_rateSzintScatt = evt->rateSzintScattChamber;
	b_rateLaBrOR = evt->rateLaBrOR;
	b_rateLaBrORNoDead = evt->rateLaBrORNoDead;

	b_coincTimeLaBr = evt->coincTimeLaBr;
	b_szintPmtUpTime = evt->szintPmtUpTime;
	b_szintPmtDownTime = evt->szintPmtDownTime;
	b_meanSzintTime = evt->meanSzintTime;

	b_tof = evt->tof;

	for(auto& hit : evt->laBr)
	{
		b_energy_LaBr.push_back(hit.energy);
		b_calibrated_energy_LaBr.push_back(hit.calibrated_energy);
		b_time_LaBr.push_back(hit.time);
		b_dtToMeanSzintTime_LaBr.push_back(hit.dtToMeanSzintTime);
		b_dtToLaBrCoincTime_LaBr.push_back(hit.dtToLaBrCoincTime);
		b_pileup_LaBr.push_back(hit.pileup);
		b_rateRaw_LaBr.push_back(hit.rateRaw);
		b_rateAccepted_LaBr.push_back(hit.rateAccepted);
		b_deadtime_LaBr.push_back(hit.deadtime);
		b_timewalk_LaBr.push_back(hit.timewalk);
		b_dtToMeanTime_Corr_LaBr.push_back(hit.dtToMeanSzintTime_Corr);
	}


	fxTree->Fill();
	return 0;
}



