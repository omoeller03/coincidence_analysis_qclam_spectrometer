#ifndef TQCLAMTRACKSTORE_H
#define TQCLAMTRACKSTORE_H

#include "TGo4EventStore.h"
#include <vector>

class TTree;
class TFile;
class TGo4UserStoreParameter;
class TQClamTrackEvent;
class TQClamStoreParameter;

class TQClamTrackStore : public TGo4EventStore
{
	public:
		TQClamTrackStore();
		TQClamTrackStore(TGo4UserStoreParameter* par);
		virtual ~TQClamTrackStore();

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


		double b_timeSzintDown = 0.;
		double b_timeSzintUp = 0.;
		double b_timeCherenkov = 0.;



		int b_trackNumber = -1;
		Int_t b_eventNumber = -1;

		bool b_converged = false;
		int b_nConvergedTracks = 0;
		int b_nNotConvergedTracks = 0;


		Bool_t b_fBX1 = kFALSE;
		Bool_t b_fBU = kFALSE;
		Bool_t b_fBX2 = kFALSE;


		/*
		double b_chi2 = NAN;
		double b_chi2ndf = NAN;
		int b_nIterations = -1;

		int b_nResiduals = -1;
		int b_nHitsUsed = -1;
		int b_nHitsRejected = -1;

		double b_meanResidual = NAN;
		double b_rmsResidual = NAN;
		double b_maxResidualAbs = NAN;
		*/

		double b_xD_X1 = NAN;
		double b_xD_U = NAN;
		double b_xD_X2 = NAN;
		double b_uD_U = NAN;
		double b_yD_U = NAN;
		double b_yD_U_corr = NAN;
		double b_phi = NAN;
		double b_theta = NAN;

		double b_scatteringTheta = NAN;
		double b_scatteringPhi = NAN;
		double b_scatteringX = NAN;
		double b_scatteringY = NAN;

		double b_focalPlaneX = NAN;
		double b_focalPlaneY = NAN;
		double b_focalPlaneZ = NAN;

		int b_channel = -1;
		double b_energyMeVCali = NAN;

		/*
		std::vector<double> b_drifttimes;
		std::vector<double> b_tot;

		std::vector<int> b_res_layerID;
		std::vector<int> b_res_wireNumber;
		std::vector<double> b_residual;
		std::vector<double> b_sigmaResidual;
		std::vector<double> b_pull;
		std::vector<double> b_z_measured;
		std::vector<double> b_z_track;
		std::vector<double> b_sigma_z_measured;
		std::vector<double> b_sigma_z_track;

		std::vector<int> b_iter;
		std::vector<int> b_iter_layerID;
		std::vector<int> b_iter_wireNumber;
		std::vector<double> b_iter_residual;
		std::vector<double> b_iter_phi;
		*/


	ClassDefOverride(TQClamTrackStore,1);
};


#endif
