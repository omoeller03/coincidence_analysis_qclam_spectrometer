// timewalkcorr_all_improved.C
//
// Verbesserte Time-Walk-Kalibration
//
// Features
// ---------------------------------------------------------
// - invertierte Driftzeiten
// - robuste Peak-Suche
// - Peak-Following
// - iterativer Gaußfit pro Slice
// - Slice-Qualitätskontrolle
// - Outlier-Rejection
// - quadratischer Timewalkfit
// - konfigurierbare Fenster
// - PDF-Ausgabe
//
// Aufruf:
// root -l 'timewalkcorr_all_improved.C()'
//


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

#include <map>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <cmath>


/* ============================================================
   Konfiguration
   ============================================================ */

// ------------------------------------------------------------
// Input / Output
// ------------------------------------------------------------

std::string gInputFile =
"/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run591_to_601/TQClamDrifttimeCalibStore.root";

std::string gTreeName =
"DrifttimeCalib";

std::string gOutputRootFile =
"/d/d04-1/ag_pi/spektrometer/QCLAM/tracking-data/calibrations/drifttime/run591_to_601/timewalk_corrected_improved.root";

std::string gOutputTreeName =
"DrifttimeCalib_TWcorr";


// ------------------------------------------------------------
// leading/trailing tauschen?
// ------------------------------------------------------------

bool gSwapLeadingTrailing = false;


// ------------------------------------------------------------
// ToT Gates
// ------------------------------------------------------------

double gToTmin = 20.0;
double gToTmax = 80.0;


// ------------------------------------------------------------
// Driftzeitbereich
// ------------------------------------------------------------

double gDriftMin = -1400.0;
double gDriftMax = 100.0;


// ------------------------------------------------------------
// Histogramm-Binning
// ------------------------------------------------------------

int    gNBinsToT   = 80;
double gToTaxisMin = 0.0;
double gToTaxisMax = 80.0;

int    gNBinsT     = 300;
double gTaxisMin   = -1400.0;
double gTaxisMax   = 100.0;


// ------------------------------------------------------------
// Slice Statistik
// ------------------------------------------------------------

int gMinEntriesPerSlice = 20;


// ------------------------------------------------------------
// ToT-Fitbereich
// ------------------------------------------------------------

double gFitToTmin = 20.0;
double gFitToTmax = 70.0;


// ------------------------------------------------------------
// Peak-Suche
// ------------------------------------------------------------

double gPeakSearchMin = -1200.0;
double gPeakSearchMax = -700.0;

double gPeakFollowWindow = 60.0;


// ------------------------------------------------------------
// Lokaler Gaußfit
// ------------------------------------------------------------

double gLocalFitWindow = 25.0;


// ------------------------------------------------------------
// Slice-Fit Qualität
// ------------------------------------------------------------

double gMaxSliceChi2NDF = 20.0;


// ------------------------------------------------------------
// Outlier-Rejection
// ------------------------------------------------------------

double gOutlierCut = 25.0;


// ------------------------------------------------------------
// minimale Punktzahl für finalen Fit
// ------------------------------------------------------------

int gMinFitPoints = 12;


// ------------------------------------------------------------
// Referenzzeit
// ------------------------------------------------------------

double gTref = -900.0;


/* ============================================================
   Timewalk Fit
   ============================================================ */

