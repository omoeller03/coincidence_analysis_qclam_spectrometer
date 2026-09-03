#include "fitmodels/TQClamFitModelSpectrum.h"

#include "TGo4Log.h"
#include "TGo4FitParameter.h"
#include "TMath.h"


ClassImp(TQClamFitModelSpectrum)


TQClamFitModelSpectrum::TQClamFitModelSpectrum(const char* name)
	: TGo4FitModel(name, "SpectrumModel", kFALSE)
{
	AddPar(new TGo4FitParameter("c1","Left Amplitude",1000.,0.,1e9,10.));
	AddPar(new TGo4FitParameter("x0","PeakPosition",5.,0.,100.,0.01));
	AddPar(new TGo4FitParameter("sigma1","Left Sigma",0.2,0.001,10.,0.001));
	AddPar(new TGo4FitParameter("c2","Right Amplitude",800.,0.,1e9,10.));
	AddPar(new TGo4FitParameter("eta","TransitionPoint",2.,0.01,20.,0.01));
	AddPar(new TGo4FitParameter("delta","Tail Exponent",3.,0.1,50.,0.01));
}

Bool_t TQClamFitModelSpectrum::BeforeEval(Int_t)
{
	fC1 = GetParValue("c1");
	fX0 = GetParValue("x0");
	fSigma1 = GetParValue("sigma1");
	fC2 = GetParValue("c2");
	fEta = GetParValue("eta");
	fDelta = GetParValue("delta");


	return kTRUE;
}


Double_t TQClamFitModelSpectrum::EvalN(const Double_t* v)
{
	const double x = v[0];

	if(fC1 <= 0.) { return 0.; }
	if(fSigma1 <= 0.) { return 0.; }
	if(fEta <= 0.) { return 0.; }
	if(fDelta <= 0.) { return 0.; }

	const double sigma2 = fSigma1 * fC2 / fC1;

	if(sigma2 <= 0.) { return 0.; }


	const double  c4 = -((fEta*fEta - fDelta) * sigma2) / fEta;

	const double c3 = fC2 * 
		TMath::Exp(-0.5 * fEta * fEta) *
		TMath::Power(fEta*sigma2 + c4, fDelta) /
		(TMath::Sqrt(2.*TMath::Pi()) * sigma2);

	if(x < fX0)
	{
		return fC1 / ( TMath::Sqrt(2.*TMath::Pi()) * fSigma1) * TMath::Exp(-0.5 * TMath::Power((x-fX0)/fSigma1, 2));
	}
	if(x < (fX0 + fEta * sigma2) )
	{
		return fC2 / (TMath::Sqrt(2.*TMath::Pi()) * sigma2) * TMath::Exp(-0.5 * TMath::Power((x-fX0)/sigma2, 2));
	}
	// else
	return c3 / TMath::Power(c4 +  x - fX0 , fDelta);
}
