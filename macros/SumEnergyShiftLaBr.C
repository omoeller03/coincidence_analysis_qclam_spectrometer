#include "addmultiplehistos.C"



void SumEnergyShiftLaBr()
{
	const char* hists[] =
	{
		"LaBr0_EnergyVsTime",
		"LaBr1_EnergyVsTime",
		"LaBr2_EnergyVsTime",
		"LaBr3_EnergyVsTime",
		"LaBr4_EnergyVsTime",
		"LaBr5_EnergyVsTime",
		"LaBr6_EnergyVsTime",
		"LaBr7_EnergyVsTime",
		"LaBr8_EnergyVsTime"
	};


	addmultiplehistos(9,hists,1.,kTRUE,"EnergyVsTime_LaBrSum","Energie-Shift LaBr Summe");
}
