#ifndef TQCLAMUNPACKEVENT_H
#define TQCLAMUNPACKEVENT_H

#include "TGo4EventElement.h"

#include <cstdint>
#include <vector>


struct Hit
{
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


struct Scaler
{
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



/*
struct RawEvent
{
        // --- Event Metadaten ---
        int eventNumber; //-------------------------- EventNummer
        int triggerID; //---------------------------- TriggerNummer (TriggerRequestCounter)
        double triggerTime; //----------------------- zeitstempel (in s) des events (Szintillator)

        // --- TDC-Hits ---
        std::vector<RawHit> hits; //----------------- alle TDC-hits

        // --- Scaler-Daten (optional, nicht notwendig fuer Rekonstruktion) ---
        std::vector<RawScaler> scalers; // vmtl nur ein RawScaler, doch sicherheitshalber vector

        // --- fuer HitFactory ---
        // in HitFactory mache ich raw.absoluteTime().
        double absoluteTime() const { return triggerTime; }
};
*/




class TQClamUnpackEvent : public TGo4EventElement
{
	public:
		TQClamUnpackEvent() {}
		TQClamUnpackEvent(const char* name = "RawEvent") : TGo4EventElement(name)
		{}

		virtual ~TQClamUnpackEvent()
		{}

		void Clear(Option_t* opt = "") override;

		// Datenfelder
	        // --- Event Metadaten ---
		int eventNumber; //-------------------------- EventNummer
        	int triggerID; //---------------------------- TriggerNummer (TriggerRequestCounter)
        	double triggerTime; //----------------------- zeitstempel (in s) des events (Szintillator)

        	// --- TDC-Hits ---
        	std::vector<Hit> hits; //----------------- alle TDC-hits

        	// --- Scaler-Daten (optional, nicht notwendig fuer Rekonstruktion) ---
        	std::vector<Scaler> scalers; // vmtl nur ein RawScaler, doch sicherheitshalber vector

        	// --- fuer HitProc ---
        	// in HitProc mache ich raw.absoluteTime().
//        	double absoluteTime() const { return triggerTime; }


	private:
		double getAbsoluteTime() const;
	

	ClassDefOverride(TQClamUnpackEvent,1)
};


#endif
