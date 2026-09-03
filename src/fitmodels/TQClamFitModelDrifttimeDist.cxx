#include "fitmodels/TQClamFitModelDrifttimeDist.h"

#include "TGo4FitParameter.h"
#include "TMath.h"


ClassImp(TQClamFitModelDrifttimeDist)




TQClamFitModelDrifttimeDist::TQClamFitModelDrifttimeDist()
	: TGo4FitModel("DrifttimeDist","DrifttimeDistModel",kFALSE)
{
	AddPar(new TGo4FitParameter("A0","Amplitude0",1000.,1.,1e9,100.));
	AddPar(new TGo4FitParameter("A1","Amplitude1",1000.,1.,1e9,100.));
	AddPar(new TGo4FitParameter("mu0","Mean0",10.,-50,200,10.));
	AddPar(new TGo4FitParameter("mu1","Mean1",10,-50.,200,10.));
	AddPar(new TGo4FitParameter("sigma0","Sigma0",10.,0.01,100.,0.5));
	AddPar(new TGo4FitParameter("sigma1","Sigma1",10.,0.01,100.,0.5));
	AddPar(new TGo4FitParameter("s0","S0",0.5,0.01,10.,0.1));
	AddPar(new TGo4FitParameter("s1","S1",0.5,0.01,10.,0.1));
}



TQClamFitModelDrifttimeDist::TQClamFitModelDrifttimeDist(const char* name)
	: TGo4FitModel(name,"DrifttimeDistModel",kFALSE)
{
	AddPar(new TGo4FitParameter("A0","Amplitude0",1000.,1.,1e9,100.));
	AddPar(new TGo4FitParameter("A1","Amplitude1",1000.,1.,1e9,100.));
	AddPar(new TGo4FitParameter("mu0","Mean0",10.,-50,200,10.));
	AddPar(new TGo4FitParameter("mu1","Mean1",10,-50.,200,10.));
	AddPar(new TGo4FitParameter("sigma0","Sigma0",10.,0.01,100.,0.5));
	AddPar(new TGo4FitParameter("sigma1","Sigma1",10.,0.01,100.,0.5));
	AddPar(new TGo4FitParameter("s0","S0",0.5,0.01,10.,0.1));
	AddPar(new TGo4FitParameter("s1","S1",0.5,0.01,10.,0.1));
}



Double_t TQClamFitModelDrifttimeDist::N_skewed(double x,double A,double mu,double sigma,double s)
{
	double norm = A / (TMath::Sqrt(2 * M_PI) * sigma);
	double exp_part = TMath::Exp(-((x-mu)*(x-mu)) / (2*sigma*sigma));
	double erf_part = TMath::Erf(1 + s * (x-mu) / (TMath::Sqrt2() * sigma));
	return norm * exp_part * erf_part;
}


Bool_t TQClamFitModelDrifttimeDist::BeforeEval(Int_t)
{
	fA0 = GetParValue("A0");
	fA1 = GetParValue("A1");
	fMean0 = GetParValue("mu0");
	fMean1 = GetParValue("mu1");
	fSigma0 = GetParValue("sigma0");
	fSigma1 = GetParValue("sigma1");
	fS0 = GetParValue("s0");
	fS1 = GetParValue("s1");

	return kTRUE;
}


Double_t TQClamFitModelDrifttimeDist::EvalN(const Double_t* v)
{
	double x = v[0];

	double skewed0 = N_skewed(x,fA0,fMean0,fSigma0,fS0);
	double skewed1 = N_skewed(x,fA1,fMean1,fSigma1,fS1);

	return skewed0 + skewed1;
}
