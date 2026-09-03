#include "parameter/TQClamStoreParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamStoreParameter)


//////////////////////////////////////////////////////////////////////

TQClamStoreParameter::TQClamStoreParameter()
	: TGo4Parameter("StoreParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamStoreParameter::TQClamStoreParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamStoreParameter] Creste %s",name);
}

//////////////////////////////////////////////////////////////////////

Bool_t TQClamStoreParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamStoreParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamStoreParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamStoreParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}


	fBAutomaticFileStore = other->fBAutomaticFileStore;
	fBPauseUserDrifttimeCalibStore = other->fBPauseUserDrifttimeCalibStore;
	fBPauseUserTrackStore = other->fBPauseUserTrackStore;

	return kTRUE;
}

