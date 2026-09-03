// timewalkcorr_all.C
//
// - liest einen (bereits offset-korrigierbaren) Drifttime-Tree ein
// - liest Draht-Offsets aus Textfile (layer wire offset_ns)
// - baut pro (layer,wire) ein 2D-Histogramm ToT vs. Driftzeit (mit Offset)
// - fitttet pro (layer,wire) eine Time-Walk-Funktion
// - berechnet pro Hit eine time-walk-korrigierte Driftzeit
// - schreibt alles in ein neues ROOT-File + neuen TTree
//
// Aufruf in ROOT z.B.:
// root -l 'timewalkcorr_all.C("input.root","DrifttimeCalib","wire_offsets_fermidirac.txt")'
//

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
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

/* ------------------------ Konfiguration ------------------------ */

// Input File
//std::string gInputFile = "/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run599/TQClamDrifttimeCalibStore_599.root";
std::string gInputFile = "/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run591_to_601/TQClamDrifttimeCalibStore.root";

// Name des Eingabe-Trees
std::string gTreeName = "DrifttimeCalib";

// Name des Ausgabefiles
std::string gOutputRootFile = "/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run591_to_601/timewalk_corrected.root";

// Name des Ausgabebaums
std::string gOutputTreeName = "DrifttimeCalib_TWcorr";

// leading <-> trailing vertauschen (wie in deinem Offset-Skript)
bool gSwapLeadingTrailing = false;

// ToT-Gate
double gToTmin = 25.0;
double gToTmax = 70.0;

// Driftzeit-Gate (nach Offset, vor Time-Walk-Korrektur)
double gDriftMin = -100.0;
double gDriftMax = 1000.0;

// Binning für 2D-Histogramme (ToT vs Driftzeit)
int    gNBinsToT   = 80;
double gToTaxisMin = 0.0;
double gToTaxisMax = 80.0;

int    gNBinsT     = 200;
double gTaxisMin   = 700.0;
double gTaxisMax   = 1200.0;

// Minimalstatistik pro ToT-Bin für Peakbestimmung
int gMinEntriesPerSlice = 10;

// Fitbereich für Time-Walk-Funktion (in ToT)
// fuer lin-fit --> 20 .. 60 gut
double gFitToTmin = 25.0;
double gFitToTmax = 60.0;

// schiebe korrigierten Zeiten nach tref:
// t_corr = t_raw - (t_fit - t_ref)
double gTref = 1000.;


/* ------------------------ Offsets einlesen ------------------------ */

std::map<std::pair<int,int>, double> gWireOffset;

void loadOffsets(const std::string& fname)
{
    gWireOffset.clear();

    std::ifstream fin(fname);
    if(!fin.is_open())
    {
        std::cerr << "Konnte Offset-Datei " << fname << " nicht oeffnen!" << std::endl;
        return;
    }

    std::string line;
    while(true)
    {
        int layer, wire;
        double off;
        if(!(fin >> layer >> wire >> off)) break;
        gWireOffset[{layer,wire}] = off;
    }

    std::cout << "Geladene Offsets: " << gWireOffset.size() << std::endl;
}


/* ------------------------ Time-Walk-Fit für EIN Histogramm ------------------------ */

TF1* fittimewalk(TH2D* h2)
{
    if(!h2)
    {
        std::cout << "fittimewalk: TH2D als Argument uebergeben!" << std::endl;
        return nullptr;
    }

    TAxis* xax = h2->GetXaxis();
    int nx = xax->GetNbins();

    std::vector<double> tot_vals;
    std::vector<double> peak_vals;

    for(int ix=1; ix<=nx; ix++)
    {
        TH1D* py = h2->ProjectionY(Form("py_%s_%d", h2->GetName(), ix), ix, ix);

        if(py->GetEntries() < gMinEntriesPerSlice)
        {
            delete py;
            continue; // zu wenig Statistik
        }

        int maxbin = py->GetMaximumBin();
        double peak = py->GetXaxis()->GetBinCenter(maxbin);
        double tot  = xax->GetBinCenter(ix);

        tot_vals.push_back(tot);
        peak_vals.push_back(peak);

        delete py;
    }

    if(tot_vals.size() < 4)
    {
        std::cout << "fittimewalk: zu wenig Punkte fuer Fit (" << tot_vals.size() << ")" << std::endl;
        return nullptr;
    }

    TGraph* gr = new TGraph((int)tot_vals.size(), &tot_vals[0], &peak_vals[0]);
    gr->SetTitle("peak drifttime vs ToT;ToT/ns;Peak drifttime/ns");
    gr->SetMarkerStyle(20);

    TF1* f = new TF1(Form("f_tw_%s", h2->GetName()),
                     "[0] + [1]*x", //+ [2]*x*x + [3]*x*x*x",
                     gFitToTmin, gFitToTmax);

    gr->Fit(f, "QR"); // ruhig, mit Range

    std::cout << "Time-Walk Fit fuer " << h2->GetName() << " mit "
              << tot_vals.size() << " Punkten." << std::endl;

    delete gr;
    return f;
}


