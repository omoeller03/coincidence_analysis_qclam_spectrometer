#include "fitmodels/TQClamFitModelTail1OverX.h"

#include "TGo4FitParameter.h"


ClassImp(TQClamFitModelTail1OverX)


TQClamFitModelTail1OverX::TQClamFitModelTail1OverX(const char* name)
	: TGo4FitModel(name,"Tail1OverXModel",kFALSE)
{
	AddPar(new TGo4FitParameter("A","Amplitude", 10., 1., 1e9, 1000));
	AddPar(new TGo4FitParameter("x0","X0", 50., -100., 200., 10));
	AddPar(new TGo4FitParameter("eps","Epsilon", 1., 0., 100., 1.));
}

Bool_t TQClamFitModelTail1OverX::BeforeEval(Int_t)
{
	fA = GetParValue("A");
	fX0 = GetParValue("x0");
	fEps = GetParValue("eps");
	return kTRUE;
}

Double_t TQClamFitModelTail1OverX::EvalN(const Double_t* v)
{
	double x = v[0];
	if (x <= fX0) { return 0.; }
	return fA / (x - fX0 + fEps);
}


