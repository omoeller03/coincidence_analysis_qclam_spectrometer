
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include <TCanvas.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TAxis.h>
#include <TStyle.h>
#include <TMultiGraph.h>

void plot_driftlength_relation()
{
    const char* filename = "driftlength_relation_X1.txt";

    // Gewünschte Winkel in 10-Grad-Schritten
    const std::vector<double> angles = {
        25., 35., 45., 55., 65.
    };

    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Fehler: Datei konnte nicht geöffnet werden: "
                  << filename << std::endl;
        return;
    }

    std::string line;

    // Header einlesen
    std::getline(file, line);

    std::istringstream header(line);

    std::string dummy;
    header >> dummy; // t(ns)

    std::vector<double> headerAngles;

    double angle;
    while (header >> angle) {
        headerAngles.push_back(angle);
    }

    // Daten einlesen
    std::vector<double> times;
    std::vector<std::vector<double>> values(headerAngles.size());

    while (std::getline(file, line)) {

        if (line.empty())
            continue;

        std::istringstream iss(line);

        double time;
        if (!(iss >> time))
            continue;

        times.push_back(time - 35);

        for (size_t i = 0; i < headerAngles.size(); ++i) {

            double value;

            if (!(iss >> value)) {
                std::cerr << "Fehler beim Einlesen der Zeile!"
                          << std::endl;
                return;
            }

            values[i].push_back(value);
        }
    }

    times.push_back(250.);
    for(size_t i=0; i<headerAngles.size(); ++i)
    {
	    values[i].push_back(12.);
    }

    file.close();

    // ROOT-Style
    gStyle->SetOptStat(0);
    gStyle->SetTitle(0);

    TCanvas* canvas = new TCanvas(
        "c_driftlength",
        "Driftlength relation",
        1000, 700
    );

    TMultiGraph* mg = new TMultiGraph();

    TLegend* legend = new TLegend(
        0.20, 0.60, 0.55, 0.87
    );

    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.035);

    const int colors[] = {
        kBlue + 1,
        kRed + 1,
        kGreen + 2,
        kMagenta + 1,
        kOrange + 7
    };

    // Kurven erzeugen
    for (size_t a = 0; a < angles.size(); ++a) {

        // Spalte des gewünschten Winkels finden
        int column = -1;

        for (size_t i = 0; i < headerAngles.size(); ++i) {
            if (std::abs(headerAngles[i] - angles[a]) < 1e-6) {
                column = i;
                break;
            }
        }

        if (column < 0) {
            std::cerr << "Winkel " << angles[a]
                      << " nicht in Tabelle gefunden!"
                      << std::endl;
            continue;
        }

        TGraph* graph = new TGraph(
            times.size(),
            times.data(),
            values[column].data()
        );

        graph->SetLineColor(colors[a]);
        graph->SetLineWidth(3);

        mg->Add(graph, "L");

        legend->AddEntry(
            graph,
            Form("%.0f^{#circ}", angles[a]),
            "l"
        );
    }

    canvas->SetLeftMargin(0.13);
    canvas->SetRightMargin(0.05);
    canvas->SetBottomMargin(0.12);

    mg->Draw("A");

    mg->GetXaxis()->SetTitle("Driftzeit [ns]");
    mg->GetXaxis()->CenterTitle();
    mg->GetYaxis()->SetTitle("Spurversatz [mm]");
    mg->GetYaxis()->CenterTitle();

    mg->GetXaxis()->SetTitleSize(0.045);
    mg->GetYaxis()->SetTitleSize(0.045);

    mg->GetXaxis()->SetLabelSize(0.04);
    mg->GetYaxis()->SetLabelSize(0.04);

    mg->GetXaxis()->SetTitleOffset(1.15);
    mg->GetYaxis()->SetTitleOffset(1.35);

    mg->GetXaxis()->SetLimits(0., 250.);
    mg->SetMinimum(0.);
    mg->SetMaximum(16.);

    legend->Draw();

    canvas->Update();

    canvas->SaveAs("driftlength_relation_X1.pdf");
}

