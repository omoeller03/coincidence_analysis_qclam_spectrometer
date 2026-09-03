#ifndef TQCLAMDAGOBERTCOINCIDENCEPROC_H
#define TQCLAMDAGOBERTCOINCIDENCEPROC_H


#include "TGo4EventProcessor.h"

/** ROOT */
class TH1;
class TH2;

/** Go4 */
class TGo4CondArray;
class TGo4Picture;

/** evts */
class TQClamDagobertCoincidenceEvent;

/** parameter */
class TDagobertCalibrationParameter;
class TQClamHistogramParameter;


class TQClamDagobertCoincidenceProc : public TGo4EventProcessor
{
	public:
		TQClamDagobertCoincidenceProc();
		TQClamDagobertCoincidenceProc(const char* name);
		virtual ~TQClamDagobertCoincidenceProc();


		Bool_t BuildEvent(TGo4EventElement* dest) override;

		int fEvtCounter = 0;
		int fTrackCounter = 0; // (nur konvergierte Tracks)

		int fLaBrORCounter = 0;
		int fLaBr0Counter = 0;
		int fLaBr1Counter = 0;
		int fLaBr2Counter = 0;
		int fLaBr3Counter = 0;
		int fLaBr4Counter = 0;
		int fLaBr5Counter = 0;
		int fLaBr6Counter = 0;
		int fLaBr7Counter = 0;
		int fLaBr8Counter = 0;



	private:
		void FillHistograms(TQClamDagobertCoincidenceEvent* evt);
	private:
		//-----------------------------------------------------------
		// 1. Parameter 
		TDagobertCalibrationParameter* fDagoCalibParam{nullptr};
		TQClamHistogramParameter* fHistoPar{nullptr};

		//-----------------------------------------------------------
		// 2. Condition
		TGo4CondArray* fWinCondLaBrVsMeanSzintCorr{nullptr};
		
		//-----------------------------------------------------------
		// 3. Histogramme

		/** Koinzidenz-Effizienzen */
		TH1* fHRelLaBrORCoinc{nullptr};

		TH1* fHRelLaBr0_to_8_CoincPerEvent{nullptr};
		TH1* fHRelLaBr0_to_8_CoincPerTrack{nullptr};

		TH1* fHLaBr0TimeWalk{nullptr};
		TH1* fHLaBr1TimeWalk{nullptr};
		TH1* fHLaBr2TimeWalk{nullptr};
		TH1* fHLaBr3TimeWalk{nullptr};
		TH1* fHLaBr4TimeWalk{nullptr};
		TH1* fHLaBr5TimeWalk{nullptr};
		TH1* fHLaBr6TimeWalk{nullptr};
		TH1* fHLaBr7TimeWalk{nullptr};
		TH1* fHLaBr8TimeWalk{nullptr};

                /** timing relativ zu Mean(SzintUp,SzintDown) */
                TH1* fHLaBr0VsMeanSzintCorr{nullptr};
                TH1* fHLaBr1VsMeanSzintCorr{nullptr};
                TH1* fHLaBr2VsMeanSzintCorr{nullptr};
                TH1* fHLaBr3VsMeanSzintCorr{nullptr};
                TH1* fHLaBr4VsMeanSzintCorr{nullptr};
                TH1* fHLaBr5VsMeanSzintCorr{nullptr};
                TH1* fHLaBr6VsMeanSzintCorr{nullptr};
                TH1* fHLaBr7VsMeanSzintCorr{nullptr};
                TH1* fHLaBr8VsMeanSzintCorr{nullptr};
                /** timing relativ zur ODER LaBr-Koinzidenz */
                TH1* fHLaBr0VsCoincCorr{nullptr};
                TH1* fHLaBr1VsCoincCorr{nullptr};
                TH1* fHLaBr2VsCoincCorr{nullptr};
                TH1* fHLaBr3VsCoincCorr{nullptr};
                TH1* fHLaBr4VsCoincCorr{nullptr};
                TH1* fHLaBr5VsCoincCorr{nullptr};
                TH1* fHLaBr6VsCoincCorr{nullptr};
                TH1* fHLaBr7VsCoincCorr{nullptr};
                TH1* fHLaBr8VsCoincCorr{nullptr};

