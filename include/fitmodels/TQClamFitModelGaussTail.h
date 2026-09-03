#ifndef TQCLAMFITMODELGAUSSTAIL_H
#define TQCLAMFITMODELGAUSSTAIL_H

#include "TGo4FitModel.h"



class TQClamFitModelGaussTail : public TGo4FitModel
{
	public:
		TQClamFitModelGaussTail(const char* name="GaussTail");

		Bool_t BeforeEval(Int_t ndim) override;
		Double_t EvalN(const Double_t* v) override;

	protected:
		Int_t GetPosParIndex(Int_t) override { return 1; }   // mu
		Int_t GetWidthParIndex(Int_t) override { return 2; } // sigma

	private:
		Double_t fA{0};
		Double_t fMu{0};
		Double_t fSigma{0};

		Double_t fTailA{0};
		Double_t fTailX0{0};
		Double_t fTailEps{0};

	ClassDefOverride(TQClamFitModelGaussTail,1);
};

#endif