TF1* fittimewalk(TH2D* h2)
{
    if(!h2)
        return nullptr;

    TAxis* xax = h2->GetXaxis();

    std::vector<double> xvals;
    std::vector<double> yvals;
    std::vector<double> yerrs;

    double lastPeak = -9999;

    // ========================================================
    // Loop über ToT-Slices
    // ========================================================

    for(int ix=1; ix<=xax->GetNbins(); ix++)
    {
        double tot =
            xax->GetBinCenter(ix);

        if(tot < gFitToTmin ||
           tot > gFitToTmax)
            continue;

        TH1D* py =
            h2->ProjectionY(
                Form("py_%s_%d",
                     h2->GetName(),
                     ix),
                ix,
                ix
            );

        if(py->GetEntries() <
           gMinEntriesPerSlice)
        {
            delete py;
            continue;
        }

        // ====================================================
        // Peak-Suchfenster
        // ====================================================

        double searchMin;
        double searchMax;

        if(lastPeak < -9000)
        {
            searchMin = gPeakSearchMin;
            searchMax = gPeakSearchMax;
        }
        else
        {
            searchMin =
                lastPeak - gPeakFollowWindow;

            searchMax =
                lastPeak + gPeakFollowWindow;
        }

        int binMin =
            py->FindBin(searchMin);

        int binMax =
            py->FindBin(searchMax);

        if(binMin >= binMax)
        {
            delete py;
            continue;
        }

        // ====================================================
        // lokales Maximum suchen
        // ====================================================

        int maxbin = -1;
        double maxcontent = -1;

        for(int b=binMin; b<=binMax; b++)
        {
            double c =
                py->GetBinContent(b);

            if(c > maxcontent)
            {
                maxcontent = c;
                maxbin = b;
            }
        }

        if(maxbin < 0)
        {
            delete py;
            continue;
        }

        double peak0 =
            py->GetBinCenter(maxbin);

        // ====================================================
        // erster Gaußfit
        // ====================================================

        double fitMin =
            peak0 - gLocalFitWindow;

        double fitMax =
            peak0 + gLocalFitWindow;

        if(py->Integral(
            py->FindBin(fitMin),
            py->FindBin(fitMax)) < 5)
        {
            delete py;
            continue;
        }

        TF1 gaus1(
            "g1",
            "gaus",
            fitMin,
            fitMax
        );

        gaus1.SetParameters(
            py->GetMaximum(),
            peak0,
            10
        );

        gaus1.SetParLimits(
            1,
            peak0-gLocalFitWindow,
            peak0+gLocalFitWindow
        );

        gaus1.SetParLimits(2,2,40);

        int status1 =
            py->Fit(&gaus1,"QNR");

        if(status1 != 0)
        {
            delete py;
            continue;
        }

        double mean1 =
            gaus1.GetParameter(1);

        double sigma1 =
            gaus1.GetParameter(2);

        if(sigma1 < 2 ||
           sigma1 > 40)
        {
            delete py;
            continue;
        }

        // ====================================================
        // zweiter Gaußfit
        // ====================================================

        double fitMin2 =
            mean1 - 1.5*sigma1;

        double fitMax2 =
            mean1 + 1.5*sigma1;

        TF1 gaus2(
            "g2",
            "gaus",
            fitMin2,
            fitMax2
        );

        gaus2.SetParameters(
            gaus1.GetParameter(0),
            mean1,
            sigma1
        );

        gaus2.SetParLimits(
            1,
            mean1-10,
            mean1+10
        );

        gaus2.SetParLimits(2,2,40);

        int status2 =
            py->Fit(&gaus2,"QNR");

        if(status2 != 0)
        {
            delete py;
            continue;
        }

        double peak =
            gaus2.GetParameter(1);

        double peakErr =
            gaus2.GetParError(1);

        // realistischer Fehler
        peakErr =
            std::max(peakErr,4.0);

        // ====================================================
        // Slice-Qualität prüfen
        // ====================================================

        double chi2ndf =
            gaus2.GetChisquare() /
            gaus2.GetNDF();

        if(chi2ndf >
           gMaxSliceChi2NDF)
        {
            delete py;
            continue;
        }

        // ====================================================
        // speichern
        // ====================================================

        xvals.push_back(tot);
        yvals.push_back(peak);
        yerrs.push_back(peakErr);

        lastPeak = peak;

        delete py;
    }

    // ========================================================
    // genug Punkte?
    // ========================================================

    if((int)xvals.size() <
       gMinFitPoints)
    {
        std::cout
            << "Zu wenig Punkte fuer "
            << h2->GetName()
            << std::endl;

        return nullptr;
    }

    // ========================================================
    // Vorfit
    // ========================================================

    TGraphErrors gr0(
        xvals.size(),
        &xvals[0],
        &yvals[0],
        nullptr,
        &yerrs[0]
    );

    TF1 prefit(
        "prefit",
        "[0]+[1]*x+[2]*x*x",
        gFitToTmin,
        gFitToTmax
    );

    prefit.SetParameters(
        -900,
        5,
        -0.05
    );

    gr0.Fit(&prefit,"Q");

    // ========================================================
    // Outlier-Rejection
    // ========================================================

    std::vector<double> x2;
    std::vector<double> y2;
    std::vector<double> e2;

    for(size_t i=0; i<xvals.size(); i++)
    {
        double yfit =
            prefit.Eval(xvals[i]);

        double resid =
            std::fabs(
                yvals[i]-yfit
            );

        if(resid < gOutlierCut)
        {
            x2.push_back(xvals[i]);
            y2.push_back(yvals[i]);
            e2.push_back(yerrs[i]);
        }
    }

    if((int)x2.size() <
       gMinFitPoints)
    {
        std::cout
            << "Zu viele Outlier fuer "
            << h2->GetName()
            << std::endl;

        return nullptr;
    }

    // ========================================================
    // Finaler Fit
    // ========================================================

    TGraphErrors* gr =
        new TGraphErrors(
            x2.size(),
            &x2[0],
            &y2[0],
            nullptr,
            &e2[0]
        );

    TF1* f =
        new TF1(
            Form("f_tw_%s",
                 h2->GetName()),
            "[0]+[1]*x+[2]*x*x",
            gFitToTmin,
            gFitToTmax
        );

    f->SetParameters(
        prefit.GetParameter(0),
        prefit.GetParameter(1),
        prefit.GetParameter(2)
    );

    gr->Fit(f,"QR");

    double finalChi2 =
        f->GetChisquare() /
        f->GetNDF();

    std::cout
        << h2->GetName()
        << "  Punkte="
        << x2.size()
        << "  chi2/ndf="
        << finalChi2
        << std::endl;

    // schlechter finaler Fit?
    if(finalChi2 > 100)
    {
        delete gr;
        delete f;
        return nullptr;
    }

    delete gr;

    return f;
}


