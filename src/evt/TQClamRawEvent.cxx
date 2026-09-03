#include "evt/TQClamRawEvent.h"

ClassImp(TQClamRawEvent)



/** Setze alle Datenfelder am Ende des Runs (Ein Event-Durchlauf) auf 0 */
void TQClamRawEvent::Clear(Option_t* opt)
{	
	TGo4EventElement::Clear(opt);

	fEvtCounter = 0;
	eventNumber = 0;
	triggerID = 0;
	triggerTime = 0.;

	hits.clear();
	scalers.clear();
}
