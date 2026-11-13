#ifndef TQCLAMTRACKEVENT_H
#define TQCLAMTRACKEVENT_H

#include "TGo4EventElement.h"

#include "structs/TQClamMbsHeader.h"
#include "structs/TQClamScaler.h"
#include "structs/TQClamTriggerDetectors.h"

#include "TVector3.h"
#include <vector>

struct TQClamCluster;

namespace genfit
{
	class Track;
}





/*
struct OutlierHitEntry
{
    int layerID = -1;
    int wireNumber = -1;
    int hitIndex = -1;

    double drifttime = NAN;
    double tot = NAN;

    double normalCoord = NAN;

    double measuredZOffset = NAN;
    double sigmaMeasuredZOffset = NAN;

    double trackDistance = NAN;

    double residual = NAN;
    double pull = NAN;

    double phi_global = NAN;
    double phi_plane = NAN;
    double theta_global = NAN;

    int iteration = -1;
};


struct OutlierRejectionSnapshot
{
	int iteration = -1;

	int layerID = -1;

	double phi_global = NAN;
	double theta_global = NAN;
	double phi_plane = NAN;

	double intersection = NAN;

	std::vector<OutlierHitEntry> hits;
};
*/

struct OutlierHitEntry
{
	int layerID = -1;
	int wireNumber = -1;
	int hitIndex = -1;

	double drifttime = NAN;
	double tot = NAN;

	double normalCoord = NAN;

	double measuredZOffset = NAN;
	double sigmaMeasuredZOffset = NAN;

	double trackDistance = NAN;

	double residual = NAN;
	double pull = NAN;

	double phi_global = NAN;
	double phi_plane = NAN;
	double theta_global = NAN;

	int iteration = -1;
};


struct OutlierRejectionSnapshot
{
	int iteration = -1;

	int layerID = -1;

	double phi_global = NAN;
	double theta_global = NAN;
	double phi_plane = NAN;

	double intersection = NAN;

	std::vector<OutlierHitEntry> hits;
};






struct ResidualEntry
{
	int layerID = -1;
	int wireNumber = -1;

	double residual = NAN;
	double sigma_residual = NAN;

	double pull = NAN; // residual/sigma_residual

	double drifttime = NAN;
	double tot = NAN;
	double z_measured = NAN; // gemessener zOffset: map (driftzeit,phi) --> zOffset aus Tabelle
	double z_track = NAN; // zOffset von Track-Fit
	double sigma_z_measured = NAN;
	double sigma_z_track = NAN;

	double phi = NAN;
};


struct IterationSummary
{
	int iteration = 0;

	double phi = 0.;
	double sigmaReg_phi = NAN;
	double theta = 0.;
	double sigmaReg_theta = NAN;

	double dPhi = NAN;
	double rmsResidual = NAN;

	double chi2 = 0.;
	double chi2ndf = 0.;

	double meanResidual = 0.;
	double maxResidualAbs = 0.;
};

struct ResidualIterationEntry
{
	int iteration = 0;
	int layerID = 0;
	int wireNumber = 0;

	double residual = 0.;
	double phi = 0.;
};



/**
 * Residuen ueber Iteration
 * TODO: bisher nur angelegt, wir noch nicht gefuellt oder verwendet
 */
struct WireResidualSummary
{
	int layerID = -1;
	int wireNumber = -1;

	std::vector<double> residuals;
	std::vector<double> pulls;
	std::vector<double> iterations;
};



class TQClamTrackEvent : public TGo4EventElement
{
	public:
		TQClamTrackEvent() {}
		TQClamTrackEvent(const char* name)
			: TGo4EventElement(name)
		{}

		virtual ~TQClamTrackEvent() {}


		

		void Clear(Option_t* opt="") override;


		/** parameter */
		MbsHeader header; //! nicht streamen
		TQClamScaler scaler; //! nicht streamen
		TQClamTriggerDetectors trigDet; //! nicht streamen

		int fEvtCounter = 0;
		Int_t fEvtNumber = 0;
		int fConvergedTrackCounter = 0;
		int fNotConvergedTrackCounter = 0;

		bool trackAccepted = false;
		// bzw. converged = false;

		std::vector<const TQClamCluster*> clusters; //! /** TODO evtl entfernen */

		/**
		 * resolution = sqrt( 1 / N sum_i^N r_i²), r_i = residuum
		 */
		Double_t zResolution_X1 = NAN;
		Double_t zResolution_U = NAN;
		Double_t zResolution_X2 = NAN;

