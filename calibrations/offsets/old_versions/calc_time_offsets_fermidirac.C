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

bool gSingleFile=true;

// eingabefile mit DrifttimeCalib-Tree


/*
 * Case: 1
 * */
/// falls gSingleFile = true
std::string gInputFile = "/d/d04-1/ag_pi/QCLAM_EEPG_CE140_2026/ana/unpacked_data/calibrations_go4/calibrations/timewalk/timewalk_corrected_run_34_40.root";

/*
 * Case: 2
 * */

// falls gSingleFile = false
std::string gInputBase = "/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run599/TQClamDrifttimeCalibStore_599";
//std::string gInputBase = "~/user/oskar/tracking-go4/go4analyseqclam/build/TQClamDrifttimeCalibStore";



/*
 * Case: 3
 * */

// hierfuer:
// gSingleFile = false
// gMultipleFiles = true
bool gMultipleFiles = false;
std::string gInputDirectory = "/d/d04-1/ag_pi/QCLAM_EEPG_CE140_2026/ana/unpacked_data/calibrations_go4/data/offset";

// inklusive
int gRunFirst = 34;
int gRunLast  = 40;


std::string gFilePrefix = "DrifttimeStore_qclam_dago_";

// name tree
std::string gTreeName = "DrifttimeCalib_TWcorr";














// referezzeit (in ns), auf die die Flanke geschoben werden soll
double gReferenceTime = 0.;

// vertausche leading <-> trailing
bool swap_leading_trailing = false;

// globaler Offset (--> muss in yaml-config uebernmommen werden !!!)
double gGlobalOffset = -0.;

// layer-Offsets (--> muss in yaml-config uebernommen werden !!!)
std::map<int,double> gLayerOffset =
{
	{0,0.0}, // X1
	{1,0.0}, // U
	{2,0.0} // X2
};

// Driftzeit invertieren
bool gInvertForFit = false;

// Histogramm-Binning
int gNBins = 300;
double gTmin = 0.;
double gTmax = 300.;

// nur bestimmte Draehte fitten (falls ein paar fits daneben... nachfitten)
std::set<std::pair<int,int>> gOnlyWires; // = { {layerID,wireNum}, ... }

// Ausgabe-Datei fuer Offsets
/*
std::string gOutputFileX1 = "time_offsets_X1_run_80_91.txt";
std::string gOutputFileU = "time_offsets_U_run_80_91.txt";
std::string gOutputFileX2 = "time_offsets_X2_run_80_91.txt";
*/

/*
std::string gOutputFileX1 = "time_offsets_X1_run_113_120.txt";
std::string gOutputFileU = "time_offsets_U_run_113_120.txt";
std::string gOutputFileX2 = "time_offsets_X2_run_113_120.txt";
*/


std::string gOutputFileX1 = "time_offsets_X1_run_34_40.txt";
std::string gOutputFileU = "time_offsets_U_run_34_40.txt";
std::string gOutputFileX2 = "time_offsets_X2_run_34_40.txt";


// minimale entry-zahl in driftzeit-histogramm fuer fit
int gMinEntriesForFit = 100;

// min/max tot fuer fit:
double minToT = 10.;
double maxToT = 300.;

// parametereinstellung:
// - fitbereich: suche maximum zwischen:
double gSearchMin = 0.;
double gSearchMax = 100.; 

// zwischen searchMin und searchMax: suche maximum
// --> grenze fuer fit:
// [max-50, max+50]
double gFitRegionBelowMax = -50.;
double gFitRegionOverMax = 5.;


// fit: parameter grenzen:
double gA0Min = 0;
double gA0Max = 1e9;
double gT0Min = 0.;
double gT0Max = 100.;
double gTauMin = 0.1;
double gTauMax = 50.;
double gTMin = 0.1;
double gTMax = 20.;






// BuildChain for input

