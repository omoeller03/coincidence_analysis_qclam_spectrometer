#include "evt/TQClamUnpackEvent.h"

ClassImp(TQClamUnpackEvent)


/*
 * Setze alle Datenfelder am Ende des Runs (Ein Event-Durchlauf) auf 0
 */
void TQClamUnpackEvent::Clear(Option_t* opt)
{
	TGo4EventElement::Clear(opt);

	eventNumber = 0;
	triggerID = 0;
	triggerTime = 0.;

	hits.clear();
	scalers.clear();


	/*
	for(auto& h : raw.hits)
	{
		h.tdcID = 0;
		h.channel = 0;
		h.rawTimeInNanoSeconds = 0.;
	}

	for(auto& s : raw.scalers)
	{
		s.scalerID = 0;
		s.timeMicroSeconds = 0;

		s.TDCTrigger = 0;
		s.scintillatorPMTUp = 0;
		s.scintillatorPMTDown = 0;
		s.cherenkovPMT = 0;
		s.coincidenceTrigger = 0;
		s.acceptedTrigger = 0;
		s.clockCounter = 0;
		s.livetimeCounter = 0;
		s.mbsLivetimeCounter = 0;
		s.faradayCupChargeCounter = 0;

		s.errorTDCTrigger = 0;
		s.errorSzintillatorPMTUp = 0;
		s.errorScintillatorPMTDown = 0;
		s.errorCherenkovPMT = 0;
		s.errorCoincidenceTrigger = 0;
		s.errorAcceptedTrigger = 0;
		s.errorClockCounter = 0;
		s.errorLivetimeCounter = 0;
		s.errorMBSLivetimeCounter = 0;
		s.errorFaradayCupChargeCounter = 0;
	}
	*/

}




double TQClamUnpackEvent::getAbsoluteTime() const
{
	return triggerTime;
}