/* ============================================================
   Hauptfunktion
   ============================================================ */

void timewalkcorr_all_improved()
{
    // ========================================================
    // Input öffnen
    // ========================================================

    TFile* fin =
        TFile::Open(
            gInputFile.c_str(),
            "READ"
        );

    if(!fin || fin->IsZombie())
    {
        std::cout
            << "Inputfile konnte "
            << "nicht geöffnet werden!"
            << std::endl;

        return;
    }

    TTree* t =
        (TTree*)fin->Get(
            gTreeName.c_str()
        );

    if(!t)
    {
        std::cout
            << "Tree nicht gefunden!"
            << std::endl;

        return;
    }

    // ========================================================
    // Reader
    // ========================================================

    TTreeReader reader(t);

    TTreeReaderArray<double>* leading =
        nullptr;

    TTreeReaderArray<double>* trailing =
        nullptr;

    if(gSwapLeadingTrailing)
    {
        leading =
            new TTreeReaderArray<double>(
                reader,
                "trailing"
            );

        trailing =
            new TTreeReaderArray<double>(
                reader,
                "leading"
            );
    }
    else
    {
        leading =
            new TTreeReaderArray<double>(
                reader,
                "leading"
            );

        trailing =
            new TTreeReaderArray<double>(
                reader,
                "trailing"
            );
    }

    TTreeReaderArray<double> tTrig(
        reader,
        "tdcTrigTime"
    );

    TTreeReaderArray<int> layer(
        reader,
        "layerID"
    );

    TTreeReaderArray<int> wire(
        reader,
        "wireNum"
    );

    // ========================================================
    // Histogramme
    // ========================================================

    std::map<std::pair<int,int>,TH2D*> h2_map;
    std::map<std::pair<int,int>,TH2D*> h2_corr;

    std::cout
        << "Erste Schleife..."
        << std::endl;

    while(reader.Next())
    {
        size_t nhits =
            layer.GetSize();

        for(size_t ih=0; ih<nhits; ih++)
        {
            int L = layer[ih];
            int W = wire[ih];

            auto key =
                std::make_pair(L,W);

            double lead =
                (*leading)[ih];

            double trail =
                (*trailing)[ih];

            double trig =
                tTrig[ih];

            double tot =
                std::fabs(
                    lead-trail
                );

            if(tot < gToTmin ||
               tot > gToTmax)
                continue;

            // invertierte Driftzeit
            double rawT =
                -(lead-trig);

            if(rawT < gDriftMin ||
               rawT > gDriftMax)
                continue;

            if(!h2_map[key])
            {
                h2_map[key] =
                    new TH2D(
                        Form(
                            "h2_L%d_W%d",
                            L,W
                        ),
                        Form(
                            "ToT vs Driftzeit "
                            "L%d W%d;"
                            "ToT/ns;"
                            "Driftzeit/ns",
                            L,W
                        ),
                        gNBinsToT,
                        gToTaxisMin,
                        gToTaxisMax,
                        gNBinsT,
                        gTaxisMin,
                        gTaxisMax
                    );
            }

            h2_map[key]->Fill(
                tot,
                rawT
            );
        }
    }

    std::cout
        << "Histogramme: "
        << h2_map.size()
        << std::endl;

    // ========================================================
    // Fits
    // ========================================================

    std::map<std::pair<int,int>,TF1*> twFit;

    for(auto& kv : h2_map)
    {
        TH2D* h2 =
            kv.second;

        if(h2->GetEntries() < 100)
        {
            twFit[kv.first] = nullptr;
            continue;
        }

        twFit[kv.first] =
            fittimewalk(h2);
    }

    // ========================================================
    // Output
    // ========================================================

    TFile* fout =
        new TFile(
            gOutputRootFile.c_str(),
            "RECREATE"
        );

    fout->mkdir("timewalk_hists");
    fout->cd("timewalk_hists");

    for(auto& kv : h2_map)
    {
        kv.second->Write();

        if(twFit[kv.first])
            twFit[kv.first]->Write();
    }

    fout->cd();

    // ========================================================
    // Neuer Tree
    // ========================================================

    TTree* tout =
        new TTree(
            gOutputTreeName.c_str(),
            "Timewalk corrected"
        );

    std::vector<int> out_layer;
    std::vector<int> out_wire;

    std::vector<double> out_tot;
    std::vector<double> out_rawT;
    std::vector<double> out_corrT;

    tout->Branch(
        "layerID",
        &out_layer
    );

    tout->Branch(
        "wireNum",
        &out_wire
    );

    tout->Branch(
        "tot",
        &out_tot
    );

    tout->Branch(
        "t_raw",
        &out_rawT
    );

    tout->Branch(
        "t_tw_corr",
        &out_corrT
    );

    // ========================================================
    // zweite Schleife
    // ========================================================

    reader.Restart();

    std::cout
        << "Zweite Schleife..."
        << std::endl;

    while(reader.Next())
    {
        out_layer.clear();
        out_wire.clear();

        out_tot.clear();
        out_rawT.clear();
        out_corrT.clear();

        size_t nhits =
            layer.GetSize();

        for(size_t ih=0; ih<nhits; ih++)
        {
            int L = layer[ih];
            int W = wire[ih];

            auto key =
                std::make_pair(L,W);

            double lead =
                (*leading)[ih];

            double trail =
                (*trailing)[ih];

            double trig =
                tTrig[ih];

            double tot =
                std::fabs(
                    lead-trail
                );

            double rawT =
                -(lead-trig);

            double corrT =
                rawT;

            TF1* f =
                twFit[key];

            if(f)
            {
                double tw =
                    f->Eval(tot);

                corrT =
                    rawT -
                    (tw-gTref);

                if(!h2_corr[key])
                {
                    h2_corr[key] =
                        new TH2D(
                            Form(
                                "h2corr_L%d_W%d",
                                L,W
                            ),
                            Form(
                                "TW corrected "
                                "L%d W%d;"
                                "ToT/ns;"
                                "Driftzeit/ns",
                                L,W
                            ),
                            gNBinsToT,
                            gToTaxisMin,
                            gToTaxisMax,
                            gNBinsT,
                            gTaxisMin,
                            gTaxisMax
                        );
                }

                h2_corr[key]->Fill(
                    tot,
                    corrT
                );
            }

            out_layer.push_back(L);
            out_wire.push_back(W);

            out_tot.push_back(tot);
            out_rawT.push_back(rawT);
            out_corrT.push_back(corrT);
        }

        tout->Fill();
    }

    // ========================================================
    // PDF
    // ========================================================

    TCanvas* c =
        new TCanvas(
            "c",
            "Timewalk",
            1000,
            800
        );

    c->Print(
        "timewalk_plots_improved.pdf["
    );

    for(auto& kv : h2_map)
    {
        TH2D* h2 =
            kv.second;

        TF1* f =
            twFit[kv.first];

        c->cd();

        h2->Draw("COLZ");

        if(f)
            f->Draw("same");

        c->Print(
            "timewalk_plots_improved.pdf"
        );

        if(h2_corr[kv.first])
        {
            h2_corr[kv.first]->Draw("COLZ");

            c->Print(
                "timewalk_plots_improved.pdf"
            );
        }
    }

    c->Print(
        "timewalk_plots_improved.pdf]"
    );

    // ========================================================
    // speichern
    // ========================================================

    fout->cd();

    tout->Write();

    for(auto& kv : h2_corr)
    {
        kv.second->Write();
    }

    fout->Close();

    std::cout
        << "Fertig."
        << std::endl;
}
