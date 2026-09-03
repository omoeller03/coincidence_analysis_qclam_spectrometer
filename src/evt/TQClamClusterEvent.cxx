#include "evt/TQClamClusterEvent.h"

#include "geometry/TQClamWire.h"



ClassImp(TQClamClusterEvent)


void TQClamClusterEvent::Clear(Option_t* opt)
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
	clusters.clear();
	hits.clear();
}
