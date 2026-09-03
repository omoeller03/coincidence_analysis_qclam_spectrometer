#ifndef TQCLAMDAGOBERTRAWEVENT_H
#define TQCLAMDAGOBERTRAWEVENT_H

#include "TGo4EventElement.h"

#include "structs/TQClamMbsHeader.h"
#include "structs/TQClamScaler.h"
#include <cstdint>
#include <vector>


struct RawHit
{
	/* TODO? noch keine Conditions ... */
	bool accepted = true; /** erfuellt Hit Conditions? */

        int tdcID; //-------------------------------- von welchen TDC (0 ... 18 in alter/aktueller DAQ)
        int channel; //------------------------------ TDC-Kanal
        //double driftTime; //------------------------- (rohe) Driftzeit aus TDC
        double rawTimeInNanoSeconds; //------------------------- (rohe) Driftzeit aus TDC
        /// nicht in verwendung
        /*
        double tot; //------------------------------- time over treshold --> wird bisher nicht weitergegeben an Hit
                    //------------------------------- vmtl auch nicht noetig. Aber dann gehts in monitoring
        double leadingEdge;
        double trailingEdge;
        double tdcTrigger;
        */
};


// auslese zeitpunkt


/*
struct RawScaler
{
	// TODO? noch keine Conditions ...
	bool accepted = true; // erfuellt Scaler Conditions?

        int scalerID = -1; //------------------------ Index ScalerTree (vmtl nicht notwendig
        double timeMicroSeconds = 0.0; //----------------- Zeitpunkt Scalerauslese

        // Hauptzaehler
        uint64_t TDCTrigger;
        uint64_t scintillatorPMTUp;
        uint64_t scintillatorPMTDown;
        uint64_t cherenkovPMT;
        uint64_t coincidenceTrigger;
        uint64_t acceptedTrigger;
        uint64_t clockCounter;
        uint64_t livetimeCounter;
        uint64_t mbsLivetimeCounter;
        uint64_t faradayCupChargeCounter;

        // Fehlerzaehler
        uint64_t errorTDCTrigger;
        uint64_t errorScintillatorPMTUp;
        uint64_t errorScintillatorPMTDown;
        uint64_t errorCherenkovPMT;
        uint64_t errorCoincidenceTrigger;
        uint64_t errorAcceptedTrigger;
        uint64_t errorClockCounter;
        uint64_t errorLivetimeCounter;
        uint64_t errorMBSLivetimeCounter;
        uint64_t errorFaradayCupChargeCounter;
};
*/


struct RawGammaHit
{
	size_t module; /** SIS3316 Modulnummer: 0 oder 1 */
	size_t channel; /** Kanalnummer 0..15 */
	

        double timestamp; /** korrigierter timestamp (MAW) */
        uint32_t startEnergyFIR;
        uint32_t maxEnergyFIR;
        uint32_t firstGateSum;
        uint32_t secondGateSum;

        uint32_t maxMAW;
        uint32_t beforeMAW;
        uint32_t afterMAW;

        uint32_t flags;
        bool pileup;
        bool mawFlag;

        std::vector<uint16_t> samples1;
        std::vector<uint16_t> samples2;

        bool hasScaler;
        std::vector<uint32_t> scaler_raw;
        std::vector<uint32_t> scaler_accepted;
        std::vector<uint32_t> scaler_deadtime;
        std::vector<uint32_t> scaler_pileup;
        std::vector<uint32_t> scaler_veto;
        std::vector<uint32_t> scaler_suppressed;

        /** optional... moeglich???? */
//        std::vector<uint16_t> trace;
};


struct RawDagobertScaler
{
	size_t module;

	double timestamp;

	std::vector<uint32_t> raw;
	std::vector<uint32_t> accepted;
	std::vector<uint32_t> deadtime;
	std::vector<uint32_t> veto;
	std::vector<uint32_t> suppressed;
};







class TQClamDagobertRawEvent : public TGo4EventElement
{
	public:
		TQClamDagobertRawEvent() {}
		TQClamDagobertRawEvent(const char* name = "RawEvent") : TGo4EventElement(name)
		{}

		virtual ~TQClamDagobertRawEvent()
		{}

		void Clear(Option_t* opt = "") override;

		// Datenfelder QCLAM
	        // --- Event Metadaten ---
		int fEvtCounter{0};
		int eventNumber; //-------------------------- EventNummer
        	int triggerID; //---------------------------- TriggerNummer (TriggerRequestCounter)
        	double triggerTime; //----------------------- zeitstempel (in s) des events (Szintillator)

		// --- Mbs-Header ---
		MbsHeader header;

        	// --- TDC-Hits ---
        	std::vector<RawHit> hits; //----------------- alle TDC-hits

        	// --- Scaler-Daten (optional, nicht notwendig fuer Rekonstruktion) ---
        	//std::vector<RawScaler> scalers; // vmtl nur ein RawScaler, doch sicherheitshalber vector
        	//RawScaler scaler;
        	TQClamScaler scaler;


		// Datenfelder DAGOBERT
		std::vector<RawGammaHit> gammaHits;
		std::vector<RawDagobertScaler> dagobertScalers;

	private:
		/* TODO? noch keine Conditions ... */
		bool accepted = true;

	

	ClassDefOverride(TQClamDagobertRawEvent,1)
};


#endif
