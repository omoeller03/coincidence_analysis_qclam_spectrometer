#ifndef TQCLAMDRIFTLENGTHCALIBSTORE_H
#define TQCLAMDRIFTLENGTHCALIBSTORE_H

#include "TGo4EventStore.h"
#include <vector>

class TTree;
class TFile;
class TGo4UserStoreParameter;
class TQClamTrackEvent;
class TQClamStoreParameter;

class TQClamDriftlengthCalibStore : public TGo4EventStore
{
	public:
		TQClamDriftlengthCalibStore();
		TQClamDriftlengthCalibStore(TGo4UserStoreParameter* par);
		virtual ~TQClamDriftlengthCalibStore();

		/** speicher daten aus eventelement */
		Int_t Store(TGo4EventElement* event) override;

		/** ignoriere das speichern von parameter etc. */
		Int_t Store(TGo4Parameter* ) override { return 0; }
		Int_t Store(TGo4Condition* ) override { return 0; }
		Int_t Store(TGo4Fitter* ) override { return 0; }
		Int_t Store(TFolder* ) override { return 0; }

		/** Zugriff auf Tree fuer Framework ermoeglichen */
		TTree* GetTree() override { return fxTree; }

	private:
		TQClamStoreParameter* storepar{nullptr};

		TFile* fxFile{nullptr}; // - file
		TTree* fxTree{nullptr}; // - tree
		TQClamTrackEvent* fxTrackEvent{nullptr}; //! - aktuelles event
		Bool_t fbBranchExists{kFALSE}; //! - branch erzeugt?


		// header
		std::string b_file_name;
		int b_run_number = -1;
		int b_file_number = -1;

		std::string b_user;
		std::string b_time;
		std::string b_label;

		uint32_t b_mbs_event_count = 0;
		uint32_t b_mbs_trigger = 0;
		uint32_t b_mbs_buffer_number = 0;
		uint32_t b_mbs_evts_in_buffer = 0;
		uint32_t b_mbs_time_sec = 0;
		uint32_t b_mbs_time_subsec = 0;




		int b_trackNumber = -1;
		Int_t b_eventNumber = -1;

		Bool_t b_fBX1 = kFALSE;
		Bool_t b_fBU = kFALSE;
		Bool_t b_fBX2 = kFALSE;


		bool b_converged = false;



		double b_xD_X1 = NAN;
		double b_xD_U = NAN;
		double b_xD_X2 = NAN;
		double b_phi = NAN;
		double b_theta = NAN;

		std::vector<double> b_drifttimes;
		std::vector<double> b_tot;

		std::vector<int> b_res_layerID;
		std::vector<int> b_res_wireNumber;
		std::vector<double> b_residual;
		std::vector<double> b_pull;

	ClassDefOverride(TQClamDriftlengthCalibStore,1);
};


#endif
