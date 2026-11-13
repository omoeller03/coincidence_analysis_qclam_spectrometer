// Nachkalibrierung eines einzelnen Drahtes.
//
// root -l -q 'calc_single_wire_offset_after_timewalk.C("X1",137)'
//
// Draehte von 0 ... 319 bzw. 255

#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TLine.h>
#include <TLegend.h>
#include <TMath.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <Math/MinimizerOptions.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

// ------------------------------------------------------------
// Konfiguration
// ------------------------------------------------------------

const char* gInputFile = "/d/d04-1/ag_pi/QCLAM_EEPG_CE140_2026/ana/unpacked_data/calibrations_go4/calibrations/timewalk/timewalk_corrected_run_34_40.root";
const char* gTreeName = "DrifttimeCalib_TWcorr";

const double gReferenceTime = 50.0;
const double gFitMin = 20.0;
const double gFitMax = 100.0;
int gMinEntriesSkewedGaussFit = 50;





// Skewed Gauss Komponente
Double_t SkewedGaussComponentSingle(Double_t x,const Double_t* p)
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

// Doppelter Skewed Gauss
Double_t DoubleSkewedGaussSingle(Double_t* x,Double_t* p)
{
	return SkewedGaussComponentSingle(x[0], &p[0])
		+ SkewedGaussComponentSingle(x[0], &p[4]);
}





// maximum des Skewed Gauss finden
double GetFitMaximumSingle(TF1* fit,double xmin,double xmax)
{
	if(!fit || xmax <= xmin) { return NAN; }

	double maximumX = NAN;
	double maximumY = -1.0;

	const int nSteps = 20000;

	for(int i = 0; i <= nSteps; ++i)
	{
		const double x = xmin + (xmax - xmin) * i / nSteps;

		const double y = fit->Eval(x);

		if(std::isfinite(y) && y > maximumY)
		{
			maximumY = y;
			maximumX = x;
		}
	}

	return maximumX;
}





