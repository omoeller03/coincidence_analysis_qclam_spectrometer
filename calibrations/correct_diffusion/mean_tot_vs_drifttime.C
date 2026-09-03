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

#include <vector>
#include <iostream>
#include <cmath>

std::string gInputFile = "../time_walk/timewalk_corrected_run_80_91.root";
std::string gTreeName  = "DrifttimeCalib_TWcorr";

int gLayer = 0;
int gWire  = 30;

// Histogramm-Binning
int gNBinsT = 300;
double gTmin = 0.;
double gTmax = 300.;

int gNBinsToT = 200;
double gToTmin = 0.;
double gToTmax = 300.;

// Cuts
double gMinEntries = 30;

// Driftzeit-Fenster
double tMin = 50.0;
double tMax = 250.0;

void mean_tot_vs_drifttime()
{
    TFile *fin = TFile::Open(gInputFile.c_str(),"READ");

    if(!fin || fin->IsZombie())
    {
        std::cout << "input file not found" << std::endl;
        return;
    }

    TTree *t = (TTree*)fin->Get(gTreeName.c_str());

    if(!t)
    {
        std::cout << "tree not found" << std::endl;
        return;
    }

    TTreeReader reader(t);

    TTreeReaderArray<int> layer(reader,"layerID");
    TTreeReaderArray<int> wire(reader,"wireNum");

    TTreeReaderArray<double> tot(reader,"tot");
    TTreeReaderArray<double> tcorr(reader,"t_tw_corr");

    // 2D Histogram
    TH2D *h2 = new TH2D(
        "h2",
        Form("Layer %d Wire %d;Driftzeit / ns;ToT / ns", gLayer, gWire),
        gNBinsT, gTmin, gTmax,
        gNBinsToT, gToTmin, gToTmax
    );

    // Fill
    while(reader.Next())
    {
        size_t nhits = layer.GetSize();

        for(size_t i = 0; i < nhits; i++)
        {
            if(layer[i] != gLayer) continue;
            if(wire[i]  != gWire)  continue;

            h2->Fill(tcorr[i], tot[i]);
        }
    }

    std::cout << "entries = " << h2->GetEntries() << std::endl;

    // Driftzeit-Bins
    int binMin = h2->GetXaxis()->FindBin(tMin);
    int binMax = h2->GetXaxis()->FindBin(tMax);

    int sliceWidth = 5;

    std::vector<double> xvals;
    std::vector<double> yvals;
    std::vector<double> yerrs;

    // Canvas
    TCanvas *c1 = new TCanvas("c1","Mean ToT vs Drifttime",900,700);

    c1->Print("mean_tot_vs_drifttime.pdf[");

    // Loop over drift slices
    for(int bx = binMin; bx <= binMax; bx += sliceWidth)
    {
        TH1D *hSlice = h2->ProjectionY(
            Form("slice_%d", bx),
            bx,
            std::min(bx + sliceWidth - 1, h2->GetNbinsX())
        );

        if(hSlice->GetEntries() < gMinEntries)
        {
            delete hSlice;
            continue;
        }

        // Mean + Error
        double mean = hSlice->GetMean();
        double meanErr = hSlice->GetMeanError();

        double drift = h2->GetXaxis()->GetBinCenter(bx);

        xvals.push_back(drift);
        yvals.push_back(mean);
        yerrs.push_back(meanErr);

        // optional plot per slice
        c1->cd();
        hSlice->Draw();
        c1->Print("mean_tot_vs_drifttime.pdf");

        delete hSlice;
    }

    // Graph
    TGraphErrors *gr = new TGraphErrors(
        xvals.size(),
        &xvals[0],
        &yvals[0],
        nullptr,
        &yerrs[0]
    );

    gr->SetTitle(Form("Layer %d Wire %d;Driftzeit / ns;Mean ToT / ns", gLayer, gWire));
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(0.8);

    // Fit: sqrt law
    TF1 *f = new TF1("f", "[0]+sqrt(2*[1]*(x-[2]))/(1 + (x-[2])/[3])", tMin, tMax);
    //TF1 *f = new TF1("f", "[0]+sqrt(2*[1]*(x-[2]))", tMin, tMax);
    f->SetParameters(30., 1.0,50,10);
    //f->SetParameters(30., 1.0,50);

    gr->Fit(f, "R");

    c1->Clear();
    gr->Draw("AP");
    f->Draw("same");

    c1->Print("mean_tot_vs_drifttime.pdf");

    // Fit: power law

    /*
    TF1 *fpow = new TF1("fpow", "[0]+[1]*pow(x,[2])", gTmin, gTmax);
    fpow->SetParameters(30., 1.0, 0.5);

    gr->Fit(fpow, "R");

    c1->Clear();
    gr->Draw("AP");
    fpow->Draw("same");
    */

    //c1->Print("mean_tot_vs_drifttime.pdf");

    // Output
    std::cout << "\nsqrt-fit:\n"
              << "a = " << f->GetParameter(0)
              << "\nb = " << f->GetParameter(1)
              << "\nc = " << f->GetParameter(2)
              << "\nd = " << f->GetParameter(3)
              << std::endl;

    /*
    std::cout << "\npowerlaw:\n"
              << "a = " << fpow->GetParameter(0)
              << "\nb = " << fpow->GetParameter(1)
              << "\nc = " << fpow->GetParameter(2)
              << " ± " << fpow->GetParError(2)
              << std::endl;
	      */

    // Save
    TFile *fout = new TFile("mean_tot_vs_drifttime.root","RECREATE");

    h2->Write();
    gr->Write();
    f->Write();
    //fpow->Write();

    fout->Close();

    c1->Print("mean_tot_vs_drifttime.pdf]");

    fin->Close();
}
