#include "store/TQClamTrackStore.h"

#include "TGo4Analysis.h"

#include "parameter/TQClamStoreParameter.h"
#include "evt/TQClamTrackEvent.h"

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TBranch.h"

#include "TGo4Log.h"
#include "TGo4Status.h"
#include "TGo4UserStoreParameter.h"
#include "TGo4EventElement.h"



ClassImp(TQClamTrackStore)


TQClamTrackStore::TQClamTrackStore()
	: TGo4EventStore("TrackStore")
{
	GO4TRACE((15,"TQClamTrackStore::TQClamTrackStore()",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));

	b_trackNumber = -1;
}

TQClamTrackStore::TQClamTrackStore(TGo4UserStoreParameter* par)
	: TGo4EventStore(par->GetName())
{
	GO4TRACE((15,"TQClamTrackStore::TQClamTrackStore(TGo4UserStoreParameter* par)",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));
	
	/** ggf.... 2GB max size
	TTree::SetMaxTreeSize(2LL * 1024 * 1024 * 1024);
	 */
	if(!par)
	{
		TGo4Log::Error("[TQClamTrackStore] FEHLER: TGo4UserStoreParameter nicht spezifiziert!");
		return;
	}

	SetName(par->GetName());

	TString fname = par->GetName();
	if(!fname.Contains(".root")) { fname.Append(".root"); }

	fxFile = TFile::Open(fname.Data(),"RECREATE");
	TGo4Log::Info("[TQClamTrackStore] Oeffne ROOT-File %s RECREATE.",fname.Data());

	fxTree = new TTree("Track","Track Go4 Store");
	fxTree->Write();
	fxTree->SetAutoSave(100000); // alle 100k Tracks storen

	b_trackNumber = -1;
}




TQClamTrackStore::~TQClamTrackStore()
{
	if(fxFile && fxTree)
	{
		TGo4Log::Info("[TQClamTrackStore] Schliesse File %s.",fxFile->GetName());
		fxFile = fxTree->GetCurrentFile(); // wg. File-Split nach 1.8 GB!
		fxFile->cd();
		fxTree->Write(nullptr,TObject::kOverwrite);
		delete fxFile;
		fxFile = nullptr;
		fxTree = nullptr;
	}

	else
	{
		TGo4Log::Error("[TQClamTrackStore] Finde Store-File oder -Tree nicht!");
	}
}





