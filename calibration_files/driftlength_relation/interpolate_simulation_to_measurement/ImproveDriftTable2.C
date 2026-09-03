#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TLine.h>
#include <TAxis.h>
#include <TSystem.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>

using namespace std;


// ============================================================
// KONFIGURATION
// ============================================================

const char* INPUT_MEASUREMENT =
    "driftlength_relation_U.txt";

const char* INPUT_SIMULATION =
    "/d/d04-1/ag_pi/spektrometer/QCLAM/Driftkammern/3Gen/GarfieldSimulationen/2D/DriftwegDriftzeitKorr/Tabellen/korrektur-daten/Driftkorrekturen_DriftkammerGen_3_Ar_80.000000_CO2_20.000000_O2_0.000000_HV_-4300.000000_AnzahlSimDraehte_10_AnzahlSimulierterWinkel_451_AnzahlSimulierterDatenProWinkel_91441.root";

const char* OUTPUT_FILE =
    "driftlength_relation_U_improved.txt";


// ============================================================
// PLOT-VERZEICHNIS
// ============================================================

const char* PLOT_DIR =
    "plots";


// ============================================================
// PHI-MATCHING
// ============================================================

const double PHI_TOLERANCE = 0.001;


// ============================================================
// Y-ACHSE
// ============================================================

const double Y_MIN = 0.0;
const double Y_MAX = 16.0;


// ============================================================
// MESSBEGINN
// ============================================================

const double MEASUREMENT_START_MIN = 20.0;
const double MEASUREMENT_START_MAX = 60.0;


// ============================================================
// ZEITSHIFT
// ============================================================

const double SHIFT_MIN  = 10.0;
const double SHIFT_MAX  = 50.0;
const double SHIFT_STEP = 0.1;


// ============================================================
// BEREICH FÜR ZEITSHIFT
// ============================================================

const double SHIFT_FIT_START = 30.0;
const double SHIFT_FIT_END   = 70.0;


// ============================================================
// WINKELABHÄNGIGER ÜBERGANG
// ============================================================

const double PHI_TRANSITION_LOW  = 25.0;
const double PHI_TRANSITION_HIGH = 65.0;

const double TRANSITION_START_LOW  = 75.0;
const double TRANSITION_START_HIGH = 55.0;

const double TRANSITION_WIDTH = 10.0;


// ============================================================
// DATENSTRUKTUREN
// ============================================================

struct Measurement
{
    vector<double> times;

    // lengths[iphi][itime]
    vector<vector<double>> lengths;

    vector<double> phis;
};


struct SimCurve
{
    double phi = 0.0;

    vector<double> time;
    vector<double> length;
};


// ============================================================
// MESSDATEI EINLESEN
// ============================================================

bool ReadMeasurement(
    const char* filename,
    Measurement& measurement)
{
    ifstream in(filename);

    if(!in)
    {
        cerr << "Fehler: Messdatei konnte nicht geoeffnet werden: "
             << filename << endl;

        return false;
    }

    string line;

    // --------------------------------------------------------
    // Header
    // --------------------------------------------------------

    if(!getline(in,line))
    {
        cerr << "Fehler: leere Messdatei." << endl;
        return false;
    }

    stringstream header(line);

    string token;

    header >> token;

    while(header >> token)
    {
        measurement.phis.push_back(stod(token));
    }

    const size_t nPhi =
        measurement.phis.size();

    cout << "Messdatei:" << endl;

    cout << "  Phi-Winkel: "
         << nPhi
         << endl;

    measurement.lengths.resize(nPhi);

    // --------------------------------------------------------
    // Daten
    // --------------------------------------------------------

    while(getline(in,line))
    {
        if(line.empty())
            continue;

        stringstream ss(line);

        double t;

        if(!(ss >> t))
            continue;

        measurement.times.push_back(t);

        for(size_t i=0;
            i<nPhi;
            ++i)
        {
            double value = 0.0;

            if(!(ss >> value))
            {
                cerr << "Warnung: fehlender Messwert bei "
                     << "t = " << t
                     << ", phi = "
                     << measurement.phis[i]
                     << endl;

                value = 0.0;
            }

            measurement.lengths[i].push_back(value);
        }
    }

    cout << "  Zeitpunkte: "
         << measurement.times.size()
         << endl;

    if(!measurement.times.empty())
    {
        cout << "  Messbereich: "
             << measurement.times.front()
             << " ns - "
             << measurement.times.back()
             << " ns"
             << endl;
    }

    return true;
}


