#include "fitmodels/TQClamFitModelGaussTail.h"

#include "TGo4FitParameter.h"
#include "TMath.h"

ClassImp(TQClamFitModelGaussTail)

TQClamFitModelGaussTail::TQClamFitModelGaussTail(const char* name)
    : TGo4FitModel(name,"GaussTailModel",kFALSE)
{
	AddPar(new TGo4FitParameter("A","Amplitude", 100., 0., 1e9, 10.));
	AddPar(new TGo4FitParameter("mu","Mean", 40., 0., 100., 0.1));
	AddPar(new TGo4FitParameter("sigma","Sigma", 0.5, 0.05, 10., 0.01));

	AddPar(new TGo4FitParameter("TailA","Tail amplitude", 10., 0., 1e9, 100.));
	AddPar(new TGo4FitParameter("TailX0","Tail x0", 50., -100., 200., 10.));
	AddPar(new TGo4FitParameter("TailEps","Tail epsilon", 1., 0., 100., 1.));
}

Bool_t TQClamFitModelGaussTail::BeforeEval(Int_t)
{
	fA = GetParValue("A");
	fMu = GetParValue("mu");
	fSigma = GetParValue("sigma");

	fTailA = GetParValue("TailA");
	fTailX0 = GetParValue("TailX0");
	fTailEps = GetParValue("TailEps");

	return kTRUE;
}

Double_t TQClamFitModelGaussTail::EvalN(const Double_t* v)
{
	double x = v[0];

	/** Gauss fuer elastische Linie/Zustand */
	double gauss = fA * TMath::Gaus(x, fMu, fSigma, kTRUE);

	/** Tail -> rechts von Gauss (nieder-energetische elektronen) */
	double tail = 0.;
	if(x > fTailX0) { tail = fTailA / (x - fTailX0 + fTailEps); }

	return gauss + tail;
}

