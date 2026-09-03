#ifndef TQCLAMHISTOGRAMPARAMETER_H
#define TQCLAMHISTOGRAMPARAMETER_H

#include "TGo4Parameter.h"

#include "Rtypes.h"

class TQClamHistogramParameter : public TGo4Parameter
{
	public:
		TQClamHistogramParameter();
		TQClamHistogramParameter(const char* name);

		/** fuelle Histogramme */
		Bool_t fBFillHistos = kTRUE;


		/** 1. HitProcessor */
		Bool_t fBFillHitProc_SzintUpTime = kTRUE; // Fuelle TH1 'SzintUpTime' (HitProcessor)
		Bool_t fBFillHitProc_SzintDownTime = kTRUE; // Fuelle TH1 'SzintDownTime' (HitProcessor)
		Bool_t fBFillHitProc_CherenkovTime = kTRUE; // Fuelle TH1 'CherenkovTime' (HitProcessor)
		Bool_t fBFillHitProc_MeanSzintTime = kTRUE; // Fuelle TH1 'MeanSzintUpDownTime' (HitProcessor)
		Bool_t fBFillHitProc_DiffSzintTime = kTRUE; // Fuelle TH1 'DiiSzintUpDownTime' (HitProcessor)
		Bool_t fBFillHitProc_SzintUpVsSzintDownTime = kTRUE; // Fuelle TH2 'TimeSzintUpVsSzintDown' (HitProcessor) 

		/** HitPattern je Ebene: Hits, EvtRate, TimeRate */
		Bool_t fBFillHitProc_HitsInLayer = kTRUE; // Fuelle TH1 'HitsInLayer' (HitProcessor)
		Bool_t fBFillHitProc_HitsInLayerCut = kTRUE; // Fuelle TH1 'HitsInLayerCond' (HitProcessor)

		Bool_t fBFillHitProc_HitRateEvtInLayer = kTRUE; // Fuelle TH1 'HitRateEvtInLayer' (HitProcessor)
		Bool_t fBFillHitProc_HitRateEvtInLayerCut = kTRUE; // Fuelle TH1 'HitRateEvtInLayerCond' (HitProcessor)

		Bool_t fBFillHitProc_HitRateTimeInLayer = kTRUE; // Fuelle TH1 'HitRateTimeInLayer' (HitProcessor)
		Bool_t fBFillHitProc_HitRateTimeInLayerCut = kTRUE; // Fuelle TH1 'HitRateTimeInLayerCond' (HitProcessor)


		/** HitPattern je Draht: Hits, EvtRate, TimeRate */
		Bool_t fBFillHitProc_HitsPerWire = kTRUE; // Fuelle TH1 'HitsInLayer' (HitProcessor)
		Bool_t fBFillHitProc_HitsPerWireCut = kTRUE; // Fuelle TH1 'HitsInLayerCond' (HitProcessor)

		Bool_t fBFillHitProc_HitRateEvtPerWire = kTRUE; // Fuelle TH1 'HitRateEvtInLayer' (HitProcessor)
		Bool_t fBFillHitProc_HitRateEvtPerWireCut = kTRUE; // Fuelle TH1 'HitRateEvtInLayerCond' (HitProcessor)

		Bool_t fBFillHitProc_HitRateTimePerWire = kTRUE; // Fuelle TH1 'HitRateTimeInLayer' (HitProcessor)
		Bool_t fBFillHitProc_HitRateTimePerWireCut = kTRUE; // Fuelle TH1 'HitRateTimeInLayerCond' (HitProcessor)


		/** Driftzeit-Verteilung */
		Bool_t fBFillHitProc_DrifttimeDist = kTRUE; // Fuelle TH1 'DrifttimeDist' (HitProcessor)
		Bool_t fBFillHitProc_DrifttimeDistCut = kTRUE; // Fuelle TH1 'DrifttimeDistCond' (HitProcessor)
		Bool_t fBFillHitProc_DrifttimeDistTimeWalk = kTRUE; // Fuelle TH1 'DrifttimeDistTimeWalk' (HitProcessor)
		/** ToT-Verteilung */
		Bool_t fBFillHitProc_ToTDist = kTRUE; // Fuelle TH1 'ToTDist' (HitProcessor)
		Bool_t fBFillHitProc_ToTDistCut = kTRUE; // Fuelle TH1 'ToTDistCond' (HitProcessor)
		Bool_t fBFillHitProc_ToTDistTimeWalk = kTRUE; // Fuelle TH1 'ToTDistTimeWalk' (HitProcessor)


