#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"

#include <map>
#include <vector>
#include <cmath>
#include <iostream>

struct DriftWindow {
    double min;
    double max;
};

// ============================================================
// CONFIG
// ============================================================

std::string gInputFile  = "/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run599/TQClamDrifttimeCalibStore_599.root";
std::string gTreeName   = "DrifttimeCalib";

std::string gOutputFile = "timewalk_hyper_final.root";

bool gInvertDrift = true;

// ToT cuts
double gToTmin = 20;
double gToTmax = 80;

// RAW drift window (BEVOR inversion!)
double gDriftRawMin = 0;
double gDriftRawMax = 2000;

// AFTER inversion expected window (for safety cuts)
double gDriftInvMin = -2000;
double gDriftInvMax = 2000;

// Layer-dependent expected peak (nur Debug/seed)
std::map<int, DriftWindow> gLayerWindow = {
    {0,{50,80}},
    {2,{50,80}},
    {1,{15,35}}
};

// histogram bins
int nTotBins = 80;
double totMin = 0;
double totMax = 80;

int nTBins = 300;
double tMin = -2000;
double tMax = 2000;

// ============================================================
// Hyperbolic fit: p0 + p1/(x - p2)
// ============================================================

TF1* MakeHyperFit(const char* name)
{
    TF1* f = new TF1(name,
        "[0] + [1]/(x - [2])",
        gToTmin, gToTmax);

    f->SetParameters(0, 1000, 10);

    f->SetParLimits(2, 0.1, 200);
    return f;
}

// ============================================================
// MAIN FIT
// ============================================================

TF1* FitTimeWalk(TH2D* h2)
{
    if(!h2 || h2->GetEntries() < 200)
        return nullptr;

    std::vector<double> x,y;

    for(int i=1;i<=h2->GetNbinsX();i++)
    {
        TH1D* proj = h2->ProjectionY("_py", i, i);

        if(proj->GetEntries() < 20) {
            delete proj;
            continue;
        }

        int maxBin = proj->GetMaximumBin();
        double peak = proj->GetBinCenter(maxBin);

        double tot = h2->GetXaxis()->GetBinCenter(i);

        x.push_back(tot);
        y.push_back(peak);

        delete proj;
    }

    if(x.size() < 10)
        return nullptr;

    TGraph gr(x.size(), &x[0], &y[0]);

    TF1* f = MakeHyperFit("hyper");

    gr.Fit(f, "Q");

    return f;
}

// ============================================================
// MAIN
// ============================================================

void timewalk_hyper_final()
{
    TFile* fin = TFile::Open(gInputFile.c_str());
    TTree* t = (TTree*)fin->Get(gTreeName.c_str());

    TTreeReader reader(t);

    TTreeReaderArray<double> lead(reader,"leading");
    TTreeReaderArray<double> trail(reader,"trailing");
    TTreeReaderArray<double> trig(reader,"tdcTrigTime");
    TTreeReaderArray<int> layer(reader,"layerID");
    TTreeReaderArray<int> wire(reader,"wireNum");

    std::map<std::pair<int,int>, TH2D*> h2;

    std::cout << "Fill..." << std::endl;

    while(reader.Next())
    {
        int n = layer.GetSize();

        for(int i=0;i<n;i++)
        {
            int L = layer[i];
            int W = wire[i];

            double tot = fabs(lead[i]-trail[i]);

            if(tot < gToTmin || tot > gToTmax)
                continue;

            double t = lead[i] - trig[i];

            if(gInvertDrift)
                t = -t;

            if(t < gDriftInvMin || t > gDriftInvMax)
                continue;

            auto key = std::make_pair(L,W);

            if(!h2[key])
            {
                h2[key] = new TH2D(
                    Form("h2_L%d_W%d",L,W),
                    "ToT vs Drift;ToT;Drift",
                    nTotBins,totMin,totMax,
                    nTBins,tMin,tMax
                );
            }

            h2[key]->Fill(tot,t);
        }
    }

    std::cout << "histos: " << h2.size() << std::endl;

    std::map<std::pair<int,int>, TF1*> fits;

    for(auto& kv : h2)
        fits[kv.first] = FitTimeWalk(kv.second);

    // OUTPUT
    TFile fout(gOutputFile.c_str(),"RECREATE");

    TCanvas c;
    c.Print("timewalk_hyper_final.pdf[");

    for(auto& kv : h2)
    {
        kv.second->Write();

        c.cd();
        kv.second->Draw("COLZ");

        if(fits[kv.first])
            fits[kv.first]->Draw("same");

        c.Print("timewalk_hyper_final.pdf");
    }

    c.Print("timewalk_hyper_final.pdf]");

    fout.Write();
    fout.Close();

    std::cout << "DONE" << std::endl;
}
