#ifndef TQCLAMCLUSTERPROC_H
#define TQCLAMCLUSTERPROC_H

#include "TGo4EventProcessor.h"

/** geometry */
#include "geometry/TQClamDriftcham.h"

#include <vector>
#include <map>

/** evts */
struct TQClamCluster;
struct TQClamHit;

/** parameter */
class TQClamClusterConditionParameter;
class TQClamDriftchamGeomParameter;
class TQClamHistogramParameter;

/** conditions */
class TGo4CondArray;
class TGo4WinCond;
class TGo4CounterCond;

/** ROOT */
class TH1;
class TH2;


class TQClamClusterProc : public TGo4EventProcessor
{
	public:
		TQClamClusterProc();
		TQClamClusterProc(const char* name);
		virtual ~TQClamClusterProc();


		/** pruefe klasse von output-eventobjekt */
		Bool_t CheckEventClass(TClass* cl) override;

		/** baue output-eventobjekt */
		Bool_t BuildEvent(TGo4EventElement* dest) override;


	private:
		const TQClamDriftcham& fDriftcham;

		/** -- Parameter, Windows, Histogramme & Variablen speichern -- */

                //-----------------------------------------------------------
                // 1. Parameter
		TQClamClusterConditionParameter* fClusterConditionPar{nullptr};
		TQClamDriftchamGeomParameter* fDriftchamGeomPar{nullptr};
		TQClamHistogramParameter* fHistoPar{nullptr};


                //-----------------------------------------------------------
		// 2. Conditions
		TGo4CondArray* fCondArrWireSpanInCluster{nullptr};
		TGo4CondArray* fCondArrHitNumberInCluster{nullptr};
		TGo4CondArray* fCondArrHitNumberVsWireSpan{nullptr};
		/*
		TGo4CondArray* fCondArrClusterVShape{nullptr};
		TGo4CondArray* fCondArrClusterAscending{nullptr};
		TGo4CondArray* fCondarrClusterDescending{nullptr};
		*/
		std::vector<TGo4CounterCond*> fCondClusterVShape;
		std::vector<TGo4CounterCond*> fCondClusterAscending;
		std::vector<TGo4CounterCond*> fCondClusterDescending;
		std::vector<TGo4CounterCond*> fCondClusterHasGaps;

                /** zaehle, wie viele Events ClusterProc passieren */
                TGo4CounterCond* fCondCountClusterProcPassed{nullptr};
                TGo4CounterCond* fCondCountClusterProcPassedX1{nullptr};
                TGo4CounterCond* fCondCountClusterProcPassedU{nullptr};
                TGo4CounterCond* fCondCountClusterProcPassedX2{nullptr};


		/** condition fuer cluster-event statistik:
		 * X1  1   1   1   0   1   0   0   0
		 * U   1   0   1   1   0   1   0   0
		 * X2  1   1   0   1   0   0   1   0 */
		TGo4CounterCond* fCondHave_X1_U_X2;
		TGo4CounterCond* fCondHave_X1_U;
		TGo4CounterCond* fCondHave_X1_X2;
		TGo4CounterCond* fCondHave_U_X2;
		TGo4CounterCond* fCondHave_X1;
		TGo4CounterCond* fCondHave_U;
		TGo4CounterCond* fCondHave_X2;
		TGo4CounterCond* fCondHave_0;



                //-----------------------------------------------------------
		// 3. Histogramme

		/** Hit-Anzahl in Cluster */
		TH1* fHHitNumberInClusterX1{nullptr};
		TH1* fHHitNumberInClusterU{nullptr};
		TH1* fHHitNumberInClusterX2{nullptr};

		TH1* fHHitNumberInClusterX1Cut{nullptr};
		TH1* fHHitNumberInClusterUCut{nullptr};
		TH1* fHHitNumberInClusterX2Cut{nullptr};

		/** Drahtspanne in Cluster */
		TH1* fHWireSpanInClusterX1{nullptr};
		TH1* fHWireSpanInClusterU{nullptr};
		TH1* fHWireSpanInClusterX2{nullptr};

