#include <TFile.h>
#include <TTree.h>
#include <TGraph.h>
#include <TCanvas.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;


// ============================================================
// KONFIGURATION
// ============================================================

// Eingabedatei mit der gemessenen Driftzeit -> Driftweg Tabelle
const char* INPUT_TABLE  = "driftlength_relation_X2.txt";

// Ausgabedatei
const char* OUTPUT_TABLE = "driftlength_relation_X2_improved.txt";

// ROOT-Datei der Simulation
const char* SIM_FILE = "/d/d04-1/ag_pi/spektrometer/QCLAM/Driftkammern/3Gen/GarfieldSimulationen/2D/DriftwegDriftzeitKorr/Tabellen/korrektur-daten/Driftkorrekturen_DriftkammerGen_3_Ar_80.000000_CO2_20.000000_O2_0.000000_HV_-4300.000000_AnzahlSimDraehte_10_AnzahlSimulierterWinkel_451_AnzahlSimulierterDatenProWinkel_91441.root";

// Tree-Name
const char* TREE_NAME = "DriftKorrTree";

// ------------------------------------------------------------
// Simulationszweige
// ------------------------------------------------------------

const char* BR_TIME   = "drifttime_MinDT";
const char* BR_ANGLE  = "trackAngleDeg";
const char* BR_LENGTH = "driftlength_Corrected";

// ------------------------------------------------------------
// Anpassungsparameter
// ------------------------------------------------------------

// Zeitbereich der Messung, der verwendet werden soll,
// um die Simulation an die Messung anzupassen.
//
// Beispiel:
// 100 ns  -> Messung bis 100 ns verwenden,
//            danach Simulation.
double FIT_TIME = 100.0;

// Toleranz bei der Winkelauswahl
double ANGLE_TOLERANCE = 0.1;

// Shift der Simulationszeit.
// Wenn NEGATIV/NULL:
// Shift wird automatisch aus dem Beginn der Messkurve bestimmt.
double TIME_SHIFT = NAN;

// ------------------------------------------------------------
// Einheiten
// ------------------------------------------------------------

// Simulation: driftlength_Corrected ist cm
// Tabelle: Driftweg ist mm
const double CM_TO_MM = 10.0;


// ============================================================
// Hilfsstruktur
// ============================================================

struct DriftTable
{
    vector<double> times;

    // angle -> drift lengths
    map<double, vector<double>> lengths;
};


// ============================================================
// MESS-TABELLE EINLESEN
// ============================================================

DriftTable ReadMeasurementTable(const char* filename)
{
    DriftTable table;

    ifstream in(filename);

    if(!in)
    {
        cerr << "ERROR: Konnte Tabelle nicht oeffnen: "
             << filename << endl;

        return table;
    }

    string line;

    // --------------------------------------------------------
    // Erste Zeile:
    //
    // t(ns)  20 21 22 ... 65
    // --------------------------------------------------------

    if(!getline(in,line))
        return table;

    stringstream ss(line);

    string first;
    ss >> first;

    double angle;

    while(ss >> angle)
    {
        table.lengths[angle] = vector<double>();
    }

    // --------------------------------------------------------
    // Daten
    // --------------------------------------------------------

    while(getline(in,line))
    {
        if(line.empty())
            continue;

        stringstream row(line);

        double time;
        row >> time;

        table.times.push_back(time);

        for(auto& [angle, values] : table.lengths)
        {
            double value;

            if(row >> value)
                values.push_back(value);
            else
                values.push_back(NAN);
        }
    }

    in.close();

    cout << "Mess-Tabelle gelesen:" << endl;
    cout << "  Zeiten : " << table.times.size() << endl;
    cout << "  Winkel : " << table.lengths.size() << endl;

    return table;
}


// ============================================================
// SIMULATION EINLESEN
// ============================================================

struct SimPoint
{
    double time;
    double length;
};