TChain* BuildInputChain()
{
        TChain* chain = new TChain(gTreeName.c_str());

        int nFiles = 0;

        std::cout
                << std::endl
                << "=================================================="
                << std::endl
                << "Input-Dateien"
                << std::endl
                << "=================================================="
                << std::endl;

        for(int run = gRunFirst; run <= gRunLast; run++)
        {
                TString pattern =
                        Form(
                                        "%s/%s%03d_*.root",
                                        gInputDirectory.c_str(),
                                        gFilePrefix.c_str(),
                                        run
                                        );

                std::cout
                        << "Run "
                        << run
                        << ": "
                        << pattern
                        << std::endl;

                int nAdded =
                        chain->Add(pattern);

                std::cout
                        << "  -> "
                        << nAdded
                        << " Dateien"
                        << std::endl;

                nFiles += nAdded;
        }


        std::cout
                << std::endl
                << "Insgesamt "
                << nFiles
                << " ROOT-Dateien hinzugefuegt."
                << std::endl;

        if(nFiles == 0)
        {
                std::cout
                        << "FEHLER: Keine Input-Dateien gefunden!"
                        << std::endl;

                delete chain;
                return nullptr;
        }


        std::cout
                << "Tree: "
                << gTreeName
                << std::endl;

        std::cout
                << "Entries: "
                << chain->GetEntries()
                << std::endl;

        std::cout
                << "=================================================="
                << std::endl
                << std::endl;

        return chain;
}












/* ------------------------ Fermi-Dirac Funktion ------------------------ */
/*
   f(t) = A / (1 + exp(-(t - (t0 + tau)) / T))
   Parameter:
   par[0] = A
   par[1] = t0
   par[2] = tau
   par[3] = T
*/
double fermi_dirac(double* xx, double* par)
{
	double t = xx[0];
	double A   = par[0];
	double t0  = par[1];
	double tau = par[2];
	double T   = par[3];

	return A / (1.0 + TMath::Exp(-(t - (t0 + tau)) / T));
}














/* ------------------------ Hauptmethode ------------------------ */
void calc_time_offsets_fermidirac()
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
	else if(!gSingleFile && gMultipleFiles)
	{
		// ========================================================
		// Input Chain aufbauen
		// ========================================================

		chain = BuildInputChain();

		if(!chain) { return; }

		t = chain;


	}
	else // old forgotten
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

	/*
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
	*/

	TTreeReaderArray<int> v_layer(reader, "layerID");
	TTreeReaderArray<int> v_wireNum(reader, "wireNum");
	TTreeReaderArray<double> v_tot(reader, "tot");
	TTreeReaderArray<double> v_tcorr(reader, "t_tw_corr");


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

			/*
			double rawT = (*v_leading)[ih] - v_tdcTrigTime[ih];

			// globaler + layer Offset aus kalibrierung anwenden
			double t_corr = rawT + gGlobalOffset + gLayerOffset[layer];
			//auto itL = gLayerOffset.find(layer);
			//if(itL != gLayerOffset.end()) { t_corr += itL->second; }
			*/

			double t_corr = v_tcorr[ih];

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
	//TF1* fModel = new TF1("fModel", drift_model, gTmin, gTmax, 8);
	TF1* fFD = new TF1("fFD", fermi_dirac, gTmin, gTmax, 4);


	/** fit + offset-berechnung */
	std::ofstream foutX1(gOutputFileX1);
	std::ofstream foutU(gOutputFileU);
	std::ofstream foutX2(gOutputFileX2);
	if(!foutX1.is_open())
	{
		std::cerr << "konnte ausgabedatei " << gOutputFileX1 << " nicht oeffnen!\nBeende nun!" << std::endl;
		return;
	}
	if(!foutU.is_open())
	{
		std::cerr << "konnte ausgabedatei " << gOutputFileU << " nicht oeffnen!\nBeende nun!" << std::endl;
		return;
	}
	if(!foutX2.is_open())
	{
		std::cerr << "konnte ausgabedatei " << gOutputFileX2 << " nicht oeffnen!\nBeende nun!" << std::endl;
		return;
	}


	//fout << "# layer wire offset_ns\n";

	// evtl entfernen...
	TCanvas* c1 = new TCanvas("c1","Driftzeit Fits",800,600);
	//c1->Print("drifttime_offset_fermidirac_fits_run_80_91.pdf["); // pdf oeffnen
	//c1->Print("drifttime_offset_fermidirac_fits_run_113_120.pdf["); // pdf oeffnen
	c1->Print("drifttime_offset_fermidirac_fits_run_34_40.pdf["); // pdf oeffnen

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
		int binMin = h->FindBin(gSearchMin);
		int binMax = h->FindBin(gSearchMax);

		int maxbin = binMin;
		double maxval = 0;

		for (int b = binMin; b <= binMax; ++b)
		{
			double c = h->GetBinContent(b);
			if (c > maxval)
			{
				maxval = c;
				maxbin = b;
			}
		}

		double maxpos = h->GetBinCenter(maxbin);


		double fitMin = maxpos + gFitRegionBelowMax;
		double fitMax = maxpos + gFitRegionOverMax;

		fitMin = std::max(fitMin,gTmin);
		fitMax = std::min(fitMax,gTmax);

		double A0 = h->GetBinContent(maxbin);
		double t0_start = maxpos + 0.5 * gFitRegionBelowMax;
		double tau_start = 5.;
		double T_start = 2.;

		fFD->SetParameters(A0,t0_start,tau_start,T_start);

		/** parameter-grenzen */
		fFD->SetParLimits(0,gA0Min,gA0Max); // A0
		fFD->SetParLimits(1,fitMin,fitMax); // t0
		fFD->SetParLimits(2,gTauMin,gTauMax); // tau
		fFD->SetParLimits(3,gTMin,gTMax); // T

		h->Fit(fFD,"Q","",fitMin,fitMax);
		fFD->SetRange(fitMin,fitMax);
	
		c1->cd();
		h->Draw("HIST");
		fFD->Draw("same");