// ============================================================
// SIMULATION EINLESEN
// ============================================================

bool ReadSimulation(
    const char* filename,
    vector<SimCurve>& curves)
{
    TFile* file =
        TFile::Open(filename,"READ");

    if(!file || file->IsZombie())
    {
        cerr << "Fehler: Simulation konnte nicht geoeffnet werden: "
             << filename
             << endl;

        return false;
    }

    TTree* tree =
        dynamic_cast<TTree*>(
            file->Get("DriftKorrTree"));

    if(!tree)
    {
        cerr << "Fehler: TTree 'DriftKorrTree' nicht gefunden."
             << endl;

        file->Close();

        return false;
    }

    double driftTime;
    double phi;
    double driftLength;

    int ySideStartDrift;

    tree->SetBranchAddress(
        "drifttime_MinDT",
        &driftTime);

    tree->SetBranchAddress(
        "trackAngleDeg",
        &phi);

    tree->SetBranchAddress(
        "driftlength_Corrected",
        &driftLength);

    tree->SetBranchAddress(
        "ySideStartDrift_MinDT",
        &ySideStartDrift);

    struct RawPoint
    {
        double phi;
        double time;
        double length;
    };

    vector<RawPoint> points;

    const Long64_t nEntries =
        tree->GetEntries();

    cout << endl;
    cout << "Simulation:" << endl;

    cout << "  Entries: "
         << nEntries
         << endl;

    // --------------------------------------------------------
    // Daten einlesen
    // --------------------------------------------------------

    for(Long64_t i=0;
        i<nEntries;
        ++i)
    {
        tree->GetEntry(i);

        // Nur gültige Startposition
        if(ySideStartDrift <= 0)
            continue;

        if(!std::isfinite(driftTime) ||
           !std::isfinite(phi) ||
           !std::isfinite(driftLength))
        {
            continue;
        }

        RawPoint p;

        p.phi    = phi;
        p.time   = driftTime;
        p.length = driftLength * 10.0;

        points.push_back(p);
    }

    // --------------------------------------------------------
    // vorhandene Simulationswinkel
    // --------------------------------------------------------

    vector<double> simPhis;

    for(const auto& p : points)
    {
        bool exists = false;

        for(double oldPhi : simPhis)
        {
            if(std::abs(oldPhi-p.phi) < 1e-9)
            {
                exists = true;
                break;
            }
        }

        if(!exists)
            simPhis.push_back(p.phi);
    }

    sort(
        simPhis.begin(),
        simPhis.end());

    cout << "  Gueltige Phi-Winkel: "
         << simPhis.size()
         << endl;

    // --------------------------------------------------------
    // Kurven aufbauen
    // --------------------------------------------------------

    for(double simPhi : simPhis)
    {
        SimCurve curve;

        curve.phi = simPhi;

        for(const auto& p : points)
        {
            if(std::abs(p.phi-simPhi) < 1e-9)
            {
                curve.time.push_back(p.time);
                curve.length.push_back(p.length);
            }
        }

        // ----------------------------------------------------
        // nach Zeit sortieren
        // ----------------------------------------------------

        vector<size_t> order(
            curve.time.size());

        for(size_t i=0;
            i<order.size();
            ++i)
        {
            order[i] = i;
        }

        sort(
            order.begin(),
            order.end(),
            [&](size_t a,size_t b)
            {
                return curve.time[a] <
                       curve.time[b];
            });

        vector<double> sortedTime;
        vector<double> sortedLength;

        sortedTime.reserve(order.size());
        sortedLength.reserve(order.size());

        for(size_t i : order)
        {
            sortedTime.push_back(
                curve.time[i]);

            sortedLength.push_back(
                curve.length[i]);
        }

        curve.time   = sortedTime;
        curve.length = sortedLength;

        curves.push_back(curve);
    }

    file->Close();

    return true;
}


