// Aufruf:
// root -q -l 'timewalkcorr_all.C()'
//


#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TAxis.h"

#include <map>
#include <vector>
#include <utility>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>


/* ============================================================
   Konfiguration
   ============================================================ */

// ------------------------------------------------------------
// Input / Output
// ------------------------------------------------------------

//std::string gInputFile = "/d/d01-3/ag_pi/QCLAM_COMM_2026/go4_data/new_calib_tables/drifttime_store_new_calib_tables_run_121_130.root";
std::string gInputFile = "/d/d04-1/ag_pi/spektrometer/QCLAM/master_proposal_omoeller/test_thresholds/drifttime_store_threshold_test_001.root";

std::string gTreeName = "DrifttimeCalib";

//std::string gOutputRootFile = "timewalk_corrected_run_80_91.root";
//std::string gOutputRootFile = "timewalk_corrected_run_113_120.root";
//std::string gOutputRootFile = "timewalk_corrected_run_121_130.root";

std::string gOutputRootFile = "timewalk_corrected_run_1.root";

std::string gOutputTreeName = "DrifttimeCalib_TWcorr";


/*
std::string gOutputTWFileX1 = "time_walk_corrections_X1_run_80_91.txt";
std::string gOutputTWFileU = "time_walk_corrections_U_run_80_91.txt";
std::string gOutputTWFileX2 = "time_walk_corrections_X2_run_80_91.txt";
*/

/*
std::string gOutputTWFileX1 = "time_walk_corrections_X1_run_113_120.txt";
std::string gOutputTWFileU = "time_walk_corrections_U_run_113_120.txt";
std::string gOutputTWFileX2 = "time_walk_corrections_X2_run_113_120.txt";
*/

/*
std::string gOutputTWFileX1 = "time_walk_corrections_X1_run_121_130.txt";
std::string gOutputTWFileU = "time_walk_corrections_U_run_121_130.txt";
std::string gOutputTWFileX2 = "time_walk_corrections_X2_run_121_130.txt";
*/


std::string gOutputTWFileX1 = "time_walk_corrections_X1_run_1.txt";
std::string gOutputTWFileU = "time_walk_corrections_U_run_1.txt";
std::string gOutputTWFileX2 = "time_walk_corrections_X2_run_1.txt";




// ------------------------------------------------------------
// leading/trailing tauschen?
// ------------------------------------------------------------

bool gSwapLeadingTrailing = false;


// ------------------------------------------------------------
// ToT Gates
// ------------------------------------------------------------

double gToTmin = 10.0;
double gToTmax = 100.0;


// ------------------------------------------------------------
// Driftzeitbereich
// ------------------------------------------------------------

double gDriftMin = 0.0;
double gDriftMax = 1000.0;

double gOffsetTime = 1000; // rawT = -(lead-trig) + gOffsetTime

// ------------------------------------------------------------
// Histogramm-Binning
// ------------------------------------------------------------

int    gNBinsToT   = 100;
double gToTaxisMin = 10.0;
double gToTaxisMax = 100.0;

int    gNBinsT     = 500;
double gTaxisMin   = 0.0;
double gTaxisMax   = 400.0;


// ------------------------------------------------------------
// Mindeststatistik pro Slice
// ------------------------------------------------------------

int gMinEntriesPerSlice = 0;


// ------------------------------------------------------------
// ToT-Fitbereich
// ------------------------------------------------------------

double gFitToTmin = 10.0;
double gFitToTmax = 100.0;


// ------------------------------------------------------------
// Peak-Suchfenster
// ------------------------------------------------------------

// globales Fenster fürs erste Slice
double gPeakSearchMin = 10.0;
double gPeakSearchMax = 100.0;

// Peak-Following Fenster
double gPeakFollowWindow = 10.0;


// ------------------------------------------------------------
// Gaußfit-Bereich
// ------------------------------------------------------------

double gLocalFitWindow = 7.0;

bool gSaveSliceFits = false;


// ------------------------------------------------------------
// Outlier-Rejection
// ------------------------------------------------------------

double gOutlierCut = 25.0;


// ------------------------------------------------------------
// Referenzzeit
// ------------------------------------------------------------

double gTref = 50.0;


