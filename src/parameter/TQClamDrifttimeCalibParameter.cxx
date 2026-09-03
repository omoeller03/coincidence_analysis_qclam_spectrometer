#include "parameter/TQClamDrifttimeCalibParameter.h"

#include "TGo4Fitter.h"
#include "TGo4FitParameter.h"
#include "TGo4Log.h"

#include "fitmodels/TQClamFitModelFermiDirac.h"
#include "fitmodels/TQClamFitModelDrifttimeDist.h"

//////////////////////////////////////////////////////////////////////

ClassImp(TQClamDrifttimeCalibParameter)

//////////////////////////////////////////////////////////////////////

TQClamDrifttimeCalibParameter::TQClamDrifttimeCalibParameter()
	: TGo4Parameter("DrifttimeCalibParam")
{
	/** nullen */
	fUpdateEventInterval = 100000;
	
	fDoFitAny = kFALSE;
	fDoPlotFit = kFALSE;

	fUseFermiDirac = kFALSE;
	fUseSkewed = kFALSE;

	/** werte fermi */
	fStartAmp = 100.;
	fStartT0 = 50.;
	fStartTau = 2.;
	fStartT = 2.;

	fMinAmp = 0.;
	fMaxAmp = 1e9;
	fMinT0 = -200.;
	fMaxT0 = 1000.;
	fMinTau = -50.;
	fMaxTau = 100.;
	fMinT = 0.;
	fMaxT = 50.;

	/** skewed */
	fStartA0 = 100.;
	fStartA1 = 100.;
	fStartMu0 = 10.;
	fStartMu1 = 10.;
	fStartSigma0 = 10.;
	fStartSigma1 = 10.;
	fStartS0 = 0.5;
	fStartS1 = 0.5;

	fMinA0 = 0.;
	fMaxA0 = 1e9;
	fMinA1 = 0.;
	fMaxA1 = 1e9;
	fMinMu0 = -50.;
	fMaxMu0 = 200.;
	fMinMu1 = -50.;
	fMaxMu1 = 200.;
	fMinSigma0 = 0.1;
	fMaxSigma0 = 100.;
	fMinSigma1 = 0.1;
	fMaxSigma1 = 100.;
	fMinS0 = 0.01;
	fMaxS0 = 10.;
	fMinS1 = 0.01;
	fMaxS1 = 10.;


	for(size_t l=0; l<MAX_LAYER; l++)
	{
		fNumWires[l] = 0;
		fDoFit[l] = kFALSE;
	
		for(size_t w=0; w<MAX_WIRE; w++)
		{
			fT0[l][w] = 0.;
			fTau[l][w] = 0.;
			fT[l][w] = 0.;
			fA[l][w] = 0.;

			fA0[l][w] = 0.;
			fA1[l][w] = 0.;
			fMu0[l][w] = 0.;
			fMu1[l][w] = 0.;
			fSigma0[l][w] = 0.;
			fSigma1[l][w] = 0.;
			fS0[l][w] = 0.;
			fS1[l][w] = 0.;
		}

	}

	/** dummy --> richtiges im prozessor setzen */
	fFitter->AddH1(__DATANAME_DRIFTDIST__,nullptr,kFALSE);

	/** fitter erzeugen */
	fFitter = new TGo4Fitter("DrifttimeDistFitter",TGo4Fitter::ff_least_squares,kTRUE);



	/** skewed modell hinzufuegen */
	auto* skewed = new TQClamFitModelDrifttimeDist("DrifttimeDist");
	fFitter->AddModel(__DATANAME_DRIFTDIST__,skewed);

	/** parameter-namen setzen */
	skewed->SetParName(0,"A0");
	skewed->SetParName(0,"mu0");
	skewed->SetParName(0,"sigma0");
	skewed->SetParName(0,"s0");
	skewed->SetParName(0,"A1");
	skewed->SetParName(0,"mu1");
	skewed->SetParName(0,"sigma1");
	skewed->SetParName(0,"s1");


	/** Fermi-Dirac Modell hinzufuegen */
	auto* fermi = new TQClamFitModelFermiDirac("FermiDirac");
	fFitter->AddModel(__DATANAME_DRIFTDIST__,fermi);

	/** parameter-namen setzen */
	fermi->SetParName(0,"A");
	fermi->SetParName(1,"t0");
	fermi->SetParName(2,"tau");
	fermi->SetParName(3,"T");

	/** startwerte setzen */
	fFitter->SetParValue("FermiDirac.A",fStartAmp);
	fFitter->SetParValue("FermiDirac.t0",fStartT0);
	fFitter->SetParValue("FermiDirac.tau",fStartTau);
	fFitter->SetParValue("FermiDirac.T",fStartT);

	/** ranges */
	fFitter->SetParRange("FermiDirac.A",fMinAmp,fMaxAmp);
	fFitter->SetParRange("FermiDirac.t0",fMinT0,fMaxT0);
	fFitter->SetParRange("FermiDirac.tau",fMinTau,fMaxTau);
	fFitter->SetParRange("FermiDirac.T",fMinT,fMaxT);
}

