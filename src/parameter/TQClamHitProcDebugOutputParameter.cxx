#include "parameter/TQClamHitProcDebugOutputParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamHitProcDebugOutputParameter)

//////////////////////////////////////////////////////////////////////

TQClamHitProcDebugOutputParameter::TQClamHitProcDebugOutputParameter()
	: TGo4Parameter("HitProcDebugOutputParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamHitProcDebugOutputParameter::TQClamHitProcDebugOutputParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamHitProcDebugOutputParameter] Create %s", name);
}

//////////////////////////////////////////////////////////////////////

Bool_t TQClamHitProcDebugOutputParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamHitProcDebugOutputParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamHitProcDebugOutputParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamHitProcDebugOutputParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	WriteRaw = other->WriteRaw;
	WriteProcessed = other->WriteProcessed;
	OutputFileRaw = other->OutputFileRaw;
	OutputFileProcessed = other->OutputFileProcessed;

	return kTRUE;
}