/* ============================================================
   Verbesserter Time-Walk-Fit
   ============================================================ */

TF1* fittimewalk(TH2D* h2, std::vector<TH1D*>* slices = nullptr)
{
    
	if(!h2) { return nullptr; }

	TAxis* xax = h2->GetXaxis();


	TCanvas* cSlices = nullptr;

	/*
	if(gSaveSliceFits)
	{
		cSlices = new TCanvas(Form("cSlices_%s", h2->GetName()), "", 800,600);

		cSlices->Print(Form("%s_slices.pdf[",h2->GetName()));
	}
	*/

	std::vector<double> xvals;
	std::vector<double> yvals;
	std::vector<double> yerrs;

	double lastPeak = -9999;

	for(int ix=1; ix<=xax->GetNbins(); ix++)
	{
		double tot = xax->GetBinCenter(ix);

		if(tot < gFitToTmin || tot > gFitToTmax) { continue; }

		TH1D* py = h2->ProjectionY(Form("py_%s_%d",h2->GetName(),ix),ix,ix	);

		if(py->GetEntries() < gMinEntriesPerSlice)
		{
			delete py;
			continue;
		}

		// ----------------------------------------------------
		// Suchfenster
		// ----------------------------------------------------

		double searchMin;
		double searchMax;

		if(lastPeak < -9000)
		{
			searchMin = gPeakSearchMin;
			searchMax = gPeakSearchMax;
		}
		else
		{
			searchMin = lastPeak - gPeakFollowWindow;
			searchMax = lastPeak + gPeakFollowWindow;
		}

		int binMin = py->FindBin(searchMin);
		int binMax = py->FindBin(searchMax);

		if(binMin >= binMax)
		{
			delete py;
			continue;
		}

		// ----------------------------------------------------
		// lokales Maximum (in ToT-Bin)
		// ----------------------------------------------------

		int maxbin = -1;
		double maxcontent = -1;

		for(int b=binMin; b<=binMax; b++)
		{
			double c = py->GetBinContent(b);

			if(c > maxcontent)
			{
				maxcontent = c;
				maxbin = b;
			}
		}

		if(maxbin < 0)
		{
			delete py;
			continue;
		}

		double peak0 = py->GetBinCenter(maxbin);

		// ----------------------------------------------------
		// lokaler Gaußfit
		// ----------------------------------------------------

		double fitMin = peak0 - gLocalFitWindow;
		double fitMax = peak0 + gLocalFitWindow;

		if(py->Integral(py->FindBin(fitMin),py->FindBin(fitMax)) < 5)
		{
			delete py;
			continue;
		}

		TF1 gaus("g","gaus",fitMin,fitMax);
		gaus.SetParameters(py->GetMaximum(),peak0,10);
		gaus.SetParLimits(1,peak0-gLocalFitWindow,peak0+gLocalFitWindow);

		gaus.SetParLimits(2,2,40);

		int status = py->Fit(&gaus,"QNR");

		if(status != 0)
		{
			delete py;
			continue;
		}

		double peak = gaus.GetParameter(1);
		double peakErr = gaus.GetParError(1);

		if(slices)
		{
			TH1D* copy = (TH1D*)py->Clone(Form("%s_slice_%d", h2->GetName(), ix));

			TF1* gcopy = (TF1*)gaus.Clone(Form("gaus_%s_%d",h2->GetName(), ix));

			gcopy->SetLineColor(kRed);
			gcopy->SetLineWidth(2);

			copy->GetListOfFunctions()->Add(gcopy);

			slices->push_back(copy);
		}

		/*
		if(gSaveSliceFits)
		{
			cSlices->cd();
			py->Draw();

			gaus.SetLineColor(kRed);
			gaus.SetLineWidth(2);
			gaus.Draw("same");
			
			cSlices->Print(Form("%s_slices.pdf", h2->GetName()));
		}
		*/

		if(peakErr > 8)
		{
			delete py;
			continue;
		}

		xvals.push_back(tot);
		yvals.push_back(peak);
		yerrs.push_back(peakErr);

		lastPeak = peak;

		delete py;
	}

	/*
	if(xvals.size() < 6)
	{
		std::cout << "Zu wenig Punkte fuer "
			<< h2->GetName()
			<< std::endl;

		return nullptr;
	}
	*/

	// ========================================================
	// Vorfit
	// ========================================================

	TGraphErrors gr0(
			xvals.size(),
			&xvals[0],
			&yvals[0],
			nullptr,
			&yerrs[0]
			);
	TF1 prefit(
			"prefit",
			"[0] * exp(-[1]*x) + [2]",
			gFitToTmin,
			gFitToTmax);

	/*
	TF1 prefit(
			"prefit",
			"[0]+[1]*x+[2]*x*x",
			gFitToTmin,
			gFitToTmax
			);
	*/

	prefit.SetParameters(10.,0.05,40);
	prefit.SetParLimits(0,0,100);
	prefit.SetParLimits(1,0.0001,1.0);
	prefit.SetParLimits(2,0,100);

	gr0.Fit(&prefit,"Q");

	// ========================================================
	// Outlier entfernen
	// ========================================================

	std::vector<double> x2;
	std::vector<double> y2;
	std::vector<double> e2;

	for(size_t i=0; i<xvals.size(); i++)
	{
		double yfit = prefit.Eval(xvals[i]);
		double resid = std::fabs(yvals[i]-yfit);

		if(resid < gOutlierCut)
		{
			x2.push_back(xvals[i]);
			y2.push_back(yvals[i]);
			e2.push_back(yerrs[i]);
		}
	}

	if(x2.size() < 5)
	{
		std::cout << "Zu viele Outlier entfernt fuer "
			<< h2->GetName()
			<< std::endl;

		return nullptr;
	}

	// ========================================================
	// Finaler Fit
	// ========================================================

	TGraphErrors* gr = new TGraphErrors(
			x2.size(),
			&x2[0],
			&y2[0],
			nullptr,
			&e2[0]
			);

	/*
	TF1* f = new TF1(
			Form("f_tw_%s",h2->GetName()),
			"[0]+[1]*x+[2]*x*x",
			gFitToTmin,
			gFitToTmax
			);

	f->SetParameters(
			prefit.GetParameter(0),
			prefit.GetParameter(1),
			prefit.GetParameter(2)
			);
	*/
	TF1* f = new TF1(
			Form("f_tw_%s",h2->GetName()),
			"[0]*exp(-[1]*x) + [2]",
			gFitToTmin,
			gFitToTmax);
	f->SetParameters(
			prefit.GetParameter(0),
			prefit.GetParameter(1),
			prefit.GetParameter(2));

	gr->Fit(f,"QR");

	std::cout
		<< h2->GetName()
		<< "  Punkte=" << x2.size()
		<< "  chi2/ndf="
		<< f->GetChisquare()/f->GetNDF()
		<< std::endl;

	delete gr;

	/*
	if(gSaveSliceFits)
	{
		cSlices->Print(Form("%s_slices.pdf]", h2->GetName()));

		delete cSlices;
	}
	*/

	return f;
}


