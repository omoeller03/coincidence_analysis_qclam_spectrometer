#include "parameter/TQClamHitConditionParameter.h"

#include "TGo4Log.h"


ClassImp(TQClamHitConditionParameter)

//////////////////////////////////////////////////////////////////////

TQClamHitConditionParameter::TQClamHitConditionParameter()
	: TGo4Parameter("HitConditionParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamHitConditionParameter::TQClamHitConditionParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamHitConditionParameter] Create %s", name);

	for(size_t i=0; i<MAX_LAYER; i++)
	{
		minDrifttime_ns[i] = -1e9;
		maxDrifttime_ns[i] = +1e9;

		minToT_ns[i] = -1e9;
		maxToT_ns[i] = +1e9;
	}
}


Bool_t TQClamHitConditionParameter::UpdateFrom(TGo4Parameter* rhs)
{
	auto* other = dynamic_cast<TQClamHitConditionParameter*>(rhs);
	if(!other) { return kFALSE; }

	for(size_t i=0; i<MAX_LAYER; i++)
	{
		minDrifttime_ns[i] = other->minDrifttime_ns[i];
		maxDrifttime_ns[i] = other->maxDrifttime_ns[i];

		minToT_ns[i] = other->minToT_ns[i];
		maxToT_ns[i] = other->maxToT_ns[i];
	}

	ToTCondMode = other->ToTCondMode;

	DrifttimeCondMode = other->DrifttimeCondMode;

	requireTdcTrig = other->requireTdcTrig;
	requireLead = other->requireLead;
	requireTrail = other->requireTrail;

	return kTRUE;
}
