#include "TGo4AbstractInterface.h"
#include "TH1.h"
#include "TF1.h"
#include "TNamed.h"
#include "TCanvas.h"
#include "TString.h"
#include "TMath.h"
#include "TAxis.h"

#include <iostream>
#include <cmath>
#include <algorithm>


// ============================================================
// SPEKTRUMSFUNKTION
// ============================================================

Double_t spectrumFunc_auto(
		Double_t *x,
		Double_t *p)
{
    
	const double xx = x[0];

    
	const double c1    = p[0];
    
	const double x0    = p[1];
    
	const double sigma1 = p[2];

    
	const double c2    = p[3];
    
	const double eta   = p[4];
    
	const double delta = p[5];


    
	if(c1 <= 0 || sigma1 <= 0 || c2 <= 0 || eta <= 0 || delta <= 0)
	{
		return 0.0;
	}


	// --------------------------------------------------------
	// Breite des zweiten Gauß-Anteils
	// --------------------------------------------------------

	const double sigma2 = sigma1 * c2 / c1;


	if(sigma2 <= 0) { return 0.0; }


	// --------------------------------------------------------
	// Übergang zum Power-Law-Tail
	// --------------------------------------------------------

	const double c4 = -((eta * eta - delta) * sigma2) / eta;


	const double argument = eta * sigma2 + c4;


	if(argument <= 0) { return 0.0; }


	const double c3 = c2 * std::exp(-0.5 * eta * eta) * std::pow(argument, delta) / (std::sqrt(2.0 * TMath::Pi()) * sigma2);


	// --------------------------------------------------------
	// Linke Gauß-Seite
	// --------------------------------------------------------

	if(xx < x0)
	{
		return 	c1 / (std::sqrt(2.0 * TMath::Pi()) * sigma1) * std::exp(-0.5 * std::pow( (xx - x0) / sigma1,2.0));
	}


	// --------------------------------------------------------
	// Rechte Gauß-Seite
	// --------------------------------------------------------

	if(xx < x0 + eta * sigma2)
	{
		return c2 / (std::sqrt(2.0 * TMath::Pi()) * sigma2) * std::exp(-0.5 * std::pow((xx - x0) / sigma2,2.0));
	}


	// --------------------------------------------------------
	// Power-Law-Tail
	// --------------------------------------------------------

	const double denominator = c4 + xx - x0;


	if(denominator <= 0) { return 0.0; }


	return c3 / std::pow(denominator,delta);
}


// ============================================================
// AUTOMATISCHE PEAK-BESTIMMUNG
// ============================================================
//
// Vorgehen:
//
// 1. Globales Maximum des Histogramms bestimmen
// 2. Position des Maximums bestimmen
// 3. Kleinen lokalen Gauß-Fit um das Maximum durchführen
// 4. Gauß-Mittelwert als Peakposition verwenden
//
// Dieser Fit dient ausschließlich zur Bestimmung
// der Peakposition.
//
// Der eigentliche Spektrumsfit erfolgt anschließend
// mit spectrumFunc_auto().
//
// ============================================================