//		c1->Update();
//		gSystem->ProcessEvents();
		//c1->Print("drifttime_offset_fermidirac_fits_run_80_91.pdf"); // seite hinzufuegen
		//c1->Print("drifttime_offset_fermidirac_fits_run_113_120.pdf"); // seite hinzufuegen
		c1->Print("drifttime_offset_fermidirac_fits_run_34_40.pdf"); // seite hinzufuegen

		double t0_fit = fFD->GetParameter(1); // erster Peak

		/** draht-offset berechnen:
		 * wireOffset = gReferenceTime - t0_fit
		 */
		double wireOffset = gReferenceTime - t0_fit;

		std::cout << "ebene " << layer << " draht " << wire
			<< " t0_fit=" << t0_fit
			<< " -> wireOffset=" << wireOffset << " ns." << std::endl;

		if(layer == 0)
		{
			foutX1 << Form("%03d\t%.4f\n", wire, wireOffset);
		}
		else if(layer == 1)
		{
			foutU << Form("%03d\t%.4f\n", wire, wireOffset);
		}
		else if(layer == 2)
		{
			foutX2 << Form("%03d\t%.4f\n", wire, wireOffset);
		}
	}


	//c1->Print("drifttime_offset_fermidirac_fits_run_80_91.pdf]"); // pdf schliessen
	//c1->Print("drifttime_offset_fermidirac_fits_run_113_120.pdf]"); // pdf schliessen
	c1->Print("drifttime_offset_fermidirac_fits_run_34_40.pdf]"); // pdf schliessen

	foutX1.close();
	foutU.close();
	foutX2.close();
	std::cout << "offsets berechnet und geschrieben nach " << gOutputFileX1 << "." << std::endl;
	std::cout << "offsets berechnet und geschrieben nach " << gOutputFileU << "." << std::endl;
	std::cout << "offsets berechnet und geschrieben nach " << gOutputFileX2 << "." << std::endl;
}

