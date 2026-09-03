#include "addmultiplehistos.C"



void SumEEPGMatricesPrompt()
{
	const char* hists[] =
	{
		"EEPGMatrixPrompt_LaBr0",
		"EEPGMatrixPrompt_LaBr1",
		"EEPGMatrixPrompt_LaBr2",
		"EEPGMatrixPrompt_LaBr3",
		"EEPGMatrixPrompt_LaBr4",
		"EEPGMatrixPrompt_LaBr5",
		"EEPGMatrixPrompt_LaBr6",
		"EEPGMatrixPrompt_LaBr7",
		"EEPGMatrixPrompt_LaBr8"
	};


	addmultiplehistos(9,hists,1.,kTRUE,"EEPGMatrixPrompt_LaBrSum","(e,e'g)-Matrix Summe (Gate auf Koinzidenzzeit-Peak)");
}
