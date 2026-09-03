#include "parameter/TQClamCalibrationParameter.h"

/** fitmodels */
#include "fitmodels/TQClamFitModelEMG.h"
#include "fitmodels/TQClamFitModelTail1OverX.h"
#include "fitmodels/TQClamFitModelGaussTail.h"

#include "TGo4Analysis.h"
#include "TGo4Fitter.h"
#include "TGo4FitModelGauss1.h"
#include "TGo4FitModelFormula.h"
#include "TGo4Log.h"

#include "TH1.h"
#include "TGraph.h"
#include "TMath.h"



ClassImp(TQClamCalibrationParameter)


//////////////////////////////////////////////////////////////////////

TQClamCalibrationParameter::TQClamCalibrationParameter()
	: TGo4Parameter("CaliParam")
{}


//////////////////////////////////////////////////////////////////////

TQClamCalibrationParameter::TQClamCalibrationParameter(const char* name, TH1* spectrum)
	: TGo4Parameter(name),
	fSpectrum(spectrum)
{
	TGo4Log::Info("[TQClamCalibrationParameter] Create %s",name);

	if(fSpectrum) { fSpectrumName = fSpectrum->GetName(); }
	else { fSpectrumName = "ProjSpectrum"; /** TODO */ }


	fFitter = new TGo4Fitter("ElectronLineFitter", TGo4Fitter::ff_least_squares, kTRUE);

	if(fSpectrum && fFitter) { fFitter->AddH1(__DATANAME__,fSpectrum,kFALSE); }


	/** Gauss-Komponente
	 * Name: 'G' */
	fFitter->AddGauss1(__DATANAME__,"G", fStartMean, fStartSigma);
	fFitter->SetParValue("G.Ampl",10000);
	fFitter->SetParValue("G.Pos",36);
	fFitter->SetParValue("G.Width",0.5);

	fFitter->SetParRange("G.Pos",0.,100.);
	fFitter->SetParRange("G.Width",0.05,10.);
	fFitter->SetParRange("G.Ampl",0.,1e9);


	/** EMG
	 * Name: 'EMG' */
	auto* emg = new TQClamFitModelEMG("EMG");
	fFitter->AddModel(__DATANAME__,emg);

	emg->SetParName(0,"Aemg");
	emg->SetParName(1,"lambda");
	emg->SetParName(2,"mu");
	emg->SetParName(3,"sigma");

	fFitter->SetParValue("EMG.Aemg", 0.5*100);
	fFitter->SetParValue("EMG.lambda", 0.1);
	fFitter->SetParValue("EMG.mu", 50.);
	fFitter->SetParValue("EMG.sigma",0.5);

	fFitter->SetParRange("EMG.lambda",0.0001,1.);
	fFitter->SetParRange("EMG.sigma",0.05,10.);
	fFitter->SetParRange("EMG.mu",0.,100.);

	/** 1/x-Tail
	 * Name: 'Tail1OverX'*/
	auto* tail = new TQClamFitModelTail1OverX("Tail1OverX");
	fFitter->AddModel(__DATANAME__,tail);

	tail->SetParName(0,"A");
	tail->SetParName(1,"x0");
	tail->SetParName(2,"eps");

	fFitter->SetParValue("Tail1OverX.A",0.2*fStartAmp);
	fFitter->SetParValue("Tail1OverX.x0",fStartMean);
	fFitter->SetParValue("Tail1OverX.eps",1.0);



	/** Gauss mit 1/x-Tail
	 * Name: 'GaussTail' */
	auto* gauss_tail = new TQClamFitModelGaussTail("GaussTail");
	fFitter->AddModel(__DATANAME__,gauss_tail);

	gauss_tail->SetParName(0,"GaussTail.A");
	gauss_tail->SetParName(1,"GaussTail.mu");
	gauss_tail->SetParName(2,"GaussTail.sigma");

	gauss_tail->SetParName(3,"GaussTail.TailA");
	gauss_tail->SetParName(4,"GaussTail.TailX0");
	gauss_tail->SetParName(5,"GaussTail.TailEps");



	fFitter->Print();

	fUseEMGLineFitter = kFALSE;
	fUseSimpleLineFitter = kFALSE;
	fUseGaussTailLineFitter = kFALSE;



}



TQClamCalibrationParameter::~TQClamCalibrationParameter()
{
	delete fFitter;
}


void TQClamCalibrationParameter::SetSpectrum(TH1* h)
{
	fSpectrum = h;
	if(fSpectrum)
	{
		fSpectrumName = fSpectrum->GetName();
		if(fFitter)
		{
			fFitter->SetH1(__DATANAME__,fSpectrum, kFALSE);
		}
	}
}



