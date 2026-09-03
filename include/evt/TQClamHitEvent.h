#ifndef TQCLAMHITEVENT_H
#define TQCLAMHITEVENT_H

#include "TGo4EventElement.h"
#include "structs/TQClamMbsHeader.h"
#include "structs/TQClamScaler.h"
#include "structs/TQClamTriggerDetectors.h"

//class TQClamWire;





struct TQClamHit
{
	bool accepted = false; /** Conditions geprueft */

	int hitID;
	/** TDC & TDC-Kanal */
	int tdcID;
	int chNum;
	/** Drahtebene & Draht */
	int layerID;
	int wireNum;

	double tdcTrigTime;
	/** Hiteigenschaften */
	double absoluteTime; /** absolute Hitzeit */
	double leading;
	double trailing;
	double drifttime;
	double driftlength;
	double tot; /** time-over-threshold */
	/** pointer auf Draht */
//	const TQClamWire* fWire; //!  // --> ROOT soll nicht streamen
};

struct TQClamTrigger
{
	bool fired = false;
	int tdcID;
	int chNum;
	double time = -1e9;
	std::vector<double> tot;
	int multiplicity = 0;
	bool accepted = false;
	/** SzintPmtUp hat gefeuert zu zeit ... und mit tot ...*/
};
/*
struct TQClamScaler
{
	bool accepted = false;

	double timeMicroSeconds = 0.0;

	// Zaehlerstaende
	uint64_t tdcTrigger = 0;
	uint64_t szintillatorPmtUp = 0;
	uint64_t szintillatorPmtDown = 0;
	uint64_t cherenkovPmt = 0;
	uint64_t coincidenceTrigger = 0;
	uint64_t acceptedTrigger = 0;
	uint64_t clockCounter = 0;
	uint64_t livetimeCounter = 0;
	uint64_t mbsLivetimeCounter = 0;
	uint64_t faradayCupChargeCounter = 0;

	// Fehler-Zaehlerstaende
	uint64_t errorTdcTrigger = 0;
	uint64_t errorSzintillatorPmtUp = 0;
	uint64_t errorSzintillatorPmtDown = 0;
	uint64_t errorCherenkovPmt = 0;
	uint64_t errorCoincidenceTrigger = 0;
	uint64_t errorAcceptedTrigger = 0;
	uint64_t errorClockCounter = 0;
	uint64_t errorLivetimeCounter = 0;
	uint64_t errorMbsLivetimeCounter = 0;
	uint64_t errorFaradayCupChargeCounter = 0;


};
*/

class TQClamHitEvent : public TGo4EventElement
{
        public:
		TQClamHitEvent() {}
                TQClamHitEvent(const char* name) : TGo4EventElement(name) {}
                virtual ~TQClamHitEvent() {}

		/** setze HitEvent-Parameter auf 0 */
                void Clear(Option_t* opt = "") override;




		/** Daten public */
		MbsHeader header; //! nicht streamen

		bool accepted = false;
		Int_t fEventNumber = 0;
		Int_t fEvtCounter = 0;

		std::vector<TQClamHit> hits; /** driftkammerhits */
		TQClamScaler scaler; //! nicht streamen /** scaler pro event - meist nur 1 x */
		TQClamTriggerDetectors trigDet; //!

		TQClamTrigger trigSzUp; //!
		TQClamTrigger trigSzDown; //!
		TQClamTrigger trigCher; //!
		TQClamTrigger trigCoinc; //!
		TQClamTrigger trigAcc; //!
		TQClamTrigger trigClock; //!
		TQClamTrigger trigLive; //!
		TQClamTrigger trigMbsLive; //!
		TQClamTrigger trigFaraday; //!


	ClassDefOverride(TQClamHitEvent, 1);
};



#endif
