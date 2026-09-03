#ifndef TQCLAMSIEVESLITCALIBPARAMETER_H
#define TQCLAMSIEVESLITCALIBPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"

class TQClamSieveSlitCalibParameter : public TGo4Parameter
{
	public:

		TQClamSieveSlitCalibParameter();
		TQClamSieveSlitCalibParameter(const char* name);

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

		/// Mapping:
		/// (x, y, phi_det)  --> (theta_scatt, phi_scatt)
		void TransformToScattCoords(
				double x,
				double y,
				double phi,
				double& phi_scatt,
				double& theta_scatt) const;

		/// Mapping:
		/// (x, y, phi_det) --> (x_target, y_target)
		void TransformToTargetCoords(
				double x,
				double y,
				double phi,
				double& xTarget,
				double& yTarget) const;

		////////////////////////////////////////////////////////////////
		// Use default calibration
		////////////////////////////////////////////////////////////////

		Bool_t fBUseDefaultSieveSlitCalib = kFALSE;

		////////////////////////////////////////////////////////////////
		// Number of polynomial coefficients
		////////////////////////////////////////////////////////////////

		static constexpr Int_t kThetaPar = 20;
		static constexpr Int_t kPhiPar   = 23;

		////////////////////////////////////////////////////////////////
		// Polynomial coefficients
		////////////////////////////////////////////////////////////////

		/// theta(x,y,phi)
		Double_t TH[kThetaPar];

		/// phi(x,y,phi)
		Double_t PH[kPhiPar];

		/// xTarget(x,y,phi)
		Double_t TX[kThetaPar];

		/// yTarget(x,y,phi)
		Double_t TY[kPhiPar];

	private:

		////////////////////////////////////////////////////////////////
		// Internal polynomial evaluation
		////////////////////////////////////////////////////////////////

		Double_t EvalThetaPolynomial(
				const Double_t* p,
				double x,
				double y,
				double phi) const;

		Double_t EvalPhiPolynomial(
				const Double_t* p,
				double x,
				double y,
				double phi) const;

		////////////////////////////////////////////////////////////////
		// Load default calibration coefficients
		////////////////////////////////////////////////////////////////

		void LoadDefaultCalibration();

		////////////////////////////////////////////////////////////////

		ClassDefOverride(TQClamSieveSlitCalibParameter,1);
};

#endif