/* ============================================================
   Hauptfunktion
   ============================================================ */

void timewalkcorr_all()
{
    
	// --------------------------------------------------------
	// Input
	// --------------------------------------------------------

	TFile* fin = TFile::Open(gInputFile.c_str(),"READ");

	if(!fin || fin->IsZombie())
	{
		std::cout << "Konnte Inputfile nicht öffnen!" << std::endl;
        
		return;
	}

	TTree* t = (TTree*)fin->Get(gTreeName.c_str());

	if(!t)
	{
		std::cout << "Tree nicht gefunden!" << std::endl;
		return;
	}

	// --------------------------------------------------------
	// Reader
	// --------------------------------------------------------

	TTreeReader reader(t);

	TTreeReaderArray<double>* leading = nullptr;
	TTreeReaderArray<double>* trailing = nullptr;

	if(gSwapLeadingTrailing)
	{
		leading = new TTreeReaderArray<double>(reader,"trailing");

		trailing = new TTreeReaderArray<double>(reader,"leading");
	}
	else
	{
		leading = new TTreeReaderArray<double>(reader,"leading");

		trailing = new TTreeReaderArray<double>(reader,"trailing");
	}

	TTreeReaderArray<double> tTrig(	reader,"tdcTrigTime");

	TTreeReaderArray<int> layer(reader,"layerID");

	TTreeReaderArray<int> wire(reader,"wireNum");

	// --------------------------------------------------------
	// Histogramme
	// --------------------------------------------------------

	std::map<std::pair<int,int>,TH2D*> h2_map;
	std::map<std::pair<int,int>,TH2D*> h2_corr;
	std::map<std::pair<int,int>,std::vector<TH1D*>> slicePlots;

	std::cout << "Erste Schleife..." << std::endl;

	while(reader.Next())
	{
		size_t nhits = layer.GetSize();

		for(size_t ih=0; ih<nhits; ih++)
		{
			int L = layer[ih];
			int W = wire[ih];

			auto key = std::make_pair(L,W);

			double lead = (*leading)[ih];

			double trail = (*trailing)[ih];

			double trig = tTrig[ih];

			double tot = std::fabs(lead-trail);

			if(tot < gToTmin || tot > gToTmax) { continue; }

			// invertierte Driftzeit
			double rawT = -(lead-trig) + gOffsetTime;

			if(rawT < gDriftMin || rawT > gDriftMax) { continue; }

			if(!h2_map[key])
			{
				h2_map[key] = new TH2D(
						Form("h2_L%d_W%d",L,W),
						Form("ToT vs Driftzeit L%d W%d;ToT/ns;Driftzeit/ns",L,W),
						gNBinsToT,
						gToTaxisMin,
						gToTaxisMax,
						gNBinsT,
						gTaxisMin,
						gTaxisMax
						);
			}

			h2_map[key]->Fill(tot,rawT);
		}
	}

	std::cout
		<< "Histogramme: "
		<< h2_map.size()
		<< std::endl;

	// --------------------------------------------------------
	// Fits
	// --------------------------------------------------------

	std::map<std::pair<int,int>,TF1*> twFit;

	for(auto& kv : h2_map)
	{
		TH2D* h2 = kv.second;

		if(h2->GetEntries() < 100)
		{
			twFit[kv.first] = nullptr;
			continue;
		}

		twFit[kv.first] = fittimewalk(h2, &slicePlots[kv.first]);
	}

	// --------------------------------------------------------
	// Output
	// --------------------------------------------------------

	TFile* fout = new TFile(gOutputRootFile.c_str(),"RECREATE");

	fout->mkdir("timewalk_hists");
	fout->cd("timewalk_hists");

	std::ofstream foutTWX1(gOutputTWFileX1);
	std::ofstream foutTWU(gOutputTWFileU);
	std::ofstream foutTWX2(gOutputTWFileX2);

	if(!foutTWX1 || !foutTWU || !foutTWX2)
	{
		std::cout << "Fehler beim Oeffnen der Time-Walk-Textdateien!" << std::endl;
		return;
	}

	for(auto& kv : h2_map)
	{
		kv.second->Write();

		TF1* f = twFit[kv.first];

		if(f)
		{
			f->Write();

			int layer = kv.first.first;
			int wire = kv.first.second;

			std::ofstream* out = nullptr;
			if(layer == 0) { out = &foutTWX1; }
			if(layer == 1) { out = &foutTWU; }
			if(layer == 2) { out = &foutTWX2; }

			if(out)
			{
				(*out)
					<< Form("%03d",wire)
					<< "\t"
					<< f->GetParameter(0)
					<< "\t"
					<< f->GetParameter(1)
					<< "\t"
					<< f->GetParameter(2)
					<< "\n";
			}
		}
		//if(twFit[kv.first]) { twFit[kv.first]->Write(); }

	}

	fout->cd();

	// --------------------------------------------------------
	// neuer Tree
	// --------------------------------------------------------

	TTree* tout = new TTree(gOutputTreeName.c_str(),"Timewalk corrected");

	std::vector<int> out_layer;
	std::vector<int> out_wire;

	std::vector<double> out_tot;
	std::vector<double> out_rawT;
	std::vector<double> out_corrT;

	tout->Branch("layerID",&out_layer);
	tout->Branch("wireNum",&out_wire);

	tout->Branch("tot",&out_tot);
	tout->Branch("t_raw",&out_rawT);
	tout->Branch("t_tw_corr",&out_corrT);

	// --------------------------------------------------------
	// zweite Schleife
	// --------------------------------------------------------

	reader.Restart();

	std::cout << "Zweite Schleife..." << std::endl;

	while(reader.Next())
	{
		out_layer.clear();
		out_wire.clear();

		out_tot.clear();
		out_rawT.clear();
		out_corrT.clear();

		size_t nhits = layer.GetSize();

		for(size_t ih=0; ih<nhits; ih++)
		{
			int L = layer[ih];
			int W = wire[ih];

			auto key = std::make_pair(L,W);

			double lead = (*leading)[ih];
			double trail = (*trailing)[ih];
			double trig = tTrig[ih];
			double tot = std::fabs(lead-trail);
			double rawT = -(lead-trig) + gOffsetTime;
			double corrT = rawT;

			TF1* f = twFit[key];

			if(f)
			{
				double tw = f->Eval(tot);

				corrT = rawT - (tw-gTref);

				if(!h2_corr[key])
				{
					h2_corr[key] = new TH2D(
							Form("h2corr_L%d_W%d",L,W),
							Form("TW corrected L%d W%d;ToT/ns;Driftzeit/ns",L,W),
							gNBinsToT,
							gToTaxisMin,
							gToTaxisMax,
							gNBinsT,
							gTaxisMin,
							gTaxisMax
							);
				}

				h2_corr[key]->Fill(tot,corrT);
			}

			out_layer.push_back(L);
			out_wire.push_back(W);

			out_tot.push_back(tot);
			out_rawT.push_back(rawT);
			out_corrT.push_back(corrT);
		}

		tout->Fill();
	}

	// --------------------------------------------------------
	// PDF
	// --------------------------------------------------------

	TCanvas* c = new TCanvas("c","Timewalk",1000,800);

	//c->Print("timewalk_plots_run_80_91.pdf[");
	//c->Print("timewalk_plots_run_113_120.pdf[");
	//c->Print("timewalk_plots_run_121_130.pdf[");
	c->Print("timewalk_plots_run_1.pdf[");

	for(auto& kv : h2_map)
	{
		TH2D* h2 = kv.second;

		TF1* f = twFit[kv.first];

		c->cd();

		/** unkorrigiert */
		h2->Draw("COLZ");
		//c->Print("timewalk_plots_run_80_91.pdf");
		//c->Print("timewalk_plots_run_113_120.pdf");
		//c->Print("timewalk_plots_run_121_130.pdf");
		c->Print("timewalk_plots_run_1.pdf");

		/** alle slices */
		if(gSaveSliceFits)
		{
			for(auto* s : slicePlots[kv.first])
			{
				s->Draw();
				//c->Print("timewalk_plots_run_80_91.pdf");
				//c->Print("timewalk_plots_run_113_120.pdf");
				//c->Print("timewalk_plots_run_121_130.pdf");
				c->Print("timewalk_plots_run_1.pdf");
			}
		}

		/** unkorrigierti + Fit */
		h2->Draw("COLZ");
		if(f) { f->Draw("same"); }
		//c->Print("timewalk_plots_run_80_91.pdf");
		//c->Print("timewalk_plots_run_113_120.pdf");
		//c->Print("timewalk_plots_run_121_130.pdf");
		c->Print("timewalk_plots_run_1.pdf");


		/** korrigiert */
		if(h2_corr[kv.first])
		{
			h2_corr[kv.first]->Draw("COLZ");

			//c->Print("timewalk_plots_run_80_91.pdf");
			//c->Print("timewalk_plots_run_113_120.pdf");
			//c->Print("timewalk_plots_run_121_130.pdf");
			c->Print("timewalk_plots_run_1.pdf");
		}
	}

	//c->Print("timewalk_plots_run_80_91.pdf]");
	//c->Print("timewalk_plots_run_113_120.pdf]");
	c->Print("timewalk_plots_run_121_130.pdf]");

	// --------------------------------------------------------
	// speichern
	// --------------------------------------------------------

	fout->cd();

	tout->Write();

	for(auto& kv : h2_corr)
	{
		kv.second->Write();
	}

	foutTWX1.close();
	foutTWU.close();
	foutTWX2.close();

	fout->Close();

	std::cout << "Fertig." << std::endl;
}
