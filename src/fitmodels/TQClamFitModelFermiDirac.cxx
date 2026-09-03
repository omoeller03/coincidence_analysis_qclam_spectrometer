#include "fitmodels/TQClamFitModelFermiDirac.h"

#include "TGo4FitParameter.h"
#include "TMath.h"


ClassImp(TQClamFitModelFermiDirac)




TQClamFitModelFermiDirac::TQClamFitModelFermiDirac()
	: TGo4FitModel("FermiDirac","FermiDiracModel",kFALSE)
{
	AddPar(new TGo4FitParameter("A","Amplitude",1000.,0.,1.e9,100.));
	AddPar(new TGo4FitParameter("t0","t0",10.,-50,100.,1.));
	AddPar(new TGo4FitParameter("tau","tau",1.,0.01,30.,1.));
	AddPar(new TGo4FitParameter("T","T",0.5,0.01,10.,0.2));
}



TQClamFitModelFermiDirac::TQClamFitModelFermiDirac(const char* name)
	: TGo4FitModel(name,"FermiDiracModel",kFALSE)
{
	AddPar(new TGo4FitParameter("A","Amplitude",1000.,0.,1.e9,100.));
	AddPar(new TGo4FitParameter("t0","t0",10.,-50,100.,1.));
	AddPar(new TGo4FitParameter("tau","tau",1.,0.01,30.,1.));
	AddPar(new TGo4FitParameter("T","T",0.5,0.01,10.,0.2));
}




Bool_t TQClamFitModelFermiDirac::BeforeEval(Int_t)
{
	fA = GetParValue("A");
	fT0 = GetParValue("t0");
	fTau = GetParValue("tau");
	fT = GetParValue("T");

	return kTRUE;
}


Double_t TQClamFitModelFermiDirac::EvalN(const Double_t* v)
{
	double t = v[0];

	return fA / (1.0 + TMath::Exp(-(t - (fT0 + fTau)) / fT));
}