double determinePeakPosition(
		TH1* h,
		double& peakError)
{
	peakError = 0.0;


	if(!h) { return NAN; }


	// --------------------------------------------------------
	// Globales Maximum
	// --------------------------------------------------------

	const int maxBin = h->GetMaximumBin();


	const double roughPeak = h->GetBinCenter(maxBin);


	const double binWidth = h->GetBinWidth(maxBin);


	// --------------------------------------------------------
	// Kleiner lokaler Fitbereich
	//
	// Wie im ursprünglichen automatischen Macro:
	// mindestens 10 Einheiten oder 8 Binbreiten
	// --------------------------------------------------------

	const double fitHalfWidth = std::max(10.0,8.0 * binWidth);


	double xmin = roughPeak - fitHalfWidth;


	double xmax = roughPeak + fitHalfWidth;


	// Histogrammgrenzen berücksichtigen

	xmin = std::max(xmin,h->GetXaxis()->GetXmin());


	xmax = std::min(xmax,h->GetXaxis()->GetXmax());


	// --------------------------------------------------------
	// Lokale Gauß-Funktion
	// --------------------------------------------------------

	TF1 fPeak("fPeak","gaus",xmin,xmax);


	fPeak.SetParameters(h->GetMaximum(),roughPeak,std::max(binWidth,h->GetRMS() / 10.0));


	// --------------------------------------------------------
	// Lokaler Gauß-Fit
	//
	// R = Fit innerhalb des definierten Bereichs
	// Q = Quiet
	// 0 = nicht in Histogramm speichern
	// --------------------------------------------------------

    
	const int fitStatus = h->Fit(&fPeak,"RQ0");


	double peak = fPeak.GetParameter(1);


	peakError = fPeak.GetParError(1);


	// --------------------------------------------------------
	// Plausibilitätsprüfung
	// --------------------------------------------------------

	if(fitStatus != 0 || !std::isfinite(peak) || peak < h->GetXaxis()->GetXmin() || peak > h->GetXaxis()->GetXmax())
	{
		std::cout
			<< "WARNING: Local peak fit failed.\n"
			<< "Using histogram maximum instead.\n";

		peak = roughPeak;

		peakError = 0.0;
	}


	return peak;
}


// ============================================================
// HAUPTFUNKTION
// ============================================================

