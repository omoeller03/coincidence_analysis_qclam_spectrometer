#ifndef TQCLAMDRIFTCHAMCALIBRATIONPARAMETER_H
#define TQCLAMDRIFTCHAMCALIBRATIONPARAMETER_H

#include "TGo4Parameter.h"
#include "TQClamConstants.h"

#include "Rtypes.h"


class TQClamDriftchamCalibrationParameter : public TGo4Parameter
{
        public:
                TQClamDriftchamCalibrationParameter();
                TQClamDriftchamCalibrationParameter(const char* name);

                /** Max Dimensionen */
                static const Int_t MAX_TDC = QCLAM_MAX_TDC; // Maximale TDC-Anzahl: 32
                static const Int_t MAX_CH = QCLAM_MAX_CH; // Maximale Anzahl Channel/TDC: 128
                static const Int_t MAX_LAYER = QCLAM_MAX_LAYER; // Maximale Anzahl Ebenen: 8
                static const Int_t MAX_WIRE = QCLAM_MAX_WIRE; // Maximale Anzahl Draehte/Ebene: 512

		/** -- MAPPING -- */

                /** tatsaechliche Dimensionen */
                Int_t numTDC = -1; // Anzahl TDCs
                Int_t numCh[MAX_TDC]; // Anzahl Channel/TDC
		Int_t numLayer = -1; // Anzahl Ebenen
		Int_t numWire[MAX_LAYER]; // Anzahl Draehte/Ebene


                /** mapping: (tdc,ch) --> layer, wire */
                Int_t layerMapLead[MAX_TDC][MAX_CH]; // Mapping Leading: (tdc,ch) --> layer (-1 = kein Mapping)
                Int_t wireMapLead[MAX_TDC][MAX_CH]; // Mapping Leading: (tdc,ch) --> wirenumber (-1 = kein Mapping)
                Int_t layerMapTrail[MAX_TDC][MAX_CH]; // Mapping Trailing: (tdc,ch) --> layer (-1 = kein Mapping)
                Int_t wireMapTrail[MAX_TDC][MAX_CH]; // Mapping Trailing: (tdc,ch) --> wirenumber (-1 = kein Mapping)

                Bool_t leadChannelDisabled[MAX_TDC][MAX_CH]; // Leading (tdc,ch) --> aktiviert: kFALSE / deaktiviert: kTRUE
                Bool_t trailChannelDisabled[MAX_TDC][MAX_CH]; // Trailing (tdc,ch) --> aktiviert: kFALSE / deaktiviert: kTRUE
                Bool_t wireDisabled[MAX_LAYER][MAX_WIRE]; // (layer,wire) --> aktiviert: kFALSE / deaktiviert: kTRUE



                /** -- Driftzeitoffsets in ns -- */

                /** globaler offset */
		Bool_t useGlobOffset = kFALSE; // aktiviere/deaktiviere Offset: Global
                Double_t globOffset = 0.0; // Driftzeit-Offset/ns: Global

                /** offset fuer tdc */
		Bool_t useTdcOffset = kFALSE; // aktiviere/deaktiviere Offset: TDC
                Double_t tdcOffset[MAX_TDC]; // Driftzeit-Offset/ns: TDC

                /** offsets fuer tdc-kanaele */
		Bool_t useTdcChannelOffset = kFALSE; // aktiviere/deaktiviere Offset: TDC-Kanal
                Double_t tdcChannelOffset[MAX_TDC][MAX_CH]; // Driftzeit-Offset/ns: TDC-Kanal

                /** offset fuer drahtebene */
		Bool_t useLayerOffset = kFALSE; // aktiviere/deaktiviere Offset: Drahtebene
                Double_t layerOffset[MAX_LAYER]; // Driftzeit-Offset/ns: Drahtebene

                /** offset fuer draht */
		Bool_t useWireOffset = kFALSE; // aktiviere/deaktiviere OFfset: Draht
		Bool_t useWireOffsetFiles = kTRUE; // keine Default Time-Offsets
		TString offsetFiles[MAX_LAYER]; // Pfade zu den Input-Files fuer die Draht-spezifischen Driftzeit-Offsets
                Double_t wireOffset[MAX_LAYER][MAX_WIRE]; // Driftzeit-Offset/ns: Draht

                /** drifzzeit am ende (nach offsets) invertieren --> *(-1) */
                Bool_t invert = kFALSE; // invertiere berechnete Driftzeit nach Anwendung der Offsets (--> *(-1))



		/** -- Time-Walk Korrekturen der Driftzeit -- */

		/** verwende time-walk korrigerte Driftzeiten? */
		Bool_t useTimeWalkCorrectedDrifttime = kFALSE; // nutze time-walk korrigierte Zeiten?
		Bool_t useGlobalTimeWalk = kTRUE; // nutze globale Korrektur-Parameter
		Bool_t useWireSpecificTimeWalk = kFALSE; // nutze Draht-spezifische Korrektur-Parameter aus Input-Files
		TString timewalkFiles[MAX_LAYER]; // Pfade zu den Input-Files fuer die Draht-spezifischen Time-Walk Korrekturen

		/** Referenzzeit, auf die das Maximum geschoben wird */
		Double_t tref = 0.0; // Referenzzeit/ns, hierauf wird das Driftzeitspektrum-Maximum geschoben

		/** Parameter der Fit-Funktion (an Drifttime-Peak vs. ToT) zurm Glätten */
		Double_t p0 = 0.0; // GLOBAL: Parameter 1 - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten: f(tot)=p0*exp(-p1*tot)
		Double_t p1 = 0.0; // GLOBAL: Parameter 2 - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten: f(tot)=p0*exp(-p1*tot)
		Double_t p2 = 0.0; // GLOBAL: Quadratischer-Parameter - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten
//		Double_t p3 = 0.0; // GLOBAL: Kubischer-Parameter - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten

		/** Parameter Draht-Spezifisch */
		Double_t p0_spec[MAX_LAYER][MAX_WIRE]; // Draht-spezifisch: Parameter 1 - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten: f(tot)=p0*exp(-p1*tot)
		Double_t p1_spec[MAX_LAYER][MAX_WIRE]; // Draht-spezifisch: Parameter 2 - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten: f(tot)=p0*exp(-p1*tot)
		Double_t p2_spec[MAX_LAYER][MAX_WIRE]; // GLOBAL: Quadratischer-Parameter - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten
//		Double_t p3_spec[MAX_LAYER][MAX_WIRE]; // GLOBAL: Kubischer-Parameter - Fit-Funktion (Driftzeitspektrum-Peak vs. ToT) zum Glaetten

		/** -- MAPPING Methoden -- */

		/** Draht aktiviert? */
                Bool_t IsWireActive(Int_t layer, Int_t wire) const;

                /** Kanal aktiviert? */
                Bool_t IsChannelActive(Int_t tdc, Int_t ch) const;

                /** Existiert Mapping fuer (tdc,ch)? */
                Bool_t HasMapping(Int_t tdc, Int_t ch) const;

                /** Fuehre Mapping durch (tdc,ch) --> (layer,wire) */
                std::pair<Int_t,Int_t> GetWire(Int_t tdc, Int_t ch) const;



		/** -- Driftzeit-Umrechnung Methode -- */

                double ApplyTimeOffsets(double rawT, int tdc, int ch, int layer, int wire) const;
		double ApplyTimeWalkCorrection(int layer, int wireNumber, double rawT, double tot) const;


		/** -- UPDATE -- */
                Bool_t UpdateFrom(TGo4Parameter* rhs) override;


        ClassDefOverride(TQClamDriftchamCalibrationParameter,1);
};

#endif