Bool_t TQClamCalibrationParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamCalibrationParameter] UpdateFrom().");

	auto* other = dynamic_cast<TQClamCalibrationParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamCalibrationParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}


	fDoFit = other->fDoFit;

	fFitUpdateEventInterval = other->fFitUpdateEventInterval;

	fUseGaussLineFitter = other->fUseGaussLineFitter;
	fUseSimpleLineFitter = other->fUseSimpleLineFitter;
	fUseEMGLineFitter = other->fUseEMGLineFitter;
	fUseGaussTailLineFitter = other->fUseGaussTailLineFitter;


	fStartMean = other->fStartMean;
	fStartSigma = other->fStartSigma;
	fStartAmp = other->fStartAmp;
	fStartEps = other->fStartEps;
	fStartLambda = other->fStartLambda;

	fSpectrumName = other->fSpectrumName;



	/** Spektrim ggf. aus Analyse holen */
	if(!fSpectrum && !fSpectrumName.IsNull())
	{
		fSpectrum = dynamic_cast<TH1*>(TGo4Analysis::Instance()->GetObject(fSpectrumName.Data()));
		if(fSpectrum && fFitter)
		{
			if(!fFitter->FindData(__DATANAME__))
			{
				fFitter->SetH1(__DATANAME__,fSpectrum,kFALSE);
			}
			else
			{
				fFitter->SetH1(__DATANAME__,fSpectrum,kFALSE);
			}
		}
	}

	/** fitten */
	/*
	if(fDoFit && fFitter && fSpectrum)
	{
		TGo4Log::Info("[TQClamCalibrationParameter] Fuehre Fit an Elektron-Linie in %s aus.",
				fSpectrum->GetName());

		fFitter->SetParValue("G.Pos",fStartMean);
		fFitter->SetParValue("G.Width",fStartSigma);
		fFitter->SetParValue("G.Ampl",fStartAmp);

		fFitter->SetParValue("EMG.mu",fStartMean);
		fFitter->SetParValue("EMG.sigma",fStartSigma);

		fFitter->SetParValue("Tail1OverX.x0",fStartMean);

		fFitter->AssignModelTo("G",__DATANAME__);

		//if(auto* mEMG = fFitter->FindModel("EMG"))
		//{
		if(fUseEMGLineFitter) { fFitter->AssignModelTo("EMG",__DATANAME__); }
		else { fFitter->ClearModelAssignmentTo("EMG",__DATANAME__); }
		//}

		//if(auto* mTail = fFitter->FindModel("Tail1OverX"))
		//{
		if(fUseSimpleLineFitter) { fFitter->AssignModelTo("Tail1OverX",__DATANAME__); }
		else { fFitter->ClearModelAssignmentTo("Tail1OverX",__DATANAME__); }
		//}


		fFitter->DoActions();
		fFitter->PrintLines();

		TGo4Log::Info("=== Fit-Ergebnisse ===");

		TGo4Log::Info("Gauss: Mean  = %f ± %f",
				fFitter->GetParValue("G.Pos"),
				fFitter->GetParError("G.Pos"));

		TGo4Log::Info("Gauss: Sigma = %f ± %f",
				fFitter->GetParValue("G.Width"),
				fFitter->GetParError("G.Width"));

		TGo4Log::Info("Gauss: Amp   = %f ± %f",
				fFitter->GetParValue("G.Ampl"),
				fFitter->GetParError("G.Ampl"));

		if(fUseEMGLineFitter)
		{
			TGo4Log::Info("EMG: Lambda = %f ± %f",
					fFitter->GetParValue("EMG.lambda"),
					fFitter->GetParError("EMG.lambda"));
		}

		if(fUseSimpleLineFitter)
		{
			TGo4Log::Info("Tail: A = %f ± %f",
					fFitter->GetParValue("Tail1OverX.A"),
					fFitter->GetParError("Tail1OverX.A"));
		}






		fGauss_Mean = fFitter->GetParValue("G.Pos");
		fGauss_Sigma = fFitter->GetParValue("G.Width");
		fGauss_Amp = fFitter->GetParValue("G.Ampl");

		if(fUseEMGLineFitter) { fEMG_Lambda = fFitter->GetParValue("EMG.lambda"); }
		if(fUseSimpleLineFitter) { fTail_Amp = fFitter->GetParValue("Tail1OverX.A"); }

		//fDoFit = kFALSE;
	}
*/

	return kTRUE;
}




/*
 * TODO: ReadCalibrationData
 *
 * TODO: Energieberechnung in abhaengigkeit von kanal
 * */


