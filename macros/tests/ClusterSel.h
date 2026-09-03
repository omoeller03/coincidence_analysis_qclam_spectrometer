#ifndef ClusterSel_h
#define ClusterSel_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TCanvas.h>
#include <TLine.h>

class ClusterSel : public TSelector {
public:
    TTree          *fChain = nullptr;

    // Branches
    Int_t eventNumber;
    Int_t wireNumber;
    Int_t layer;
    Int_t clusterNumber;
    Double_t drifttime;

    // Canvas + Event-Auswahl
    TCanvas* c = nullptr;
    Int_t wantedEvent = -1;

    ClusterSel() {}
    virtual ~ClusterSel() {}
    virtual Int_t   Version() const { return 2; }
    virtual void    Begin(TTree *tree);
    virtual void    SlaveBegin(TTree *tree);
    virtual void    Init(TTree *tree);
    virtual Bool_t  Process(Long64_t entry);
    virtual void    SlaveTerminate();
    virtual void    Terminate();

    ClassDef(ClusterSel,0);
};

#endif

