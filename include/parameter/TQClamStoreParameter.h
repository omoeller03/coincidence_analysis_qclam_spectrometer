#ifndef TQCLAMSTOREPARAMETER_H
#define TQCLAMSTOREPARAMETER_H

#include "TGo4Parameter.h"

#include "Rtypes.h"


class TQClamStoreParameter : public TGo4Parameter
{
	public:
		TQClamStoreParameter();
		TQClamStoreParameter(const char* name);

		Bool_t fBAutomaticFileStore = kFALSE;

		Bool_t fBPauseUserDrifttimeCalibStore = kFALSE;
		Bool_t fBPauseUserTrackStore = kFALSE;



		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamStoreParameter,1);
};


#endif