		TH1* fHWireSpanInClusterX1Cut{nullptr};
		TH1* fHWireSpanInClusterUCut{nullptr};
		TH1* fHWireSpanInClusterX2Cut{nullptr};

		/** 2D-Plot HitNumber vs. WireSpan */
		TH2* fHHitNumberVsWireSpanX1{nullptr};
		TH2* fHHitNumberVsWireSpanU{nullptr};
		TH2* fHHitNumberVsWireSpanX2{nullptr};

		TH2* fHHitNumberVsWireSpanX1Cut{nullptr};
		TH2* fHHitNumberVsWireSpanUCut{nullptr};
		TH2* fHHitNumberVsWireSpanX2Cut{nullptr};


		/** Cluster-mids plotten --> erstes "schlechtes" spektrum */
		TH1* fHXMidClusterX1{nullptr};
		TH1* fHXMidClusterU{nullptr};
		TH1* fHXMidClusterX2{nullptr};

//		TH1* fHYMidClusterU{nullptr};


		TH1* fHXMidClusterX1Cut{nullptr};
		TH1* fHXMidClusterUCut{nullptr};
		TH1* fHXMidClusterX2Cut{nullptr};

//		TH1* fHYMidClusterUCut{nullptr};


		/** Drahtnummern mit min Driftzeit in Cluster */
		TH1* fHWireNumberWithMinDrifttimeInClusterX1{nullptr};
		TH1* fHWireNumberWithMinDrifttimeInClusterU{nullptr};
		TH1* fHWireNumberWithMinDrifttimeInClusterX2{nullptr};

		TH1* fHWireNumberWithMinDrifttimeInClusterX1Cut{nullptr};
		TH1* fHWireNumberWithMinDrifttimeInClusterUCut{nullptr};
		TH1* fHWireNumberWithMinDrifttimeInClusterX2Cut{nullptr};


		/** minimale driftzeit in clustern */
		TH1* fHMinDrifttimeInClusterX1{nullptr};
		TH1* fHMinDrifttimeInClusterU{nullptr};
		TH1* fHMinDrifttimeInClusterX2{nullptr};

		TH1* fHMinDrifttimeInClusterX1Cut{nullptr};
		TH1* fHMinDrifttimeInClusterUCut{nullptr};
		TH1* fHMinDrifttimeInClusterX2Cut{nullptr};

		/** Cluster-Eigenschaften (Gaps, NoGaps, VShape, Ascending, Descending) */
		TH2* fHClusterPropertiesBeforeCut{nullptr};
		TH2* fHClusterPropertiesAfterCut{nullptr};

		/** Cluster-Koinzidenzen */
		TH1* fHClusterCoincidences{nullptr};
		TH1* fHClusterCoincidencesRel{nullptr};

		/** Cluster-Koinzidenzen */
		TH1* fHClusterCoincidencesCut{nullptr};
		TH1* fHClusterCoincidencesRelCut{nullptr};

		/** Effizienzen pro Layer  ... wie viele ueberleben cuts ... hmm */
		TH1* fHClusterConditionEfficiency{nullptr};
		TH1* fHClusterEfficiency{nullptr};




		/** - Kriterien fuer Cleaning - */

		/** Hit-Verlust durch Cleaning */
		TH1* fHClusterRemovedHits{nullptr};
		
		/** ClusterGroesse vor/nach Cleaning */
		TH1* fHClusterSizeBeforeClean{nullptr};
		TH1* fHClusterSizeAfterClean{nullptr};
		TH2* fHClusterSizeBeforeVsAfterClean{nullptr};

		/** effekt cluster-shape auf shape */
		TH1* fHClusterShapeChange{nullptr};

		/** Drifttime-Jump Histogramm (fuer Cleaning-Kriterium) */
		TH1* fHDrifttimeJump{nullptr};
		TH1* fHDrifttimeResidualAll{nullptr};
		TH1* fHDrifttimeResidualRemoved{nullptr};

                //-----------------------------------------------------------
		// 4. Pictures

		/** Hit-Anzahl in Cluster */
		TGo4Picture* fPHitNumberClusterCond{nullptr};
		/** Drahtspanne in Cluster */
		TGo4Picture* fPWireSpanClusterCond{nullptr};
		/** 2D-Plots dieser */
		TGo4Picture* fPHitNumberVsWireSpanCond{nullptr};

