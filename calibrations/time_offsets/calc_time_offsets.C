#include "TROOT.h"
#include "TSystem.h"

#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TChain.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"

#include <map>
#include <utility>
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


/* ------------------------ Konfiguration ------------------------ */

bool gSingleFile=false;

// eingabefile mit DrifttimeCalib-Tree
//std::string gInputFile = "/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/TQClamDrifttimeCalibStore_Run599.root";
std::string gInputFile = "~/user/oskar/tracking-go4/go4analyseqclam/build/TQClamDrifttimeCalibStore.root";

//std::string gInputBase = "/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/TQClamDrifttimeCalibStore_Run599";
std::string gInputBase = "~/user/oskar/tracking-go4/go4analyseqclam/build/TQClamDrifttimeCalibStore";

// name tree
std::string gTreeName = "DrifttimeCalib";

// referezzeit (in ns), auf die die Flanke geschoben werden soll
double gReferenceTime = 0.;

// vertausche leading <-> trailing
bool swap_leading_trailing = true;

// globaler Offset (--> muss in yaml-config uebernmommen werden !!!)
double gGlobalOffset = -1000.;

// layer-Offsets (--> muss in yaml-config uebernommen werden !!!)
std::map<int,double> gLayerOffset =
{
	{0,0.}, // X1
	{1,0.}, // U
	{2,0.} // X2
};

// Driftzeit invertieren
bool gInvertForFit = true;

// Histogramm-Binning
int gNBins = 250;
double gTmin = -100;
double gTmax = 400;

// nur bestimmte Draehte fitten (falls ein paar fits daneben... nachfitten)
std::set<std::pair<int,int>> gOnlyWires; // = { {layerID,wireNum}, ... }

// Ausgabe-Datei fuer Offsets
std::string gOutputFile = "wire_offsets.txt";

// minimale entry-zahl in driftzeit-histogramm fuer fit
int gMinEntriesForFit = 100;

// min/max tot fuer fit:
double minToT = 30.;
double maxToT = 60.;





/* ------------------------ Hilfsmethoden ------------------------ */
double N_skewed(double x, double A, double mu, double sigma, double s)
{
	double norm = A / (TMath::Sqrt(2*TMath::Pi()) * sigma);
	double exp_part = TMath::Exp(-((x-mu) * (x-mu)) / (2.0 * sigma*sigma));
	double erf_part = TMath::Erf(1.0 + s*(x-mu) / (TMath::Sqrt2() * sigma));
	return norm * exp_part * erf_part;
}

/** fit-funktion: summe zwei skewed peaks */
double drift_model(double* xx, double* par)
{
	double x = xx[0];
	double A0 = par[0];
	double mu0 = par[1];
	double sig0 = par[2];
	double s0 = par[3];
	double A1 = par[4];
	double mu1 = par[5];
	double sig1 = par[6];
	double s1 = par[7];

	return N_skewed(x,A0,mu0,sig0,s0) + N_skewed(x,A1,mu1,sig1,s1);
}






