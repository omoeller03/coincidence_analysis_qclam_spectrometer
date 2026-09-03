#ifndef TQCLAMFITPARAMETER_H
#define TQCLAMFITPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"


#define __DATANAME__ "data"


class TGraph;
class TH1;
class TGo4Fitter;

class TQClamFitParameter : public TGo4Parameter
{
        public:
                TQClamFitParameter();
                TQClamFitParameter(const char* name, TH1* spectrum);// TGraph* curve);
                virtual ~TQClamFitParameter();

                Bool_t UpdateFrom(TGo4Parameter* rhs) override;

                /** Fitter */
                TGo4Fitter* fFitter{nullptr}; // Fit-Funktions

                /** Startwerte */
                Double_t fStartMean{50.}; // Startwert, Mittelwert
                Double_t fStartSigma{0.5}; // Startwert, Stdw
		Double_t fStartAmp1{1000.}; // Startwert Amp1
		Double_t fStartAmp2{1000.}; // Startwert Amp2
		Double_t fStartEta{2.}; // Startwert Uebergangsparameter
		Double_t fStartDelta{3.}; // Startwert Tail-Parameter



                Double_t fStartAmp{100.}; // Startwert, Amplitude (nicht fuer Spectrum)
                Double_t fStartEps{0.5}; // Startwert, Eps (nicht fuer Spectrum)
                Double_t fStartLambda{0.1}; // Startwert, Lambda (nicht fuer Spectrum)
	

		Bool_t fUseSpectrumFitter{kTRUE}; // Fitte korrektes Modell fuer Zustaende: zweifacher Gauss mit Tail
                Bool_t fUseGaussLineFitter{kFALSE}; // Fitte einen normalen Gauss
                Bool_t fUseSimpleLineFitter{kFALSE}; // Fitte einen 1/x-Tail
                Bool_t fUseEMGLineFitter{kFALSE}; // Fitte EMG
                Bool_t fUseGaussTailLineFitter{kFALSE}; // Fitte Gauss mit abfallendem 1/x-Tail

		/** Spektrum-Modell 2-fach Gauss mit Tail  */
		Double_t fSpectrum_c1{0.}; // Spektrum-Modell: Amplitude Gauss Links
		Double_t fSpectrum_x0{0.}; // Spektrum-Modell: Mittelpunkt
		Double_t fSpectrum_sigma1{0.}; // Spektrum-Modell: Stdw. Gauss Links
		Double_t fSpectrum_sigma2{0.}; // Spektrum-Modell: Stdw. Gauss Rechts
		Double_t fSpectrum_sigmaFull{0.}; // Spektrum-Modell: Stdw. Peak
		Double_t fSpectrum_FWHMFull{0.}; // Spektrum-Modell: Stdw. Peak
		Double_t fSpectrum_c2{0.}; // Spektrum-Modell: Amplitude Gauss Rechts
		Double_t fSpectrum_eta{0.}; // Spektrum-Modell: Uebergangs-Punkt
		Double_t fSpectrum_delta{0.}; // Spektrum-Modell: Tail-Parameter

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


                /** Fit-Flags (in GUI setzen) */
                Bool_t fDoFit{kFALSE}; // Soll regelmaessig ein Fit ausgefuehrt werden?
                /** Fit-Update Interval */
                Int_t fFitUpdateEventInterval {100000}; // Event Update-Interval zum Updaten des Fits

				    //
                /** Pointer auf Spektrum --> nicht streamen */
                TH1* fSpectrum{nullptr}; //!

                TString fSpectrumName; // Name des Spektrums




                void SetSpectrum(TH1* h);
                TGo4Fitter* GetFitter() const { return fFitter; }

        ClassDefOverride(TQClamFitParameter,1);
};

#endif

