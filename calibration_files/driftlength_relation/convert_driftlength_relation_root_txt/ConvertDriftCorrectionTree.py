#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <stdexcept>


// ============================================================
// Parameters
// ============================================================

const char* INPUT_FILE =
    "Driftkorrekturen_DriftkammerGen_3_Ar_80.000000_CO2_20.000000_O2_0.000000_HV_-4200.000000_AnzahlSimDraehte_320_maxStepSizeDrift_0.000000_AnzahlSimulierterWinkel_31_AnzahlSimulierterDatenProWinkel_930553.root";

const char* OUTPUT_FILE =
    "driftlength_relation_from_tree.txt";

// Drift-time offset
const double DRIFTTIME_OFFSET = 40.0;   // ns

// Drift length conversion
// Tree: cm
// Output: mm
const double DRIFTLENGTH_SCALE = 10.0;


// ============================================================
// Main
// ============================================================

void ConvertDriftCorrectionTree()
{
    // --------------------------------------------------------
    // Open ROOT file
    // --------------------------------------------------------

    TFile* file = TFile::Open(INPUT_FILE, "READ");

    if (!file || file->IsZombie()) {
        std::cerr << "ERROR: Could not open input file:\n"
                  << INPUT_FILE << std::endl;
        return;
    }

    TTree* tree = nullptr;
    file->GetObject("DriftKorrTree", tree);

    if (!tree) {
        std::cerr << "ERROR: TTree 'DriftKorrTree' not found."
                  << std::endl;
        file->Close();
        return;
    }


    // --------------------------------------------------------
    // Branch variables
    // --------------------------------------------------------

    double trackAngleDeg = 0.0;
    double drifttime_MinDT = 0.0;
    double driftlength_Corrected = 0.0;

    tree->SetBranchAddress(
        "trackAngleDeg",
        &trackAngleDeg
    );

    tree->SetBranchAddress(
        "drifttime_MinDT",
        &drifttime_MinDT
    );

    tree->SetBranchAddress(
        "driftlength_Corrected",
        &driftlength_Corrected
    );


    // --------------------------------------------------------
    // Store data by angle
    // --------------------------------------------------------

    //
    // angle -> vector of (time, drift length)
    //
    std::map<
        double,
        std::vector<std::pair<double, double>>
    > data;


    const Long64_t nEntries = tree->GetEntries();

    std::cout << "Entries: " << nEntries << std::endl;


    // --------------------------------------------------------
    // Read tree
    // --------------------------------------------------------

    for (Long64_t i = 0; i < nEntries; ++i) {

        tree->GetEntry(i);

        // Apply drift-time offset
        const double time =
            drifttime_MinDT + DRIFTTIME_OFFSET;

        // Convert cm -> mm
        const double driftLength =
            driftlength_Corrected * DRIFTLENGTH_SCALE;

        data[trackAngleDeg].push_back(
            std::make_pair(time, driftLength)
        );
    }


    // --------------------------------------------------------
    // Sort data for each angle by drift time
    // --------------------------------------------------------

    for (auto& [angle, values] : data) {

        std::sort(
            values.begin(),
            values.end(),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            }
        );
    }


    // --------------------------------------------------------
    // Print information
    // --------------------------------------------------------

    std::cout << "\nFound "
              << data.size()
              << " different angles:\n\n";

    for (const auto& [angle, values] : data) {

        std::cout << "  phi = "
                  << std::fixed
                  << std::setprecision(2)
                  << angle
                  << " deg : "
                  << values.size()
                  << " points"
                  << std::endl;
    }


    // --------------------------------------------------------
    // Determine common time grid
    // --------------------------------------------------------

    //
    // We use the time values of the first angle as the
    // output time grid.
    //

    if (data.empty()) {
        std::cerr << "ERROR: No data found." << std::endl;
        file->Close();
        return;
    }

    const auto& firstAngleData =
        data.begin()->second;

    std::vector<double> times;

    for (const auto& [time, length] : firstAngleData) {
        times.push_back(time);
    }


    // --------------------------------------------------------
    // Check that all angles have the same number of points
    // --------------------------------------------------------

    for (const auto& [angle, values] : data) {

        if (values.size() != times.size()) {

            std::cerr
                << "WARNING: Angle "
                << angle
                << " has "
                << values.size()
                << " points, while first angle has "
                << times.size()
                << " points."
                << std::endl;
        }
    }


    // --------------------------------------------------------
    // Open output file
    // --------------------------------------------------------

    std::ofstream out(OUTPUT_FILE);

    if (!out.is_open()) {

        std::cerr
            << "ERROR: Could not open output file:\n"
            << OUTPUT_FILE
            << std::endl;

        file->Close();
        return;
    }


    // --------------------------------------------------------
    // Write header
    // --------------------------------------------------------

    out << "t(ns)";

    for (const auto& [angle, values] : data) {

        out << "\t"
            << std::fixed
            << std::setprecision(0)
            << angle;
    }

    out << "\n";


    // --------------------------------------------------------
    // Write table
    // --------------------------------------------------------

    //
    // Since every angle should contain the same time grid,
    // we can use the index to construct the table.
    //

    for (size_t i = 0; i < times.size(); ++i) {

        out << std::fixed
            << std::setprecision(1)
            << times[i];

        for (const auto& [angle, values] : data) {

            out << "\t"
                << std::fixed
                << std::setprecision(10)
                << values[i].second;
        }

        out << "\n";
    }


    // --------------------------------------------------------
    // Close
    // --------------------------------------------------------

    out.close();
    file->Close();


    std::cout << "\n========================================\n";
    std::cout << "Conversion finished.\n";
    std::cout << "Input :  " << INPUT_FILE << "\n";
    std::cout << "Output:  " << OUTPUT_FILE << "\n";
    std::cout << "Drift-time offset: "
              << DRIFTTIME_OFFSET
              << " ns\n";
    std::cout << "Drift-length scale: "
              << DRIFTLENGTH_SCALE
              << " (cm -> mm)\n";
    std::cout << "========================================\n";
}
