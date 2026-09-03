#ifndef TQCLAMTRACKTESTINGSTORE_H
#define TQCLAMTRACKTESTINGSTORE_H

#include "TGo4EventStore.h"
#include <vector>

class TTree;
class TFile;
class TGo4UserStoreParameter;
class TQClamTrackEvent;
class TQClamStoreParameter;

class TQClamTrackTestingStore : public TGo4EventStore
{
	public:
		TQClamTrackTestingStore();
		TQClamTrackTestingStore(TGo4UserStoreParameter* par);
		virtual ~TQClamTrackTestingStore();

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


		Bool_t b_fBX1 = kFALSE;
		Bool_t b_fBU = kFALSE;
		Bool_t b_fBX2 = kFALSE;


		int b_trackNumber = -1;
		Int_t b_eventNumber = -1;



		bool b_converged = false;
		int b_nConvergedTracks = 0;
		int b_nNotConvergedTracks = 0;

		double b_chi2 = NAN;
		double b_chi2ndf = NAN;
		int b_nIterations = -1;

		int b_nResiduals = -1;
		int b_nHitsUsed = -1;
		int b_nHitsRejected = -1;

		double b_meanResidual = NAN;
		double b_rmsResidual = NAN;
		double b_maxResidualAbs = NAN;

		double b_xD_X1 = NAN;
		double b_xD_U = NAN;
		double b_xD_X2 = NAN;
		double b_yD_U = NAN;
		double b_yD_U_corr = NAN;
		double b_phi = NAN;
		double b_theta = NAN;

		double b_phi_X1 = NAN;
		double b_phi_U = NAN;
		double b_phi_X2 = NAN;

		double b_scatteringTheta = NAN;
		double b_scatteringPhi = NAN;
		double b_scatteringX = NAN;
		double b_scatteringY = NAN;

		double b_focalPlaneX = NAN;
		double b_focalPlaneY = NAN;
		double b_focalPlaneZ = NAN;

		int b_channel = -1;
		double b_energyMeVCali = NAN;

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

		// testen der einzelnen Iterationen 
		std::vector<int> b_iter;
		std::vector<int> b_iter_layerID;
		std::vector<int> b_iter_wireNumber;
		std::vector<double> b_iter_residual;
		std::vector<double> b_iter_phi;

		// testen der outlier-rejection
		std::vector<int> b_outlier_iteration;
		std::vector<int> b_outlier_layerID;
		std::vector<int> b_outlier_wireNumber;
		std::vector<int> b_outlier_hitIndex;

		std::vector<double> b_outlier_drifttime;
		std::vector<double> b_outlier_tot;
		std::vector<double> b_outlier_normalCoord;

		std::vector<double> b_outlier_measuredZOffset;
		std::vector<double> b_outlier_sigmaMeasuredZOffset;

		std::vector<double> b_outlier_trackDistance;

		std::vector<double> b_outlier_residual;
		std::vector<double> b_outlier_pull;

		std::vector<double> b_outlier_phi_global;
		std::vector<double> b_outlier_phi_plane;
		std::vector<double> b_outlier_theta_global;

		std::vector<double> b_outlier_intersection;


	ClassDefOverride(TQClamTrackTestingStore,1);
};


#endif
