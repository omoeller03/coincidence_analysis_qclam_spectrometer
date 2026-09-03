void Process(TObject* obj)
{
    // TBrowser übergibt IMMER ein TObject*
    TTree* t = dynamic_cast<TTree*>(obj);
    if (!t) {
        std::cout << "Das ausgewählte Objekt ist kein TTree" << std::endl;
        return;
    }

    drawCluster(t);
}

void drawCluster(TTree* t)
{
    int event, wire, layer, cluster;
    double drift;

    t->SetBranchAddress("eventNumber",&event);
    t->SetBranchAddress("wireNumber",&wire);
    t->SetBranchAddress("drifttime",&drift);
    t->SetBranchAddress("layer",&layer);
    t->SetBranchAddress("clusterNumber",&cluster);

    int ev;
    std::cout << "Welches Event anzeigen? ";
    std::cin >> ev;

    TCanvas* c = new TCanvas("c","Cluster Display",900,700);
    c->Divide(1,3);

    for (int lay=0; lay<3; lay++) {
        c->cd(lay+1);
        gPad->DrawFrame(0,0,200,400,Form("Layer %d;wire;drift", lay));
    }

    Long64_t N = t->GetEntries();

    for (Long64_t i=0;i<N;i++) {
        t->GetEntry(i);

        if (event == ev && cluster >= 0) {
            c->cd(layer+1);
            TLine* L = new TLine(wire, 0, wire, drift);
            L->SetLineWidth(3);
            L->Draw();
        }
    }

    c->Update();
}

