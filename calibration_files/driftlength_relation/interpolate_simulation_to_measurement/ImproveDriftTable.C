#include <TFile.h>
#include <TTree.h>

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
    "driftlength_relation_X1.txt";

const char* INPUT_SIMULATION =
    "/d/d04-1/ag_pi/spektrometer/QCLAM/Driftkammern/3Gen/GarfieldSimulationen/2D/DriftwegDriftzeitKorr/Tabellen/korrektur-daten/Driftkorrekturen_DriftkammerGen_3_Ar_80.000000_CO2_20.000000_O2_0.000000_HV_-4300.000000_AnzahlSimDraehte_10_AnzahlSimulierterWinkel_451_AnzahlSimulierterDatenProWinkel_91441.root";

const char* OUTPUT_FILE =
    "driftlength_relation_X1_improved.txt";


// Bereich ab Beginn der Messkurve, in dem Simulation
// an die Messung angepasst wird.
const double FIT_RANGE_NS = 0.0;


// maximal erlaubte Phi-Abweichung
const double PHI_TOLERANCE = 0.001;


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


struct SimPoint
{
    double time;
    double length;
};


struct SimCurve
{
    double phi;

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

    // erstes Feld "t(ns)"
    header >> token;


    while(header >> token)
    {
        measurement.phis.push_back(stod(token));
    }


    const size_t nPhi = measurement.phis.size();


    cout << "Messdatei:" << endl;
    cout << "  Phi-Winkel: " << nPhi << endl;


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

        ss >> t;

        measurement.times.push_back(t);


        for(size_t i=0; i<nPhi; ++i)
        {
            double value;

            ss >> value;

            measurement.lengths[i].push_back(value);
        }
    }


    cout << "  Zeitpunkte: "
         << measurement.times.size()
         << endl;


    return true;
}


// ============================================================
// SIMULATION EINLESEN
// ============================================================

