#include "parameter/TQClamFaradayCupCurrentCalibrationParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamFaradayCupCurrentCalibrationParameter)

//////////////////////////////////////////////////////////////////////

TQClamFaradayCupCurrentCalibrationParameter::TQClamFaradayCupCurrentCalibrationParameter()
	: TGo4Parameter("FaradayCupCurrentCalibParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamFaradayCupCurrentCalibrationParameter::TQClamFaradayCupCurrentCalibrationParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamFaradayCupCurrentCalibrationParameter] Create %s", name);

	offsetCurrent = 0.0;
	scalefactorCurrent = 1.0;
	invert = true;
}


Bool_t TQClamFaradayCupCurrentCalibrationParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamFaradayCupCurrentCalibrationParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamFaradayCupCurrentCalibrationParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamFaradayCupCurrentCalibrationParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	
	offsetCurrent = other->offsetCurrent;
	scalefactorCurrent = other->scalefactorCurrent;
	invert = other->invert;

	return kTRUE;
}