TQClamDrifttimeCalibParameter::TQClamDrifttimeCalibParameter(const char* name)
	: TGo4Parameter(name)
{
	/** nullen */
	fUpdateEventInterval = 100000;

	fDoFitAny = kFALSE;
	fDoPlotFit = kFALSE;

	fUseFermiDirac = kFALSE;
	fUseSkewed = kFALSE;
	
	/** Fermi */
	fStartAmp = 100.;
	fStartT0 = 50.;
	fStartTau = 2.;
	fStartT = 2.;

	fMinAmp = 0.;
	fMaxAmp = 1e9;
	fMinT0 = -200.;
	fMaxT0 = 1000.;
	fMinTau = -50.;
	fMaxTau = 100.;
	fMinT = 0.;
	fMaxT = 50.;


	/** skewed */
	fStartA0 = 100.;
	fStartA1 = 100.;
	fStartMu0 = 10.;
	fStartMu1 = 10.;
	fStartSigma0 = 10.;
	fStartSigma1 = 10.;
	fStartS0 = 0.5;
	fStartS1 = 0.5;

	fMinA0 = 0.;
	fMaxA0 = 1e9;
	fMinA1 = 0.;
	fMaxA1 = 1e9;
	fMinMu0 = -50.;
	fMaxMu0 = 200.;
	fMinMu1 = -50.;
	fMaxMu1 = 200.;
	fMinSigma0 = 0.1;
	fMaxSigma0 = 100.;
	fMinSigma1 = 0.1;
	fMaxSigma1 = 100.;
	fMinS0 = 0.01;
	fMaxS0 = 10.;
	fMinS1 = 0.01;
	fMaxS1 = 10.;


	for(size_t l=0; l<MAX_LAYER; l++)
	{
		fNumWires[l] = 0;
		fDoFit[l] = kFALSE;
	
		for(size_t w=0; w<MAX_WIRE; w++)
		{
			fT0[l][w] = 0.;
			fTau[l][w] = 0.;
			fT[l][w] = 0.;
			fA[l][w] = 0.;

			fA0[l][w] = 0.;
			fA1[l][w] = 0.;
			fMu0[l][w] = 0.;
			fMu1[l][w] = 0.;
			fSigma0[l][w] = 0.;
			fSigma1[l][w] = 0.;
			fS0[l][w] = 0.;
			fS1[l][w] = 0.;
		}

	}



	/** fitter erzeugen */
	fFitter = new TGo4Fitter("DrifttimeDistFitter",TGo4Fitter::ff_least_squares,kTRUE);

	/** dummy --> richtiges im prozessor setzen */
	fFitter->AddH1(__DATANAME_DRIFTDIST__,nullptr,kFALSE);


	/** skewed modell hinzufuegen */
	auto* skewed = new TQClamFitModelDrifttimeDist("DrifttimeDist");
	fFitter->AddModel(__DATANAME_DRIFTDIST__,skewed);

	/** parameter-namen setzen */
	skewed->SetParName(0,"A0");
	skewed->SetParName(0,"mu0");
	skewed->SetParName(0,"sigma0");
	skewed->SetParName(0,"s0");
	skewed->SetParName(0,"A1");
	skewed->SetParName(0,"mu1");
	skewed->SetParName(0,"sigma1");
	skewed->SetParName(0,"s1");


	/** Fermi-Dirac Modell hinzufuegen */
	auto* fermi = new TQClamFitModelFermiDirac("FermiDirac");
	fFitter->AddModel(__DATANAME_DRIFTDIST__,fermi);

	/** parameter-namen setzen */
	fermi->SetParName(0,"A");
	fermi->SetParName(1,"t0");
	fermi->SetParName(2,"tau");
	fermi->SetParName(3,"T");

	/** startwerte setzen */
	fFitter->SetParValue("FermiDirac.A",fStartAmp);
	fFitter->SetParValue("FermiDirac.t0",fStartT0);
	fFitter->SetParValue("FermiDirac.tau",fStartTau);
	fFitter->SetParValue("FermiDirac.T",fStartT);

	/** ranges */
	fFitter->SetParRange("FermiDirac.A",fMinAmp,fMaxAmp);
	fFitter->SetParRange("FermiDirac.t0",fMinT0,fMaxT0);
	fFitter->SetParRange("FermiDirac.tau",fMinTau,fMaxTau);
	fFitter->SetParRange("FermiDirac.T",fMinT,fMaxT);
}




