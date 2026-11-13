#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2.h>
#include <vector>


#include <Math/MinimizerOptions.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TF1.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TMath.h>
#include <TLine.h>
#include <TLegend.h>
#include <TString.h>
#include <TDirectory.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>


std::string gInputFile = "/d/d04-1/ag_pi/QCLAM_EEPG_CE140_2026/ana/unpacked_data/calibrations_go4/calibrations/timewalk/timewalk_corrected_run_34_40.root";
std::string gInputTree = "DrifttimeCalib_TWcorr";


double gReferenceTime = 50.;

// == fit params ==
int gMinEntriesSkewedGaussFit = 50;

bool gUseAutomaticFitXLimits = true;
double gFitMin = 20.; // falls nicht automatische grenzen (driftzeit)
double gFitMax = 100.; // falls nicht automatische grenzen (driftzeit)





// Schiefsymmetrische Gaußfunktion.
// Parameter: A, mu, sigma, alpha
Double_t SkewedGauss(Double_t* x, Double_t* p)
{
	const double A     = p[0];
	const double mu    = p[1];
	const double sigma = p[2];
	const double alpha = p[3];

	if(sigma <= 0.0) { return 0.0; }

	const double z = (x[0] - mu) / sigma;

	return A * std::exp(-0.5 * z * z) * (1.0 + TMath::Erf(alpha * z / std::sqrt(2.0)));
}




// Eine schiefsymmetrische Gaußfunktion.
// Parameter: A, mu, sigma, s
Double_t SkewedGaussComponent(Double_t x, const Double_t* p)
{
	const double A     = p[0];
	const double mu    = p[1];
	const double sigma = p[2];
	const double s     = p[3];

	if(sigma <= 0.0) { return 0.0; }

	const double z = (x - mu) / sigma;

	return A / (std::sqrt(2.0 * TMath::Pi()) * sigma)
		* std::exp(-0.5 * z * z)
		* (1.0 + TMath::Erf(s * z / std::sqrt(2.0)));
}


// Summe zweier schiefsymmetrischer Gaußfunktionen.
// Parameter:
// 0..3: erster Peak  (A1, mu1, sigma1, s1)
// 4..7: zweiter Peak (A2, mu2, sigma2, s2)
Double_t DoubleSkewedGauss(Double_t* x, Double_t* p)
{
	return SkewedGaussComponent(x[0], &p[0])
		+ SkewedGaussComponent(x[0], &p[4]);
}





// Numerische Bestimmung des Maximums der Fitfunktion.
double GetFitMaximum(TF1* fit, double xmin, double xmax)
{
	if(!fit || xmax <= xmin) { return NAN; }

	double xmaxFit = NAN;
	double ymaxFit = -1.0;

	const int nSteps = 20000;

	for(int i = 0; i <= nSteps; ++i)
	{
		const double x = xmin + (xmax - xmin) * i / nSteps;
		const double y = fit->Eval(x);

		if(std::isfinite(y) && y > ymaxFit)
		{
			ymaxFit = y;
			xmaxFit = x;
		}
	}

	return xmaxFit;
}









