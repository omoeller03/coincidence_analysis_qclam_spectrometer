#include "parameter/TDagobertCalibrationParameter.h"

#include "TGo4Log.h"
#include <cmath>



ClassImp(TDagobertCalibrationParameter)




//////////////////////////////////////////////////////////////////////

TDagobertCalibrationParameter::TDagobertCalibrationParameter()
	: TGo4Parameter("DagoCaliParam")
{}


//////////////////////////////////////////////////////////////////////

TDagobertCalibrationParameter::TDagobertCalibrationParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TDagobertCalibrationParameter] Create %s.",name);

	fDoToFCorr = kFALSE;
	fDoTimeWalkCorr = kFALSE;
	fDoRateCorr = kFALSE;

	for(size_t n=0; n<N_DETECTORS; n++)
	{
		minCoincTimeWin[n] = -5.;
		maxCoincTimeWin[n] = +5.;

		a[n] = 0.;
		b[n] = 0.;
		c[n] = 0.;
		d[n] = 0.;
		e[n] = 0.;
		f[n] = 0.;

		rate_p0[n] = 0;
		rate_p1[n] = 0;
		rate_p2[n] = 0;
		rate_p3[n] = 0;

		timewalk_c[n] = 0;
		timewalk_E1[n] = 0;
		timewalk_E2[n] = 0;
	}

	tof_c = 0;
	tof_x1 = 0;
	tof_phi1 = 0;
	tof_phi2 = 0;
	tof_x1phi1 = 0;
	tof_x2 = 0;
	tof_phi3 = 0;
	tof_x1phi2 = 0;
	tof_x2phi1 = 0;
	tof_x3 = 0;
	tof_phi4 = 0;
	tof_x2phi2 = 0;
}




/** wende Kalibrierung an fuer Kanal channel in Detektor n */
double TDagobertCalibrationParameter::CalibrateEnergy(/*int*/ double channel, int n) const
{
	const double ch = channel;
	return (((((f[n]*ch + e[n])*ch + d[n])*ch + c[n])*ch + b[n])*ch + a[n]);
}


/** wende Raten-Korrektur der Energie an */
void TDagobertCalibrationParameter::CorrectRateDependence(int detector, double E_raw, double rate, double& E_corr)
{
	if(!fDoRateCorr)
	{
		E_corr = E_raw;
		return;
	}
	if(rate <= 0.0)
	{
		E_corr = E_raw; // NAN;
		return;
	}

	/**
	 * Corr-Type 1
	 */
	E_corr =  (1. + rate_p0[detector]*rate) * rate_p1[detector] * E_raw;

	/*
	 * Corr-Type 2
	const double denominator =
		std::pow(rate, rate_p0[detector])
		+ rate_p1[detector]
		* std::pow(rate, rate_p2[detector]);

	if(denominator == 0.0)
	{
		E_corr = E_raw;
		return;
	}

	E_corr = 
		(1.0 + 1.0 /denominator)
		* rate_p3[detector]
		* E_raw;
	*/
		/*
		(1 + 1 / (std::pow(rate,rate_p0[detector]) + rate_p1[detector]*std::pow(rate,rate_p2[detector])) )
		* rate_p3[detector] 
		* E_raw;
		* */
}

/** wende time-walk korrektur an */
void TDagobertCalibrationParameter::CorrectTimeWalk(int detector, double E, double& t_corr)
{
	t_corr = timewalk_c[detector] + timewalk_E1[detector]*E + timewalk_E2[detector]*E*E;
}

/** wende tof-korrektur an */
void TDagobertCalibrationParameter::CorrectToF(double x, double phi, double& tof)
{
	tof = tof_c
		+ tof_x1*x + tof_phi1*phi
		+ tof_phi2*phi*phi + tof_x1phi1*x*phi + tof_x2*x*x
		+ tof_phi3*phi*phi*phi + tof_x1phi2*x*phi*phi + tof_x2phi1*x*x*phi + tof_x3*x*x*x
		+ tof_phi4*phi*phi*phi*phi + tof_x2phi2*x*x*phi*phi;
}


void TDagobertCalibrationParameter::CorrectLaBrTimeTotal(double tof, double timewalk, double& t)
{
	if(fDoTimeWalkCorr && !std::isnan(timewalk)) { t = t - timewalk; }
	if(fDoToFCorr && !std::isnan(tof)) { t = t - tof; }
}





Bool_t TDagobertCalibrationParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TDagobertCalibrationParameter] UpdateFrom().");

	auto* other = static_cast<TDagobertCalibrationParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TDagobertCalibrationParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	fDoToFCorr = other->fDoToFCorr;
	fDoTimeWalkCorr = other->fDoTimeWalkCorr;
	fDoRateCorr = other->fDoRateCorr;

	for(size_t n=0; n<N_DETECTORS; n++)
	{
		minCoincTimeWin[n] = other->minCoincTimeWin[n];
		maxCoincTimeWin[n] = other->maxCoincTimeWin[n];

		a[n] = other->a[n];
		b[n] = other->b[n];
		c[n] = other->c[n];
		d[n] = other->d[n];
		e[n] = other->e[n];
		f[n] = other->f[n];

		rate_p0[n] = other->rate_p0[n];
		rate_p1[n] = other->rate_p1[n];
		rate_p2[n] = other->rate_p2[n];
		rate_p3[n] = other->rate_p3[n];

		timewalk_c[n] = other->timewalk_c[n];
		timewalk_E1[n] = other->timewalk_E1[n];
		timewalk_E2[n] = other->timewalk_E2[n];
	}


	tof_c = other->tof_c;
	tof_x1 = other->tof_x1;
	tof_phi1 = other->tof_phi1;
	tof_phi2 = other->tof_phi2;
	tof_x1phi1 = other->tof_x1phi1;
	tof_x2 = other->tof_x2;
	tof_phi3 = other->tof_phi3;
	tof_x1phi2 = other->tof_x1phi2;
	tof_x2phi1 = other->tof_x2phi1;
	tof_x3 = other->tof_x3;
	tof_phi4 = other->tof_phi4;
	tof_x2phi2 = other->tof_x2phi2;


	TGo4Log::Info("[TDagobertCalibrationParameter] UpdateFrom() erfolgreich.");

	return kTRUE;
}
