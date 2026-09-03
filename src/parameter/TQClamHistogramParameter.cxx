#include "parameter/TQClamHistogramParameter.h"

#include "TGo4Log.h"



ClassImp(TQClamHistogramParameter)

//////////////////////////////////////////////////////////////////////

TQClamHistogramParameter::TQClamHistogramParameter()
	: TGo4Parameter("HistoParam")
{
	TGo4Log::Info("Create %s.",GetName());
}




//////////////////////////////////////////////////////////////////////

TQClamHistogramParameter::TQClamHistogramParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("Create %s.", name);


	/** fuelle Histogramme */
	fBFillHistos = kTRUE;

                
	/** 1. HitProcessor */
	fBFillHitProc_SzintUpTime = kTRUE; // Fuelle TH1 'SzintUpTime' (HitProcessor)
	fBFillHitProc_SzintDownTime = kTRUE; // Fuelle TH1 'SzintDownTime' (HitProcessor)
	fBFillHitProc_CherenkovTime = kTRUE; // Fuelle TH1 'CherenkovTime' (HitProcessor)
	fBFillHitProc_MeanSzintTime = kTRUE; // Fuelle TH1 'MeanSzintUpDownTime' (HitProcessor)
	fBFillHitProc_DiffSzintTime = kTRUE; // Fuelle TH1 'DiiSzintUpDownTime' (HitProcessor)
	fBFillHitProc_SzintUpVsSzintDownTime = kTRUE; // Fuelle TH2 'TimeSzintUpVsSzintDown' (HitProcessor) 
                
	/** HitPattern je Ebene: Hits, EvtRate, TimeRate */
	fBFillHitProc_HitsInLayer = kTRUE; // Fuelle TH1 'HitsInLayer' (HitProcessor)
	fBFillHitProc_HitsInLayerCut = kTRUE; // Fuelle TH1 'HitsInLayerCond' (HitProcessor)
                
	fBFillHitProc_HitRateEvtInLayer = kTRUE; // Fuelle TH1 'HitRateEvtInLayer' (HitProcessor)
	fBFillHitProc_HitRateEvtInLayerCut = kTRUE; // Fuelle TH1 'HitRateEvtInLayerCond' (HitProcessor)
                
	fBFillHitProc_HitRateTimeInLayer = kTRUE; // Fuelle TH1 'HitRateTimeInLayer' (HitProcessor)
	fBFillHitProc_HitRateTimeInLayerCut = kTRUE; // Fuelle TH1 'HitRateTimeInLayerCond' (HitProcessor)


	/** HitPattern je Draht: Hits, EvtRate, TimeRate */
	fBFillHitProc_HitsPerWire = kTRUE; // Fuelle TH1 'HitsInLayer' (HitProcessor)
	fBFillHitProc_HitsPerWireCut = kTRUE; // Fuelle TH1 'HitsInLayerCond' (HitProcessor)
                
	fBFillHitProc_HitRateEvtPerWire = kTRUE; // Fuelle TH1 'HitRateEvtInLayer' (HitProcessor)
	fBFillHitProc_HitRateEvtPerWireCut = kTRUE; // Fuelle TH1 'HitRateEvtInLayerCond' (HitProcessor)
	fBFillHitProc_HitRateTimePerWire = kTRUE; // Fuelle TH1 'HitRateTimeInLayer' (HitProcessor)
	fBFillHitProc_HitRateTimePerWireCut = kTRUE; // Fuelle TH1 'HitRateTimeInLayerCond' (HitProcessor)


                
	/** Driftzeit-Verteilung */
	fBFillHitProc_DrifttimeDist = kTRUE; // Fuelle TH1 'DrifttimeDist' (HitProcessor)
	fBFillHitProc_DrifttimeDistCut = kTRUE; // Fuelle TH1 'DrifttimeDistCond' (HitProcessor)
	fBFillHitProc_DrifttimeDistTimeWalk = kTRUE; // Fuelle TH1 'DrifttimeDistTimeWalk' (HitProcessor)
	/** ToT-Verteilung */
	fBFillHitProc_ToTDist = kTRUE; // Fuelle TH1 'ToTDist' (HitProcessor)
	fBFillHitProc_ToTDistCut = kTRUE; // Fuelle TH1 'ToTDistCond' (HitProcessor)
	fBFillHitProc_ToTDistTimeWalk = kTRUE; // Fuelle TH1 'ToTDistTimeWalk' (HitProcessor)


	/** Driftzeit vs Draht */
	fBFillHitProc_DrifttimeVsWire = kTRUE; // Fuelle TH2 'ToTVsWire' (HitProcessor)
	fBFillHitProc_DrifttimeVsWireCut = kTRUE; // Fuelle TH2 'ToTVsWireCond' (HitProcessor)
	fBFillHitProc_DrifttimeVsWireTimeWalk = kTRUE; // Fuelle TH2 'ToTVsWireTimeWalk' (HitProcessor)


	/** ToT vs Draht */
	fBFillHitProc_ToTVsWire = kTRUE; // Fuelle TH2 'ToTVsWire' (HitProcessor)
	fBFillHitProc_ToTVsWireCut = kTRUE; // Fuelle TH2 'ToTVsWireCond' (HitProcessor)
	fBFillHitProc_ToTVsWireTimeWalk = kTRUE; // Fuelle TH2 'ToTVsWireTimeWalk' (HitProcessor)


	/** Driftzeit vs ToT */
	fBFillHitProc_DrifttimeVsToT = kTRUE; // Fuelle TH2 'DrifttimeVsToT' (HitProcessor)
	fBFillHitProc_DrifttimeVsToTCut = kTRUE; // Fuelle TH2 'DrifttimeVsToTCond' (HitProcessor)
	fBFillHitProc_DrifttimeVsToTTimeWalk = kTRUE; // Fuelle TH2 'DrifttimeVsToTTimeWalk' (HitProcessor)




	fBFillHitProc_FaradayCupCurrent = kTRUE; // Fuelle TGo4RollingGraph 'FaradayCupCurrent' (HitProcessor)
	fBFillHitProc_ScalerTriggerRate = kTRUE; // Fuelle TGo4RollingGraph 'ScalerTriggerRate' (HitProcessor)
	fBFillHitProc_SzintUpTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'SzintillatorPmtUp' (HitProcessor)
	fBFillHitProc_SzintDownTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'SzintillatorPmtDown' (HitProcessor)
	fBFillHitProc_CherenkovTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'CherenkovPmtTrigger' (HitProcessor)
	fBFillHitProc_CoincTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'CoincidenceTrigger' (HitProcessor)
	fBFillHitProc_AccTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'AcceptedTrigger' (HitProcessor)
	fBFillHitProc_RelDeadtime = kTRUE; // Fuelle TGo4RollingGraph 'RelDeadtime' (HitProcessor)
	fBFillHitProc_RelMbsDeadtime = kTRUE; // Fuelle TGo4RollingGraph 'RelMbsDeadtime' (HitProcessor)



	/** 2. ClusterProc */

	fBFillClusterProc_HitNumberInCluster = kTRUE; // Fuelle TH1 'HitNumberInCluster' X1, U, X2 (ClusterProcessor)
	fBFillClusterProc_WireSpanInCluster = kTRUE; // Fuelle TH1 'WireSpanInCluster' X1, U, X2 (ClusterProcessor)
	fBFillClusterProc_HitNumberVsWireSpan = kTRUE; // Fuelle TH2 'HitNumberVsWireSpan' X1, U, X2 (ClusterProcessor)
                
	fBFillClusterProc_XMidCluster = kTRUE; // Fuelle TH1 'XMidCluster' X1, U, X2 (ClusterProcessor)
	fBFillClusterProc_XMidClusterCut = kTRUE; // Fuelle TH1 'XMidClusterCond' X1, U, X2 (ClusterProcessor)
                
	fBFillClusterProc_WireNumberWithMinDrifttimeInCluster = kFALSE; // Fuelle TH1 'WireNumberWithMinDrifttimeInCluster' X1, U, X2 (ClusterProcessor)
	fBFillClusterProc_WireNumberWithMinDrifttimeInClusterCut = kFALSE; // Fuelle TH1 'WireNumberWithMinDrifttimeInClusterCond' X1, U, X2 (ClusterProcessor)
                
	fBFillClusterProc_MinDrifttimeInCluster = kTRUE; // Fuelle TH1 'MinDrifttimeInCluster' X1, U, X2 (ClusterProcessor)
	fBFillClusterProc_MinDrifttimeInClusterCut = kTRUE; // Fuelle TH1 'MinDrifttimeInClusterCond' X1, U, X2 (ClusterProcessor)
                
	fBFillClusterProc_ClusterPropertiesBeforeCut = kTRUE; // Fuelle TH2 'ClusterPropertiesBeforeCut' (ClusterProcessor)
	fBFillClusterProc_ClusterPropertiesAfterCut = kTRUE; // Fuelle TH2 'ClusterPropertiesAfterCut' (ClusterProcessor)
                
	fBFillClusterProc_ClusterCoincidences = kTRUE; // Fuelle TH1 'ClusterCoincidences'
	fBFillClusterProc_ClusterCoincidencesRel = kTRUE; // Fuelle TH1 'ClusterCoincidencesRel' (ClusterProcessor)
                
	fBFillClusterProc_ClusterEfficiency = kTRUE; // Fuelle TH1 'ClusterEffeciency' X1, U, X2 (wie viel Cluster ueberleben Cuts) (ClusterProcessor)



	fBFillClusterProc_ClusterRemovedHits = kTRUE; // Fuelle TH1 'ClusterRemovedHits' (ClusterProcessor)
	fBFillClusterProc_ClusterSizeBeforeClean = kTRUE; // Fuelle TH1 'ClusterSizeBeforeClean' (ClusterProcessor)
	fBFillClusterProc_ClusterSizeAfterClean = kTRUE; // Fuelle TH1 'ClusterSizeAfterClean' (ClusterProcessor)
	fBFillClusterProc_ClusterSizeBeforeVsAfterClean = kTRUE; // Fuelle TH1 'ClusterSizeBeforeVsAfterClean' (ClusterProcessor)
	fBFillClusterProc_ClusterShapeChange = kTRUE; // Fuelle TH1 'ClusterShapeChange' (ClusterProcessor)
	fBFillClusterProc_DrifttimeJump = kTRUE; // Fuelle TH1 'DrifttimeJump' (ClusterProcessor)
	fBFillClusterProc_DrifttimeResidualAll = kTRUE; // Fuelle TH1 'DrifttimeResidualAll' (ClusterProcessor)
	fBFillClusterProc_DrifttimeResidualRemoved = kTRUE; // Fuelle TH1 'DrifttimeResidualRemoved' (ClusterProcessor)




	/** 3. TrackProc */
	fBFillTrackProc_XSpectrumX1Plane = kTRUE; // Fuelle TH1 'XSpectrumX1Plane' (TrackProc)
	fBFillTrackProc_YSpectrumX1Plane = kTRUE; // Fuelle TH1 'YSpectrumX1Plane' (TrackProc)
	fBFillTrackProc_XSpectrumUPlane = kTRUE; // Fuelle TH1 'XSpectrumUPlane' (TrackProc)
	fBFillTrackProc_YSpectrumUPlane = kTRUE; // Fuelle TH1 'YSpectrumUPlane' (TrackProc)
	fBFillTrackProc_XSpectrumX2Plane = kTRUE; // Fuelle TH1 'XSpectrumX2Plane' (TrackProc)
	fBFillTrackProc_USpectrum = kTRUE; // Fuelle TH1 'USpectrum' (TrackProc)

	fBFillTrackProc_ProjSpectrum = kTRUE; // Fuelle TH1 'ProjSpectrum' (TrackProc)
	fBFillTrackProc_XCorrSpectrum = kTRUE; // Fuelle TH1 'XCorrSpectrum' (TrackProc)

	fBFillTrackProc_PhiSpectrum = kTRUE; // Fuelle TH1 'PhiSpectrum' (TrackProc)
	fBFillTrackProc_ThetaSpectrum = kTRUE; // Fuelle TH1 'ThetaSpectrum' (TrackProc)

	fBFillTrackProc_PhiVsX1 = kTRUE; // Fuelle TH2 'PhiVsX1' (TrackProc)
	fBFillTrackProc_PhiVsX2 = kTRUE; // Fuelle TH2 'PhiVsX2' (TrackProc)
	fBFillTrackProc_PhiVsXCorr = kTRUE; // Fuelle TH2 'PhiVsXCorr' (TrackProc)

	fBFillTrackProc_ThetaVsY = kTRUE; // Fuelle TH2 'ThetaVsY' (TrackProc)

	fBFillTrackProc_Chi2 = kTRUE; // Fuelle TH1 'Chi2' (TrackProc)
	fBFillTrackProc_Chi2NDF = kTRUE; // Fuelle TH1 'Chi2NDF' (TrackProc)

	fBFillTrackProc_MaxResidualSigned = kTRUE; // Fuelle TH1 'MaxResidualSigned' (TrackProc)
	fBFillTrackProc_MaxResidualAbs = kTRUE; // Fuelle TH1 'MaxResidualAbs' (TrackProc)
	fBFillTrackProc_MeanResidual = kTRUE; // Fuelle TH1 'MeanResidual' (TrackProc)

	fBFillTrackProc_Converged = kTRUE; // Fuelle TH1 'Converged' (TrackProc)
	fBFillTrackProc_Iterations = kTRUE; // Fuelle TH1 'Iterations' (TrackProc)
	fBFillTrackProc_Rejected = kTRUE; // Fuelle TH1 'Rejected' (TrackProc)

	fBFillTrackProc_Pull = kTRUE; // Fuelle TH1 'Pull' (TrackProc)
	fBFillTrackProc_ResidualVsWire = kTRUE; // Fuelle TH1 'ResidualVsWire' X1,U,X2 (TrackProc)


	fBFillTrackProc_MeanResidualX1 = kTRUE; // Fuelle TH1 'MeanResidualX1' (TrackProc)
       	fBFillTrackProc_MeanResidualU = kTRUE; // Fuelle TH1 'MeanResidualU' (TrackProc)
	fBFillTrackProc_MeanResidualX2 = kTRUE; // Fuelle TH1 'MeanResidualX2' (TrackProc)

	fBFillTrackProc_RmsResidualX1 = kTRUE; // Fuelle TH1 'RmsResidualX1' (TrackProc)
	fBFillTrackProc_RmsResidualU = kTRUE; // Fuelle TH1 'RmsResidualU' (TrackProc)
	fBFillTrackProc_RmsResidualX2 = kTRUE; // Fuelle TH1 'RmsResidualX2' (TrackProc)

	fBFillTrackProc_ResidualX1 = kTRUE; // Fuelle TH1 'ResidualX1' (TrackProc)
	fBFillTrackProc_ResidualU = kTRUE; // Fuelle TH1 'ResidualU' (TrackProc)
	fBFillTrackProc_ResidualX2 = kTRUE; // Fuelle TH1 'ResidualX2' (TrackProc)




	/** DagobertHitProc */

	/** CoincidenceProc */






}





Bool_t TQClamHistogramParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamHistogramParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamHistogramParameter*>(rhs);

	if(!other)
	{
		TGo4Log::Error("[TQClamHistogramParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	// TODO

	return kTRUE;
}
