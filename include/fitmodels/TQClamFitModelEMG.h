#ifndef TQCLAMFITMODELEMG_H
#define TQCLAMFITMODELEMG_H

#include "TGo4FitModel.h"



class TQClamFitModelEMG : public TGo4FitModel
{
	public:
		TQClamFitModelEMG(const char* name="EMG");

		Bool_t BeforeEval(Int_t ndim) override;
		Double_t EvalN(const Double_t* v) override;

	protected:
		Int_t GetPosParIndex(Int_t) override { return 2; }   // mu
		Int_t GetWidthParIndex(Int_t) override { return 3; } // sigma

	private:
		Double_t fA, fLambda, fMu, fSigma;

	ClassDefOverride(TQClamFitModelEMG,1);
};



#endif
