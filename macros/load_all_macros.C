#include <TROOT.h>
#include <TSystem.h>

void load_all_macros()
{
	// Verzeichnis, in dem load_all_macros.C liegt
	TString macroDir = gSystem->DirName(__FILE__);

	// Falls __FILE__ nur "load_all_macros.C" enthält,
	// liegt die Datei im aktuellen Verzeichnis.
	if(macroDir == ".")
	{ 
		macroDir = gSystem->WorkingDirectory();
	}

	gROOT->ProcessLine(Form(".L %s/CopyAndClearTimePeakHist.C", macroDir.Data()));

	gROOT->ProcessLine(Form(".L %s/fit_spectrum.C", macroDir.Data()));
	gROOT->ProcessLine(Form(".L %s/fit_spectrum_auto.C", macroDir.Data()));

	gROOT->ProcessLine(Form(".L %s/SubtractPromptBkgrdMatrices.C", macroDir.Data()));
	gROOT->ProcessLine(Form(".L %s/SubtractMultiplePromptBkgrdMatrices.C", macroDir.Data()));

	gROOT->ProcessLine(Form(".L %s/SumEEPGMatrices.C", macroDir.Data()));
	gROOT->ProcessLine(Form(".L %s/SumEEPGMatricesPrompt.C", macroDir.Data()));

	gROOT->ProcessLine(Form(".L %s/SumEnergyShiftLaBr.C", macroDir.Data()));
	gROOT->ProcessLine(Form(".L %s/SumLaBrCorrCoincTimePeaks.C", macroDir.Data()));

	gROOT->ProcessLine(Form(".L %s/TrackEventViewer.C", macroDir.Data()));

	std::cout << "\n";
	std::cout << "========================================\n";
	std::cout << " All analysis macros loaded.\n";
	std::cout << " Macro directory: " << macroDir << "\n";
	std::cout << "========================================\n";
}
