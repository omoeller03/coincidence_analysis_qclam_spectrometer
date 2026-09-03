#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <cmath>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "TH1D.h"
#include "TCanvas.h"
#include "TStyle.h"

using namespace std;




double NormalCDF(double x,
                 double mu,
                 double sigma)
{
    return 0.5*(1.0+TMath::Erf((x-mu)/(std::sqrt(2.)*sigma)));
}




double SkewedGaussian(double *x,double *par)
{
    double t=x[0];

    double Ap=par[0];
    double t0=par[1];
    double sigma=par[2];
    double s=par[3];

    return
        Ap*
        exp(-0.5*pow((t-t0)/sigma,2.0))*
        (1.+TMath::Erf(
            s*(t-t0)/(sqrt(2.)*sigma)
        ));
}




double DriftModel(double *x,double *par)
{
    double t=x[0];

    double Ap      = par[0];
    double t0      = par[1];
    double sigma   = par[2];
    double s       = par[3];

    double At      = par[4];
    double sigma_e = par[5];
    double sigma_t = par[6];
    double L       = par[7];

    double peak =
        Ap*
        exp(-0.5*pow((t-t0)/sigma,2.0))*
        (1.+TMath::Erf(
            s*(t-t0)/(sqrt(2.)*sigma)));

    double plateau =
        At*
        NormalCDF(t,t0,sigma_e)*
        (1.-NormalCDF(t,t0+L,sigma_t));

    return peak+plateau;
}




////////////////////////////////////////////////////////////////////////////////
// Konfiguration
////////////////////////////////////////////////////////////////////////////////

