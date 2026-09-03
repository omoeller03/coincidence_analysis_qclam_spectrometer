#include "TGo4AbstractInterface.h"
#include "TH1.h"
#include "TF1.h"
#include "TNamed.h"
#include "TCanvas.h"
#include "TString.h"
#include "TMath.h"
#include <iostream>
#include <ctime>

// ------------------------------------------------------------
// FIT FUNCTION
// ------------------------------------------------------------
Double_t spectrumFunc(Double_t *x, Double_t *p)
{
	double xx = x[0];

	double c1 = p[0];
	double x0 = p[1];
	double s1 = p[2];
	double c2 = p[3];
	double eta = p[4];
	double delta = p[5];

	if(c1 <= 0 || s1 <= 0 || eta <= 0 || delta <= 0) { return 0; }

	double sigma2 = s1 * c2 / c1;
	if(sigma2 <= 0) { return 0; }

	double c4 = -((eta*eta - delta) * sigma2) / eta;

	double c3 = c2 *
		exp(-0.5 * eta * eta) *
		pow(eta*sigma2 + c4, delta) /
		(sqrt(2.0 * TMath::Pi()) * sigma2);

	if (xx < x0)
	{
		return c1 / (sqrt(2.0 * TMath::Pi()) * s1) * exp(-0.5 * pow((xx - x0)/s1, 2));
	}

	if (xx < (x0 + eta * sigma2))
	{
		return c2 / (sqrt(2.0 * TMath::Pi()) * sigma2) * exp(-0.5 * pow((xx - x0)/sigma2, 2));
	}

	return c3 / pow(c4 + xx - x0, delta);
}

// ------------------------------------------------------------
// MAIN FIT MACRO (Go4 SAFE STYLE)
// ------------------------------------------------------------
Bool_t fit_spectrum(
		const char* histname = "XCorrSpectrum",
		Bool_t draw = kTRUE,
		Double_t xmin = 0,
		Double_t xmax = 1000,
		Double_t x0 = 500)
{
	auto go4 = TGo4AbstractInterface::Instance();
	if(!go4)
	{
		std::cout << "ERROR: no Go4 instance\n";
		return kFALSE;
	}

	// ========================================================
	// 1) FIND HISTOGRAM (ROBUST like addmultiplehistos)
	// ========================================================
	TString full = go4->FindItem(histname);

	if(full.IsNull())
	{
		std::cout << "ERROR: FindItem failed for " << histname << std::endl;
		return kFALSE;
	}

	TObject* obj = go4->GetObject(full, 1000);

	TH1* h = dynamic_cast<TH1*>(obj);
	if(!h)
	{
		std::cout << "ERROR: histogram not found: " << histname << std::endl;
		return kFALSE;
	}

	// ========================================================
	// 2) CLONE
	// ========================================================
	TH1* hfit = (TH1*)h->Clone(Form("%s_fit", histname));
	hfit->SetDirectory(nullptr);

	// ========================================================
	// 3) FIT FUNCTION
	// ========================================================
	TF1* f = new TF1("spectrumFunc", spectrumFunc, xmin, xmax, 6);

	f->SetParameters(
			hfit->GetMaximum(),
			//hfit->GetMean(),
			x0,
			1.5, // hfit->GetRMS()/2.0,
			hfit->GetMaximum()/2,
			2.0,
			1. //3.0
			);

	f->SetParLimits(2, 0.001, 10);
	f->SetParLimits(4, 0.001, 20);
	f->SetParLimits(5, 0.01, 50);

	std::cout << "Fitting spectrum...\n";

	hfit->Fit(f, "RQN");

	f->SetLineColor(kRed);
	hfit->GetListOfFunctions()->Add(f);

	// ========================================================
	// 4) SAVE HISTOGRAM
	// ========================================================
	TString rname = go4->SaveToMemory("Fits", hfit, kTRUE);
	std::cout << "Saved histogram to: " << rname << std::endl;

	// ========================================================
	// 5) DERIVED VALUES
	// ========================================================
	double sigma1 = f->GetParameter(2);
	double c1 = f->GetParameter(0);
	double c2 = f->GetParameter(3);

	double sigma2 = (c1 != 0) ? sigma1 * c2 / c1 : 0;

	double fwhm1 = 2.35482 * sigma1;
	double fwhm2 = 2.35482 * sigma2;
	double fwhm_avg = 0.5 * (fwhm1 + fwhm2);

	// ========================================================
	// 6) SAFE TEXT STORAGE (NO CRASH EVER)
	// ========================================================
	TString txt;
	txt.Form(
			"c1=%.6f +- %.6f\n"
			"x0=%.6f +- %.6f\n"
			"sigma1=%.6f +- %.6f\n"
			"c2=%.6f +- %.6f\n"
			"eta=%.6f +- %.6f\n"
			"delta=%.6f +- %.6f\n"
			"sigma2=%.6f\n"
			"FWHM1=%.6f\n"
			"FWHM2=%.6f\n"
			"FWHMavg=%.6f",
		f->GetParameter(0), f->GetParError(0),
		f->GetParameter(1), f->GetParError(1),
		f->GetParameter(2), f->GetParError(2),
		f->GetParameter(3), f->GetParError(3),
		f->GetParameter(4), f->GetParError(4),
		f->GetParameter(5), f->GetParError(5),
		sigma2, fwhm1, fwhm2, fwhm_avg
			);

   
	TString infoName = Form("%s_fitinfo"/*_%ld"*/, histname); //, time(nullptr));

	TString fullOld = go4->FindItem(infoName);
	if(!fullOld.IsNull())
	{
		TObject* old = go4->GetObject(fullOld, 1);
		if (old)
		{
			delete old;
			//go4->RemoveObject(fullOld);   // <- wichtig
		}
	}

	/*
	   if (go4->FindItem(infoName) != "") {
	   TObject* old = go4->GetObject(go4->FindItem(infoName), 1);
	   if (old) delete old;
	   }
	*/
   
	TNamed* info = new TNamed(infoName, txt);

	go4->SaveToMemory("Fits", info, kTRUE);

	std::cout << "\n===== FIT RESULTS =====\n";
	std::cout << txt.Data() << std::endl;

	// ========================================================
	// 7) DRAW
	// ========================================================
   
	if(draw)
	{
		TCanvas* c = new TCanvas("fit_canvas", "Fit", 900, 600);
		hfit->Draw();
		f->Draw("same");
	}

	return kTRUE;
}
