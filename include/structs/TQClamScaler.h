// include/structs/TQClamScaler.h

#ifndef TQCLAMSCALER_H
#define TQCLAMSCALER_H

#include <cstdint>

struct TQClamScaler
{
	bool accepted = false;

	int scalerID = -1;

	double timeMicroSeconds = 0.0;

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

#endif
