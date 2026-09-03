#include "addmultiplehistos.C"



void SumEEPGMatrices()
{
	const char* hists[] =
	{
		"EEPGMatrix_LaBr0",
		"EEPGMatrix_LaBr1",
		"EEPGMatrix_LaBr2",
		"EEPGMatrix_LaBr3",
		"EEPGMatrix_LaBr4",
		"EEPGMatrix_LaBr5",
		"EEPGMatrix_LaBr6",
		"EEPGMatrix_LaBr7",
		"EEPGMatrix_LaBr8"
	};


	addmultiplehistos(9,hists,1.,kTRUE,"EEPGMatrix_LaBrSum","(e,e'g)-Matrix Summe");
}
