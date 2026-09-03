#ifndef TQCLAMDAGOSTORE_H
#define TQCLAMDAGOSTORE_H

#include "TGo4EventStore.h"
#include <vector>

class TTree;
class TFile;
class TGo4UserStoreParameter;
class TQClamDagobertCoincidenceEvent;
class TQClamStoreParameter;

class TQClamDagoStore : public TGo4EventStore
{
	public:
		TQClamDagoStore();
		TQClamDagoStore(TGo4UserStoreParameter* par);
		virtual ~TQClamDagoStore();

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
		TQClamDagobertCoincidenceEvent* fxCoincEvent{nullptr}; //! - aktuelles event
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


		// scaler
                int b_scalerID = 0;

                double b_scaler_timeMicroSeconds = 0.0;

                uint64_t b_scaler_tdcTrigger = 0;
                uint64_t b_scaler_szintillatorPmtUp = 0;
                uint64_t b_scaler_szintillatorPmtDown = 0;
                uint64_t b_scaler_cherenkovPmt = 0;
                uint64_t b_scaler_coincidenceTrigger = 0;
                uint64_t b_scaler_acceptedTrigger = 0;
                uint64_t b_scaler_clockCounter = 0;
                uint64_t b_scaler_livetimeCounter = 0;
                uint64_t b_scaler_mbsLivetimeCounter = 0;
                uint64_t b_scaler_faradayCupChargeCounter = 0;

                uint64_t b_scaler_errorTdcTrigger = 0;
                uint64_t b_scaler_errorSzintillatorPmtUp = 0;
                uint64_t b_scaler_errorSzintillatorPmtDown = 0;
                uint64_t b_scaler_errorCherenkovPmt = 0;
                uint64_t b_scaler_errorCoincidenceTrigger = 0;
                uint64_t b_scaler_errorAcceptedTrigger = 0;
                uint64_t b_scaler_errorClockCounter = 0;
                uint64_t b_scaler_errorLivetimeCounter = 0;
                uint64_t b_scaler_errorMbsLivetimeCounter = 0;
                uint64_t b_scaler_errorFaradayCupChargeCounter = 0;



		// trigger-detectors
                double b_timeSzintDown = 0.;
                double b_timeSzintUp = 0.;
                double b_timeCherenkov = 0.;



		double b_phi_deg{NAN};
		double b_theta_deg{NAN};
		double b_phi_deg_corr{NAN};

		double b_xD_X1{NAN};
		double b_xD_U{NAN};
		double b_xD_X2{NAN};
		double b_uD_U{NAN};
		double b_yD_U{NAN};

		double b_xD_X1_corr{NAN};
		double b_yD_U_corr{NAN};

		double b_el_channel{NAN};
		double b_el_energy_MeV{NAN};

		double b_scattX{NAN};
		double b_scattY{NAN};
		double b_scattPhi{NAN};
		double b_scattTheta{NAN};

		double b_rateENTON{NAN};
		double b_rateSzintScatt{NAN};
		double b_rateLaBrOR{NAN};
		double b_rateLaBrORNoDead{NAN};

		double b_coincTimeLaBr{NAN};
		double b_szintPmtUpTime{NAN};
		double b_szintPmtDownTime{NAN};
		double b_meanSzintTime{NAN};

		std::vector<double> b_energy_LaBr{NAN};
		std::vector<double> b_calibrated_energy_LaBr{NAN};
		std::vector<double> b_time_LaBr{NAN};
		std::vector<double> b_dtToMeanSzintTime_LaBr{NAN};
		std::vector<double> b_dtToLaBrCoincTime_LaBr{NAN};
		std::vector<bool> b_pileup_LaBr{false};
		std::vector<double> b_rateRaw_LaBr{NAN};
		std::vector<double> b_rateAccepted_LaBr{NAN};
		std::vector<double> b_deadtime_LaBr{NAN};
		std::vector<double> b_timewalk_LaBr{NAN};
		std::vector<double> b_dtToMeanTime_Corr_LaBr{NAN};


		double b_tof{NAN};

	ClassDefOverride(TQClamDagoStore,1);
};


#endif