		/** Driftzeit vs Draht */
		Bool_t fBFillHitProc_DrifttimeVsWire = kTRUE; // Fuelle TH2 'ToTVsWire' (HitProcessor)
		Bool_t fBFillHitProc_DrifttimeVsWireCut = kTRUE; // Fuelle TH2 'ToTVsWireCond' (HitProcessor)
		Bool_t fBFillHitProc_DrifttimeVsWireTimeWalk = kTRUE; // Fuelle TH2 'ToTVsWireTimeWalk' (HitProcessor)


		/** ToT vs Draht */
		Bool_t fBFillHitProc_ToTVsWire = kTRUE; // Fuelle TH2 'ToTVsWire' (HitProcessor)
		Bool_t fBFillHitProc_ToTVsWireCut = kTRUE; // Fuelle TH2 'ToTVsWireCond' (HitProcessor)
		Bool_t fBFillHitProc_ToTVsWireTimeWalk = kTRUE; // Fuelle TH2 'ToTVsWireTimeWalk' (HitProcessor)


		/** Driftzeit vs ToT */
		Bool_t fBFillHitProc_DrifttimeVsToT = kTRUE; // Fuelle TH2 'DrifttimeVsToT' (HitProcessor)
		Bool_t fBFillHitProc_DrifttimeVsToTCut = kTRUE; // Fuelle TH2 'DrifttimeVsToTCond' (HitProcessor)
		Bool_t fBFillHitProc_DrifttimeVsToTTimeWalk = kTRUE; // Fuelle TH2 'DrifttimeVsToTTimeWalk' (HitProcessor)



		Bool_t fBFillHitProc_FaradayCupCurrent = kTRUE; // Fuelle TGo4RollingGraph 'FaradayCupCurrent' (HitProcessor)
		Bool_t fBFillHitProc_ScalerTriggerRate = kTRUE; // Fuelle TGo4RollingGraph 'ScalerTriggerRate' (HitProcessor)
		Bool_t fBFillHitProc_SzintUpTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'SzintillatorPmtUp' (HitProcessor)
		Bool_t fBFillHitProc_SzintDownTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'SzintillatorPmtDown' (HitProcessor)
		Bool_t fBFillHitProc_CherenkovTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'CherenkovPmtTrigger' (HitProcessor)
		Bool_t fBFillHitProc_CoincTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'CoincidenceTrigger' (HitProcessor)
		Bool_t fBFillHitProc_AccTrigRate = kTRUE; // Fuelle TGo4RollingGraph 'AcceptedTrigger' (HitProcessor)
		Bool_t fBFillHitProc_RelDeadtime = kTRUE; // Fuelle TGo4RollingGraph 'RelDeadtime' (HitProcessor)
		Bool_t fBFillHitProc_RelMbsDeadtime = kTRUE; // Fuelle TGo4RollingGraph 'RelMbsDeadtime' (HitProcessor)






		/** 2. ClusterProc */

		Bool_t fBFillClusterProc_HitNumberInCluster = kTRUE; // Fuelle TH1 'HitNumberInCluster' X1, U, X2 (ClusterProcessor)
		Bool_t fBFillClusterProc_WireSpanInCluster = kTRUE; // Fuelle TH1 'WireSpanInCluster' X1, U, X2 (ClusterProcessor)
		Bool_t fBFillClusterProc_HitNumberVsWireSpan = kTRUE; // Fuelle TH2 'HitNumberVsWireSpan' X1, U, X2 (ClusterProcessor)

		Bool_t fBFillClusterProc_XMidCluster = kTRUE; // Fuelle TH1 'XMidCluster' X1, U, X2 (ClusterProcessor)
		Bool_t fBFillClusterProc_XMidClusterCut = kTRUE; // Fuelle TH1 'XMidClusterCond' X1, U, X2 (ClusterProcessor)