Bool_t fit_spectrum_auto(
		const char* histname = "XCorrSpectrum",
		Bool_t draw = kTRUE)
{
	auto go4 = TGo4AbstractInterface::Instance();


	if(!go4)
	{
		std::cout
			<< "ERROR: no Go4 instance\n";

		return kFALSE;
	}


	// ========================================================
	// 1) HISTOGRAMM FINDEN
	// ========================================================

	TString full = go4->FindItem(histname);


	if(full.IsNull())
	{
		std::cout
			<< "ERROR: FindItem failed for "
			<< histname
			<< std::endl;


		return kFALSE;
	}


	TObject* obj = go4->GetObject(full,1000);


	TH1* h = dynamic_cast<TH1*>(obj);


	if(!h)
	{
		std::cout
			<< "ERROR: histogram not found: "
			<< histname
			<< std::endl;


		return kFALSE;
	}


	// ========================================================
	// 2) AUTOMATISCHE PEAK-SUCHE
	// ========================================================

	double peakError = 0.0;


	const double peak = determinePeakPosition(h,peakError);


	if(!std::isfinite(peak))
	{
		std::cout
			<< "ERROR: automatic peak determination failed\n";


		return kFALSE;
	}


	std::cout
		<< "\n";
	std::cout
		<< "===== AUTOMATIC PEAK DETECTION =====\n";
	std::cout
		<< "Peak position = "
		<< peak
		<< " +- "
		<< peakError
		<< std::endl;


	// ========================================================
	// 3) FITBEREICH AUTOMATISCH BESTIMMEN
	// ========================================================
	//
	// Der Fitbereich basiert auf der automatisch
	// bestimmten Peakposition.
	//
	//     fitMin = peak - 20
	//     fitMax = peak + 80
	//
	// ========================================================

	double fitMin = peak - 20.0;


	double fitMax = peak + 80.0;


	// Histogrammgrenzen nicht verlassen

	fitMin = std::max(fitMin,h->GetXaxis()->GetXmin());


	fitMax = std::min(fitMax,h->GetXaxis()->GetXmax());


	std::cout
		<< "Automatically determined fit range: "
		<< fitMin
		<< " ... "
		<< fitMax
		<< std::endl;


	// ========================================================
	// 4) HISTOGRAMM KLONEN
	// ========================================================

	TH1* hfit = dynamic_cast<TH1*>(h->Clone(Form("%s_fit",histname)));


	if(!hfit)
	{
		std::cout << "ERROR: could not clone histogram\n";


		return kFALSE;
	}


	hfit->SetDirectory(nullptr);


    // ========================================================
    // 5) SPEKTRUMSFUNKTION ERZEUGEN
    // ========================================================

    
	TF1* f = new TF1(
            
			Form("spectrumFunc_auto_%s",histname),
            
			spectrumFunc_auto,
            
			fitMin,
            
			fitMax,
            
			6
        
			);


    
	// ========================================================
    
	// 6) STARTPARAMETER
    
	// ========================================================

    
	const double maximum =
        
		hfit->GetBinContent(
            
				hfit->GetMaximumBin()
        
				);


    
	f->SetParameters(
			maximum * 2.0, // c1
			peak,          // x0
			2.0,           // sigma1
			maximum,       // c2
			2.0,           // eta
			2.0            // delta
			);


	// ========================================================
	// 7) PARAMETERGRENZEN
	// ========================================================

	// c1

	f->SetParLimits(
			0,
			0.0,
			maximum * 100.0
			);


	// x0

	f->SetParLimits(
			1,
			peak - 10.0,
			peak + 10.0
			);


	// sigma1

	f->SetParLimits(
			2,
			0.001,
			10.0
			);


	// c2

	f->SetParLimits(
			3,
			0.0,
			maximum * 100.0
			);


	// eta

	f->SetParLimits(
			4,
			0.001,
			20.0
			);


	// delta

	f->SetParLimits(
			5,
			0.01,
			50.0
			);


	// ========================================================
	// 8) SPEKTRUM FITTEN
	// ========================================================

	std::cout
		<< "\n";
	std::cout
		<< "Fitting spectrum...\n";


	const int fitStatus = hfit->Fit(f,"RQN");


	if(fitStatus != 0)
	{
		std::cout
			<< "WARNING: spectrum fit returned status "
			<< fitStatus
			<< std::endl;
	}


	f->SetLineColor(kRed);


	//hfit->GetListOfFunctions()->Add(f);


	// ========================================================
	// 9) FITERGEBNISSE
	// ========================================================

	const double c1 = f->GetParameter(0);


	const double sigma1 = f->GetParameter(2);


	const double c2 = f->GetParameter(3);


	const double sigma2 = (c1 != 0.0) ? sigma1 * c2 / c1 : 0.0;


	const double fwhm1 = 2.35482 * sigma1;


	const double fwhm2 = 2.35482 * sigma2;


	const double fwhm_avg = 0.5 * (fwhm1 + fwhm2);


    
	// ========================================================
    
	// 11) FIT-INFORMATIONEN SPEICHERN
    
	// ========================================================

    
	TString txt;


    
	txt.Form(
			"Automatic peak detection\n"
			"peak=%.6f +- %.6f\n"
			"fitMin=%.6f\n"
			"fitMax=%.6f\n\n"

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

			peak,
			peakError,
			fitMin,
			fitMax,

			f->GetParameter(0),
			f->GetParError(0),

			f->GetParameter(1),
			f->GetParError(1),

			f->GetParameter(2),
			f->GetParError(2),

			f->GetParameter(3),
			f->GetParError(3),

			f->GetParameter(4),
			f->GetParError(4),

			f->GetParameter(5),
			f->GetParError(5),

			sigma2,
			fwhm1,
			fwhm2,
			fwhm_avg);


	TString infoName = Form("%s_fitinfo", histname);


	TString fullOld = go4->FindItem(infoName);


	if(!fullOld.IsNull())
	{
		TObject* old = go4->GetObject(fullOld,1);

		if(old) { delete old; }
	}


	TNamed* info = new TNamed(infoName,txt);


	go4->SaveToMemory("Fits",info,kTRUE);

	// ========================================================
	// 12) AUSGABE
	// ========================================================

	std::cout
		<< "\n";
	std::cout
		<< "===== FIT RESULTS =====\n";
	std::cout
		<< txt.Data()
		<< std::endl;


	// ========================================================
	// 13) ZEICHNEN
	// ========================================================

	if(draw)
	{
		TCanvas* c = new TCanvas(Form("fit_canvas_%s",histname),"Fit",900,600);


		hfit->Draw();


		f->Draw("same");


		c->Update();
	}

	// ========================================================
	// 10) HISTOGRAMM SPEICHERN
	// ========================================================

	TString rname = go4->SaveToMemory("Fits",hfit,kTRUE);


	std::cout
		<< "Saved histogram to: "
		<< rname
		<< std::endl;




	return kTRUE;
}
