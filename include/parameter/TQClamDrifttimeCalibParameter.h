#ifndef TQCLAMDRIFTTIMECALIBPARAMETER_H
#define TQCLAMDRIFTTIMECALIBPARAMETER_H

#include "TGo4Parameter.h"
#include "TQClamConstants.h"
#include "Rtypes.h"

#define __DATANAME_DRIFTDIST__ "data"

class TGo4Fitter;

class TQClamDrifttimeCalibParameter : public TGo4Parameter
{
	public:
		TQClamDrifttimeCalibParameter();
		TQClamDrifttimeCalibParameter(const char* name);
		virtual ~TQClamDrifttimeCalibParameter();

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

		static const Int_t MAX_LAYER = QCLAM_MAX_LAYER; // Maximale Anzahl Ebenen (gesetzt in TQCLamConstants.h)
		static const Int_t MAX_WIRE = QCLAM_MAX_WIRE; // Maximale Anzahl Draehte (gesetzt in TQCLamConstants.h)

		Int_t fUpdateEventInterval{100000}; // Update-Interval fuer Fit --> Ganz zum Schluss dann noch einmal fitten und Daten wegscheiben
		Int_t fNumWires[MAX_LAYER]; // Anzahl Kanaele je Ebene 

		Bool_t fDoFitAny{kFALSE}; // Kalibrierung/Fits durchfuehren?
		Bool_t fDoPlotFit{kFALSE}; // plotte fit von an jedes Driftzeit-Spektrum je Draht

		/** Fit-Steuerung */
		Bool_t fDoFit[MAX_LAYER]; // Fit fuer Ebene ausfuehren?

		/** fit auswaehlen */
		Bool_t fUseFermiDirac{kFALSE}; // Fit an ansteigende Flanke
		Bool_t fUseSkewed{kFALSE}; // Fit an gesamtes Driftzeitspektrum

		/** -- FermiDirac --> beschreibt nur ansteigende Flanke -- */
		/** Startwerte */
		Double_t fStartAmp{100.}; // Startwert Amplitude (Normierung)
		Double_t fStartT0{50.0}; // Startwert fuer t0 (Start-Position Flanke)
		Double_t fStartTau{5.0}; // Startwert fuer tau (Abstand t0 zu Mitte Flanke)
		Double_t fStartT{2.0}; // Startwert 'Fermi-Temperatur'

		/** Ranges */
		Double_t fMinAmp{0.}; // Minimum Amplitude
		Double_t fMaxAmp{1e9}; // Maximum Amplitude
		Double_t fMinT0{-200.}; // Minimum t0
		Double_t fMaxT0{1000.}; // Maximum t0
		Double_t fMinTau{-50.}; // Minimum tau
		Double_t fMaxTau{100.}; // Maximum tau
		Double_t fMinT{0.1}; // Minimum Fermi-Temp T
		Double_t fMaxT{50.}; // Maximum Fermi-Temp T

		/** Fitwerte */
		Double_t fT0[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis t0 je Draht in Ebene
		Double_t fTau[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis tau je Draht in Ebene
		Double_t fT[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis Fermi-Temp T je Draht in Ebene
		Double_t fA[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis Amplitude (Normierung) je Draht in Ebene

		/** -- Skewed --> berschreibt gesamtes Driftzeitspektrum -- */
		/** Startwerte */
		Double_t fStartA0{100.}; // Startwert, Driftzeit-Verteilung Fit: Amplitude0
		Double_t fStartA1{100.}; // Startwert, Driftzeit-Verteilung Fit: Amplitude1
		Double_t fStartMu0{10.}; // Startwert, Driftzeit-Verteilung Fit: Mittelwert0
		Double_t fStartMu1{10.}; // Startwert, Driftzeit-Verteilung Fit: Mittelwert1
		Double_t fStartSigma0{10.}; // Startwert, Driftzeit-Verteilung Fit: Stdw0
		Double_t fStartSigma1{10.}; // Startwert, Driftzeit-Verteilung Fit: Stdw1
		Double_t fStartS0{0.5}; // Startwert, Driftzeit-Verteilung Fit: S0
		Double_t fStartS1{0.5}; // Startwert, Driftzeit-Verteilung Fit: S1

		/** Ranges */
		Double_t fMinA0{0.}; // Minimum, Driftzeit-Verteilung Fit: Amplitude0
		Double_t fMaxA0{1e9}; // Maximum, Driftzeit-Verteilung Fit: Amplitude0
		Double_t fMinA1{0.}; // Minimum, Driftzeit-Verteilung Fit: Amplitude1
		Double_t fMaxA1{1e9}; // Maximum, Driftzeit-Verteilung Fit: Amplitude1
		Double_t fMinMu0{-50.}; // Minimum, Driftzeit-Verteilung Fit: Mittelwert0
		Double_t fMaxMu0{200.}; // Maximum, Driftzeit-Verteilung Fit: Mittelwert0
		Double_t fMinMu1{-50.}; // Minimum, Driftzeit-Verteilung Fit: Mittelwert1
		Double_t fMaxMu1{200.}; // Maximum, Driftzeit-Verteilung Fit: Mittelwert1
		Double_t fMinSigma0{0.1}; // Minimum, Driftzeit-Verteilung Fit: Stdw0
		Double_t fMaxSigma0{100.}; // Maximum, Driftzeit-Verteilung Fit: Stdw0
		Double_t fMinSigma1{0.1}; // Minimum, Driftzeit-Verteilung Fit: Stdw1
		Double_t fMaxSigma1{100.}; // Maximum, Driftzeit-Verteilung Fit: Stdw1
		Double_t fMinS0{0.01}; // Minimum, Driftzeit-Verteilung Fit: S0
		Double_t fMaxS0{10.}; // Maximum, Driftzeit-Verteilung Fit: S0
		Double_t fMinS1{0.01}; // Minimum, Driftzeit-Verteilung Fit: S1
		Double_t fMaxS1{10.}; // Maximum, Driftzeit-Verteilung Fit: S1

		/** Fitwerte */
		Double_t fA0[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: Amplitude0
		Double_t fA1[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: Amplitude1
		Double_t fMu0[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: Mittelwert0
		Double_t fMu1[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: Mittelwert1
		Double_t fSigma0[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: Stdw0
		Double_t fSigma1[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: Stdw1
		Double_t fS0[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: S0
		Double_t fS1[MAX_LAYER][MAX_WIRE]; // Fit-Ergebnis, Driftzeit-Verteilung Fit: S1



		/** Fitter --> enthaehlt Fermi-Modell */
		TGo4Fitter* fFitter{nullptr}; // Fitter (enthaehlt Fermi-Dirac FitModell)


	ClassDefOverride(TQClamDrifttimeCalibParameter,1);
};

#endif
