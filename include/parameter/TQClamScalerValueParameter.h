#ifndef TQCLAMSCALERVALUEPARAMETER_H
#define TQCLAMSCALERVALUEPARAMETER_H

#include "TGo4Parameter.h"

class TQClamScalerValueParameter : public TGo4Parameter
{
	public:
		TQClamScalerValueParameter();
		TQClamScalerValueParameter(const char* name);

		ULong64_t TdcTrigger; // Anzahl 'Tdc-Trigger'
		ULong64_t SzintPmtUp; // Anzahl Trigger 'SzintillatorPmtUp'
		ULong64_t SzintPmtDown; // Anzahl Trigger 'SzintillatorPmtDown'
		ULong64_t CherenkovPmt; // Anzahl Trigger 'CherenkovPmt'
		ULong64_t CoincTrigger; // Anzahl 'Coincidence-Trigger'
		ULong64_t AccTrigger; // Anzahl 'Accepted Trigger'
		ULong64_t ClockCounter; // Anzahl 'Clock-Zyklen'
		ULong64_t LivetimeCounter; // Livetime/ns
		ULong64_t MbsLivetimeCounter; // Mbs-Livetime/ns
		ULong64_t FaradayCupChargeCounter; // 'Faraday-Cup-Charge-Counter'


		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamScalerValueParameter,1);

};

#endif
