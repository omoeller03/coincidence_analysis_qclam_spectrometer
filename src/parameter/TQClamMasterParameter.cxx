#include "parameter/TQClamMasterParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamMasterParameter)

//////////////////////////////////////////////////////////////////////

TQClamMasterParameter::TQClamMasterParameter()
	: TGo4Parameter("MasterParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamMasterParameter::TQClamMasterParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamMasterParameter] Create %s", name);
	std::string path = std::string(PROJECT_SOURCE_DIR) + "/" + fMasterConfigFile.Data();
	TGo4Log::Info("[TQClamMasterParameter] Path To MasterConfig: %s", path.c_str());
}

//////////////////////////////////////////////////////////////////////

Bool_t TQClamMasterParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamMasterParameter] UpdateFrom()");
	
	auto* other = dynamic_cast<TQClamMasterParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamMasterParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	fMasterConfigFile = other->fMasterConfigFile;
	fSourceConfigFile = other->fSourceConfigFile;
	fDriftchamConfigFile = other->fDriftchamConfigFile;
	fTdcMappingConfigFile = other->fTdcMappingConfigFile;
	fTriggerMappingConfigFile = other->fTriggerMappingConfigFile;
	fTriggerConditionConfigFile = other->fTriggerConditionConfigFile;
	fSignalEdgeConfigFile = other->fSignalEdgeConfigFile;
	fDrifttimeConfigFile = other->fDrifttimeConfigFile;
	fDrifttimeTimeWalkCorrFile = other->fDrifttimeTimeWalkCorrFile;
	fFaradayCupCurrentCalibrationFile = other->fFaradayCupCurrentCalibrationFile;
	fHitConditionConfigFile = other->fHitConditionConfigFile;
	fHitProcDebugOutputConfigFile = other->fHitProcDebugOutputConfigFile;
	fDisplaySettingConfigFile = other->fDisplaySettingConfigFile;
	fClusterConditionConfigFile = other->fClusterConditionConfigFile;
	fTrackConditionConfigFile = other->fTrackConditionConfigFile;
	fDriftToZOffsetTable = other->fDriftToZOffsetTable;
	fFocalPlaneConfigFile = other->fFocalPlaneConfigFile;
	fFitConfigFile = other->fFitConfigFile;
	fSpecCalibFile = other->fSpecCalibFile;
	fDrifttimeCalibConfigFile = other->fDrifttimeCalibConfigFile;
	fUnpackConfigFile = other->fUnpackConfigFile;

	fTcpConnectionFile = other->fTcpConnectionFile;
	fDagoCalibrationFile = other->fDagoCalibrationFile;

	fStoreConfigFile = other->fStoreConfigFile;
	fHistoFillingConfigFile = other->fHistoFillingConfigFile;

	return kTRUE;
}
