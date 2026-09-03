#include "store/TQClamDriftlengthCalibStore.h"

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



ClassImp(TQClamDriftlengthCalibStore)


TQClamDriftlengthCalibStore::TQClamDriftlengthCalibStore()
	: TGo4EventStore("DriftlengthCalibStore")
{
	GO4TRACE((15,"TQClamDriftlengthCalibStore::TQClamDriftlengthCalibStore()",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));

	b_trackNumber = -1;
}

TQClamDriftlengthCalibStore::TQClamDriftlengthCalibStore(TGo4UserStoreParameter* par)
	: TGo4EventStore(par->GetName())
{
	GO4TRACE((15,"TQClamDriftlengthCalibStore::TQClamDriftlengthCalibStore(TGo4UserStoreParameter* par)",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
        storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));
	
	/** ggf.... 2GB max size
	TTree::SetMaxTreeSize(2LL * 1024 * 1024 * 1024);
	 */
	if(!par)
	{
		TGo4Log::Error("[TQClamDriftlengthCalibStore] FEHLER: TGo4UserStoreParameter nicht spezifiziert!");
		return;
	}

	SetName(par->GetName());

	TString fname = par->GetName();
	if(!fname.Contains(".root")) { fname.Append(".root"); }

	fxFile = TFile::Open(fname.Data(),"RECREATE");
	TGo4Log::Info("[TQClamDriftlengthCalibStore] Oeffne ROOT-File %s RECREATE.",fname.Data());

	fxTree = new TTree("Track","Track Go4 Store");
	fxTree->Write();
	fxTree->SetAutoSave(100000); // alle 100k Tracks storen

	b_trackNumber = -1;
}




TQClamDriftlengthCalibStore::~TQClamDriftlengthCalibStore()
{
	if(fxFile && fxTree)
	{
		TGo4Log::Info("[TQClamDriftlengthCalibStore] Schliesse File %s.",fxFile->GetName());
		fxFile = fxTree->GetCurrentFile(); // wg. File-Split nach 1.8 GB!
		fxFile->cd();
		fxTree->Write(nullptr,TObject::kOverwrite);
		delete fxFile;
		fxFile = nullptr;
		fxTree = nullptr;
	}

	else
	{
		TGo4Log::Error("[TQClamDriftlengthCalibStore] Finde Store-File oder -Tree nicht!");
	}
}





Int_t TQClamDriftlengthCalibStore::Store(TGo4EventElement* event)
{
	//if(storepar->fBPauseUserDriftlengthCalibStore) { return 0; }


	GO4TRACE((12,"[TQClamDriftlengthCalibStore] Store.",__LINE__,__FILE__));

	
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
		fxTree->Branch("converged",&b_converged);
		fxTree->Branch("xD_X1", &b_xD_X1);
		fxTree->Branch("xD_U", &b_xD_U);
		fxTree->Branch("xD_X2", &b_xD_X2);
		fxTree->Branch("phi", &b_phi);
		fxTree->Branch("theta", &b_theta);
		fxTree->Branch("res_layerID", &b_res_layerID);
		fxTree->Branch("res_wireNumber", &b_res_wireNumber);
		fxTree->Branch("drifttimes", &b_drifttimes);
		fxTree->Branch("tot", &b_tot);
		fxTree->Branch("residual", &b_residual);
		fxTree->Branch("pull", &b_pull);

		fxTrackEvent = evt;
		/*
		fxTree->Branch("evennumber",fxTrackEvent->fEvtNumber);
		fxTree->Branch("hits",&fxTrackEvent->hits,32000,0);
		*/
		fbBranchExists = kTRUE;
	}
	else if(fxTrackEvent != evt)
	{
		TGo4Log::Info("[TQClamDriftlengthCalibStore] Event pointer changed!");
		return 1; // error, sollte nicht passieren
	}

	/** vektoren leeren */

	b_drifttimes.clear();
	b_tot.clear();
	b_res_layerID.clear();
	b_res_wireNumber.clear();
	b_residual.clear();
	b_pull.clear();

	size_t n = evt->residualEntries.size();
	b_drifttimes.reserve(n);
	b_tot.reserve(n);
	b_res_layerID.reserve(n);
	b_res_wireNumber.reserve(n);
	b_residual.reserve(n);
	b_pull.reserve(n);



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


	b_converged = evt->trackAccepted;

	b_xD_X1 = evt->xD_X1;
	b_xD_U = evt->xD_U;
	b_xD_X2 = evt->xD_X2;
	b_phi = evt->phi*180./M_PI;
	b_theta = evt->theta*180./M_PI;

	for(const auto& r : evt->residualEntries)
	{
		b_res_layerID.push_back(r.layerID);
		b_res_wireNumber.push_back(r.wireNumber);
		b_drifttimes.push_back(r.drifttime);
		b_tot.push_back(r.tot);
		b_residual.push_back(r.residual);
		b_pull.push_back(r.pull);
	}
	

	fxTree->Fill();
	return 0;
}



