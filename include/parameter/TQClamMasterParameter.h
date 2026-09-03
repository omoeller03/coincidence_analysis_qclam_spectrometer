#ifndef TQCLAMMASTERPARAMETER_H
#define TQCLAMMASTERPARAMETER_H

#include "TGo4Parameter.h"

#include "Rtypes.h"

class TQClamMasterParameter : public TGo4Parameter
{
	public:
		TQClamMasterParameter();
		TQClamMasterParameter(const char* name);

		/** pfade zu config-files */
		TString fMasterConfigFile = "configs/master.yaml"; // Pfad zu MasterParameter Config File (relativ zum Projektordner)

		TString fSourceConfigFile; // Pfad zu SourceParameter Config-File
		TString fDriftchamConfigFile; // Pfad zu DriftchamGeomParameter Config-File
		TString fTdcMappingConfigFile; // Pfad zu TdcMappingParameter Config-File
		TString fTriggerMappingConfigFile; // Pfad zu Trigger-Mapping Config-File
		TString fTriggerConditionConfigFile; // Pfad zu Trigger-Condition Config-File
		TString fSignalEdgeConfigFile; // Pfad zu SignalEdgeParameter Config-File
		TString fDrifttimeConfigFile; // Pfad zu DrifttimeParameter Config-File
		TString fDrifttimeTimeWalkCorrFile; // Pfad zur Time-Walk Korrektur Config-File
		TString fFaradayCupCurrentCalibrationFile; // Pfad zu Faraday-Cup Strom Kalibrierdatei
		TString fHitConditionConfigFile; // Pfad zu HitConditionParameter Config-File
		TString fHitProcDebugOutputConfigFile; // Pfad zu HitProcDebugOutputParameter Config-File
		TString fDisplaySettingConfigFile; // Pfad zur Konfifurationsdatei der Histogramm-/Graph-/...-Setzwerte
		TString fClusterConditionConfigFile; // Pfad zu ClusterConditionParameter Config-File
		TString fTrackConditionConfigFile; // Pfad zu TrackConditionParameter Config-File
		TString fDriftToZOffsetTable; // Tabelle zur Umrechnung (Driftzeit,Phi_Winkel) --> zOffset
		TString fFocalPlaneConfigFile; // Funktionsvorschrift der Fokalebene
		TString fFitConfigFile; // Konfigurations-Datei fuer Spektrum-Fits und Energiekalibrierung
		TString fSpecCalibFile; // Konfigurations-Datei fuer Spektrum-Kalibrierungen
		TString fSieveSlitCalibFile; // Kalibrationsdaten fuer Sieve-Slit (Abbildungsmatrix)
		TString fDrifttimeCalibConfigFile; // Konfigurations-Datei fuer Driftzeit-Offset Kalibrierung (-> Fitte an Flanke Driftzeit-Spektrum)
		TString fUnpackConfigFile; // Konfigurations-Datei Spektrum-Kalibrierung: X-Phi Korrektur, Energiekalibrierung

		TString fTcpConnectionFile; // Konfigurations-Daten fuer Tcp-Verbindung
		TString fDagoCalibrationFile; // Konfigurations-Datei fuer Dagobert-Kalibration
	
		TString fStoreConfigFile; // Konfigurations-Datei fuer Store-Parameter
		TString fHistoFillingConfigFile; // Konfigurations-Datei fuer Histogram-Parameter

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;
		
	ClassDefOverride(TQClamMasterParameter,1);
};

#endif
