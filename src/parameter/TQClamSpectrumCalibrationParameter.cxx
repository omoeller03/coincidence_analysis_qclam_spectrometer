#include "parameter/TQClamSpectrumCalibrationParameter.h"

#include "TGo4Log.h"


ClassImp(TQClamSpectrumCalibrationParameter)

//////////////////////////////////////////////////////////////////////

TQClamSpectrumCalibrationParameter::TQClamSpectrumCalibrationParameter()
	: TGo4Parameter("SpectrumCalibParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamSpectrumCalibrationParameter::TQClamSpectrumCalibrationParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamSpectrumCalibrationParameter] Create %s.",name);

	// parameter nullen
	a_phi_corr = 0.;
	b_phi_corr = 0.;

	a_0_0 = 0.;
	a_1_0 = 0.;
	a_0_1 = 0.;
	a_1_1 = 0.;
	a_2_0 = 0.;
	a_2_1 = 0.;
	a_0_2 = 0.;
	a_1_2 = 0.;
	a_2_2 = 0.;

	a_e_corr = 0.;
	b_e_corr = 0.;
	c_e_corr = 0.;
}

//////////////////////////////////////////////////////////////////////

void TQClamSpectrumCalibrationParameter::CorrectPhi(
		double x_raw, // input
		double sigma_x_raw, // input 
		double phi_raw, // input 
		double sigma_phi_raw, // input
		double& phi_corr, // set 
		double& sigma_phi_corr // set
		) const
{
	phi_corr = phi_raw - (a_phi_corr + x_raw * ( b_phi_corr + x_raw * c_phi_corr ) );
	sigma_phi_corr = 0.; // TODO
}

//////////////////////////////////////////////////////////////////////

void TQClamSpectrumCalibrationParameter::CorrectX(
		double x_raw, // input
		double sigma_x_raw, // input
		double phi_corr, // input 
		double sigma_phi_corr, // input
		double& x_corr, // set
		double& sigma_x_corr // set
		) const
{
	double A0_phi = a_0_0 + phi_corr * (a_0_1 + a_0_2 * phi_corr);
	double A1_phi = a_1_0 + phi_corr * (a_1_1 + a_1_2 * phi_corr);
	double A2_phi = a_2_0 + phi_corr * (a_2_1 + a_2_2 * phi_corr);

	x_corr = A0_phi + x_raw * ( A1_phi + x_raw * A2_phi);


	sigma_x_corr = 0.; // TODO
}



void TQClamSpectrumCalibrationParameter::Calibrate(
		double x_corr, // input
		double sigma_x_corr, // input
		double& e_MeV, // set
		double& sigma_e_MeV // set
		) const
{
	e_MeV = /*eS_corr -*/ e0_corr * ( 1 +  a_e_corr + x_corr * ( b_e_corr + x_corr * c_e_corr ) );

	sigma_e_MeV = 0.; // TODO
}


Bool_t TQClamSpectrumCalibrationParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamSpectrumCalibrationParameter] UpdateFrom().");

	auto* other = dynamic_cast<TQClamSpectrumCalibrationParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamSpectrumCalibrationParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	u_org = other->u_org;

	a_phi_corr = other->a_phi_corr;
	b_phi_corr = other->b_phi_corr;

	a_0_0 = other->a_0_0;
	a_1_0 = other->a_1_0;
	a_0_1 = other->a_0_1;
	a_1_1 = other->a_1_1;
	a_2_0 = other->a_2_0;
	a_2_1 = other->a_2_1;
	a_0_2 = other->a_0_2;
	a_1_2 = other->a_1_2;
	a_2_2 = other->a_2_2;

	eS_corr = other->eS_corr;
	e0_corr = other->e0_corr;
	a_e_corr = other->a_e_corr;
	b_e_corr = other->b_e_corr;
	c_e_corr = other->c_e_corr;

	return kTRUE;
}

