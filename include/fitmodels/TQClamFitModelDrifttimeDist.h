#ifndef TQCLAMFITMODELDRIFTTIMEDIST_H
#define TQCLAMFITMODELDRIFTTIMEDIST_H


#include "TGo4FitModel.h"

class TQClamFitModelDrifttimeDist : public TGo4FitModel
{
	public:
		TQClamFitModelDrifttimeDist();
		TQClamFitModelDrifttimeDist(const char* name);

		Double_t N_skewed(double x,double A,double mu,double sigma,double s);
		Bool_t BeforeEval(Int_t ndim) override;
		Double_t EvalN(const Double_t* v) override;

	private:
		Double_t fA0{0}; /** amplitude0 (normierung) */
		Double_t fA1{0}; /** amplitude1 (normierung) */
		Double_t fMean0{0}; /** mittelwert0 */
		Double_t fMean1{0}; /** mittelwert1 */
		Double_t fSigma0{0}; /** stdw0 */
		Double_t fSigma1{0}; /** stdw1 */
		Double_t fS0{0}; /** s0 */
		Double_t fS1{0}; /** s1 */


	ClassDefOverride(TQClamFitModelDrifttimeDist,1);
};

#endif
