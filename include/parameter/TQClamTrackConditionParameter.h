#ifndef TQCLAMTRACKCONDITIONPARAMETER_H
#define TQCLAMTRACKCONDITIONPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"
#include "TQClamConstants.h"

enum class EConvergenceCriterion
{
        Phi, // | phi_i - phi_i-1 | < epsilon_phi
        RMS, // rms_residual < epsilon_residual
	PhiAndRMS, // both with AND
	PhiOrRMS // both with OR
};





class TQClamTrackConditionParameter : public TGo4Parameter
{
	public:
		TQClamTrackConditionParameter();
		TQClamTrackConditionParameter(const char* name);

		/** Max Dimensionen */
		static const Int_t MAX_LAYER = QCLAM_MAX_LAYER; // Maximale Anzahl Ebenen: 8


		/** Condition-Modi: Umgang mit Conditions */
		static constexpr Int_t kOff = QCLAM_COND_OFF; // Condition-Mode 0: Condition hat keine Auswirkung
		static constexpr Int_t kFlagOnly = QCLAM_COND_FLAG_ONLY; // Condition-Mode 1: Condition setzt Event-Flag true/false (Default)
		static constexpr Int_t kFilter = QCLAM_COND_FILTER; // Condition-Mode 2: Condition verwirft Event





		Bool_t AllowedLayersForTrack[MAX_LAYER]; // Ebenen, deren Cluster fuer das Tracking verwendet werden duerfen
		Bool_t RequiredLayersForTrack[MAX_LAYER]; // Ebenen, die Cluster zum Track beitragen muessen

		Double_t min_x_X1 = -1e9;
		Double_t max_x_X1 = 1e9;

		Double_t min_phi_deg = 0.;
		Double_t max_phi_deg = 360.;

		Int_t fMinIter = 2; // Minimale Iterations-Anzahl (Tracking)
		Int_t fMaxIter = 50; // Maximale Iterations-Anzahl (Tracking)
		Double_t RelaxationAlpha = 0.3; // Relaxation fuer bessere Konvergenz (0 < alpha <= 1)
		Double_t EpsilonPhi = 1e-5; // Abbruchkriterium Iteration Track-Winkel phi (in rad): |phi_last - phi_new| < EpsilonPhi
		Double_t EpsilonTheta = 1e-3; // Abbruchkriterium Iteration Track-Winkel theta (in rad): |theta_last - theta_new| < EpsilonTheta
		Double_t EpsilonChi2 = 1.e-3; // Abbruchkriterium Chi2
		Double_t EpsilonRMS = 3.; // Abbruchkriterium RMS

		Double_t EpsilonPhiStagnation = 1e-12; // Wenn phi-Aendurung in Iterationsschritten kleiner EpsilonPhiStagnation und Konvergenzkriterium nicht erreicht: verwerfe Track

		Bool_t UseOutlierRejection = kFALSE; // verwerfe schlechte Hits
		Double_t OutlierRejectionLimit = 3.0; // abs(residual) / sigmaResidual < OutlierRejectionLimit --> 3sigma cut!
		Double_t UpperOutlierRejectionLimit = 3.0;

		Int_t fMaxRejectedHitsX1PerIter = 2;
		Int_t fMaxRejectedHitsUPerIter = 2;
		Int_t fMaxRejectedHitsX2PerIter = 2;

		Bool_t ConvergenceCriterion_Phi = kTRUE;
		Bool_t ConvergenceCriterion_RMS = kFALSE;
		Bool_t ConvergenceCriterion_PhiAndRMS = kFALSE;
		Bool_t ConvergenceCriterion_PhiOrRMS = kFALSE;

		EConvergenceCriterion ConvergenceCriterion = EConvergenceCriterion::Phi; // Default


		Bool_t Test_Phi_Deg(double phi) const;

		Bool_t Test_X_X1(double x) const;

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamTrackConditionParameter,1);
};


#endif
