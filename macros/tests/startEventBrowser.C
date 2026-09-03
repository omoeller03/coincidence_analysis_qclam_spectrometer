#include <vector>
#include <algorithm>
#include <iostream>

TTree* t = nullptr;
std::vector<int> eventList;
int currentEvent = 0;

// ------------------------------------------------------------
// Zeichnet das aktuelle Event als TGraph
// ------------------------------------------------------------
void drawEvent(int _layer)
{
    if (!t) return;

    int event, wire, cluster, layer;
    double drift;
    t->SetBranchAddress("eventNumber",&event);
    t->SetBranchAddress("wireNumber",&wire);
    t->SetBranchAddress("drifttime",&drift);
    t->SetBranchAddress("layer", &layer);
    t->SetBranchAddress("clusterNumber",&cluster);

    TGraph* g = new TGraph();
    g->SetTitle(Form("Event %d;wire;drift", eventList[currentEvent]));

    int n = 0;
    Long64_t N = t->GetEntries();

    for (Long64_t i=0;i<N;i++) {
        t->GetEntry(i);
	std::cout << "currentEvent: " << std::to_string(currentEvent) << std::endl;

        if (event == eventList[currentEvent] && cluster == 0 && layer==_layer) {
            g->SetPoint(n, wire, drift);
            n++;
        }
    }

    g->SetMarkerStyle(20);
    g->SetMarkerSize(0.8);

    g->Draw("AP");
    gPad->Update();
}

// ------------------------------------------------------------
// Nächstes Event
// ------------------------------------------------------------
void nextEvent(int layer)
{
    if (currentEvent < (int)eventList.size()-1)
        currentEvent++;

    drawEvent(layer);
}

// ------------------------------------------------------------
// Vorheriges Event
// ------------------------------------------------------------
void prevEvent(int layer)
{
    if (currentEvent > 0)
        currentEvent--;

    drawEvent(layer);
}

// ------------------------------------------------------------
// Startet den Event-Browser
// ------------------------------------------------------------
void startEventBrowser(TTree* tree, int layer)
{
    t = tree;

    // Eventliste erzeugen
    Long64_t n = t->Draw("eventNumber","clusterNumber>=0","goff");
    Double_t* ev = t->GetV3();

    for (Long64_t i=0;i<n;i++)
        eventList.push_back((int)ev[i]);

    std::sort(eventList.begin(), eventList.end());
    eventList.erase(std::unique(eventList.begin(), eventList.end()), eventList.end());

    currentEvent = 0;

    // Canvas + Buttons
    TCanvas* c = new TCanvas("c","Event Browser",900,700);

    TButton* bNext = new TButton("Next","nextEvent()",0.80,0.92,0.95,0.99);
    bNext->Draw();

    TButton* bPrev = new TButton("Prev","prevEvent()",0.05,0.92,0.20,0.99);
    bPrev->Draw();

    drawEvent(layer);
}

