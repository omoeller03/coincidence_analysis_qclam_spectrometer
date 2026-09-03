#include "fitmodels/TQClamFitModelEMG.h"

#include "TGo4FitParameter.h"
#include "TMath.h"


ClassImp(TQClamFitModelEMG)




TQClamFitModelEMG::TQClamFitModelEMG(const char* name)
	: TGo4FitModel(name,"EMGModel",kFALSE)
{
	AddPar(new TGo4FitParameter("Aemg","Amplitude", 100., 0., 1e9, 10.));
	AddPar(new TGo4FitParameter("lambda","Lambda", 0.1, 0., 50., 0.01));
	AddPar(new TGo4FitParameter("mu","Mean", 40., 0., 100., 0.1));
	AddPar(new TGo4FitParameter("sigma","Sigma", 0.5, 0.05, 10., 0.01));
}

Bool_t TQClamFitModelEMG::BeforeEval(Int_t)
{
	fA = GetParValue("Aemg");
	fLambda = GetParValue("lambda");
	fMu = GetParValue("mu");
	fSigma = GetParValue("sigma");
	return kTRUE;
}

Double_t TQClamFitModelEMG::EvalN(const Double_t* v)
{
	double x = v[0];
	double t = (fMu + fLambda*fSigma*fSigma - x) / (TMath::Sqrt2()*fSigma);

	return fA * (fLambda/2.0) * TMath::Exp(fLambda/2.0*(2*fMu + fLambda*fSigma*fSigma - 2*x)) * TMath::Erfc(t);
}