		Bool_t fBFillClusterProc_WireNumberWithMinDrifttimeInCluster = kFALSE; // Fuelle TH1 'WireNumberWithMinDrifttimeInCluster' X1, U, X2 (ClusterProcessor)
		Bool_t fBFillClusterProc_WireNumberWithMinDrifttimeInClusterCut = kFALSE; // Fuelle TH1 'WireNumberWithMinDrifttimeInClusterCond' X1, U, X2 (ClusterProcessor)

		Bool_t fBFillClusterProc_MinDrifttimeInCluster = kTRUE; // Fuelle TH1 'MinDrifttimeInCluster' X1, U, X2 (ClusterProcessor)
		Bool_t fBFillClusterProc_MinDrifttimeInClusterCut = kTRUE; // Fuelle TH1 'MinDrifttimeInClusterCond' X1, U, X2 (ClusterProcessor)

		Bool_t fBFillClusterProc_ClusterPropertiesBeforeCut = kTRUE; // Fuelle TH2 'ClusterPropertiesBeforeCut' (ClusterProcessor)
		Bool_t fBFillClusterProc_ClusterPropertiesAfterCut = kTRUE; // Fuelle TH2 'ClusterPropertiesAfterCut' (ClusterProcessor)

		Bool_t fBFillClusterProc_ClusterCoincidences = kTRUE; // Fuelle TH1 'ClusterCoincidences'
		Bool_t fBFillClusterProc_ClusterCoincidencesRel = kTRUE; // Fuelle TH1 'ClusterCoincidencesRel' (ClusterProcessor)

		Bool_t fBFillClusterProc_ClusterEfficiency = kTRUE; // Fuelle TH1 'ClusterEffeciency' X1, U, X2 (wie viel Cluster ueberleben Cuts) (ClusterProcessor)


		Bool_t fBFillClusterProc_ClusterRemovedHits = kTRUE; // Fuelle TH1 'ClusterRemovedHits' (ClusterProcessor)
		Bool_t fBFillClusterProc_ClusterSizeBeforeClean = kTRUE; // Fuelle TH1 'ClusterSizeBeforeClean' (ClusterProcessor)
		Bool_t fBFillClusterProc_ClusterSizeAfterClean = kTRUE; // Fuelle TH1 'ClusterSizeAfterClean' (ClusterProcessor)
		Bool_t fBFillClusterProc_ClusterSizeBeforeVsAfterClean = kTRUE; // Fuelle TH1 'ClusterSizeBeforeVsAfterClean' (ClusterProcessor)
		Bool_t fBFillClusterProc_ClusterShapeChange = kTRUE; // Fuelle TH1 'ClusterShapeChange' (ClusterProcessor)
		Bool_t fBFillClusterProc_DrifttimeJump = kTRUE; // Fuelle TH1 'DrifttimeJump' (ClusterProcessor)
		Bool_t fBFillClusterProc_DrifttimeResidualAll = kTRUE; // Fuelle TH1 'DrifttimeResidualAll' (ClusterProcessor)
		Bool_t fBFillClusterProc_DrifttimeResidualRemoved = kTRUE; // Fuelle TH1 'DrifttimeResidualRemoved' (ClusterProcessor)




		/** 3. TrackProc */

		Bool_t fBFillTrackProc_XSpectrumX1Plane = kTRUE; // Fuelle TH1 'XSpectrumX1Plane' (TrackProc)
		Bool_t fBFillTrackProc_YSpectrumX1Plane = kTRUE; // Fuelle TH1 'YSpectrumX1Plane' (TrackProc)
		Bool_t fBFillTrackProc_XSpectrumUPlane = kTRUE; // Fuelle TH1 'XSpectrumUPlane' (TrackProc)
		Bool_t fBFillTrackProc_YSpectrumUPlane = kTRUE; // Fuelle TH1 'YSpectrumUPlane' (TrackProc)
		Bool_t fBFillTrackProc_XSpectrumX2Plane = kTRUE; // Fuelle TH1 'XSpectrumX2Plane' (TrackProc)
		Bool_t fBFillTrackProc_USpectrum = kTRUE; // Fuelle TH1 'USpectrum' (TrackProc)