Int_t TQClamTrackStore::Store(TGo4EventElement* event)
{
	if(storepar->fBPauseUserTrackStore) { return 0; }


	GO4TRACE((12,"[TQClamTrackStore] Store.",__LINE__,__FILE__));

	
	TQClamTrackEvent* evt = dynamic_cast<TQClamTrackEvent*>(event);
	if(!evt) { return 1; } // error

	if(!fbBranchExists)
	{
		fxTree->Branch("file_number", &b_file_number);
		fxTree->Branch("run_number", &b_run_number);

		//fxTree->Branch("user", &b_user);
		fxTree->Branch("time", &b_time);
		//fxTree->Branch("label", &b_label);

		fxTree->Branch("mbs_event_count", &b_mbs_event_count);
		fxTree->Branch("mbs_trigger", &b_mbs_trigger);
		//fxTree->Branch("mbs_buffer_number", &b_mbs_buffer_number);
		//fxTree->Branch("mbs_evts_in_buffer", &b_mbs_evts_in_buffer);
		fxTree->Branch("mbs_time_sec", &b_mbs_time_sec);
		fxTree->Branch("mbs_time_subsec", &b_mbs_time_subsec);

		//fxTree->Branch("scalerID", &b_scalerID);
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



		fxTree->Branch("eventnumber", &b_eventNumber);
		fxTree->Branch("tracknumber", &b_trackNumber);
		fxTree->Branch("fBX1", &b_fBX1);
		fxTree->Branch("fBU", &b_fBU);
		fxTree->Branch("fBX2", &b_fBX2);
		fxTree->Branch("converged", &b_converged);
		fxTree->Branch("nConvergedTracks",&b_nConvergedTracks);
		fxTree->Branch("nNotConvergedTracks",&b_nNotConvergedTracks);
		/*
		fxTree->Branch("chi2", &b_chi2);
		fxTree->Branch("chi2ndf", &b_chi2ndf);
		fxTree->Branch("nIterations", &b_nIterations);
		fxTree->Branch("nResiduals", &b_nResiduals);
		fxTree->Branch("nHitsUsed", &b_nHitsUsed);
		fxTree->Branch("nHitsRejected", &b_nHitsRejected);
		fxTree->Branch("meanResidual", &b_meanResidual);
		fxTree->Branch("rmsResidual", &b_rmsResidual);
		fxTree->Branch("maxResidualAbs", &b_maxResidualAbs);
		*/

		fxTree->Branch("xD_X1", &b_xD_X1);
		fxTree->Branch("xD_U", &b_xD_U);
		fxTree->Branch("xD_X2", &b_xD_X2);
		fxTree->Branch("uD_U", &b_uD_U);
		fxTree->Branch("yD_U", &b_yD_U);
		fxTree->Branch("yD_U_corr", &b_yD_U_corr);
		fxTree->Branch("phi", &b_phi);
		fxTree->Branch("theta", &b_theta);
		fxTree->Branch("scattX", &b_scatteringX);
		fxTree->Branch("scattY", &b_scatteringY);
		fxTree->Branch("scattPhi", &b_scatteringPhi);
		fxTree->Branch("scattTheta", &b_scatteringTheta);
		fxTree->Branch("focalX", &b_focalPlaneX);
		fxTree->Branch("focalY", &b_focalPlaneY);
		fxTree->Branch("focalZ", &b_focalPlaneZ);
		fxTree->Branch("channel", &b_channel);
		fxTree->Branch("energyMeVCali", &b_energyMeVCali);

		/*
		fxTree->Branch("res_layerID", &b_res_layerID);
		fxTree->Branch("res_wireNumber", &b_res_wireNumber);
		fxTree->Branch("drifttimes", &b_drifttimes);
		fxTree->Branch("tot", &b_tot);
		fxTree->Branch("residual", &b_residual);
		fxTree->Branch("sigmaResidual", &b_sigmaResidual);
		fxTree->Branch("pull", &b_pull);
		fxTree->Branch("z_measured", &b_z_measured);
		fxTree->Branch("sigma_z_measured", &b_sigma_z_measured);
		fxTree->Branch("z_track", &b_z_track);
		fxTree->Branch("sigma_z_track", &b_sigma_z_track);
		fxTree->Branch("iter", &b_iter);
		fxTree->Branch("iter_layerID", &b_iter_layerID);
		fxTree->Branch("iter_wireNumber", &b_iter_wireNumber);
		fxTree->Branch("iter_residual", &b_iter_residual);
		fxTree->Branch("iter_phi", &b_iter_phi);
		*/

		fxTrackEvent = evt;
		/*
		fxTree->Branch("evennumber",fxTrackEvent->fEvtNumber);
		fxTree->Branch("hits",&fxTrackEvent->hits,32000,0);
		*/
		fbBranchExists = kTRUE;
	}
	else if(fxTrackEvent != evt)
	{
		TGo4Log::Info("[TQClamTrackStore] Event pointer changed!");
		return 1; // error, sollte nicht passieren
	}

	/** vektoren leeren */

	/*
	b_drifttimes.clear();
	b_tot.clear();
	b_res_layerID.clear();
	b_res_wireNumber.clear();
	b_residual.clear();
	b_sigmaResidual.clear();
	b_pull.clear();
	b_z_measured.clear();
	b_z_track.clear();
	b_sigma_z_measured.clear();
	b_sigma_z_track.clear();

	b_iter.clear();
	b_iter_layerID.clear();
	b_iter_wireNumber.clear();
	b_iter_residual.clear();
	b_iter_phi.clear();

	size_t n = evt->residualEntries.size();
	b_drifttimes.reserve(n);
	b_tot.reserve(n);
	b_res_layerID.reserve(n);
	b_res_wireNumber.reserve(n);
	b_residual.reserve(n);
	b_sigmaResidual.reserve(n);
	b_pull.reserve(n);
	b_z_measured.reserve(n);
	b_z_track.reserve(n);
	b_sigma_z_measured.reserve(n);
	b_sigma_z_track.reserve(n);

	size_t n_iter = evt->residualIterationEntries.size();
	b_iter.reserve(n_iter);
	b_iter_layerID.reserve(n_iter);
	b_iter_wireNumber.reserve(n_iter);
	b_iter_residual.reserve(n_iter);
	b_iter_phi.reserve(n_iter);
	*/




	/** neue werte setzen */
	b_file_name = evt->header.file_name;
	b_run_number = evt->header.run_number;
	b_file_number = evt->header.file_number;

	//b_user = evt->header.user;
	b_time = evt->header.time;
	//b_label = evt->header.label;

	b_mbs_event_count = evt->header.mbs_event_count;
	b_mbs_trigger = evt->header.mbs_trigger;
	//b_mbs_buffer_number = evt->header.mbs_buffer_number;
	//b_mbs_evts_in_buffer = evt->header.mbs_evts_in_buffer;
	b_mbs_time_sec = evt->header.mbs_time_sec;
	b_mbs_time_subsec = evt->header.mbs_time_subsec;


	//b_scalerID = evt->scaler.scalerID;
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




	b_trackNumber++;
	b_eventNumber = evt->fEvtNumber;


	/*
	b_chi2 = evt->chi2;
	b_chi2ndf = evt->chi2ndf;
	b_nIterations = evt->nIterations;
	b_nResiduals = evt->nResiduals;
	b_nHitsUsed = evt->nHitsUsed;
	b_nHitsRejected = evt->nHitsRejected;
	
	b_meanResidual = evt->meanResidual;
	b_rmsResidual = evt->rmsResidual;
	b_maxResidualAbs = evt->maxResidualAbs;
	*/

	b_fBX1 = evt->fBX1;
	b_fBU = evt->fBU;
	b_fBX2 = evt->fBX2;

	b_converged = evt->trackAccepted;
	b_nConvergedTracks = evt->fConvergedTrackCounter;
	b_nNotConvergedTracks = evt->fNotConvergedTrackCounter;


	b_xD_X1 = evt->xD_X1;
	b_xD_U = evt->xD_U;
	b_xD_X2 = evt->xD_X2;
	b_uD_U = evt->uD;
	b_yD_U = evt->yD_U;
	b_yD_U_corr = evt->yD_U_corr;
	b_phi = evt->phi*180./M_PI;
	b_theta = evt->theta*180./M_PI;

	b_scatteringTheta = evt->scatteringTheta;
	b_scatteringPhi = evt->scatteringPhi;
	b_scatteringX = evt->scatteringX;
	b_scatteringY = evt->scatteringY;

	b_focalPlaneX = evt->focalPlaneX;
	b_focalPlaneY = evt->focalPlaneY;
	b_focalPlaneZ = evt->focalPlaneZ;

	b_channel = evt->channel;
	b_energyMeVCali = evt->energyMeVCali;

	/*
	for(const auto& r : evt->residualEntries)
	{
		b_res_layerID.push_back(r.layerID);
		b_res_wireNumber.push_back(r.wireNumber);
		b_drifttimes.push_back(r.drifttime);
		b_tot.push_back(r.tot);
		b_residual.push_back(r.residual);
		b_sigmaResidual.push_back(r.sigma_residual);
		b_pull.push_back(r.pull);
		b_z_measured.push_back(r.z_measured);
		b_z_track.push_back(r.z_track);
		b_sigma_z_measured.push_back(r.sigma_z_measured);
		b_sigma_z_track.push_back(r.sigma_z_track);
	}

	for(const auto& r : evt->residualIterationEntries)
	{
		b_iter.push_back(r.iteration);
		b_iter_layerID.push_back(r.layerID);
		b_iter_wireNumber.push_back(r.wireNumber);
		b_iter_residual.push_back(r.residual);
		b_iter_phi.push_back(r.phi*180./M_PI);
	}
	*/
	

	/*
	b_layerID.clear();
	b_wireNum.clear();
	b_leading.clear();
	b_trailing.clear();
	b_tdcTrigTime.clear();
	b_tot.clear();
	b_drifttime.clear();
	*/

	/** hits aus event in vektoren kopieren */
	/*
	for(const auto& hit : evt->hits)
	{
		b_layerID.push_back(hit.layerID);
		b_wireNum.push_back(hit.wireNum);
		b_leading.push_back(hit.leading);
		b_trailing.push_back(hit.trailing);
		b_tdcTrigTime.push_back(hit.tdcTrigTime);
		b_tot.push_back(hit.tot);
		b_drifttime.push_back(hit.drifttime);
	}
	*/

	fxTree->Fill();
	return 0;
}



