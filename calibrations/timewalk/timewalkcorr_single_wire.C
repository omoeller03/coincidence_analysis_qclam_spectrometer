#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TSystem.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLine.h"
#include "TMath.h"

using namespace std;


// == Konfiguration ==

// Input
const char* gInputDirectory = "/d/d04-1/ag_pi/QCLAM_EEPG_CE140_2026/ana/unpacked_data/calibrations_go4/data/timewalk";

const char* gFilePrefix = "DrifttimeStore_qclam_dago_";
const char* gTreeName   = "DrifttimeCalib";

const int gRunFirst = 34;
const int gRunLast  = 40;


// Zeitdefinition
const double gOffsetTime = 1000.0;

const double gToTmin   = 10.0;
const double gToTmax   = 100.0;

const double gDriftMin = 0.0;
const double gDriftMax = 1000.0;


// Histogramme
const int    gNBinsToT  = 100;
const double gToTaxisMin = 10.0;
const double gToTaxisMax = 100.0;

const int    gNBinsT    = 500;
const double gTaxisMin  = 0.0;
const double gTaxisMax  = 400.0;


// Peakbestimmung
const double gPeakSearchMin   = 10.0;
const double gPeakSearchMax   = 100.0;

const double gPeakFollowWindow = 10.0;
const double gLocalFitWindow   = 7.0;

const double gMaxPeakError = 8.0;
const double gMinLocalIntegral = 5.0;


// Time-Walk-Fit
const double gFitToTmin = 10.0;
const double gFitToTmax = 100.0;

const double gOutlierCut = 25.0;
const double gTref       = 50.0;





int GetLayerID(const string& layer)
{
	if(layer == "X1") { return 0; }
	if(layer == "U") { return 1; } 
	if(layer == "X2") { return 2; }

	return -1;
}


// fit-fkt
double TimeWalkFunction(double* x, double* p)
{
	return p[0] * exp(-p[1] * x[0]) + p[2];
}