void generateDriftCorrTable(
    const char* inputFile = "Track.root",
    const char* treeName = "Track",
    const char* outputDir = ".")
{

//=============================================================================
// Winkel
//=============================================================================

double phiMin = 25.0;
double phiMax = 65.0;
double phiBinWidth = 1.0;

//=============================================================================
// Ausgabe-Tabelle
//=============================================================================

double phiTableMin = 25.0;
double phiTableMax = 65.0;
double phiTableBinWidth = 1.0;

//=============================================================================
// Driftzeit
//=============================================================================

double tMin = 0.0;
double tMax = 250.0;
double dt   = 0.5;

//=============================================================================
// Driftzellenhöhe
//=============================================================================

double driftCellHeight = 12.0;

//=============================================================================
// Drahtbereiche
//=============================================================================

vector<pair<int,int>> wireRanges =
{
    {1,50}
};

//=============================================================================
// Nur bestimmte Drähte benutzen
// leer => alle Drähte aus wireRanges
//=============================================================================

set<int> wireList =
{
//5,
//6,
//17
};

//=============================================================================
// Ausgeschlossene Drähte
//=============================================================================

set<int> excludedWires =
{
//13,
//14,
//27
};

//=============================================================================
// Qualitätscuts
//=============================================================================

bool usePullCut=false;
double maxPull=3.0;

bool useResidualCut=false;
double maxResidual=0.5;

bool useTOTCut=false;
double minTOT=0;
double maxTOT=300;

//=============================================================================

bool useAbsolutePhi=true;

//=============================================================================
// Layernamen
//=============================================================================

const char* layerName[3]=
{
    "X1",
    "U",
    "X2"
};

////////////////////////////////////////////////////////////////////////////////
//
// Hilfsfunktionen
//
////////////////////////////////////////////////////////////////////////////////

auto acceptWire =
[&](int wire)
{

    bool inside=false;

    for(auto const &r : wireRanges)
    {
        if(wire>=r.first && wire<=r.second)
        {
            inside=true;
            break;
        }
    }

    if(!inside)
        return false;

    if(!wireList.empty())
    {
        if(wireList.find(wire)==wireList.end())
            return false;
    }

    if(excludedWires.find(wire)!=excludedWires.end())
        return false;

    return true;
};

////////////////////////////////////////////////////////////////////////////////
//
// ROOT öffnen
//
////////////////////////////////////////////////////////////////////////////////

TFile *fin = TFile::Open(inputFile);

if(fin==nullptr)
{
    cout<<"Cannot open "<<inputFile<<endl;
    return;
}

TTree *tree = (TTree*)fin->Get(treeName);

if(tree==nullptr)
{
    cout<<"Tree "<<treeName<<" not found"<<endl;
    return;
}

cout<<"Entries : "<<tree->GetEntries()<<endl;

////////////////////////////////////////////////////////////////////////////////
//
// TreeReader
//
////////////////////////////////////////////////////////////////////////////////

TTreeReader reader(tree);

TTreeReaderValue<double> phi(reader);

TTreeReaderValue<vector<double>> drifttimes(reader);
TTreeReaderValue<vector<double>> tot(reader);
TTreeReaderValue<vector<double>> residual(reader);
TTreeReaderValue<vector<double>> pull(reader);

TTreeReaderValue<vector<int>> layerID(reader);
TTreeReaderValue<vector<int>> wireNumber(reader);

////////////////////////////////////////////////////////////////////////////////
//
// Winkel vorbereiten
//
////////////////////////////////////////////////////////////////////////////////

vector<double> phiBins;

for(double p=phiMin;p<=phiMax+1e-6;p+=phiBinWidth)
    phiBins.push_back(p);

const int nPhiBins=phiBins.size();

cout<<"Phi bins = "<<nPhiBins<<endl;

////////////////////////////////////////////////////////////////////////////////
//
// Histogramme
//
////////////////////////////////////////////////////////////////////////////////

vector<vector<TH1D*>> hist(3);

for(int layer=0;layer<3;layer++)
{

    hist[layer].resize(nPhiBins);

    for(int b=0;b<nPhiBins;b++)
    {

        TString hname;

        hname.Form("h_%s_%03d",
                   layerName[layer],
                   b);

        hist[layer][b]=new TH1D(
            hname,
            hname,
            int((tMax-tMin)/dt),
            tMin,
            tMax);

    }

}


////////////////////////////////////////////////////////////////////////////////
//
// Tree durchlaufen
//
////////////////////////////////////////////////////////////////////////////////

Long64_t nTracks = 0;
Long64_t nHits = 0;
Long64_t nHitsAccepted = 0;

while(reader.Next())
{
    ++nTracks;

    double phiTrack = *phi;

    if(useAbsolutePhi)
        phiTrack = std::abs(phiTrack);

    if(phiTrack < phiMin)
        continue;

    if(phiTrack >= phiMax)
        continue;

    int phiBin = int((phiTrack - phiMin) / phiBinWidth);

    if(phiBin < 0 || phiBin >= nPhiBins)
        continue;

    const size_t n = drifttimes->size();

    for(size_t i=0;i<n;i++)
    {
        ++nHits;

        int layer = res_layerID->at(i);

        if(layer < 0 || layer > 2)
            continue;

        int wire = res_wireNumber->at(i);

        //--------------------------------------------------------
        // Drahtauswahl
        //--------------------------------------------------------

        if(!acceptWire(wire))
            continue;

        //--------------------------------------------------------
        // Pull-Cut
        //--------------------------------------------------------

        if(usePullCut)
        {
            if(std::abs(pull->at(i)) > maxPull)
                continue;
        }

        //--------------------------------------------------------
        // Residual-Cut
        //--------------------------------------------------------

        if(useResidualCut)
        {
            if(std::abs(residual->at(i)) > maxResidual)
                continue;
        }

        //--------------------------------------------------------
        // TOT-Cut
        //--------------------------------------------------------

        if(useTOTCut)
        {
            double t = tot->at(i);

            if(t < minTOT)
                continue;

            if(t > maxTOT)
                continue;
        }

        //--------------------------------------------------------
        // Driftzeit
        //--------------------------------------------------------

        double dtHit = drifttimes->at(i);

        if(dtHit < tMin)
            continue;

        if(dtHit > tMax)
            continue;

        hist[layer][phiBin]->Fill(dtHit);

        ++nHitsAccepted;
    }
}

cout << endl;
cout << "Tracks gelesen      : " << nTracks << endl;
cout << "Hits gelesen        : " << nHits << endl;
cout << "Hits verwendet      : " << nHitsAccepted << endl;
cout << endl;




////////////////////////////////////////////////////////////////////////////////
//
// Histogramme normieren
//
////////////////////////////////////////////////////////////////////////////////

for(int layer=0; layer<3; layer++)
{
    for(int b=0; b<nPhiBins; b++)
    {
        TH1D* h = hist[layer][b];

        double integral = h->Integral("width");

        if(integral>0)
            h->Scale(1./integral);
    }
}




////////////////////////////////////////////////////////////////////////////////
//
// Kontrollplots
//
////////////////////////////////////////////////////////////////////////////////

for(int layer=0; layer<3; layer++)
{
    TCanvas *c = new TCanvas();

    bool first=true;

    for(int b=0;b<nPhiBins;b++)
    {
        if(first)
        {
            hist[layer][b]->Draw("hist");
            first=false;
        }
        else
        {
            hist[layer][b]->Draw("hist same");
        }
    }

    TString pdf;

    pdf.Form("%s/drifttimeHistograms_%s.pdf",
             outputDir,
             layerName[layer]);

    c->SaveAs(pdf);

    delete c;
}



std::vector<std::vector<TF1*>> fits(3);

for(int layer=0;layer<3;layer++)
{
    fits[layer].resize(nPhiBins);

    for(int b=0;b<nPhiBins;b++)
    {

        TH1D *h=hist[layer][b];

        if(h->GetEntries()<100)
            continue;

        TString fname;

        fname.Form("fit_%d_%d",
                   layer,
                   b);

        TF1 *f=new TF1(
            fname,
            DriftModel,
            tMin,
            tMax,
            8);

        f->SetParNames(
            "Ap",
            "t0",
            "sigma",
            "s",
            "At",
            "sigma_e",
            "sigma_t",
            "L");

        f->SetParameters(

            0.03,     // Ap

            45.,      // t0

            6.,       // sigma

            0.5,      // s

            0.01,     // At

            5.,       // sigma_e

            5.,       // sigma_t

            120.      // L

        );

        f->SetParLimits(0,0,10);
        f->SetParLimits(1,20,80);
        f->SetParLimits(2,1,20);
        f->SetParLimits(3,-5,5);
        f->SetParLimits(4,0,10);
        f->SetParLimits(5,0.5,30);
        f->SetParLimits(6,0.5,30);
        f->SetParLimits(7,20,200);

        h->Fit(f,"RQ0");

        fits[layer][b]=f;

    }
}
