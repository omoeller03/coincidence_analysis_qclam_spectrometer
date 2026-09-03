#include "evt/TQClamDagobertRawEvent.h"

ClassImp(TQClamDagobertRawEvent)



/** Setze alle Datenfelder am Ende des Runs (Ein Event-Durchlauf) auf 0 */
void TQClamDagobertRawEvent::Clear(Option_t* opt)
{	
	TGo4EventElement::Clear(opt);

	header.file_name = "";
	header.run_number = -1;
	header.file_number = -1;

	header.user = "";
	header.time = "";
	header.label = "";

	header.mbs_event_count = 0;
	header.mbs_trigger = 0;
	
	header.mbs_buffer_number = 0;

	header.mbs_time_sec = 0;
	header.mbs_time_subsec = 0;


	fEvtCounter = 0;
	eventNumber = 0;
	triggerID = 0;
	triggerTime = 0.;

	/*
	 * finde raus, wie viel reserven
	 */
	/*
	static size_t maxHits = 0;
	if(scalers.size() > maxHits)
	{
		maxHits = scalers.size();
		printf("Max scalers: %ld\n",maxHits);
	}
	*/
	hits.clear();
	//scalers.clear();
	gammaHits.clear();
	dagobertScalers.clear();

	hits.reserve(1000);
	gammaHits.reserve(32);


	// reserve?
}
