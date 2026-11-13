#include "store/TQClamTrackTestingStore.h"

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



ClassImp(TQClamTrackTestingStore)


TQClamTrackTestingStore::TQClamTrackTestingStore()
	: TGo4EventStore("TrackTestingStore")
{
	GO4TRACE((15,"TQClamTrackTestingStore::TQClamTrackTestingStore()",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));

	b_trackNumber = -1;
}

TQClamTrackTestingStore::TQClamTrackTestingStore(TGo4UserStoreParameter* par)
	: TGo4EventStore(par->GetName())
{
	GO4TRACE((15,"TQClamTrackTestingStore::TQClamTrackTestingStore(TGo4UserStoreParameter* par)",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));
	
	/** ggf.... 2GB max size
	TTree::SetMaxTreeSize(2LL * 1024 * 1024 * 1024);
	 */
	if(!par)
	{
		TGo4Log::Error("[TQClamTrackTestingStore] FEHLER: TGo4UserStoreParameter nicht spezifiziert!");
		return;
	}

	SetName(par->GetName());

	TString fname = par->GetName();
	if(!fname.Contains(".root")) { fname.Append(".root"); }

	fxFile = TFile::Open(fname.Data(),"RECREATE");
	TGo4Log::Info("[TQClamTrackTestingStore] Oeffne ROOT-File %s RECREATE.",fname.Data());

	fxTree = new TTree("Track","Track Go4 Store");
	fxTree->Write();
	fxTree->SetAutoSave(100000); // alle 100k Tracks storen

	b_trackNumber = -1;
}




TQClamTrackTestingStore::~TQClamTrackTestingStore()
{
	if(fxFile && fxTree)
	{
		TGo4Log::Info("[TQClamTrackTestingStore] Schliesse File %s.",fxFile->GetName());
		fxFile = fxTree->GetCurrentFile(); // wg. File-Split nach 1.8 GB!
		fxFile->cd();
		fxTree->Write(nullptr,TObject::kOverwrite);
		delete fxFile;
		fxFile = nullptr;
		fxTree = nullptr;
	}

	else
	{
		TGo4Log::Error("[TQClamTrackTestingStore] Finde Store-File oder -Tree nicht!");
	}
}





