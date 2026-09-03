#include "parameter/TQClamUnpackParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamUnpackParameter)


//////////////////////////////////////////////////////////////////////

TQClamUnpackParameter::TQClamUnpackParameter()
	: TGo4Parameter("UpackParam")
{
	TString fFolderTdcCalibration = "calibration_files/tdcCalibration";

	fTimeInterpolationElbVfb6_Random = kTRUE;
	fTimeInterpolationElbVfb6_Linear = kFALSE;
	fTimeInterpolationElbVfb6_None = kFALSE;
}



//////////////////////////////////////////////////////////////////////

TQClamUnpackParameter::TQClamUnpackParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamUnpackParameter] Create %s", name);

	TString fFolderTdcCalibration = "calibration_files/tdcCalibration";

	fTimeInterpolationElbVfb6_Random = kTRUE;
	fTimeInterpolationElbVfb6_Linear = kFALSE;
	fTimeInterpolationElbVfb6_None = kFALSE;
}


Bool_t TQClamUnpackParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamUnpackParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamUnpackParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamUnpackParameter] UpdateFrom nicht erfolgreich!");
		return kFALSE;
	}

	fFolderTdcCalibration = other->fFolderTdcCalibration;

	fTimeInterpolationElbVfb6_Random = other->fTimeInterpolationElbVfb6_Random;
	fTimeInterpolationElbVfb6_Linear = other->fTimeInterpolationElbVfb6_Linear;
	fTimeInterpolationElbVfb6_None = other->fTimeInterpolationElbVfb6_None;

	return kTRUE;
}
