#include "parameter/TQClamDisplaySettingParameter.h"

#include "TGo4Log.h"


ClassImp(TQClamDisplaySettingParameter)

//////////////////////////////////////////////////////////////////////

TQClamDisplaySettingParameter::TQClamDisplaySettingParameter()
	: TGo4Parameter("DisplaySettingParam")
{}


//////////////////////////////////////////////////////////////////////

TQClamDisplaySettingParameter::TQClamDisplaySettingParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("Create %s", name);

	RollGraphTriggerIntegrationWindow = 1000; /** default */
}


Bool_t TQClamDisplaySettingParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamDisplaySettingParameter] UpdateFrom().");
	auto* other = dynamic_cast<TQClamDisplaySettingParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamDisplaySettingParameter] UpdateFrom nicht erfolgreich!");
		return kFALSE;
	}

	RollGraphTriggerIntegrationWindow = other->RollGraphTriggerIntegrationWindow;

	return kTRUE;
}
