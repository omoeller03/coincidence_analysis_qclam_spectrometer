#include "store/TQClamDrifttimeCalibStore.h"

#include "TGo4Analysis.h"

#include "parameter/TQClamStoreParameter.h"
#include "evt/TQClamHitEvent.h"
#include "evt/TQClamClusterEvent.h"

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TBranch.h"

#include "TGo4Log.h"
#include "TGo4UserStoreParameter.h"
#include "TGo4EventElement.h"



ClassImp(TQClamDrifttimeCalibStore)


TQClamDrifttimeCalibStore::TQClamDrifttimeCalibStore()
	: TGo4EventStore("DrifttimeCalibStore")
{
	GO4TRACE((15,"TQClamDrifttimeCalibStore::TQClamDrifttimeCalibStore()",__LINE__,__FILE__));

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
	storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));
}

TQClamDrifttimeCalibStore::TQClamDrifttimeCalibStore(TGo4UserStoreParameter* par)
	: TGo4EventStore(par->GetName())
{
	GO4TRACE((15,"TQClamDrifttimeCalibStore::TQClamDrifttimeCalibStore(TGo4UserStoreParameter* par)",__LINE__,__FILE__));
	TGo4Log::Info("Store object this=%p", this);

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
	storepar = dynamic_cast<TQClamStoreParameter*>(TGo4Analysis::Instance()->GetParameter("StoreParam"));

	/** ggf.... 2GB max size
	TTree::SetMaxTreeSize(2LL * 1024 * 1024 * 1024);
	 */
	if(!par)
	{
		TGo4Log::Error("[TQClamDrifttimeCalibStore] FEHLER: TGo4UserStoreParameter nicht spezifiziert!");
		return;
	}

	SetName(par->GetName());

	TString fname = par->GetName();
	if(!fname.Contains(".root")) { fname.Append(".root"); }

	fxFile = TFile::Open(fname.Data(),"RECREATE");
	TGo4Log::Info("[TQClamDrifttimeCalibStore] Oeffne ROOT-File %s RECREATE.",fname.Data());

	fxTree = new TTree("DrifttimeCalib","Drifttime Calibration Go4 Store");
	fxTree->Write();
	fxTree->SetAutoSave(100000); // alle 100k HitEvents storen
}




TQClamDrifttimeCalibStore::~TQClamDrifttimeCalibStore()
{
	if(fxFile && fxTree)
	{
		TGo4Log::Info("[TQClamDrifttimeCalibStore] Schliesse File %s.",fxFile->GetName());
		fxFile = fxTree->GetCurrentFile(); // wg. File-Split nach 1.8 GB!
		fxFile->cd();
		fxTree->Write(nullptr,TObject::kOverwrite);
		delete fxFile;
		fxFile = nullptr;
		fxTree = nullptr;
	}

	else
	{
		TGo4Log::Error("[TQClamDrifttimeCalibStore] Finde Store-File oder -Tree nicht!");
	}
}





Int_t TQClamDrifttimeCalibStore::Store(TGo4EventElement* event)
{
	if(storepar->fBPauseUserDrifttimeCalibStore) { return 0; }


	GO4TRACE((12,"[TQClamDrifttimeCalibStore] Store.",__LINE__,__FILE__));
	
	//TQClamHitEvent* evt = dynamic_cast<TQClamHitEvent*>(event);
	TQClamClusterEvent* evt = dynamic_cast<TQClamClusterEvent*>(event);
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
		fxTree->Branch("layerID", &b_layerID);
		fxTree->Branch("wireNum", &b_wireNum);
		fxTree->Branch("leading", &b_leading);
		fxTree->Branch("trailing", &b_trailing);
		fxTree->Branch("tdcTrigTime", &b_tdcTrigTime);
		fxTree->Branch("tot", &b_tot);
		fxTree->Branch("drifttime", &b_drifttime);

		fxHitEvent = evt;
		/*
		fxTree->Branch("evennumber",fxHitEvent->fEventNumber);
		fxTree->Branch("hits",&fxHitEvent->hits,32000,0);
		*/
		fbBranchExists = kTRUE;
	}
	else if(fxHitEvent != evt)
	{
		TGo4Log::Info("[TQClamDrifttimeCalibStore] Event pointer changed!");
		return 1; // error, sollte nicht passieren
	}

	/** vektoren leeren */
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


	b_eventNumber = evt->fEvtCounter; //Number;

	b_layerID.clear();
	b_wireNum.clear();
	b_leading.clear();
	b_trailing.clear();
	b_tdcTrigTime.clear();
	b_tot.clear();
	b_drifttime.clear();

	/** hits aus event in vektoren kopieren */
	for(const auto& cl : evt->clusters)
	{
		for(const auto* hit : cl.hits)
		{
			b_layerID.push_back(hit->layerID);
			b_wireNum.push_back(hit->wireNum);
			b_leading.push_back(hit->leading);
			b_trailing.push_back(hit->trailing);
			b_tdcTrigTime.push_back(hit->tdcTrigTime);
			b_tot.push_back(hit->tot);
			b_drifttime.push_back(hit->drifttime);
		}
	}

	fxTree->Fill();
	return 0;
}



