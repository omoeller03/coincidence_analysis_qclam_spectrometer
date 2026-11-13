#include "evt/TQClamTrackEvent.h"

#include "evt/TQClamClusterEvent.h"

#include "TVector3.h"




ClassImp(TQClamTrackEvent)

void TQClamTrackEvent::Clear(Option_t* opt)
{
	TGo4EventElement::Clear(opt);

	header.file_name = "";
	header.run_number = -1;
	header.file_number = -1;

	header.user = "";
	header.time = "";
	header.label = "";

	header.mbs_event_count = 0;
	header.mbs_trigger = 0;
	
	header.mbs_buffer_number = 0;

	header.mbs_time_sec = 0;
	header.mbs_time_subsec = 0;


	fEvtCounter = 0;
	fConvergedTrackCounter = 0;
	fNotConvergedTrackCounter = 0;
	trackAccepted = false;

	fBX1 = kFALSE;
	fBU = kFALSE;
	fBX2 = kFALSE;

	clusters.clear();
	residualEntries.clear();
	residualIterationEntries.clear();
	iterationSummaries.clear();
	outlierRejectionSnapshots.clear();

//	residuals.clear();

	posAtX1 = {NAN,NAN,NAN};
	dirAtX1 = {NAN,NAN,NAN};

	phi = NAN;
	phi_deg = NAN;
	theta = NAN;
	theta_deg = NAN;
	sigmaReg_theta = NAN;

	xD_X1 = NAN;
	xD_U = NAN;
	xD_X2 = NAN;
	yD_X1 = NAN;
	yD_U = NAN;
	yD_U_corr = NAN;

	/** unsicherheit aus regression */
	sigmaReg_xD_X1 = NAN;
	sigmaReg_uD_U = NAN;
	sigmaReg_xD_X2 = NAN;
	sigmaReg_xD_U = NAN;
	sigmaReg_yD_U = NAN;



	phi_X1 = NAN;
	phi_U = NAN;
	phi_X2 = NAN;

	/** unsicherheit aus regression */
	sigmaReg_phi_X1 = NAN;
	sigmaReg_phi_U = NAN;
	sigmaReg_phi_X2 = NAN;

	sigmaReg_phi = NAN;



	zResolution_X1 = NAN;
	zResolution_U = NAN;
	zResolution_X2 = NAN;

	xResolution_X1 = NAN;
	xResolution_X2 = NAN;

	uResolution_U = NAN;
	yResolution_U = NAN;

	phiResolution = NAN;

	x_corr = NAN;
	sigma_x_corr = NAN;
	phi_corr = NAN;
	sigma_phi_corr = NAN;
	e_MeV = NAN;
	sigma_e_MeV = NAN;

	chi2 = NAN;
	chi2ndf = NAN;

	nResiduals = 0;
	nHitsUsed = 0;
	nHitsRejected = 0;

	meanResidual = NAN;
	rmsResidual = NAN;
	maxResidualAbs = NAN;


	/** Projektion */
	proj_position = {NAN,NAN,NAN};
	focalPlaneX = NAN;
	focalPlaneY = NAN;
	focalPlaneZ = NAN;

	energyValid = false;
	energyMeVCali = NAN;
	energyMaVRaw = NAN;

	scatteringTheta = NAN;
	scatteringPhi = NAN;
	scatteringX = NAN;
	scatteringY = NAN;


	xTargetResolution = NAN;
	yTargetResolution = NAN;
	phiTargetResolution = NAN;
	thetaTargetResolution = NAN;

	sigmaReg_xTarget = NAN;
	sigmaReg_yTarget = NAN;
	sigmaReg_phiTarget = NAN;
	sigmaReg_thetaTarget = NAN;

}