/* ------------------------ Hauptmethode ------------------------ */
//void calc_time_offsets(const char* filename)
void calc_time_offsets()
{
//	gROOT->SetBatch(kFALSE);

	TTree* t = nullptr;
	TChain* chain = nullptr;

	if(gSingleFile)
	{
		TFile* f = TFile::Open(gInputFile.c_str(), "READ");
		if(!f || f->IsZombie())
		{
			std::cerr << "kann datei " << gInputFile << " nicht oeffnen!\nBeende nun!" << std::endl;
			return;
		}

		t = (TTree*)f->Get(gTreeName.c_str());
		if(!t)
		{
			std::cerr << "kann tree " << gTreeName << " nicht oeffnen!\nBeende nun!" << std::endl;
			return;
		}
	}
	else
	{
		chain = new TChain(gTreeName.c_str());

		chain->Add((gInputBase + "*root").c_str());
		t = chain;
	}


	/** histogramme pro (layer,wire) */
	std::map<std::pair<int,int>, TH1D*> hists;

//	Long64_t nEntries = t->GetEntries();
//	std::cout << "lese " << nEntries << " Events..." << std::endl;


	TTreeReader reader(t);

	TTreeReaderArray<double>* v_leading = nullptr;
	if(swap_leading_trailing)
	{
		v_leading = new TTreeReaderArray<double>(reader,"trailing");
	}
	else
	{
		v_leading = new TTreeReaderArray<double>(reader,"leading");
	}
	
	TTreeReaderArray<int> v_layer(reader, "layerID");
	TTreeReaderArray<int>    v_wireNum(reader, "wireNum");
	TTreeReaderArray<double> v_tdcTrigTime(reader, "tdcTrigTime");
	TTreeReaderArray<double> v_tot(reader, "tot");


	/** tree durchlaufen */
	while(reader.Next())
	{

		size_t nhits = v_layer.GetSize();
		for(size_t ih=0; ih<nhits; ++ih)
		{
			int layer = v_layer[ih];
			int wire = v_wireNum[ih];
			double tot = v_tot[ih];
			if(minToT>tot || maxToT < tot) { continue; }

			/** falls nur bestimmt draehte kalibriert werden sollen:
			 * ueberspringe alle anderen */
			if(!gOnlyWires.empty())
			{
				if(gOnlyWires.find({layer,wire}) == gOnlyWires.end()) { continue; }
			}

			double rawT = (*v_leading)[ih] - v_tdcTrigTime[ih]; /** rohe driftzeit */

			/** globaler + layer Offset aus kalibrierung anwenden */
			double t_corr = rawT + gGlobalOffset + gLayerOffset[layer];
			//auto itL = gLayerOffset.find(layer);
			//if(itL != gLayerOffset.end()) { t_corr += itL->second; }

			/** invertieren */
			if(gInvertForFit)
			{
				t_corr = -t_corr;
			}

			auto key = std::make_pair(layer,wire);
			if(!hists[key])
			{
				hists[key] = new TH1D(Form("h_dt_l%d_w%d", layer, wire),
						Form("Driftzeit: Ebene %d, Draht %d;driftzeit/ns ;cts",layer,wire),
						gNBins,gTmin,gTmax);
			}
			hists[key]->Fill(t_corr);
		}
	}

	std::cout << "anzahl gefuellte histogramme: " << hists.size() << std::endl;

	/** fitfunktion definieren */
	TF1* fModel = new TF1("fModel", drift_model, gTmin, gTmax, 8);


	/** fit + offset-berechnung */
	std::ofstream fout(gOutputFile);
	if(!fout.is_open())
	{
		std::cerr << "konnte ausgabedatei " << gOutputFile << " nicht oeffnen!\nBeende nun!" << std::endl;
		return;
	}

	fout << "# layer wire offset_ns\n";

	// evtl entfernen...
	TCanvas* c1 = new TCanvas("c1","Driftzeit Fits",800,600);
	c1->Print("drifttime_offset_fits.pdf["); // pdf oeffnen

	/** ueber alle draehte iterieren */
	for(auto& kv : hists)
	{
		int layer = kv.first.first;
		int wire = kv.first.second;

		TH1D* h = kv.second;

		if(h->GetEntries() < gMinEntriesForFit)
		{
			std::cout << "zu gerinde statistik fuer ebene " << layer << ", draht " << wire << "! --> ueberspringe!" << std::endl;
			continue;
		}

		/** startwerte grob schaetzen */
		int nbins = h->GetNbinsX();
		int mid = nbins/2;

		int maxbin1 = 1;
		double max1 = 0.;
		for(int ib=1; ib<=mid; ++ib)
		{
			double c = h->GetBinContent(ib);
			if(c > max1) { max1 = c; maxbin1 = ib; }
		}

		int maxbin2 = mid+1;
		double max2 = 0.;
		for(int ib = mid+1; ib<=nbins; ++ib)
		{
			double c = h->GetBinContent(ib);
			if(c > max2) { max2 = c; maxbin2 = ib; }
		}

		double mu0 = h->GetBinCenter(maxbin1);
		double mu1 = h->GetBinCenter(maxbin2);
		double A0 = max1 * (gTmax-gTmin)/gNBins;
		double A1 = max2 * (gTmax-gTmin)/gNBins;
		double sigma0 = 10.;
		double sigma1 = 10.;
		double s0 = 0.5;
		double s1 = 0.5;

		fModel->SetParameters(A0,mu0,sigma0,s0, A1,mu1,sigma1,s1);

		/** parameter-grenzen */
		fModel->SetParLimits(0,0,1e9); // A0
		fModel->SetParLimits(4,0,1e9); // A1
		fModel->SetParLimits(2,0.1,100.); // sigma0
		fModel->SetParLimits(6,0.1,100.); // sigma1
		fModel->SetParLimits(3,0.01,10.); // s0
		fModel->SetParLimits(7,0.01,10.); // s1

		h->Fit(fModel,"Q");
	
		c1->cd();
		h->Draw("HIST");
		fModel->Draw("same");

//		c1->Update();
//		gSystem->ProcessEvents();
		c1->Print("drifttime_offset_fits.pdf"); // seite hinzufuegen

		double mu_fit = fModel->GetParameter(1); // erster Peak

		/** draht-offset berechnen:
		 * wireOffset = gReferenceTime - mu_fit
		 */
		double wireOffset = gReferenceTime - mu_fit;

		std::cout << "ebene " << layer << " draht " << wire
			<< " mu_fit=" << mu_fit
			<< " -> wireOffset=" << wireOffset << " ns." << std::endl;

		fout << layer << " " << wire << " " << wireOffset << "\n";
	}


	c1->Print("drifttime_offset_fits.pdf]"); // pdf schliessen

	fout.close();
	std::cout << "offsets berechnet und geschrieben nach " << gOutputFile << "." << std::endl;
}

