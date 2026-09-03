#include "processor/TQClamDagobertCoincidenceProc.h"


/** events */
#include "evt/TQClamTrackEvent.h"
#include "evt/TDagobertHitEvent.h"
#include "evt/TQClamDagobertCoincidenceEvent.h"

/** Go4 includes */
#include "TGo4Log.h"
#include "TGo4Picture.h"
#include "TGo4CondArray.h"

/** ROOT includes */
#include "TH1.h"
#include "TH2.h"

/** parameter */
#include "parameter/TDagobertCalibrationParameter.h"
#include "parameter/TQClamHistogramParameter.h"

ClassImp(TQClamDagobertCoincidenceProc)


//***********************************************************
TQClamDagobertCoincidenceProc::TQClamDagobertCoincidenceProc()
	: TGo4EventProcessor("QClamDagoCoincProc")
{
	TGo4Log::Info("[TQClamDagobertCoincidenceProc] Create QClamDagoCoincProc");
}

//***********************************************************
TQClamDagobertCoincidenceProc::TQClamDagobertCoincidenceProc(const char* name)
	: TGo4EventProcessor(name)
{
	TGo4Log::Info("[TQClamDagobertCoincidenceProc] Create %s",name);


	fEvtCounter = 0;
	fTrackCounter = 0; // (nur konvergierte Tracks)

	fLaBrORCounter = 0;
	fLaBr0Counter = 0;
	fLaBr1Counter = 0;
	fLaBr2Counter = 0;
	fLaBr3Counter = 0;
	fLaBr4Counter = 0;
	fLaBr5Counter = 0;
	fLaBr6Counter = 0;
	fLaBr7Counter = 0;
	fLaBr8Counter = 0;


	//-----------------------------------------------------------
	// 1. initialisiere Parameter
	fDagoCalibParam = (TDagobertCalibrationParameter*) GetParameter("DagoCalibParam");
	fHistoPar = (TQClamHistogramParameter*) GetParameter("HistoParam");


	//-----------------------------------------------------------
	// 2. initialisiere Conditions
	if(!fDagoCalibParam)
{
   TGo4Log::Error("fDagoCalibParam is NULL!");
   return;
}

	fWinCondLaBrVsMeanSzintCorr = (TGo4CondArray*)GetAnalysisCondition("CondArrCoincPrompt");
	if(!fWinCondLaBrVsMeanSzintCorr)
	{
		fWinCondLaBrVsMeanSzintCorr = new TGo4CondArray("CondArrCoincPrompt",9,"TGo4WinCond");
		fWinCondLaBrVsMeanSzintCorr->SetValues(-10.,10.); /** default: -10 ns ... +10 ns */
		fWinCondLaBrVsMeanSzintCorr->Enable();
		TGo4Log::Info("SetCondition");
		for(int i=0; i<9; i++)
		{
			((*fWinCondLaBrVsMeanSzintCorr)[i])->SetValues(
				fDagoCalibParam->minCoincTimeWin[i],
				fDagoCalibParam->maxCoincTimeWin[i]);
			AddAnalysisCondition((*fWinCondLaBrVsMeanSzintCorr)[i]);
		}
	}
	else
	{
		TGo4Log::Info("[TQClamDagobertCoincidenceProc] Lade CondArrCoincPrompt aus Autosave-Datei.");
		fWinCondLaBrVsMeanSzintCorr->ResetCounts();
	}


	//-----------------------------------------------------------
	// 3. initialisiere Histogramme
	fHRelLaBrORCoinc = MakeTH1('D',"RelLaBrORCoinc","Relative Koinzidenzen (LaBr-OR)",2,0,2);
	fHRelLaBrORCoinc->GetXaxis()->SetBinLabel(1,"frac_{Evt}");
	fHRelLaBrORCoinc->GetXaxis()->SetBinLabel(2,"frac_{track}");
	fHRelLaBrORCoinc->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHRelLaBrORCoinc->GetYaxis()->CenterTitle();

	fHRelLaBr0_to_8_CoincPerEvent = MakeTH1('D',"RelLaBrCoincPerEvent","Koinzidenzen pro Event",9,0,9);
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(1,"LaBr0");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(2,"LaBr1");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(3,"LaBr2");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(4,"LaBr3");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(5,"LaBr4");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(6,"LaBr5");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(7,"LaBr6");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(8,"LaBr7");
	fHRelLaBr0_to_8_CoincPerEvent->GetXaxis()->SetBinLabel(9,"LaBr8");
	fHRelLaBr0_to_8_CoincPerEvent->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHRelLaBr0_to_8_CoincPerEvent->GetYaxis()->CenterTitle();


	fHRelLaBr0_to_8_CoincPerTrack = MakeTH1('D',"RelLaBrCoincPerTrack","Koinzidenzen pro Track",9,0,9);
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(1,"LaBr0");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(2,"LaBr1");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(3,"LaBr2");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(4,"LaBr3");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(5,"LaBr4");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(6,"LaBr5");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(7,"LaBr6");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(8,"LaBr7");
	fHRelLaBr0_to_8_CoincPerTrack->GetXaxis()->SetBinLabel(9,"LaBr8");
	fHRelLaBr0_to_8_CoincPerTrack->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHRelLaBr0_to_8_CoincPerTrack->GetYaxis()->CenterTitle();


	fHLaBr0TimeWalk = MakeTH1('D',"LaBr0TimeWalk","Time-Walk (LaBr0)",1000,-10,10);
	fHLaBr0TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr0TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr0TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr0TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr1TimeWalk = MakeTH1('D',"LaBr1TimeWalk","Time-Walk (LaBr1)",1000,-10,10);
	fHLaBr1TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr1TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr1TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr1TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr2TimeWalk = MakeTH1('D',"LaBr2TimeWalk","Time-Walk (LaBr2)",1000,-10,10);
	fHLaBr2TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr2TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr2TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr2TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr3TimeWalk = MakeTH1('D',"LaBr3TimeWalk","Time-Walk (LaBr3)",1000,-10,10);
	fHLaBr3TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr3TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr3TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr3TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr4TimeWalk = MakeTH1('D',"LaBr4TimeWalk","Time-Walk (LaBr4)",1000,-10,10);
	fHLaBr4TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr4TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr4TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr4TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr5TimeWalk = MakeTH1('D',"LaBr5TimeWalk","Time-Walk (LaBr5)",1000,-10,10);
	fHLaBr5TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr5TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr5TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr5TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr6TimeWalk = MakeTH1('D',"LaBr6TimeWalk","Time-Walk (LaBr6)",1000,-10,10);
	fHLaBr6TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr6TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr6TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr6TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr7TimeWalk = MakeTH1('D',"LaBr7TimeWalk","Time-Walk (LaBr7)",1000,-10,10);
	fHLaBr7TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr7TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr7TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr7TimeWalk->GetYaxis()->CenterTitle();

	fHLaBr8TimeWalk = MakeTH1('D',"LaBr8TimeWalk","Time-Walk (LaBr8)",1000,-10,10);
	fHLaBr8TimeWalk->GetXaxis()->SetTitle("timewalk / ns");
	fHLaBr8TimeWalk->GetXaxis()->CenterTitle();
	fHLaBr8TimeWalk->GetYaxis()->SetTitle("Haeufigkeit / (0.1 ns)");
	fHLaBr8TimeWalk->GetYaxis()->CenterTitle();




        fHLaBr0VsMeanSzintCorr = MakeTH1('D', "Time_LaBr0_vs_MeanSzintCorr", "LaBr0 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr0VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr0VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr0VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr0VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[0])->SetHistogram("Time_LaBr0_vs_MeanSzintCorr");

        fHLaBr1VsMeanSzintCorr = MakeTH1('D', "Time_LaBr1_vs_MeanSzintCorr", "LaBr1 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr1VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr1VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr1VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr1VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[1])->SetHistogram("Time_LaBr1_vs_MeanSzintCorr");

        fHLaBr2VsMeanSzintCorr = MakeTH1('D', "Time_LaBr2_vs_MeanSzintCorr", "LaBr2 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr2VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr2VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr2VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr2VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[2])->SetHistogram("Time_LaBr2_vs_MeanSzintCorr");

        fHLaBr3VsMeanSzintCorr = MakeTH1('D', "Time_LaBr3_vs_MeanSzintCorr", "LaBr3 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr3VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr3VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr3VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr3VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[3])->SetHistogram("Time_LaBr3_vs_MeanSzintCorr");

        fHLaBr4VsMeanSzintCorr = MakeTH1('D', "Time_LaBr4_vs_MeanSzintCorr", "LaBr4 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr4VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr4VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr4VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr4VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[4])->SetHistogram("Time_LaBr4_vs_MeanSzintCorr");

        fHLaBr5VsMeanSzintCorr = MakeTH1('D', "Time_LaBr5_vs_MeanSzintCorr", "LaBr5 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr5VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr5VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr5VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr5VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[5])->SetHistogram("Time_LaBr5_vs_MeanSzintCorr");

        fHLaBr6VsMeanSzintCorr = MakeTH1('D', "Time_LaBr6_vs_MeanSzintCorr", "LaBr6 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr6VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr6VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr6VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr6VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[6])->SetHistogram("Time_LaBr6_vs_MeanSzintCorr");

        fHLaBr7VsMeanSzintCorr = MakeTH1('D', "Time_LaBr7_vs_MeanSzintCorr", "LaBr7 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr7VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr7VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr7VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr7VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[7])->SetHistogram("Time_LaBr7_vs_MeanSzintCorr");

        fHLaBr8VsMeanSzintCorr = MakeTH1('D', "Time_LaBr8_vs_MeanSzintCorr", "LaBr8 vs MeanSzint (korrigiert)", 1000, -100, 100);
        fHLaBr8VsMeanSzintCorr->GetXaxis()->SetTitle("Zeit/ns");
        fHLaBr8VsMeanSzintCorr->GetXaxis()->CenterTitle();
        fHLaBr8VsMeanSzintCorr->GetYaxis()->SetTitle("Haeufigkeit/(1ns)");
        fHLaBr8VsMeanSzintCorr->GetYaxis()->CenterTitle();
	((*fWinCondLaBrVsMeanSzintCorr)[8])->SetHistogram("Time_LaBr8_vs_MeanSzintCorr");


	fHToF = MakeTH1('D',"ToF","Time-of-Flight",360,-30.,30.);
	fHToF->GetXaxis()->SetTitle("Zeit in ns");
	fHToF->GetXaxis()->CenterTitle();
	fHToF->GetYaxis()->SetTitle("Haeufigkeit / (0.333 ns)");
	fHToF->GetYaxis()->CenterTitle();




	// Matrizen

	// alle Evts
	EEPGMatrix_LaBr0 = MakeTH2('D',"EEPGMatrix_LaBr0","(e,e'g)-Matrix LaBr0",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr0->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr0->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr0->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr0->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr1 = MakeTH2('D',"EEPGMatrix_LaBr1","(e,e'g)-Matrix LaBr1",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr1->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr1->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr1->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr1->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr2 = MakeTH2('D',"EEPGMatrix_LaBr2","(e,e'g)-Matrix LaBr2",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr2->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr2->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr2->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr2->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr3 = MakeTH2('D',"EEPGMatrix_LaBr3","(e,e'g)-Matrix LaBr3",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr3->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr3->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr3->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr3->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr4 = MakeTH2('D',"EEPGMatrix_LaBr4","(e,e'g)-Matrix LaBr4",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr4->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr4->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr4->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr4->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr5 = MakeTH2('D',"EEPGMatrix_LaBr5","(e,e'g)-Matrix LaBr5",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr5->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr5->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr5->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr5->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr6 = MakeTH2('D',"EEPGMatrix_LaBr6","(e,e'g)-Matrix LaBr6",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr6->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr6->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr6->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr6->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr7 = MakeTH2('D',"EEPGMatrix_LaBr7","(e,e'g)-Matrix LaBr7",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr7->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr7->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr7->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr7->GetYaxis()->CenterTitle();

	EEPGMatrix_LaBr8 = MakeTH2('D',"EEPGMatrix_LaBr8","(e,e'g)-Matrix LaBr8",1000,0,1000,4000,0,10);
	EEPGMatrix_LaBr8->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrix_LaBr8->GetXaxis()->CenterTitle();
	EEPGMatrix_LaBr8->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrix_LaBr8->GetYaxis()->CenterTitle();

	// promts
	EEPGMatrixPrompt_LaBr0 = MakeTH2('D',"EEPGMatrixPrompt_LaBr0","(e,e'g)-Matrix LaBr0 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr0->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr0->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr0->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr0->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr1 = MakeTH2('D',"EEPGMatrixPrompt_LaBr1","(e,e'g)-Matrix LaBr1 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr1->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr1->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr1->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr1->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr2 = MakeTH2('D',"EEPGMatrixPrompt_LaBr2","(e,e'g)-Matrix LaBr2 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr2->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr2->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr2->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr2->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr3 = MakeTH2('D',"EEPGMatrixPrompt_LaBr3","(e,e'g)-Matrix LaBr3 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr3->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr3->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr3->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr3->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr4 = MakeTH2('D',"EEPGMatrixPrompt_LaBr4","(e,e'g)-Matrix LaBr4 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr4->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr4->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr4->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr4->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr5 = MakeTH2('D',"EEPGMatrixPrompt_LaBr5","(e,e'g)-Matrix LaBr5 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr5->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr5->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr5->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr5->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr6 = MakeTH2('D',"EEPGMatrixPrompt_LaBr6","(e,e'g)-Matrix LaBr6 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr6->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr6->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr6->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr6->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr7 = MakeTH2('D',"EEPGMatrixPrompt_LaBr7","(e,e'g)-Matrix LaBr7 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr7->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr7->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr7->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr7->GetYaxis()->CenterTitle();

	EEPGMatrixPrompt_LaBr8 = MakeTH2('D',"EEPGMatrixPrompt_LaBr8","(e,e'g)-Matrix LaBr8 (Koinzidenz)",1000,0,1000,4000,0,10);
	EEPGMatrixPrompt_LaBr8->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixPrompt_LaBr8->GetXaxis()->CenterTitle();
	EEPGMatrixPrompt_LaBr8->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixPrompt_LaBr8->GetYaxis()->CenterTitle();

	// Background
	EEPGMatrixBkgrd_LaBr0 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr0","(e,e'g)-Matrix LaBr0 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr0->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr0->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr0->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr0->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr1 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr1","(e,e'g)-Matrix LaBr1 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr1->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr1->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr1->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr1->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr2 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr2","(e,e'g)-Matrix LaBr2 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr2->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr2->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr2->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr2->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr3 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr3","(e,e'g)-Matrix LaBr3 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr3->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr3->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr3->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr3->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr4 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr4","(e,e'g)-Matrix LaBr4 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr4->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr4->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr4->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr4->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr5 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr5","(e,e'g)-Matrix LaBr5 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr5->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr5->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr5->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr5->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr6 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr6","(e,e'g)-Matrix LaBr6 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr6->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr6->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr6->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr6->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr7 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr7","(e,e'g)-Matrix LaBr7 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr7->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr7->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr7->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr7->GetYaxis()->CenterTitle();

	EEPGMatrixBkgrd_LaBr8 = MakeTH2('D',"EEPGMatrixBkgrd_LaBr8","(e,e'g)-Matrix LaBr8 (Untergrund)",1000,0,1000,4000,0,10);
	EEPGMatrixBkgrd_LaBr8->GetXaxis()->SetTitle("QCLAM: x / mm (korrigiert)");
	EEPGMatrixBkgrd_LaBr8->GetXaxis()->CenterTitle();
	EEPGMatrixBkgrd_LaBr8->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixBkgrd_LaBr8->GetYaxis()->CenterTitle();







	// Energie-kalibrierte Elektronen

	EEPGMatrixCalib_LaBr0 = MakeTH2('D',"EEPGMatrixCalib_LaBr0","(e,e'g)-Matrix LaBr0 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr0->GetXaxis()->SetTitle("QCLAM: Energie / MeV");
	EEPGMatrixCalib_LaBr0->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr0->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr0->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr1 = MakeTH2('D',"EEPGMatrixCalib_LaBr1","(e,e'g)-Matrix LaBr1 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr1->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr1->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr1->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr1->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr2 = MakeTH2('D',"EEPGMatrixCalib_LaBr2","(e,e'g)-Matrix LaBr2 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr2->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr2->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr2->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr2->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr3 = MakeTH2('D',"EEPGMatrixCalib_LaBr3","(e,e'g)-Matrix LaBr3 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr3->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr3->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr3->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr3->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr4 = MakeTH2('D',"EEPGMatrixCalib_LaBr4","(e,e'g)-Matrix LaBr4 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr4->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr4->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr4->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr4->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr5 = MakeTH2('D',"EEPGMatrixCalib_LaBr5","(e,e'g)-Matrix LaBr5 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr5->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr5->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr5->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr5->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr6 = MakeTH2('D',"EEPGMatrixCalib_LaBr6","(e,e'g)-Matrix LaBr6 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr6->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr6->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr6->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr6->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr7 = MakeTH2('D',"EEPGMatrixCalib_LaBr7","(e,e'g)-Matrix LaBr7 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr7->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr7->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr7->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr7->GetYaxis()->CenterTitle();

	EEPGMatrixCalib_LaBr8 = MakeTH2('D',"EEPGMatrixCalib_LaBr8","(e,e'g)-Matrix LaBr8 (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalib_LaBr8->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalib_LaBr8->GetXaxis()->CenterTitle();
	EEPGMatrixCalib_LaBr8->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalib_LaBr8->GetYaxis()->CenterTitle();

	// promts
	EEPGMatrixCalibPrompt_LaBr0 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr0","(e,e'g)-Matrix LaBr0 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr0->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr0->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr0->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr0->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr1 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr1","(e,e'g)-Matrix LaBr1 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr1->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr1->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr1->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr1->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr2 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr2","(e,e'g)-Matrix LaBr2 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr2->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr2->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr2->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr2->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr3 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr3","(e,e'g)-Matrix LaBr3 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr3->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr3->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr3->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr3->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr4 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr4","(e,e'g)-Matrix LaBr4 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr4->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr4->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr4->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr4->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr5 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr5","(e,e'g)-Matrix LaBr5 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr5->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr5->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr5->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr5->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr6 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr6","(e,e'g)-Matrix LaBr6 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr6->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr6->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr6->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr6->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr7 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr7","(e,e'g)-Matrix LaBr7 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr7->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr7->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr7->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr7->GetYaxis()->CenterTitle();

	EEPGMatrixCalibPrompt_LaBr8 = MakeTH2('D',"EEPGMatrixCalibPrompt_LaBr8","(e,e'g)-Matrix LaBr8 (Koinzidenz) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibPrompt_LaBr8->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibPrompt_LaBr8->GetXaxis()->CenterTitle();
	EEPGMatrixCalibPrompt_LaBr8->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibPrompt_LaBr8->GetYaxis()->CenterTitle();

	// Background
	EEPGMatrixCalibBkgrd_LaBr0 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr0","(e,e'g)-Matrix LaBr0 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr0->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr0->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr0->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr0->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr1 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr1","(e,e'g)-Matrix LaBr1 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr1->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr1->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr1->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr1->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr2 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr2","(e,e'g)-Matrix LaBr2 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr2->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr2->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr2->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr2->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr3 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr3","(e,e'g)-Matrix LaBr3 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr3->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr3->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr3->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr3->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr4 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr4","(e,e'g)-Matrix LaBr4 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr4->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr4->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr4->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr4->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr5 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr5","(e,e'g)-Matrix LaBr5 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr5->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr5->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr5->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr5->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr6 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr6","(e,e'g)-Matrix LaBr6 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr6->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr6->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr6->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr6->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr7 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr7","(e,e'g)-Matrix LaBr7 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr7->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr7->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr7->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr7->GetYaxis()->CenterTitle();

	EEPGMatrixCalibBkgrd_LaBr8 = MakeTH2('D',"EEPGMatrixCalibBkgrd_LaBr8","(e,e'g)-Matrix LaBr8 (Untergrund) (Kalibrierte Elektron-Energie)",1100,-10,100,4000,0,10);
	EEPGMatrixCalibBkgrd_LaBr8->GetXaxis()->SetTitle("QCLAM: Enerige / MeV");
	EEPGMatrixCalibBkgrd_LaBr8->GetXaxis()->CenterTitle();
	EEPGMatrixCalibBkgrd_LaBr8->GetYaxis()->SetTitle("LaBr-Energie / MeV");
	EEPGMatrixCalibBkgrd_LaBr8->GetYaxis()->CenterTitle();





	// Pictures
	fPCoincStatistics = GetPicture("CoincStatistics");
	if(!fPCoincStatistics)
	{
		fPCoincStatistics = new TGo4Picture("CoincStatistics","Koinzidenzstatistik");
		fPCoincStatistics->SetDivision(2,2);

		fPCoincStatistics->Pic(0,0)->AddObject(fHRelLaBrORCoinc);
		fPCoincStatistics->Pic(1,0)->AddObject(fHRelLaBr0_to_8_CoincPerEvent);
		fPCoincStatistics->Pic(0,1)->AddObject(fHToF);
		fPCoincStatistics->Pic(1,1)->AddObject(fHRelLaBr0_to_8_CoincPerTrack);

		AddPicture(fPCoincStatistics);
	}

	fPLaBrCoincTimeMeanSzint = GetPicture("DagoTimePeakCorr_MeanSzint");
	if(!fPLaBrCoincTimeMeanSzint)
	{
		fPLaBrCoincTimeMeanSzint = new TGo4Picture("DagoTimePeakCorr_MeanSzint","Koinzidenzzeit-Peak (korrigiert) - relativ zu Mean-Szint");
		fPLaBrCoincTimeMeanSzint->SetDivision(3,3);

		fPLaBrCoincTimeMeanSzint->Pic(2,0)->AddObject(fHLaBr0VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(2,0)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[0]);

		fPLaBrCoincTimeMeanSzint->Pic(1,0)->AddObject(fHLaBr1VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(1,0)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[1]);

		fPLaBrCoincTimeMeanSzint->Pic(0,0)->AddObject(fHLaBr2VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(0,0)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[2]);

		fPLaBrCoincTimeMeanSzint->Pic(2,1)->AddObject(fHLaBr3VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(2,1)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[3]);

		fPLaBrCoincTimeMeanSzint->Pic(1,1)->AddObject(fHLaBr4VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(1,1)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[4]);

		fPLaBrCoincTimeMeanSzint->Pic(0,1)->AddObject(fHLaBr5VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(0,1)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[5]);

		fPLaBrCoincTimeMeanSzint->Pic(2,2)->AddObject(fHLaBr6VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(2,2)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[6]);

		fPLaBrCoincTimeMeanSzint->Pic(1,2)->AddObject(fHLaBr7VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(1,2)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[7]);

		fPLaBrCoincTimeMeanSzint->Pic(0,2)->AddObject(fHLaBr8VsMeanSzintCorr);
		fPLaBrCoincTimeMeanSzint->Pic(0,2)->AddCondition((*fWinCondLaBrVsMeanSzintCorr)[8]);

		AddPicture(fPLaBrCoincTimeMeanSzint);
	}

	fPLaBrTimeWalk = GetPicture("LaBrTimeWalk");
	if(!fPLaBrTimeWalk)
	{
		fPLaBrTimeWalk = new TGo4Picture("LaBrTimeWalk","Time-Walk LaBr");
		fPLaBrTimeWalk->SetDivision(3,3);

		fPLaBrTimeWalk->Pic(2,0)->AddObject(fHLaBr0TimeWalk);
		fPLaBrTimeWalk->Pic(1,0)->AddObject(fHLaBr1TimeWalk);
		fPLaBrTimeWalk->Pic(0,0)->AddObject(fHLaBr2TimeWalk);
		fPLaBrTimeWalk->Pic(2,1)->AddObject(fHLaBr3TimeWalk);
		fPLaBrTimeWalk->Pic(1,1)->AddObject(fHLaBr4TimeWalk);
		fPLaBrTimeWalk->Pic(0,1)->AddObject(fHLaBr5TimeWalk);
		fPLaBrTimeWalk->Pic(2,2)->AddObject(fHLaBr6TimeWalk);
		fPLaBrTimeWalk->Pic(1,2)->AddObject(fHLaBr7TimeWalk);
		fPLaBrTimeWalk->Pic(0,2)->AddObject(fHLaBr8TimeWalk);

		AddPicture(fPLaBrTimeWalk);
	}

	fPCoincMatrices = GetPicture("EEPGMatrices");
	if(!fPCoincMatrices)
	{
		fPCoincMatrices = new TGo4Picture("EEPGMatrices","(e,e'g) Matrizen");
		fPCoincMatrices->SetDivision(3,3);

		fPCoincMatrices->Pic(2,0)->AddObject(EEPGMatrix_LaBr0);
		fPCoincMatrices->Pic(1,0)->AddObject(EEPGMatrix_LaBr1);
		fPCoincMatrices->Pic(0,0)->AddObject(EEPGMatrix_LaBr2);
		fPCoincMatrices->Pic(2,1)->AddObject(EEPGMatrix_LaBr3);
		fPCoincMatrices->Pic(1,1)->AddObject(EEPGMatrix_LaBr4);
		fPCoincMatrices->Pic(0,1)->AddObject(EEPGMatrix_LaBr5);
		fPCoincMatrices->Pic(2,2)->AddObject(EEPGMatrix_LaBr6);
		fPCoincMatrices->Pic(1,2)->AddObject(EEPGMatrix_LaBr7);
		fPCoincMatrices->Pic(0,2)->AddObject(EEPGMatrix_LaBr8);

		AddPicture(fPCoincMatrices);
	}


	fPCoincMatricesCalib = GetPicture("EEPGMatricesCalib");
	if(!fPCoincMatricesCalib)
	{
		fPCoincMatricesCalib = new TGo4Picture("EEPGMatricesCalib","(e,e'g) Matrizen (Kalibrierte Elektron-Energie)");
		fPCoincMatricesCalib->SetDivision(3,3);

		fPCoincMatricesCalib->Pic(2,0)->AddObject(EEPGMatrixCalib_LaBr0);
		fPCoincMatricesCalib->Pic(1,0)->AddObject(EEPGMatrixCalib_LaBr1);
		fPCoincMatricesCalib->Pic(0,0)->AddObject(EEPGMatrixCalib_LaBr2);
		fPCoincMatricesCalib->Pic(2,1)->AddObject(EEPGMatrixCalib_LaBr3);
		fPCoincMatricesCalib->Pic(1,1)->AddObject(EEPGMatrixCalib_LaBr4);
		fPCoincMatricesCalib->Pic(0,1)->AddObject(EEPGMatrixCalib_LaBr5);
		fPCoincMatricesCalib->Pic(2,2)->AddObject(EEPGMatrixCalib_LaBr6);
		fPCoincMatricesCalib->Pic(1,2)->AddObject(EEPGMatrixCalib_LaBr7);
		fPCoincMatricesCalib->Pic(0,2)->AddObject(EEPGMatrixCalib_LaBr8);

		AddPicture(fPCoincMatricesCalib);
	}

	fPCoincMatricesPrompt = GetPicture("EEPGMatricesPrompt");
	if(!fPCoincMatricesPrompt)
	{
		fPCoincMatricesPrompt = new TGo4Picture("EEPGMatricesPrompt","(e,e'g) Matrizen (Gate auf Koinzidenzzeit-Peak)");
		fPCoincMatricesPrompt->SetDivision(3,3);

		fPCoincMatricesPrompt->Pic(2,0)->AddObject(EEPGMatrixPrompt_LaBr0);
		fPCoincMatricesPrompt->Pic(1,0)->AddObject(EEPGMatrixPrompt_LaBr1);
		fPCoincMatricesPrompt->Pic(0,0)->AddObject(EEPGMatrixPrompt_LaBr2);
		fPCoincMatricesPrompt->Pic(2,1)->AddObject(EEPGMatrixPrompt_LaBr3);
		fPCoincMatricesPrompt->Pic(1,1)->AddObject(EEPGMatrixPrompt_LaBr4);
		fPCoincMatricesPrompt->Pic(0,1)->AddObject(EEPGMatrixPrompt_LaBr5);
		fPCoincMatricesPrompt->Pic(2,2)->AddObject(EEPGMatrixPrompt_LaBr6);
		fPCoincMatricesPrompt->Pic(1,2)->AddObject(EEPGMatrixPrompt_LaBr7);
		fPCoincMatricesPrompt->Pic(0,2)->AddObject(EEPGMatrixPrompt_LaBr8);

		AddPicture(fPCoincMatricesPrompt);
	}


	fPCoincMatricesCalibPrompt = GetPicture("EEPGMatricesCalibPrompt");
	if(!fPCoincMatricesCalibPrompt)
	{
		fPCoincMatricesCalibPrompt = new TGo4Picture("EEPGMatricesCalibPrompt","(e,e'g) Matrizen (Gate auf Koinzidenzzeit-Peak) (Kalibrierte Elektron-Energie)");
		fPCoincMatricesCalibPrompt->SetDivision(3,3);

		fPCoincMatricesCalibPrompt->Pic(2,0)->AddObject(EEPGMatrixCalibPrompt_LaBr0);
		fPCoincMatricesCalibPrompt->Pic(1,0)->AddObject(EEPGMatrixCalibPrompt_LaBr1);
		fPCoincMatricesCalibPrompt->Pic(0,0)->AddObject(EEPGMatrixCalibPrompt_LaBr2);
		fPCoincMatricesCalibPrompt->Pic(2,1)->AddObject(EEPGMatrixCalibPrompt_LaBr3);
		fPCoincMatricesCalibPrompt->Pic(1,1)->AddObject(EEPGMatrixCalibPrompt_LaBr4);
		fPCoincMatricesCalibPrompt->Pic(0,1)->AddObject(EEPGMatrixCalibPrompt_LaBr5);
		fPCoincMatricesCalibPrompt->Pic(2,2)->AddObject(EEPGMatrixCalibPrompt_LaBr6);
		fPCoincMatricesCalibPrompt->Pic(1,2)->AddObject(EEPGMatrixCalibPrompt_LaBr7);
		fPCoincMatricesCalibPrompt->Pic(0,2)->AddObject(EEPGMatrixCalibPrompt_LaBr8);

		AddPicture(fPCoincMatricesCalibPrompt);
	}

	fPCoincMatricesBkgrd = GetPicture("EEPGMatricesBkgrd");
	if(!fPCoincMatricesBkgrd)
	{
		fPCoincMatricesBkgrd = new TGo4Picture("EEPGMatricesBkgrd","(e,e'g) Matrizen (Gate auf Hintergrund)");
		fPCoincMatricesBkgrd->SetDivision(3,3);

		fPCoincMatricesBkgrd->Pic(2,0)->AddObject(EEPGMatrixBkgrd_LaBr0);
		fPCoincMatricesBkgrd->Pic(1,0)->AddObject(EEPGMatrixBkgrd_LaBr1);
		fPCoincMatricesBkgrd->Pic(0,0)->AddObject(EEPGMatrixBkgrd_LaBr2);
		fPCoincMatricesBkgrd->Pic(2,1)->AddObject(EEPGMatrixBkgrd_LaBr3);
		fPCoincMatricesBkgrd->Pic(1,1)->AddObject(EEPGMatrixBkgrd_LaBr4);
		fPCoincMatricesBkgrd->Pic(0,1)->AddObject(EEPGMatrixBkgrd_LaBr5);
		fPCoincMatricesBkgrd->Pic(2,2)->AddObject(EEPGMatrixBkgrd_LaBr6);
		fPCoincMatricesBkgrd->Pic(1,2)->AddObject(EEPGMatrixBkgrd_LaBr7);
		fPCoincMatricesBkgrd->Pic(0,2)->AddObject(EEPGMatrixBkgrd_LaBr8);

		AddPicture(fPCoincMatricesBkgrd);
	}


	fPCoincMatricesCalibBkgrd = GetPicture("EEPGMatricesCalibBkgrd");
	if(!fPCoincMatricesCalibBkgrd)
	{
		fPCoincMatricesCalibBkgrd = new TGo4Picture("EEPGMatricesCalibBkgrd","(e,e'g) Matrizen (Gate auf Hintergrund) (Kalibrierte Elektron-Energie)");
		fPCoincMatricesCalibBkgrd->SetDivision(3,3);

		fPCoincMatricesCalibBkgrd->Pic(2,0)->AddObject(EEPGMatrixCalibBkgrd_LaBr0);
		fPCoincMatricesCalibBkgrd->Pic(1,0)->AddObject(EEPGMatrixCalibBkgrd_LaBr1);
		fPCoincMatricesCalibBkgrd->Pic(0,0)->AddObject(EEPGMatrixCalibBkgrd_LaBr2);
		fPCoincMatricesCalibBkgrd->Pic(2,1)->AddObject(EEPGMatrixCalibBkgrd_LaBr3);
		fPCoincMatricesCalibBkgrd->Pic(1,1)->AddObject(EEPGMatrixCalibBkgrd_LaBr4);
		fPCoincMatricesCalibBkgrd->Pic(0,1)->AddObject(EEPGMatrixCalibBkgrd_LaBr5);
		fPCoincMatricesCalibBkgrd->Pic(2,2)->AddObject(EEPGMatrixCalibBkgrd_LaBr6);
		fPCoincMatricesCalibBkgrd->Pic(1,2)->AddObject(EEPGMatrixCalibBkgrd_LaBr7);
		fPCoincMatricesCalibBkgrd->Pic(0,2)->AddObject(EEPGMatrixCalibBkgrd_LaBr8);

		AddPicture(fPCoincMatricesCalibBkgrd);
	}



}

//***********************************************************
TQClamDagobertCoincidenceProc::~TQClamDagobertCoincidenceProc()
{
	TGo4Log::Info("[TQClamDagobertCoincidenceProc] Delete %s",GetName());
}



Bool_t TQClamDagobertCoincidenceProc::BuildEvent(TGo4EventElement* dest)
{
	auto* qclam = dynamic_cast<TQClamTrackEvent*>(GetInputEvent("CoincQClamProvider"));
	auto* dago = dynamic_cast<TDagobertHitEvent*>(GetInputEvent("CoincDagProvider"));

	auto* evt = dynamic_cast<TQClamDagobertCoincidenceEvent*>(dest);

	if(!dago || !qclam || !evt)
	{
		TGo4Log::Warn("[TQClamDagobertCoincidenceProc] Input-Evts existieren nicht. Beende aktuelles Event!");
		evt->SetValid(kFALSE);
		return kFALSE;
	}

	if(!qclam->trackAccepted) { return kFALSE; }

	evt->header = qclam->header;
	evt->scaler = qclam->scaler;
	evt->trigDet = qclam->trigDet;

	evt->fEvtCounter = qclam->fEvtCounter;
	fEvtCounter = qclam->fEvtCounter;
	evt->fTrackCounter = qclam->fConvergedTrackCounter;
	fTrackCounter = qclam->fConvergedTrackCounter;

	// ===============================
	// uebernehme TrackEvent-Daten
	// ===============================
	evt->phi_rad = qclam->phi;
	evt->phi_deg = qclam->phi_deg;
	evt->phi_deg_corr = qclam->phi_corr;
	evt->theta_rad = qclam->theta;
	evt->theta_deg = qclam->theta_deg;

	evt->xD_X1 = qclam->xD_X1;
	evt->xD_U = qclam->xD_U;
	evt->xD_X2 = qclam->xD_X2;
	evt->uD_U = qclam->uD;
	evt->yD_U = qclam->yD_U;
	evt->xD_X1_corr = qclam->x_corr;
	evt->yD_U_corr = qclam->yD_U_corr; // shifted

	evt->el_channel = qclam->channel;
	evt->el_energy_MeV = qclam->e_MeV;
	
	evt->scattX = qclam->scatteringX;
	evt->scattY = qclam->scatteringY;
	evt->scattZ = NAN;
	evt->scattPhi = qclam->scatteringPhi;
	evt->scattTheta = qclam->scatteringTheta;

	// ===============================
	// uebernehme DagoHit-Daten
	// ===============================
	evt->rateEnton = dago->rateEnton;
	evt->rateSzintScattChamber = dago->rateSzintScattChamber;
	evt->rateLaBrOR = dago->rateLaBrOR;
	evt->rateLaBrORNoDead = dago->rateLaBrORNoDead;
	evt->coincTimeLaBr = dago->coincTimeLaBr;
	evt->szintPmtUpTime = dago->szintPmtUpTime;
	evt->szintPmtDownTime = dago->szintPmtDownTime;
	evt->meanSzintTime = dago->meanSzintTime;

	for(int i=0; i<9; i++)
	{
		evt->laBr[i].module = dago->laBr[i].module;
		evt->laBr[i].channel = dago->laBr[i].channel;
		evt->laBr[i].energy = dago->laBr[i].energy;
		evt->laBr[i].calibrated_energy = dago->laBr[i].calibrated_energy;
		evt->laBr[i].time = dago->laBr[i].time;
		evt->laBr[i].dtToMeanSzintTime = dago->laBr[i].dtToMeanSzintTime;
		evt->laBr[i].dtToLaBrCoincTime = dago->laBr[i].dtToLaBrCoincTime;
		evt->laBr[i].pileup = dago->laBr[i].pileup;
		evt->laBr[i].rawScaler = dago->laBr[i].rawScaler;
		evt->laBr[i].accScaler = dago->laBr[i].accScaler;
		evt->laBr[i].vetoScaler = dago->laBr[i].vetoScaler;
		evt->laBr[i].hasScaler = dago->laBr[i].hasScaler;
		evt->laBr[i].rateRaw = dago->laBr[i].rateRaw;
		evt->laBr[i].rateAccepted = dago->laBr[i].rateAccepted;
		evt->laBr[i].deadtime = dago->laBr[i].deadtime;

	}


	// ===============================
	// Analysis
	// ===============================
	double tof = NAN;
	fDagoCalibParam->CorrectToF(
			evt->xD_X1, // X1-Position
			evt->phi_deg, // phi-Winkel
			evt->tof // wird gesetzt
			);
	//TGo4Log::Info("tof=%f",evt->tof);

	bool laBrORCoinc = false;

	/** korrigiere Koinzidenzzeit je LaBr */
	for(int i=0; i<9; i++)
	{
		// time-walk korrektur
		//TGo4Log::Info("detector i=%d",i);

		if(std::isnan(evt->laBr[i].dtToMeanSzintTime)) { continue; }

		if(i==0)
		{
			fLaBr0Counter++;
			laBrORCoinc = true;
		}
		else if(i==1)
		{
			fLaBr1Counter++;
			laBrORCoinc = true;
		}
		else if(i==2)
		{
			fLaBr2Counter++;
			laBrORCoinc = true;
		}
		else if(i==3)
		{
			fLaBr3Counter++;
			laBrORCoinc = true;
		}
		else if(i==4)
		{
			fLaBr4Counter++;
			laBrORCoinc = true;
		}
		else if(i==5)
		{
			fLaBr5Counter++;
			laBrORCoinc = true;
		}
		else if(i==6)
		{
			fLaBr6Counter++;
			laBrORCoinc = true;
		}
		else if(i==7)
		{
			fLaBr7Counter++;
			laBrORCoinc = true;
		}
		else if(i==8)
		{
			fLaBr8Counter++;
			laBrORCoinc = true;
		}

		fDagoCalibParam->CorrectTimeWalk(
				i, // detektor
				evt->laBr[i].calibrated_energy, // laBr-energie
				evt->laBr[i].timewalk // wird gesetzt
				);
		//TGo4Log::Info("timewalk=%f",evt->laBr[i].timewalk);

		// korrigiere koinzidenzzeit (bzgl MeanSzint)
		evt->laBr[i].dtToMeanSzintTime_Corr = evt->laBr[i].dtToMeanSzintTime;
		fDagoCalibParam->CorrectLaBrTimeTotal(
				evt->tof, // tof
				evt->laBr[i].timewalk, // time-walk
				evt->laBr[i].dtToMeanSzintTime_Corr // wird gesetzt
				);

		//TGo4Log::Info("evt->laBr[i].dtToMeanSzintTime=%f",evt->laBr[i].dtToMeanSzintTime);
		//TGo4Log::Info("evt->laBr[i].dtToMeanSzintTime_Corr=%f",evt->laBr[i].dtToMeanSzintTime_Corr);


		// korrigiere koinzidenzzeit (bzgl LaBrCoinc)
		evt->laBr[i].dtToLaBrCoincTime_Corr = evt->laBr[i].dtToLaBrCoincTime;
		fDagoCalibParam->CorrectLaBrTimeTotal(
				evt->tof, // tof
				evt->laBr[i].timewalk, // time-walk
				evt->laBr[i].dtToLaBrCoincTime_Corr // wird gesetzt
				);

	}


	if(laBrORCoinc) { fLaBrORCounter++; }

	evt->fLaBrORCounter = fLaBrORCounter;
	evt->fLaBr0Counter = fLaBr0Counter;
	evt->fLaBr1Counter = fLaBr1Counter;
	evt->fLaBr2Counter = fLaBr2Counter;
	evt->fLaBr3Counter = fLaBr3Counter;
	evt->fLaBr4Counter = fLaBr4Counter;
	evt->fLaBr5Counter = fLaBr5Counter;
	evt->fLaBr6Counter = fLaBr6Counter;
	evt->fLaBr7Counter = fLaBr7Counter;
	evt->fLaBr8Counter = fLaBr8Counter;



	// ===============================
	// histos fuellen
	// ===============================
	if(fHistoPar->fBFillHistos) { FillHistograms(evt); }



	evt->SetValid(kTRUE);
	return kTRUE;
}



void TQClamDagobertCoincidenceProc::FillHistograms(TQClamDagobertCoincidenceEvent* evt)
{
	fHRelLaBrORCoinc->SetBinContent(1, 100.*double(fLaBrORCounter) / double(fEvtCounter));
	fHRelLaBrORCoinc->SetBinContent(2, 100.*double(fLaBrORCounter) / double(fTrackCounter));

	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(1, 100.*double(fLaBr0Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(2, 100.*double(fLaBr1Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(3, 100.*double(fLaBr2Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(4, 100.*double(fLaBr3Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(5, 100.*double(fLaBr4Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(6, 100.*double(fLaBr5Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(7, 100.*double(fLaBr6Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(8, 100.*double(fLaBr7Counter) / double(fEvtCounter));
	fHRelLaBr0_to_8_CoincPerEvent->SetBinContent(9, 100.*double(fLaBr8Counter) / double(fEvtCounter));

	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(1, 100.*double(fLaBr0Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(2, 100.*double(fLaBr1Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(3, 100.*double(fLaBr2Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(4, 100.*double(fLaBr3Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(5, 100.*double(fLaBr4Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(6, 100.*double(fLaBr5Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(7, 100.*double(fLaBr6Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(8, 100.*double(fLaBr7Counter) / double(fTrackCounter));
	fHRelLaBr0_to_8_CoincPerTrack->SetBinContent(9, 100.*double(fLaBr8Counter) / double(fTrackCounter));



	if(!std::isnan(evt->laBr[0].dtToMeanSzintTime_Corr))
	{
		fHLaBr0VsMeanSzintCorr->Fill(evt->laBr[0].dtToMeanSzintTime_Corr);
		fHLaBr0TimeWalk->Fill(evt->laBr[0].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[0])->Test(evt->laBr[0].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr0->Fill(evt->xD_X1_corr,evt->laBr[0].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr0->Fill(evt->el_energy_MeV,evt->laBr[0].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr0->Fill(evt->xD_X1_corr,evt->laBr[0].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr0->Fill(evt->el_energy_MeV,evt->laBr[0].calibrated_energy);
		}
	}

	if(!std::isnan(evt->laBr[1].dtToMeanSzintTime_Corr))
	{
		fHLaBr1VsMeanSzintCorr->Fill(evt->laBr[1].dtToMeanSzintTime_Corr);
		fHLaBr1TimeWalk->Fill(evt->laBr[1].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[1])->Test(evt->laBr[1].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr1->Fill(evt->xD_X1_corr,evt->laBr[1].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr1->Fill(evt->el_energy_MeV,evt->laBr[1].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr1->Fill(evt->xD_X1_corr,evt->laBr[1].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr1->Fill(evt->el_energy_MeV,evt->laBr[1].calibrated_energy);
		}
	}

	if(!std::isnan(evt->laBr[2].dtToMeanSzintTime_Corr))
	{
		fHLaBr2VsMeanSzintCorr->Fill(evt->laBr[2].dtToMeanSzintTime_Corr);
		fHLaBr2TimeWalk->Fill(evt->laBr[2].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[2])->Test(evt->laBr[2].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr2->Fill(evt->xD_X1_corr,evt->laBr[2].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr2->Fill(evt->el_energy_MeV,evt->laBr[2].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr2->Fill(evt->xD_X1_corr,evt->laBr[2].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr2->Fill(evt->el_energy_MeV,evt->laBr[2].calibrated_energy);
		}
	}

	if(!std::isnan(evt->laBr[3].dtToMeanSzintTime_Corr))
	{
		fHLaBr3VsMeanSzintCorr->Fill(evt->laBr[3].dtToMeanSzintTime_Corr);
		fHLaBr3TimeWalk->Fill(evt->laBr[3].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[3])->Test(evt->laBr[3].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr3->Fill(evt->xD_X1_corr,evt->laBr[3].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr3->Fill(evt->el_energy_MeV,evt->laBr[3].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr3->Fill(evt->xD_X1_corr,evt->laBr[3].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr3->Fill(evt->el_energy_MeV,evt->laBr[3].calibrated_energy);
		}
	}
	
	if(!std::isnan(evt->laBr[4].dtToMeanSzintTime_Corr))
	{
		fHLaBr4VsMeanSzintCorr->Fill(evt->laBr[4].dtToMeanSzintTime_Corr);
		fHLaBr4TimeWalk->Fill(evt->laBr[4].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[4])->Test(evt->laBr[4].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr4->Fill(evt->xD_X1_corr,evt->laBr[4].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr4->Fill(evt->el_energy_MeV,evt->laBr[4].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr4->Fill(evt->xD_X1_corr,evt->laBr[4].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr4->Fill(evt->el_energy_MeV,evt->laBr[4].calibrated_energy);
		}
	}

	if(!std::isnan(evt->laBr[5].dtToMeanSzintTime_Corr))
	{
		fHLaBr5VsMeanSzintCorr->Fill(evt->laBr[5].dtToMeanSzintTime_Corr);
		fHLaBr5TimeWalk->Fill(evt->laBr[5].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[5])->Test(evt->laBr[5].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr5->Fill(evt->xD_X1_corr,evt->laBr[5].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr5->Fill(evt->el_energy_MeV,evt->laBr[5].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr5->Fill(evt->xD_X1_corr,evt->laBr[5].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr5->Fill(evt->el_energy_MeV,evt->laBr[5].calibrated_energy);
		}
	}

	if(!std::isnan(evt->laBr[6].dtToMeanSzintTime_Corr))
	{
		fHLaBr6VsMeanSzintCorr->Fill(evt->laBr[6].dtToMeanSzintTime_Corr);
		fHLaBr6TimeWalk->Fill(evt->laBr[6].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[6])->Test(evt->laBr[6].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr6->Fill(evt->xD_X1_corr,evt->laBr[6].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr6->Fill(evt->el_energy_MeV,evt->laBr[6].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr6->Fill(evt->xD_X1_corr,evt->laBr[6].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr6->Fill(evt->el_energy_MeV,evt->laBr[6].calibrated_energy);
		}
	}

	if(!std::isnan(evt->laBr[7].dtToMeanSzintTime_Corr))
	{
		fHLaBr7VsMeanSzintCorr->Fill(evt->laBr[7].dtToMeanSzintTime_Corr);
		fHLaBr7TimeWalk->Fill(evt->laBr[7].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[7])->Test(evt->laBr[7].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr7->Fill(evt->xD_X1_corr,evt->laBr[7].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr7->Fill(evt->el_energy_MeV,evt->laBr[7].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr7->Fill(evt->xD_X1_corr,evt->laBr[7].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr7->Fill(evt->el_energy_MeV,evt->laBr[7].calibrated_energy);
		}
	}

	if(!std::isnan(evt->laBr[8].dtToMeanSzintTime_Corr))
	{
		fHLaBr8VsMeanSzintCorr->Fill(evt->laBr[8].dtToMeanSzintTime_Corr);
		fHLaBr8TimeWalk->Fill(evt->laBr[8].timewalk);

		if( ((*fWinCondLaBrVsMeanSzintCorr)[8])->Test(evt->laBr[8].dtToMeanSzintTime_Corr) )
		{
			EEPGMatrixPrompt_LaBr8->Fill(evt->xD_X1_corr,evt->laBr[8].calibrated_energy);
			EEPGMatrixCalibPrompt_LaBr8->Fill(evt->el_energy_MeV,evt->laBr[8].calibrated_energy);
		}
		else
		{
			EEPGMatrixBkgrd_LaBr8->Fill(evt->xD_X1_corr,evt->laBr[8].calibrated_energy);
			EEPGMatrixCalibBkgrd_LaBr8->Fill(evt->el_energy_MeV,evt->laBr[8].calibrated_energy);
		}
	}

	fHToF->Fill(evt->tof);



	EEPGMatrix_LaBr0->Fill(evt->xD_X1_corr,evt->laBr[0].calibrated_energy);
	EEPGMatrixCalib_LaBr0->Fill(evt->el_energy_MeV,evt->laBr[0].calibrated_energy);

	EEPGMatrix_LaBr1->Fill(evt->xD_X1_corr,evt->laBr[1].calibrated_energy);
	EEPGMatrixCalib_LaBr1->Fill(evt->el_energy_MeV,evt->laBr[1].calibrated_energy);

	EEPGMatrix_LaBr2->Fill(evt->xD_X1_corr,evt->laBr[2].calibrated_energy);
	EEPGMatrixCalib_LaBr2->Fill(evt->el_energy_MeV,evt->laBr[2].calibrated_energy);

	EEPGMatrix_LaBr3->Fill(evt->xD_X1_corr,evt->laBr[3].calibrated_energy);
	EEPGMatrixCalib_LaBr3->Fill(evt->el_energy_MeV,evt->laBr[3].calibrated_energy);

	EEPGMatrix_LaBr4->Fill(evt->xD_X1_corr,evt->laBr[4].calibrated_energy);
	EEPGMatrixCalib_LaBr4->Fill(evt->el_energy_MeV,evt->laBr[4].calibrated_energy);

	EEPGMatrix_LaBr5->Fill(evt->xD_X1_corr,evt->laBr[5].calibrated_energy);
	EEPGMatrixCalib_LaBr5->Fill(evt->el_energy_MeV,evt->laBr[5].calibrated_energy);

	EEPGMatrix_LaBr6->Fill(evt->xD_X1_corr,evt->laBr[6].calibrated_energy);
	EEPGMatrixCalib_LaBr6->Fill(evt->el_energy_MeV,evt->laBr[6].calibrated_energy);

	EEPGMatrix_LaBr7->Fill(evt->xD_X1_corr,evt->laBr[7].calibrated_energy);
	EEPGMatrixCalib_LaBr7->Fill(evt->el_energy_MeV,evt->laBr[7].calibrated_energy);

	EEPGMatrix_LaBr8->Fill(evt->xD_X1_corr,evt->laBr[8].calibrated_energy);
	EEPGMatrixCalib_LaBr8->Fill(evt->el_energy_MeV,evt->laBr[8].calibrated_energy);


}
