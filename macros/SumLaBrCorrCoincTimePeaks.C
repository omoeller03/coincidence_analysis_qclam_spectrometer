#include "addmultiplehistos.C"



void SumLaBrCorrCoincTimePeaks()
{
	const char* hists[] =
	{
		"Time_LaBr0_vs_MeanSzintCorr",
		"Time_LaBr1_vs_MeanSzintCorr",
		"Time_LaBr2_vs_MeanSzintCorr",
		"Time_LaBr3_vs_MeanSzintCorr",
		"Time_LaBr4_vs_MeanSzintCorr",
		"Time_LaBr5_vs_MeanSzintCorr",
		"Time_LaBr6_vs_MeanSzintCorr",
		"Time_LaBr7_vs_MeanSzintCorr",
		"Time_LaBr8_vs_MeanSzintCorr"
	};


	addmultiplehistos(9,hists,1.,kTRUE,"CoincTimePeak_LaBrSum","Summe Koinzidenzzeit-Peaks");
}