		TH1* fHToF{nullptr};

		/// == Matrizen gegen x ==

		// Matrizen ohne Gate
		TH2* EEPGMatrix_LaBr0{nullptr};
		TH2* EEPGMatrix_LaBr1{nullptr};
		TH2* EEPGMatrix_LaBr2{nullptr};
		TH2* EEPGMatrix_LaBr3{nullptr};
		TH2* EEPGMatrix_LaBr4{nullptr};
		TH2* EEPGMatrix_LaBr5{nullptr};
		TH2* EEPGMatrix_LaBr6{nullptr};
		TH2* EEPGMatrix_LaBr7{nullptr};
		TH2* EEPGMatrix_LaBr8{nullptr};

		// Gate auf Koinzidenzzeit
		TH2* EEPGMatrixPrompt_LaBr0{nullptr};
		TH2* EEPGMatrixPrompt_LaBr1{nullptr};
		TH2* EEPGMatrixPrompt_LaBr2{nullptr};
		TH2* EEPGMatrixPrompt_LaBr3{nullptr};
		TH2* EEPGMatrixPrompt_LaBr4{nullptr};
		TH2* EEPGMatrixPrompt_LaBr5{nullptr};
		TH2* EEPGMatrixPrompt_LaBr6{nullptr};
		TH2* EEPGMatrixPrompt_LaBr7{nullptr};
		TH2* EEPGMatrixPrompt_LaBr8{nullptr};

		// Gate auf Background (Coinc TimePeak)
		TH2* EEPGMatrixBkgrd_LaBr0{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr1{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr2{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr3{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr4{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr5{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr6{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr7{nullptr};
		TH2* EEPGMatrixBkgrd_LaBr8{nullptr};


		/// === Matrizen gegen Elektron-Energie

		// Matrizen ohne Gate
		TH2* EEPGMatrixCalib_LaBr0{nullptr};
		TH2* EEPGMatrixCalib_LaBr1{nullptr};
		TH2* EEPGMatrixCalib_LaBr2{nullptr};
		TH2* EEPGMatrixCalib_LaBr3{nullptr};
		TH2* EEPGMatrixCalib_LaBr4{nullptr};
		TH2* EEPGMatrixCalib_LaBr5{nullptr};
		TH2* EEPGMatrixCalib_LaBr6{nullptr};
		TH2* EEPGMatrixCalib_LaBr7{nullptr};
		TH2* EEPGMatrixCalib_LaBr8{nullptr};

		// Gate auf Koinzidenzzeit
		TH2* EEPGMatrixCalibPrompt_LaBr0{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr1{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr2{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr3{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr4{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr5{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr6{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr7{nullptr};
		TH2* EEPGMatrixCalibPrompt_LaBr8{nullptr};

		// Gate auf Background (Coinc TimePeak)
		TH2* EEPGMatrixCalibBkgrd_LaBr0{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr1{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr2{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr3{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr4{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr5{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr6{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr7{nullptr};
		TH2* EEPGMatrixCalibBkgrd_LaBr8{nullptr};


                //-----------------------------------------------------------
                // 5. Pictures
		TGo4Picture* fPCoincStatistics{nullptr};
		TGo4Picture* fPCoincMatrices{nullptr};
		TGo4Picture* fPCoincMatricesCalib{nullptr};
		TGo4Picture* fPCoincMatricesPrompt{nullptr};
		TGo4Picture* fPCoincMatricesCalibPrompt{nullptr};
		TGo4Picture* fPCoincMatricesBkgrd{nullptr};
		TGo4Picture* fPCoincMatricesCalibBkgrd{nullptr};
		TGo4Picture* fPLaBrTimeWalk{nullptr};
		TGo4Picture* fPLaBrCoincTimeMeanSzint{nullptr};

	ClassDefOverride(TQClamDagobertCoincidenceProc,1);
};


#endif