		Bool_t fBFillTrackProc_ProjSpectrum = kTRUE; // Fuelle TH1 'ProjSpectrum' (TrackProc)
		Bool_t fBFillTrackProc_XCorrSpectrum = kTRUE; // Fuelle TH1 'XCorrSpectrum' (TrackProc)

		Bool_t fBFillTrackProc_PhiSpectrum = kTRUE; // Fuelle TH1 'PhiSpectrum' (TrackProc)
		Bool_t fBFillTrackProc_ThetaSpectrum = kTRUE; // Fuelle TH1 'ThetaSpectrum' (TrackProc)

		Bool_t fBFillTrackProc_PhiVsX1 = kTRUE; // Fuelle TH2 'PhiVsX1' (TrackProc)
		Bool_t fBFillTrackProc_PhiVsX2 = kTRUE; // Fuelle TH2 'PhiVsX2' (TrackProc)
		Bool_t fBFillTrackProc_PhiVsXCorr = kTRUE; // Fuelle TH2 'PhiVsXCorr' (TrackProc)

		Bool_t fBFillTrackProc_ThetaVsY = kTRUE; // Fuelle TH2 'ThetaVsY' (TrackProc)

		Bool_t fBFillTrackProc_Chi2 = kTRUE; // Fuelle TH1 'Chi2' (TrackProc)
		Bool_t fBFillTrackProc_Chi2NDF = kTRUE; // Fuelle TH1 'Chi2NDF' (TrackProc)

		Bool_t fBFillTrackProc_MaxResidualSigned = kTRUE; // Fuelle TH1 'MaxResidualSigned' (TrackProc)
		Bool_t fBFillTrackProc_MaxResidualAbs = kTRUE; // Fuelle TH1 'MaxResidualAbs' (TrackProc)
		Bool_t fBFillTrackProc_MeanResidual = kTRUE; // Fuelle TH1 'MeanResidual' (TrackProc)

		Bool_t fBFillTrackProc_Converged = kTRUE; // Fuelle TH1 'Converged' (TrackProc)
		Bool_t fBFillTrackProc_Iterations = kTRUE; // Fuelle TH1 'Iterations' (TrackProc)
		Bool_t fBFillTrackProc_Rejected = kTRUE; // Fuelle TH1 'Rejected' (TrackProc)

		Bool_t fBFillTrackProc_Pull = kTRUE; // Fuelle TH1 'Pull' (TrackProc)
		Bool_t fBFillTrackProc_ResidualVsWire = kTRUE; // Fuelle TH1 'ResidualVsWire' X1,U,X2 (TrackProc)

		Bool_t fBFillTrackProc_MeanResidualX1 = kTRUE; // Fuelle TH1 'MeanResidualX1' (TrackProc)
		Bool_t fBFillTrackProc_MeanResidualU = kTRUE; // Fuelle TH1 'MeanResidualU' (TrackProc)
		Bool_t fBFillTrackProc_MeanResidualX2 = kTRUE; // Fuelle TH1 'MeanResidualX2' (TrackProc)

		Bool_t fBFillTrackProc_RmsResidualX1 = kTRUE; // Fuelle TH1 'RmsResidualX1' (TrackProc)
		Bool_t fBFillTrackProc_RmsResidualU = kTRUE; // Fuelle TH1 'RmsResidualU' (TrackProc)
		Bool_t fBFillTrackProc_RmsResidualX2 = kTRUE; // Fuelle TH1 'RmsResidualX2' (TrackProc)

		Bool_t fBFillTrackProc_ResidualX1 = kTRUE; // Fuelle TH1 'ResidualX1' (TrackProc)
		Bool_t fBFillTrackProc_ResidualU = kTRUE; // Fuelle TH1 'ResidualU' (TrackProc)
		Bool_t fBFillTrackProc_ResidualX2 = kTRUE; // Fuelle TH1 'ResidualX2' (TrackProc)




		/** DagobertHitProc */

		/** CoincidenceProc */

		/** UpdateFrom */
		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


	ClassDefOverride(TQClamHistogramParameter,1);
};


#endif