// == main ==
void timewalkcorr_single_wire(const char* layerName = "X1", int targetWire = 101)
{
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1111);

    const string layer(layerName);
    const int targetLayer = GetLayerID(layer);

    if(targetLayer < 0)
    {
        cerr << "FEHLER: Unbekannte Ebene '" << layer
             << "'. Erlaubt: X1, U, X2." << endl;
        return;
    }

    if(targetWire < 0)
    {
        cerr << "FEHLER: Ungueltige Drahtnummer "
             << targetWire << endl;
        return;
    }

    cout << "\n============================================\n";
    cout << " Einzel-Draht-Time-Walk-Kalibrierung\n";
    cout << " Ebene: " << layer << "\n";
    cout << " Draht: " << targetWire << "\n";
    cout << " Runs:  " << gRunFirst << " - " << gRunLast << "\n";
    cout << "============================================\n\n";


    // --------------------------------------------------------
    // 1. Eingabedateien laden
    // --------------------------------------------------------

    TChain chain(gTreeName);

    int nFiles = 0;

    for(int run = gRunFirst; run <= gRunLast; ++run)
    {
        TString pattern = Form(
            "%s/%s%03d_*.root",
            gInputDirectory,
            gFilePrefix,
            run
        );

        int added = chain.Add(pattern);

        if(added > 0)
        {
            cout << "Run " << run
                 << ": " << added
                 << " Datei(en) hinzugefuegt." << endl;

            nFiles += added;
        }
    }

    if(nFiles == 0 || chain.GetEntries() == 0)
    {
        cerr << "FEHLER: Keine passenden Eingabedateien "
             << "oder keine Eintraege gefunden!" << endl;
        return;
    }

    cout << "\nDateien insgesamt: " << nFiles << endl;
    cout << "Tree-Eintraege:    " << chain.GetEntries() << endl;


    // Branches
    vector<double>* leading     = nullptr;
    vector<double>* trailing    = nullptr;
    vector<double>* tdcTrigTime = nullptr;

    vector<int>* layerID = nullptr;
    vector<int>* wireNum = nullptr;

    if(chain.SetBranchAddress("leading", &leading) < 0 ||
       chain.SetBranchAddress("trailing", &trailing) < 0 ||
       chain.SetBranchAddress("tdcTrigTime", &tdcTrigTime) < 0 ||
       chain.SetBranchAddress("layerID", &layerID) < 0 ||
       chain.SetBranchAddress("wireNum", &wireNum) < 0)
    {
        cerr << "FEHLER: Mindestens ein Branch konnte nicht "
             << "verbunden werden." << endl;

        cerr << "Bitte pruefe die Branchnamen und Datentypen "
             << "im Tree " << gTreeName << "." << endl;

        return;
    }


    // Histogramm fuer genau einen Draht
    TH2D* h2 = new TH2D(
        Form("h2_%s_wire%d", layer.c_str(), targetWire),
        Form("%s, wire %d;ToT [ns];t_{raw} [ns]",
             layer.c_str(), targetWire),
        gNBinsToT,
        gToTaxisMin,
        gToTaxisMax,
        gNBinsT,
        gTaxisMin,
        gTaxisMax
    );

    h2->SetDirectory(nullptr);


    // Eingabedaten lesen und Ziel-Draht selektieren
    Long64_t nEntries = chain.GetEntries();

    Long64_t nTargetHits = 0;
    Long64_t nAccepted   = 0;

    for(Long64_t entry = 0; entry < nEntries; ++entry)
    {
        chain.GetEntry(entry);

        if(!leading || !trailing || !tdcTrigTime ||
           !layerID || !wireNum)
        {
            continue;
        }

        size_t n = leading->size();

        n = min(n, trailing->size());
        n = min(n, tdcTrigTime->size());
        n = min(n, layerID->size());
        n = min(n, wireNum->size());

        for(size_t j = 0; j < n; ++j)
        {
            if(layerID->at(j) != targetLayer ||
               wireNum->at(j) != targetWire)
            {
                continue;
            }

            ++nTargetHits;

            const double lead = leading->at(j);
            const double trail = trailing->at(j);
            const double trig = tdcTrigTime->at(j);

            const double tot = fabs(lead - trail);

            if(!std::isfinite(tot) ||
               !std::isfinite(lead) ||
               !std::isfinite(trail) ||
               !std::isfinite(trig))
            {
                continue;
            }

            if(tot < gToTmin || tot > gToTmax)
            {
                continue;
            }

            const double rawT = -(lead - trig) + gOffsetTime;

            if(!std::isfinite(rawT))
            {
                continue;
            }

            if(rawT < gDriftMin || rawT > gDriftMax)
            {
                continue;
            }

            h2->Fill(tot, rawT);
            ++nAccepted;
        }
    }

    cout << "\nTreffer fuer Ziel-Draht: " << nTargetHits << endl;
    cout << "Treffer nach Selektion:  " << nAccepted << endl;
    cout << "Histogramm-Eintraege:    " << h2->GetEntries() << endl;

    if(nAccepted == 0)
    {
        cerr << "FEHLER: Keine akzeptierten Treffer fuer "
             << layer << ", Draht " << targetWire << "." << endl;

        delete h2;
        return;
    }


    // Peakpositionen fuer einzelne ToT-Bins bestimmen
    vector<double> xvals;
    vector<double> yvals;
    vector<double> yerrs;

    double previousPeak = -1.0;

    for(int binX = 1; binX <= h2->GetNbinsX(); ++binX)
    {
        const double totCenter = h2->GetXaxis()->GetBinCenter(binX);

        if(totCenter < gFitToTmin ||
           totCenter > gFitToTmax)
        {
            continue;
        }

        TH1D* proj = h2->ProjectionY(
            Form("proj_%s_wire%d_bin%d",
                 layer.c_str(), targetWire, binX),
            binX,
            binX
        );

        if(!proj)
        {
            continue;
        }

        if(proj->GetEntries() < 1)
        {
            delete proj;
            continue;
        }

        // Erstes ToT-Bin: globale Suche.
        // Danach: Peak in der Umgebung des vorherigen Peaks.
        double searchMin = gPeakSearchMin;
        double searchMax = gPeakSearchMax;

        if(previousPeak >= 0.0)
        {
            searchMin = previousPeak - gPeakFollowWindow;
            searchMax = previousPeak + gPeakFollowWindow;

            searchMin = max(searchMin, gTaxisMin);
            searchMax = min(searchMax, gTaxisMax);
        }

        const int firstBin = proj->GetXaxis()->FindBin(searchMin);
        const int lastBin  = proj->GetXaxis()->FindBin(searchMax);

        if(firstBin > lastBin)
        {
            delete proj;
            continue;
        }

        const int maxBin = firstBin +
            proj->GetMaximumBin() - 1;

        // Maximum explizit innerhalb des Suchbereichs suchen.
        int peakBin = firstBin;
        double maxContent = -1.0;

        for(int b = firstBin; b <= lastBin; ++b)
        {
            const double content = proj->GetBinContent(b);

            if(content > maxContent)
            {
                maxContent = content;
                peakBin = b;
            }
        }

        if(maxContent <= 0.0)
        {
            delete proj;
            continue;
        }

        const double peak0 = proj->GetXaxis()->GetBinCenter(peakBin);

        // Lokaler Gauss-Fit um den gefundenen Peak.
        const double fitMin = peak0 - gLocalFitWindow;
        const double fitMax = peak0 + gLocalFitWindow;

        TF1 gaus(
            Form("gaus_%s_wire%d_bin%d",
                 layer.c_str(), targetWire, binX),
            "gaus",
            fitMin,
            fitMax
        );

        gaus.SetParameters(
            maxContent,
            peak0,
            5.0
        );

        gaus.SetParLimits(1, peak0 - gLocalFitWindow,
                             peak0 + gLocalFitWindow);

        gaus.SetParLimits(2, 2.0, 40.0);

        const double localIntegral =
            proj->Integral(
                proj->GetXaxis()->FindBin(fitMin),
                proj->GetXaxis()->FindBin(fitMax)
            );

        if(localIntegral < gMinLocalIntegral)
        {
            delete proj;
            continue;
        }

        const int fitStatus = proj->Fit(&gaus, "QRS0");

        if(fitStatus != 0)
        {
            delete proj;
            continue;
        }

        const double peak = gaus.GetParameter(1);
        const double peakErr = gaus.GetParError(1);

        if(!std::isfinite(peak) ||
           !std::isfinite(peakErr) ||
           peakErr > gMaxPeakError)
        {
            delete proj;
            continue;
        }

        xvals.push_back(totCenter);
        yvals.push_back(peak);
        yerrs.push_back(peakErr);

        previousPeak = peak;

        delete proj;
    }

    cout << "\nErfolgreich bestimmte Peakpunkte: "
         << xvals.size() << endl;

    if(xvals.size() < 5)
    {
        cerr << "FEHLER: Zu wenige Peakpunkte fuer einen "
             << "stabilen exponentiellen Fit." << endl;

        delete h2;
        return;
    }


    // Vorfit der Time-Walk-Funktion
    TGraphErrors* graph = new TGraphErrors(
        static_cast<int>(xvals.size()),
        xvals.data(),
        yvals.data(),
        nullptr,
        yerrs.data()
    );

    graph->SetName(
        Form("TimeWalkGraph_%s_wire%d",
             layer.c_str(), targetWire)
    );

    graph->SetTitle(
        Form("%s, wire %d;ToT [ns];Peak position [ns]",
             layer.c_str(), targetWire)
    );

    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(0.8);

    TF1* fPre = new TF1(
        Form("fPre_%s_wire%d", layer.c_str(), targetWire),
        TimeWalkFunction,
        gFitToTmin,
        gFitToTmax,
        3
    );

    fPre->SetParameters(10.0, 0.05, 40.0);

    fPre->SetParLimits(0, 0.0, 100.0);
    fPre->SetParLimits(1, 0.0001, 1.0);
    fPre->SetParLimits(2, 0.0, 100.0);

    TFitResultPtr preResult = graph->Fit(fPre, "QRS0");

    if(static_cast<int>(preResult) != 0)
    {
        cerr << "WARNUNG: Vorfit meldet Status "
             << static_cast<int>(preResult) << endl;
    }


    // Ausreisser entfernen
    vector<double> xClean;
    vector<double> yClean;
    vector<double> yErrClean;

    cout << "\nAusreisserpruefung:" << endl;

    for(size_t i = 0; i < xvals.size(); ++i)
    {
        const double prediction = fPre->Eval(xvals[i]);
        const double residual = yvals[i] - prediction;

        if(std::fabs(residual) < gOutlierCut)
        {
            xClean.push_back(xvals[i]);
            yClean.push_back(yvals[i]);
            yErrClean.push_back(yerrs[i]);
        }
        else
        {
            cout << "  Verworfen: ToT=" << xvals[i]
                 << " ns, Peak=" << yvals[i]
                 << " ns, Residuum=" << residual
                 << " ns" << endl;
        }
    }

    cout << "Peakpunkte vor Outlier-Cut: " << xvals.size() << endl;
    cout << "Peakpunkte nach Outlier-Cut: " << xClean.size() << endl;

    if(xClean.size() < 5)
    {
        cerr << "FEHLER: Nach dem Outlier-Cut bleiben weniger "
             << "als 5 Punkte uebrig." << endl;

        delete graph;
        delete fPre;
        delete h2;
        return;
    }


    // TimeWalk-Fit
    TGraphErrors* graphClean = new TGraphErrors(
        static_cast<int>(xClean.size()),
        xClean.data(),
        yClean.data(),
        nullptr,
        yErrClean.data()
    );

    graphClean->SetName(
        Form("TimeWalkGraphClean_%s_wire%d",
             layer.c_str(), targetWire)
    );

    graphClean->SetTitle(
        Form("%s, wire %d;ToT [ns];Peak position [ns]",
             layer.c_str(), targetWire)
    );

    graphClean->SetMarkerStyle(20);
    graphClean->SetMarkerSize(0.8);

    TF1* fFinal = new TF1(
        Form("fTimeWalk_%s_wire%d",
             layer.c_str(), targetWire),
        TimeWalkFunction,
        gFitToTmin,
        gFitToTmax,
        3
    );

    // Startwerte aus dem Vorfit.
    fFinal->SetParameters(
        fPre->GetParameter(0),
        fPre->GetParameter(1),
        fPre->GetParameter(2)
    );

    fFinal->SetParLimits(0, 0.0, 100.0);
    fFinal->SetParLimits(1, 0.0001, 1.0);
    fFinal->SetParLimits(2, 0.0, 100.0);

    TFitResultPtr finalResult = graphClean->Fit(fFinal, "QRS0");

    if(static_cast<int>(finalResult) != 0)
    {
        cerr << "WARNUNG: Finaler Fit meldet Status "
             << static_cast<int>(finalResult) << endl;
    }


    // Fitparameter ausgeben
    const double a = fFinal->GetParameter(0);
    const double b = fFinal->GetParameter(1);
    const double c = fFinal->GetParameter(2);

    const double ea = fFinal->GetParError(0);
    const double eb = fFinal->GetParError(1);
    const double ec = fFinal->GetParError(2);

    cout << "\n============================================\n";
    cout << "Ergebnis: " << layer << ", Draht " << targetWire << "\n";
    cout << "============================================\n";

    cout << "f(ToT) = a * exp(-b * ToT) + c\n\n";

    cout << "a = " << a << " +- " << ea << " ns\n";
    cout << "b = " << b << " +- " << eb << " ns^-1\n";
    cout << "c = " << c << " +- " << ec << " ns\n";

    cout << "Chi2/NDF = " << fFinal->GetChisquare()
         << " / " << fFinal->GetNDF() << endl;


    // Einzeldatei mit den Fitparametern
    const TString txtName = Form(
        "time_walk_corrections_%s_wire%d.txt",
        layer.c_str(),
        targetWire
    );

    ofstream txt(txtName.Data());

    if(!txt)
    {
        cerr << "FEHLER: Textdatei konnte nicht geschrieben werden: "
             << txtName << endl;
    }
    else
    {
        txt << "# layer wire a b c\n";
        txt << layer << " "
            << targetWire << " "
            << a << " "
            << b << " "
            << c << "\n";

        txt.close();

        cout << "\nFitparameter gespeichert in: "
             << txtName << endl;
    }


    // ROOT-Datei mit Histogramm, Graph und Fit
    const TString rootName = Form(
        "timewalk_calib_%s_wire%d.root",
        layer.c_str(),
        targetWire
    );

    TFile* outFile = TFile::Open(rootName.Data(), "RECREATE");

    if(!outFile || outFile->IsZombie())
    {
        cerr << "FEHLER: ROOT-Ausgabedatei konnte nicht "
             << "geoeffnet werden." << endl;

        if(outFile)
        {
            delete outFile;
        }

        delete graph;
        delete graphClean;
        delete fPre;
        delete fFinal;
        delete h2;
        return;
    }

    outFile->cd();

    h2->Write();
    graph->Write();
    graphClean->Write();
    fPre->Write();
    fFinal->Write();

    outFile->Close();
    delete outFile;


    // Grafische Ausgabe als PDF
    const TString pdfName = Form(
        "timewalk_calib_%s_wire%d.pdf",
        layer.c_str(),
        targetWire
    );

    TCanvas* c1 = new TCanvas(
        Form("c1_%s_wire%d", layer.c_str(), targetWire),
        "Time-Walk calibration",
        1200,
        900
    );

    c1->Divide(1, 2);

    c1->cd(1);
    gPad->SetRightMargin(0.14);
    h2->Draw("COLZ");

    c1->cd(2);
    gPad->SetGrid();

    graphClean->Draw("AP");
    fFinal->SetLineColor(kRed + 1);
    fFinal->Draw("SAME");

    TLegend* legend = new TLegend(0.12, 0.75, 0.48, 0.88);

    legend->AddEntry(graphClean, "Peak positions", "p");
    legend->AddEntry(fFinal, "Exponential fit", "l");
    legend->Draw();

    c1->SaveAs(pdfName);

    cout << "ROOT-Datei: " << rootName << endl;
    cout << "PDF-Datei:  " << pdfName << endl;

    cout << "\nEinzelkalibrierung abgeschlossen.\n";

    delete legend;
    delete c1;

    delete graph;
    delete graphClean;
    delete fPre;
    delete fFinal;
    delete h2;
}