TQClamDrifttimeCalibParameter::~TQClamDrifttimeCalibParameter()
{
	delete fFitter;
}


Bool_t TQClamDrifttimeCalibParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamDrifttimeCalibParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamDrifttimeCalibParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamDrifttimeCalibParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}


	for(size_t l=0; l<MAX_LAYER; l++)
	{
		fNumWires[l] = other->fNumWires[l];
		fDoFit[l] = other->fDoFit[l];
	}

	fDoFitAny = other->fDoFitAny;
	fDoPlotFit = other->fDoPlotFit;

	fUseFermiDirac = other->fUseFermiDirac;
	fUseSkewed = other->fUseSkewed;

	fStartAmp = other->fStartAmp;
	fStartT0 = other->fStartT0;
	fStartTau = other->fStartTau;
	fStartT = other->fStartT;
	
	fMinAmp = other->fMinAmp;
	fMaxAmp = other->fMaxAmp;
	fMinT0 = other->fMinT0;
	fMaxT0 = other->fMaxT0;
	fMinTau = other->fMinTau;
	fMaxTau = other->fMaxTau;
	fMinT = other->fMinT;
	fMaxT = other->fMaxT;

	if(fUseFermiDirac)
	{
	        /** startwerte setzen */
	        fFitter->SetParValue("FermiDirac.A",fStartAmp);
	        fFitter->SetParValue("FermiDirac.t0",fStartT0);
	        fFitter->SetParValue("FermiDirac.tau",fStartTau);
	        fFitter->SetParValue("FermiDirac.T",fStartT);

		/** ranges */
		fFitter->SetParRange("FermiDirac.A",fMinAmp,fMaxAmp);
		fFitter->SetParRange("FermiDirac.t0",fMinT0,fMaxT0);
		fFitter->SetParRange("FermiDirac.tau",fMinTau,fMaxTau);
		fFitter->SetParRange("FermiDirac.T",fMinT,fMaxT);
	}

	/** skewed */
	fStartA0 = other->fStartA0;
	fStartA1 = other->fStartA1;
	fStartMu0 = other->fStartMu0;
	fStartMu1 = other->fStartMu1;
	fStartSigma0 = other->fStartSigma0;
	fStartSigma1 = other->fStartSigma1;
	fStartS0 = other->fStartS0;
	fStartS1 = other->fStartS1;

	fMinA0 = other->fMinA0;
	fMaxA0 = other->fMaxA0;
	fMinA1 = other->fMinA1;
	fMaxA1 = other->fMaxA1;
	fMinMu0 = other->fMinMu0;
	fMaxMu0 = other->fMaxMu0;
	fMinMu1 = other->fMinMu1;
	fMaxMu1 = other->fMaxMu1;
	fMinSigma0 = other->fMinSigma0;
	fMaxSigma0 = other->fMaxSigma0;
	fMinSigma1 = other->fMinSigma1;
	fMaxSigma1 = other->fMaxSigma1;
	fMinS0 = other->fMinS0;
	fMaxS0 = other->fMaxS0;
	fMinS1 = other->fMinS1;
	fMaxS1 = other->fMaxS1;

	if(fUseSkewed)
	{
		/** Startwerte setzen */
		fFitter->SetParValue("DrifttimeDist.A0",fStartA0);
		fFitter->SetParValue("DrifttimeDist.A1",fStartA1);
		fFitter->SetParValue("DrifttimeDist.Mu0",fStartMu0);
		fFitter->SetParValue("DrifttimeDist.Mu1",fStartMu1);
		fFitter->SetParValue("DrifttimeDist.Sigma0",fStartSigma0);
		fFitter->SetParValue("DrifttimeDist.Sigma1",fStartSigma1);
		fFitter->SetParValue("DrifttimeDist.S0",fStartS0);
		fFitter->SetParValue("DrifttimeDist.S1",fStartS1);

		/** ranges */
		fFitter->SetParRange("DrifttimeDist.A0",fMinA0,fMaxA0);
		fFitter->SetParRange("DrifttimeDist.A1",fMaxA1,fMaxA1);
		fFitter->SetParRange("DrifttimeDist.Mu0",fMinMu0,fMaxMu0);
		fFitter->SetParRange("DrifttimeDist.Mu1",fMinMu1,fMaxMu1);
		fFitter->SetParRange("DrifttimeDist.Sigma0",fMinSigma0,fMaxSigma0);
		fFitter->SetParRange("DrifttimeDist.Sigma1",fMinSigma1,fMaxSigma1);
		fFitter->SetParRange("DrifttimeDist.S0",fMinS0,fMaxS0);
		fFitter->SetParRange("DrifttimeDist.S1",fMinS1,fMaxS1);
	}





	/** fit ergebnisse nicht updaten!! */

	return kTRUE;
}
