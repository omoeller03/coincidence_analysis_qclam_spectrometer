#ifndef TQCLAMFITMODELFERMIDIRAC_H
#define TQCLAMFITMODELFERMIDIRAC_H

/**
 * fit an dei ansteigende flanke der Driftzeitspektren, um zeitoffsets zu bestimmen */

#include "TGo4FitModel.h"

class TQClamFitModelFermiDirac : public TGo4FitModel
{
	public:
		TQClamFitModelFermiDirac();
		TQClamFitModelFermiDirac(const char* name);

		Bool_t BeforeEval(Int_t ndim) override;
		Double_t EvalN(const Double_t* v) override;

	private:
		Double_t fA{0}; /** amplitude (normierung) */
		Double_t fT{0}; /** 'fermi-temperatur' */
		Double_t fTau{0}; /** Abstand t0 zu Mitte Flanke */
		Double_t fT0{0}; /** Position t0: Start Flanke */

	ClassDefOverride(TQClamFitModelFermiDirac,1);
};

#endif
