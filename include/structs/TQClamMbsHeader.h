#ifndef TQCLAMMBSHEADER_H
#define TQCLAMMBSHEADER_H

#include <string>

struct MbsHeader
{
	std::string file_name;

	int run_number = -1;
	int file_number = -1;

//	std::string run_id = -1;

	std::string user;
	std::string time;
	std::string label;

	uint32_t mbs_event_count = 0; // Mbs Evt-Counter innerhalb Run (mbs->GetCount())
	uint16_t mbs_trigger = 0; // Trigger-Typ

	uint32_t mbs_buffer_number = 0; // Mbs Buffer-Counter (Buffer hat viele Evts) (mbs->GetTrigger)
	uint32_t mbs_evts_in_buffer = 0;

	uint32_t mbs_time_sec = 0; // Buffer-Zeitstempel (buf->l_time[0])
	uint32_t mbs_time_subsec = 0; // Buffer-Zeitstempel (buf->l_time[1])
};

#endif