map<double, vector<SimPoint>>
ReadSimulation(const char* filename,
               const char* treeName,
               const char* brTime,
               const char* brAngle,
               const char* brLength)
{
    map<double, vector<SimPoint>> simulation;

    TFile* file = TFile::Open(filename);

    if(!file || file->IsZombie())
    {
        cerr << "ERROR: Konnte ROOT-Datei nicht oeffnen."
             << endl;

        return simulation;
    }

    TTree* tree = dynamic_cast<TTree*>(file->Get(treeName));

    if(!tree)
    {
        cerr << "ERROR: Tree nicht gefunden: "
             << treeName << endl;

        file->Close();
        return simulation;
    }


    double time;
    double angle;
    double length;

    tree->SetBranchAddress(brTime,   &time);
    tree->SetBranchAddress(brAngle,  &angle);
    tree->SetBranchAddress(brLength, &length);


    Long64_t nEntries = tree->GetEntries();

    cout << "Simulation:" << endl;
    cout << "  Entries = " << nEntries << endl;


    for(Long64_t i=0; i<nEntries; ++i)
    {
        tree->GetEntry(i);

        if(!std::isfinite(time))
            continue;

        if(!std::isfinite(angle))
            continue;

        if(!std::isfinite(length))
            continue;

        simulation[angle].push_back(
            {
                time,
                length * CM_TO_MM
            }
        );
    }


    file->Close();

    return simulation;
}


// ============================================================
// NÄCHSTEN SIMULATIONS-WINKEL FINDEN
// ============================================================

double FindClosestAngle(
        double requestedAngle,
        const map<double, vector<SimPoint>>& simulation)
{
    double bestAngle = NAN;
    double bestDiff = 1e99;

    for(const auto& [angle, points] : simulation)
    {
        double diff = fabs(angle - requestedAngle);

        if(diff < bestDiff)
        {
            bestDiff = diff;
            bestAngle = angle;
        }
    }

    if(bestDiff > ANGLE_TOLERANCE)
    {
        cerr << "WARNING: Kein Simulationswinkel nahe "
             << requestedAngle
             << " deg gefunden. Naechster Winkel = "
             << bestAngle
             << " deg" << endl;
    }

    return bestAngle;
}


// ============================================================
// LINEARE INTERPOLATION DER SIMULATION
// ============================================================

double InterpolateSimulation(
        const vector<SimPoint>& points,
        double time)
{
    if(points.empty())
        return NAN;

    if(time <= points.front().time)
        return points.front().length;

    if(time >= points.back().time)
        return points.back().length;


    for(size_t i=1; i<points.size(); ++i)
    {
        if(points[i].time >= time)
        {
            double t1 = points[i-1].time;
            double t2 = points[i].time;

            double y1 = points[i-1].length;
            double y2 = points[i].length;

            double f = (time - t1)/(t2-t1);

            return y1 + f*(y2-y1);
        }
    }

    return NAN;
}


// ============================================================
// ZEIT-SHIFT BESTIMMEN
// ============================================================
//
// Die Simulation beginnt typischerweise bei einer anderen
// Driftzeit als die Messung.
//
// Wir bestimmen zunächst:
//
//     t_shift = t_meas_start - t_sim_start
//
// ============================================================

double DetermineTimeShift(
        const vector<double>& measurementTimes,
        const vector<SimPoint>& simulation)
{
    if(measurementTimes.empty() || simulation.empty())
        return 0.0;

    double tMeas = measurementTimes.front();
    double tSim  = simulation.front().time;

    return tMeas - tSim;
}


// ============================================================
// NEUE KURVE ERZEUGEN
// ============================================================

vector<double> BuildImprovedCurve(
        const vector<double>& measurementTimes,
        const vector<double>& measurementLengths,
        const vector<SimPoint>& simulation,
        double timeShift,
        double fitTime)
{
    vector<double> result;

    result.reserve(measurementTimes.size());


    for(size_t i=0; i<measurementTimes.size(); ++i)
    {
        double t = measurementTimes[i];

        // ----------------------------------------------------
        // Bereich 1:
        // Messung verwenden
        // ----------------------------------------------------

        if(t <= fitTime)
        {
            result.push_back(measurementLengths[i]);
            continue;
        }


        // ----------------------------------------------------
        // Bereich 2:
        // Simulation verwenden
        // ----------------------------------------------------

        double tSim = t - timeShift;

        double length =
            InterpolateSimulation(simulation,tSim);

        result.push_back(length);
    }

    return result;
}