void CalibrateLayer(
		TH2D* hDriftRaw,
		TH2D* hMinRaw,
		TH2D* hDriftCorr,
		TH2D* hMinCorr,
		const char* layerName,
		TFile* fout,
		int nWires)
{
    
	if(!hDriftCorr || !hMinCorr || !fout)
	{
		std::cerr << "Fehler: Histogramm oder Ausgabedatei fehlt für " << layerName << '\n';
		return;
	}

	const double referenceTime = gReferenceTime;



	const TString pdfName = TString::Format("timewalk_%s.pdf", layerName);

	bool firstPdfPage = true;

	auto PrintCanvasToPDF = [&](TCanvas* canvas, bool lastPage = false)
	{
		if(!canvas) return;

		TString outputName = pdfName;

		if(firstPdfPage)
		{
			outputName += "(";
			firstPdfPage = false;
		}
		else if(lastPage)
		{
			outputName += ")";
		}

		canvas->Print(outputName);
	};



	std::vector<double> offsets(nWires, 0.0);
	std::vector<double> maxima(nWires, NAN);
	std::vector<bool> valid(nWires, false);

	const TString txtName = TString::Format("offsets_%s.txt", layerName);
	std::ofstream txt(txtName.Data());

	if(!txt)
	{
		std::cerr << "Kann " << txtName << " nicht schreiben.\n";
		return;
	}

    
	txt << "# wire offset/ns\n";

    
	fout->cd();

    
	// Histogramme nach Anwendung der drahtweisen Offsets.
    
	TH2D* hDriftAfter = (TH2D*)hDriftCorr->Clone(TString::Format("DrifttimeVsWireCorr_%s_After", layerName));
	hDriftAfter->Reset();

	hDriftAfter->SetTitle(TString::Format("%s: drift time after wire-offset correction;Wire number;Drift time [ns]",layerName));

	TH2D* hMinAfter = (TH2D*)hMinCorr->Clone(TString::Format("MinDrifttimeVsWireCorr_%s_After", layerName));
	hMinAfter->Reset();

	hMinAfter->SetTitle(TString::Format("%s: minimum drift time after wire-offset correction;Wire number;Minimum drift time [ns]",layerName));



	// --------------------------------------------------------
	// Fit jeder Drahtprojektion
	// --------------------------------------------------------

	for(int wire = 0; wire < nWires; ++wire)
	{
		// Bin in der X-Achse, dessen Mittelpunkt dem Draht entspricht.
		const int wireBin = hMinCorr->GetXaxis()->FindBin(wire + 0.5);

		if(wireBin < 1 || wireBin > hMinCorr->GetNbinsX())
		{
			continue;
		}

		TH1D* proj = hMinCorr->ProjectionY(TString::Format("Projection_%s_wire_%d", layerName, wire),wireBin,wireBin);

        
		proj->SetDirectory(nullptr);

        
		if(proj->GetEntries() < gMinEntriesSkewedGaussFit)
		{
			std::cerr << layerName << " Draht " << wire << ": zu wenige Einträge, kein Fit.\n";
			fout->cd();
			proj->Write();
			delete proj;
			continue;
		}

		// Fitbereich aus dem belegten Histogrammbereich bestimmen.
		const int first = proj->FindFirstBinAbove(0);
		const int last  = proj->FindLastBinAbove(0);

		if(first <= 0 || last <= first)
		{
			delete proj;
			continue;
		}

		double xmin = gFitMin;
		double xmax = gFitMax;
		if(gUseAutomaticFitXLimits)
		{
			xmin = proj->GetXaxis()->GetBinLowEdge(first);
			xmax = proj->GetXaxis()->GetBinUpEdge(last);
		}

		// ----------------------------------------------------
		// Datenabhängige Startwerte für die doppelte
		// schiefsymmetrische Gaußfunktion
		// ----------------------------------------------------

        
		const int maxBin = proj->GetMaximumBin();
        
		const double mu1_0 = proj->GetBinCenter(maxBin);
        
		const double peak1 = proj->GetBinContent(maxBin);

		// Entsprechend dem Python-Skript:
		// Der zweite Peak wird rechts von mu1 + 3*sigma0 gesucht.
		const double sigma0 = 15.0;
		const double secondSearchMin = mu1_0 + 3.0 * sigma0;

		double mu2_0 = mu1_0 + 3.0 * sigma0;
		double peak2 = 0.0;

		for(int bin = 1; bin <= proj->GetNbinsX(); ++bin)
		{
			const double xbin = proj->GetBinCenter(bin);
			const double ybin = proj->GetBinContent(bin);

			if(xbin <= secondSearchMin) { continue; }

			if(xbin < xmin || xbin > xmax) { continue; }

			if(ybin > peak2)
			{
				peak2 = ybin;
				mu2_0 = xbin;
			}
		}

		// Falls rechts kein zweiter Peak gefunden wurde,
		// eine alternative Startposition verwenden.
		if(peak2 <= 0.0)
		{
			mu2_0 = std::min(xmax, mu1_0 + 3.0 * sigma0);
			peak2 = std::max(1.0, peak1 * 0.1);
		}

		// Amplitudenstartwerte passend zur Normierung
		// der schiefsymmetrischen Gaußfunktion.
		const double A1_0 = std::max(1.0, peak1 * std::sqrt(2.0 * TMath::Pi()) * sigma0);

		const double A2_0 = std::max(1.0, peak2 * std::sqrt(2.0 * TMath::Pi()) * sigma0);

		TF1* fit = new TF1(
				TString::Format("SkewFit_%s_wire_%d", layerName, wire),
				DoubleSkewedGauss,
				xmin,
				xmax,
				8);

		fit->SetParNames(
				"A1", "mu1", "sigma1", "s1",
				"A2", "mu2", "sigma2", "s2"
				);

        
		fit->SetParameters(
				A1_0, mu1_0, sigma0, 0.0,
				A2_0, mu2_0, sigma0, 0.0
				);

		// Parametergrenzen wie bei bastian.
		fit->SetParLimits(0, 0.0, 1.e12);
		fit->SetParLimits(1, xmin, xmax);
		fit->SetParLimits(2, 5.0, 100.0);
		fit->SetParLimits(3, -5.0, 5.0);

		fit->SetParLimits(4, 0.0, 1.e12);
		fit->SetParLimits(5, xmin, xmax);
		fit->SetParLimits(6, 5.0, 100.0);
		fit->SetParLimits(7, -5.0, 5.0);

		// ROOT-Fit mit Minuit2/Migrad.
		TFitResultPtr fitResult = proj->Fit(fit, "QRS0");

		const int fitStatus = static_cast<int>(fitResult);
		const double maximum = GetFitMaximum(fit, xmin, xmax);

		if(/*fitStatus == 0 &&*/ std::isfinite(maximum))
		{
			maxima[wire] = maximum;
			offsets[wire] = referenceTime - maximum;
			valid[wire] = true;
			txt << wire << '\t' << offsets[wire] << '\n';

			std::cout << layerName << " wire " << wire
				<< ": maximum = " << maximum
				<< " ns, offset = " << offsets[wire]
				<< " ns, fit status = " << fitStatus;

			if(fitResult.Get())
			{
				std::cout << ", chi2/ndf = "
					<< fitResult->Chi2() << "/"
					<< fitResult->Ndf();
			}

			std::cout << '\n';
		} else {
			std::cerr << layerName << " wire " << wire
				<< ": Fit fehlgeschlagen, Status = "
				<< fitStatus << '\n';
		}


        
		// Für jeden Draht ein Canvas mit Daten und Fit.
		TCanvas* cFit = new TCanvas(
				TString::Format("c_fit_%s_wire_%d", layerName, wire),
				TString::Format("%s wire %d", layerName, wire),
				1000, 700
				);

		proj->SetTitle(TString::Format("%s wire %d: minimum drift-time distribution;Minimum drift time [ns];Events",layerName, wire));

		proj->SetMarkerStyle(20);
		proj->SetMarkerSize(0.5);
		proj->Draw("E");

		fit->SetLineColor(kRed);
		fit->SetLineWidth(2);
		fit->Draw("SAME");

		if(valid[wire])
		{
			TLine* reference = new TLine(referenceTime, 0,referenceTime, proj->GetMaximum() * 1.05);
			reference->SetLineStyle(2);
			reference->SetLineColor(kGreen + 2);
			reference->Draw("SAME");
		}

		fout->cd();
		cFit->Write();
		proj->Write();
		fit->Write();
		PrintCanvasToPDF(cFit);

		delete cFit;
		delete proj;
		delete fit;
	}

	txt.close();

	// --------------------------------------------------------
	// Verschobene 2D-Histogramme erzeugen
	// --------------------------------------------------------
	//
	// Jeder Eintrag aus einem Draht wird um dessen Offset
	// entlang der Driftzeitachse verschoben.
	// Ungültige Fits lassen den jeweiligen Draht unverändert.
	//
	// Die Verschiebung erfolgt binweise anhand der ursprünglichen
	// Histogramme. Dadurch bleibt die Gesamtzahl der Einträge
	// erhalten, soweit die verschobenen Zeiten im Histogrammbereich
	// liegen.
	auto ShiftHistogram = [&](TH2D* source, TH2D* destination)
	{

		for (int xb = 1; xb <= source->GetNbinsX(); ++xb)
		{

			const int wire = static_cast<int>(std::floor(source->GetXaxis()->GetBinCenter(xb)));

			double offset = 0.0;

			if(wire >= 0 && wire < nWires && valid[wire]) { offset = offsets[wire]; }

			for(int yb = 1; yb <= source->GetNbinsY(); ++yb)
			{

				const double count = source->GetBinContent(xb, yb);

				if(count == 0.0) { continue; }

				const double oldTime = source->GetYaxis()->GetBinCenter(yb);
				const double newTime = oldTime + offset;

				const int newYBin = destination->GetYaxis()->FindBin(newTime);

				if(newYBin < 1 || newYBin > destination->GetNbinsY()) { continue; }

				destination->AddBinContent(destination->GetBin(xb, newYBin), count);
			}
		}

		destination->SetEntries(source->GetEntries());

	};

	ShiftHistogram(hDriftCorr, hDriftAfter);
	ShiftHistogram(hMinCorr, hMinAfter);

	std::cout << "\n" << layerName << " Histogramm-Check:\n";

	std::cout << "Min BEFORE: entries = "
		<< hMinCorr->GetEntries()
		<< ", integral = " << hMinCorr->Integral()
		<< '\n';

	std::cout << "Min AFTER:  entries = "
		<< hMinAfter->GetEntries()
		<< ", integral = " << hMinAfter->Integral()
		<< '\n';

	std::cout << "Drift BEFORE: entries = "
		<< hDriftCorr->GetEntries()
		<< ", integral = " << hDriftCorr->Integral()
		<< '\n';

	std::cout << "Drift AFTER:  entries = "
		<< hDriftAfter->GetEntries()
		<< ", integral = " << hDriftAfter->Integral()
		<< '\n';





	// == Kontrollplots ==
	TCanvas* cMinRaw = new TCanvas(
			TString::Format("c_MinDrifttimeVsWire_%s_Raw", layerName),
			TString::Format("%s raw", layerName),
			1200, 800
			);

	hMinRaw->SetTitle(
			TString::Format("%s minimum drift time before timewalk correction (raw);"
				"Wire number;Minimum drift time [ns]",
				layerName)
			);
	hMinRaw->Draw("COLZ");




	TCanvas* cBefore = new TCanvas(
			TString::Format("c_MinDrifttimeVsWire_%s_Before", layerName),
			TString::Format("%s before correction", layerName),
			1200, 800
			);

	hMinCorr->SetTitle(
			TString::Format("%s minimum drift time before correction;"
				"Wire number;Minimum drift time [ns]",
				layerName)
			);
	hMinCorr->Draw("COLZ");

	TCanvas* cAfter = new TCanvas(
			TString::Format("c_MinDrifttimeVsWire_%s_After", layerName),
			TString::Format("%s after correction", layerName),
			1200, 800
			);

	hMinAfter->Draw("COLZ");




	TCanvas* cDriftRaw = new TCanvas(
			TString::Format("c_DrifttimeVsWire_%s_Raw", layerName),
			TString::Format("%s raw", layerName),
			1200, 800
			);

	hDriftRaw->SetTitle(
			TString::Format("%s drift time before timewalk correction (raw);"
				"Wire number;drift time [ns]",
				layerName)
			);
	hDriftRaw->Draw("COLZ");



	TCanvas* cDriftBefore = new TCanvas(
			TString::Format("c_DrifttimeVsWire_%s_Before", layerName),
			TString::Format("%s drift time before correction", layerName),
			1200, 800
			);

	hDriftCorr->Draw("COLZ");

	TCanvas* cDriftAfter = new TCanvas(
			TString::Format("c_DrifttimeVsWire_%s_After", layerName),
			TString::Format("%s drift time after correction", layerName),
			1200, 800
			);

	hDriftAfter->Draw("COLZ");




	// Fitmaximum gegen Drahtnummer vor/nach Korrektur.
	std::vector<double> wireNumbers;
	std::vector<double> maximaBefore;
	std::vector<double> maximaAfter;

	for(int wire = 0; wire < nWires; ++wire)
	{
		if(!valid[wire]) { continue; }

		wireNumbers.push_back(wire);
		maximaBefore.push_back(maxima[wire]);
		maximaAfter.push_back(maxima[wire] + offsets[wire]);
	}

	TGraph* gBefore = new TGraph(
			static_cast<int>(wireNumbers.size()),
			wireNumbers.data(),
			maximaBefore.data()
			);

	TGraph* gAfter = new TGraph(
			static_cast<int>(wireNumbers.size()),
			wireNumbers.data(),
			maximaAfter.data()
			);

	gBefore->SetName(TString::Format("FitMaximumBefore_%s", layerName));
	gAfter->SetName(TString::Format("FitMaximumAfter_%s", layerName));

	gBefore->SetTitle(
			TString::Format("%s fit maximum before correction;"
				"Wire number;Fit maximum [ns]",
				layerName)
			);

	gAfter->SetTitle(
			TString::Format("%s fit maximum after correction;"
				"Wire number;Fit maximum [ns]",
				layerName)
			);

	TCanvas* cMaxBefore = new TCanvas(
			TString::Format("c_FitMaximum_%s_Before", layerName),
			"Fit maximum before correction", 1200, 700
			);

	gBefore->SetMarkerStyle(20);
	gBefore->Draw("AP");

	TLine* lineBefore = new TLine(0, referenceTime, nWires, referenceTime);
	lineBefore->SetLineStyle(2);
	lineBefore->Draw("SAME");

	TCanvas* cMaxAfter = new TCanvas(
			TString::Format("c_FitMaximum_%s_After", layerName),
			"Fit maximum after correction", 1200, 700);

	gAfter->SetMarkerStyle(20);
	gAfter->Draw("AP");

	TLine* lineAfter = new TLine(0, referenceTime, nWires, referenceTime);
	lineAfter->SetLineStyle(2);
	lineAfter->Draw("SAME");

	// Alles in der ROOT-Datei ablegen.
	fout->cd();

	hDriftAfter->Write();
	hMinAfter->Write();

	cMinRaw->Write();
	cBefore->Write();
	cAfter->Write();
	cDriftRaw->Write();
	cDriftBefore->Write();
	cDriftAfter->Write();
	cMaxBefore->Write();
	cMaxAfter->Write();

	gBefore->Write();
	gAfter->Write();

	PrintCanvasToPDF(cMinRaw);
	PrintCanvasToPDF(cBefore);
	PrintCanvasToPDF(cAfter);
	PrintCanvasToPDF(cDriftRaw);
	PrintCanvasToPDF(cDriftBefore);
	PrintCanvasToPDF(cDriftAfter);
	PrintCanvasToPDF(cMaxBefore);

	// Letzte Seite: PDF schließen
	PrintCanvasToPDF(cMaxAfter, true);

	delete cMinRaw;
	delete cBefore;
	delete cAfter;
	delete cDriftRaw;
	delete cDriftBefore;
	delete cDriftAfter;
	delete cMaxBefore;
	delete cMaxAfter;

	delete lineBefore;
	delete lineAfter;
	delete gBefore;
	delete gAfter;

	std::cout << "Offsets gespeichert in " << txtName << '\n';
}







