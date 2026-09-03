#include "parameter/TQClamClusterConditionParameter.h"

#include "TGo4Log.h"


ClassImp(TQClamClusterConditionParameter)


//////////////////////////////////////////////////////////////////////

TQClamClusterConditionParameter::TQClamClusterConditionParameter()
	: TGo4Parameter("ClusterConditionParam")
{
	fCleanCluster = kFALSE;
	WireSpanCondMode = kFlagOnly;
	HitNumberCondMode = kFlagOnly;

	for(size_t l=0; l<MAX_LAYER; l++)
	{
		minWireSpanInCluster[l] = -1;
		maxWireSpanInCluster[l] = -1;
		minHitNumberInCluster[l] = -1;
		maxHitNumberInCluster[l] = -1;
		requireVShape[l] = kFALSE;
		requireAscending[l] = kFALSE;
		requireDescending[l] = kFALSE;

		fMaxAllowedGapInCleaning[l] = -1;
		fMaxDrifttimeJump[l] = -1.;
		fMaxRemovedHits[l] = -1;
		fMaxCleaningIterations[l] = 0;
	}
}

//////////////////////////////////////////////////////////////////////


TQClamClusterConditionParameter::TQClamClusterConditionParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("ClusterConditionPar] Create %s.", name);

	fCleanCluster = kFALSE;
	WireSpanCondMode = kFlagOnly;
	HitNumberCondMode = kFlagOnly;

	for(size_t l=0; l<MAX_LAYER; l++)
	{
		minWireSpanInCluster[l] = -1;
		maxWireSpanInCluster[l] = -1;
		minHitNumberInCluster[l] = -1;
		maxHitNumberInCluster[l] = -1;
		requireVShape[l] = kFALSE;
		requireAscending[l] = kFALSE;
		requireDescending[l] = kFALSE;
		requireClusterWithoutGaps[l] = kFALSE;

		fMaxAllowedGapInCleaning[l] = -1;
		fMaxDrifttimeJump[l] = -1.;
		fMaxRemovedHits[l] = -1;
		fMaxCleaningIterations[l] = 0;
	}
}


Bool_t TQClamClusterConditionParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamClusterConditionParameter] UpdateFrom()");
	auto* other = static_cast<TQClamClusterConditionParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamClusterConditionParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	fCleanCluster = other->fCleanCluster;
	WireSpanCondMode = other->WireSpanCondMode;
	HitNumberCondMode = other->HitNumberCondMode;

	for(size_t l=0; l<MAX_LAYER; l++)
	{
		minWireSpanInCluster[l] = other->minWireSpanInCluster[l];
		maxWireSpanInCluster[l] = other->maxWireSpanInCluster[l];
		minHitNumberInCluster[l] = other->minHitNumberInCluster[l];
		maxHitNumberInCluster[l] = other->maxHitNumberInCluster[l];
		requireVShape[l] = other->requireVShape[l];
		requireAscending[l] = other->requireAscending[l];
		requireDescending[l] = other->requireDescending[l];
		requireClusterWithoutGaps[l] = other->requireClusterWithoutGaps[l];

                fMaxAllowedGapInCleaning[l] = other->fMaxAllowedGapInCleaning[l];
                fMaxDrifttimeJump[l] = other->fMaxDrifttimeJump[l];
                fMaxRemovedHits[l] = other->fMaxRemovedHits[l];
                fMaxCleaningIterations[l] = other->fMaxCleaningIterations[l];

	}


	return kTRUE;
}