// ============================================================
// TABELLE SCHREIBEN
// ============================================================

void WriteMeasurementTable(
        const char* filename,
        const DriftTable& table)
{
    ofstream out(filename);

    if(!out)
    {
        cerr << "ERROR: Konnte Ausgabedatei nicht schreiben."
             << endl;
        return;
    }


    // --------------------------------------------------------
    // Header
    // --------------------------------------------------------

    out << "t(ns)";

    for(const auto& [angle, values] : table.lengths)
    {
        out << setw(16)
            << fixed
            << setprecision(6)
            << angle;
    }

    out << "\n";


    // --------------------------------------------------------
    // Daten
    // --------------------------------------------------------

    for(size_t i=0; i<table.times.size(); ++i)
    {
        out << fixed
            << setprecision(1)
            << table.times[i];

        for(const auto& [angle, values] : table.lengths)
        {
            double value = values[i];

            out << "   "
                << fixed
                << setprecision(10)
                << value;
        }

        out << "\n";
    }

    out.close();

    cout << "Neue Tabelle geschrieben: "
         << filename << endl;
}


// ============================================================
// HAUPTFUNKTION
// ============================================================

void ImproveDriftTable()
{
    // --------------------------------------------------------
    // 1. Mess-Tabelle lesen
    // --------------------------------------------------------

    DriftTable measurement =
        ReadMeasurementTable(INPUT_TABLE);


    if(measurement.times.empty())
    {
        cerr << "Keine Messdaten!" << endl;
        return;
    }


    // --------------------------------------------------------
    // 2. Simulation lesen
    // --------------------------------------------------------

    auto simulation =
        ReadSimulation(
            SIM_FILE,
            TREE_NAME,
            BR_TIME,
            BR_ANGLE,
            BR_LENGTH
        );


    if(simulation.empty())
    {
        cerr << "Keine Simulationsdaten!" << endl;
        return;
    }


    // --------------------------------------------------------
    // 3. Jeden Winkel bearbeiten
    // --------------------------------------------------------

    DriftTable improved;

    improved.times = measurement.times;


    for(const auto& [measurementAngle, measurementValues]
        : measurement.lengths)
    {
        cout << endl;
        cout << "========================================"
             << endl;

        cout << "Phi = "
             << measurementAngle
             << " deg"
             << endl;


        // ----------------------------------------------------
        // passenden Simulationswinkel suchen
        // ----------------------------------------------------

        double simAngle =
            FindClosestAngle(
                measurementAngle,
                simulation
            );


        if(std::isnan(simAngle))
        {
            improved.lengths[measurementAngle] =
                measurementValues;

            continue;
        }


        const auto& sim =
            simulation.at(simAngle);


        // ----------------------------------------------------
        // Zeitshift bestimmen
        // ----------------------------------------------------

        double shift;

        if(std::isfinite(TIME_SHIFT))
        {
            shift = TIME_SHIFT;
        }
        else
        {
            shift =
                DetermineTimeShift(
                    measurement.times,
                    sim
                );
        }


        cout << "Simulation angle = "
             << simAngle
             << " deg" << endl;

        cout << "Time shift = "
             << shift
             << " ns" << endl;

        cout << "Measurement used until = "
             << FIT_TIME
             << " ns" << endl;


        // ----------------------------------------------------
        // neue Kurve
        // ----------------------------------------------------

        vector<double> newValues =
            BuildImprovedCurve(
                measurement.times,
                measurementValues,
                sim,
                shift,
                FIT_TIME
            );


        improved.lengths[measurementAngle] =
            newValues;
    }


    // --------------------------------------------------------
    // 4. neue Tabelle schreiben
    // --------------------------------------------------------

    WriteMeasurementTable(
        OUTPUT_TABLE,
        improved
    );


    cout << endl;
    cout << "========================================"
         << endl;

    cout << "Fertig." << endl;
}