		// Cluster-Koinzidenzen, Gaps, VShape, Ascending, Descending
		TGo4Picture* fPClusterQuality{nullptr};

		TGo4Picture* fPClusterCleaningDiagnostics{nullptr};

		TGo4Picture* fPMinDrifttimeInCluster{nullptr};



		/** Mittelpunkte Cluster */
		TGo4Picture* fPMidPointsClusterCond{nullptr};

		TGo4Picture* fPWireNumberWithMinDrifttimeInCluster{nullptr};


	private:
		/** Methoden */
		std::map<int,std::vector<const TQClamHit*>> GroupHitsByLayer(const std::vector<TQClamHit>& hits) const ;
		void SortHitsByWireNumber(std::vector<const TQClamHit*>& hits) const;
		bool ProofVShape(TQClamCluster& cluster) const;
		void ProofDeOrAscending(TQClamCluster& cluster, const std::vector<double>& drifttimes) const;
		void ComputeClusterMidpoint(TQClamCluster& cluster);
		void ProofMultiWireHits(TQClamCluster& cluster);
		void FillClusterProperties(TQClamCluster& cluster);
		bool ProofClusterShape(TQClamCluster& cluster);
		bool ProofClusterSize(TQClamCluster& cluster);
//		void BuildClusterForLayer(TQClamCluster& cluster);

		void FillHistogramsWithOutCond(const TQClamCluster& cluster);
		void FillHistogramsWithCond(const TQClamCluster& cluster);

		bool CleanCluster(TQClamCluster& cluster);//, size_t iter, size_t maxIter);


	protected:
		Int_t fEvtCounter = 0;

		Int_t fX1Counter = 0;
		Int_t fUCounter = 0;
		Int_t fX2Counter = 0;

		Int_t fX1ClusterWithOutCut = 0;
		Int_t fX1ClusterWithCut = 0;
		Int_t fUClusterWithOutCut = 0;
		Int_t fUClusterWithCut = 0;
		Int_t fX2ClusterWithOutCut = 0;
		Int_t fX2ClusterWithCut = 0;

		Int_t fCountCoincX1UX2 = 0;
		Int_t fCountCoincX1U = 0;
		Int_t fCountCoincX1X2 = 0;
		Int_t fCountCoincUX2 = 0;
		Int_t fCountCoincX1 = 0;
		Int_t fCountCoincU = 0;
		Int_t fCountCoincX2 = 0;
		Int_t fCountCoincNone = 0;

		Int_t fCountX1NoGap = 0;
		Int_t fCountX1Gap = 0;
		Int_t fCountX1Asc = 0;
		Int_t fCountX1Desc = 0;
		Int_t fCountX1VShape = 0;

		Int_t fCountUNoGap = 0;
		Int_t fCountUGap = 0;
		Int_t fCountUAsc = 0;
		Int_t fCountUDesc = 0;
		Int_t fCountUVShape = 0;

		Int_t fCountX2NoGap = 0;
		Int_t fCountX2Gap = 0;
		Int_t fCountX2Asc = 0;
		Int_t fCountX2Desc = 0;
		Int_t fCountX2VShape = 0;

		Int_t fCountAfterX1NoGap = 0;
		Int_t fCountAfterX1Gap = 0;
		Int_t fCountAfterX1Asc = 0;
		Int_t fCountAfterX1Desc = 0;
		Int_t fCountAfterX1VShape = 0;

		Int_t fCountAfterUNoGap = 0;
		Int_t fCountAfterUGap = 0;
		Int_t fCountAfterUAsc = 0;
		Int_t fCountAfterUDesc = 0;
		Int_t fCountAfterUVShape = 0;

		Int_t fCountAfterX2NoGap = 0;
		Int_t fCountAfterX2Gap = 0;
		Int_t fCountAfterX2Asc = 0;
		Int_t fCountAfterX2Desc = 0;
		Int_t fCountAfterX2VShape = 0;


	ClassDefOverride(TQClamClusterProc,1);
};

#endif
