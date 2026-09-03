#define ClusterSel_cxx
#include "ClusterSel.h"
#include <TH2.h>
#include <TStyle.h>
#include <iostream>

void ClusterSel::Init(TTree *tree)
{
    if (!tree) return;
    fChain = tree;

    fChain->SetBranchAddress("eventNumber",   &eventNumber);
    fChain->SetBranchAddress("wireNumber",    &wireNumber);
    fChain->SetBranchAddress("layer",         &layer);
    fChain->SetBranchAddress("clusterNumber", &clusterNumber);
    fChain->SetBranchAddress("drifttime",     &drifttime);
}

void ClusterSel::Begin(TTree *)
{
    std::cout << "Welches Event anzeigen? ";
    std::cin >> wantedEvent;
}

void ClusterSel::SlaveBegin(TTree *)
{
}

Bool_t ClusterSel::Process(Long64_t entry)
{
    fChain->GetEntry(entry);

    if (eventNumber != wantedEvent) return kTRUE;

    if (!c) {
        c = new TCanvas("c","Cluster Display",900,700);
        c->Divide(1,3);

        for (int lay=0; lay<3; lay++) {
            c->cd(lay+1);
            gPad->DrawFrame(0,0,200,400,Form("Layer %d;wire;drift", lay));
        }
    }

    c->cd(layer+1);
    TLine* L = new TLine(wireNumber, 0, wireNumber, drifttime);
    L->SetLineWidth(3);
    L->SetLineColor(kBlue+1);
    L->Draw();

    return kTRUE;
}

void ClusterSel::SlaveTerminate()
{
}

void ClusterSel::Terminate()
{
}

