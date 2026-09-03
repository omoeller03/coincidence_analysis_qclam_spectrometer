#ifndef TQCLAMFITMODELSPECTRUM_H
#define TQCLAMFITMODELSPECTRUM_H


#include "TGo4FitModel.h"


class TQClamFitModelSpectrum : public TGo4FitModel
{
	public:
		TQClamFitModelSpectrum(const char* name = "Spectrum");

		Bool_t BeforeEval(Int_t ndim) override;
		Double_t EvalN(const Double_t* v) override;

	protected:
		Int_t GetPosParIndex(Int_t) override { return 1; } // x0
		Int_t GetWidthParIndex(Int_t) override { return 2; } // sigma1

	private:
		Double_t fC1{0.};
		Double_t fX0{0.};
		Double_t fSigma1{0.};
		Double_t fC2{0.};
		Double_t fEta{0.};
		Double_t fDelta{0.};
		

	ClassDefOverride(TQClamFitModelSpectrum,1);
};


#endif
