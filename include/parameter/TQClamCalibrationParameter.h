#ifndef TQCLAMCALIBRATIONPARAMETER_H
#define TQCLAMCALIBRATIONPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"


#define __DATANAME__ "data"


class TGraph;
class TH1;
class TGo4Fitter;

class TQClamCalibrationParameter : public TGo4Parameter
{
	public:
		TQClamCalibrationParameter();
		TQClamCalibrationParameter(const char* name, TH1* spectrum);// TGraph* curve);
		virtual ~TQClamCalibrationParameter();

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


		/** Fit-Flags (in GUI setzen) */
		Bool_t fDoFit{kFALSE}; // Soll regelmaessig ein Fit ausgefuehrt werden?
		/** Fit-Update Interval */
		Int_t fFitUpdateEventInterval {100000}; // Event Update-Interval zum Updaten des Fits

		Bool_t fUseGaussLineFitter{kFALSE}; // Fitte einen normalen Gauss
		Bool_t fUseSimpleLineFitter{kFALSE}; // Fitte einen 1/x-Tail
		Bool_t fUseEMGLineFitter{kFALSE}; // Fitte EMG
		Bool_t fUseGaussTailLineFitter{kFALSE}; // Fitte Gauss mit abfallendem 1/x-Tail

		/** Gauss-Parameter */
		Double_t fGauss_Amp{0.}; // Gauss: Amplitude
		Double_t fGauss_Mean{0.}; // Gauss: Mittelwert
		Double_t fGauss_Sigma{0.}; // Gauss: Stdw

		/** EMG-Parameter */
		Double_t fEMG_Amp{0.}; // EMG: Amplitude
		Double_t fEMG_Lambda{0.}; // EMG-Tail
		Double_t fEMG_Mean{0.}; // EMG: Mittelwert
		Double_t fEMG_Sigma{0.}; // EMG: Stdw

		/** 1/x-Tail */
		Double_t fTail_Amp{0.}; // Tail: Amplitude
		Double_t fTail_Mean{0.}; // Tail: Mittelwert
		Double_t fTail_Eps{0.}; // Tail: Eps

		/** Gauss mit 1/x-Tail */
		Double_t fGaussTail_GAmp{0.}; // Gauss mit Tail: Amplitude Gauss
		Double_t fGaussTail_GMean{0.}; // Gauss mit Tail: Mittelwert Gauss
		Double_t fGaussTail_GSigma{0.}; // Gauss mit Tail: Stdw Gauss

		Double_t fGaussTail_TAmp{0.}; // Gauss mit Tail: Amplitude Tail
		Double_t fGaussTail_TMean{0.}; // Gauss mit Tail: Mittelwert Tail
		Double_t fGaussTail_TEps{0.}; // Gauss mit Tail: Eps Tail


		/** Startwerte */
		Double_t fStartMean{50.}; // Startwert, Mittelwert
		Double_t fStartSigma{0.5}; // Startwert, Stdw
		Double_t fStartAmp{100.}; // Startwert, Amplitude
		Double_t fStartEps{0.5}; // Startwert, Eps
		Double_t fStartLambda{0.1}; // Startwert, Lambda

		/** Fitter */
		TGo4Fitter* fFitter{nullptr}; // Fit-Funktions

		/** Pointer auf Spektrum --> nicht streamen */
		TH1* fSpectrum{nullptr}; //!

		TString fSpectrumName; // Name des Spektrums




		void SetSpectrum(TH1* h);
		TGo4Fitter* GetFitter() const { return fFitter; }

	ClassDefOverride(TQClamCalibrationParameter,1);
};

#endif