// ============================================================
// PASSENDEN SIMULATIONSPHI FINDEN
// ============================================================

int FindSimulationPhi(
    double measurementPhi,
    const vector<SimCurve>& curves)
{
    int best = -1;

    double bestDifference =
        PHI_TOLERANCE;

    for(size_t i=0;
        i<curves.size();
        ++i)
    {
        const double difference =
            std::abs(
                curves[i].phi -
                measurementPhi);

        if(difference < bestDifference)
        {
            bestDifference = difference;
            best = static_cast<int>(i);
        }
    }

    return best;
}


// ============================================================
// LINEARE INTERPOLATION
// ============================================================

double Interpolate(
    const vector<double>& x,
    const vector<double>& y,
    double xValue)
{
    if(x.empty())
        return NAN;

    if(xValue <= x.front())
        return y.front();

    if(xValue >= x.back())
        return y.back();

    auto it =
        lower_bound(
            x.begin(),
            x.end(),
            xValue);

    const size_t i =
        static_cast<size_t>(
            it-x.begin());

    if(i == 0)
        return y[0];

    const double x1 = x[i-1];
    const double x2 = x[i];

    const double y1 = y[i-1];
    const double y2 = y[i];

    if(x2 == x1)
        return y1;

    const double f =
        (xValue-x1) /
        (x2-x1);

    return y1 +
           f*(y2-y1);
}


// ============================================================
// ERSTEN SINNVOLLEN MESSPUNKT FINDEN
// ============================================================

size_t FindMeasurementStart(
    const vector<double>& times,
    const vector<double>& lengths)
{
    const size_t n =
        std::min(
            times.size(),
            lengths.size());

    for(size_t i=0;
        i<n;
        ++i)
    {
        const double t =
            times[i];

        const double l =
            lengths[i];

        if(t < MEASUREMENT_START_MIN ||
           t > MEASUREMENT_START_MAX)
        {
            continue;
        }

        if(l > 0.0)
            return i;
    }

    return n;
}


// ============================================================
// ZEITSHIFT BESTIMMEN
// ============================================================

double DetermineTimeShift(
    const vector<double>& measurementTimes,
    const vector<double>& measurementLengths,
    const SimCurve& sim)
{
    if(sim.time.empty())
        return 0.0;

    double bestShift = 0.0;

    double bestScore =
        1e300;

    for(double shift = SHIFT_MIN;
        shift <= SHIFT_MAX + 1e-9;
        shift += SHIFT_STEP)
    {
        double score = 0.0;

        int n = 0;

        for(size_t i=0;
            i<measurementTimes.size();
            ++i)
        {
            const double tMeas =
                measurementTimes[i];

            if(tMeas < SHIFT_FIT_START ||
               tMeas > SHIFT_FIT_END)
            {
                continue;
            }

            const double lMeas =
                measurementLengths[i];

            if(lMeas <= 0.0)
                continue;

            const double tSim =
                tMeas - shift;

            if(tSim < sim.time.front() ||
               tSim > sim.time.back())
            {
                continue;
            }

            const double lSim =
                Interpolate(
                    sim.time,
                    sim.length,
                    tSim);

            const double diff =
                lSim - lMeas;

            score += diff*diff;

            ++n;
        }

        if(n < 3)
            continue;

        score /= static_cast<double>(n);

        if(score < bestScore)
        {
            bestScore = score;
            bestShift = shift;
        }
    }

    return bestShift;
}


// ============================================================
// WINKELABHÄNGIGER ÜBERGANGSBEGINN
// ============================================================

double GetTransitionStart(
    double phi)
{
    if(phi <= PHI_TRANSITION_LOW)
        return TRANSITION_START_LOW;

    if(phi >= PHI_TRANSITION_HIGH)
        return TRANSITION_START_HIGH;

    const double f =
        (phi - PHI_TRANSITION_LOW) /
        (PHI_TRANSITION_HIGH -
         PHI_TRANSITION_LOW);

    return
        TRANSITION_START_LOW +
        f *
        (TRANSITION_START_HIGH -
         TRANSITION_START_LOW);
}