/*
root [2] DrifttimeCalib_TWcorr->Show()
======> EVENT:-1
 layerID         = NULL
 wireNum         = NULL
 tot             = NULL
 t_raw           = NULL
 t_tw_corr       = NULL
root [3] 
*/

void calc_offset_after_timewalk()
{
	ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Migrad");
	ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(300000);
	ROOT::Math::MinimizerOptions::SetDefaultMaxIterations(30000);
	ROOT::Math::MinimizerOptions::SetDefaultTolerance(0.05);


	TFile* fin = TFile::Open(gInputFile.c_str()); //"/d/d04-1/ag_pi/QCLAM_EEPG_CE140_2026/ana/unpacked_data/calibrations_go4/calibrations/timewalk/timewalk_corrected_run_34_40.root");

	TTree* tin = (TTree*)fin->Get(gInputTree.c_str()); //"DrifttimeCalib_TWcorr");

	std::vector<int>* layerID = nullptr;
	std::vector<int>* wireNum = nullptr;
	std::vector<double>* tot = nullptr;
	std::vector<double>* t_raw = nullptr;
	std::vector<double>* t_tw_corr = nullptr;

	tin->SetBranchAddress("layerID", &layerID);
	tin->SetBranchAddress("wireNum", &wireNum);
	tin->SetBranchAddress("tot", &tot);
	tin->SetBranchAddress("t_raw", &t_raw);
	tin->SetBranchAddress("t_tw_corr", &t_tw_corr);

	std::vector<std::vector<int>> layerIDs, wireNums;
	std::vector<std::vector<double>> tots, t_raws, t_tw_corrs;

	std::vector<int> minDT_layerID, minDT_wireNum;
	std::vector<double> minDT_tot, minDT_t_raw, minDT_t_tw_corr;


	// driftzeit unkorrigiert
	TH2D* DrifttimeVsWireRaw_X1 = new TH2D("DrifttimeVsWireRaw_X1","DrifttimeVsWireRaw_X1",320,0,320,500,0,500);
	DrifttimeVsWireRaw_X1->GetXaxis()->SetTitle("Drahtnummer");
	DrifttimeVsWireRaw_X1->GetXaxis()->CenterTitle();
	DrifttimeVsWireRaw_X1->GetYaxis()->SetTitle("Driftzeit / ns");
	DrifttimeVsWireRaw_X1->GetYaxis()->CenterTitle();

	TH2D* DrifttimeVsWireRaw_U = new TH2D("DrifttimeVsWireRaw_U","DrifttimeVsWireRaw_U",256,0,256,500,0,500);
	DrifttimeVsWireRaw_U->GetXaxis()->SetTitle("Drahtnummer");
	DrifttimeVsWireRaw_U->GetXaxis()->CenterTitle();
	DrifttimeVsWireRaw_U->GetYaxis()->SetTitle("Driftzeit / ns");
	DrifttimeVsWireRaw_U->GetYaxis()->CenterTitle();


	TH2D* DrifttimeVsWireRaw_X2 = new TH2D("DrifttimeVsWireRaw_X2","DrifttimeVsWireRaw_X2",320,0,320,500,0,500);
	DrifttimeVsWireRaw_X2->GetXaxis()->SetTitle("Drahtnummer");
	DrifttimeVsWireRaw_X2->GetXaxis()->CenterTitle();
	DrifttimeVsWireRaw_X2->GetYaxis()->SetTitle("Driftzeit / ns");
	DrifttimeVsWireRaw_X2->GetYaxis()->CenterTitle();


	// driftzeit korrigiert
	TH2D* DrifttimeVsWireCorr_X1 = new TH2D("DrifttimeVsWireCorr_X1","DrifttimeVsWireCorr_X1",320,0,320,500,0,500);
	DrifttimeVsWireCorr_X1->GetXaxis()->SetTitle("Drahtnummer");
	DrifttimeVsWireCorr_X1->GetXaxis()->CenterTitle();
	DrifttimeVsWireCorr_X1->GetYaxis()->SetTitle("Driftzeit / ns");
	DrifttimeVsWireCorr_X1->GetYaxis()->CenterTitle();

	TH2D* DrifttimeVsWireCorr_U = new TH2D("DrifttimeVsWireCorr_U","DrifttimeVsWireCorr_U",256,0,256,500,0,500);
	DrifttimeVsWireCorr_U->GetXaxis()->SetTitle("Drahtnummer");
	DrifttimeVsWireCorr_U->GetXaxis()->CenterTitle();
	DrifttimeVsWireCorr_U->GetYaxis()->SetTitle("Driftzeit / ns");
	DrifttimeVsWireCorr_U->GetYaxis()->CenterTitle();


	TH2D* DrifttimeVsWireCorr_X2 = new TH2D("DrifttimeVsWireCorr_X2","DrifttimeVsWireCorr_X2",320,0,320,500,0,500);
	DrifttimeVsWireCorr_X2->GetXaxis()->SetTitle("Drahtnummer");
	DrifttimeVsWireCorr_X2->GetXaxis()->CenterTitle();
	DrifttimeVsWireCorr_X2->GetYaxis()->SetTitle("Driftzeit / ns");
	DrifttimeVsWireCorr_X2->GetYaxis()->CenterTitle();



	// min driftzeit unkorrigiert
	TH2D* MinDrifttimeVsWireRaw_X1 = new TH2D("MinDrifttimeVsWireRaw_X1","MinDrifttimeVsWireRaw_X1",320,0,320,500,0,500);
	MinDrifttimeVsWireRaw_X1->GetXaxis()->SetTitle("Drahtnummer");
	MinDrifttimeVsWireRaw_X1->GetXaxis()->CenterTitle();
	MinDrifttimeVsWireRaw_X1->GetYaxis()->SetTitle("Min. Driftzeit / ns");
	MinDrifttimeVsWireRaw_X1->GetYaxis()->CenterTitle();

	TH2D* MinDrifttimeVsWireRaw_U = new TH2D("MinDrifttimeVsWireRaw_U","MinDrifttimeVsWireRaw_U",256,0,256,500,0,500);
	MinDrifttimeVsWireRaw_U->GetXaxis()->SetTitle("Drahtnummer");
	MinDrifttimeVsWireRaw_U->GetXaxis()->CenterTitle();
	MinDrifttimeVsWireRaw_U->GetYaxis()->SetTitle("Min. Driftzeit / ns");
	MinDrifttimeVsWireRaw_U->GetYaxis()->CenterTitle();


	TH2D* MinDrifttimeVsWireRaw_X2 = new TH2D("MinDrifttimeVsWireRaw_X2","MinDrifttimeVsWireRaw_X2",320,0,320,500,0,500);
	MinDrifttimeVsWireRaw_X2->GetXaxis()->SetTitle("Drahtnummer");
	MinDrifttimeVsWireRaw_X2->GetXaxis()->CenterTitle();
	MinDrifttimeVsWireRaw_X2->GetYaxis()->SetTitle("Min. Driftzeit / ns");
	MinDrifttimeVsWireRaw_X2->GetYaxis()->CenterTitle();


	// min driftzeit korrigiert
	TH2D* MinDrifttimeVsWireCorr_X1 = new TH2D("MinDrifttimeVsWireCorr_X1","MinDrifttimeVsWireCorr_X1",320,0,320,500,0,500);
	MinDrifttimeVsWireCorr_X1->GetXaxis()->SetTitle("Drahtnummer");
	MinDrifttimeVsWireCorr_X1->GetXaxis()->CenterTitle();
	MinDrifttimeVsWireCorr_X1->GetYaxis()->SetTitle("Min. Driftzeit / ns");
	MinDrifttimeVsWireCorr_X1->GetYaxis()->CenterTitle();

	TH2D* MinDrifttimeVsWireCorr_U = new TH2D("MinDrifttimeVsWireCorr_U","MinDrifttimeVsWireCorr_U",256,0,256,500,0,500);
	MinDrifttimeVsWireCorr_U->GetXaxis()->SetTitle("Drahtnummer");
	MinDrifttimeVsWireCorr_U->GetXaxis()->CenterTitle();
	MinDrifttimeVsWireCorr_U->GetYaxis()->SetTitle("Min. Driftzeit / ns");
	MinDrifttimeVsWireCorr_U->GetYaxis()->CenterTitle();


	TH2D* MinDrifttimeVsWireCorr_X2 = new TH2D("MinDrifttimeVsWireCorr_X2","MinDrifttimeVsWireCorr_X2",320,0,320,500,0,500);
	MinDrifttimeVsWireCorr_X2->GetXaxis()->SetTitle("Drahtnummer");
	MinDrifttimeVsWireCorr_X2->GetXaxis()->CenterTitle();
	MinDrifttimeVsWireCorr_X2->GetYaxis()->SetTitle("Min. Driftzeit / ns");
	MinDrifttimeVsWireCorr_X2->GetYaxis()->CenterTitle();


	for(int i=0; i<tin->GetEntries(); i++)
	{
		tin->GetEntry(i);
		if(!layerID || !wireNum || !tot || !t_raw || !t_tw_corr)
		{
			continue;
		}

		layerIDs.push_back(*layerID);
		wireNums.push_back(*wireNum);
		tots.push_back(*tot);

		int n = layerID->size();

		// Prüfen, ob alle Branch-Vektoren dieselbe Länge haben
		if(wireNum->size() != n || tot->size() != n || t_raw->size() != n || t_tw_corr->size() != n)
		{
			std::cerr << "Fehler: Unterschiedliche Vektorlängen in Event " << i << std::endl;
			continue;
		}

		// ------------------------------------------------------------
		// 1. Alle Treffer in die normalen Driftzeit-Histogramme füllen
		// ------------------------------------------------------------
		for(int j = 0; j < n; j++)
		{
			int id = layerID->at(j);

			if(id == 0)
			{
				DrifttimeVsWireRaw_X1->Fill(
						wireNum->at(j), t_raw->at(j));

				DrifttimeVsWireCorr_X1->Fill(
						wireNum->at(j), t_tw_corr->at(j));
			}
			else if(id == 1)
			{
				DrifttimeVsWireRaw_U->Fill(
						wireNum->at(j), t_raw->at(j));

				DrifttimeVsWireCorr_U->Fill(
						wireNum->at(j), t_tw_corr->at(j));
			}
			else if(id == 2)
			{
				DrifttimeVsWireRaw_X2->Fill(
						wireNum->at(j), t_raw->at(j));

				DrifttimeVsWireCorr_X2->Fill(
						wireNum->at(j), t_tw_corr->at(j));
			}
		}

		// ------------------------------------------------------------
		// 2. Für jede Ebene den Treffer mit minimaler korrigierter
		//    Driftzeit bestimmen
		// ------------------------------------------------------------

		// Ebenen: 0 = X1, 1 = U, 2 = X2
		int minIndex[3] = {-1, -1, -1};

		for(int j = 0; j < n; j++)
		{
			int id = layerID->at(j);

			// Nur bekannte Ebenen berücksichtigen
			if(id < 0 || id > 2)
			{
				continue;
			}

			// Erster Treffer dieser Ebene oder neue minimale Driftzeit?
			if(minIndex[id] == -1 || t_tw_corr->at(j) < t_tw_corr->at(minIndex[id]))
			{
				minIndex[id] = j;
			}
		}

		// ------------------------------------------------------------
		// 3. Minimum jeder vorhandenen Ebene in die Histogramme füllen
		// ------------------------------------------------------------

		for(int id = 0; id < 3; id++)
		{
			int j = minIndex[id];

			// In diesem Event gab es keinen Treffer in dieser Ebene
			if(j == -1)
			{
				continue;
			}

			int minWireNum = wireNum->at(j);
			double minRaw  = t_raw->at(j);
			double minCorr = t_tw_corr->at(j);
			double minTot  = tot->at(j);

			minDT_layerID.push_back(id);
			minDT_wireNum.push_back(minWireNum);
			minDT_tot.push_back(minTot);
			minDT_t_raw.push_back(minRaw);
			minDT_t_tw_corr.push_back(minCorr);

			if(id == 0)
			{
				MinDrifttimeVsWireRaw_X1->Fill(minWireNum, minRaw);
				MinDrifttimeVsWireCorr_X1->Fill(minWireNum, minCorr);
			}
			else if(id == 1)
			{
				MinDrifttimeVsWireRaw_U->Fill(minWireNum, minRaw);
				MinDrifttimeVsWireCorr_U->Fill(minWireNum, minCorr);
			}
			else if(id == 2)
			{
				MinDrifttimeVsWireRaw_X2->Fill(minWireNum, minRaw);
				MinDrifttimeVsWireCorr_X2->Fill(minWireNum, minCorr);
			}
		}
	}


	TFile* fout = new TFile("timewalk_correction.root","RECREATE");

	DrifttimeVsWireRaw_X1->Write();
	DrifttimeVsWireCorr_X1->Write();
	DrifttimeVsWireRaw_U->Write();
	DrifttimeVsWireCorr_U->Write();
	DrifttimeVsWireRaw_X2->Write();
	DrifttimeVsWireCorr_X2->Write();

	MinDrifttimeVsWireRaw_X1->Write();
	MinDrifttimeVsWireCorr_X1->Write();
	MinDrifttimeVsWireRaw_U->Write();
	MinDrifttimeVsWireCorr_U->Write();
	MinDrifttimeVsWireRaw_X2->Write();
	MinDrifttimeVsWireCorr_X2->Write();
	fout->Close();


	delete fout;

	// Datei für Fits, Offsets und Kontrollplots.
	TFile* fcal = new TFile("calibrated_time_offsets.root", "RECREATE");

	CalibrateLayer(
			DrifttimeVsWireRaw_X1,
			MinDrifttimeVsWireRaw_X1,
			DrifttimeVsWireCorr_X1,
			MinDrifttimeVsWireCorr_X1,
			"X1", fcal, 320
			);

	CalibrateLayer(
			DrifttimeVsWireRaw_U,
			MinDrifttimeVsWireRaw_U,
			DrifttimeVsWireCorr_U,
			MinDrifttimeVsWireCorr_U,
			"U", fcal, 256
			);

	CalibrateLayer(
			DrifttimeVsWireRaw_X2,
			MinDrifttimeVsWireRaw_X2,
			DrifttimeVsWireCorr_X2,
			MinDrifttimeVsWireCorr_X2,
			"X2", fcal, 320
			);

	fcal->Close();
	delete fcal;

	std::cout << "Kalibrierung abgeschlossen.\n";
	std::cout << "ROOT-Ausgabe: calibrated_time_offsets.root\n";
	std::cout << "TXT-Ausgaben: offsets_X1.txt, offsets_U.txt, offsets_X2.txt\n";


}