// main
void calc_single_wire_offset_after_timewalk(const char* layerName = "X1",int targetWire = 0)
{
	ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Migrad");
	ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(300000);
	ROOT::Math::MinimizerOptions::SetDefaultMaxIterations(30000);
	ROOT::Math::MinimizerOptions::SetDefaultTolerance(0.05);

	// --------------------------------------------------------
	// Ebene ueberpruefen
	// --------------------------------------------------------
	int targetLayerID = -1;
	int nWires = 0;
	const TString layer(layerName);

	if(layer == "X1")
	{
		targetLayerID = 0;
		nWires = 320;
	}
	else if (layer == "U")
	{
		targetLayerID = 1;
		nWires = 256;
	}
	else if (layer == "X2")
	{
		targetLayerID = 2;
		nWires = 320;
	}
	else
	{
		std::cerr
			<< "Fehler: Unbekannte Ebene '" << layerName
			<< "'. Erlaubt sind X1, U und X2.\n";
		return;
	}

	if(targetWire < 0 || targetWire >= nWires)
	{
		std::cerr
			<< "Fehler: Drahtnummer " << targetWire
			<< " liegt außerhalb des gueltigen Bereichs 0 bis "
			<< nWires - 1 << ".\n";
		return;
	}

	std::cout
		<< "\nEinzelkalibrierung\n"
		<< "Ebene:       " << layerName << '\n'
		<< "Draht:       " << targetWire << '\n'
		<< "Referenz:    " << gReferenceTime << " ns\n"
		<< "Fitbereich:  " << gFitMin << " bis "
		<< gFitMax << " ns\n\n";


	// file oeffnen
	TFile* fin = TFile::Open(gInputFile, "READ");

	if(!fin || fin->IsZombie())
	{
		std::cerr
			<< "Fehler: Eingabedatei kann nicht geoeffnet werden:\n"
			<< gInputFile << '\n';

		if(fin) { delete fin; }

		return;
	}

	TTree* tree = dynamic_cast<TTree*>(fin->Get(gTreeName));

	if(!tree)
	{
		std::cerr
			<< "Fehler: Tree '" << gTreeName
			<< "' nicht gefunden.\n";

		fin->Close();
		delete fin;
		return;
	}


	// branches lesen
	std::vector<int>* layerID = nullptr;
	std::vector<int>* wireNum = nullptr;
	std::vector<double>* tot = nullptr;
	std::vector<double>* t_raw = nullptr;
	std::vector<double>* t_tw_corr = nullptr;

	tree->SetBranchAddress("layerID", &layerID);
	tree->SetBranchAddress("wireNum", &wireNum);
	tree->SetBranchAddress("tot", &tot);
	tree->SetBranchAddress("t_raw", &t_raw);
	tree->SetBranchAddress("t_tw_corr", &t_tw_corr);

	// histogramm fuer ausgewaehlten draht holen
	TH1D* hMin = new TH1D(
			TString::Format("MinDrifttime_%s_wire_%d",layerName, targetWire),
			TString::Format("%s wire %d: minimum drift-time distribution;Minimum drift time [ns];Events",layerName, targetWire),
			500, 0.0, 500.0);

	hMin->SetDirectory(nullptr);
	hMin->SetMarkerStyle(20);
	hMin->SetMarkerSize(0.7);


	// suche minimale driftzeit je Event fuer gewaehlte ebene & draht
	Long64_t nFilled = 0;
	Long64_t nBadEvents = 0;

	const Long64_t nEntries = tree->GetEntries();

	for(Long64_t i = 0; i < nEntries; ++i)
	{
		tree->GetEntry(i);

		if(!layerID || !wireNum || !tot || !t_raw || !t_tw_corr)
		{
			++nBadEvents;
			continue;
		}

		const size_t n = layerID->size();

		if(wireNum->size() != n || tot->size() != n || t_raw->size() != n || t_tw_corr->size() != n)
		{
			++nBadEvents;
			continue;
		}

		// Index des Treffers mit minimaler korrigierter
		// Driftzeit fuer jede der drei Ebenen.
		int minIndex[3] = {-1, -1, -1};

		for(size_t j = 0; j < n; ++j)
		{
			const int id = layerID->at(j);

			if(id < 0 || id > 2) { continue; }

			const double t = t_tw_corr->at(j);

			if(!std::isfinite(t)) { continue; }

			if(minIndex[id] == -1 || t < t_tw_corr->at(minIndex[id]))
			{
				minIndex[id] = static_cast<int>(j);
			}
		}

		// Minimum der gewuenschten Ebene pruefen.
		const int j = minIndex[targetLayerID];

		if(j < 0) { continue; }

		if(wireNum->at(j) != targetWire) { continue; }

		const double minTime = t_tw_corr->at(j);

		if(!std::isfinite(minTime)) { continue; }

		hMin->Fill(minTime);
		++nFilled;
	}

	std::cout
		<< "Tree-Eintraege:          " << nEntries << '\n'
		<< "Ungueltige Ereignisse:   " << nBadEvents << '\n'
		<< "Eintraege im Histogramm: " << nFilled << '\n';

	fin->Close();
	delete fin;

	// == Genuegend Daten? ==
	if(hMin->GetEntries() < gMinEntriesSkewedGaussFit)
	{
		std::cerr
			<< "\nZu wenige Eintraege fuer einen zuverlaessigen Fit: "
			<< hMin->GetEntries()
			<< "\nEs werden mindestens 50 Eintraege verlangt.\n";

		TCanvas* c = new TCanvas("c_single_wire","Single-wire calibration",1000, 700);

		hMin->Draw("E");

		c->SaveAs(TString::Format("single_wire_%s_%d_insufficient_statistics.pdf",layerName, targetWire));

		delete c;
		delete hMin;
		return;
	}

	// == Fitbereich pruefen ==
	const double xmin = gFitMin;
	const double xmax = gFitMax;

	const int firstBin = hMin->FindBin(xmin);
	const int lastBin  = hMin->FindBin(xmax);

	if(firstBin < 1 || lastBin > hMin->GetNbinsX() || firstBin >= lastBin)
	{
		std::cerr << "Fehler: Ungueltiger Fitbereich.\n";
		delete hMin;
		return;
	}

	// == Startwerte ==
	const int maxBin = hMin->GetMaximumBin();

	const double mu1_0 = hMin->GetBinCenter(maxBin);
	const double peak1 = hMin->GetBinContent(maxBin);

	const double sigma0 = 15.0;
	const double secondSearchMin = mu1_0 + 3.0 * sigma0;

	double mu2_0 = mu1_0 + 3.0 * sigma0;
	double peak2 = 0.0;

	for(int bin = firstBin; bin <= lastBin; ++bin)
	{
		const double x = hMin->GetBinCenter(bin);
		const double y = hMin->GetBinContent(bin);

		if(x <= secondSearchMin) { continue; }

		if(y > peak2)
		{
			peak2 = y;
			mu2_0 = x;
		}
	}

	if(peak2 <= 0.0)
	{
		mu2_0 = std::min(xmax, mu1_0 + 3.0 * sigma0);
		peak2 = std::max(1.0, peak1 * 0.1);
	}

	const double A1_0 = std::max(1.0,peak1 * std::sqrt(2.0 * TMath::Pi()) * sigma0);

	const double A2_0 = std::max(1.0,peak2 * std::sqrt(2.0 * TMath::Pi()) * sigma0);


	// == Fitfunktion ==
	TF1* fit = new TF1(TString::Format("SkewFit_%s_wire_%d", layerName, targetWire),DoubleSkewedGaussSingle,xmin, xmax, 8);

	fit->SetParNames("A1", "mu1", "sigma1", "s1", "A2", "mu2", "sigma2", "s2");

	fit->SetParameters(A1_0, mu1_0, sigma0, 0.0, A2_0, mu2_0, sigma0, 0.0);

	fit->SetParLimits(0, 0.0, 1.e12);
	fit->SetParLimits(1, xmin, xmax);
	fit->SetParLimits(2, 5.0, 100.0);
	fit->SetParLimits(3, -5.0, 5.0);

	fit->SetParLimits(4, 0.0, 1.e12);
	fit->SetParLimits(5, xmin, xmax);
	fit->SetParLimits(6, 5.0, 100.0);
	fit->SetParLimits(7, -5.0, 5.0);

	TFitResultPtr fitResult = hMin->Fit(fit, "QRS0");

	const int fitStatus = static_cast<int>(fitResult);

	const double maximum = GetFitMaximumSingle(fit, xmin, xmax);

	// == Ergebnis bestimmen ==
	if(/*fitStatus != 0 ||*/ !std::isfinite(maximum))
	{
		std::cerr << "\nFit fehlgeschlagen!\n" << "Fit-Status: " << fitStatus << '\n';

		if(fitResult.Get())
		{
			std::cerr
				<< "Chi2/NDF: "
				<< fitResult->Chi2() << "/"
				<< fitResult->Ndf() << '\n';
		}

		// Trotzdem den Fit zur Diagnose speichern.
		TCanvas* c = new TCanvas("c_single_wire_failed","Single-wire fit diagnostic",1000, 700);

		hMin->Draw("E");
		fit->SetLineColor(kRed);
		fit->Draw("SAME");

		c->SaveAs(TString::Format("single_wire_%s_%d_fit_failed.pdf",layerName, targetWire));

		delete c;
		delete fit;
		delete hMin;
		return;
	}

	const double offset = gReferenceTime - maximum;
	const double correctedMaximum = maximum + offset;

	std::cout
		<< "\n========== Kalibrierungsergebnis ==========\n"
		<< "Ebene:                  " << layerName << '\n'
		<< "Draht:                  " << targetWire << '\n'
		<< "Fitmaximum vorher:      " << maximum << " ns\n"
		<< "Referenzzeit:           " << gReferenceTime << " ns\n"
		<< "Ermittelter Offset:     " << offset << " ns\n"
		<< "Maximum nach Korrektur: " << correctedMaximum << " ns\n"
		<< "Fitstatus:              " << fitStatus << '\n';

	if(fitResult.Get())
	{
		std::cout
			<< "Chi2/NDF:               "
			<< fitResult->Chi2() << "/"
			<< fitResult->Ndf() << '\n';
	}

	std::cout << "===========================================" << std::endl;

	// --------------------------------------------------------
	// Kontrollplot
	// --------------------------------------------------------

	TCanvas* c = new TCanvas(
			TString::Format("c_single_wire_%s_%d",layerName, targetWire),
			TString::Format("%s wire %d",layerName, targetWire),
			1000, 700);

	hMin->SetTitle(TString::Format("%s wire %d: minimum drift-time distribution;Minimum drift time [ns];Events",layerName, targetWire));

	hMin->GetXaxis()->SetRangeUser(xmin, xmax);
	hMin->SetMarkerStyle(20);
	hMin->SetMarkerSize(0.7);
	hMin->Draw("E");

	fit->SetLineColor(kRed);
	fit->SetLineWidth(2);
	fit->Draw("SAME");

	const double ymax = hMin->GetMaximum() * 1.15;

	// Referenzzeit: 50 ns
	TLine* referenceLine = new TLine(gReferenceTime, 0.0,gReferenceTime, ymax);

	referenceLine->SetLineColor(kGreen + 2);
	referenceLine->SetLineStyle(2);
	referenceLine->SetLineWidth(2);
	referenceLine->Draw("SAME");

	// Position des Fitmaximums
	TLine* maximumLine = new TLine(maximum, 0.0,maximum, ymax);

	maximumLine->SetLineColor(kBlue + 1);
	maximumLine->SetLineStyle(3);
	maximumLine->SetLineWidth(2);
	maximumLine->Draw("SAME");

	TLegend* legend = new TLegend(0.55, 0.68, 0.88, 0.88);

	legend->AddEntry(hMin, "Daten", "lep");
	legend->AddEntry(fit, "Doppelter Skewed-Gauss-Fit", "l");
	legend->AddEntry(referenceLine, "Referenz: 50 ns", "l");
	legend->AddEntry(maximumLine,TString::Format("Fitmaximum: %.3f ns", maximum),"l");

	legend->Draw();

	const TString pdfName = TString::Format("single_wire_%s_%d.pdf",layerName, targetWire);

	c->SaveAs(pdfName);


	// Ergebnisse separat speichern
	const TString rootName = TString::Format("single_wire_%s_%d.root",layerName, targetWire);

	TFile* fout = TFile::Open(rootName, "RECREATE");

	if(fout && !fout->IsZombie())
	{
		fout->cd();

		hMin->Write();
		fit->Write();
		c->Write();

		fout->Close();
		delete fout;
	}
	else
	{
		std::cerr << "Warnung: ROOT-Ausgabedatei konnte nicht " << "geschrieben werden." << std::endl;

		if(fout) { delete fout; }
	}

	const TString txtName = TString::Format("single_wire_%s_%d.txt",layerName, targetWire);

	std::ofstream txt(txtName.Data());

	if(txt)
	{
		txt << "# layer wire maximum_before_ns "
			<< "reference_ns offset_ns maximum_after_ns "
			<< "fit_status chi2 ndf entries\n";

		txt << layerName << '\t'
			<< targetWire << '\t'
			<< maximum << '\t'
			<< gReferenceTime << '\t'
			<< offset << '\t'
			<< correctedMaximum << '\t'
			<< fitStatus << '\t';

		if(fitResult.Get())
		{
			txt << fitResult->Chi2() << '\t' << fitResult->Ndf();
		}
		else
		{
			txt << "nan\tnan";
		}

		txt << '\t' << hMin->GetEntries() << '\n';
		txt.close();
	}
	else
	{
		std::cerr << "Warnung: TXT-Ausgabedatei konnte nicht " << "geschrieben werden." << std::endl;
	}

	std::cout
		<< "\nAusgabedateien:\n"
		<< "  " << pdfName << '\n'
		<< "  " << rootName << '\n'
		<< "  " << txtName << '\n';

	delete legend;
	delete referenceLine;
	delete maximumLine;
	delete c;
	delete fit;
	delete hMin;
}