Int_t TQClamTrackTestingStore::Store(TGo4EventElement* event)
{
	//if(storepar->fBPauseUserTrackTestingStore) { return 0; }


	GO4TRACE((12,"[TQClamTrackTestingStore] Store.",__LINE__,__FILE__));

	
	TQClamTrackEvent* evt = dynamic_cast<TQClamTrackEvent*>(event);
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


		fxTree->Branch("eventnumber", &b_eventNumber);
		fxTree->Branch("tracknumber", &b_trackNumber);
		fxTree->Branch("fBX1",&b_fBX1);
		fxTree->Branch("fBU",&b_fBU);
		fxTree->Branch("fBX2",&b_fBX2);
		//fxTree->Branch("converged", &b_converged);
		fxTree->Branch("nConvergedTracks",&b_nConvergedTracks);
		fxTree->Branch("nNotConvergedTracks",&b_nNotConvergedTracks);
		fxTree->Branch("chi2", &b_chi2);
		fxTree->Branch("chi2ndf", &b_chi2ndf);
		fxTree->Branch("nIterations", &b_nIterations);
		fxTree->Branch("nResiduals", &b_nResiduals);
		fxTree->Branch("nHitsUsed", &b_nHitsUsed);
		fxTree->Branch("nHitsRejected", &b_nHitsRejected);
		fxTree->Branch("meanResidual", &b_meanResidual);
		fxTree->Branch("rmsResidual", &b_rmsResidual);
		fxTree->Branch("maxResidualAbs", &b_maxResidualAbs);
		fxTree->Branch("xD_X1", &b_xD_X1);
		fxTree->Branch("xD_U", &b_xD_U);
		fxTree->Branch("xD_X2", &b_xD_X2);
		fxTree->Branch("yD_U", &b_yD_U);
		fxTree->Branch("yD_U_corr", &b_yD_U_corr);
		fxTree->Branch("phi", &b_phi);
		fxTree->Branch("theta", &b_theta);
		fxTree->Branch("phi_X1", &b_phi_X1);
		fxTree->Branch("phi_U", &b_phi_U);
		fxTree->Branch("phi_X2", &b_phi_X2);
		fxTree->Branch("scattX", &b_scatteringX);
		fxTree->Branch("scattY", &b_scatteringY);
		fxTree->Branch("scattPhi", &b_scatteringPhi);
		fxTree->Branch("scattTheta", &b_scatteringTheta);
		fxTree->Branch("focalX", &b_focalPlaneX);
		fxTree->Branch("focalY", &b_focalPlaneY);
		fxTree->Branch("focalZ", &b_focalPlaneZ);
		fxTree->Branch("channel", &b_channel);
		fxTree->Branch("energyMeVCali", &b_energyMeVCali);

		fxTree->Branch("zResolution_X1", &b_zResolution_X1);
		fxTree->Branch("zResolution_U", &b_zResolution_U);
		fxTree->Branch("zResolution_X2", &b_zResolution_X2);
		fxTree->Branch("xResolution_X1", &b_xResolution_X1);
		fxTree->Branch("xResolution_U", &b_xResolution_U);
		fxTree->Branch("xResolution_X2", &b_xResolution_X2);
		fxTree->Branch("uResolution_U", &b_uResolution_U);
		fxTree->Branch("yResolution_U", &b_yResolution_U);

		fxTree->Branch("sigmaReg_xD_X1", &b_sigmaReg_xD_X1);
		fxTree->Branch("sigmaReg_uD_U", &b_sigmaReg_uD_U);
		fxTree->Branch("sigmaReg_xD_X2", &b_sigmaReg_xD_X2);
		fxTree->Branch("sigmaReg_xD_U", &b_sigmaReg_xD_U);
		fxTree->Branch("sigmaReg_yD_U", &b_sigmaReg_yD_U);

		fxTree->Branch("sigmaReg_phi_X1", &b_sigmaReg_phi_X1);
		fxTree->Branch("sigmaReg_phi_U", &b_sigmaReg_phi_U);
		fxTree->Branch("sigmaReg_phi_X2", &b_sigmaReg_phi_X2);

		fxTree->Branch("sigmaReg_phi", &b_sigmaReg_phi);
		fxTree->Branch("sigmaReg_theta", &b_sigmaReg_theta);


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

		// iterations-test
		fxTree->Branch("iter", &b_iter);
		fxTree->Branch("iter_layerID", &b_iter_layerID);
		fxTree->Branch("iter_wireNumber", &b_iter_wireNumber);
		fxTree->Branch("iter_residual", &b_iter_residual);
		fxTree->Branch("iter_phi", &b_iter_phi);

		// outlier-rejection test
		fxTree->Branch("outlier_iteration", &b_outlier_iteration);
		fxTree->Branch("outlier_layerID", &b_outlier_layerID);
		fxTree->Branch("outlier_wireNumber", &b_outlier_wireNumber);
		fxTree->Branch("outlier_hitIndex", &b_outlier_hitIndex);

		fxTree->Branch("outlier_drifttime", &b_outlier_drifttime);
		fxTree->Branch("outlier_tot", &b_outlier_tot);
		fxTree->Branch("outlier_normalCoord", &b_outlier_normalCoord);

		fxTree->Branch("outlier_measuredZOffset",&b_outlier_measuredZOffset);
		fxTree->Branch("outlier_sigmaMeasuredZOffset",&b_outlier_sigmaMeasuredZOffset);
		fxTree->Branch("outlier_trackDistance",&b_outlier_trackDistance);

		fxTree->Branch("outlier_residual", &b_outlier_residual);
		fxTree->Branch("outlier_pull", &b_outlier_pull);

		fxTree->Branch("outlier_phi_global", &b_outlier_phi_global);
		fxTree->Branch("outlier_phi_plane", &b_outlier_phi_plane);
		fxTree->Branch("outlier_theta_global", &b_outlier_theta_global);

		fxTree->Branch("outlier_intersection", &b_outlier_intersection);



		fxTrackEvent = evt;
		/*
		fxTree->Branch("evennumber",fxTrackEvent->fEvtNumber);
		fxTree->Branch("hits",&fxTrackEvent->hits,32000,0);
		*/
		fbBranchExists = kTRUE;
	}
	else if(fxTrackEvent != evt)
	{
		TGo4Log::Info("[TQClamTrackTestingStore] Event pointer changed!");
		return 1; // error, sollte nicht passieren
	}

	/** vektoren leeren */

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


	b_outlier_iteration.clear();
	b_outlier_layerID.clear();
	b_outlier_wireNumber.clear();
	b_outlier_hitIndex.clear();

	b_outlier_drifttime.clear();
	b_outlier_tot.clear();
	b_outlier_normalCoord.clear();

	b_outlier_measuredZOffset.clear();
	b_outlier_sigmaMeasuredZOffset.clear();

	b_outlier_trackDistance.clear();

	b_outlier_residual.clear();
	b_outlier_pull.clear();

	b_outlier_phi_global.clear();
	b_outlier_phi_plane.clear();
	b_outlier_theta_global.clear();

	b_outlier_intersection.clear();


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




	b_trackNumber++;
	b_eventNumber = evt->fEvtNumber;


	b_fBX1 = evt->fBX1;
	b_fBU = evt->fBU;
	b_fBX2 = evt->fBX2;

	//b_converged = evt->trackAccepted;
	b_nConvergedTracks = evt->fConvergedTrackCounter;
	b_nNotConvergedTracks = evt->fNotConvergedTrackCounter;

	b_chi2 = evt->chi2;
	b_chi2ndf = evt->chi2ndf;
	b_nIterations = evt->nIterations;
	b_nResiduals = evt->nResiduals;
	b_nHitsUsed = evt->nHitsUsed;
	b_nHitsRejected = evt->nHitsRejected;
	
	b_meanResidual = evt->meanResidual;
	b_rmsResidual = evt->rmsResidual;
	b_maxResidualAbs = evt->maxResidualAbs;

	b_xD_X1 = evt->xD_X1;
	b_xD_U = evt->xD_U;
	b_xD_X2 = evt->xD_X2;
	b_yD_U = evt->yD_U;
	b_yD_U_corr = evt->yD_U_corr;
	b_phi = evt->phi*180./M_PI;
	b_phi_X1 = evt->phi_X1;
	b_phi_U = evt->phi_U;
	b_phi_X2 = evt->phi_X2;
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


	b_zResolution_X1 = evt->zResolution_X1;
	b_zResolution_U = evt->zResolution_U;
	b_zResolution_X2 = evt->zResolution_X2;
	b_xResolution_X1 = evt->xResolution_X1;
	b_xResolution_U = evt->xResolution_U;
	b_xResolution_X2 = evt->xResolution_X2;
	b_uResolution_U = evt->uResolution_U;
	b_yResolution_U = evt->yResolution_U;

	b_sigmaReg_phi_X1 = evt->sigmaReg_phi_X1;
	b_sigmaReg_phi_U = evt->sigmaReg_phi_U;
	b_sigmaReg_phi_X2 = evt->sigmaReg_phi_X2;

	b_sigmaReg_xD_X1 = evt->sigmaReg_xD_X1;
	b_sigmaReg_uD_U = evt->sigmaReg_uD_U;
	b_sigmaReg_xD_X2 = evt->sigmaReg_xD_X2;
	b_sigmaReg_xD_U = evt->sigmaReg_xD_U;
	b_sigmaReg_yD_U = evt->sigmaReg_yD_U;

	b_sigmaReg_phi = evt->sigmaReg_phi;
	b_sigmaReg_theta = evt->sigmaReg_theta;



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

	// iterationen test
	for(const auto& r : evt->residualIterationEntries)
	{
		b_iter.push_back(r.iteration);
		b_iter_layerID.push_back(r.layerID);
		b_iter_wireNumber.push_back(r.wireNumber);
		b_iter_residual.push_back(r.residual);
		b_iter_phi.push_back(r.phi*180./M_PI);
	}


	// outlier-rejection test
	for(const auto& snapshot : evt->outlierRejectionSnapshots)
	{
		for(const auto& h : snapshot.hits)
		{
			b_outlier_iteration.push_back(snapshot.iteration);

			b_outlier_layerID.push_back(h.layerID);
			b_outlier_wireNumber.push_back(h.wireNumber);
			b_outlier_hitIndex.push_back(h.hitIndex);

			b_outlier_drifttime.push_back(h.drifttime);
			b_outlier_tot.push_back(h.tot);

			b_outlier_normalCoord.push_back(h.normalCoord);

			b_outlier_measuredZOffset.push_back(h.measuredZOffset);
			b_outlier_sigmaMeasuredZOffset.push_back(h.sigmaMeasuredZOffset);
			b_outlier_trackDistance.push_back(h.trackDistance);

			b_outlier_residual.push_back(h.residual);
			b_outlier_pull.push_back(h.pull);

			b_outlier_phi_global.push_back(h.phi_global * 180. / M_PI);
			b_outlier_phi_plane.push_back(h.phi_plane * 180. / M_PI);
			b_outlier_theta_global.push_back(h.theta_global * 180. / M_PI);
			b_outlier_intersection.push_back(snapshot.intersection);
		}
	}
	

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