// ============================================================
// SMOOTHSTEP
// ============================================================

double SmoothStep(double x)
{
    x =
        std::max(
            0.0,
            std::min(
                1.0,
                x));

    return x*x*(3.0 - 2.0*x);
}


// ============================================================
// SIMULATIONSWERT FÜR MESSZEIT
// ============================================================

double GetSimulationValue(
    double measurementTime,
    const SimCurve& sim,
    double timeShift)
{
    if(sim.time.empty())
        return 0.0;

    const double tSim =
        measurementTime - timeShift;

    if(tSim <= sim.time.front())
        return sim.length.front();

    if(tSim >= sim.time.back())
        return sim.length.back();

    return Interpolate(
        sim.time,
        sim.length,
        tSim);
}


// ============================================================
// VERBESSERTE KURVE
//
// WICHTIG:
// Die Eingabe times ist jetzt eine eigene Ausgabezeitachse.
// Dadurch kann die Kurve über 250 ns hinaus bis zum Ende der
// Simulation laufen.
// ============================================================

vector<double> BuildImprovedCurve(
    const vector<double>& outputTimes,
    const vector<double>& measurementTimes,
    const vector<double>& measurementLengths,
    const SimCurve& sim,
    double timeShift,
    double phi)
{
    vector<double> result(
        outputTimes.size(),
        0.0);

    const size_t first =
        FindMeasurementStart(
            measurementTimes,
            measurementLengths);

    if(first >= measurementTimes.size())
    {
        cerr << "Warnung: kein Messbeginn gefunden fuer phi = "
             << phi
             << endl;

        return result;
    }

    // --------------------------------------------------------
    // Winkelabhängiger Übergang
    // --------------------------------------------------------

    const double transitionStart =
        GetTransitionStart(phi);

    const double transitionEnd =
        transitionStart +
        TRANSITION_WIDTH;

    cout << "  Transition start = "
         << transitionStart
         << " ns"
         << endl;

    cout << "  Transition end   = "
         << transitionEnd
         << " ns"
         << endl;

    // --------------------------------------------------------
    // Kurve erzeugen
    // --------------------------------------------------------

    for(size_t i=0;
        i<outputTimes.size();
        ++i)
    {
        const double t =
            outputTimes[i];

        // ----------------------------------------------------
        // Vor Messbeginn
        // ----------------------------------------------------

        if(t < measurementTimes[first])
        {
            result[i] = 0.0;
            continue;
        }

        // ----------------------------------------------------
        // Messwert bestimmen
        //
        // Innerhalb des Messbereichs nehmen wir den tatsächlichen
        // Messwert.
        //
        // Außerhalb des Messbereichs existiert keine Messung.
        // ----------------------------------------------------

        double lMeas = 0.0;

        bool hasMeasurement = false;

        if(!measurementTimes.empty() &&
           t >= measurementTimes.front() &&
           t <= measurementTimes.back())
        {
            lMeas =
                Interpolate(
                    measurementTimes,
                    measurementLengths,
                    t);

            hasMeasurement = true;
        }

        // ----------------------------------------------------
        // Simulationswert
        // ----------------------------------------------------

        const double lSim =
            GetSimulationValue(
                t,
                sim,
                timeShift);

        // ----------------------------------------------------
        // Messung vollständig verwenden
        // ----------------------------------------------------

        if(hasMeasurement &&
           t <= transitionStart)
        {
            result[i] = lMeas;
            continue;
        }

        // ----------------------------------------------------
        // Simulation vollständig verwenden
        // ----------------------------------------------------

        if(t >= transitionEnd)
        {
            result[i] = lSim;
            continue;
        }

        // ----------------------------------------------------
        // Übergangsbereich
        // ----------------------------------------------------

        if(hasMeasurement)
        {
            const double x =
                (t-transitionStart) /
                TRANSITION_WIDTH;

            const double w =
                SmoothStep(x);

            result[i] =
                (1.0-w)*lMeas +
                w*lSim;
        }
        else
        {
            // Sicherheitsfall:
            // Keine Messung vorhanden -> Simulation
            result[i] = lSim;
        }
    }

    return result;
}