		Double_t xResolution_X1 = NAN;
		Double_t xResolution_U = NAN;
		Double_t xResolution_X2 = NAN;

		Double_t uResolution_U = NAN;
		Double_t yResolution_U = NAN;

		Double_t phiResolution = NAN;
		//Double_t thetaResolution = NAN; kann nicht berechnet werden.. kein phiUResolution

		Bool_t fBX1 = kFALSE;
		Bool_t fBU = kFALSE;
		Bool_t fBX2 = kFALSE;

		/** parameter in X1-Ebene */
		TVector3 posAtX1 = {0,0,0};
		TVector3 dirAtX1 = {0,0,0};
		double phi = NAN; /** dispersiv */
		double phi_deg = NAN; /** dispersiv */
		double theta = NAN; /** nicht-dispersiv */
		double theta_deg = NAN; /** nicht-dispersiv */
		double sigmaReg_theta = NAN; /** aus regression propagierte unsicherheit */

		double phi_X1 = NAN;
		double phi_U = NAN;
		double phi_X2 = NAN;

		/** unsicherheit aus regression */
		double sigmaReg_phi_X1 = NAN;
		double sigmaReg_phi_U = NAN;
		double sigmaReg_phi_X2 = NAN;

		/** Durchstosspkte X1 & X2 */
		double xD_X1 = NAN;
		double xD_U = NAN;
		double xD_X2 = NAN;
		double uD = NAN;
		double yD_X1 = NAN;
		double yD_U = NAN;
		double yD_U_corr = NAN;

		/** unsicherheiten aus regression */
		double sigmaReg_xD_X1 = NAN;
		double sigmaReg_uD_U = NAN;
		double sigmaReg_xD_X2 = NAN;
		double sigmaReg_xD_U = NAN;
		double sigmaReg_yD_U = NAN;

		/** X-Phi korrigierte werte */
		double phi_corr = NAN;
		double sigma_phi_corr = NAN; // nicht implementiert (gleichsetzen mit sigmaReg_phi???)
		double x_corr = NAN;
		double sigma_x_corr = NAN; // nicht implementiert

		/** unsicherheiten aus regression */
		double sigmaReg_phi = NAN;
		
		/** Energiekalibrierung der X-Phi korrigierten Daten */
		double e_MeV = NAN;
		double sigma_e_MeV = NAN; // nicht implementiert 



		/** parameter projiziert in Fokalebene */

		/** Projektionsparameter */
		TVector3 proj_position{0,0,0};

		double focalPlaneX = 0.;
		double focalPlaneY = 0.;
		double focalPlaneZ = 0.;

		int channel = 0;
		bool energyValid = false;
		double energyMeVCali = NAN;
		double energyMaVRaw = NAN;

		/** streukoordinatne / -winkel */
		double scatteringTheta = NAN;
		double scatteringPhi = NAN;
		double scatteringX = NAN;
		double scatteringY = NAN;

		/** Aufloesung streukoordinaten aus Residuenaufloesung */
		double xTargetResolution = NAN;
		double yTargetResolution = NAN;
		double phiTargetResolution = NAN;
		double thetaTargetResolution = NAN;


		/** Aufloesung streukoordinaten aus Regressionsunsicherheit */
		double sigmaReg_xTarget = NAN;
		double sigmaReg_yTarget = NAN;
		double sigmaReg_phiTarget = NAN;
		double sigmaReg_thetaTarget = NAN;


		/** Fit-Parameter? */
		double chi2 = NAN;
		double chi2ndf = NAN;
		int nIterations = 0;

		int nResiduals = 0;
		int nHitsUsed = 0;
		int nHitsRejected = 0;

		double meanResidual = NAN;
		double rmsResidual = NAN;
		double maxResidualAbs = NAN;

		double meanResidual_X1 = NAN;
		double meanResidual_U = NAN;
		double meanResidual_X2 = NAN;

		double sigmaResidual_X1 = NAN;
		double sigmaResidual_U = NAN;
		double sigmaResidual_X2 = NAN;

		double rmsResidual_X1 = NAN;
		double rmsResidual_U = NAN;
		double rmsResidual_X2 = NAN;

		int nResiduals_X1 = 0;
		int nResiduals_U = 0;
		int nResiduals_X2 = 0;



		std::vector<ResidualEntry> residualEntries;
		std::vector<ResidualIterationEntry> residualIterationEntries;
		std::vector<IterationSummary> iterationSummaries;

		std::vector<OutlierRejectionSnapshot> outlierRejectionSnapshots;



//		std::vector<double> residuals;





	ClassDefOverride(TQClamTrackEvent,1);
};


#endif
