#include "evt/TQClamHitEvent.h"


ClassImp(TQClamHitEvent)


/** Setze alle Datenfelder am Ende des Runs (Ein Event-Durchlauf) auf 0 */
void TQClamHitEvent::Clear(Option_t* opt)
{
	accepted = false;


	/*
	static size_t maxHits = 0;
	if(hits.size() > maxHits)
	{
		maxHits = hits.size();
		printf("Max hits: %ld\n",maxHits);
	}
	*/

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
	fEventNumber = 0;

	hits.clear();
	//scaler.clear();

	hits.reserve(1000);

	trigSzUp = {};
	trigSzDown = {};
	trigCher = {};
	trigCoinc = {};
	trigAcc = {};
	trigClock = {};
	trigLive = {};
	trigMbsLive = {};
	trigFaraday = {};
}