// ============================================================
// AUSGABEZEITACHSE ERZEUGEN
//
// Die Messung endet normalerweise bei 250 ns.
//
// Die Simulation wird aber nach dem Shift weitergeführt.
//
// Deshalb erzeugen wir hier eine gemeinsame Zeitachse:
//
// 0 ... Messende ... SimulationEnde + Shift
//
// mit dem gleichen Zeitraster wie die Messdatei.
// ============================================================

vector<double> BuildOutputTimes(
    const Measurement& measurement,
    const vector<SimCurve>& simulations)
{
    vector<double> outputTimes;

    if(measurement.times.empty())
        return outputTimes;

    // --------------------------------------------------------
    // Zeitraster der Messung bestimmen
    // --------------------------------------------------------

    double dt = 0.5;

    if(measurement.times.size() >= 2)
    {
        dt =
            measurement.times[1] -
            measurement.times[0];
    }

    if(dt <= 0.0)
        dt = 0.5;

    // --------------------------------------------------------
    // Wir brauchen zunächst die maximale Simulationszeit.
    //
    // Da der Shift winkelabhängig bestimmt wird, können die
    // einzelnen Winkel theoretisch unterschiedliche Endzeiten
    // besitzen.
    //
    // Wir nehmen deshalb zunächst das globale maximale Ende.
    // --------------------------------------------------------

    double maxOutputTime =
        measurement.times.back();

    for(const auto& sim : simulations)
    {
        if(sim.time.empty())
            continue;

        maxOutputTime =
            std::max(
                maxOutputTime,
                sim.time.back() + SHIFT_MAX);
    }

    // --------------------------------------------------------
    // Zeitachse erzeugen
    // --------------------------------------------------------

    const double startTime =
        measurement.times.front();

    for(double t = startTime;
        t <= maxOutputTime + 1e-9;
        t += dt)
    {
        outputTimes.push_back(t);
    }

    return outputTimes;
}


// ============================================================
// PLOT: EINZELNER WINKEL
// ============================================================

