#ifndef TQCLAMSIEVESLITCALIBPARAMETER_H
#define TQCLAMSIEVESLITCALIBPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"

class TQClamSieveSlitCalibParameter : public TGo4Parameter
{
	public:
		TQClamSieveSlitCalibParameter();
		TQClamSieveSlitCalibParameter(const char* name);


		// map: (x^i y^j phi^k) -> (scatt_theta scatt_phi)



		/// USE DEFAULT FROM BSc jbuschinger
		Bool_t fBUseDefaultSieveSlitCalib = kFALSE; // nutze Sieve-Slit Kalibration (Abbildungsmatrix) aus BSc JBuschinger


		Double_t PH_000 = 23.6;
		Double_t PH_100 = 0.00566;
		Double_t PH_010 = -0.00389;
		Double_t PH_001 = -0.905;
		Double_t PH_110 = 2.21e-6;
		Double_t PH_101 = 4.12e-5;
		Double_t PH_011 = 3.97e-4;
		Double_t PH_111 = -8.21e-8;
		Double_t PH_200 = -7.51e-7;
		Double_t PH_020 = 3.13e-4;
		Double_t PH_002 = 0.0100;
		Double_t PH_210 = -2.26e-9;
		Double_t PH_201 = 5.06e-8;
		Double_t PH_120 = -5.98e-8;
		Double_t PH_021 = -1.77e-5;
		Double_t PH_102 = -2.82e-6;
		Double_t PH_012 = -2.84e-6;
		Double_t PH_220 = -1.36e-10;
		Double_t PH_202 = 1.24e-9;
		Double_t PH_022 = 3.41e-7;
		Double_t PH_300 = 8.82e-10;
		Double_t PH_030 = -1.29e-6;
		Double_t PH_003 = -5.90e-5;



		Double_t TH_000 = -3.60;
		Double_t TH_100 = 0.00199;
		Double_t TH_010 = 0.0654;
		Double_t TH_001 = 0.181;
		Double_t TH_110 = -1.50e-4;
		Double_t TH_101 = -1.31e-4;
		Double_t TH_011 = 0.00391;
		Double_t TH_111 = 2.68e-6;
		Double_t TH_200 = 3.42e-7;
		Double_t TH_020 = 5.14e-5;
		Double_t TH_002 = -0.00179;
		Double_t TH_210 = 8.86e-9;
		Double_t TH_201 = -3.03e-8;
		Double_t TH_120 = 3.20e-7;
		Double_t TH_021 = -6.74e-6;
		Double_t TH_102 = 1.90e-6;
		Double_t TH_012 = -5.70e-5;
		Double_t TH_220 = 1.18e-10;
		Double_t TH_202 = 0.;
		Double_t TH_022 = 2.71e-8;
		Double_t TH_300 = 0.;
		Double_t TH_030 = -9.95e-6;
		Double_t TH_003 = 0.;


		void TransformToScattCoords(double x, double y, double phi, double& phi_scatt, double& theta_scatt) const;

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


	ClassDefOverride(TQClamSieveSlitCalibParameter,1);
};


#endif
