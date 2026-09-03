#ifdef __CLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class std::vector<unsigned short>+;
#pragma link C++ class std::vector<std::vector<unsigned short>>+;



/** constants */
//#pragma link C++ class TQClamConstants+;

/** analysis */
#pragma link C++ class TQClamAnalysis+;

// sources
#pragma link C++ class TQClamTcpSource+;

/** processors */
#pragma link C++ class TQClamHitProc+;
#pragma link C++ class TQClamClusterProc+;
#pragma link C++ class TQClamTrackProc+;
#pragma link C++ class TQClamBackgroundRateProc+;

//#pragma link C++ class TQClamUnpackProc+;

/** events */
#pragma link C++ class TQClamHitEvent+;
#pragma link C++ class TQClamClusterEvent+;
#pragma link C++ class TQClamTrackEvent+;
#pragma link C++ class TQClamTcpEvent+;
#pragma link C++ class TQClamBackgroundRateEvent+;


/** -- parameters -- */
#pragma link C++ class TQClamMasterParameter+;
#pragma link C++ class TQClamSourceParameter+;
#pragma link C++ class TQClamDriftchamGeomParameter+;
#pragma link C++ class TQClamFaradayCupCurrentCalibrationParameter+;
#pragma link C++ class TQClamDriftToZOffsetParameter+;
#pragma link C++ class TQClamDisplaySettingParameter+;
#pragma link C++ class TQClamEdgeMappingParameter+;
#pragma link C++ class TQClamDriftchamCalibrationParameter+;
#pragma link C++ class TQClamTriggerCalibrationParameter+;
#pragma link C++ class TQClamScalerValueParameter+;
#pragma link C++ class TQClamHitConditionParameter+;
#pragma link C++ class TQClamHitProcDebugOutputParameter+;
#pragma link C++ class TQClamClusterConditionParameter+;
#pragma link C++ class TQClamTrackConditionParameter+;
#pragma link C++ class TQClamFocalPlaneParameter+;
#pragma link C++ class TQClamSpectrumCalibrationParameter+;
#pragma link C++ class TQClamFitParameter+;
#pragma link C++ class TQClamDrifttimeCalibParameter+;
#pragma link C++ class TQClamUnpackParameter+;
#pragma link C++ class TQClamStoreParameter+;
#pragma link C++ class TQClamHistogramParameter+;
#pragma link C++ class TQClamSieveSlitCalibParameter+;


/** formeln fuer Fitter */
#pragma link C++ class TQClamFitModelSpectrum+;
#pragma link C++ class TQClamFitModelEMG+;
#pragma link C++ class TQClamFitModelTail1OverX+;
#pragma link C++ class TQClamFitModelGaussTail+;
#pragma link C++ class TQClamFitModelFermiDirac+;
#pragma link C++ class TQClamFitModelDrifttimeDist+;


/** stores */
#pragma link C++ class TQClamDrifttimeCalibStore+;
#pragma link C++ class TQClamTrackStore+;
#pragma link C++ class TQClamTrackTestingStore+;
#pragma link C++ class TQClamDriftlengthCalibStore+;
#pragma link C++ class TQClamDagoStore+;

/** -- QCLAM_SINGLE -- */
#ifdef QCLAM_SINGLE

#pragma link C++ class TQClamRawEvent+;
#pragma link C++ class TQClamUnpackProc+;

#endif // QCLAM_SINGLE


/** -- QCLAM_SINGLE_ROOT -- */
#ifdef QCLAM_SINGLE_ROOT

#pragma link C++ class TQClamRawEvent+;
#pragma link C++ class TQClamOfflineRootSource+; // user source, entpackte root-files --> nicht gelinkt... evtl aendern? TODO

#endif // QCLAM_SINGLE_ROOT


/** -- QCLAM_DAGOBERT -- */
#ifdef QCLAM_DAGOBERT

#pragma link C++ class TQClamDagobertRawEvent+;
#pragma link C++ class TQClamDagobertUnpackProc+;
#pragma link C++ class TDagobertHitProc+;
#pragma link C++ class TDagobertHitEvent+;
#pragma link C++ class TQClamDagobertCoincidenceEvent+;
#pragma link C++ class TQClamDagobertCoincidenceProc+;
#pragma link C++ class TQClamDagobertProviderProc+;
#pragma link C++ class TQClamDagobertDummyEvent+;
#pragma link C++ class TQClamTcpParameter+;
#pragma link C++ class TDagobertCalibrationParameter+;

#endif // QCLAM_DAGOBERT






#endif
