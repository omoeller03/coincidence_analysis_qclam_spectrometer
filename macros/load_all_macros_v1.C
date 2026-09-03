// ============================================================
// load_all_macros.C
//
// Lädt alle Go4/ROOT-Macros einmalig in den Cling-Interpreter.
// Danach können die Funktionen direkt aufgerufen werden:
//
//   fit_spectrum();
//   fit_spectrum_auto();
//   addhistos(...);
//   ...
//
// Einmal:
//
//   .L macros/load_all_macros.C
//
// ============================================================

void load_all_macros()
{
	//gROOT->ProcessLine(".L macros/addhistos.C");
	//gROOT->ProcessLine(".L macros/addmultiplehistos.C");

	gROOT->ProcessLine(".L macros/CopyAndClearTimePeakHist.C");

	gROOT->ProcessLine(".L macros/fit_spectrum.C");
	gROOT->ProcessLine(".L macros/fit_spectrum_auto.C");

	gROOT->ProcessLine(".L macros/SubtractPromptBkgrdMatrices.C");
	gROOT->ProcessLine(".L macros/SubtractMultiplePromptBkgrdMatrices.C");

	gROOT->ProcessLine(".L macros/SumEEPGMatrices.C");
	gROOT->ProcessLine(".L macros/SumEEPGMatricesPrompt.C");

	gROOT->ProcessLine(".L macros/SumEnergyShiftLaBr.C");
	gROOT->ProcessLine(".L macros/SumLaBrCorrCoincTimePeaks.C");

	gROOT->ProcessLine(".L macros/TrackEventViewer.C");

	std::cout << "\n";
	std::cout << "========================================\n";
	std::cout << " All analysis macros loaded.\n";
	std::cout << "========================================\n";
}
