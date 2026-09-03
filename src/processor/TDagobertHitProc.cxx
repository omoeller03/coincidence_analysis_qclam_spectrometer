#include "processor/TDagobertHitProc.h"


#include "evt/TQClamDagobertRawEvent.h"
#include "evt/TDagobertHitEvent.h"

#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"

#include "TGo4RollingGraph.h"
#include "TGo4Picture.h"
#include "TGo4EventErrorException.h"
#include "TGo4EventEndException.h"
#include "TGo4Log.h"


#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>


/** Parameter */
#include "parameter/TQClamTcpParameter.h"
#include "parameter/TDagobertCalibrationParameter.h"
#include "parameter/TQClamHistogramParameter.h"

	
ClassImp(TDagobertHitProc)


TDagobertHitProc::TDagobertHitProc()
	: TGo4EventProcessor("DagobertHitProc"),
	fbTcpIsOpen(kFALSE),
	fTcpSocket(-1)
{}
TDagobertHitProc::TDagobertHitProc(const char* name)
	: TGo4EventProcessor(name),
	fbTcpIsOpen(kFALSE),
	fTcpSocket(-1)
{
	TGo4Log::Info("[TDagobertHitProc] Create %s",name);


	/** Parameter initialisieren */
	fTcpPar = (TQClamTcpParameter*) GetParameter("TcpParam");
	fDagoCaliPar = (TDagobertCalibrationParameter*) GetParameter("DagoCalibParam");
	fHistoPar = (TQClamHistogramParameter*) GetParameter("HistoParam");



	/** -- Histogramme initialisieren -- */
	/** unkalibrierte energien */
	fHRawEnergyLaBr0  = MakeTH1('D', "RawEnergy_ch0",  "LaBr 0 Energie unkalibriert (Tower 1, unten)",  40000, 0, 400000);
	fHRawEnergyLaBr0->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr0->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr0->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr0->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr1  = MakeTH1('D', "RawEnergy_ch1",  "LaBr 1 Energie unkalibriert (Tower 1, mitte)",  40000, 0, 400000);
	fHRawEnergyLaBr1->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr1->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr1->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr1->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr2  = MakeTH1('D', "RawEnergy_ch2",  "LaBr 2 Energie unkalibriert (Tower 1, oben)",  40000, 0, 400000);
	fHRawEnergyLaBr2->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr2->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr2->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr2->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr3  = MakeTH1('D', "RawEnergy_ch3",  "LaBr 3 Energie unkalibriert (Tower 2, unten)",  40000, 0, 400000);
	fHRawEnergyLaBr3->GetXaxis()->SetTitle("Kanal");

	fHRawEnergyLaBr3->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr3->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr3->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr4  = MakeTH1('D', "RawEnergy_ch4",  "LaBr 4 Energie unkalibriert (Tower 2, mitte)",  40000, 0, 400000);
	fHRawEnergyLaBr4->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr4->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr4->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr4->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr5  = MakeTH1('D', "RawEnergy_ch5",  "LaBr 5 Energie unkalibriert (Tower 2, oben)",  40000, 0, 400000);
	fHRawEnergyLaBr5->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr5->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr5->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr5->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr6  = MakeTH1('D', "RawEnergy_ch6",  "LaBr 6 Energie unkalibriert (Tower 3, unten)",  40000, 0, 400000);
	fHRawEnergyLaBr6->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr6->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr6->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr6->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr7  = MakeTH1('D', "RawEnergy_ch7",  "LaBr 7 Energie unkalibriert (Tower 3, mitte)",  40000, 0, 400000);
	fHRawEnergyLaBr7->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr7->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr7->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr7->GetYaxis()->CenterTitle();
	fHRawEnergyLaBr8  = MakeTH1('D', "RawEnergy_ch8",  "LaBr 8 Energie unkalibriert (Tower 3, oben)",  40000, 0, 400000);
	fHRawEnergyLaBr8->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBr8->GetXaxis()->CenterTitle();
	fHRawEnergyLaBr8->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBr8->GetYaxis()->CenterTitle();
	fHRawEnergyLaBrForward = MakeTH1('D', "RawEnergy_ForwardLaBr", "Vorwaerts-LaBr Energie unkalibriert", 4000, 0, 40000);
	fHRawEnergyLaBrForward->GetXaxis()->SetTitle("Kanal");
	fHRawEnergyLaBrForward->GetXaxis()->CenterTitle();
	fHRawEnergyLaBrForward->GetYaxis()->SetTitle("Haeufigkeit");
	fHRawEnergyLaBrForward->GetYaxis()->CenterTitle();

	/** kalibrierte energien */
	fHCaliEnergyLaBr0  = MakeTH1('D', "CaliEnergy_ch0",  "LaBr 0 Energie kalibriert (Tower 1, unten)",  1000,0,10);
	fHCaliEnergyLaBr0->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr0->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr0->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr0->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr1  = MakeTH1('D', "CaliEnergy_ch1",  "LaBr 1 Energie kalibriert (Tower 1, mitte)",  1000, 0, 10);
	fHCaliEnergyLaBr1->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr1->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr1->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr1->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr2  = MakeTH1('D', "CaliEnergy_ch2",  "LaBr 2 Energie kalibriert (Tower 1, oben)",  1000, 0, 10);
	fHCaliEnergyLaBr2->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr2->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr2->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr2->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr3  = MakeTH1('D', "CaliEnergy_ch3",  "LaBr 3 Energie kalibriert (Tower 2, unten)",  1000, 0, 10);
	fHCaliEnergyLaBr3->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr3->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr3->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr3->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr4  = MakeTH1('D', "CaliEnergy_ch4",  "LaBr 4 Energie kalibriert (Tower 2, mitte)",  1000, 0, 10);
	fHCaliEnergyLaBr4->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr4->GetXaxis()->CenterTitle();

	fHCaliEnergyLaBr4->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr4->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr5  = MakeTH1('D', "CaliEnergy_ch5",  "LaBr 5 Energie kalibriert (Tower 2, oben)",  1000, 0, 10);
	fHCaliEnergyLaBr5->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr5->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr5->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr5->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr6  = MakeTH1('D', "CaliEnergy_ch6",  "LaBr 6 Energie kalibriert (Tower 3, unten)",  1000, 0, 10);
	fHCaliEnergyLaBr6->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr6->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr6->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr6->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr7  = MakeTH1('D', "CaliEnergy_ch7",  "LaBr 7 Energie kalibriert (Tower 3, mitte)",  1000, 0, 10);
	fHCaliEnergyLaBr7->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr7->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr7->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr7->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBr8  = MakeTH1('D', "CaliEnergy_ch8",  "LaBr 8 Energie kalibriert (Tower 3, oben)",  1000, 0, 10);
	fHCaliEnergyLaBr8->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBr8->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBr8->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBr8->GetYaxis()->CenterTitle();
	fHCaliEnergyLaBrForward = MakeTH1('D', "CaliEnergy_ch10", "Vorwaerts-LaBr Energie kalibriert", 1000, 0, 10);
	fHCaliEnergyLaBrForward->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBrForward->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBrForward->GetYaxis()->SetTitle("Haeufigkeit/(10 keV)");
	fHCaliEnergyLaBrForward->GetYaxis()->CenterTitle();
	/** timing labr relativ zu Mean(SzUp,SzDown) */
	fHLaBr0VsMeanSzint = MakeTH1('D', "Time_LaBr0_vs_MeanSzint", "LaBr0 vs MeanSzint", 1000, -100, 100);
	fHLaBr0VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr0VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr0VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr0VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr1VsMeanSzint = MakeTH1('D', "Time_LaBr1_vs_MeanSzint", "LaBr1 vs MeanSzint", 1000, -100, 100);
	fHLaBr1VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr1VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr1VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr1VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr2VsMeanSzint = MakeTH1('D', "Time_LaBr2_vs_MeanSzint", "LaBr2 vs MeanSzint", 1000, -100, 100);
	fHLaBr2VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr2VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr2VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr2VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr3VsMeanSzint = MakeTH1('D', "Time_LaBr3_vs_MeanSzint", "LaBr3 vs MeanSzint", 1000, -100, 100);
	fHLaBr3VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr3VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr3VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr3VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr4VsMeanSzint = MakeTH1('D', "Time_LaBr4_vs_MeanSzint", "LaBr4 vs MeanSzint", 1000, -100, 100);
	fHLaBr4VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr4VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr4VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr4VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr5VsMeanSzint = MakeTH1('D', "Time_LaBr5_vs_MeanSzint", "LaBr5 vs MeanSzint", 1000, -100, 100);
	fHLaBr5VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr5VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr5VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");

	fHLaBr5VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr6VsMeanSzint = MakeTH1('D', "Time_LaBr6_vs_MeanSzint", "LaBr6 vs MeanSzint", 1000, -100, 100);
	fHLaBr6VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr6VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr6VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr6VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr7VsMeanSzint = MakeTH1('D', "Time_LaBr7_vs_MeanSzint", "LaBr7 vs MeanSzint", 1000, -100, 100);
	fHLaBr7VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr7VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr7VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr7VsMeanSzint->GetYaxis()->CenterTitle();
	fHLaBr8VsMeanSzint = MakeTH1('D', "Time_LaBr8_vs_MeanSzint", "LaBr8 vs MeanSzint", 1000, -100, 100);
	fHLaBr8VsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr8VsMeanSzint->GetXaxis()->CenterTitle();
	fHLaBr8VsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr8VsMeanSzint->GetYaxis()->CenterTitle();
	/** timing labr relativ zu ODER Koinzidenz LaBr */
	fHLaBr0VsCoinc = MakeTH1('D', "Time_LaBr0_vs_Coinc", "LaBr0 vs Coinc", 1000, -100, 100);
	fHLaBr0VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr0VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr0VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr0VsCoinc->GetYaxis()->CenterTitle();
	fHLaBr1VsCoinc = MakeTH1('D', "Time_LaBr1_vs_Coinc", "LaBr1 vs Coinc", 1000, -100, 100);
	fHLaBr1VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr1VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr1VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr1VsCoinc->GetYaxis()->CenterTitle();
	fHLaBr2VsCoinc = MakeTH1('D', "Time_LaBr2_vs_Coinc", "LaBr2 vs Coinc", 1000, -100, 100);
	fHLaBr2VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr2VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr2VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr2VsCoinc->GetYaxis()->CenterTitle();
	fHLaBr3VsCoinc = MakeTH1('D', "Time_LaBr3_vs_Coinc", "LaBr3 vs Coinc", 1000, -100, 100);
	fHLaBr3VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr3VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr3VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr3VsCoinc->GetYaxis()->CenterTitle();
	fHLaBr4VsCoinc = MakeTH1('D', "Time_LaBr4_vs_Coinc", "LaBr4 vs Coinc", 1000, -100, 100);
	fHLaBr4VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr4VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr4VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr4VsCoinc->GetYaxis()->CenterTitle();
	fHLaBr5VsCoinc = MakeTH1('D', "Time_LaBr5_vs_Coinc", "LaBr5 vs Coinc", 1000, -100, 100);
	fHLaBr5VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr5VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr5VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr5VsCoinc->GetYaxis()->CenterTitle();
	fHLaBr6VsCoinc = MakeTH1('D', "Time_LaBr6_vs_Coinc", "LaBr6 vs Coinc", 1000, -100, 100);
	fHLaBr6VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr6VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr6VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr6VsCoinc->GetYaxis()->CenterTitle();
	fHLaBr7VsCoinc = MakeTH1('D', "Time_LaBr7_vs_Coinc", "LaBr7 vs Coinc", 1000, -100, 100);
	fHLaBr7VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr7VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr7VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr7VsCoinc->GetYaxis()->CenterTitle();

	fHLaBr8VsCoinc = MakeTH1('D', "Time_LaBr8_vs_Coinc", "LaBr8 vs Coinc", 1000, -100, 100);
	fHLaBr8VsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBr8VsCoinc->GetXaxis()->CenterTitle();
	fHLaBr8VsCoinc->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBr8VsCoinc->GetYaxis()->CenterTitle();
	/** timing vorwaerts-detektoren */
	fHForwardLaBrVsMeanSzint = MakeTH1('D',
			"Time_ForwardLaBr_vs_MeanSzint", "Forward LaBr vs MeanSzint",
			2000, -2000, 2000);
	fHForwardLaBrVsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHForwardLaBrVsMeanSzint->GetXaxis()->CenterTitle();
	fHForwardLaBrVsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit");
	fHForwardLaBrVsMeanSzint->GetYaxis()->CenterTitle();
	fHForwardLaBrVsCoinc = MakeTH1('D',
			"Time_ForwardLaBr_vs_Coinc", "Forward LaBr vs Coinc",
			2000, -2000, 2000);
	fHForwardLaBrVsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHForwardLaBrVsCoinc->GetXaxis()->CenterTitle();
	fHForwardLaBrVsCoinc->GetYaxis()->SetTitle("Haeufigkeit");
	fHForwardLaBrVsCoinc->GetYaxis()->CenterTitle();
	fHCherenkovVsMeanSzint = MakeTH1('D',
			"Time_Cherenkov_vs_MeanSzint", "Cherenkov vs MeanSzint",
			2000, -2000, 2000);
	fHCherenkovVsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHCherenkovVsMeanSzint->GetXaxis()->CenterTitle();
	fHCherenkovVsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit");
	fHCherenkovVsMeanSzint->GetYaxis()->CenterTitle();
	fHCherenkovVsCoinc = MakeTH1('D',
			"Time_Cherenkov_vs_Coinc", "Cherenkov vs Coinc",
			2000, -2000, 2000);
	fHCherenkovVsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHCherenkovVsCoinc->GetXaxis()->CenterTitle();
	fHCherenkovVsCoinc->GetYaxis()->SetTitle("Haeufigkeit");
	fHCherenkovVsCoinc->GetYaxis()->CenterTitle();
	fHPlasticVsMeanSzint = MakeTH1('D',
			"Time_Plastic_vs_MeanSzint", "Plastic vs MeanSzint",
			2000, -2000, 2000);
	fHPlasticVsMeanSzint->GetXaxis()->SetTitle("Zeit/ns");
	fHPlasticVsMeanSzint->GetXaxis()->CenterTitle();
	fHPlasticVsMeanSzint->GetYaxis()->SetTitle("Haeufigkeit");
	fHPlasticVsMeanSzint->GetYaxis()->CenterTitle();
	fHPlasticVsCoinc = MakeTH1('D',
			"Time_Plastic_vs_Coinc", "Plastic vs Coinc",
			2000, -2000, 2000);
	fHPlasticVsCoinc->GetXaxis()->SetTitle("Zeit/ns");
	fHPlasticVsCoinc->GetXaxis()->CenterTitle();
	fHPlasticVsCoinc->GetYaxis()->SetTitle("Haeufigkeit");
	fHPlasticVsCoinc->GetYaxis()->CenterTitle();
	/** Energy-Shifts: Energie Vs Zeit */
	fHLaBr0EnergyVsTime = MakeTH2('D',
			"LaBr0_EnergyVsTime", "LaBr0 Energy vs Time",

			144, 0, 1440,     // 24h in Sekunden
			4000, 0, 40000);    // Energie
	fHLaBr0EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr0EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr0EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr0EnergyVsTime->GetYaxis()->CenterTitle();
	fHLaBr1EnergyVsTime = MakeTH2('D',
			"LaBr1_EnergyVsTime", "LaBr1 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr1EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr1EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr1EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr1EnergyVsTime->GetYaxis()->CenterTitle();
	fHLaBr2EnergyVsTime = MakeTH2('D',
			"LaBr2_EnergyVsTime", "LaBr2 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr2EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr2EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr2EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr2EnergyVsTime->GetYaxis()->CenterTitle();
	fHLaBr3EnergyVsTime = MakeTH2('D',
			"LaBr3_EnergyVsTime", "LaBr3 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr3EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr3EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr3EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr3EnergyVsTime->GetYaxis()->CenterTitle();
	fHLaBr4EnergyVsTime = MakeTH2('D',
			"LaBr4_EnergyVsTime", "LaBr4 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr4EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr4EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr4EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr4EnergyVsTime->GetYaxis()->CenterTitle();
	fHLaBr5EnergyVsTime = MakeTH2('D',
			"LaBr5_EnergyVsTime", "LaBr5 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr5EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr5EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr5EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr5EnergyVsTime->GetYaxis()->CenterTitle();
	fHLaBr6EnergyVsTime = MakeTH2('D',
			"LaBr6_EnergyVsTime", "LaBr6 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr6EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr6EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr6EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr6EnergyVsTime->GetYaxis()->CenterTitle();
	fHLaBr7EnergyVsTime = MakeTH2('D',
			"LaBr7_EnergyVsTime", "LaBr7 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr7EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr7EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr7EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr7EnergyVsTime->GetYaxis()->CenterTitle();

	fHLaBr8EnergyVsTime = MakeTH2('D',
			"LaBr8_EnergyVsTime", "LaBr8 Energy vs Time",
			144, 0, 1440,
			4000, 0, 40000);
	fHLaBr8EnergyVsTime->GetXaxis()->SetTitle("Zeit/Min");
	fHLaBr8EnergyVsTime->GetXaxis()->CenterTitle();
	fHLaBr8EnergyVsTime->GetYaxis()->SetTitle("Energie-Kanal");
	fHLaBr8EnergyVsTime->GetYaxis()->CenterTitle();

	/** Summen Histogramme */
	fHCaliEnergyLaBrSum = MakeTH1('D',"CaliEnergySum","Energie kalibriert: Summe LaBr 0-8",
			10000,0,10);
	fHCaliEnergyLaBrSum->GetXaxis()->SetTitle("Energie/MeV");
	fHCaliEnergyLaBrSum->GetXaxis()->CenterTitle();
	fHCaliEnergyLaBrSum->GetYaxis()->SetTitle("Haeufigkeit/(1keV)");
	fHCaliEnergyLaBrSum->GetYaxis()->CenterTitle();

	fHLaBrVsMeanSzintSum = MakeTH1('D',"Time_LaBrSum_vs_MeanSzint","Summe LaBr vs MeanSzint",
			4000,-2000,2000);
	fHLaBrVsMeanSzintSum->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBrVsMeanSzintSum->GetXaxis()->CenterTitle();
	fHLaBrVsMeanSzintSum->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBrVsMeanSzintSum->GetYaxis()->CenterTitle();

	fHLaBrVsCoincSum = MakeTH1('D',"Time_LaBrSum_vs_Coinc","Summe LaBr vs Coinc",
			4000,-2000,2000);
	fHLaBrVsCoincSum->GetXaxis()->SetTitle("Zeit/ns");
	fHLaBrVsCoincSum->GetXaxis()->CenterTitle();
	fHLaBrVsCoincSum->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
	fHLaBrVsCoincSum->GetYaxis()->CenterTitle();




	/** --- TGraphen --- */
	// LaBr 0
	fGLaBr0Accepted = new TGraph();
	fGLaBr0Accepted->SetName("LaBr0_Accepted_Graph");
	fGLaBr0Accepted->SetTitle("LaBr0 Accepted Rate");
	fGLaBr0Accepted->SetLineColor(kGray);
	fGLaBr0Accepted->SetLineWidth(2);
	fGLaBr0Accepted->SetLineStyle(1);
	fGLaBr0Accepted->SetMarkerStyle(0);
	fGLaBr0Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr0Accepted->GetXaxis()->CenterTitle();
	fGLaBr0Accepted->GetYaxis()->SetTitle("Rate in 1/s");
	fGLaBr0Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr0Accepted);

	fGLaBr0Deadtime = new TGraph();
	fGLaBr0Deadtime->SetName("LaBr0_Deadtime_Graph");
	fGLaBr0Deadtime->SetTitle("LaBr0 Deadtime");
	fGLaBr0Deadtime->SetLineColor(kGray);
	fGLaBr0Deadtime->SetLineWidth(2);
	fGLaBr0Deadtime->SetLineStyle(1);
	fGLaBr0Deadtime->SetMarkerStyle(0);
	fGLaBr0Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr0Deadtime->GetXaxis()->CenterTitle();
	fGLaBr0Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr0Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr0Deadtime);

	fGLaBr0Raw = new TGraph();
	fGLaBr0Raw->SetName("LaBr0_Raw_Graph");
	fGLaBr0Raw->SetTitle("LaBr0 Raw Rate");
	fGLaBr0Raw->SetLineColor(kGray);
	fGLaBr0Raw->SetLineWidth(2);
	fGLaBr0Raw->SetLineStyle(1);
	fGLaBr0Raw->SetMarkerStyle(0);
	fGLaBr0Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr0Raw->GetXaxis()->CenterTitle();
	fGLaBr0Raw->GetYaxis()->SetTitle("Rate in 1/s");
	fGLaBr0Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr0Raw);

	// LaBr 1
	fGLaBr1Accepted = new TGraph();
	fGLaBr1Accepted->SetName("LaBr1_Accepted_Graph");
	fGLaBr1Accepted->SetTitle("LaBr1 Accepted Rate");
	fGLaBr1Accepted->SetLineColor(kCyan);
	fGLaBr1Accepted->SetLineWidth(2);
	fGLaBr1Accepted->SetLineStyle(1);
	fGLaBr1Accepted->SetMarkerStyle(0);
	fGLaBr1Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr1Accepted->GetXaxis()->CenterTitle();
	fGLaBr1Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr1Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr1Accepted);

	fGLaBr1Deadtime = new TGraph();
	fGLaBr1Deadtime->SetName("LaBr1_Deadtime_Graph");
	fGLaBr1Deadtime->SetTitle("LaBr1 Deadtime");
	fGLaBr1Deadtime->SetLineColor(kCyan);
	fGLaBr1Deadtime->SetLineWidth(2);
	fGLaBr1Deadtime->SetLineStyle(1);
	fGLaBr1Deadtime->SetMarkerStyle(0);
	fGLaBr1Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr1Deadtime->GetXaxis()->CenterTitle();
	fGLaBr1Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr1Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr1Deadtime);

	fGLaBr1Raw = new TGraph();
	fGLaBr1Raw->SetName("LaBr1_Raw_Graph");
	fGLaBr1Raw->SetTitle("LaBr1 Raw Rate");
	fGLaBr1Raw->SetLineColor(kCyan);
	fGLaBr1Raw->SetLineWidth(2);
	fGLaBr1Raw->SetLineStyle(1);
	fGLaBr1Raw->SetMarkerStyle(0);
	fGLaBr1Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr1Raw->GetXaxis()->CenterTitle();
	fGLaBr1Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr1Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr1Raw);

	// LaBr 2
	fGLaBr2Accepted = new TGraph();
	fGLaBr2Accepted->SetName("LaBr2_Accepted_Graph");
	fGLaBr2Accepted->SetTitle("LaBr2 Accepted Rate");
	fGLaBr2Accepted->SetLineColor(kBlue);
	fGLaBr2Accepted->SetLineWidth(2);
	fGLaBr2Accepted->SetLineStyle(1);
	fGLaBr2Accepted->SetMarkerStyle(0);
	fGLaBr2Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr2Accepted->GetXaxis()->CenterTitle();
	fGLaBr2Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr2Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr2Accepted);

	fGLaBr2Deadtime = new TGraph();
	fGLaBr2Deadtime->SetName("LaBr2_Deadtime_Graph");
	fGLaBr2Deadtime->SetTitle("LaBr2 Deadtime");
	fGLaBr2Deadtime->SetLineColor(kBlue);
	fGLaBr2Deadtime->SetLineWidth(2);
	fGLaBr2Deadtime->SetLineStyle(1);
	fGLaBr2Deadtime->SetMarkerStyle(0);
	fGLaBr2Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr2Deadtime->GetXaxis()->CenterTitle();
	fGLaBr2Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr2Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr2Deadtime);

	fGLaBr2Raw = new TGraph();
	fGLaBr2Raw->SetName("LaBr2_Raw_Graph");
	fGLaBr2Raw->SetTitle("LaBr2 Raw Rate");
	fGLaBr2Raw->SetLineColor(kBlue);
	fGLaBr2Raw->SetLineWidth(2);
	fGLaBr2Raw->SetLineStyle(1);
	fGLaBr2Raw->SetMarkerStyle(0);
	fGLaBr2Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr2Raw->GetXaxis()->CenterTitle();
	fGLaBr2Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr2Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr2Raw);

	// LaBr 3
	fGLaBr3Accepted = new TGraph();
	fGLaBr3Accepted->SetName("LaBr3_Accepted_Graph");
	fGLaBr3Accepted->SetTitle("LaBr3 Accepted Rate");
	fGLaBr3Accepted->SetLineColor(kOrange);
	fGLaBr3Accepted->SetLineWidth(2);
	fGLaBr3Accepted->SetLineStyle(1);
	fGLaBr3Accepted->SetMarkerStyle(0);
	fGLaBr3Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr3Accepted->GetXaxis()->CenterTitle();
	fGLaBr3Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr3Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr3Accepted);

	fGLaBr3Deadtime = new TGraph();
	fGLaBr3Deadtime->SetName("LaBr3_Deadtime_Graph");
	fGLaBr3Deadtime->SetTitle("LaBr3 Deadtime");
	fGLaBr3Deadtime->SetLineColor(kOrange);
	fGLaBr3Deadtime->SetLineWidth(2);
	fGLaBr3Deadtime->SetLineStyle(1);
	fGLaBr3Deadtime->SetMarkerStyle(0);
	fGLaBr3Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr3Deadtime->GetXaxis()->CenterTitle();
	fGLaBr3Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr3Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr3Deadtime);

	fGLaBr3Raw = new TGraph();
	fGLaBr3Raw->SetName("LaBr3_Raw_Graph");
	fGLaBr3Raw->SetTitle("LaBr3 Raw Rate");
	fGLaBr3Raw->SetLineColor(kOrange);
	fGLaBr3Raw->SetLineWidth(2);
	fGLaBr3Raw->SetLineStyle(1);
	fGLaBr3Raw->SetMarkerStyle(0);
	fGLaBr3Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr3Raw->GetXaxis()->CenterTitle();
	fGLaBr3Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr3Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr3Raw);

	// LaBr 4
	fGLaBr4Accepted = new TGraph();
	fGLaBr4Accepted->SetName("LaBr4_Accepted_Graph");
	fGLaBr4Accepted->SetTitle("LaBr4 Accepted Rate");
	fGLaBr4Accepted->SetLineColor(kViolet);
	fGLaBr4Accepted->SetLineWidth(2);
	fGLaBr4Accepted->SetLineStyle(1);
	fGLaBr4Accepted->SetMarkerStyle(0);
	fGLaBr4Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr4Accepted->GetXaxis()->CenterTitle();
	fGLaBr4Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr4Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr4Accepted);

	fGLaBr4Deadtime = new TGraph();
	fGLaBr4Deadtime->SetName("LaBr4_Deadtime_Graph");
	fGLaBr4Deadtime->SetTitle("LaBr4 Deadtime");
	fGLaBr4Deadtime->SetLineColor(kViolet);
	fGLaBr4Deadtime->SetLineWidth(2);
	fGLaBr4Deadtime->SetLineStyle(1);
	fGLaBr4Deadtime->SetMarkerStyle(0);
	fGLaBr4Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr4Deadtime->GetXaxis()->CenterTitle();
	fGLaBr4Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr4Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr4Deadtime);

	fGLaBr4Raw = new TGraph();
	fGLaBr4Raw->SetName("LaBr4_Raw_Graph");
	fGLaBr4Raw->SetTitle("LaBr4 Raw Rate");
	fGLaBr4Raw->SetLineColor(kViolet);
	fGLaBr4Raw->SetLineWidth(2);
	fGLaBr4Raw->SetLineStyle(1);
	fGLaBr4Raw->SetMarkerStyle(0);
	fGLaBr4Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr4Raw->GetXaxis()->CenterTitle();
	fGLaBr4Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr4Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr4Raw);

	// LaBr 5
	fGLaBr5Accepted = new TGraph();
	fGLaBr5Accepted->SetName("LaBr5_Accepted_Graph");
	fGLaBr5Accepted->SetTitle("LaBr5 Accepted Rate");
	fGLaBr5Accepted->SetLineColor(kGreen);
	fGLaBr5Accepted->SetLineWidth(2);
	fGLaBr5Accepted->SetLineStyle(1);
	fGLaBr5Accepted->SetMarkerStyle(0);
	fGLaBr5Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr5Accepted->GetXaxis()->CenterTitle();
	fGLaBr5Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr5Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr5Accepted);

	fGLaBr5Deadtime = new TGraph();
	fGLaBr5Deadtime->SetName("LaBr5_Deadtime_Graph");
	fGLaBr5Deadtime->SetTitle("LaBr5 Deadtime");
	fGLaBr5Deadtime->SetLineColor(kGreen);
	fGLaBr5Deadtime->SetLineWidth(2);
	fGLaBr5Deadtime->SetLineStyle(1);
	fGLaBr5Deadtime->SetMarkerStyle(0);
	fGLaBr5Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr5Deadtime->GetXaxis()->CenterTitle();
	fGLaBr5Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr5Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr5Deadtime);

	fGLaBr5Raw = new TGraph();
	fGLaBr5Raw->SetName("LaBr5_Raw_Graph");
	fGLaBr5Raw->SetTitle("LaBr5 Raw Rate");
	fGLaBr5Raw->SetLineColor(kGreen);
	fGLaBr5Raw->SetLineWidth(2);
	fGLaBr5Raw->SetLineStyle(1);
	fGLaBr5Raw->SetMarkerStyle(0);
	fGLaBr5Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr5Raw->GetXaxis()->CenterTitle();
	fGLaBr5Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr5Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr5Raw);

	// LaBr 6
	fGLaBr6Accepted = new TGraph();
	fGLaBr6Accepted->SetName("LaBr6_Accepted_Graph");
	fGLaBr6Accepted->SetTitle("LaBr6 Accepted Rate");
	fGLaBr6Accepted->SetLineColor(kMagenta);
	fGLaBr6Accepted->SetLineWidth(2);
	fGLaBr6Accepted->SetLineStyle(1);
	fGLaBr6Accepted->SetMarkerStyle(0);
	fGLaBr6Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr6Accepted->GetXaxis()->CenterTitle();
	fGLaBr6Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr6Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr6Accepted);

	fGLaBr6Deadtime = new TGraph();
	fGLaBr6Deadtime->SetName("LaBr6_Deadtime_Graph");
	fGLaBr6Deadtime->SetTitle("LaBr6 Deadtime");
	fGLaBr6Deadtime->SetLineColor(kMagenta);
	fGLaBr6Deadtime->SetLineWidth(2);
	fGLaBr6Deadtime->SetLineStyle(1);
	fGLaBr6Deadtime->SetMarkerStyle(0);
	fGLaBr6Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr6Deadtime->GetXaxis()->CenterTitle();
	fGLaBr6Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr6Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr6Deadtime);

	fGLaBr6Raw = new TGraph();
	fGLaBr6Raw->SetName("LaBr6_Raw_Graph");
	fGLaBr6Raw->SetTitle("LaBr6 Raw Rate");
	fGLaBr6Raw->SetLineColor(kMagenta);
	fGLaBr6Raw->SetLineWidth(2);
	fGLaBr6Raw->SetLineStyle(1);
	fGLaBr6Raw->SetMarkerStyle(0);
	fGLaBr6Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr6Raw->GetXaxis()->CenterTitle();
	fGLaBr6Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr6Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr6Raw);

	// LaBr 7
	fGLaBr7Accepted = new TGraph();
	fGLaBr7Accepted->SetName("LaBr7_Accepted_Graph");
	fGLaBr7Accepted->SetTitle("LaBr7 Accepted Rate");
	fGLaBr7Accepted->SetLineColor(kRed);
	fGLaBr7Accepted->SetLineWidth(2);
	fGLaBr7Accepted->SetLineStyle(1);
	fGLaBr7Accepted->SetMarkerStyle(0);
	fGLaBr7Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr7Accepted->GetXaxis()->CenterTitle();
	fGLaBr7Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr7Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr7Accepted);

	fGLaBr7Deadtime = new TGraph();
	fGLaBr7Deadtime->SetName("LaBr7_Deadtime_Graph");
	fGLaBr7Deadtime->SetTitle("LaBr7 Deadtime");
	fGLaBr7Deadtime->SetLineColor(kRed);
	fGLaBr7Deadtime->SetLineWidth(2);
	fGLaBr7Deadtime->SetLineStyle(1);
	fGLaBr7Deadtime->SetMarkerStyle(0);
	fGLaBr7Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr7Deadtime->GetXaxis()->CenterTitle();
	fGLaBr7Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr7Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr7Deadtime);

	fGLaBr7Raw = new TGraph();
	fGLaBr7Raw->SetName("LaBr7_Raw_Graph");
	fGLaBr7Raw->SetTitle("LaBr7 Raw Rate");
	fGLaBr7Raw->SetLineColor(kRed);
	fGLaBr7Raw->SetLineWidth(2);
	fGLaBr7Raw->SetLineStyle(1);
	fGLaBr7Raw->SetMarkerStyle(0);
	fGLaBr7Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr7Raw->GetXaxis()->CenterTitle();
	fGLaBr7Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr7Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr7Raw);

	// LaBr 8
	fGLaBr8Accepted = new TGraph();
	fGLaBr8Accepted->SetName("LaBr8_Accepted_Graph");
	fGLaBr8Accepted->SetTitle("LaBr8 Accepted Rate");
	fGLaBr8Accepted->SetLineColor(kBlack);
	fGLaBr8Accepted->SetLineWidth(2);
	fGLaBr8Accepted->SetLineStyle(1);
	fGLaBr8Accepted->SetMarkerStyle(0);
	fGLaBr8Accepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr8Accepted->GetXaxis()->CenterTitle();
	fGLaBr8Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr8Accepted->GetYaxis()->CenterTitle();
	AddObject(fGLaBr8Accepted);
	
	fGLaBr8Deadtime = new TGraph();
	fGLaBr8Deadtime->SetName("LaBr8_Deadtime_Graph");
	fGLaBr8Deadtime->SetTitle("LaBr8 Deadtime");
	fGLaBr8Deadtime->SetLineColor(kBlack);
	fGLaBr8Deadtime->SetLineWidth(2);
	fGLaBr8Deadtime->SetLineStyle(1);
	fGLaBr8Deadtime->SetMarkerStyle(0);
	fGLaBr8Deadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr8Deadtime->GetXaxis()->CenterTitle();
	fGLaBr8Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGLaBr8Deadtime->GetYaxis()->CenterTitle();
	AddObject(fGLaBr8Deadtime);

	fGLaBr8Raw = new TGraph();
	fGLaBr8Raw->SetName("LaBr8_Raw_Graph");
	fGLaBr8Raw->SetTitle("LaBr8 Raw Rate");
	fGLaBr8Raw->SetLineColor(kBlack);
	fGLaBr8Raw->SetLineWidth(2);
	fGLaBr8Raw->SetLineStyle(1);
	fGLaBr8Raw->SetMarkerStyle(0);
	fGLaBr8Raw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGLaBr8Raw->GetXaxis()->CenterTitle();
	fGLaBr8Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGLaBr8Raw->GetYaxis()->CenterTitle();
	AddObject(fGLaBr8Raw);

	// Forward LaBr
	fGForwardLaBrAccepted = new TGraph();
	fGForwardLaBrAccepted->SetName("ForwardLaBr_Accepted_Graph");
	fGForwardLaBrAccepted->SetTitle("Forward LaBr Accepted");
	fGForwardLaBrAccepted->SetLineColor(kBlue);
	fGForwardLaBrAccepted->SetLineWidth(2);
	fGForwardLaBrAccepted->SetLineStyle(1);
	fGForwardLaBrAccepted->SetMarkerStyle(0);
	fGForwardLaBrAccepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGForwardLaBrAccepted->GetXaxis()->CenterTitle();
	fGForwardLaBrAccepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGForwardLaBrAccepted->GetYaxis()->CenterTitle();
	AddObject(fGForwardLaBrAccepted);

	fGForwardLaBrDeadtime = new TGraph();
	fGForwardLaBrDeadtime->SetName("ForwardLaBr_Deadtime_Graph");
	fGForwardLaBrDeadtime->SetTitle("Forward LaBr Deadtime");
	fGForwardLaBrDeadtime->SetLineColor(kBlue);
	fGForwardLaBrDeadtime->SetLineWidth(2);
	fGForwardLaBrDeadtime->SetLineStyle(1);
	fGForwardLaBrDeadtime->SetMarkerStyle(0);
	fGForwardLaBrDeadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGForwardLaBrDeadtime->GetXaxis()->CenterTitle();
	fGForwardLaBrDeadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGForwardLaBrDeadtime->GetYaxis()->CenterTitle();
	AddObject(fGForwardLaBrDeadtime);

	fGForwardLaBrRaw = new TGraph();
	fGForwardLaBrRaw->SetName("ForwardLaBr_Raw_Graph");
	fGForwardLaBrRaw->SetTitle("Forward LaBr Raw");
	fGForwardLaBrRaw->SetLineColor(kBlue);
	fGForwardLaBrRaw->SetLineWidth(2);
	fGForwardLaBrRaw->SetLineStyle(1);
	fGForwardLaBrRaw->SetMarkerStyle(0);
	fGForwardLaBrRaw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGForwardLaBrRaw->GetXaxis()->CenterTitle();
	fGForwardLaBrRaw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGForwardLaBrRaw->GetYaxis()->CenterTitle();
	AddObject(fGForwardLaBrRaw);

	// Cherenkov
	fGCherenkovAccepted = new TGraph();
	fGCherenkovAccepted->SetName("Cherenkov_Accepted_Graph");
	fGCherenkovAccepted->SetTitle("Cherenkov Accepted");
	fGCherenkovAccepted->SetLineColor(kMagenta);
	fGCherenkovAccepted->SetLineWidth(2);
	fGCherenkovAccepted->SetLineStyle(1);
	fGCherenkovAccepted->SetMarkerStyle(0);
	fGCherenkovAccepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGCherenkovAccepted->GetXaxis()->CenterTitle();
	fGCherenkovAccepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGCherenkovAccepted->GetYaxis()->CenterTitle();
	AddObject(fGCherenkovAccepted);

	fGCherenkovDeadtime = new TGraph();
	fGCherenkovDeadtime->SetName("Cherenkov_Deadtime_Graph");
	fGCherenkovDeadtime->SetTitle("Cherenkov Deadtime");
	fGCherenkovDeadtime->SetLineColor(kMagenta);
	fGCherenkovDeadtime->SetLineWidth(2);
	fGCherenkovDeadtime->SetLineStyle(1);
	fGCherenkovDeadtime->SetMarkerStyle(0);
	fGCherenkovDeadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGCherenkovDeadtime->GetXaxis()->CenterTitle();
	fGCherenkovDeadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGCherenkovDeadtime->GetYaxis()->CenterTitle();
	AddObject(fGCherenkovDeadtime);

	fGCherenkovRaw = new TGraph();
	fGCherenkovRaw->SetName("Cherenkov_Raw_Graph");
	fGCherenkovRaw->SetTitle("Cherenkov Raw");
	fGCherenkovRaw->SetLineColor(kMagenta);
	fGCherenkovRaw->SetLineWidth(2);
	fGCherenkovRaw->SetLineStyle(1);
	fGCherenkovRaw->SetMarkerStyle(0);
	fGCherenkovRaw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGCherenkovRaw->GetXaxis()->CenterTitle();
	fGCherenkovRaw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGCherenkovRaw->GetYaxis()->CenterTitle();
	AddObject(fGCherenkovRaw);

	// Plastic
	fGPlasticAccepted = new TGraph();
	fGPlasticAccepted->SetName("Plastic_Accepted_Graph");
	fGPlasticAccepted->SetTitle("Plastic Accepted");
	fGPlasticAccepted->SetLineColor(kGreen);
	fGPlasticAccepted->SetLineWidth(2);
	fGPlasticAccepted->SetLineStyle(1);
	fGPlasticAccepted->SetMarkerStyle(0);
	fGPlasticAccepted->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGPlasticAccepted->GetXaxis()->CenterTitle();
	fGPlasticAccepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGPlasticAccepted->GetYaxis()->CenterTitle();
	AddObject(fGPlasticAccepted);

	fGPlasticDeadtime = new TGraph();
	fGPlasticDeadtime->SetName("Plastic_Deadtime_Graph");
	fGPlasticDeadtime->SetTitle("Plastic Deadtime");
	fGPlasticDeadtime->SetLineColor(kGreen);
	fGPlasticDeadtime->SetLineWidth(2);
	fGPlasticDeadtime->SetLineStyle(1);
	fGPlasticDeadtime->SetMarkerStyle(0);
	fGPlasticDeadtime->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGPlasticDeadtime->GetXaxis()->CenterTitle();
	fGPlasticDeadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGPlasticDeadtime->GetYaxis()->CenterTitle();
	AddObject(fGPlasticDeadtime);

	fGPlasticRaw = new TGraph();
	fGPlasticRaw->SetName("Plastic_Raw_Graph");
	fGPlasticRaw->SetTitle("Plastic Raw");
	fGPlasticRaw->SetLineColor(kGreen);
	fGPlasticRaw->SetLineWidth(2);
	fGPlasticRaw->SetLineStyle(1);
	fGPlasticRaw->SetMarkerStyle(0);
	fGPlasticRaw->GetXaxis()->SetTitle("Zeit seit Start / Min");
	fGPlasticRaw->GetXaxis()->CenterTitle();
	fGPlasticRaw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGPlasticRaw->GetYaxis()->CenterTitle();
	AddObject(fGPlasticRaw);







	/** --- Rolling Graphen --- */
	// LaBr 0
	fGRLaBr0Accepted = MakeRollingGraph("LaBr0_Accepted", "LaBr0 Accepted Rate (Zoom)", 1000);
	fGRLaBr0Accepted->SetLineColor(kGray);
	fGRLaBr0Accepted->SetLineWidth(2);
	fGRLaBr0Accepted->SetLineStyle(1);
	fGRLaBr0Accepted->SetMarkerStyle(0);
	fGRLaBr0Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr0Accepted->GetXaxis()->CenterTitle();
	fGRLaBr0Accepted->GetYaxis()->SetTitle("Rate in 1/s");
	fGRLaBr0Accepted->GetYaxis()->CenterTitle();

	fGRLaBr0Deadtime = MakeRollingGraph("LaBr0_Deadtime", "LaBr0 Deadtime (Zoom)", 1000);
	fGRLaBr0Deadtime->SetLineColor(kGray);
	fGRLaBr0Deadtime->SetLineWidth(2);
	fGRLaBr0Deadtime->SetLineStyle(1);
	fGRLaBr0Deadtime->SetMarkerStyle(0);
	fGRLaBr0Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr0Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr0Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr0Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr0Raw = MakeRollingGraph("LaBr0_Raw", "LaBr0 Raw Rate (Zoom)", 1000);
	fGRLaBr0Raw->SetLineColor(kGray);
	fGRLaBr0Raw->SetLineWidth(2);
	fGRLaBr0Raw->SetLineStyle(1);
	fGRLaBr0Raw->SetMarkerStyle(0);
	fGRLaBr0Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr0Raw->GetXaxis()->CenterTitle();
	fGRLaBr0Raw->GetYaxis()->SetTitle("Rate in 1/s");
	fGRLaBr0Raw->GetYaxis()->CenterTitle();

	// LaBr 1
	fGRLaBr1Accepted = MakeRollingGraph("LaBr1_Accepted", "LaBr1 Accepted Rate (Zoom)", 1000);
	fGRLaBr1Accepted->SetLineColor(kCyan);
	fGRLaBr1Accepted->SetLineWidth(2);
	fGRLaBr1Accepted->SetLineStyle(1);
	fGRLaBr1Accepted->SetMarkerStyle(0);
	fGRLaBr1Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr1Accepted->GetXaxis()->CenterTitle();
	fGRLaBr1Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr1Accepted->GetYaxis()->CenterTitle();

	fGRLaBr1Deadtime = MakeRollingGraph("LaBr1_Deadtime", "LaBr1 Deadtime (Zoom)", 1000);
	fGRLaBr1Deadtime->SetLineColor(kCyan);
	fGRLaBr1Deadtime->SetLineWidth(2);
	fGRLaBr1Deadtime->SetLineStyle(1);
	fGRLaBr1Deadtime->SetMarkerStyle(0);
	fGRLaBr1Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr1Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr1Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr1Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr1Raw = MakeRollingGraph("LaBr1_Raw", "LaBr1 Raw Rate (Zoom)", 1000);
	fGRLaBr1Raw->SetLineColor(kCyan);
	fGRLaBr1Raw->SetLineWidth(2);
	fGRLaBr1Raw->SetLineStyle(1);
	fGRLaBr1Raw->SetMarkerStyle(0);
	fGRLaBr1Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr1Raw->GetXaxis()->CenterTitle();
	fGRLaBr1Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr1Raw->GetYaxis()->CenterTitle();

	// LaBr 2
	fGRLaBr2Accepted = MakeRollingGraph("LaBr2_Accepted", "LaBr2 Accepted Rate (Zoom)", 1000);
	fGRLaBr2Accepted->SetLineColor(kBlue);
	fGRLaBr2Accepted->SetLineWidth(2);
	fGRLaBr2Accepted->SetLineStyle(1);
	fGRLaBr2Accepted->SetMarkerStyle(0);
	fGRLaBr2Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr2Accepted->GetXaxis()->CenterTitle();
	fGRLaBr2Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr2Accepted->GetYaxis()->CenterTitle();

	fGRLaBr2Deadtime = MakeRollingGraph("LaBr2_Deadtime", "LaBr2 Deadtime (Zoom)", 1000);
	fGRLaBr2Deadtime->SetLineColor(kBlue);
	fGRLaBr2Deadtime->SetLineWidth(2);
	fGRLaBr2Deadtime->SetLineStyle(1);
	fGRLaBr2Deadtime->SetMarkerStyle(0);
	fGRLaBr2Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr2Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr2Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr2Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr2Raw = MakeRollingGraph("LaBr2_Raw", "LaBr2 Raw Rate (Zoom)", 1000);
	fGRLaBr2Raw->SetLineColor(kBlue);
	fGRLaBr2Raw->SetLineWidth(2);
	fGRLaBr2Raw->SetLineStyle(1);
	fGRLaBr2Raw->SetMarkerStyle(0);
	fGRLaBr2Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr2Raw->GetXaxis()->CenterTitle();
	fGRLaBr2Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr2Raw->GetYaxis()->CenterTitle();

	// LaBr 3
	fGRLaBr3Accepted = MakeRollingGraph("LaBr3_Accepted", "LaBr3 Accepted Rate (Zoom)", 1000);
	fGRLaBr3Accepted->SetLineColor(kOrange);
	fGRLaBr3Accepted->SetLineWidth(2);
	fGRLaBr3Accepted->SetLineStyle(1);
	fGRLaBr3Accepted->SetMarkerStyle(0);
	fGRLaBr3Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr3Accepted->GetXaxis()->CenterTitle();
	fGRLaBr3Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr3Accepted->GetYaxis()->CenterTitle();

	fGRLaBr3Deadtime = MakeRollingGraph("LaBr3_Deadtime", "LaBr3 Deadtime (Zoom)", 1000);
	fGRLaBr3Deadtime->SetLineColor(kOrange);
	fGRLaBr3Deadtime->SetLineWidth(2);
	fGRLaBr3Deadtime->SetLineStyle(1);
	fGRLaBr3Deadtime->SetMarkerStyle(0);
	fGRLaBr3Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr3Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr3Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr3Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr3Raw = MakeRollingGraph("LaBr3_Raw", "LaBr3 Raw Rate (Zoom)", 1000);
	fGRLaBr3Raw->SetLineColor(kOrange);
	fGRLaBr3Raw->SetLineWidth(2);
	fGRLaBr3Raw->SetLineStyle(1);
	fGRLaBr3Raw->SetMarkerStyle(0);
	fGRLaBr3Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr3Raw->GetXaxis()->CenterTitle();
	fGRLaBr3Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr3Raw->GetYaxis()->CenterTitle();

	// LaBr 4
	fGRLaBr4Accepted = MakeRollingGraph("LaBr4_Accepted", "LaBr4 Accepted Rate (Zoom)", 1000);
	fGRLaBr4Accepted->SetLineColor(kViolet);
	fGRLaBr4Accepted->SetLineWidth(2);
	fGRLaBr4Accepted->SetLineStyle(1);
	fGRLaBr4Accepted->SetMarkerStyle(0);
	fGRLaBr4Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr4Accepted->GetXaxis()->CenterTitle();
	fGRLaBr4Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr4Accepted->GetYaxis()->CenterTitle();

	fGRLaBr4Deadtime = MakeRollingGraph("LaBr4_Deadtime", "LaBr4 Deadtime (Zoom)", 1000);
	fGRLaBr4Deadtime->SetLineColor(kViolet);
	fGRLaBr4Deadtime->SetLineWidth(2);
	fGRLaBr4Deadtime->SetLineStyle(1);
	fGRLaBr4Deadtime->SetMarkerStyle(0);
	fGRLaBr4Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr4Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr4Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr4Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr4Raw = MakeRollingGraph("LaBr4_Raw", "LaBr4 Raw Rate (Zoom)", 1000);
	fGRLaBr4Raw->SetLineColor(kViolet);
	fGRLaBr4Raw->SetLineWidth(2);
	fGRLaBr4Raw->SetLineStyle(1);
	fGRLaBr4Raw->SetMarkerStyle(0);
	fGRLaBr4Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr4Raw->GetXaxis()->CenterTitle();
	fGRLaBr4Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr4Raw->GetYaxis()->CenterTitle();

	// LaBr 5
	fGRLaBr5Accepted = MakeRollingGraph("LaBr5_Accepted", "LaBr5 Accepted Rate (Zoom)", 1000);
	fGRLaBr5Accepted->SetLineColor(kGreen);
	fGRLaBr5Accepted->SetLineWidth(2);
	fGRLaBr5Accepted->SetLineStyle(1);
	fGRLaBr5Accepted->SetMarkerStyle(0);
	fGRLaBr5Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr5Accepted->GetXaxis()->CenterTitle();
	fGRLaBr5Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr5Accepted->GetYaxis()->CenterTitle();

	fGRLaBr5Deadtime = MakeRollingGraph("LaBr5_Deadtime", "LaBr5 Deadtime (Zoom)", 1000);
	fGRLaBr5Deadtime->SetLineColor(kGreen);
	fGRLaBr5Deadtime->SetLineWidth(2);
	fGRLaBr5Deadtime->SetLineStyle(1);
	fGRLaBr5Deadtime->SetMarkerStyle(0);
	fGRLaBr5Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr5Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr5Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr5Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr5Raw = MakeRollingGraph("LaBr5_Raw", "LaBr5 Raw Rate (Zoom)", 1000);
	fGRLaBr5Raw->SetLineColor(kGreen);
	fGRLaBr5Raw->SetLineWidth(2);
	fGRLaBr5Raw->SetLineStyle(1);
	fGRLaBr5Raw->SetMarkerStyle(0);
	fGRLaBr5Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr5Raw->GetXaxis()->CenterTitle();
	fGRLaBr5Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr5Raw->GetYaxis()->CenterTitle();

	// LaBr 6
	fGRLaBr6Accepted = MakeRollingGraph("LaBr6_Accepted", "LaBr6 Accepted Rate (Zoom)", 1000);
	fGRLaBr6Accepted->SetLineColor(kMagenta);
	fGRLaBr6Accepted->SetLineWidth(2);
	fGRLaBr6Accepted->SetLineStyle(1);
	fGRLaBr6Accepted->SetMarkerStyle(0);
	fGRLaBr6Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr6Accepted->GetXaxis()->CenterTitle();
	fGRLaBr6Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr6Accepted->GetYaxis()->CenterTitle();

	fGRLaBr6Deadtime = MakeRollingGraph("LaBr6_Deadtime", "LaBr6 Deadtime (Zoom)", 1000);
	fGRLaBr6Deadtime->SetLineColor(kMagenta);
	fGRLaBr6Deadtime->SetLineWidth(2);
	fGRLaBr6Deadtime->SetLineStyle(1);
	fGRLaBr6Deadtime->SetMarkerStyle(0);
	fGRLaBr6Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr6Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr6Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr6Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr6Raw = MakeRollingGraph("LaBr6_Raw", "LaBr6 Raw Rate (Zoom)", 1000);
	fGRLaBr6Raw->SetLineColor(kMagenta);
	fGRLaBr6Raw->SetLineWidth(2);
	fGRLaBr6Raw->SetLineStyle(1);
	fGRLaBr6Raw->SetMarkerStyle(0);
	fGRLaBr6Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr6Raw->GetXaxis()->CenterTitle();
	fGRLaBr6Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr6Raw->GetYaxis()->CenterTitle();

	// LaBr 7
	fGRLaBr7Accepted = MakeRollingGraph("LaBr7_Accepted", "LaBr7 Accepted Rate (Zoom)", 1000);
	fGRLaBr7Accepted->SetLineColor(kRed);
	fGRLaBr7Accepted->SetLineWidth(2);
	fGRLaBr7Accepted->SetLineStyle(1);
	fGRLaBr7Accepted->SetMarkerStyle(0);
	fGRLaBr7Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr7Accepted->GetXaxis()->CenterTitle();
	fGRLaBr7Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr7Accepted->GetYaxis()->CenterTitle();

	fGRLaBr7Deadtime = MakeRollingGraph("LaBr7_Deadtime", "LaBr7 Deadtime (Zoom)", 1000);
	fGRLaBr7Deadtime->SetLineColor(kRed);
	fGRLaBr7Deadtime->SetLineWidth(2);
	fGRLaBr7Deadtime->SetLineStyle(1);
	fGRLaBr7Deadtime->SetMarkerStyle(0);
	fGRLaBr7Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr7Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr7Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr7Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr7Raw = MakeRollingGraph("LaBr7_Raw", "LaBr7 Raw Rate (Zoom)", 1000);
	fGRLaBr7Raw->SetLineColor(kRed);
	fGRLaBr7Raw->SetLineWidth(2);
	fGRLaBr7Raw->SetLineStyle(1);
	fGRLaBr7Raw->SetMarkerStyle(0);
	fGRLaBr7Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr7Raw->GetXaxis()->CenterTitle();
	fGRLaBr7Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr7Raw->GetYaxis()->CenterTitle();

	// LaBr 8
	fGRLaBr8Accepted = MakeRollingGraph("LaBr8_Accepted", "LaBr8 Accepted Rate (Zoom)", 1000);
	fGRLaBr8Accepted->SetLineColor(kBlack);
	fGRLaBr8Accepted->SetLineWidth(2);
	fGRLaBr8Accepted->SetLineStyle(1);
	fGRLaBr8Accepted->SetMarkerStyle(0);
	fGRLaBr8Accepted->GetXaxis()->SetTitle("a.u.");
	fGRLaBr8Accepted->GetXaxis()->CenterTitle();
	fGRLaBr8Accepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr8Accepted->GetYaxis()->CenterTitle();

	fGRLaBr8Deadtime = MakeRollingGraph("LaBr8_Deadtime", "LaBr8 Deadtime (Zoom)", 1000);
	fGRLaBr8Deadtime->SetLineColor(kBlack);
	fGRLaBr8Deadtime->SetLineWidth(2);
	fGRLaBr8Deadtime->SetLineStyle(1);
	fGRLaBr8Deadtime->SetMarkerStyle(0);
	fGRLaBr8Deadtime->GetXaxis()->SetTitle("a.u.");
	fGRLaBr8Deadtime->GetXaxis()->CenterTitle();
	fGRLaBr8Deadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRLaBr8Deadtime->GetYaxis()->CenterTitle();

	fGRLaBr8Raw = MakeRollingGraph("LaBr8_Raw", "LaBr8 Raw Rate (Zoom)", 1000);
	fGRLaBr8Raw->SetLineColor(kBlack);
	fGRLaBr8Raw->SetLineWidth(2);
	fGRLaBr8Raw->SetLineStyle(1);
	fGRLaBr8Raw->SetMarkerStyle(0);
	fGRLaBr8Raw->GetXaxis()->SetTitle("a.u.");
	fGRLaBr8Raw->GetXaxis()->CenterTitle();
	fGRLaBr8Raw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRLaBr8Raw->GetYaxis()->CenterTitle();

	// Forward LaBr
	fGRForwardLaBrAccepted = MakeRollingGraph("ForwardLaBr_Accepted", "Forward LaBr Accepted (Zoom)", 1000);
	fGRForwardLaBrAccepted->SetLineColor(kBlue);
	fGRForwardLaBrAccepted->SetLineWidth(2);
	fGRForwardLaBrAccepted->SetLineStyle(1);
	fGRForwardLaBrAccepted->SetMarkerStyle(0);
	fGRForwardLaBrAccepted->GetXaxis()->SetTitle("a.u.");
	fGRForwardLaBrAccepted->GetXaxis()->CenterTitle();
	fGRForwardLaBrAccepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRForwardLaBrAccepted->GetYaxis()->CenterTitle();

	fGRForwardLaBrDeadtime = MakeRollingGraph("ForwardLaBr_Deadtime", "Forward LaBr Deadtime (Zoom)", 1000);
	fGRForwardLaBrDeadtime->SetLineColor(kBlue);
	fGRForwardLaBrDeadtime->SetLineWidth(2);
	fGRForwardLaBrDeadtime->SetLineStyle(1);
	fGRForwardLaBrDeadtime->SetMarkerStyle(0);
	fGRForwardLaBrDeadtime->GetXaxis()->SetTitle("a.u.");
	fGRForwardLaBrDeadtime->GetXaxis()->CenterTitle();
	fGRForwardLaBrDeadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRForwardLaBrDeadtime->GetYaxis()->CenterTitle();

	fGRForwardLaBrRaw = MakeRollingGraph("ForwardLaBr_Raw", "Forward LaBr Raw (Zoom)", 1000);
	fGRForwardLaBrRaw->SetLineColor(kBlue);
	fGRForwardLaBrRaw->SetLineWidth(2);
	fGRForwardLaBrRaw->SetLineStyle(1);
	fGRForwardLaBrRaw->SetMarkerStyle(0);
	fGRForwardLaBrRaw->GetXaxis()->SetTitle("a.u.");
	fGRForwardLaBrRaw->GetXaxis()->CenterTitle();
	fGRForwardLaBrRaw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRForwardLaBrRaw->GetYaxis()->CenterTitle();

	// Cherenkov
	fGRCherenkovAccepted = MakeRollingGraph("Cherenkov_Accepted", "Cherenkov Accepted (Zoom)", 1000);
	fGRCherenkovAccepted->SetLineColor(kMagenta);
	fGRCherenkovAccepted->SetLineWidth(2);
	fGRCherenkovAccepted->SetLineStyle(1);
	fGRCherenkovAccepted->SetMarkerStyle(0);
	fGRCherenkovAccepted->GetXaxis()->SetTitle("a.u.");
	fGRCherenkovAccepted->GetXaxis()->CenterTitle();
	fGRCherenkovAccepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRCherenkovAccepted->GetYaxis()->CenterTitle();

	fGRCherenkovDeadtime = MakeRollingGraph("Cherenkov_Deadtime", "Cherenkov Deadtime (Zoom)", 1000);
	fGRCherenkovDeadtime->SetLineColor(kMagenta);
	fGRCherenkovDeadtime->SetLineWidth(2);
	fGRCherenkovDeadtime->SetLineStyle(1);
	fGRCherenkovDeadtime->SetMarkerStyle(0);
	fGRCherenkovDeadtime->GetXaxis()->SetTitle("a.u.");
	fGRCherenkovDeadtime->GetXaxis()->CenterTitle();
	fGRCherenkovDeadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRCherenkovDeadtime->GetYaxis()->CenterTitle();

	fGRCherenkovRaw = MakeRollingGraph("Cherenkov_Raw", "Cherenkov Raw (Zoom)", 1000);
	fGRCherenkovRaw->SetLineColor(kMagenta);
	fGRCherenkovRaw->SetLineWidth(2);
	fGRCherenkovRaw->SetLineStyle(1);
	fGRCherenkovRaw->SetMarkerStyle(0);
	fGRCherenkovRaw->GetXaxis()->SetTitle("a.u.");
	fGRCherenkovRaw->GetXaxis()->CenterTitle();
	fGRCherenkovRaw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRCherenkovRaw->GetYaxis()->CenterTitle();

	// Plastic
	fGRPlasticAccepted = MakeRollingGraph("Plastic_Accepted", "Plastic Accepted (Zoom)", 1000);
	fGRPlasticAccepted->SetLineColor(kGreen);
	fGRPlasticAccepted->SetLineWidth(2);
	fGRPlasticAccepted->SetLineStyle(1);
	fGRPlasticAccepted->SetMarkerStyle(0);
	fGRPlasticAccepted->GetXaxis()->SetTitle("a.u.");
	fGRPlasticAccepted->GetXaxis()->CenterTitle();
	fGRPlasticAccepted->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRPlasticAccepted->GetYaxis()->CenterTitle();

	fGRPlasticDeadtime = MakeRollingGraph("Plastic_Deadtime", "Plastic Deadtime (Zoom)", 1000);
	fGRPlasticDeadtime->SetLineColor(kGreen);
	fGRPlasticDeadtime->SetLineWidth(2);
	fGRPlasticDeadtime->SetLineStyle(1);
	fGRPlasticDeadtime->SetMarkerStyle(0);
	fGRPlasticDeadtime->GetXaxis()->SetTitle("a.u.");
	fGRPlasticDeadtime->GetXaxis()->CenterTitle();
	fGRPlasticDeadtime->GetYaxis()->SetTitle("Relative Totzeit: 1 - (Live/Total)");
	fGRPlasticDeadtime->GetYaxis()->CenterTitle();

	fGRPlasticRaw = MakeRollingGraph("Plastic_Raw", "Plastic Raw (Zoom)", 1000);
	fGRPlasticRaw->SetLineColor(kGreen);
	fGRPlasticRaw->SetLineWidth(2);
	fGRPlasticRaw->SetLineStyle(1);
	fGRPlasticRaw->SetMarkerStyle(0);
	fGRPlasticRaw->GetXaxis()->SetTitle("a.u.");
	fGRPlasticRaw->GetXaxis()->CenterTitle();
	fGRPlasticRaw->GetYaxis()->SetTitle("Rate/(1/s)");
	fGRPlasticRaw->GetYaxis()->CenterTitle();


	fGRLaBr0RawDivEnton = MakeRollingGraph("LaBr0Raw_Divided_Enton", "LaBr0 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr0RawDivEnton->SetLineColor(kGray);
	fGRLaBr0RawDivEnton->SetLineWidth(2);
	fGRLaBr0RawDivEnton->SetLineStyle(1);
	fGRLaBr0RawDivEnton->SetMarkerStyle(0);
	fGRLaBr0RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr0RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr0RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr0RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr1RawDivEnton = MakeRollingGraph("LaBr1Raw_Divided_Enton", "LaBr1 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr1RawDivEnton->SetLineColor(kCyan);
	fGRLaBr1RawDivEnton->SetLineWidth(2);
	fGRLaBr1RawDivEnton->SetLineStyle(1);
	fGRLaBr1RawDivEnton->SetMarkerStyle(0);
	fGRLaBr1RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr1RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr1RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr1RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr2RawDivEnton = MakeRollingGraph("LaBr2Raw_Divided_Enton", "LaBr2 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr2RawDivEnton->SetLineColor(kBlue);
	fGRLaBr2RawDivEnton->SetLineWidth(2);
	fGRLaBr2RawDivEnton->SetLineStyle(1);
	fGRLaBr2RawDivEnton->SetMarkerStyle(0);
	fGRLaBr2RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr2RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr2RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr2RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr3RawDivEnton = MakeRollingGraph("LaBr3Raw_Divided_Enton", "LaBr3 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr3RawDivEnton->SetLineColor(kOrange);
	fGRLaBr3RawDivEnton->SetLineWidth(2);
	fGRLaBr3RawDivEnton->SetLineStyle(1);
	fGRLaBr3RawDivEnton->SetMarkerStyle(0);
	fGRLaBr3RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr3RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr3RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr3RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr4RawDivEnton = MakeRollingGraph("LaBr4Raw_Divided_Enton", "LaBr4 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr4RawDivEnton->SetLineColor(kViolet);
	fGRLaBr4RawDivEnton->SetLineWidth(2);
	fGRLaBr4RawDivEnton->SetLineStyle(1);
	fGRLaBr4RawDivEnton->SetMarkerStyle(0);
	fGRLaBr4RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr4RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr4RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr4RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr5RawDivEnton = MakeRollingGraph("LaBr5Raw_Divided_Enton", "LaBr5 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr5RawDivEnton->SetLineColor(kGreen);
	fGRLaBr5RawDivEnton->SetLineWidth(2);
	fGRLaBr5RawDivEnton->SetLineStyle(1);
	fGRLaBr5RawDivEnton->SetMarkerStyle(0);
	fGRLaBr5RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr5RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr5RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr5RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr6RawDivEnton = MakeRollingGraph("LaBr6Raw_Divided_Enton", "LaBr6 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr6RawDivEnton->SetLineColor(kMagenta);
	fGRLaBr6RawDivEnton->SetLineWidth(2);
	fGRLaBr6RawDivEnton->SetLineStyle(1);
	fGRLaBr6RawDivEnton->SetMarkerStyle(0);
	fGRLaBr6RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr6RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr6RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr6RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr7RawDivEnton = MakeRollingGraph("LaBr7Raw_Divided_Enton", "LaBr7 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr7RawDivEnton->SetLineColor(kRed);
	fGRLaBr7RawDivEnton->SetLineWidth(2);
	fGRLaBr7RawDivEnton->SetLineStyle(1);
	fGRLaBr7RawDivEnton->SetMarkerStyle(0);
	fGRLaBr7RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr7RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr7RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr7RawDivEnton->GetYaxis()->CenterTitle();


	fGRLaBr8RawDivEnton = MakeRollingGraph("LaBr8Raw_Divided_Enton", "LaBr8 Raw / Enton Raw (Zoom)",1000);
	fGRLaBr8RawDivEnton->SetLineColor(kBlack);
	fGRLaBr8RawDivEnton->SetLineWidth(2);
	fGRLaBr8RawDivEnton->SetLineStyle(1);
	fGRLaBr8RawDivEnton->SetMarkerStyle(0);
	fGRLaBr8RawDivEnton->GetXaxis()->SetTitle("a.u.");
	fGRLaBr8RawDivEnton->GetXaxis()->CenterTitle();
	fGRLaBr8RawDivEnton->GetYaxis()->SetTitle("Rate LaBr0 / Rate Enton");
	fGRLaBr8RawDivEnton->GetYaxis()->CenterTitle();



	/** Pictures */
	fPDagEnergyRaw = GetPicture("DagoRawEnergyHist");
	if(!fPDagEnergyRaw)
	{
		fPDagEnergyRaw = new TGo4Picture("DagoRawEnergyHist","Unkalibrierte Energiehistogramme");
		fPDagEnergyRaw->SetDivision(3,3);
		fPDagEnergyRaw->Pic(2,0)->AddObject(fHRawEnergyLaBr0);
		fPDagEnergyRaw->Pic(1,0)->AddObject(fHRawEnergyLaBr1);
		fPDagEnergyRaw->Pic(0,0)->AddObject(fHRawEnergyLaBr2);
		fPDagEnergyRaw->Pic(2,1)->AddObject(fHRawEnergyLaBr3);
		fPDagEnergyRaw->Pic(1,1)->AddObject(fHRawEnergyLaBr4);
		fPDagEnergyRaw->Pic(0,1)->AddObject(fHRawEnergyLaBr5);
		fPDagEnergyRaw->Pic(2,2)->AddObject(fHRawEnergyLaBr6);
		fPDagEnergyRaw->Pic(1,2)->AddObject(fHRawEnergyLaBr7);
		fPDagEnergyRaw->Pic(0,2)->AddObject(fHRawEnergyLaBr8);
		AddPicture(fPDagEnergyRaw);
	}
	fPDagEnergyCali = GetPicture("DagoCaliEnergyHist");
	if(!fPDagEnergyCali)
	{
		fPDagEnergyCali = new TGo4Picture("DagoCaliEnergyHist","Kalibrierte Energiehistogramme");
		fPDagEnergyCali->SetDivision(3,3);
		fPDagEnergyCali->Pic(2,0)->AddObject(fHCaliEnergyLaBr0);
		fPDagEnergyCali->Pic(1,0)->AddObject(fHCaliEnergyLaBr1);
		fPDagEnergyCali->Pic(0,0)->AddObject(fHCaliEnergyLaBr2);
		fPDagEnergyCali->Pic(2,1)->AddObject(fHCaliEnergyLaBr3);
		fPDagEnergyCali->Pic(1,1)->AddObject(fHCaliEnergyLaBr4);
		fPDagEnergyCali->Pic(0,1)->AddObject(fHCaliEnergyLaBr5);
		fPDagEnergyCali->Pic(2,2)->AddObject(fHCaliEnergyLaBr6);
		fPDagEnergyCali->Pic(1,2)->AddObject(fHCaliEnergyLaBr7);
		fPDagEnergyCali->Pic(0,2)->AddObject(fHCaliEnergyLaBr8);
		AddPicture(fPDagEnergyCali);

	}
	fPDagEnergyShift = GetPicture("DagoEnergyShift");
	if(!fPDagEnergyShift)
	{
		fPDagEnergyShift = new TGo4Picture("DagoEnergyShift","Unkalibrierte Energiehistogramme");
		fPDagEnergyShift->SetDivision(3,3);
		fPDagEnergyShift->Pic(2,0)->AddObject(fHLaBr0EnergyVsTime);
		fPDagEnergyShift->Pic(1,0)->AddObject(fHLaBr1EnergyVsTime);
		fPDagEnergyShift->Pic(0,0)->AddObject(fHLaBr2EnergyVsTime);
		fPDagEnergyShift->Pic(2,1)->AddObject(fHLaBr3EnergyVsTime);
		fPDagEnergyShift->Pic(1,1)->AddObject(fHLaBr4EnergyVsTime);
		fPDagEnergyShift->Pic(0,1)->AddObject(fHLaBr5EnergyVsTime);
		fPDagEnergyShift->Pic(2,2)->AddObject(fHLaBr6EnergyVsTime);
		fPDagEnergyShift->Pic(1,2)->AddObject(fHLaBr7EnergyVsTime);
		fPDagEnergyShift->Pic(0,2)->AddObject(fHLaBr8EnergyVsTime);
		AddPicture(fPDagEnergyShift);
	}
	fPTimePeak_MeanSzint = GetPicture("DagoTimePeak_MeanSzint");
	if(!fPTimePeak_MeanSzint)
	{
		fPTimePeak_MeanSzint = new TGo4Picture("DagoTimePeak_MeanSzint","Koinzidenzzeit-Peak - Zeit relativ zu Mean-Szint");
		fPTimePeak_MeanSzint->SetDivision(3,3);
		fPTimePeak_MeanSzint->Pic(2,0)->AddObject(fHLaBr0VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(1,0)->AddObject(fHLaBr1VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(0,0)->AddObject(fHLaBr2VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(2,1)->AddObject(fHLaBr3VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(1,1)->AddObject(fHLaBr4VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(0,1)->AddObject(fHLaBr5VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(2,2)->AddObject(fHLaBr6VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(1,2)->AddObject(fHLaBr7VsMeanSzint);
		fPTimePeak_MeanSzint->Pic(0,2)->AddObject(fHLaBr8VsMeanSzint);
		AddPicture(fPTimePeak_MeanSzint);
	}
	fPTimePeak_Coinc = GetPicture("DagoTimePeak_LaBrCoinc");
	if(!fPTimePeak_Coinc)
	{
		fPTimePeak_Coinc = new TGo4Picture("DagoTimePeak_LaBrCoinc","Koinzidenzzeit-Peak - Zeit relativ zu LaBr-Coinc");
		fPTimePeak_Coinc->SetDivision(3,3);
		fPTimePeak_Coinc->Pic(2,0)->AddObject(fHLaBr0VsCoinc);
		fPTimePeak_Coinc->Pic(1,0)->AddObject(fHLaBr1VsCoinc);
		fPTimePeak_Coinc->Pic(0,0)->AddObject(fHLaBr2VsCoinc);
		fPTimePeak_Coinc->Pic(2,1)->AddObject(fHLaBr3VsCoinc);
		fPTimePeak_Coinc->Pic(1,1)->AddObject(fHLaBr4VsCoinc);
		fPTimePeak_Coinc->Pic(0,1)->AddObject(fHLaBr5VsCoinc);
		fPTimePeak_Coinc->Pic(2,2)->AddObject(fHLaBr6VsCoinc);
		fPTimePeak_Coinc->Pic(1,2)->AddObject(fHLaBr7VsCoinc);
		fPTimePeak_Coinc->Pic(0,2)->AddObject(fHLaBr8VsCoinc);
		AddPicture(fPTimePeak_Coinc);
	}
	fPSumSpectra = GetPicture("DagoSumSpectra");
	if(!fPSumSpectra)
	{
		fPSumSpectra = new TGo4Picture("DagoSumSpectra","Dagobert Summen-Spektren");
		fPSumSpectra->SetDivision(2,2);

		fPSumSpectra->Pic(0,0)->AddObject(fHLaBrVsMeanSzintSum);
		fPSumSpectra->Pic(1,0)->AddObject(fHLaBrVsCoincSum);
		fPSumSpectra->Pic(0,1)->AddObject(fHCaliEnergyLaBrSum);
		AddPicture(fPSumSpectra);
	}
	fPForwardDetectors = GetPicture("DagoForwardDetectors");
	if(!fPForwardDetectors)
	{
		fPForwardDetectors = new TGo4Picture("DagoForwardDetectors","Vorwaertswinkel-Detektoren");
		fPForwardDetectors->SetDivision(2,3);
		fPForwardDetectors->Pic(0,0)->AddObject(fHRawEnergyLaBrForward);
		fPForwardDetectors->Pic(1,0)->AddObject(fHCaliEnergyLaBrForward);
		/*
		fPForwardDetectors->Pic(0,0)->AddObject();
		fPForwardDetectors->Pic(0,0)->AddObject();
		fPForwardDetectors->Pic(0,0)->AddObject();
		*/
		AddPicture(fPForwardDetectors);
	}
	fPDagoRateMonitor = GetPicture("DagoRateMonitor");


	fPDagoRateMonitorRollingGraph = GetPicture("DagoRateMonitorRollGraph");
	if(!fPDagoRateMonitorRollingGraph)
	{
		fPDagoRateMonitorRollingGraph = new TGo4Picture("DagoRateMonitorRollGraph","Raten Dagobert");
		fPDagoRateMonitorRollingGraph->SetDivision(2,3);
		
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr0Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr1Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr2Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr3Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr4Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr5Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr6Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr7Raw);
		fPDagoRateMonitorRollingGraph->Pic(0,0)->AddObject(fGRLaBr8Raw);

		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr0Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr1Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr2Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr3Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr4Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr5Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr6Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr7Accepted);
		fPDagoRateMonitorRollingGraph->Pic(0,1)->AddObject(fGRLaBr8Accepted);

		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr0Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr1Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr2Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr3Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr4Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr5Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr6Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr7Deadtime);
		fPDagoRateMonitorRollingGraph->Pic(0,2)->AddObject(fGRLaBr8Deadtime);

		fPDagoRateMonitorRollingGraph->Pic(1,0)->AddObject(fGRForwardLaBrRaw);
		fPDagoRateMonitorRollingGraph->Pic(1,0)->AddObject(fGRCherenkovRaw);
		fPDagoRateMonitorRollingGraph->Pic(1,0)->AddObject(fGRPlasticRaw);

		fPDagoRateMonitorRollingGraph->Pic(1,1)->AddObject(fGRForwardLaBrAccepted);
		fPDagoRateMonitorRollingGraph->Pic(1,1)->AddObject(fGRCherenkovAccepted);
		fPDagoRateMonitorRollingGraph->Pic(1,1)->AddObject(fGRPlasticAccepted);

		fPDagoRateMonitorRollingGraph->Pic(1,2)->AddObject(fGRForwardLaBrDeadtime);
		fPDagoRateMonitorRollingGraph->Pic(1,2)->AddObject(fGRCherenkovDeadtime);
		fPDagoRateMonitorRollingGraph->Pic(1,2)->AddObject(fGRPlasticDeadtime);


		AddPicture(fPDagoRateMonitorRollingGraph);
	}




	if(!fPDagoRateMonitor)
	{
		fPDagoRateMonitor = new TGo4Picture("DagoRateMonitor","Raten Dagobert");
		fPDagoRateMonitor->SetDivision(2,3);

		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr0Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr1Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr2Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr3Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr4Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr5Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr6Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr7Raw);
		fPDagoRateMonitor->Pic(0,0)->AddObject(fGLaBr8Raw);


		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr0Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr1Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr2Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr3Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr4Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr5Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr6Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr7Accepted);
		fPDagoRateMonitor->Pic(0,1)->AddObject(fGLaBr8Accepted);

		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr0Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr1Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr2Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr3Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr4Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr5Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr6Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr7Deadtime);
		fPDagoRateMonitor->Pic(0,2)->AddObject(fGLaBr8Deadtime);
		
		fPDagoRateMonitor->Pic(1,0)->AddObject(fGForwardLaBrRaw);
		fPDagoRateMonitor->Pic(1,0)->AddObject(fGCherenkovRaw);
		fPDagoRateMonitor->Pic(1,0)->AddObject(fGPlasticRaw);

		fPDagoRateMonitor->Pic(1,1)->AddObject(fGForwardLaBrAccepted);
		fPDagoRateMonitor->Pic(1,1)->AddObject(fGCherenkovAccepted);
		fPDagoRateMonitor->Pic(1,1)->AddObject(fGPlasticAccepted);

		fPDagoRateMonitor->Pic(1,2)->AddObject(fGForwardLaBrDeadtime);
		fPDagoRateMonitor->Pic(1,2)->AddObject(fGCherenkovDeadtime);
		fPDagoRateMonitor->Pic(1,2)->AddObject(fGPlasticDeadtime);


		AddPicture(fPDagoRateMonitor);
	}








	/** starte tcp-verbindung */
	OpenTcpConnection();
}

TDagobertHitProc::~TDagobertHitProc()
{
	/* schliesse tcp-verbindung */
	CloseTcpConnection();
}
/**
 * Start Tcp-Verbindung */
Int_t TDagobertHitProc::OpenTcpConnection()
{
	printf("StarteTcpVerbindung!\n");
	printf("Host: %s.\n",fTcpPar->fxTcpHost.Data());
	if(!fTcpPar->fbOpenTcpConnection)
	{
		fbTcpIsOpen = kFALSE;
		return 0;
	}
	if(fbTcpIsOpen) { return 0; }
	fTcpSocket = socket(AF_INET,SOCK_STREAM,0);
	if(fTcpSocket < 0)
	{
		TGo4Log::Error("[TDagobertHitProc] Socket wurde nicht erstellt!");
		throw TGo4EventErrorException(this);
	}
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(fTcpPar->fiTcpPort);
	inet_pton(AF_INET,fTcpPar->fxTcpHost.Data(),&addr.sin_addr);
	if(connect(fTcpSocket,(sockaddr*)&addr,sizeof(addr))<0)
	{
		TGo4Log::Error("[TDagobertHitProc] Verbindung zu %s:%d fehlgeschlagen!",fTcpPar->fxTcpHost.Data(),fTcpPar->fiTcpPort);
		throw TGo4EventErrorException(this);
	}
	int flags = fcntl(fTcpSocket, F_GETFL, 0);
	fcntl(fTcpSocket, F_SETFL, flags | O_NONBLOCK);
	fbTcpIsOpen = kTRUE;
	TGo4Log::Info("[TDagobertHitProc] Verbindung hergestellt zu %s:%d.",fTcpPar->fxTcpHost.Data(),fTcpPar->fiTcpPort);
	return 0;
}
/** schliesse tcp-verbindung */
Int_t TDagobertHitProc::CloseTcpConnection()
{
	if(!fbTcpIsOpen) { return 0; }
	if(fTcpSocket >= 0) { close(fTcpSocket); }
	fTcpSocket = -1;
	fbTcpIsOpen = kFALSE;
	return 0;
}





/** baue output event */
Bool_t TDagobertHitProc::BuildEvent(TGo4EventElement* dest)
{
	/*
	TGo4Log::Info("");
	TGo4Log::Info("");
	TGo4Log::Info("NEW EVENT");
	*/


	auto* raw = dynamic_cast<TQClamDagobertRawEvent*>(GetInputEvent("DagRawProvider"));
	auto* evt = dynamic_cast<TDagobertHitEvent*>(dest);
	if(!raw || !evt) { return kFALSE; }


	/// 
	// zeit holen
	double eventTime = raw->header.mbs_time_sec + raw->header.mbs_time_subsec * 1e-9;

	if(fFirstEventTime)
	{
		fStartTime = eventTime;
		fFirstEventTime = false;
	}
	
	fGraphTime = (eventTime - fStartTime) / 60.; // in Min.




	/** Lese Daten aus Tcp-Verbindung und schreibe sie ins output-event */
	if(fTcpPar->fbOpenTcpConnection)
	{
		/** nicht pausiert und offen --> process */
		if(!fTcpPar->fbPauseTcpConnection && fbTcpIsOpen)
		{
			if(fTcpSocket >= 0) { ProcessTcpEvents(evt); }
		}
		/** nicht pausiert, geschlossen --> open */
		else if(!fTcpPar->fbPauseTcpConnection && !fbTcpIsOpen) { OpenTcpConnection(); }
		/** pausiert, offen --> close */
		else if(fTcpPar->fbPauseTcpConnection && fbTcpIsOpen) { CloseTcpConnection(); }
	}


	//for(const auto& gh : raw->gammaHits)
	for(const auto& sc : raw->dagobertScalers)
	{
		ProcessScalerSnapshot(sc);
	}

	for(const auto& gh : raw->gammaHits)
	{
		DagHit h = BuildHit(gh);



		if(h.module == 0 && h.channel == 0) { evt->coincTimeLaBr = h.time; }
		else if(h.module == 0 && h.channel == 1) { evt->szintPmtDownTime = h.time; }
		else if(h.module == 0 && h.channel == 2) { evt->szintPmtUpTime = h.time; }
		else if(h.module == 0 && h.channel == 4) { evt->rateLaBrOR = h.rateRaw; }
		else if(h.module == 0 && h.channel == 6) { evt->rateLaBrORNoDead = h.rateRaw; }
		// LaBr-Detektoren in Towers 
		else if(h.module == 0 && h.channel >= 8 && h.channel <= 13)
		{
			int idx = h.channel - 8;
			int scalerIndex = h.channel; // 8..10

			double energy = NAN;
//			std::cout << "energy,"
//				<< " module = 1, ch = " << h.channel
//				<< " E_Raw = " << h.energy
//				<< " rate = " << fChan[0][scalerIndex].rateRaw
//				<< std::endl;
			fDagoCaliPar->CorrectRateDependence(
					idx,
					h.energy,
					fChan[0][scalerIndex].rateRaw,//rateAccepted,
					energy);


//			std::cout << "rate-corrected energy = " << energy << std::endl;
			h.calibrated_energy = fDagoCaliPar->CalibrateEnergy(energy,idx);
//			std::cout << "calibrated energy = " << h.calibrated_energy << std::endl;

			
			evt->laBr[idx] = h;
		} // LaBr 0..5
		else if(h.module == 1 && h.channel >= 8 && h.channel <= 10)
		{
			int idx = h.channel - 8 + 6;
			int scalerIndex = h.channel; // 8..10

			/** energiekalibrierung */
//			h.calibrated_energy = fDagoCaliPar->CalibrateEnergy(h.energy,idx);


			double energy = NAN;
//			std::cout << "energy,"
//				<< " module = 1, ch = " << h.channel
//				<< " E_Raw = " << h.energy
//				<< " rate = " << fChan[1][scalerIndex].rateRaw
//				<< std::endl;
			fDagoCaliPar->CorrectRateDependence(
					idx,
					h.energy,
					fChan[1][scalerIndex].rateRaw,//rateAccepted,
					energy);


//			std::cout << "rate-corrected energy = " << energy << std::endl;
			h.calibrated_energy = fDagoCaliPar->CalibrateEnergy(energy,idx);
//			std::cout << "calibrated energy = " << h.calibrated_energy << std::endl;



			/*
			double energy = fDagoCaliPar->CalibrateEnergy(h.energy,idx);

			fDagoCaliPar->CorrectRateDependence(
					idx,
					energy,
					fChan[1][scalerIndex].rateRaw,//rateAccepted,
					h.calibrated_energy);
*/
		
			
			evt->laBr[idx] = h;
		} // LaBr 6..8

		// Vorwaertswinkel-Detektoren 
		else if(h.module == 1 && h.channel == 11)
		{

			double energy = NAN;
//			std::cout << "energy,"
//				<< " module = 1, ch = 11 "
//				<< " E_Raw = " << h.energy
//				<< " rate = " << fChan[1][11].rateRaw
//				<< std::endl;
			fDagoCaliPar->CorrectRateDependence(
					fDagoCaliPar->N_DETECTORS-1,
					h.energy,
					fChan[1][11].rateRaw,//rateAccepted,
					energy);


//			std::cout << "rate-corrected energy = " << energy << std::endl;
			h.calibrated_energy = fDagoCaliPar->CalibrateEnergy(energy,fDagoCaliPar->N_DETECTORS-1);
//			std::cout << "calibrated energy = " << h.calibrated_energy << std::endl;



			/*
			double energy = fDagoCaliPar->CalibrateEnergy(h.energy,fDagoCaliPar->N_DETECTORS-1);
			fDagoCaliPar->CorrectRateDependence(
					fDagoCaliPar->N_DETECTORS-1,
					energy,fChan[1][11].rateRaw,//rateAccepted,
					h.calibrated_energy);
					*/


			evt->forwardLaBr = h;
		} // LaBr Vorwaertswinkel
		else if(h.module == 1 && h.channel == 15)
		{
			fSumCherenkovAccepted += fChan[1][15].rateAccepted;
			fSumCherenkovDeadtime += fChan[1][15].deadtime;
			evt->cherenkov = h;
		} // Cherenkov Vorwaertswinkel
		else if(h.module == 0 && h.channel == 15)
		{
			fSumPlasticAccepted += fChan[0][15].rateAccepted;
			fSumPlasticDeadtime += fChan[0][15].deadtime;
			evt->plastic = h;
		} // Plastic Szint Vorwaertswinkel (Beamtime 2025)
		// Timing + Raten
//		FillHistograms(h);
	}
	/** berechne meanSzint-Zeit --> dient als Referenzzeit */
	if(evt->szintPmtUpTime > 0 && evt->szintPmtDownTime > 0)
	{
		evt->meanSzintTime = 0.5 * (evt->szintPmtUpTime + evt->szintPmtDownTime);
	}
	// LaBr 0..8
	for(int i=0; i<9; ++i)
	{
		auto& h = evt->laBr[i];
		if(h.time > 0)
		{
			h.dtToMeanSzintTime = evt->meanSzintTime - h.time;
			h.dtToLaBrCoincTime = evt->coincTimeLaBr - h.time;
		}
	}
	// Forward LaBr
	if(evt->forwardLaBr.time > 0)
	{
		evt->forwardLaBr.dtToMeanSzintTime = evt->meanSzintTime - evt->forwardLaBr.time;
		evt->forwardLaBr.dtToLaBrCoincTime = evt->coincTimeLaBr - evt->forwardLaBr.time;
	}
	// Cherenkov
	if(evt->cherenkov.time > 0)
	{
		evt->cherenkov.dtToMeanSzintTime = evt->meanSzintTime - evt->cherenkov.time;
		evt->cherenkov.dtToLaBrCoincTime = evt->coincTimeLaBr - evt->cherenkov.time;
	}
	// Plastic Szint
	if(evt->plastic.time > 0)
	{
		evt->plastic.dtToMeanSzintTime = evt->meanSzintTime - evt->plastic.time;
		evt->plastic.dtToLaBrCoincTime = evt->coincTimeLaBr - evt->plastic.time;
	}




	// setze startzeit, um relativ dazu zeiten zu berechnen
	if(fFirstTime < 0) { fFirstTime = evt->meanSzintTime; }
	else
	{
	
		if(fHistoPar->fBFillHistos)
		{
			// LaBr 0
			if(evt->laBr[0].time > 0)
			{
				double t = (evt->laBr[0].time - fFirstTime) * 1e-9; // ns -> s
				fHLaBr0EnergyVsTime->Fill(t/60., evt->laBr[0].energy);
			}
			// LaBr 1
			if(evt->laBr[1].time > 0)
			{
				double t = (evt->laBr[1].time - fFirstTime) * 1e-9;
				fHLaBr1EnergyVsTime->Fill(t/60., evt->laBr[1].energy);
			}
			// LaBr 2
			if(evt->laBr[2].time > 0)
			{
				double t = (evt->laBr[2].time - fFirstTime) * 1e-9;
				fHLaBr2EnergyVsTime->Fill(t/60., evt->laBr[2].energy);
			}
	
			// LaBr 3
			if(evt->laBr[3].time > 0)
			{
				double t = (evt->laBr[3].time - fFirstTime) * 1e-9;
				fHLaBr3EnergyVsTime->Fill(t/60., evt->laBr[3].energy);
			}
			// LaBr 4
			if(evt->laBr[4].time > 0)
			{
				double t = (evt->laBr[4].time - fFirstTime) * 1e-9;
				fHLaBr4EnergyVsTime->Fill(t/60., evt->laBr[4].energy);
			}
			// LaBr 5
			if(evt->laBr[5].time > 0)
			{
				double t = (evt->laBr[5].time - fFirstTime) * 1e-9;
				fHLaBr5EnergyVsTime->Fill(t/60., evt->laBr[5].energy);
			}
			// LaBr 6
			if(evt->laBr[6].time > 0)
			{
				double t = (evt->laBr[6].time - fFirstTime) * 1e-9;
				fHLaBr6EnergyVsTime->Fill(t/60., evt->laBr[6].energy);
			}
			// LaBr 7
			if(evt->laBr[7].time > 0)
			{
				double t = (evt->laBr[7].time - fFirstTime) * 1e-9;
				fHLaBr7EnergyVsTime->Fill(t/60., evt->laBr[7].energy);
			}
			// LaBr 8
			if(evt->laBr[8].time > 0)
			{
				double t = (evt->laBr[8].time - fFirstTime) * 1e-9;
				fHLaBr8EnergyVsTime->Fill(t/60., evt->laBr[8].energy);
			}
		}
	}


	if(fHistoPar->fBFillHistos) { FillHistograms(evt); }


	evt->SetValid(kTRUE);
	return kTRUE;




	//TGo4Log::Info()
}





Bool_t TDagobertHitProc::ProcessTcpEvents(TDagobertHitEvent* evt)
{
	std::string line;
	if(!ReadLine(line)) { return kFALSE; }
	sscanf(line.c_str(), "%lf,%lf", &evt->rateEnton, &evt->rateSzintScattChamber);
	return kTRUE;
}

/** Lese Zeile aus Tcp-Verbindung */
Bool_t TDagobertHitProc::ReadLine(std::string& out)
{
        out.clear();
        char c;
        while(true)//read(fSocket, &c, 1) == 1)
        {
                int n = read(fTcpSocket, &c, 1);
                if(n==1)
                {
                        // byte empfangen
                        if(c == '\n')
                        {
                                // komplette zeile fertig
                                out = fTcpBuffer;
                                fTcpBuffer.clear();
                                return kTRUE;
                        }
                        fTcpBuffer.push_back(c);
                }
                else if(n == -1 && errno == EWOULDBLOCK)
                {
                        // keine neuen Daten
                        return kFALSE;
                }
                else
                {
                        // Verbindung beendet
			TGo4Log::Error("[TDagobertHitProc] ReadLine: Verbindung beendet oder Fehler (n=%d, errno=%d)", n, errno);
                        throw TGo4EventEndException(this);
                }
        }
        return kFALSE;
}



void TDagobertHitProc::ProcessScalerSnapshot(const RawDagobertScaler& sc)
{
	// TODO: umziehen nach TQClamConstants
	constexpr int CLOCK_CHANNEL = 3;
	constexpr double CLOCK_FREQ = 1e7; // 10 MHz SIS scaler clock


	auto& state = fScalerState[sc.module];


	if(sc.suppressed.size() <= CLOCK_CHANNEL) { return; }


	uint64_t clock = sc.suppressed[CLOCK_CHANNEL];


	// erster Snapshot
	if(!state.initialized)
	{
		state.prevClock = clock;
		state.prevRaw = sc.raw;
		state.prevAccepted = sc.accepted;

		state.initialized = true;
		return;
	}


	// Zeitdifferenz
	uint64_t dClock = clock - state.prevClock;

	double dt = double(dClock) / CLOCK_FREQ;


	if(dt <= 0)
	{
		state.prevClock = clock;
		state.prevRaw = sc.raw;
		state.prevAccepted = sc.accepted;
		return;
	}



	size_t n = std::min(sc.raw.size(), state.prevRaw.size());


	for(size_t ch=0; ch<n; ch++)
	{

		auto& st = fChan[sc.module][ch];


		uint64_t raw = sc.raw[ch];
		uint64_t oldRaw = state.prevRaw[ch];

		uint64_t acc = sc.accepted[ch];
		uint64_t oldAcc = state.prevAccepted[ch];



		// Counter reset / overflow
		if(raw < oldRaw || acc < oldAcc)
		{
			st.rateRaw = 0.;
			st.rateAccepted = 0.;
			st.deadtime = 0.;
			continue;
		}


		uint64_t dRaw = raw - oldRaw;
		uint64_t dAcc = acc - oldAcc;


		/*
		if(sc.module == 0 && (ch==8 || ch==9 || ch==10))
		{
			std::cout
				<< "[SCALER DEBUG]"
        			<< " module=" << sc.module
        			<< " ch=" << ch
        			<< " clock=" << clock
        			<< " dClock=" << dClock
        			<< " dt=" << dt
        			<< " raw=" << raw
        			<< " oldRaw=" << oldRaw
        			<< " dRaw=" << dRaw
        			<< " acc=" << acc
        			<< " oldAcc=" << oldAcc
        			<< " dAcc=" << dAcc
        			<< std::endl;
		}
		*/


		st.rateRaw = double(dRaw)/dt;


		st.rateAccepted = double(dAcc)/dt;

		// Deadtime
		if(dRaw > 0)
		{
			st.deadtime = 1.0 - double(dAcc)/double(dRaw);

			if(st.deadtime < 0) { st.deadtime = 0; }

			if(st.deadtime > 1) { st.deadtime = 1; }
		}
		else
		{
			st.deadtime = 0;
		}



		if(sc.module == 0 && ch >= 8 && ch <= 13)
		{
			switch(ch)
			{
				case 8:
				{
					fGRLaBr0Raw->Fill(st.rateRaw,0,0);
					fGRLaBr0Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr0Deadtime->Fill(st.deadtime,0,0);

					int N_Raw0 = fGLaBr0Raw->GetN();
					fGLaBr0Raw->SetPoint(N_Raw0,fGraphTime,st.rateRaw);
					int N_Acc0 = fGLaBr0Accepted->GetN();
					fGLaBr0Accepted->SetPoint(N_Acc0,fGraphTime,st.rateAccepted);
					int N_Dead0 = fGLaBr0Deadtime->GetN();
					fGLaBr0Deadtime->SetPoint(N_Dead0,fGraphTime,st.deadtime);
					break;
				}

				case 9:
				{
					fGRLaBr1Raw->Fill(st.rateRaw,0,0);
					fGRLaBr1Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr1Deadtime->Fill(st.deadtime,0,0);

					int N_Raw1 = fGLaBr1Raw->GetN();
					fGLaBr1Raw->SetPoint(N_Raw1,fGraphTime,st.rateRaw);
					int N_Acc1 = fGLaBr1Accepted->GetN();
					fGLaBr1Accepted->SetPoint(N_Acc1,fGraphTime,st.rateAccepted);
					int N_Dead1 = fGLaBr1Deadtime->GetN();
					fGLaBr1Deadtime->SetPoint(N_Dead1,fGraphTime,st.deadtime);
					break;
				}

				case 10:
				{
					fGRLaBr2Raw->Fill(st.rateRaw,0,0);
					fGRLaBr2Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr2Deadtime->Fill(st.deadtime,0,0);

					int N_Raw2 = fGLaBr2Raw->GetN();
					fGLaBr2Raw->SetPoint(N_Raw2,fGraphTime,st.rateRaw);
					int N_Acc2 = fGLaBr2Accepted->GetN();
					fGLaBr2Accepted->SetPoint(N_Acc2,fGraphTime,st.rateAccepted);
					int N_Dead2 = fGLaBr2Deadtime->GetN();
					fGLaBr2Deadtime->SetPoint(N_Dead2,fGraphTime,st.deadtime);

					break;
				}

				case 11:
				{
					fGRLaBr3Raw->Fill(st.rateRaw,0,0);
					fGRLaBr3Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr3Deadtime->Fill(st.deadtime,0,0);

					int N_Raw3 = fGLaBr3Raw->GetN();
					fGLaBr3Raw->SetPoint(N_Raw3,fGraphTime,st.rateRaw);
					int N_Acc3 = fGLaBr3Accepted->GetN();
					fGLaBr3Accepted->SetPoint(N_Acc3,fGraphTime,st.rateAccepted);
					int N_Dead3 = fGLaBr3Deadtime->GetN();
					fGLaBr3Deadtime->SetPoint(N_Dead3,fGraphTime,st.deadtime);
					break;
				}

				case 12:
				{
					fGRLaBr4Raw->Fill(st.rateRaw,0,0);
					fGRLaBr4Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr4Deadtime->Fill(st.deadtime,0,0);

					int N_Raw4 = fGLaBr4Raw->GetN();
					fGLaBr4Raw->SetPoint(N_Raw4,fGraphTime,st.rateRaw);
					int N_Acc4 = fGLaBr4Accepted->GetN();
					fGLaBr4Accepted->SetPoint(N_Acc4,fGraphTime,st.rateAccepted);
					int N_Dead4 = fGLaBr4Deadtime->GetN();
					fGLaBr4Deadtime->SetPoint(N_Dead4,fGraphTime,st.deadtime);
					break;
				}

				case 13:
				{
					fGRLaBr5Raw->Fill(st.rateRaw,0,0);
					fGRLaBr5Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr5Deadtime->Fill(st.deadtime,0,0);

					int N_Raw5 = fGLaBr5Raw->GetN();
					fGLaBr5Raw->SetPoint(N_Raw5,fGraphTime,st.rateRaw);
					int N_Acc5 = fGLaBr5Accepted->GetN();
					fGLaBr5Accepted->SetPoint(N_Acc5,fGraphTime,st.rateAccepted);
					int N_Dead5 = fGLaBr5Deadtime->GetN();
					fGLaBr5Deadtime->SetPoint(N_Dead5,fGraphTime,st.deadtime);
					break;
				}
			}
		}




		if(sc.module == 1 && ch >=8 && ch <=10)
		{
			int idx = ch-8+6;

			switch(idx)
			{
				case 6:
				{
					fGRLaBr6Raw->Fill(st.rateRaw,0,0);
					fGRLaBr6Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr6Deadtime->Fill(st.deadtime,0,0);

					int N_Raw6 = fGLaBr6Raw->GetN();
					fGLaBr6Raw->SetPoint(N_Raw6,fGraphTime,st.rateRaw);
					int N_Acc6 = fGLaBr6Accepted->GetN();
					fGLaBr6Accepted->SetPoint(N_Acc6,fGraphTime,st.rateAccepted);
					int N_Dead6 = fGLaBr6Deadtime->GetN();
					fGLaBr6Deadtime->SetPoint(N_Dead6,fGraphTime,st.deadtime);
					break;
				}

				case 7:
				{
					fGRLaBr7Raw->Fill(st.rateRaw,0,0);
					fGRLaBr7Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr7Deadtime->Fill(st.deadtime,0,0);

					int N_Raw7 = fGLaBr7Raw->GetN();
					fGLaBr7Raw->SetPoint(N_Raw7,fGraphTime,st.rateRaw);
					int N_Acc7 = fGLaBr7Accepted->GetN();
					fGLaBr7Accepted->SetPoint(N_Acc7,fGraphTime,st.rateAccepted);
					int N_Dead7 = fGLaBr7Deadtime->GetN();
					fGLaBr7Deadtime->SetPoint(N_Dead7,fGraphTime,st.deadtime);
					break;
				}
	
				case 8:
				{
					fGRLaBr8Raw->Fill(st.rateRaw,0,0);
					fGRLaBr8Accepted->Fill(st.rateAccepted,0,0);
					fGRLaBr8Deadtime->Fill(st.deadtime,0,0);
	
					int N_Raw8 = fGLaBr8Raw->GetN();
					fGLaBr8Raw->SetPoint(N_Raw8,fGraphTime,st.rateRaw);
					int N_Acc8 = fGLaBr8Accepted->GetN();
					fGLaBr8Accepted->SetPoint(N_Acc8,fGraphTime,st.rateAccepted);
					int N_Dead8 = fGLaBr8Deadtime->GetN();
					fGLaBr8Deadtime->SetPoint(N_Dead8,fGraphTime,st.deadtime);
					break;
				}
			}
		}


		if(sc.module==1 && ch==11)
		{
			fGRForwardLaBrRaw->Fill(st.rateRaw,0,0);
			fGRForwardLaBrAccepted->Fill(st.rateAccepted,0,0);
			fGRForwardLaBrDeadtime->Fill(st.deadtime,0,0);

			int N_RawForw = fGForwardLaBrRaw->GetN();
			fGForwardLaBrRaw->SetPoint(N_RawForw,fGraphTime,st.rateRaw);
			int N_AccForw = fGForwardLaBrAccepted->GetN();
			fGForwardLaBrAccepted->SetPoint(N_AccForw,fGraphTime,st.rateAccepted);
			int N_DeadForw = fGForwardLaBrDeadtime->GetN();
			fGForwardLaBrDeadtime->SetPoint(N_DeadForw,fGraphTime,st.deadtime);

			fSumForwardRaw += st.rateRaw;
			fSumForwardAccepted += st.rateAccepted;
			fSumForwardDeadtime += st.deadtime;
		}



		if(sc.module==1 && ch==15)
		{
			fGRCherenkovRaw->Fill(st.rateRaw,0,0);
			fGRCherenkovAccepted->Fill(st.rateAccepted,0,0);
			fGRCherenkovDeadtime->Fill(st.deadtime,0,0);

			int N_RawCher = fGCherenkovRaw->GetN();
			fGCherenkovRaw->SetPoint(N_RawCher,fGraphTime,st.rateRaw);
			int N_AccCher = fGCherenkovAccepted->GetN();
			fGCherenkovAccepted->SetPoint(N_AccCher,fGraphTime,st.rateAccepted);
			int N_DeadCher = fGCherenkovDeadtime->GetN();
			fGCherenkovDeadtime->SetPoint(N_DeadCher,fGraphTime,st.deadtime);


			fSumCherenkovRaw += st.rateRaw;
			fSumCherenkovAccepted += st.rateAccepted;
			fSumCherenkovDeadtime += st.deadtime;
		}


		// TODO: Plastic???



	        /*
	         * Rolling Graph Summen
	         */
	        if(ch>=8 && ch<=13)
	        {

			int idx=-1;


			if(sc.module==0) { idx = ch-8; }

			else if(sc.module==1) { idx = ch-8+6; }



			if(idx>=0 && idx<9)
			{
				fSumLaBrRaw[idx] += st.rateRaw;
				fSumLaBrAccepted[idx] += st.rateAccepted;
				fSumLaBrDeadtime[idx] += st.deadtime;
			}
		}



	        // Forward LaBr
	        if(sc.module==1 && ch==11)
	        {
			fSumForwardRaw += st.rateRaw;
			fSumForwardAccepted += st.rateAccepted;
			fSumForwardDeadtime += st.deadtime;
	        }


		// Cherenkov
		if(sc.module==1 && ch==15)
		{
			fSumCherenkovRaw += st.rateRaw;
			fSumCherenkovAccepted += st.rateAccepted;
			fSumCherenkovDeadtime += st.deadtime;
		}


		// Plastic
		if(sc.module==0 && ch==15)
		{
			fSumPlasticRaw += st.rateRaw;
			fSumPlasticAccepted += st.rateAccepted;
			fSumPlasticDeadtime += st.deadtime;
		}
	}



	// alten Snapshot speichern
	state.prevClock = clock;
	state.prevRaw = sc.raw;
	state.prevAccepted = sc.accepted;
}





DagHit TDagobertHitProc::BuildHit(const RawGammaHit& gh)
{
	DagHit h;
	h.module = gh.module;
	h.channel = gh.channel;

	/** scaler uebernehmen */
	h.hasScaler = gh.hasScaler;

	/** Zeitkorrektur (MAW-Interpolation)  */
	h.time = CorrectTimestamp(
			gh.timestamp,
			gh.maxMAW,
			gh.beforeMAW,
			gh.afterMAW);

	/** Energie (Standard: GateH - 2*GateG) */
	h.energy = EnergyFromGates(gh.firstGateSum, gh.secondGateSum);

	/** pileup uebernehmen */
	h.pileup = gh.pileup;
	
	return h;
}

/** berechne zeit in ns aus timestamps */
double TDagobertHitProc::CorrectTimestamp(
		uint64_t ts,
		uint32_t maxMAW /** max moving average */,
		uint32_t beforeMAW /** before moving average */,
		uint32_t afterMAW /** after moving average */) const
{
	double ts_raw = double(ts);

	/** berechne: diff_max_after / diff_before_after 
	 * --> wie weit zwischen takten liegt triggerzeitpunkt? 
	 * --> Takt-Abstand: CYCLE_LENGTH 
	 *
	 * offset 0x8000000: sis modul speichert wert unsigned durch addition von offset --> muss offset jeweils abzeihen */
	double diff_max_after = (double(maxMAW) - 0x8000000) / 2.0 - (double(afterMAW) - 0x8000000);
	double diff_before_after = (double(beforeMAW) - 0x8000000) - (double(afterMAW) - 0x8000000);
	double ts_corr = ts_raw - diff_max_after / diff_before_after;

	return ts_corr * CYCLE_LENGTH;
}


/** simple energieberechnung --> nicht ueber traces */
double TDagobertHitProc::EnergyFromGates(
		uint32_t gateG /** Baseline */,
		uint32_t gateH /** Baseline + Signal */) const
{
	/** baseline in H laenger integriet -> faktor 2 */
	return (double(gateH) - 2.0 * double(gateG));
}



void TDagobertHitProc::FillHistograms(TDagobertHitEvent* evt)
{
	// LaBr 0
	if(evt->laBr[0].time > 0)
	{
		fHLaBr0VsMeanSzint->Fill(evt->laBr[0].dtToMeanSzintTime);
		fHLaBr0VsCoinc->Fill(evt->laBr[0].dtToLaBrCoincTime);
		fHRawEnergyLaBr0->Fill(evt->laBr[0].energy);
		fHCaliEnergyLaBr0->Fill(evt->laBr[0].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[0].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[0].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[0].dtToLaBrCoincTime);
	}
	// LaBr 1
	if(evt->laBr[1].time > 0)
	{
		fHLaBr1VsMeanSzint->Fill(evt->laBr[1].dtToMeanSzintTime);
		fHLaBr1VsCoinc->Fill(evt->laBr[1].dtToLaBrCoincTime);
		fHRawEnergyLaBr1->Fill(evt->laBr[1].energy);
		fHCaliEnergyLaBr1->Fill(evt->laBr[1].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[1].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[1].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[1].dtToLaBrCoincTime);
	}
	// LaBr 2
	if(evt->laBr[2].time > 0)

	{
		fHLaBr2VsMeanSzint->Fill(evt->laBr[2].dtToMeanSzintTime);
		fHLaBr2VsCoinc->Fill(evt->laBr[2].dtToLaBrCoincTime);
		fHRawEnergyLaBr2->Fill(evt->laBr[2].energy);
		fHCaliEnergyLaBr2->Fill(evt->laBr[2].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[2].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[2].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[2].dtToLaBrCoincTime);
	}
	// LaBr 3
	if(evt->laBr[3].time > 0)
	{
		fHLaBr3VsMeanSzint->Fill(evt->laBr[3].dtToMeanSzintTime);
		fHLaBr3VsCoinc->Fill(evt->laBr[3].dtToLaBrCoincTime);
		fHRawEnergyLaBr3->Fill(evt->laBr[3].energy);
		fHCaliEnergyLaBr3->Fill(evt->laBr[3].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[3].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[3].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[3].dtToLaBrCoincTime);
	}
	// LaBr 4
	if(evt->laBr[4].time > 0)
	{
		fHLaBr4VsMeanSzint->Fill(evt->laBr[4].dtToMeanSzintTime);
		fHLaBr4VsCoinc->Fill(evt->laBr[4].dtToLaBrCoincTime);
		fHRawEnergyLaBr4->Fill(evt->laBr[4].energy);
		fHCaliEnergyLaBr4->Fill(evt->laBr[4].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[4].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[4].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[4].dtToLaBrCoincTime);
	}
	// LaBr 5
	if(evt->laBr[5].time > 0)
	{
		fHLaBr5VsMeanSzint->Fill(evt->laBr[5].dtToMeanSzintTime);
		fHLaBr5VsCoinc->Fill(evt->laBr[5].dtToLaBrCoincTime);
		fHRawEnergyLaBr5->Fill(evt->laBr[5].energy);
		fHCaliEnergyLaBr5->Fill(evt->laBr[5].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[5].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[5].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[5].dtToLaBrCoincTime);
	}
	// LaBr 6
	if(evt->laBr[6].time > 0)
	{
		fHLaBr6VsMeanSzint->Fill(evt->laBr[6].dtToMeanSzintTime);
		fHLaBr6VsCoinc->Fill(evt->laBr[6].dtToLaBrCoincTime);
		fHRawEnergyLaBr6->Fill(evt->laBr[6].energy);
		fHCaliEnergyLaBr6->Fill(evt->laBr[6].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[6].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[6].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[6].dtToLaBrCoincTime);
	}
	// LaBr 7
	if(evt->laBr[7].time > 0)

	{
		fHLaBr7VsMeanSzint->Fill(evt->laBr[7].dtToMeanSzintTime);
		fHLaBr7VsCoinc->Fill(evt->laBr[7].dtToLaBrCoincTime);
		fHRawEnergyLaBr7->Fill(evt->laBr[7].energy);
		fHCaliEnergyLaBr7->Fill(evt->laBr[7].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[7].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[7].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[7].dtToLaBrCoincTime);
	}
	// LaBr 8
	if(evt->laBr[8].time > 0)
	{
		fHLaBr8VsMeanSzint->Fill(evt->laBr[8].dtToMeanSzintTime);
		fHLaBr8VsCoinc->Fill(evt->laBr[8].dtToLaBrCoincTime);
		fHRawEnergyLaBr8->Fill(evt->laBr[8].energy);
		fHCaliEnergyLaBr8->Fill(evt->laBr[8].calibrated_energy);
		fHCaliEnergyLaBrSum->Fill(evt->laBr[8].calibrated_energy);
		fHLaBrVsMeanSzintSum->Fill(evt->laBr[8].dtToMeanSzintTime);
		fHLaBrVsCoincSum->Fill(evt->laBr[8].dtToLaBrCoincTime);
	}
	// vorwaerts LaBr
	if(evt->forwardLaBr.time > 0)
	{
		fHForwardLaBrVsMeanSzint->Fill(evt->forwardLaBr.dtToMeanSzintTime);
		fHForwardLaBrVsCoinc->Fill(evt->forwardLaBr.dtToLaBrCoincTime);
		fHRawEnergyLaBrForward->Fill(evt->forwardLaBr.energy);
		fHCaliEnergyLaBrForward->Fill(evt->forwardLaBr.calibrated_energy);
	}
	// Vorwaerts Cherenkov
	if(evt->cherenkov.time > 0)
	{
		fHCherenkovVsMeanSzint->Fill(evt->cherenkov.dtToMeanSzintTime);
		fHCherenkovVsCoinc->Fill(evt->cherenkov.dtToLaBrCoincTime);
	}
	// Vorwaerts Plastic Szint (nur 2025)
	if(evt->plastic.time > 0)
	{
		fHPlasticVsMeanSzint->Fill(evt->plastic.dtToMeanSzintTime);
		fHPlasticVsCoinc->Fill(evt->plastic.dtToLaBrCoincTime);
	}
}

