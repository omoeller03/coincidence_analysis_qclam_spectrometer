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

// Histogramm

int gNBinsT = 300;
double gTmin = 0.;
double gTmax = 300.;

int gNBinsToT = 200;
double gToTmin = 20.;
double gToTmax = 30.;


// Slice-Einstellungen

int gMinEntries = 30;

double gFitWindow = 50.0;

double gPeakSearchMin = 20.;
double gPeakSearchMax = 30.;


double gMinDrifttime = 60.;
double gMaxDrifttime = 250.;



void fit_tot_vs_drifttime()
{

    TFile *fin = TFile::Open(gInputFile.c_str(),"READ");

    if(!fin || fin->IsZombie())
    {
        std::cout<<"input file not found"<<std::endl;
        return;
    }

    TTree *t=(TTree*)fin->Get(gTreeName.c_str());

    if(!t)
    {
        std::cout<<"tree not found"<<std::endl;
        return;
    }

    TTreeReader reader(t);

    TTreeReaderArray<int> layer(reader,"layerID");
    TTreeReaderArray<int> wire(reader,"wireNum");

    TTreeReaderArray<double> tot(reader,"tot");
    TTreeReaderArray<double> tcorr(reader,"t_tw_corr");



    TH2D *h2=new TH2D(
            "h2",
            Form("Layer %d  Wire %d;Driftzeit / ns;ToT / ns",
                gLayer,gWire),
            gNBinsT,
            gTmin,
            gTmax,
            gNBinsToT,
            gToTmin,
            gToTmax);



    while(reader.Next())
    {

        size_t nhits=layer.GetSize();

        for(size_t i=0;i<nhits;i++)
        {

            if(layer[i]!=gLayer) continue;

            if(wire[i]!=gWire) continue;

            h2->Fill(
                    tcorr[i],
                    tot[i]);

        }

    }

    std::cout<<"entries = "<<h2->GetEntries()<<std::endl;


    //------------------------------------------------------------
    // Canvas + PDF
    //------------------------------------------------------------

    TCanvas *c1 = new TCanvas("c1","Slices",900,700);

    c1->Print("tot_vs_drifttime_fit.pdf[");

    //------------------------------------------------------------
    // Punkte fuer den Fit
    //------------------------------------------------------------

    std::vector<double> xvals;
    std::vector<double> yvals;
    std::vector<double> yerrs;

    //------------------------------------------------------------
    // ueber Driftzeit-Bins iterieren
    //------------------------------------------------------------

    /*
    for(int bx=1; bx<=h2->GetNbinsX(); ++bx)
    {
    */
    int sliceWidth = 5;

    int binMin = h2->GetXaxis()->FindBin(gMinDrifttime);
    int binMax = h2->GetXaxis()->FindBin(gMaxDrifttime);
//for(int bx=1; bx<=h2->GetNbinsX(); bx += sliceWidth)
for(int bx=binMin; bx<=binMax; bx += sliceWidth)
{
    TH1D *hSlice = h2->ProjectionY(
        Form("slice_%d",bx),
        bx,
        std::min(bx+sliceWidth-1,h2->GetNbinsX()));

    /*
        TH1D *hSlice = h2->ProjectionY(
                            Form("slice_%d",bx),
                            bx,
                            bx);
			    */

        if(hSlice->GetEntries()<gMinEntries)
        {
            delete hSlice;
            continue;
        }

        //--------------------------------------------------------
        // Maximum suchen
        //--------------------------------------------------------

        int bmin = hSlice->FindBin(gPeakSearchMin);
        int bmax = hSlice->FindBin(gPeakSearchMax);

        int maxbin = bmin;
        double maxcontent = 0.;

        for(int b=bmin;b<=bmax;++b)
        {
            if(hSlice->GetBinContent(b)>maxcontent)
            {
                maxcontent=hSlice->GetBinContent(b);
                maxbin=b;
            }
        }

        double peak0 = hSlice->GetBinCenter(maxbin);

        //--------------------------------------------------------
        // lokaler Gaußfit
        //--------------------------------------------------------

        TF1 *g = new TF1(
                    Form("g_%d",bx),
                    "gaus",
                    peak0-gFitWindow,
                    peak0+gFitWindow);

        g->SetParameters(
                maxcontent,
                peak0,
                0.5);

        hSlice->Fit(g,"RQ0");

        double peak    = g->GetParameter(1);
        double peakErr = g->GetParError(1);

        double drifttime =
            h2->GetXaxis()->GetBinCenter(bx);

        xvals.push_back(drifttime);
        yvals.push_back(peak);
        yerrs.push_back(peakErr);

        //--------------------------------------------------------
        // Plot
        //--------------------------------------------------------

        c1->cd();

        hSlice->Draw();
        g->Draw("same");

        c1->Print("tot_vs_drifttime_fit.pdf");

        delete hSlice;
        delete g;
    }



    //------------------------------------------------------------
    // TGraph erzeugen
    //------------------------------------------------------------

    TGraphErrors *gr = new TGraphErrors(
            xvals.size(),
            &xvals[0],
            &yvals[0],
            nullptr,
            &yerrs[0]);

    gr->SetName("gr_tot_vs_dt");

    gr->SetTitle(
            Form("Layer %d  Wire %d;Driftzeit / ns;ToT / ns",
                gLayer,
                gWire));

    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(0.8);


    //------------------------------------------------------------
    // Fit:
    // ToT = a + b * sqrt(t)
    //------------------------------------------------------------

    TF1 *f = new TF1(
            "f",
            "[0]+sqrt(2*[1]*(x-[2]))",
            gTmin,
            gTmax);

    f->SetParameters(30.,0.003,50);

    gr->Fit(f,"R");


    //------------------------------------------------------------
    // Plot
    //------------------------------------------------------------

    c1->Clear();

    gr->Draw("AP");

    f->Draw("same");

    c1->Print("tot_vs_drifttime_fit.pdf");


    //------------------------------------------------------------
    // alternativ:
    // ToT = a + b * x^c
    //------------------------------------------------------------

    TF1 *fpow = new TF1(
            "fpow",
            "[0]+[1]*pow(x,[2])",
            gTmin,
            gTmax);

    fpow->SetParameters(
            30.,
            1.,
            0.5);

    gr->Fit(fpow,"R");

    c1->Clear();

    gr->Draw("AP");

    fpow->Draw("same");

    c1->Print("tot_vs_drifttime_fit.pdf");


    //------------------------------------------------------------
    // Ausgabe
    //------------------------------------------------------------

    std::cout << std::endl;

    std::cout
        << "sqrt-Fit:\n"
        << "Offset = "
        << f->GetParameter(0)
        << "\nSteigung = "
        << f->GetParameter(1)
        << std::endl;

    std::cout << std::endl;

    std::cout
        << "Powerlaw:\n"
        << "Offset = "
        << fpow->GetParameter(0)
        << "\nAmplitude = "
        << fpow->GetParameter(1)
        << "\nExponent = "
        << fpow->GetParameter(2)
        << " +- "
        << fpow->GetParError(2)
        << std::endl;


    //------------------------------------------------------------
    // Histogramm speichern
    //------------------------------------------------------------

    TFile *fout =
        new TFile(
            "tot_vs_drifttime_fit.root",
            "RECREATE");

    h2->Write();

    gr->Write();

    f->Write();

    fpow->Write();

    fout->Close();


    //------------------------------------------------------------

    c1->Print("tot_vs_drifttime_fit.pdf]");

    fin->Close();

}


