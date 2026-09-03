#ifndef TQCLAMDRIFTTIMECALIBSTORE_H
#define TQCLAMDRIFTTIMECALIBSTORE_H

#include "TGo4EventStore.h"
#include <vector>

class TTree;
class TFile;
class TGo4UserStoreParameter;
class TQClamHitEvent;
class TQClamClusterEvent;
class TQClamStoreParameter;

class TQClamDrifttimeCalibStore : public TGo4EventStore
{
	public:
		TQClamDrifttimeCalibStore();
		TQClamDrifttimeCalibStore(TGo4UserStoreParameter* par);
		virtual ~TQClamDrifttimeCalibStore();

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
		TQClamClusterEvent* fxHitEvent{nullptr}; //! - aktuelles event
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


		Int_t b_eventNumber;

		std::vector<int> b_layerID;
		std::vector<int> b_wireNum;
		std::vector<double> b_leading;
		std::vector<double> b_trailing;
		std::vector<double> b_tdcTrigTime;
		std::vector<double> b_tot;
		std::vector<double> b_drifttime;


	ClassDefOverride(TQClamDrifttimeCalibStore,1);
};


#endif