void MakeIndividualPlot(
    const Measurement& measurement,
    size_t iphi,
    const SimCurve& sim,
    double timeShift,
    const vector<double>& outputTimes,
    const vector<double>& improved)
{
    const double phi =
        measurement.phis[iphi];

    // --------------------------------------------------------
    // Dateiname
    // --------------------------------------------------------

    stringstream filename;

    filename << PLOT_DIR
             << "/driftlength_phi_"
             << fixed
             << setprecision(1)
             << phi
             << ".pdf";

    TCanvas canvas(
        Form("c_phi_%03d",
             static_cast<int>(iphi)),
        Form("Drift length phi %.1f",
             phi),
        1000,
        700);

    canvas.SetGrid();

    // --------------------------------------------------------
    // Messung
    // --------------------------------------------------------

    TGraph gMeas(
        static_cast<int>(
            measurement.times.size()));

    for(size_t i=0;
        i<measurement.times.size();
        ++i)
    {
        gMeas.SetPoint(
            static_cast<int>(i),
            measurement.times[i],
            measurement.lengths[iphi][i]);
    }

    // --------------------------------------------------------
    // Simulation
    //
    // Zeit wird auf Messzeitachse verschoben.
    // --------------------------------------------------------

    TGraph gSim;

    int nSim = 0;

    for(size_t i=0;
        i<sim.time.size();
        ++i)
    {
        const double t =
            sim.time[i] + timeShift;

        gSim.SetPoint(
            nSim++,
            t,
            sim.length[i]);
    }

    // --------------------------------------------------------
    // Verbesserte Kurve
    // --------------------------------------------------------

    TGraph gImproved(
        static_cast<int>(
            outputTimes.size()));

    for(size_t i=0;
        i<outputTimes.size();
        ++i)
    {
        gImproved.SetPoint(
            static_cast<int>(i),
            outputTimes[i],
            improved[i]);
    }

    // --------------------------------------------------------
    // Farben + Linienstile
    // --------------------------------------------------------

    // Measurement:
    // blau + Punkte / durchgezogene Linie
    gMeas.SetLineColor(kBlue + 1);
    gMeas.SetMarkerColor(kBlue + 1);
    gMeas.SetLineStyle(1);
    gMeas.SetLineWidth(2);

    gMeas.SetMarkerStyle(20);
    gMeas.SetMarkerSize(0.5);

    // Simulation:
    // rot + dashed
    gSim.SetLineColor(kRed + 1);
    gSim.SetLineStyle(2);
    gSim.SetLineWidth(2);

    // Improved:
    // grün + durchgezogen + stärker
    gImproved.SetLineColor(kGreen + 2);
    gImproved.SetLineStyle(1);
    gImproved.SetLineWidth(3);

    // --------------------------------------------------------
    // Zeichnen
    // --------------------------------------------------------

    gMeas.SetTitle(
        Form("Drift length relation, #phi = %.1f#circ",
             phi));

    gMeas.GetXaxis()->SetTitle(
        "Drift time (ns)");

    gMeas.GetYaxis()->SetTitle(
        "Drift length (mm)");

    gMeas.GetYaxis()->SetRangeUser(
        Y_MIN,
        Y_MAX);

    gMeas.Draw("APL");

    gSim.Draw("L SAME");

    gImproved.Draw("L SAME");

    // --------------------------------------------------------
    // Übergangsbereich markieren
    // --------------------------------------------------------

    const double transitionStart =
        GetTransitionStart(phi);

    const double transitionEnd =
        transitionStart +
        TRANSITION_WIDTH;

    TLine lineStart(
        transitionStart,
        Y_MIN,
        transitionStart,
        Y_MAX);

    TLine lineEnd(
        transitionEnd,
        Y_MIN,
        transitionEnd,
        Y_MAX);

    lineStart.SetLineColor(kBlack);
    lineEnd.SetLineColor(kBlack);

    lineStart.SetLineStyle(3);
    lineEnd.SetLineStyle(3);

    lineStart.SetLineWidth(1);
    lineEnd.SetLineWidth(1);

    lineStart.Draw("same");
    lineEnd.Draw("same");

    // --------------------------------------------------------
    // Legende LINKS OBEN
    // --------------------------------------------------------

    TLegend legend(
        0.15,
        0.68,
        0.45,
        0.88);

    legend.AddEntry(
        &gMeas,
        "Measurement",
        "lp");

    legend.AddEntry(
        &gSim,
        "Simulation",
        "l");

    legend.AddEntry(
        &gImproved,
        "Improved",
        "l");

    legend.AddEntry(
        &lineStart,
        "Transition start/end",
        "l");

    legend.SetBorderSize(0);

    legend.Draw();

    canvas.SaveAs(
        filename.str().c_str());
}


// ============================================================
// ÜBERSICHTSPLOT
//
// Jetzt ebenfalls bis zum maximalen Simulationsende.
// ============================================================

void MakeOverviewPlot(
    const Measurement& measurement,
    const vector<double>& outputTimes,
    const vector<vector<double>>& improved)
{
    if(outputTimes.empty())
        return;

    TCanvas canvas(
        "c_overview",
        "Improved drift length relation",
        1100,
        800);

    canvas.SetGrid();

    TLegend legend(
        0.15,
        0.60,
        0.40,
        0.88);

    bool first = true;

    for(size_t iphi=0;
        iphi<measurement.phis.size();
        ++iphi)
    {
        TGraph* g =
            new TGraph(
                static_cast<int>(
                    outputTimes.size()));

        for(size_t i=0;
            i<outputTimes.size();
            ++i)
        {
            g->SetPoint(
                static_cast<int>(i),
                outputTimes[i],
                improved[iphi][i]);
        }

        // ----------------------------------------------------
        // Farbe nach Winkel
        // ----------------------------------------------------

        const int colorIndex =
            static_cast<int>(iphi % 9);

        const int colors[] =
        {
            kBlue + 1,
            kRed + 1,
            kGreen + 2,
            kMagenta + 1,
            kCyan + 1,
            kOrange + 7,
            kViolet + 1,
            kAzure + 2,
            kPink + 1
        };

        g->SetLineColor(
            colors[colorIndex]);

        // zusätzlich unterschiedliche Linienstile
        const int styles[] =
        {
            1,
            2,
            3,
            4,
            5,
            6
        };

        g->SetLineStyle(
            styles[iphi % 6]);

        g->SetLineWidth(2);

        if(first)
        {
            g->SetTitle(
                "Improved drift length relations");

            g->GetXaxis()->SetTitle(
                "Drift time (ns)");

            g->GetYaxis()->SetTitle(
                "Drift length (mm)");

            g->GetYaxis()->SetRangeUser(
                Y_MIN,
                Y_MAX);

            g->Draw("AL");

            first = false;
        }
        else
        {
            g->Draw("L SAME");
        }

        legend.AddEntry(
            g,
            Form("#phi = %.1f#circ",
                 measurement.phis[iphi]),
            "l");
    }

    legend.SetBorderSize(0);

    legend.Draw();

    string filename =
        string(PLOT_DIR) +
        "/driftlength_overview.pdf";

    canvas.SaveAs(
        filename.c_str());
}


