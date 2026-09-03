#ifndef TQCLAMFITMODELTAIL1OVERV_H
#define TQCLAMFITMODELTAIL1OVERV_H

#include "TGo4FitModel.h"


class TQClamFitModelTail1OverX : public TGo4FitModel
{
	public:
		TQClamFitModelTail1OverX(const char* name="Tail1OverX");

		Bool_t BeforeEval(Int_t ndim) override;
		Double_t EvalN(const Double_t* v) override;

	private:
		Double_t fA{0};
		Double_t fX0{0};
		Double_t fEps{0};

	ClassDefOverride(TQClamFitModelTail1OverX,1);
};

#endif
