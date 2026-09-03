#include "parameter/TQClamSieveSlitCalibParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamSieveSlitCalibParameter)


TQClamSieveSlitCalibParameter::TQClamSieveSlitCalibParameter()
	: TGo4Parameter("SieveSlitCalibParam")
{

		/// USE DEFAULT FROM BSc jbuschinger
	fBUseDefaultSieveSlitCalib = kTRUE; // nutze Sieve-Slit Kalibration (Abbildungsmatrix) aus BSc JBuschinger


	PH_000 = 23.6;
	PH_100 = 0.00566;
	PH_010 = -0.00389;
	PH_001 = -0.905;
	PH_110 = 2.21e-6;
	PH_101 = 4.12e-5;
	PH_011 = 3.97e-4;
	PH_111 = -8.21e-8;
	PH_200 = -7.51e-7;
	PH_020 = 3.13e-4;
	PH_002 = 0.0100;
	PH_210 = -2.26e-9;
	PH_201 = 5.06e-8;
	PH_120 = -5.98e-8;
	PH_021 = -1.77e-5;
	PH_102 = -2.82e-6;
	PH_012 = -2.84e-6;
	PH_220 = -1.36e-10;
	PH_202 = 1.24e-9;
	PH_022 = 3.41e-7;
	PH_300 = 8.82e-10;
	PH_030 = -1.29e-6;
	PH_003 = -5.90e-5;



	TH_000 = -3.60;
	TH_100 = 0.00199;
	TH_010 = 0.0654;
	TH_001 = 0.181;
	TH_110 = -1.50e-4;
	TH_101 = -1.31e-4;
	TH_011 = 0.00391;
	TH_111 = 2.68e-6;
	TH_200 = 3.42e-7;
	TH_020 = 5.14e-5;
	TH_002 = -0.00179;
	TH_210 = 8.86e-9;
	TH_201 = -3.03e-8;
	TH_120 = 3.20e-7;
	TH_021 = -6.74e-6;
	TH_102 = 1.90e-6;
	TH_012 = -5.70e-5;
	TH_220 = 1.18e-10;
	TH_202 = 0.;
}



TQClamSieveSlitCalibParameter::TQClamSieveSlitCalibParameter(const char* name)
	: TGo4Parameter(name)
{
	
	TGo4Log::Info("[TQClamSieveSlitCalibParameter] Create %s",name);

	/// USE DEFAULT FROM BSc jbuschinger
	fBUseDefaultSieveSlitCalib = kTRUE; // nutze Sieve-Slit Kalibration (Abbildungsmatrix) aus BSc JBuschinger


	PH_000 = 23.6;
	PH_100 = 0.00566;
	PH_010 = -0.00389;
	PH_001 = -0.905;
	PH_110 = 2.21e-6;
	PH_101 = 4.12e-5;
	PH_011 = 3.97e-4;
	PH_111 = -8.21e-8;
	PH_200 = -7.51e-7;
	PH_020 = 3.13e-4;
	PH_002 = 0.0100;
	PH_210 = -2.26e-9;
	PH_201 = 5.06e-8;
	PH_120 = -5.98e-8;
	PH_021 = -1.77e-5;
	PH_102 = -2.82e-6;
	PH_012 = -2.84e-6;
	PH_220 = -1.36e-10;
	PH_202 = 1.24e-9;
	PH_022 = 3.41e-7;
	PH_300 = 8.82e-10;
	PH_030 = -1.29e-6;
	PH_003 = -5.90e-5;



	TH_000 = -3.60;
	TH_100 = 0.00199;
	TH_010 = 0.0654;
	TH_001 = 0.181;
	TH_110 = -1.50e-4;
	TH_101 = -1.31e-4;
	TH_011 = 0.00391;
	TH_111 = 2.68e-6;
	TH_200 = 3.42e-7;
	TH_020 = 5.14e-5;
	TH_002 = -0.00179;
	TH_210 = 8.86e-9;
	TH_201 = -3.03e-8;
	TH_120 = 3.20e-7;
	TH_021 = -6.74e-6;
	TH_102 = 1.90e-6;
	TH_012 = -5.70e-5;
	TH_220 = 1.18e-10;
	TH_202 = 0.;
	TH_022 = 2.71e-8;
	TH_300 = 0.;
	TH_030 = -9.95e-6;
	TH_003 = 0.;
}



Bool_t TQClamSieveSlitCalibParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamSieveSlitCalibParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamSieveSlitCalibParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamSieveSlitCalibParameter] UpdateFrom nicht erfolgreich!");
		return kFALSE;
	}


	PH_000 = other->PH_000;
	PH_100 = other->PH_100;
	PH_010 = other->PH_010;
	PH_001 = other->PH_001;
	PH_110 = other->PH_110;
	PH_101 = other->PH_101;
	PH_011 = other->PH_011;
	PH_111 = other->PH_111;
	PH_200 = other->PH_200;
	PH_020 = other->PH_020;
	PH_002 = other->PH_002;
	PH_210 = other->PH_210;
	PH_201 = other->PH_201;
	PH_120 = other->PH_120;
	PH_021 = other->PH_021;
	PH_102 = other->PH_102;
	PH_012 = other->PH_012;
	PH_220 = other->PH_220;
	PH_202 = other->PH_202;
	PH_022 = other->PH_022;
	PH_300 = other->PH_300;
	PH_030 = other->PH_030;
	PH_003 = other->PH_003;



	TH_000 = other->TH_000;
	TH_100 = other->TH_100;
	TH_010 = other->TH_010;
	TH_001 = other->TH_001;
	TH_110 = other->TH_110;
	TH_101 = other->TH_101;
	TH_011 = other->TH_011;
	TH_111 = other->TH_111;
	TH_200 = other->TH_200;
	TH_020 = other->TH_020;
	TH_002 = other->TH_002;
	TH_210 = other->TH_210;
	TH_201 = other->TH_201;
	TH_120 = other->TH_120;
	TH_021 = other->TH_021;
	TH_102 = other->TH_102;
	TH_012 = other->TH_012;
	TH_220 = other->TH_220;
	TH_202 = other->TH_202;
	TH_022 = other->TH_022;
	TH_300 = other->TH_300;
	TH_030 = other->TH_030;
	TH_003 = other->TH_003;


	return kTRUE;
}




void TQClamSieveSlitCalibParameter::TransformToScattCoords(
		double x,
		double y,
		double phi,
		double& phi_scatt,
		double& theta_scatt) const
{
	if(fBUseDefaultSieveSlitCalib && std::isfinite(y))
	{
		phi_scatt = PH_000 +
					
			x * (PH_100 + x * (PH_200 + x * PH_300)) +
			y * (PH_010 + y * (PH_020 + y * PH_030)) +
			phi * (PH_001 + phi * (PH_002 + phi * PH_003)) +

			x * y * phi * PH_111 +

			x * y * (PH_110 + y * PH_120) +
			x * phi * (PH_101 + phi * PH_102) +
			y * phi * (PH_011 + phi * PH_012) +

			x * x * y * PH_210 +
			x * x * phi * PH_201 +
			y * y * phi * PH_021 +

			x * x * y * y * PH_220 +
			x * x * phi * phi * PH_202 +
			y * y * phi * phi * PH_022;



		theta_scatt = TH_000 +
				
			x * (TH_100 + x * (TH_200 + x * TH_300)) +
			y * (TH_010 + y * (TH_020 + y * TH_030)) +
			phi * (TH_001 + phi * (TH_002 + phi * TH_003)) +

			x * y * phi * TH_111 +
	
			x * y * (TH_110 + y * TH_120) +
			x * phi * (TH_101 + phi * TH_102) +
			y * phi * (TH_011 + phi * TH_012) +

			x * x * y * TH_210 +
			x * x * phi * TH_201 +
			y * y * phi * TH_021 +

			x * x * y * y * TH_220 +
			x * x * phi * phi * TH_202 +
			y * y * phi * phi * TH_022;

	}

	else
	{
		phi_scatt = NAN;
		theta_scatt = NAN;
	}
		
}