// ============================================================
// TABELLE SCHREIBEN
//
// Jetzt mit eigener outputTimes-Achse.
// ============================================================

bool WriteMeasurement(
    const char* filename,
    const Measurement& measurement,
    const vector<double>& outputTimes,
    const vector<vector<double>>& result)
{
    ofstream out(filename);

    if(!out)
    {
        cerr << "Fehler: Ausgabedatei konnte nicht erstellt werden: "
             << filename
             << endl;

        return false;
    }

    // --------------------------------------------------------
    // Header
    // --------------------------------------------------------

    out << "t(ns)";

    for(double phi : measurement.phis)
    {
        out << "\t"
            << fixed
            << setprecision(6)
            << phi;
    }

    out << "\n";

    // --------------------------------------------------------
    // Daten
    // --------------------------------------------------------

    for(size_t it=0;
        it<outputTimes.size();
        ++it)
    {
        out << fixed
            << setprecision(1)
            << outputTimes[it];

        for(size_t iphi=0;
            iphi<measurement.phis.size();
            ++iphi)
        {
            out << "\t"
                << setprecision(10)
                << result[iphi][it];
        }

        out << "\n";
    }

    out.close();

    return true;
}


// ============================================================
// HAUPTFUNKTION
// ============================================================

void ImproveDriftTable2()
{
    cout << endl;
    cout << "========================================" << endl;
    cout << " Improve Drift Table" << endl;
    cout << "========================================" << endl;

    // --------------------------------------------------------
    // Plot-Verzeichnis erzeugen
    // --------------------------------------------------------

    gSystem->mkdir(
        PLOT_DIR,
        true);

    // --------------------------------------------------------
    // Messung
    // --------------------------------------------------------

    Measurement measurement;

    if(!ReadMeasurement(
           INPUT_MEASUREMENT,
           measurement))
    {
        return;
    }

    // --------------------------------------------------------
    // Simulation
    // --------------------------------------------------------

    vector<SimCurve> simulations;

    if(!ReadSimulation(
           INPUT_SIMULATION,
           simulations))
    {
        return;
    }

    // --------------------------------------------------------
    // Gemeinsame Ausgabezeitachse
    //
    // Wichtig:
    // Die Zeitachse geht jetzt bis zum Ende der Simulation.
    // --------------------------------------------------------

    vector<double> outputTimes =
        BuildOutputTimes(
            measurement,
            simulations);

    if(outputTimes.empty())
    {
        cerr << "Fehler: keine Ausgabezeitpunkte."
             << endl;

        return;
    }

    cout << endl;
    cout << "Ausgabezeitachse:" << endl;

    cout << "  Start = "
         << outputTimes.front()
         << " ns"
         << endl;

    cout << "  Ende  = "
         << outputTimes.back()
         << " ns"
         << endl;

    cout << "  Punkte = "
         << outputTimes.size()
         << endl;

    // --------------------------------------------------------
    // Ergebnis
    // --------------------------------------------------------

    vector<vector<double>> result;

    result.resize(
        measurement.phis.size());

    // --------------------------------------------------------
    // Alle Winkel
    // --------------------------------------------------------

    for(size_t iphi=0;
        iphi<measurement.phis.size();
        ++iphi)
    {
        const double phi =
            measurement.phis[iphi];

        cout << endl;
        cout << "----------------------------------------"
             << endl;

        cout << "Phi = "
             << phi
             << " deg"
             << endl;

        // ----------------------------------------------------
        // passende Simulation
        // ----------------------------------------------------

        const int isim =
            FindSimulationPhi(
                phi,
                simulations);

        if(isim < 0)
        {
            cerr << "  KEINE Simulation innerhalb "
                 << PHI_TOLERANCE
                 << " deg gefunden!"
                 << endl;

            // Auch hier eine Ausgabe über die komplette
            // Zeitachse erzeugen.
            result[iphi].resize(
                outputTimes.size(),
                0.0);

            for(size_t i=0;
                i<outputTimes.size();
                ++i)
            {
                if(outputTimes[i] < measurement.times.front())
                {
                    result[iphi][i] = 0.0;
                }
                else if(outputTimes[i] <= measurement.times.back())
                {
                    result[iphi][i] =
                        Interpolate(
                            measurement.times,
                            measurement.lengths[iphi],
                            outputTimes[i]);
                }
                else
                {
                    result[iphi][i] =
                        measurement.lengths[iphi].back();
                }
            }

            continue;
        }

        const SimCurve& sim =
            simulations[isim];

        cout << "  Simulation Phi = "
             << sim.phi
             << " deg"
             << endl;

        cout << "  Delta Phi = "
             << std::abs(
                    sim.phi-phi)
             << " deg"
             << endl;

        if(!sim.time.empty())
        {
            cout << "  Simulation time = "
                 << sim.time.front()
                 << " ns - "
                 << sim.time.back()
                 << " ns"
                 << endl;
        }

        // ----------------------------------------------------
        // Zeitshift
        // ----------------------------------------------------

        const double timeShift =
            DetermineTimeShift(
                measurement.times,
                measurement.lengths[iphi],
                sim);

        cout << "  Time shift = "
             << timeShift
             << " ns"
             << endl;

        // ----------------------------------------------------
        // tatsächliches Ende der verschobenen Simulation
        // ----------------------------------------------------

        if(!sim.time.empty())
        {
            cout << "  Shifted simulation end = "
                 << sim.time.back() + timeShift
                 << " ns"
                 << endl;
        }

        // ----------------------------------------------------
        // verbesserte Kurve
        // ----------------------------------------------------

        result[iphi] =
            BuildImprovedCurve(
                outputTimes,
                measurement.times,
                measurement.lengths[iphi],
                sim,
                timeShift,
                phi);

        // ----------------------------------------------------
        // Einzelplot
        // ----------------------------------------------------

        MakeIndividualPlot(
            measurement,
            iphi,
            sim,
            timeShift,
            outputTimes,
            result[iphi]);

        cout << "  Plot gespeichert."
             << endl;
    }

    // --------------------------------------------------------
    // Übersicht
    // --------------------------------------------------------

    MakeOverviewPlot(
        measurement,
        outputTimes,
        result);

    // --------------------------------------------------------
    // Tabelle schreiben
    // --------------------------------------------------------

    if(!WriteMeasurement(
           OUTPUT_FILE,
           measurement,
           outputTimes,
           result))
    {
        return;
    }

    cout << endl;
    cout << "========================================" << endl;
    cout << "Fertig!" << endl;
    cout << endl;

    cout << "Ausgabe:" << endl;
    cout << "  "
         << OUTPUT_FILE
         << endl;

    cout << endl;

    cout << "Plots:" << endl;
    cout << "  "
         << PLOT_DIR
         << "/" << endl;

    cout << endl;

    cout << "Ausgabezeit:" << endl;
    cout << "  "
         << outputTimes.front()
         << " ns - "
         << outputTimes.back()
         << " ns"
         << endl;

    cout << "========================================" << endl;
}