bool ReadSimulation(
    const char* filename,
    vector<SimCurve>& curves)
{
    TFile* file = TFile::Open(filename,"READ");

    if(!file || file->IsZombie())
    {
        cerr << "Fehler: Simulation konnte nicht geoeffnet werden: "
             << filename << endl;

        return false;
    }


    TTree* tree =
        dynamic_cast<TTree*>(file->Get("DriftKorrTree"));

    if(!tree)
    {
        cerr << "Fehler: TTree 'TrackTree' nicht gefunden."
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


    // --------------------------------------------------------
    // Alle gültigen Simulationspunkte sammeln
    // --------------------------------------------------------

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


    for(Long64_t i=0; i<nEntries; ++i)
    {
        tree->GetEntry(i);


        // ----------------------------------------------------
        // WICHTIG:
        // Nur Einträge mit ySideStartDrift_MinDT > 0
        // ----------------------------------------------------

        if(ySideStartDrift <= 0.0)
            continue;


        if(!std::isfinite(driftTime) ||
           !std::isfinite(phi) ||
           !std::isfinite(driftLength))
        {
            continue;
        }


        RawPoint p;

        p.phi = phi;
        p.time = driftTime;

        // Simulation ist in cm
        // Ausgabe soll mm sein
        p.length = driftLength * 10.0;


        points.push_back(p);
    }


    // --------------------------------------------------------
    // verfügbare Phi-Winkel bestimmen
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


    sort(simPhis.begin(),simPhis.end());


    cout << "  Gueltige Phi-Winkel: "
         << simPhis.size()
         << endl;


    // --------------------------------------------------------
    // Für jeden Phi-Winkel Kurve aufbauen
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


        // nach Zeit sortieren

        vector<size_t> order(curve.time.size());

        for(size_t i=0; i<order.size(); ++i)
            order[i] = i;


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


        for(size_t i : order)
        {
            sortedTime.push_back(curve.time[i]);
            sortedLength.push_back(curve.length[i]);
        }


        curve.time = sortedTime;
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


    for(size_t i=0; i<curves.size(); ++i)
    {
        double difference =
            std::abs(
                curves[i].phi -
                measurementPhi);


        if(difference < bestDifference)
        {
            bestDifference = difference;
            best = i;
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


    size_t i =
        it-x.begin();


    if(i == 0)
        return y[0];


    double x1 = x[i-1];
    double x2 = x[i];

    double y1 = y[i-1];
    double y2 = y[i];


    double f =
        (xValue-x1)/(x2-x1);


    return y1 + f*(y2-y1);
}


// ============================================================
// ERSTEN POSITIVEN MESSPUNKT FINDEN
// ============================================================

size_t FindFirstMeasurementPoint(
    const vector<double>& times,
    const vector<double>& lengths)
{
    for(size_t i=0; i<lengths.size(); ++i)
    {
        if(lengths[i] > 0.0)
            return i;
    }


    return lengths.size();
}


// ============================================================
// ZEITSHIFT BESTIMMEN
//
// Wir suchen einen Shift, sodass die Simulation möglichst gut
// auf die Messung im ersten FIT_RANGE_NS-Bereich passt.
//
// Hier wird zunächst nur der Anfangsbereich betrachtet.
// Der Shift wird anschließend auf die komplette Simulation
// angewendet.
// ============================================================

double DetermineTimeShift(
    const vector<double>& measurementTimes,
    const vector<double>& measurementLengths,
    const SimCurve& sim)
{
    size_t first =
        FindFirstMeasurementPoint(
            measurementTimes,
            measurementLengths);


    if(first >= measurementTimes.size())
        return 0.0;


    double measurementStart =
        measurementTimes[first];


    double fitEnd =
        measurementStart +
        FIT_RANGE_NS;


    double bestShift = 0.0;

    double bestChi2 =
        1e300;


    // Shift-Suchbereich
    //
    // Die Simulation wird um dt verschoben:
    //
    // t_shifted = t_sim + dt
    //

    const double SHIFT_MIN = -100.0;
    const double SHIFT_MAX =  100.0;
    const double SHIFT_STEP = 0.1;


    for(double shift=SHIFT_MIN;
        shift<=SHIFT_MAX;
        shift+=SHIFT_STEP)
    {
        double chi2 = 0.0;

        int n = 0;


        for(size_t i=first;
            i<measurementTimes.size();
            ++i)
        {
            double tMeas =
                measurementTimes[i];


            if(tMeas > fitEnd)
                break;


            double lMeas =
                measurementLengths[i];


            if(lMeas <= 0.0)
                continue;


            double tSim =
                tMeas-shift;


            if(tSim < sim.time.front() ||
               tSim > sim.time.back())
            {
                continue;
            }


            double lSim =
                Interpolate(
                    sim.time,
                    sim.length,
                    tSim);


            double diff =
                lSim-lMeas;


            chi2 += diff*diff;

            ++n;
        }


        if(n == 0)
            continue;


        chi2 /= double(n);


        if(chi2 < bestChi2)
        {
            bestChi2 = chi2;
            bestShift = shift;
        }
    }


    return bestShift;
}


// ============================================================
// VERBESSERTE KURVE ERZEUGEN
// ============================================================

vector<double> BuildImprovedCurve(
    const vector<double>& measurementTimes,
    const vector<double>& measurementLengths,
    const SimCurve& sim,
    double timeShift)
{
    vector<double> result;

    result.resize(
        measurementTimes.size());


    size_t first =
        FindFirstMeasurementPoint(
            measurementTimes,
            measurementLengths);


    double measurementStart =
        measurementTimes[first];


    double fitEnd =
        measurementStart +
        FIT_RANGE_NS;


    for(size_t i=0;
        i<measurementTimes.size();
        ++i)
    {
        double t =
            measurementTimes[i];


        // ----------------------------------------------------
        // 1. Vor Beginn der Messkurve
        // ----------------------------------------------------

        if(i < first)
        {
            result[i] = 0.0;
            continue;
        }


        // ----------------------------------------------------
        // 2. Anpassungsbereich
        //
        // Hier verwenden wir die Messung.
        // ----------------------------------------------------

        if(t <= fitEnd)
        {
            result[i] =
                measurementLengths[i];

            continue;
        }


        // ----------------------------------------------------
        // 3. DANACH:
        //
        // ausschliesslich die verschobene Simulation.
        //
        // Ganz wichtig:
        // Keine Begrenzung auf den Mess-Maximalwert!
        // ----------------------------------------------------

        double tSim =
            t-timeShift;


        if(tSim <= sim.time.front())
        {
            result[i] =
                sim.length.front();
        }
        else if(tSim >= sim.time.back())
        {
            result[i] =
                sim.length.back();
        }
        else
        {
            result[i] =
                Interpolate(
                    sim.time,
                    sim.length,
                    tSim);
        }
    }


    return result;
}


// ============================================================
// AUSGABE
// ============================================================

bool WriteMeasurement(
    const char* filename,
    const Measurement& measurement,
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
        out << "   "
            << fixed
            << setprecision(6)
            << phi;
    }


    out << "\n";


    // --------------------------------------------------------
    // Daten
    // --------------------------------------------------------

    out << setprecision(10);


    for(size_t it=0;
        it<measurement.times.size();
        ++it)
    {
        out << fixed
            << setprecision(1)
            << measurement.times[it];


        for(size_t iphi=0;
            iphi<measurement.phis.size();
            ++iphi)
        {
            out << "   "
                << setprecision(10)
                << result[iphi][it];
        }


        out << "\n";
    }


    return true;
}


// ============================================================
// HAUPTFUNKTION
// ============================================================

void ImproveDriftTable()
{
    cout << endl;
    cout << "========================================" << endl;
    cout << " Improve Drift Table" << endl;
    cout << "========================================" << endl;


    // --------------------------------------------------------
    // 1. Messung
    // --------------------------------------------------------

    Measurement measurement;


    if(!ReadMeasurement(
           INPUT_MEASUREMENT,
           measurement))
    {
        return;
    }


    // --------------------------------------------------------
    // 2. Simulation
    // --------------------------------------------------------

    vector<SimCurve> simulations;


    if(!ReadSimulation(
           INPUT_SIMULATION,
           simulations))
    {
        return;
    }


    // --------------------------------------------------------
    // Ergebnis
    // --------------------------------------------------------

    vector<vector<double>> result;

    result.resize(
        measurement.phis.size());


    // --------------------------------------------------------
    // Für jeden Messwinkel
    // --------------------------------------------------------

    for(size_t iphi=0;
        iphi<measurement.phis.size();
        ++iphi)
    {
        double phi =
            measurement.phis[iphi];


        cout << endl;
        cout << "----------------------------------------"
             << endl;

        cout << "Phi = "
             << phi
             << " deg"
             << endl;


        // ----------------------------------------------------
        // passenden Simulationswinkel suchen
        // ----------------------------------------------------

        int isim =
            FindSimulationPhi(
                phi,
                simulations);


        if(isim < 0)
        {
            cerr << "  KEINE Simulation innerhalb "
                 << PHI_TOLERANCE
                 << " deg gefunden!"
                 << endl;

            // Fallback: Messkurve unverändert
            result[iphi] =
                measurement.lengths[iphi];

            continue;
        }


        const SimCurve& sim =
            simulations[isim];


        cout << "  Simulation Phi = "
             << sim.phi
             << " deg"
             << endl;

        cout << "  Delta Phi = "
             << std::abs(sim.phi-phi)
             << " deg"
             << endl;


        // ----------------------------------------------------
        // Zeitshift
        // ----------------------------------------------------

        double timeShift =
            DetermineTimeShift(
                measurement.times,
                measurement.lengths[iphi],
                sim);


        cout << "  Time shift = "
             << timeShift
             << " ns"
             << endl;


        // ----------------------------------------------------
        // verbesserte Kurve
        // ----------------------------------------------------

        result[iphi] =
            BuildImprovedCurve(
                measurement.times,
                measurement.lengths[iphi],
                sim,
                timeShift);
    }


    // --------------------------------------------------------
    // Ausgabe
    // --------------------------------------------------------

    if(!WriteMeasurement(
           OUTPUT_FILE,
           measurement,
           result))
    {
        return;
    }


    cout << endl;
    cout << "========================================" << endl;
    cout << "Fertig!" << endl;
    cout << "Ausgabe: "
         << OUTPUT_FILE
         << endl;
    cout << "========================================" << endl;
}