/* ------------------------ Hauptfunktion: alles zusammen ------------------------ */

void timewalkcorr_all()//const char* inputFile = "TQClamDrifttimeCalibStore.root",
{
//	gROOT->SetBatch(kFALSE);

	// Offsets laden
	//loadOffsets(offsetFile);

	// Eingabefile öffnen
	TFile* fin = TFile::Open(gInputFile.c_str(), "READ");
	if(!fin || fin->IsZombie())
	{
		std::cerr << "Konnte Eingabedatei " << gInputFile << " nicht oeffnen!" << std::endl;
		return;
	}

	TTree* t = (TTree*)fin->Get(gTreeName.c_str());
	if(!t)
	{
		std::cerr << "Konnte Tree " << gTreeName << " nicht finden!" << std::endl;
		return;
	}

	// Reader
	TTreeReader reader(t);

	TTreeReaderArray<double>* leading = nullptr;
	TTreeReaderArray<double>* trailing = nullptr;
	if(gSwapLeadingTrailing)
	{
		leading = new TTreeReaderArray<double>(reader, "trailing");
		trailing = new TTreeReaderArray<double>(reader, "leading");
	}
	else
	{
		leading = new TTreeReaderArray<double>(reader, "leading");
		trailing = new TTreeReaderArray<double>(reader, "trailing");
	}

	TTreeReaderArray<double> tTrig(reader, "tdcTrigTime");
	TTreeReaderArray<int>    layer(reader, "layerID");
	TTreeReaderArray<int>    wire(reader,  "wireNum");

	// 2D-Histogramme pro Draht: ToT vs Driftzeit (mit Offset, ohne TW-Korrektur)
	std::map<std::pair<int,int>, TH2D*> h2_map;
	std::map<std::pair<int,int>, TH2D*> h2_corr;

	std::cout << "Erste Schleife: 2D-Histogramme fuellen..." << std::endl;

	// 1. Schleife: 2D-Histogramme fuellen
	while(reader.Next())
	{
		size_t nhits = layer.GetSize();
		for(size_t ih=0; ih<nhits; ++ih)
		{
			int L = layer[ih];
			int W = wire[ih];
			auto key = std::make_pair(L,W);

			double lead = (*leading)[ih];
			double trail = (*trailing)[ih];
			double trig = tTrig[ih];

			double tot = std::fabs(lead - trail);

			if(tot < gToTmin || tot > gToTmax) { continue; }

			double rawT = lead - trig;

//			double offset = 0.0;
//			auto itOff = gWireOffset.find(key);
//			if(itOff != gWireOffset.end())
//			{
//				offset = itOff->second;
//			}

            
			double t_corr = rawT;// + offset;

            
			if(t_corr < gDriftMin || t_corr > gDriftMax) { continue; }

			if(!h2_map[key])
			{
				h2_map[key] = new TH2D(Form("h2_L%d_W%d",L,W),
						Form("ToT vs Driftzeit L%d W%d;ToT/ns;Driftzeit/ns",L,W),
						gNBinsToT, gToTaxisMin, gToTaxisMax,
						gNBinsT,  gTaxisMin,   gTaxisMax);
			}

			h2_map[key]->Fill(tot, t_corr);
		}
	}

	std::cout << "Anzahl gefuellter 2D-Histogramme: " << h2_map.size() << std::endl;

	// Time-Walk-Fits pro Draht
	std::map<std::pair<int,int>, TF1*> twFit;

	std::cout << "Time-Walk-Fits pro Draht..." << std::endl;

	for(auto& kv : h2_map)
	{
		int L = kv.first.first;
		int W = kv.first.second;
		TH2D* h2 = kv.second;

		if(h2->GetEntries() < 100)
		{
			std::cout << "Zu wenig Statistik fuer L=" << L << " W=" << W
				<< " -> ueberspringe Fit." << std::endl;
			twFit[kv.first] = nullptr;
			continue;
		}

		TF1* f = fittimewalk(h2);
		twFit[kv.first] = f;

		if(f)
		{
			std::cout << "Fit-Parameter fuer layer " << L << " wire " << W << ":" << std::endl;
			std::cout << "p0=" << f->GetParameter(0)
				<< "  p1=" << f->GetParameter(1)
				//<< "  p2=" << f->GetParameter(2)
				//<< "  p3=" << f->GetParameter(3)
				<< std::endl;

			std::cout << "Ch2/NDF=" << f->GetChisquare() << " / " << f->GetNDF()
				<< " = " << f->GetChisquare() / f->GetNDF()
				<< std::endl;
		}

		else
		{
			std::cout << "Fit fuer layer " << L << " wire " << W << " fehlgeschlagen!" << std::endl;
		}
	}

	// Ausgabe-ROOT-File
	TFile* fout = new TFile(gOutputRootFile.c_str(), "RECREATE");

	// 2D-Histogramme und Fits speichern
	fout->mkdir("timewalk_hists");
	fout->cd("timewalk_hists");

	for(auto& kv : h2_map)
	{
		kv.second->Write();
		if(twFit[kv.first])
		{
			twFit[kv.first]->Write();
		}
	}
	fout->cd();

	// Neuen TTree mit korrigierten Driftzeiten anlegen
	TTree* tout = new TTree(gOutputTreeName.c_str(), "Tree mit time-walk-korrigierten Driftzeiten");

	// Wir kopieren die Struktur: pro Event Vektoren
	std::vector<int>    out_layer;
	std::vector<int>    out_wire;
	std::vector<double> out_leading;
	std::vector<double> out_trailing;
	std::vector<double> out_tTrig;
	std::vector<double> out_tot;
	std::vector<double> out_t_raw;
//	std::vector<double> out_t_offset;
	std::vector<double> out_t_tw_corr;

	tout->Branch("layerID",   &out_layer);
	tout->Branch("wireNum",   &out_wire);
	tout->Branch("leading",   &out_leading);
	tout->Branch("trailing",  &out_trailing);
	tout->Branch("tdcTrigTime",&out_tTrig);
	tout->Branch("tot",       &out_tot);
	tout->Branch("t_raw",     &out_t_raw);
//	tout->Branch("t_offset",  &out_t_offset);
	tout->Branch("t_tw_corr", &out_t_tw_corr);

	// Zweite Schleife: Time-Walk-Korrektur anwenden und in neuen Tree schreiben
	reader.Restart();

	std::cout << "Zweite Schleife: Time-Walk-Korrektur anwenden und Tree schreiben..." << std::endl;

	while(reader.Next())
	{
		out_layer.clear();
		out_wire.clear();
		out_leading.clear();
		out_trailing.clear();
		out_tTrig.clear();
		out_tot.clear();
		out_t_raw.clear();
//		out_t_offset.clear();
		out_t_tw_corr.clear();

		size_t nhits = layer.GetSize();
		for(size_t ih=0; ih<nhits; ++ih)
		{
			int L = layer[ih];
			int W = wire[ih];
			auto key = std::make_pair(L,W);

			double lead = (*leading)[ih];
			double trail = (*trailing)[ih];
			double trig = tTrig[ih];

			double tot = std::fabs(lead - trail);
			double rawT = lead - trig;

//			double offset = 0.0;
//			auto itOff = gWireOffset.find(key);
//			if(itOff != gWireOffset.end()) { offset = itOff->second; }

			double t_corr = rawT;// + offset;

			// Time-Walk-Korrektur
			double t_tw_corr = t_corr;
			TF1* f = twFit[key];
			if(f)
			{
				double tw = f->Eval(tot);
				// hier ggf. noch eine Referenzzeit tref einbauen,
				// aktuell wird einfach der Fitwert abgezogen:
				t_tw_corr = t_corr - (tw - gTref);

				if(!h2_corr[key])
				{
					h2_corr[key] = new TH2D(Form("h2corr_L%d_W_%d",L,W),
							Form("TimeWalk-korrigiert L%d W%d;ToT/ns;Driftzeit TWcorr/n",L,W),
							gNBinsToT,gToTaxisMin,gToTaxisMax,
							gNBinsT,gTaxisMin,gTaxisMax);
				}
			
				h2_corr[key]->Fill(tot,t_tw_corr);
			}


			// Optional: hier nochmal Gates anwenden:
			// if(tot < gToTmin || tot > gToTmax) continue;
			// if(t_corr < gDriftMin || t_corr > gDriftMax) continue;

			out_layer.push_back(L);
			out_wire.push_back(W);
			out_leading.push_back(lead);
			out_trailing.push_back(trail);
			out_tTrig.push_back(trig);
			out_tot.push_back(tot);
			out_t_raw.push_back(rawT);
//			out_t_offset.push_back(t_corr);
			out_t_tw_corr.push_back(t_tw_corr);
		}

		tout->Fill();
	}


	// --- PDF-Ausgabe ---
	TCanvas* c = new TCanvas("c","Time-Walk Plots",900,700);
	c->Print("timewalk_plots.pdf[");

	for(auto& kv : h2_map)
	{
		int L = kv.first.first;
		int W = kv.first.second;

		TH2D* h2  = kv.second;
		TH2D* h2c = h2_corr[kv.first];
		TF1*  f   = twFit[kv.first];

		// --- Plot 1: Unkorrigiert ---
		c->cd();
		h2->Draw("COLZ");
		if(f) { f->Draw("same"); }
		c->Print("timewalk_plots.pdf");

    
		// --- Plot 2: Korrigiert ---
		if(h2c)
		{
			c->cd();
			h2c->Draw("COLZ");
			std::cout << "Print for layer " << L << " wire " << W << std::endl;
			c->Print("timewalk_plots.pdf");
		}
	}

	c->Print("timewalk_plots.pdf]");



	fout->cd();
	tout->Write();
	fout->Close();

	std::cout << "Fertig. Ausgabe geschrieben nach " << gOutputRootFile << std::endl;
}

