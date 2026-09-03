#include "parameter/TQClamScalerValueParameter.h"

#include "TGo4Log.h"


//////////////////////////////////////////////////////////////////////

TQClamScalerValueParameter::TQClamScalerValueParameter()
	: TGo4Parameter("ScalerValueParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamScalerValueParameter::TQClamScalerValueParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamScalerValueParameter] Create %s", name);

	TdcTrigger = 0;
	SzintPmtUp = 0;
	SzintPmtDown = 0;
	CherenkovPmt = 0;
	CoincTrigger = 0;
	AccTrigger = 0;
	ClockCounter = 0;
	LivetimeCounter = 0;
	MbsLivetimeCounter = 0;
	FaradayCupChargeCounter = 0;
}

//////////////////////////////////////////////////////////////////////

Bool_t TQClamScalerValueParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamScalerValueParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamScalerValueParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamScalerValueParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	TdcTrigger = other->TdcTrigger;
	SzintPmtUp = other->SzintPmtUp;
	SzintPmtDown = other->SzintPmtDown;
	CherenkovPmt = other->CherenkovPmt;
	CoincTrigger = other->CoincTrigger;
	AccTrigger = other->AccTrigger;
	ClockCounter = other->ClockCounter;
	LivetimeCounter = other->LivetimeCounter;
	MbsLivetimeCounter = other->MbsLivetimeCounter;
	FaradayCupChargeCounter = other->FaradayCupChargeCounter;

	return kTRUE;
}
