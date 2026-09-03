#include "processor/TQClamClusterProc.h"

/** go4-includes */
#include "TGo4Log.h"
#include "TGo4EventElement.h"
#include "TGo4Picture.h"

/** ROOT-includes */
#include "TClass.h"
#include "TVector3.h"
#include "TMath.h"
#include "TCutG.h"
#include "TH1.h"
#include "TH2.h"

/** evts */
#include "evt/TQClamHitEvent.h"
#include "evt/TQClamClusterEvent.h"

/** parameter */
#include "parameter/TQClamClusterConditionParameter.h"
#include "parameter/TQClamDriftchamGeomParameter.h"
#include "parameter/TQClamHistogramParameter.h"

/** conditions */
#include "TGo4CondArray.h"
#include "TGo4WinCond.h"
#include "TGo4PolyCond.h"
#include "TGo4CounterCond.h"

/** geometry */
#include "geometry/TQClamGeometry.h"
#include "geometry/TQClamDriftcham.h"



ClassImp(TQClamClusterProc)

TQClamClusterProc::TQClamClusterProc()
	: TGo4EventProcessor(),
	fDriftcham(TQClamGeometry::Instance().GetDriftcham())
{}


TQClamClusterProc::TQClamClusterProc(const char* name)
	: TGo4EventProcessor(name),
	fDriftcham(TQClamGeometry::Instance().GetDriftcham())
{
	TGo4Log::Info("[TQClamClusterProc] Create %s", name);

	/** initialisierungen */


        //-----------------------------------------------------------
        // initialisiere interne Variablen/Parameter

	fEvtCounter = 0;
	fX1Counter = 0;
	fUCounter = 0;
	fX2Counter = 0;
	                
	fX1ClusterWithOutCut = 0;
	fX1ClusterWithCut = 0;
	fUClusterWithOutCut = 0;
	fUClusterWithCut = 0;
	fX2ClusterWithOutCut = 0;
	fX2ClusterWithCut = 0;

	fCountCoincX1UX2 = 0;
	fCountCoincX1U = 0;
	fCountCoincX1X2 = 0;
	fCountCoincUX2 = 0;
	fCountCoincX1 = 0;
	fCountCoincU = 0;
	fCountCoincX2 = 0;
	fCountCoincNone = 0;

	fCountX1NoGap = 0;
	fCountX1Gap = 0;
	fCountX1Asc = 0;
	fCountX1Desc = 0;
	fCountX1VShape = 0;

	fCountUNoGap = 0;
	fCountUGap = 0;
	fCountUAsc = 0;
	fCountUDesc = 0;
	fCountUVShape = 0;

	fCountX2NoGap = 0;
	fCountX2Gap = 0;
	fCountX2Asc = 0;
	fCountX2Desc = 0;
	fCountX2VShape = 0;

        fCountAfterX1NoGap = 0;
	fCountAfterX1Gap = 0;
	fCountAfterX1Asc = 0;
	fCountAfterX1Desc = 0;
	fCountAfterX1VShape = 0;

	fCountAfterUNoGap = 0;
	fCountAfterUGap = 0;
	fCountAfterUAsc = 0;
	fCountAfterUDesc = 0;
	fCountAfterUVShape = 0;

	fCountAfterX2NoGap = 0;
	fCountAfterX2Gap = 0;
	fCountAfterX2Asc = 0;
       	fCountAfterX2Desc = 0;
	fCountAfterX2VShape = 0;



        //-----------------------------------------------------------
        // Parameter, Histogramme etc initialisieren
        //-----------------------------------------------------------

        //-----------------------------------------------------------
        // 1. initialisiere Parameter
	fClusterConditionPar = (TQClamClusterConditionParameter*) GetParameter("ClusterConditionParam");
	fDriftchamGeomPar = (TQClamDriftchamGeomParameter*) GetParameter("DriftchamGeomParam");
	fHistoPar = (TQClamHistogramParameter*) GetParameter("HistoParam");



        //-----------------------------------------------------------
	// 2. Conditions

	/** WireSpan-Condition in Cluster: condition_minimum <= |maxDrahtnummer - minDrahtnummer| <= condition_maximum */
	fCondArrWireSpanInCluster = (TGo4CondArray*)GetAnalysisCondition("CondArrWireSpanInCluster");
	if(!fCondArrWireSpanInCluster)
	{
		fCondArrWireSpanInCluster = new TGo4CondArray("CondArrWireSpanInCluster",fClusterConditionPar->MAX_LAYER,"TGo4WinCond");
		fCondArrWireSpanInCluster->SetValues(0,100); /** default --> keine Einschraenkung */
		fCondArrWireSpanInCluster->Enable();
		for(size_t i=0; i<fClusterConditionPar->MAX_LAYER; i++)
		{
			/** fuer jede Ebene einzelne Condition */
			((*fCondArrWireSpanInCluster)[i])->SetValues(fClusterConditionPar->minWireSpanInCluster[i],fClusterConditionPar->maxWireSpanInCluster[i]);
			AddAnalysisCondition((*fCondArrWireSpanInCluster)[i]);
			/** TODO ->SetHistogram */
		}
		//AddAnalysisCondition(fCondArrWireSpanInCluster);
	}
	else
	{
		TGo4Log::Info("[TQClamClusterProc] Lade CondArrWireSpanInCluster aus Autosave-Datei.");
		fCondArrWireSpanInCluster->ResetCounts();
	}




	/** HitNumber-Condition in Cluster: condition_minimum <= Anzahl Hits in Cluster <= condition_maximum */
	fCondArrHitNumberInCluster = (TGo4CondArray*)GetAnalysisCondition("CondArrHitNumberInCluster");
	if(!fCondArrHitNumberInCluster)
	{
		fCondArrHitNumberInCluster = new TGo4CondArray("CondArrHitNumberInCluster", fClusterConditionPar->MAX_LAYER,"TGo4WinCond");
		fCondArrHitNumberInCluster->SetValues(0,100); /** default --> keine Einschraenkung */
		fCondArrHitNumberInCluster->Enable();
		for(size_t i=0; i<fClusterConditionPar->MAX_LAYER; i++)
		{
			((*fCondArrHitNumberInCluster)[i])->SetValues(fClusterConditionPar->minHitNumberInCluster[i],fClusterConditionPar->maxHitNumberInCluster[i]);
			AddAnalysisCondition((*fCondArrHitNumberInCluster)[i]);
			/** TODO ->SetHistogram */
		}
		//AddAnalysisCondition(fCondArrHitNumberInCluster);
	}
	else
	{
		TGo4Log::Info("[TQClamClusterProc] Lade CondArrHitNumberInCluster aus Autosave-Datei.");
		fCondArrHitNumberInCluster->ResetCounts();
	}


	/** 2D: Hit-Anzahl vs Drahtspanne */
	fCondArrHitNumberVsWireSpan = (TGo4CondArray*)GetAnalysisCondition("CondArrHitNumberVsWireSpan");
	if(!fCondArrHitNumberVsWireSpan)
	{
		fCondArrHitNumberVsWireSpan = new TGo4CondArray("CondArrHitNumberVsWireSpan",fClusterConditionPar->MAX_LAYER,"TGo4PolyCond");
		Double_t xvalDefault[5] = { -1e9,1e9,1e9,-1e9,-1e9 };
		Double_t yvalDefault[5] = { -1e9,-1e9,1e9,1e9,-1e9 };
		TCutG* cutDefault = new TCutG("cut",5,xvalDefault,yvalDefault);
		fCondArrHitNumberVsWireSpan->SetValues(cutDefault);
		fCondArrHitNumberVsWireSpan->Enable();
		delete cutDefault;

		for(size_t i=0; i<fClusterConditionPar->MAX_LAYER; i++)
		{
			double xmin = fClusterConditionPar->minWireSpanInCluster[i];
			double xmax = fClusterConditionPar->maxWireSpanInCluster[i];
			double ymin = fClusterConditionPar->minHitNumberInCluster[i];
			double ymax = fClusterConditionPar->maxHitNumberInCluster[i];

			Double_t xval[5] = { xmin,xmax,xmax,xmin,xmin };
			Double_t yval[5] = { ymin,ymin,ymax,ymax,ymin };

			TCutG* cut = new TCutG("cut",5,xval,yval);
			((*fCondArrHitNumberVsWireSpan)[i])->SetValues(cut);
			AddAnalysisCondition((*fCondArrHitNumberVsWireSpan)[i]);
			delete cut;
		}
	}
	else
	{
		TGo4Log::Info("[TQClamClusterProc] Lade CondArrHitNumberVsWireSpan aus Autosave-Datei.");
		fCondArrHitNumberVsWireSpan->ResetCounts();
	}



	/** ClusterVShape?-Condition: Driftzeit vs. Drahtnummer in Cluster wie V-Form? --> Counter-Condition */
	fCondClusterVShape.resize(fClusterConditionPar->MAX_LAYER);
	for(size_t i=0; i<fClusterConditionPar->MAX_LAYER; i++)
	{
		fCondClusterVShape[i] = new TGo4CounterCond(Form("CondClusterVShape_Layer%zu",i));
		AddAnalysisCondition(fCondClusterVShape[i]);
	}



	/** ClusterAscending?-Condition: Driftzeit vs. Drahtnummer in Cluster steigend? TRUE/FALSE --> CounterCondition */
	fCondClusterAscending.resize(fClusterConditionPar->MAX_LAYER);
	for(size_t i=0; i<fClusterConditionPar->MAX_LAYER; i++)
	{
		fCondClusterAscending[i] = new TGo4CounterCond(Form("CondClusterAscending_Layer%zu",i));
		AddAnalysisCondition(fCondClusterAscending[i]);
	}


	/** ClusterDescending?-Condition: Driftzeit vs. Drahtnummer in Cluster fallend? TRUE/FALSE --> CounterCondition */
	fCondClusterDescending.resize(fClusterConditionPar->MAX_LAYER);
	for(size_t i=0; i<fClusterConditionPar->MAX_LAYER; i++)
	{
		fCondClusterDescending[i] = new TGo4CounterCond(Form("CondClusterDescending_Layer%zu",i));
		AddAnalysisCondition(fCondClusterDescending[i]);
	}

	/** ClusterHasGaps?-Condition: Drahtloecher in Cluster? */
	fCondClusterHasGaps.resize(fClusterConditionPar->MAX_LAYER);
	for(size_t i=0; i<fClusterConditionPar->MAX_LAYER; i++)
	{
		fCondClusterHasGaps[i] = new TGo4CounterCond(Form("CondClusterHasGaps_Layer%zu",i));
		AddAnalysisCondition(fCondClusterHasGaps[i]);
	}


	fCondHave_X1_U_X2 = new TGo4CounterCond("CondHaveClusterOf_X1_U_X2");
	AddAnalysisCondition(fCondHave_X1_U_X2);
	fCondHave_X1_U = new TGo4CounterCond("CondHaveClusterOf_X1_U");
	AddAnalysisCondition(fCondHave_X1_U);
	fCondHave_X1_X2 = new TGo4CounterCond("CondHaveClusterOf_X1_X2");
	AddAnalysisCondition(fCondHave_X1_X2);
	fCondHave_U_X2 = new TGo4CounterCond("CondHaveClusterOf_U_X2");
	AddAnalysisCondition(fCondHave_U_X2);
	fCondHave_X1 = new TGo4CounterCond("CondHaveClusterOf_X1");
	AddAnalysisCondition(fCondHave_X1);
	fCondHave_U = new TGo4CounterCond("CondHaveClusterOf_U");
	AddAnalysisCondition(fCondHave_U);
	fCondHave_X2 = new TGo4CounterCond("CondHaveClusterOf_X2");
	AddAnalysisCondition(fCondHave_X2);
	fCondHave_0 = new TGo4CounterCond("CondHaveNoCluster");
	AddAnalysisCondition(fCondHave_0);




        /** zaehle, wie viele Hits HitProc passieren */
        fCondCountClusterProcPassed = (TGo4CounterCond*)GetAnalysisCondition("CondCountClusterProcPassedClusters");
        if(!fCondCountClusterProcPassed)
        {
                fCondCountClusterProcPassed = new TGo4CounterCond("CondCountClusterProcPassedClusters");
                AddAnalysisCondition(fCondCountClusterProcPassed);
        }
        else
        {
                TGo4Log::Info("[TQClamOfflineClusterProc] Lade CondCountClusterProcPassedClusters aus Autosave-Datei");
                fCondCountClusterProcPassed->ResetCounts();
        }

        /** nun fuer X1 */
        fCondCountClusterProcPassedX1 = (TGo4CounterCond*)GetAnalysisCondition("CondCountClusterProcPassedX1Clusters");
        if(!fCondCountClusterProcPassedX1)
        {
                fCondCountClusterProcPassedX1 = new TGo4CounterCond("CondCountClusterProcPassedX1Clusters");
                AddAnalysisCondition(fCondCountClusterProcPassedX1);
        }
        else
        {
                TGo4Log::Info("[TQClamOfflineClusterProc] Lade CondCountClusterProcPassedX1Clusters aus Autosave-Datei");
                fCondCountClusterProcPassedX1->ResetCounts();
        }

        /** nun fuer U */
        fCondCountClusterProcPassedU = (TGo4CounterCond*)GetAnalysisCondition("CondCountClusterProcPassedUClusters");
        if(!fCondCountClusterProcPassedU)
        {
                fCondCountClusterProcPassedU = new TGo4CounterCond("CondCountClusterProcPassedUClusters");
                AddAnalysisCondition(fCondCountClusterProcPassedU);
        }
        else
        {
                TGo4Log::Info("[TQClamOfflineClusterProc] Lade CondCountClusterProcPassedUClusters aus Autosave-Datei");
                fCondCountClusterProcPassedU->ResetCounts();
        }

        /** nun fuer X2 */
        fCondCountClusterProcPassedX2 = (TGo4CounterCond*)GetAnalysisCondition("CondCountClusterProcPassedX2Clusters");
        if(!fCondCountClusterProcPassedX2)
        {
                fCondCountClusterProcPassedX2 = new TGo4CounterCond("CondCountClusterProcPassedX2Clusters");
                AddAnalysisCondition(fCondCountClusterProcPassedX2);
        }
        else
        {
                TGo4Log::Info("[TQClamOfflineClusterProc] Lade CondCountClusterProcPassedX2Clusters aus Autosave-Datei");
	}




        //-----------------------------------------------------------
	// 3. Histogramme
	
	/** -- hole grenzen fuer histogramme */
	/** hit-anzahl in cluster */
	int minHitN_0 = fClusterConditionPar->minHitNumberInCluster[0];
	int maxHitN_0 = fClusterConditionPar->maxHitNumberInCluster[0];
	int nBinsHitN_0 = maxHitN_0 - minHitN_0;

	int minHitN_1 = fClusterConditionPar->minHitNumberInCluster[1];
	int maxHitN_1 = fClusterConditionPar->maxHitNumberInCluster[1];
	int nBinsHitN_1 = maxHitN_1 - minHitN_1;

	int minHitN_2 = fClusterConditionPar->minHitNumberInCluster[2];
	int maxHitN_2 = fClusterConditionPar->maxHitNumberInCluster[2];
	int nBinsHitN_2 = maxHitN_2 - minHitN_2;

	/** wire-span in cluster*/
	int minSpan_0 = fClusterConditionPar->minWireSpanInCluster[0];
	int maxSpan_0 = fClusterConditionPar->maxWireSpanInCluster[0];
	int nBinsSpan_0 = maxSpan_0 - minSpan_0;

	int minSpan_1 = fClusterConditionPar->minWireSpanInCluster[1];
	int maxSpan_1 = fClusterConditionPar->maxWireSpanInCluster[1];
	int nBinsSpan_1 = maxSpan_1 - minSpan_1;

	int minSpan_2 = fClusterConditionPar->minWireSpanInCluster[2];
	int maxSpan_2 = fClusterConditionPar->maxWireSpanInCluster[2];
	int nBinsSpan_2 = maxSpan_2 - minSpan_2;

	/** Mid-Cluster Positionen */
	double minXPos_0 = fDriftchamGeomPar->xOffset[0];
	double maxXPos_0 = fDriftchamGeomPar->xLength[0] + minXPos_0;
	int nBinsXPos_0 = fDriftchamGeomPar->nWires[0];

	double minXPos_1 = fDriftchamGeomPar->xOffset[1];
	double maxXPos_1 = fDriftchamGeomPar->xLength[1] + minXPos_1;
	int nBinsXPos_1 = fDriftchamGeomPar->nWires[1];

	double minXPos_2 = fDriftchamGeomPar->xOffset[2];
	double maxXPos_2 = fDriftchamGeomPar->xLength[2] + minXPos_2;
	int nBinsXPos_2 = fDriftchamGeomPar->nWires[2];

	double minYPos_0 = fDriftchamGeomPar->yOffset[0];
	double maxYPos_0 = fDriftchamGeomPar->yLength[0] + minYPos_0;
	int nBinsYPos_0 = 100;

	double minYPos_1 = fDriftchamGeomPar->yOffset[1];
	double maxYPos_1 = fDriftchamGeomPar->yLength[1] + minYPos_1;
	int nBinsYPos_1 = 100;

	double minYPos_2 = fDriftchamGeomPar->yOffset[2];
	double maxYPos_2 = fDriftchamGeomPar->yLength[2] + minYPos_2;
	int nBinsYPos_2 = 100;

        /* drahtnummer */
        int minWire_0 = 0;
        int maxWire_0 = fDriftchamGeomPar->nWires[0];
        int nBinsWire_0 = maxWire_0 - minWire_0;

        int minWire_1 = 0;
        int maxWire_1 = fDriftchamGeomPar->nWires[1];
        int nBinsWire_1 = maxWire_1 - minWire_1;

        int minWire_2 = 0;
        int maxWire_2 = fDriftchamGeomPar->nWires[2];
        int nBinsWire_2 = maxWire_2 - minWire_2;

        /** Ebenen */
        int minLayer = 0;
        int maxLayer = fDriftchamGeomPar->nPlanes;
        int nBinsLayer = maxLayer - minLayer;



	/** Hit-Anzahl in Cluster */
	fHHitNumberInClusterX1 = MakeTH1('I',"HitNumberInClusterX1","Verteilung Hit-Anzahl in Clustern - X1",50,0,50);
	fHHitNumberInClusterX1->GetXaxis()->SetTitle("Anzahl Hits in Cluster");
	fHHitNumberInClusterX1->GetXaxis()->CenterTitle();
	fHHitNumberInClusterX1->GetYaxis()->SetTitle("Haeufigkeit");
	fHHitNumberInClusterX1->GetYaxis()->CenterTitle();

	fHHitNumberInClusterU = MakeTH1('I',"HitNumberInClusterU","Verteilung Hit-Anzahl in Clustern - U",50,0,50);
	fHHitNumberInClusterU->GetXaxis()->SetTitle("Anzahl Hits in Cluster");
	fHHitNumberInClusterU->GetXaxis()->CenterTitle();
	fHHitNumberInClusterU->GetYaxis()->SetTitle("Haeufigkeit");
	fHHitNumberInClusterU->GetYaxis()->CenterTitle();

	fHHitNumberInClusterX2 = MakeTH1('I',"HitNumberInClusterX2","Verteilung Hit-Anzahl in Clustern - X2",50,0,50);
	fHHitNumberInClusterX2->GetXaxis()->SetTitle("Anzahl Hits in Cluster");
	fHHitNumberInClusterX2->GetXaxis()->CenterTitle();
	fHHitNumberInClusterX2->GetYaxis()->SetTitle("Haeufigkeit");
	fHHitNumberInClusterX2->GetYaxis()->CenterTitle();


	((*fCondArrHitNumberInCluster)[0])->SetHistogram("HitNumberInClusterX1");
	((*fCondArrHitNumberInCluster)[1])->SetHistogram("HitNumberInClusterU");
	((*fCondArrHitNumberInCluster)[2])->SetHistogram("HitNumberInClusterX2");



	/** Hit-Anzahl in Cluster - Condition */
	fHHitNumberInClusterX1Cut = MakeTH1('I',"HitNumberInClusterX1Cut","Verteilung Hit-Anzahl in Clustern - X1, Condition angewendet",
			nBinsHitN_0,minHitN_0,maxHitN_0);
	fHHitNumberInClusterX1Cut->GetXaxis()->SetTitle("Anzahl Hits in Cluster");
	fHHitNumberInClusterX1Cut->GetXaxis()->CenterTitle();
	fHHitNumberInClusterX1Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHHitNumberInClusterX1Cut->GetYaxis()->CenterTitle();

	fHHitNumberInClusterUCut = MakeTH1('I',"HitNumberInClusterUCut","Verteilung Hit-Anzahl in Clustern - U, Condition angewendet",
			nBinsHitN_1,minHitN_1,maxHitN_1);
	fHHitNumberInClusterUCut->GetXaxis()->SetTitle("Anzahl Hits in Cluster");
	fHHitNumberInClusterUCut->GetXaxis()->CenterTitle();
	fHHitNumberInClusterUCut->GetYaxis()->SetTitle("Haeufigkeit");
	fHHitNumberInClusterUCut->GetYaxis()->CenterTitle();

	fHHitNumberInClusterX2Cut = MakeTH1('I',"HitNumberInClusterX2Cut","Verteilung Hit-Anzahl in Clustern - X2, Condition angewendet",
			nBinsHitN_2,minHitN_2,maxHitN_2);
	fHHitNumberInClusterX2Cut->GetXaxis()->SetTitle("Anzahl Hits in Cluster");
	fHHitNumberInClusterX2Cut->GetXaxis()->CenterTitle();
	fHHitNumberInClusterX2Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHHitNumberInClusterX2Cut->GetYaxis()->CenterTitle();



	
	/** Drahtspanne in Cluster */
	fHWireSpanInClusterX1 = MakeTH1('I',"WireSpanInClusterX1","Verteilung Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Clustern - X1",50,0,50);
	fHWireSpanInClusterX1->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Cluster");
	fHWireSpanInClusterX1->GetXaxis()->CenterTitle();
	fHWireSpanInClusterX1->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireSpanInClusterX1->GetYaxis()->CenterTitle();

	fHWireSpanInClusterU = MakeTH1('I',"WireSpanInClusterU","Verteilung Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Clustern - U",50,0,50);
	fHWireSpanInClusterU->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Cluster");
	fHWireSpanInClusterU->GetXaxis()->CenterTitle();
	fHWireSpanInClusterU->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireSpanInClusterU->GetYaxis()->CenterTitle();

	fHWireSpanInClusterX2 = MakeTH1('I',"WireSpanInClusterX2","Verteilung Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Clustern - X2",50,0,50);
	fHWireSpanInClusterX2->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Cluster");
	fHWireSpanInClusterX2->GetXaxis()->CenterTitle();
	fHWireSpanInClusterX2->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireSpanInClusterX2->GetYaxis()->CenterTitle();


	((*fCondArrWireSpanInCluster)[0])->SetHistogram("WireSpanInClusterX1");
	((*fCondArrWireSpanInCluster)[1])->SetHistogram("WireSpanInClusterU");
	((*fCondArrWireSpanInCluster)[2])->SetHistogram("WireSpanInClusterX2");


	/** Drahtspanne in Cluster - Condition */
	fHWireSpanInClusterX1Cut = MakeTH1('I',"WireSpanInClusterX1Cut","Verteilung Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Clustern - X1, Condition angewendet",
			nBinsSpan_0,minSpan_0,maxSpan_0);
	fHWireSpanInClusterX1Cut->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Cluster");
	fHWireSpanInClusterX1Cut->GetXaxis()->CenterTitle();
	fHWireSpanInClusterX1Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireSpanInClusterX1Cut->GetYaxis()->CenterTitle();

	fHWireSpanInClusterUCut = MakeTH1('I',"WireSpanInClusterUCut","Verteilung Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Clustern - U, Condition angewendet",
			nBinsSpan_1,minSpan_1,maxSpan_1);
	fHWireSpanInClusterUCut->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Cluster");
	fHWireSpanInClusterUCut->GetXaxis()->CenterTitle();
	fHWireSpanInClusterUCut->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireSpanInClusterUCut->GetYaxis()->CenterTitle();


	fHWireSpanInClusterX2Cut = MakeTH1('I',"WireSpanInClusterX2Cut","Verteilung Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Clustern - X2, Condition angewendet",
			nBinsSpan_2,minSpan_2,maxSpan_2);
	fHWireSpanInClusterX2Cut->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1) in Cluster");
	fHWireSpanInClusterX2Cut->GetXaxis()->CenterTitle();
	fHWireSpanInClusterX2Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireSpanInClusterX2Cut->GetYaxis()->CenterTitle();



	/** HitNumber vs WireSpan plotten */
	fHHitNumberVsWireSpanX1 = MakeTH2('I',"HitNumberVsWireSpanX1","Cluster: Hit-Anzahl vs. Drahtspanne - X1", 50,0,50, 50,0,50);
	fHHitNumberVsWireSpanX1->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1)");
	fHHitNumberVsWireSpanX1->GetXaxis()->CenterTitle();
	fHHitNumberVsWireSpanX1->GetYaxis()->SetTitle("Hit-Anzahl");
	fHHitNumberVsWireSpanX1->GetYaxis()->CenterTitle();

	fHHitNumberVsWireSpanU = MakeTH2('I',"HitNumberVsWireSpanU","Cluster: Hit-Anzahl vs. Drahtspanne - U", 50,0,50, 50,0,50);
	fHHitNumberVsWireSpanU->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1)");
	fHHitNumberVsWireSpanU->GetXaxis()->CenterTitle();
	fHHitNumberVsWireSpanU->GetYaxis()->SetTitle("Hit-Anzahl");
	fHHitNumberVsWireSpanU->GetYaxis()->CenterTitle();

	fHHitNumberVsWireSpanX2 = MakeTH2('I',"HitNumberVsWireSpanX2","Cluster: Hit-Anzahl vs. Drahtspanne - X2", 50,0,50, 50,0,50);
	fHHitNumberVsWireSpanX2->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1)");
	fHHitNumberVsWireSpanX2->GetXaxis()->CenterTitle();
	fHHitNumberVsWireSpanX2->GetYaxis()->SetTitle("Hit-Anzahl");
	fHHitNumberVsWireSpanX2->GetYaxis()->CenterTitle();



	((*fCondArrHitNumberVsWireSpan)[0])->SetHistogram("HitNumberVsWireSpanX1");
	((*fCondArrHitNumberVsWireSpan)[1])->SetHistogram("HitNumberVsWireSpanU");
	((*fCondArrHitNumberVsWireSpan)[2])->SetHistogram("HitNumberVsWireSpanX2");


	/** HitNumber vs WireSpan plotten - Condition angewendet */
	fHHitNumberVsWireSpanX1Cut = MakeTH2('I',"HitNumberVsWireSpanX1Cut","Cluster: Hit-Anzahl vs. Drahtspanne - X1, Condition angewendet",
			nBinsSpan_0,minSpan_0,maxSpan_0,nBinsHitN_0,minHitN_0,maxHitN_0);
	fHHitNumberVsWireSpanX1Cut->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1)");
	fHHitNumberVsWireSpanX1Cut->GetXaxis()->CenterTitle();
	fHHitNumberVsWireSpanX1Cut->GetYaxis()->SetTitle("Hit-Anzahl");
	fHHitNumberVsWireSpanX1Cut->GetYaxis()->CenterTitle();

	fHHitNumberVsWireSpanUCut = MakeTH2('I',"HitNumberVsWireSpanUCut","Cluster: Hit-Anzahl vs. Drahtspanne - U, Condition angewendet",
			nBinsSpan_1,minSpan_1,maxSpan_1,nBinsHitN_1,minHitN_1,maxHitN_1);
	fHHitNumberVsWireSpanUCut->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1)");
	fHHitNumberVsWireSpanUCut->GetXaxis()->CenterTitle();
	fHHitNumberVsWireSpanUCut->GetYaxis()->SetTitle("Hit-Anzahl");
	fHHitNumberVsWireSpanUCut->GetYaxis()->CenterTitle();

	fHHitNumberVsWireSpanX2Cut = MakeTH2('I',"HitNumberVsWireSpanX2Cut","Cluster: Hit-Anzahl vs. Drahtspanne - X2, Condition angewendet",
			nBinsSpan_2,minSpan_2,maxSpan_2,nBinsHitN_2,minHitN_2,maxHitN_2);
	fHHitNumberVsWireSpanX2Cut->GetXaxis()->SetTitle("Drahtspanne (MaxDrahtnummer-MinDrahtnummer+1)");
	fHHitNumberVsWireSpanX2Cut->GetXaxis()->CenterTitle();
	fHHitNumberVsWireSpanX2Cut->GetYaxis()->SetTitle("Hit-Anzahl");
	fHHitNumberVsWireSpanX2Cut->GetYaxis()->CenterTitle();


	/** Cluster-Mittelpunkte plotten */
	fHXMidClusterX1 = MakeTH1('D',"XMidClusterX1","Driftzeit-gewichtete x-Koor. Cluster-Mittelpunkte - X1",
			nBinsXPos_0,minXPos_0,maxXPos_0);
	fHXMidClusterX1->GetXaxis()->SetTitle("x-Position/cm");
	fHXMidClusterX1->GetXaxis()->CenterTitle();
	fHXMidClusterX1->GetYaxis()->SetTitle("Haeufigkeit");
	fHXMidClusterX1->GetYaxis()->CenterTitle();

	fHXMidClusterU = MakeTH1('D',"XMidClusterU","Driftzeit-gewichtete x-Koor. Cluster-Mittelpunkte - U",
			nBinsXPos_1,minXPos_1,maxXPos_1);
	fHXMidClusterU->GetXaxis()->SetTitle("x-Position/cm");
	fHXMidClusterU->GetXaxis()->CenterTitle();
	fHXMidClusterU->GetYaxis()->SetTitle("Haeufigkeit");
	fHXMidClusterU->GetYaxis()->CenterTitle();

	fHXMidClusterX2 = MakeTH1('D',"XMidClusterX2","Driftzeit-gewichtete x-Koor. Cluster-Mittelpunkte - X2",
			nBinsXPos_2,minXPos_2,maxXPos_2);
	fHXMidClusterX2->GetXaxis()->SetTitle("x-Position in cm");
	fHXMidClusterX2->GetXaxis()->CenterTitle();
	fHXMidClusterX2->GetYaxis()->SetTitle("Haeufigkeit");
	fHXMidClusterX2->GetYaxis()->CenterTitle();


/*
	fHYMidClusterU = MakeTH1('D',"YMidClusterU","Driftzeit-gewichtete y-Koor. Cluster-Mittelpunkt - U",
			nBinsYPos_1,minYPos_1,maxYPos_1);
	fHYMidClusterU->GetXaxis()->SetTitle("y-Position in cm");
	fHYMidClusterU->GetXaxis()->CenterTitle();
	fHYMidClusterU->GetYaxis()->SetTitle("Haeufigkeit");
	fHYMidClusterU->GetYaxis()->CenterTitle();
*/



	/** Cluster-Mittelpunkte plotten - nach Condition */
	fHXMidClusterX1Cut = MakeTH1('D',"XMidClusterX1Cut","Driftzeit-gewichtete x-Koor. Cluster-Mittelpunkte - X1, Condition angewendet",
			nBinsXPos_0,minXPos_0,maxXPos_0);
	fHXMidClusterX1Cut->GetXaxis()->SetTitle("x-Position/cm");
	fHXMidClusterX1Cut->GetXaxis()->CenterTitle();
	fHXMidClusterX1Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHXMidClusterX1Cut->GetYaxis()->CenterTitle();


	fHXMidClusterUCut = MakeTH1('D',"XMidClusterUCut","Driftzeit-gewichtete x-Koor. Cluster-Mittelpunkte - U, Condition angewendet",
			nBinsXPos_1,minXPos_1,maxXPos_1);
	fHXMidClusterUCut->GetXaxis()->SetTitle("x-Position/cm");
	fHXMidClusterUCut->GetXaxis()->CenterTitle();
	fHXMidClusterUCut->GetYaxis()->SetTitle("Haeufigkeit");
	fHXMidClusterUCut->GetYaxis()->CenterTitle();


	fHXMidClusterX2Cut = MakeTH1('D',"XMidClusterX2Cut","Driftzeit-gewichtete x-Koor. Cluster-Mittelpunkte - X2, Condition angewendet",
			nBinsXPos_2,minXPos_2,maxXPos_2);
	fHXMidClusterX2Cut->GetXaxis()->SetTitle("x-Position/cm");
	fHXMidClusterX2Cut->GetXaxis()->CenterTitle();
	fHXMidClusterX2Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHXMidClusterX2Cut->GetYaxis()->CenterTitle();


/*
	fHYMidClusterUCut = MakeTH1('D',"YMidClusterUCut","Driftzeit-gewichtete y-Koor. Cluster-Mittelpunkte - U, Condition angewendet",
			nBinsYPos_1,minYPos_1,maxYPos_1);
	fHYMidClusterUCut->GetXaxis()->SetTitle("y-Position in cm");
	fHYMidClusterUCut->GetXaxis()->CenterTitle();
	fHYMidClusterUCut->GetYaxis()->SetTitle("Haeufigkeit");
	fHYMidClusterUCut->GetYaxis()->CenterTitle();
*/



	/** Cluster Drahtnumer mit min Driftzeit plotten */
	fHWireNumberWithMinDrifttimeInClusterX1 = MakeTH1('I',"WireNumberWithMinDrifttimeInClusterX1","Drahtnummer minimaler Driftzeit in Cluster - X1",
			nBinsWire_0,minWire_0,maxWire_0);
	fHWireNumberWithMinDrifttimeInClusterX1->GetXaxis()->SetTitle("Drahtnummer");
	fHWireNumberWithMinDrifttimeInClusterX1->GetXaxis()->CenterTitle();
	fHWireNumberWithMinDrifttimeInClusterX1->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireNumberWithMinDrifttimeInClusterX1->GetYaxis()->CenterTitle();

	fHWireNumberWithMinDrifttimeInClusterU = MakeTH1('I',"WireNumberWithMinDrifttimeInClusterU","Drahtnummer minimaler Driftzeit in Cluster - U",
			nBinsWire_1,minWire_1,maxWire_1);
	fHWireNumberWithMinDrifttimeInClusterU->GetXaxis()->SetTitle("Drahtnummer");
	fHWireNumberWithMinDrifttimeInClusterU->GetXaxis()->CenterTitle();
	fHWireNumberWithMinDrifttimeInClusterU->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireNumberWithMinDrifttimeInClusterU->GetYaxis()->CenterTitle();


	fHWireNumberWithMinDrifttimeInClusterX2 = MakeTH1('I',"WireNumberWithMinDrifttimeInClusterX2","Drahtnummer minimaler Driftzeit in Cluster - X2",
			nBinsWire_2,minWire_2,maxWire_2);
	fHWireNumberWithMinDrifttimeInClusterX2->GetXaxis()->SetTitle("Drahtnummer");
	fHWireNumberWithMinDrifttimeInClusterX2->GetXaxis()->CenterTitle();
	fHWireNumberWithMinDrifttimeInClusterX2->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireNumberWithMinDrifttimeInClusterX2->GetYaxis()->CenterTitle();


	/** Cluster Drahtnumer mit min Driftzeit plotten - nach Condition */
	fHWireNumberWithMinDrifttimeInClusterX1Cut = MakeTH1('I',"WireNumberWithMinDrifttimeInClusterX1Cut","Drahtnummer minimaler Driftzeit in Cluster - X1, Condition angewendet",
			nBinsWire_0,minWire_0,maxWire_0);
	fHWireNumberWithMinDrifttimeInClusterX1Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHWireNumberWithMinDrifttimeInClusterX1Cut->GetXaxis()->CenterTitle();
	fHWireNumberWithMinDrifttimeInClusterX1Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireNumberWithMinDrifttimeInClusterX1Cut->GetYaxis()->CenterTitle();

	fHWireNumberWithMinDrifttimeInClusterUCut = MakeTH1('I',"WireNumberWithMinDrifttimeInClusterUCut","Drahtnummer minimaler Driftzeit in Cluster - U, Condition angewendet",
			nBinsWire_1,minWire_1,maxWire_1);
	fHWireNumberWithMinDrifttimeInClusterUCut->GetXaxis()->SetTitle("Drahtnummer");
	fHWireNumberWithMinDrifttimeInClusterUCut->GetXaxis()->CenterTitle();
	fHWireNumberWithMinDrifttimeInClusterUCut->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireNumberWithMinDrifttimeInClusterUCut->GetYaxis()->CenterTitle();


	fHWireNumberWithMinDrifttimeInClusterX2Cut = MakeTH1('I',"WireNumberWithMinDrifttimeInClusterX2Cut","Drahtnummer minimaler Driftzeit in Cluster - X2, Condition angewendet",
			nBinsWire_2,minWire_2,maxWire_2);
	fHWireNumberWithMinDrifttimeInClusterX2Cut->GetXaxis()->SetTitle("Drahtnummer");
	fHWireNumberWithMinDrifttimeInClusterX2Cut->GetXaxis()->CenterTitle();
	fHWireNumberWithMinDrifttimeInClusterX2Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHWireNumberWithMinDrifttimeInClusterX2Cut->GetYaxis()->CenterTitle();


	/** Minimale Driftzeiten in Cluster */
	fHMinDrifttimeInClusterX1 = MakeTH1('D',"MinimalDrifttimeInClusterX1","Minimale Driftzeit in X1-Clustern", 1200,0,1200);
	fHMinDrifttimeInClusterX1->GetXaxis()->SetTitle("Minimale Driftzeit/ns");
	fHMinDrifttimeInClusterX1->GetXaxis()->CenterTitle();
	fHMinDrifttimeInClusterX1->GetYaxis()->SetTitle("Haeufigkeit");
	fHMinDrifttimeInClusterX1->GetYaxis()->CenterTitle();

	fHMinDrifttimeInClusterU = MakeTH1('D',"MinimalDrifttimeInClusterU","Minimale Driftzeit in U-Clustern",1200,0,1200);
	fHMinDrifttimeInClusterU->GetXaxis()->SetTitle("Minimale Driftzeit/ns");
	fHMinDrifttimeInClusterU->GetXaxis()->CenterTitle();
	fHMinDrifttimeInClusterU->GetYaxis()->SetTitle("Haeufigkeit");
	fHMinDrifttimeInClusterU->GetYaxis()->CenterTitle();

	fHMinDrifttimeInClusterX2 = MakeTH1('D',"MinimalDrifttimeInClusterX2","Minimale Driftzeit in X2-Clustern",1200,0,1200);
	fHMinDrifttimeInClusterX2->GetXaxis()->SetTitle("Minimale Driftzeit/ns");
	fHMinDrifttimeInClusterX2->GetXaxis()->CenterTitle();
	fHMinDrifttimeInClusterX2->GetYaxis()->SetTitle("Haeufigkeit");
	fHMinDrifttimeInClusterX2->GetYaxis()->CenterTitle();



	fHMinDrifttimeInClusterX1Cut = MakeTH1('D',"MinimalDrifttimeInClusterX1Cut","Minimale Driftzeit in X1-Clustern - Condition angewendet", 1200,0,1200);
	fHMinDrifttimeInClusterX1Cut->GetXaxis()->SetTitle("Minimale Driftzeit/ns");
	fHMinDrifttimeInClusterX1Cut->GetXaxis()->CenterTitle();
	fHMinDrifttimeInClusterX1Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHMinDrifttimeInClusterX1Cut->GetYaxis()->CenterTitle();

	fHMinDrifttimeInClusterUCut = MakeTH1('D',"MinimalDrifttimeInClusterUCut","Minimale Driftzeit in U-Clustern - Condition angewendet",1200,0,1200);
	fHMinDrifttimeInClusterUCut->GetXaxis()->SetTitle("Minimale Driftzeit/ns");
	fHMinDrifttimeInClusterUCut->GetXaxis()->CenterTitle();
	fHMinDrifttimeInClusterUCut->GetYaxis()->SetTitle("Haeufigkeit");
	fHMinDrifttimeInClusterUCut->GetYaxis()->CenterTitle();

	fHMinDrifttimeInClusterX2Cut = MakeTH1('D',"MinimalDrifttimeInClusterX2Cut","Minimale Driftzeit in X2-Clustern - Condition angewendet",1200,0,1200);
	fHMinDrifttimeInClusterX2Cut->GetXaxis()->SetTitle("Minimale Driftzeit/ns");
	fHMinDrifttimeInClusterX2Cut->GetXaxis()->CenterTitle();
	fHMinDrifttimeInClusterX2Cut->GetYaxis()->SetTitle("Haeufigkeit");
	fHMinDrifttimeInClusterX2Cut->GetYaxis()->CenterTitle();



	/** Cluster-Eigenschaften (Gaps,NoGaps,VShape,Ascending,Descending)
	 * x: 0=NoGap, 1=Gap, 2=Ascending, 3=Descending, 4=VShape
	 */

	fHClusterPropertiesBeforeCut = MakeTH2('D',
			"ClusterPropertiesBeforeCut",
			"Cluster Eigenschaften",
			5, 0, 5,   // Kategorien
			3, 0, 3);  // Ebene 0,1,2
	fHClusterPropertiesBeforeCut->GetYaxis()->SetTitle("Ebene (0=X1, 1=U, 2=X2)");
	fHClusterPropertiesBeforeCut->GetYaxis()->CenterTitle();
	fHClusterPropertiesBeforeCut->GetXaxis()->SetBinLabel(1,"NoGap");
	fHClusterPropertiesBeforeCut->GetXaxis()->SetBinLabel(2,"Gap");
	fHClusterPropertiesBeforeCut->GetXaxis()->SetBinLabel(3,"Asc");
	fHClusterPropertiesBeforeCut->GetXaxis()->SetBinLabel(4,"Desc");
	fHClusterPropertiesBeforeCut->GetXaxis()->SetBinLabel(5,"VShape");
	fHClusterPropertiesBeforeCut->GetYaxis()->SetBinLabel(1,"X1");
	fHClusterPropertiesBeforeCut->GetYaxis()->SetBinLabel(2,"U");
	fHClusterPropertiesBeforeCut->GetYaxis()->SetBinLabel(3,"X2");

	fHClusterPropertiesAfterCut = MakeTH2('D',
			"ClusterPropertiesAfterCut",
			"Cluster Eigenschaften - Condition angewendet",
			5, 0, 5,   // Kategorie
			3, 0, 3);  // Ebene 0,1,2
	fHClusterPropertiesAfterCut->GetYaxis()->SetTitle("Ebene (0=X1, 1=U, 2=X2)");
	fHClusterPropertiesAfterCut->GetYaxis()->CenterTitle();
	fHClusterPropertiesAfterCut->GetXaxis()->SetBinLabel(1,"NoGap");
	fHClusterPropertiesAfterCut->GetXaxis()->SetBinLabel(2,"Gap");
	fHClusterPropertiesAfterCut->GetXaxis()->SetBinLabel(3,"Asc");
	fHClusterPropertiesAfterCut->GetXaxis()->SetBinLabel(4,"Desc");
	fHClusterPropertiesAfterCut->GetXaxis()->SetBinLabel(5,"VShape");
	fHClusterPropertiesAfterCut->GetYaxis()->SetBinLabel(1,"X1");
	fHClusterPropertiesAfterCut->GetYaxis()->SetBinLabel(2,"U");
	fHClusterPropertiesAfterCut->GetYaxis()->SetBinLabel(3,"X2");




	/** TH1: Cluster-Koinzidenzen */
	fHClusterCoincidences = MakeTH1('I',
			"ClusterCoincidences",
			"Cluster-Koincidenz Pattern",
			8, 0, 8);
	fHClusterCoincidences->GetXaxis()->SetBinLabel(1,"X1&U&X2");
	fHClusterCoincidences->GetXaxis()->SetBinLabel(2,"U&X2");
	fHClusterCoincidences->GetXaxis()->SetBinLabel(3,"X1&X2");
	fHClusterCoincidences->GetXaxis()->SetBinLabel(4,"X2");
	fHClusterCoincidences->GetXaxis()->SetBinLabel(5,"X1&U");
	fHClusterCoincidences->GetXaxis()->SetBinLabel(6,"U");
	fHClusterCoincidences->GetXaxis()->SetBinLabel(7,"X1");
	fHClusterCoincidences->GetXaxis()->SetBinLabel(8,"None");
	fHClusterCoincidences->GetYaxis()->SetTitle("Haeufigkeit");
	fHClusterCoincidences->GetYaxis()->CenterTitle();

	/** Relative Häufigkeiten - Cluster-Koinzidenzen */
	fHClusterCoincidencesRel = MakeTH1('D',
			"ClusterCoincidencesRel",
			"Cluster-Koinzidenz Pattern (Relativ)",
			8, 0, 8);
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(1,"X1&U&X2");
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(2,"U&X2");
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(3,"X1&X2");
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(4,"X2");
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(5,"X1&U");
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(6,"U");
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(7,"X1");
	fHClusterCoincidencesRel->GetXaxis()->SetBinLabel(8,"None");
	fHClusterCoincidencesRel->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHClusterCoincidencesRel->GetYaxis()->CenterTitle();



	// Effizienz pro Layer
	fHClusterEfficiency = MakeTH1('D',
			"ClusterEfficiency",
			"Cluster Effizienz je Ebene: N_{Cluster} / N_{Events} / %",
			3, 0, 3);
	fHClusterEfficiency->GetXaxis()->SetBinLabel(1,"X1");
	fHClusterEfficiency->GetXaxis()->SetBinLabel(2,"U");
	fHClusterEfficiency->GetXaxis()->SetBinLabel(3,"X2");
	fHClusterEfficiency->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHClusterEfficiency->GetYaxis()->CenterTitle();


	// Effizienz pro Layer (Condition)
	fHClusterConditionEfficiency = MakeTH1('D',
			"ClusterConditionEfficiency",
			"Cluster Effizienz je Ebene (Cuts passiert?)",
			3, 0, 3);
	fHClusterConditionEfficiency->GetXaxis()->SetBinLabel(1,"X1");
	fHClusterConditionEfficiency->GetXaxis()->SetBinLabel(2,"U");
	fHClusterConditionEfficiency->GetXaxis()->SetBinLabel(3,"X2");
	fHClusterConditionEfficiency->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHClusterConditionEfficiency->GetYaxis()->CenterTitle();

	fHClusterConditionEfficiency = MakeTH1('D',
			"ClusterConditionEfficiency",
			"Cluster Effizienz je Ebene (Cuts passiert?) / %",
			3, 0, 3);
	fHClusterConditionEfficiency->GetXaxis()->SetBinLabel(1,"X1");
	fHClusterConditionEfficiency->GetXaxis()->SetBinLabel(2,"U");
	fHClusterConditionEfficiency->GetXaxis()->SetBinLabel(3,"X2");
	fHClusterConditionEfficiency->GetYaxis()->SetTitle("Relative Haeufigkeit / %");
	fHClusterConditionEfficiency->GetYaxis()->CenterTitle();



	/** Kriterien fuer ClusterCleaning */

	// Anzahl entfernter Hits durch Cluster-Cleaning
	fHClusterRemovedHits = MakeTH1('I',"RemovedHitsClusterClean","Anzahl entfernter Hits durch Cluster-Cleaning",
			100,0,100);
	fHClusterRemovedHits->GetXaxis()->SetTitle("Anzahl entfernter Hits");
	fHClusterRemovedHits->GetXaxis()->CenterTitle();
	fHClusterRemovedHits->GetYaxis()->SetTitle("Haeufigkeit");
	fHClusterRemovedHits->GetYaxis()->CenterTitle();


	// Clustergröße vor Cleaning

	fHClusterSizeBeforeClean = MakeTH1(
			'I',
			"ClusterSizeBeforeClean",
			"Clustergroesse vor Cleaning",
			200, 0, 200);
	fHClusterSizeBeforeClean->GetXaxis()->SetTitle("Anzahl Hits");
	fHClusterSizeBeforeClean->GetXaxis()->CenterTitle();
	fHClusterSizeBeforeClean->GetYaxis()->SetTitle("Haeufigkeit");
	fHClusterSizeBeforeClean->GetYaxis()->CenterTitle();


	// Clustergröße nach Cleaning
	fHClusterSizeAfterClean = MakeTH1(
			'I',
			"ClusterSizeAfterClean",
			"Clustergroesse nach Cleaning",
			200, 0, 200);
	fHClusterSizeAfterClean->GetXaxis()->SetTitle("Anzahl Hits");
	fHClusterSizeAfterClean->GetXaxis()->CenterTitle();
	fHClusterSizeAfterClean->GetYaxis()->SetTitle("Haeufigkeit");
	fHClusterSizeAfterClean->GetYaxis()->CenterTitle();

	// Clustergröße vor/nach Cleaning Vgl
	fHClusterSizeBeforeVsAfterClean = MakeTH2(
			'I',
			"ClusterSizeBeforeVsAfterClean",
			"Clustergroesse vor/nach Cleaning - Vgl.",
			200, 0, 200, 200, 0, 200);
	fHClusterSizeBeforeVsAfterClean->GetXaxis()->SetTitle("Hit-Anzahl vorher");
	fHClusterSizeBeforeVsAfterClean->GetXaxis()->CenterTitle();
	fHClusterSizeBeforeVsAfterClean->GetYaxis()->SetTitle("Hit-Anzahl nachher");
	fHClusterSizeBeforeVsAfterClean->GetYaxis()->CenterTitle();



	fHClusterShapeChange = MakeTH1(
			'I',
			"ClusterShapeChange",
			"Effekt des Cleanings auf Cluster-Shape",
			2, 0, 2   // 0 = keine Änderung, 1 = VShape gewonnen, 2 = VShape verloren, 3 = sonstige Änderung
			);
	fHClusterShapeChange->GetXaxis()->SetBinLabel(1, "keine Aenderung");
	fHClusterShapeChange->GetXaxis()->SetBinLabel(2, "Aenderung");
	//fHClusterShapeChange->GetXaxis()->SetBinLabel(3, "VShape verloren");
	//fHClusterShapeChange->GetXaxis()->SetBinLabel(4, "andere Aenderung");
	fHClusterShapeChange->GetYaxis()->SetTitle("Anzahl Cluster");



	// maximaler Driftzeit-Sprung benachbarter Hits
	fHDrifttimeJump = MakeTH1(
			'D',
			"DrifttimeJump",
			"Maximaler Driftzeit-Sprung zwischen benachbarten Draehten im Cluster",
			500, 0.0, 500.0);
	fHDrifttimeJump->GetXaxis()->SetTitle("Max Driftzeitunterschied  / ns");
	fHDrifttimeJump->GetXaxis()->CenterTitle();
	fHDrifttimeJump->GetYaxis()->SetTitle("Haeufigkeit");
	fHDrifttimeJump->GetYaxis()->CenterTitle();

	// ------------------------------------------------------------
        // Residual aller getesteten lokalen Punkte
        //
        // residual =
        // | curr - 0.5*(prev+next) |
        // ------------------------------------------------------------

        fHDrifttimeResidualAll = MakeTH1(
                        'D',
                        "DrifttimeResidualAll",
                        "Lokales Driftzeit-Residuum |t_{curr} - (t_{prev}+t_{next})/2|",
                        500, 0.0, 500.0);
        fHDrifttimeResidualAll->GetXaxis()->SetTitle("Driftzeit-Residuum / ns");
        fHDrifttimeResidualAll->GetXaxis()->CenterTitle();
        fHDrifttimeResidualAll->GetYaxis()->SetTitle("Haeufigkeit");
        fHDrifttimeResidualAll->GetYaxis()->CenterTitle();


        // ------------------------------------------------------------
        // Residual der entfernten Hits
        // ------------------------------------------------------------

        fHDrifttimeResidualRemoved = MakeTH1(
                        'D',
                        "DrifttimeResidualRemoved",
                        "Lokales Driftzeit-Residuum |t_{curr} - (t_{prev}+t_{next})/2| entfernter Hits",
                        500, 0.0, 500.0);
        fHDrifttimeResidualRemoved->GetXaxis()->SetTitle("Driftzeit-Residuum / ns");
        fHDrifttimeResidualRemoved->GetXaxis()->CenterTitle();
        fHDrifttimeResidualRemoved->GetYaxis()->SetTitle("Haeufigkeit");
        fHDrifttimeResidualRemoved->GetYaxis()->CenterTitle();



        //-----------------------------------------------------------
	// 4. Pictures
	
	/** Hit-Anzahl in Cluster */
	fPHitNumberClusterCond = GetPicture("HitNumberInClusterCond");
	if(!fPHitNumberClusterCond)
	{
		fPHitNumberClusterCond = new TGo4Picture("HitNumberInClusterCond","Verteilung Hit-Anzahl in Clustern - Anwendung Condition");
		fPHitNumberClusterCond->SetDivision(2,3); /** TODO: evtl ganze kammer -> 2,4? */

		fPHitNumberClusterCond->Pic(0,0)->AddObject(fHHitNumberInClusterX1);
		fPHitNumberClusterCond->Pic(0,0)->SetFillAtt(3,3031);
		fPHitNumberClusterCond->Pic(0,0)->AddCondition((*fCondArrHitNumberInCluster)[0]);
		fPHitNumberClusterCond->Pic(0,1)->AddObject(fHHitNumberInClusterU);
		fPHitNumberClusterCond->Pic(0,1)->SetFillAtt(6,3031);
		fPHitNumberClusterCond->Pic(0,1)->AddCondition((*fCondArrHitNumberInCluster)[1]);
		fPHitNumberClusterCond->Pic(0,2)->AddObject(fHHitNumberInClusterX2);
		fPHitNumberClusterCond->Pic(0,2)->SetFillAtt(7,3031);
		fPHitNumberClusterCond->Pic(0,2)->AddCondition((*fCondArrHitNumberInCluster)[2]);

		fPHitNumberClusterCond->Pic(1,0)->AddObject(fHHitNumberInClusterX1Cut);
		fPHitNumberClusterCond->Pic(1,0)->SetFillAtt(3,3031);
		fPHitNumberClusterCond->Pic(1,1)->AddObject(fHHitNumberInClusterUCut);
		fPHitNumberClusterCond->Pic(1,1)->SetFillAtt(6,3031);
		fPHitNumberClusterCond->Pic(1,2)->AddObject(fHHitNumberInClusterX2Cut);
		fPHitNumberClusterCond->Pic(1,2)->SetFillAtt(7,3031);

		AddPicture(fPHitNumberClusterCond);
	}

	/** Drahtspanne in Cluster */
	fPWireSpanClusterCond = GetPicture("WireSpanInClusterCond");
	if(!fPWireSpanClusterCond)
	{
		fPWireSpanClusterCond = new TGo4Picture("WireSpanInClusterCond","Verteilung Drahtspanne (MaxDrahtnummer-MinDrahtnummer) in Clustern - Anwendung Condition");
		fPWireSpanClusterCond->SetDivision(2,3); /** TODO: evtl ganze kammer -> 2,4? */

		fPWireSpanClusterCond->Pic(0,0)->AddObject(fHWireSpanInClusterX1);
		fPWireSpanClusterCond->Pic(0,0)->SetFillAtt(3,3031);
		fPWireSpanClusterCond->Pic(0,0)->AddCondition((*fCondArrWireSpanInCluster)[0]);
		fPWireSpanClusterCond->Pic(0,1)->AddObject(fHWireSpanInClusterU);
		fPWireSpanClusterCond->Pic(0,1)->SetFillAtt(6,3031);
		fPWireSpanClusterCond->Pic(0,1)->AddCondition((*fCondArrWireSpanInCluster)[1]);
		fPWireSpanClusterCond->Pic(0,2)->AddObject(fHWireSpanInClusterX2);
		fPWireSpanClusterCond->Pic(0,2)->SetFillAtt(7,3031);
		fPWireSpanClusterCond->Pic(0,2)->AddCondition((*fCondArrWireSpanInCluster)[2]);

		fPWireSpanClusterCond->Pic(1,0)->AddObject(fHWireSpanInClusterX1Cut);
		fPWireSpanClusterCond->Pic(1,0)->SetFillAtt(3,3031);
		fPWireSpanClusterCond->Pic(1,1)->AddObject(fHWireSpanInClusterUCut);
		fPWireSpanClusterCond->Pic(1,1)->SetFillAtt(6,3031);
		fPWireSpanClusterCond->Pic(1,2)->AddObject(fHWireSpanInClusterX2Cut);
		fPWireSpanClusterCond->Pic(1,2)->SetFillAtt(7,3031);

		AddPicture(fPWireSpanClusterCond);
	}


	/** 2D: HitNumber vs. WireSpan */
	fPHitNumberVsWireSpanCond = GetPicture("HitNumberVsWireSpanCond");
	if(!fPHitNumberVsWireSpanCond)
	{
		fPHitNumberVsWireSpanCond = new TGo4Picture("HitNumberVsWireSpanCond","Hit-Anzahl versus Drahtspanne (MaxDrahtnummer-MinDrahtnummer) in Clustern - Anwendung Condition");
		fPHitNumberVsWireSpanCond->SetDivision(2,3);

		fPHitNumberVsWireSpanCond->Pic(0,0)->AddObject(fHHitNumberVsWireSpanX1);
		fPHitNumberVsWireSpanCond->Pic(0,0)->SetFillAtt(3,3031);
		fPHitNumberVsWireSpanCond->Pic(0,0)->AddCondition((*fCondArrHitNumberVsWireSpan)[0]);
		fPHitNumberVsWireSpanCond->Pic(0,1)->AddObject(fHHitNumberVsWireSpanU);
		fPHitNumberVsWireSpanCond->Pic(0,1)->SetFillAtt(6,3031);
		fPHitNumberVsWireSpanCond->Pic(0,1)->AddCondition((*fCondArrHitNumberVsWireSpan)[1]);
		fPHitNumberVsWireSpanCond->Pic(0,2)->AddObject(fHHitNumberVsWireSpanX2);
		fPHitNumberVsWireSpanCond->Pic(0,2)->SetFillAtt(7,3031);
		fPHitNumberVsWireSpanCond->Pic(0,2)->AddCondition((*fCondArrHitNumberVsWireSpan)[2]);

		fPHitNumberVsWireSpanCond->Pic(1,0)->AddObject(fHHitNumberVsWireSpanX1Cut);
		fPHitNumberVsWireSpanCond->Pic(1,0)->SetFillAtt(3,3031);
		fPHitNumberVsWireSpanCond->Pic(1,1)->AddObject(fHHitNumberVsWireSpanUCut);
		fPHitNumberVsWireSpanCond->Pic(1,1)->SetFillAtt(6,3031);
		fPHitNumberVsWireSpanCond->Pic(1,2)->AddObject(fHHitNumberVsWireSpanX2Cut);
		fPHitNumberVsWireSpanCond->Pic(1,2)->SetFillAtt(7,3031);

		AddPicture(fPHitNumberVsWireSpanCond);
	}



	fPClusterQuality = GetPicture("ClusterQuality");
	if(!fPClusterQuality)
	{
		TGo4Picture* fPClusterQuality = new TGo4Picture("ClusterQuality","Cluster Qualitaet");
		fPClusterQuality->SetDivision(2,3);


		fPClusterQuality->Pic(0,0)->AddObject(fHClusterPropertiesBeforeCut);
		fPClusterQuality->Pic(1,0)->AddObject(fHClusterPropertiesAfterCut);
		fPClusterQuality->Pic(0,1)->AddObject(fHClusterCoincidences);
		fPClusterQuality->Pic(1,1)->AddObject(fHClusterCoincidencesRel);
		fPClusterQuality->Pic(0,2)->AddObject(fHClusterEfficiency);
		fPClusterQuality->Pic(1,2)->AddObject(fHClusterConditionEfficiency);

		AddPicture(fPClusterQuality);
	}


	fPClusterCleaningDiagnostics = GetPicture("ClusterCleaningDiagnosics");
	if(!fPClusterCleaningDiagnostics)
	{
		TGo4Picture* fPClusterCleaningDiagnostics = new TGo4Picture("ClusterCleaningDiagnosics","Diagnostik des Saeuberns der Cluster von unpassenden Hits");
		fPClusterCleaningDiagnostics->SetDivision(2,4);

		fPClusterCleaningDiagnostics->Pic(0,0)->AddObject(fHClusterRemovedHits);
		fPClusterCleaningDiagnostics->Pic(1,0)->AddObject(fHClusterSizeBeforeVsAfterClean);
		fPClusterCleaningDiagnostics->Pic(0,1)->AddObject(fHClusterSizeBeforeClean);
		fPClusterCleaningDiagnostics->Pic(1,1)->AddObject(fHClusterSizeAfterClean);
		fPClusterCleaningDiagnostics->Pic(0,2)->AddObject(fHClusterShapeChange);
		fPClusterCleaningDiagnostics->Pic(1,2)->AddObject(fHDrifttimeJump);
		fPClusterCleaningDiagnostics->Pic(0,3)->AddObject(fHDrifttimeResidualAll);
		fPClusterCleaningDiagnostics->Pic(1,3)->AddObject(fHDrifttimeResidualRemoved);


		AddPicture(fPClusterCleaningDiagnostics);
	}


	/** minimale Driftzeit in Clustern */
	fPMinDrifttimeInCluster = GetPicture("MinDrifttimeInClusterCond");
	if(!fPMinDrifttimeInCluster)
	{
		fPMinDrifttimeInCluster = new TGo4Picture("MinDrifttimeInClusterCond","Minimale Driftzeit in Clustern - Anwendung Condition");
		fPMinDrifttimeInCluster->SetDivision(2,3);

		fPMinDrifttimeInCluster->Pic(0,0)->AddObject(fHMinDrifttimeInClusterX1);
		fPMinDrifttimeInCluster->Pic(0,0)->SetFillAtt(3,3031);
		fPMinDrifttimeInCluster->Pic(0,1)->AddObject(fHMinDrifttimeInClusterU);
		fPMinDrifttimeInCluster->Pic(0,1)->SetFillAtt(6,3031);
		fPMinDrifttimeInCluster->Pic(0,2)->AddObject(fHMinDrifttimeInClusterX2);
		fPMinDrifttimeInCluster->Pic(0,2)->SetFillAtt(7,3031);

		fPMinDrifttimeInCluster->Pic(1,0)->AddObject(fHMinDrifttimeInClusterX1Cut);
		fPMinDrifttimeInCluster->Pic(1,0)->SetFillAtt(3,3031);
		fPMinDrifttimeInCluster->Pic(1,1)->AddObject(fHMinDrifttimeInClusterUCut);
		fPMinDrifttimeInCluster->Pic(1,1)->SetFillAtt(6,3031);
		fPMinDrifttimeInCluster->Pic(1,2)->AddObject(fHMinDrifttimeInClusterX2Cut);
		fPMinDrifttimeInCluster->Pic(1,2)->SetFillAtt(7,3031);

		AddPicture(fPMinDrifttimeInCluster);
	}




	/** Cluster Mittelpunkte */
	fPMidPointsClusterCond = GetPicture("MidPointsClusterCond");
	if(!fPMidPointsClusterCond)
	{
		fPMidPointsClusterCond = new TGo4Picture("MidPointsClusterCond","Driftzeit-gewichtete x-Mittelpunkte Cluster");
		fPMidPointsClusterCond->SetDivision(2,3);

		fPMidPointsClusterCond->Pic(0,0)->AddObject(fHXMidClusterX1);
		fPMidPointsClusterCond->Pic(0,0)->SetFillAtt(3,3031);
		fPMidPointsClusterCond->Pic(0,1)->AddObject(fHXMidClusterU);
		fPMidPointsClusterCond->Pic(0,1)->SetFillAtt(6,3031);
		fPMidPointsClusterCond->Pic(0,2)->AddObject(fHXMidClusterX2);
		fPMidPointsClusterCond->Pic(0,2)->SetFillAtt(7,3031);
		/*
		fPMidPointsClusterCond->Pic(0,3)->AddObject(fHYMidClusterU);
		fPMidPointsClusterCond->Pic(0,3)->SetFillAtt(6,3031);
		*/

		fPMidPointsClusterCond->Pic(1,0)->AddObject(fHXMidClusterX1Cut);
		fPMidPointsClusterCond->Pic(1,0)->SetFillAtt(3,3031);
		fPMidPointsClusterCond->Pic(1,1)->AddObject(fHXMidClusterUCut);
		fPMidPointsClusterCond->Pic(1,1)->SetFillAtt(6,3031);
		fPMidPointsClusterCond->Pic(1,2)->AddObject(fHXMidClusterX2Cut);
		fPMidPointsClusterCond->Pic(1,2)->SetFillAtt(7,3031);
		/*
		fPMidPointsClusterCond->Pic(1,3)->AddObject(fHYMidClusterUCut);
		fPMidPointsClusterCond->Pic(1,3)->SetFillAtt(6,3031);
		*/

		AddPicture(fPMidPointsClusterCond);
	}

	/** Cluster Drahtnummer mit min Driftzeit */
	fPWireNumberWithMinDrifttimeInCluster = GetPicture("WireNumberWithMinDrifttimeInClusterCond");
	if(!fPWireNumberWithMinDrifttimeInCluster)
	{
		fPWireNumberWithMinDrifttimeInCluster = new TGo4Picture("WireNumberWithMinDrifttimeInClusterCond","Drahtnummer mit minimaler Driftzeit in Cluster");
		fPWireNumberWithMinDrifttimeInCluster->SetDivision(2,4);

		fPWireNumberWithMinDrifttimeInCluster->Pic(0,0)->AddObject(fHWireNumberWithMinDrifttimeInClusterX1);
		fPWireNumberWithMinDrifttimeInCluster->Pic(0,0)->SetFillAtt(3,3031);
		fPWireNumberWithMinDrifttimeInCluster->Pic(0,1)->AddObject(fHWireNumberWithMinDrifttimeInClusterU);
		fPWireNumberWithMinDrifttimeInCluster->Pic(0,1)->SetFillAtt(6,3031);
		fPWireNumberWithMinDrifttimeInCluster->Pic(0,2)->AddObject(fHWireNumberWithMinDrifttimeInClusterX2);
		fPWireNumberWithMinDrifttimeInCluster->Pic(0,2)->SetFillAtt(7,3031);


		fPWireNumberWithMinDrifttimeInCluster->Pic(1,0)->AddObject(fHWireNumberWithMinDrifttimeInClusterX1Cut);
		fPWireNumberWithMinDrifttimeInCluster->Pic(1,0)->SetFillAtt(3,3031);
		fPWireNumberWithMinDrifttimeInCluster->Pic(1,1)->AddObject(fHWireNumberWithMinDrifttimeInClusterUCut);
		fPWireNumberWithMinDrifttimeInCluster->Pic(1,1)->SetFillAtt(6,3031);
		fPWireNumberWithMinDrifttimeInCluster->Pic(1,2)->AddObject(fHWireNumberWithMinDrifttimeInClusterX2Cut);
		fPWireNumberWithMinDrifttimeInCluster->Pic(1,2)->SetFillAtt(7,3031);


		AddPicture(fPWireNumberWithMinDrifttimeInCluster);
	}

}





//***********************************************************
TQClamClusterProc::~TQClamClusterProc()
{

	/** Conditions printen */
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("\n");
	printf("\n");
	printf("\n");
        printf("===================================================================================\n");
        printf("===================================================================================\n");
	TGo4Log::Info("[TQClamClusterProc] GEBE ALLE CONDITION-STATISTIKEN DES CLUSTER-PROZESSORS AUS:");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Hit-Anzahl in X1-Cluster -----------------------");
	printf("\n");
	((*fCondArrHitNumberInCluster)[0])->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Hit-Anzahl in U-Cluster -----------------------");
	printf("\n");
	((*fCondArrHitNumberInCluster)[1])->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Hit-Anzahl in X2-Cluster -----------------------");
	printf("\n");
	((*fCondArrHitNumberInCluster)[2])->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");


        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Drahtspanne in X1-Clustern -----------------------");
	printf("\n");
	((*fCondArrWireSpanInCluster)[0])->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Drahtspanne in U-Clustern -----------------------");
	printf("\n");
	((*fCondArrWireSpanInCluster)[1])->PrintCondition(true);
	printf("\n");
	printf("...\n");
	//TGo4Log::Info("...");
        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Drahtspanne in X2-Clustern -----------------------");
	printf("\n");
	((*fCondArrWireSpanInCluster)[2])->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");
	printf("...\n");



	/** Cluster-Form Conditions */
        printf("===================================================================================\n");
        printf("===================================================================================\n");
	TGo4Log::Info("[TQClamClusterProc] GEBE STATISTIKEN ZUR CLUSTER-FORM AUS:");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");


        TGo4Log::Info("[TQClamClusterProc]          --- Condition: V-Form X1-Cluster -----------------------");
	printf("\n");
	fCondClusterVShape[0]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: V-Form U-Cluster -----------------------");
	printf("\n");
	fCondClusterVShape[1]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: V-Form X2-Cluster -----------------------");
	printf("\n");
	fCondClusterVShape[2]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");


        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Driftzeit versus Drahtnummer wachsend X1-Cluster -----------------------");
	printf("\n");
	fCondClusterAscending[0]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Driftzeit versus Drahtnummer wachsend U-Cluster -----------------------");
	printf("\n");
	fCondClusterAscending[1]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Driftzeit versus Drahtnummer wachsend X2-Cluster -----------------------");
	printf("\n");
	fCondClusterAscending[2]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");


        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Driftzeit versus Drahtnummer fallend X1-Cluster -----------------------");
	printf("\n");
	fCondClusterDescending[0]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Driftzeit versus Drahtnummer fallend U-Cluster -----------------------");
	printf("\n");
	fCondClusterDescending[1]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: Driftzeit versus Drahtnummer fallend X2-Cluster -----------------------");
	printf("\n");
	fCondClusterDescending[2]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");



        TGo4Log::Info("[TQClamClusterProc]          --- Condition: X1-Cluster hat Drahtluecken -----------------------");
	printf("\n");
	fCondClusterHasGaps[0]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: U-Cluster hat Drahtluecken -----------------------");
	printf("\n");
	fCondClusterHasGaps[1]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: X2-Cluster hat Drahtluecken -----------------------");
	printf("\n");
	fCondClusterHasGaps[2]->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");
	printf("...\n");





	/** Cluster-Belegungen der Ebenen je Event */
        printf("===================================================================================\n");
        printf("===================================================================================\n");
	TGo4Log::Info("[TQClamClusterProc] GEBE STATISTIKEN ZU CLUSTER-BELEGUNG DER EBENEN JE EVENT AUS:");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");


        TGo4Log::Info("[TQClamClusterProc]          --- Condition: X1- && U- && X2-Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_X1_U_X2->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: X1- && U-Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_X1_U->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: X1- && X2-Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_X1_X2->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: U- && X2-Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_U_X2->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: X1-Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_X1->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: U-Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_U->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: X2-Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_X2->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Condition: 0 Cluster im Event  -----------------------");
	printf("\n");
	fCondHave_0->PrintCondition(true);
	printf("\n");
	//TGo4Log::Info("...");
	//TGo4Log::Info("...");
        //TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");
	printf("...\n");


        printf("===================================================================================\n");
        printf("===================================================================================\n");
        TGo4Log::Info("[TQClamClusterProc] GEBE CONDITION-STATISTIK ALLER CLUSTERPROZESSOR PASSIERTEN CLUSTERS AUS:");
        //TGo4Log::Info("...");
        //TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");

        TGo4Log::Info("[TQClamClusterProc]          --- Gesamtanzahl passierte Clusters -----------------------");
	printf("\n");
        fCondCountClusterProcPassed->PrintCondition(true);
	printf("\n");
        //TGo4Log::Info("...");
	printf("...\n");
        TGo4Log::Info("[TQClamClusterProc]          --- Anzahl passierter Clusters aus X1 -----------------------");
	printf("\n");
        fCondCountClusterProcPassedX1->PrintCondition(true);
	printf("\n");
        //TGo4Log::Info("...");
	printf("...\n");
        TGo4Log::Info("[TQClamClusterProc]          --- Anzahl passierter Clusters aus U -----------------------");
	printf("\n");
        fCondCountClusterProcPassedU->PrintCondition(true);
	printf("\n");
        //TGo4Log::Info("...");
	printf("...\n");
        TGo4Log::Info("[TQClamClusterProc]          --- Anzahl passierter Clusters aus X2 -----------------------");
	printf("\n");
        fCondCountClusterProcPassedX2->PrintCondition(true);
	printf("\n");
        //TGo4Log::Info("...");
        //TGo4Log::Info("...");
	printf("...\n");
	printf("...\n");


	TGo4Log::Info("[TQClamClusterProc] Delete %s", GetName());
}





//***********************************************************
// pruefe klasse von output-eventobjekt
Bool_t TQClamClusterProc::CheckEventClass(TClass* cl)
{
	return cl->InheritsFrom(TQClamClusterEvent::Class());
}





//***********************************************************
// baue output-eventobjekt
Bool_t TQClamClusterProc::BuildEvent(TGo4EventElement* dest)
{
	/** output event */
	TQClamClusterEvent* evt = dynamic_cast<TQClamClusterEvent*>(dest);

	/** input event */
	TQClamHitEvent* i_evt = dynamic_cast<TQClamHitEvent*>(GetInputEvent());

	if(!i_evt || !evt)
	{
		TGo4Log::Warn("[TQClamClusterProc] Input- oder Output-Event existiert nicht!");
		return kFALSE;
	}



	/// HEADER
	evt->header = i_evt->header;

	evt->scaler = i_evt->scaler;
	evt->trigDet = i_evt->trigDet;





	evt->fEvtCounter = i_evt->fEvtCounter;
	fEvtCounter = i_evt->fEvtCounter;

	evt->layerHasCluster.reset();

	/** gruppiere hits nach layerID und sortiere Hits gleicher layerID nach Drahtnummer */
	auto hitsGroupedByLayer = GroupHitsByLayer(i_evt->hits);

	/** pruefe alle Cluster zusammen */
	for(const auto& [layer,hits] : hitsGroupedByLayer)
	{
		TQClamCluster cluster;
		cluster.layerID = layer;
		cluster.hits = hits;

	
		/**
		 * Berechne alle Clustereigenschaften:
		 * - Draht-Normalkoordinaten
		 * - 1. Durchstosspunkt fuer Tracking-Iteration
		 * - Drahtspanne berechnen
		 * ...
		 */
		FillClusterProperties(cluster);

		if(fClusterConditionPar->fCleanCluster)
		{
			cluster.originalHits = cluster.hits;
			bool changed = CleanCluster(cluster);
			//if(changed) { FillClusterProperties(cluster); 
			FillClusterProperties(cluster);

		}


		bool shapeOk = ProofClusterShape(cluster);
		bool sizeOk = ProofClusterSize(cluster);


		if(fHistoPar->fBFillHistos) { FillHistogramsWithOutCond(cluster); }

		/** Conditions TODO */

		if(shapeOk && sizeOk)
		{
			evt->layerHasCluster.set(cluster.layerID);

			evt->clusters.push_back(cluster);


			if(fHistoPar->fBFillHistos) { FillHistogramsWithCond(cluster); }

			/** zaehle Cluster, die passieren */
			fCondCountClusterProcPassed->Test(true);
			switch(layer)
			{
				case 0:
					fCondCountClusterProcPassedX1->Test(true);
					fX1ClusterWithCut += 1;
					break;
				case 1:
					fCondCountClusterProcPassedU->Test(true);
					fUClusterWithCut += 1;
					break;
				case 2:
					fCondCountClusterProcPassedX2->Test(true);
					fX2ClusterWithCut += 1;
					break;
				default:
					break;
			}
		}
		else
		{
			fCondCountClusterProcPassed->Test(false);
			
			switch(layer)
			{
				case 0:
					fCondCountClusterProcPassedX1->Test(false);
					fX1ClusterWithOutCut += 1;
					break;
				case 1:
					fCondCountClusterProcPassedU->Test(false);
					fUClusterWithOutCut += 1;
					break;
				case 2:
					fCondCountClusterProcPassedX2->Test(false);
					fX2ClusterWithOutCut += 1;
					break;
				default:
					break;
			}
		}

		continue;
	}

	/** pruefe, die Clusterverteilung auf die Ebenen:
	 * X2 | U | X1 | pattern
	 *
	 * 1  | 1 | 1  | 7
	 * 1  | 1 | 0  | 6
	 * 1  | 0 | 1  | 5
	 * 1  | 0 | 0  | 4
	 * 0  | 1 | 1  | 3
	 * 0  | 1 | 0  | 2
	 * 0  | 0 | 1  | 1
	 * 0  | 0 | 0  | 0
	 * */
	unsigned pattern = evt->layerHasCluster.to_ulong();


	switch(pattern)
	{
		case 7: // 111 = X1 + U + X2
			fCondHave_X1_U_X2->Test(true);
			fCountCoincX1UX2++;
			fHClusterCoincidencesRel->SetBinContent(1,100.*double(fCountCoincX1UX2)/double(fEvtCounter));

			fCondHave_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_U->Test(false);
			fCondHave_X1->Test(false);
			fCondHave_0->Test(false);

			fX1Counter++;
			fUCounter++;
			fX2Counter++;
			break;

		case 6: // 110 = U + X2
			fCondHave_U_X2->Test(true);
			fCountCoincUX2++;
			fHClusterCoincidencesRel->SetBinContent(2,100.*double(fCountCoincUX2)/double(fEvtCounter));

			fCondHave_X1_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_U->Test(false);
			fCondHave_X1->Test(false);
			fCondHave_0->Test(false);

			fUCounter++;
			fX2Counter++;
			break;

		case 5: // 101 = X1 + X2
			fCondHave_X1_X2->Test(true);
			fCountCoincX1X2++;
			fHClusterCoincidencesRel->SetBinContent(3,100.*double(fCountCoincX1X2)/double(fEvtCounter));

			fCondHave_X1_U_X2->Test(false);
			fCondHave_U_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_U->Test(false);
			fCondHave_X1->Test(false);
			fCondHave_0->Test(false);

			fX1Counter++;
			fX2Counter++;
			break;

		case 4: // 100 = X2
			fCondHave_X2->Test(true);
			fCountCoincX2++;
			fHClusterCoincidencesRel->SetBinContent(4,100.*double(fCountCoincX2)/double(fEvtCounter));

			fCondHave_X1_U_X2->Test(false);
			fCondHave_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_U->Test(false);
			fCondHave_X1->Test(false);
			fCondHave_0->Test(false);

			fX2Counter++;
			break;

		case 3: // 011 = X1 + U
			fCondHave_X1_U->Test(true);
			fCountCoincX1U++;
			fHClusterCoincidencesRel->SetBinContent(5,100.*double(fCountCoincX1U)/double(fEvtCounter));

			fCondHave_X1_U_X2->Test(false);
			fCondHave_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_U->Test(false);
			fCondHave_X1->Test(false);
			fCondHave_0->Test(false);

			fX1Counter++;
			fUCounter++;
			break;

		case 2: // 010 = U
			fCondHave_U->Test(true);
			fCountCoincU++;
			fHClusterCoincidencesRel->SetBinContent(6,100.*double(fCountCoincU)/double(fEvtCounter));

			fCondHave_X1_U_X2->Test(false);
			fCondHave_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_X1->Test(false);
			fCondHave_0->Test(false);

			fUCounter++;
			break;

		case 1: // 001 = X1
			fCondHave_X1->Test(true);
			fCountCoincX1++;
			fHClusterCoincidencesRel->SetBinContent(7,100.*double(fCountCoincX1)/double(fEvtCounter));

			fCondHave_X1_U_X2->Test(false);
			fCondHave_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_U->Test(false);
			fCondHave_0->Test(false);

			fX1Counter++;
			break;

		case 0: // 000 = none
			fCondHave_0->Test(true);
			fCountCoincNone++;

	
			if(fHistoPar->fBFillHistos)
			{
				fHClusterCoincidencesRel->SetBinContent(8,100.*double(fCountCoincNone)/double(fEvtCounter));
				fHClusterCoincidences->Fill(7);
			}

			fCondHave_X1_U_X2->Test(false);
			fCondHave_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_U->Test(false);
			fCondHave_X1->Test(false);

			/** ohne cluster kein track moeglich */
			evt->SetValid(kFALSE);
			return kFALSE;

		default:
			fCondHave_0->Test(true);
			fCountCoincNone++;

			if(fHistoPar->fBFillHistos)
			{
				fHClusterCoincidencesRel->SetBinContent(8,100.*double(fCountCoincNone)/double(fEvtCounter));
				fHClusterCoincidences->Fill(7);
			}

			fCondHave_X1_U_X2->Test(false);
			fCondHave_U_X2->Test(false);
			fCondHave_X1_X2->Test(false);
			fCondHave_X1_U->Test(false);
			fCondHave_X2->Test(false);
			fCondHave_U->Test(false);
			fCondHave_X1->Test(false);

			/** ohne cluster kein track moeglich */
			evt->SetValid(kFALSE);
			return kFALSE;
	}


	/** Cluster-Koinzidenz fuellen */

	if(fHistoPar->fBFillHistos) { fHClusterCoincidences->Fill(7-pattern); }


	/** effizienzen */
	double effX1 = double(fX1ClusterWithCut)/(double(fX1ClusterWithCut + fX1ClusterWithOutCut));

	double effU = double(fUClusterWithCut) / (double(fUClusterWithCut + fUClusterWithOutCut));

	double effX2 = double(fX2ClusterWithCut) / (double(fX2ClusterWithCut + fX2ClusterWithOutCut));

	if(fHistoPar->fBFillHistos)
	{
		fHClusterConditionEfficiency->SetBinContent(1, 100.*effX1);
		fHClusterConditionEfficiency->SetBinContent(2, 100.*effU);
		fHClusterConditionEfficiency->SetBinContent(3, 100.*effX2);

		fHClusterEfficiency->SetBinContent(1, 100.*double(fX1Counter)/double(fEvtCounter));
		fHClusterEfficiency->SetBinContent(2, 100.*double(fUCounter)/double(fEvtCounter));
		fHClusterEfficiency->SetBinContent(3, 100.*double(fX2Counter)/double(fEvtCounter));
	}




	evt->SetValid(kTRUE);
	return kTRUE;
}





//***********************************************************
// pruefe form des Clusters (VShape/Ascending/Descending),
// setze entsprechende Conditions
// und vergleiche mit Forderungen an Cluster
// --> nutze Cluster?
bool TQClamClusterProc::ProofClusterShape(TQClamCluster& cluster)
{
	/** setze V-Shape, Ascending und Descending sowie entsprechende Conditions */
	fCondClusterVShape[cluster.layerID]->Test(ProofVShape(cluster));
	fCondClusterAscending[cluster.layerID]->Test(cluster.ascending);
	fCondClusterDescending[cluster.layerID]->Test(cluster.descending);
	fCondClusterHasGaps[cluster.layerID]->Test(cluster.hasGaps);

	int layer = cluster.layerID;

	/** falls V-Form gefordert, aber keine V-Form gegeben --> verwerfe cluster */
	if(fClusterConditionPar->requireVShape[cluster.layerID] && !cluster.hasVShape)
	{
		return false;
	}

	/** falls Ascending gefordert, aber kein Ascending gegeben --> verwerfe cluster */
	if(fClusterConditionPar->requireAscending[cluster.layerID] && !cluster.ascending)
	{
		return false;
	}

	/** falls Descending gefordert, aber kein Descending gegeben --> verwerfe Cluster */
	if(fClusterConditionPar->requireDescending[cluster.layerID] && !cluster.descending)
	{
		return false;
	}

	/** falls Cluster ohne Gaps gefordert, aber Gaps gegeben --> verwerfe Cluster */
	if(fClusterConditionPar->requireClusterWithoutGaps[cluster.layerID] && cluster.hasGaps)
	{
		return false;
	}

	/** sonst */
	return true;
}





//***********************************************************
// pruefe anzahl hits in cluster & Cluster-Spanne,
// setze entsprechende Conditions
// und vergleiche mit Forderungen an Cluster
// --> nutze Cluster
bool TQClamClusterProc::ProofClusterSize(TQClamCluster& cluster)
{
	int layer = cluster.layerID;

        /** ToT-Fenster pruefen */
        if(fClusterConditionPar->WireSpanCondMode != TQClamClusterConditionParameter::kOff)
        {
                /** Condition pruefen */
                bool inWireSpanWindow = false;
                if(layer == 0) { inWireSpanWindow = ((*fCondArrWireSpanInCluster)[0])->Test(cluster.wireSpan); }
                else if(layer == 1) { inWireSpanWindow = ((*fCondArrWireSpanInCluster)[1])->Test(cluster.wireSpan); }
                else if(layer == 2) { inWireSpanWindow = ((*fCondArrWireSpanInCluster)[2])->Test(cluster.wireSpan); }

                /** FlagOnly */
                if(fClusterConditionPar->WireSpanCondMode == TQClamClusterConditionParameter::kFlagOnly)
                {
                        /** falls Condition nicht erfuellt: markieren und weitergeben */
                        if(!inWireSpanWindow)
                        {
                                cluster.accepted = false;
                                return true;
                        }
                }

                /** Filter */
                if(fClusterConditionPar->WireSpanCondMode == TQClamClusterConditionParameter::kFilter)
                {
                        if(!inWireSpanWindow)
                        {
                                /** Ereignis markieren und verwerfen */
                                cluster.accepted = false;
                                return false;
                        }
                }
        }



        /** Driftzeit-Fenster pruefen */
        if(fClusterConditionPar->HitNumberCondMode != TQClamClusterConditionParameter::kOff)
        {
                /** Condition pruefen */
                bool inHitNumberWindow = false;
                if(layer == 0) { inHitNumberWindow = ((*fCondArrHitNumberInCluster)[0])->Test(cluster.numHits); }
                else if(layer == 1) { inHitNumberWindow = ((*fCondArrHitNumberInCluster)[1])->Test(cluster.numHits); }
                else if(layer == 2) { inHitNumberWindow = ((*fCondArrHitNumberInCluster)[2])->Test(cluster.numHits); }

                /** FlagOnly */
                if(fClusterConditionPar->HitNumberCondMode == TQClamClusterConditionParameter::kFlagOnly)
                {
                        /** falls Condition nicht erfuellt: markieren und weitergeben */
                        if(!inHitNumberWindow)
                        {
                                cluster.accepted = false;
                                return true;
                        }
                }

                /** Filter */
                if(fClusterConditionPar->HitNumberCondMode == TQClamClusterConditionParameter::kFilter)
                {
                        if(!inHitNumberWindow)
                        {
                                /** Ereignis markieren und verwerfen */
                                cluster.accepted = false;
                                return false;
                        }
                }
        }

        cluster.accepted = true;
        return true;
}





//***********************************************************
// setze Drahtnummern, Driftzeiten
void TQClamClusterProc::FillClusterProperties(TQClamCluster& cluster)
{
	/** anzahl hits */
	cluster.numHits = cluster.hits.size();

	/** driftzeit-, drahtnummer-vektor fuellen */
	cluster.drifttimes.reserve(cluster.numHits);
	cluster.tot.reserve(cluster.numHits);
	cluster.wireNumbers.reserve(cluster.numHits);
	cluster.normalCoords.reserve(cluster.numHits);

	int lastWire = -1;
	bool first = true;

	cluster.hasGaps = false;

	/** hits sind bereits nach Drahtnummer sortiert
	 * (GroupHitsByLayer ruft SortHitsByWireNumber auf) */
	for(auto* hit : cluster.hits)
	{
		cluster.drifttimes.push_back(hit->drifttime);
		cluster.tot.push_back(hit->tot);
		cluster.wireNumbers.push_back(hit->wireNum);
		cluster.normalCoords.push_back(fDriftcham.GetWire(hit->layerID,hit->wireNum)->GetNormalCoordinate());

		/*
		 * DEBUG TEST
		if(hit->layerID == 1)
		{
			std::cout << "normalCoord = " << fDriftcham.GetWire(hit->layerID,hit->wireNum)->GetNormalCoordinate() << std::endl;
		}
		*/

		if(!first)
		{
			if(hit->wireNum != lastWire +1) { cluster.hasGaps = true; }
		}

		lastWire = hit->wireNum;
		first = false;
	}

	/** setze mult-wire-hits */
	ProofMultiWireHits(cluster);

	cluster.meanDrifttime = TMath::Mean(cluster.drifttimes.size(), cluster.drifttimes.data());
	cluster.minDrifttime = *std::min_element(cluster.drifttimes.begin(),cluster.drifttimes.end());

	/** wireNumbers sortiert durch SortHitsByWireNumbers, aufgerufen in GroupHitsByLayer */
	cluster.minWireNumber = cluster.wireNumbers.front();
	cluster.maxWireNumber = cluster.wireNumbers.back();
	cluster.wireSpan = cluster.maxWireNumber - cluster.minWireNumber + 1;

	auto it = std::min_element(cluster.hits.begin(), cluster.hits.end(),
			[](auto a, auto b){ return a->drifttime < b->drifttime; });

	cluster.hitWithMinDrifttime = *it;
	cluster.wireNumberOfHitWithMinDrifttime = (*it)->wireNum;

	/**
	 * Berechne ersten Durchstosspunkt fuer die Track-Iteration
	 * -> gewichte jede Drahtposition mit der inversen Driftzeit.
	 */
	ComputeClusterMidpoint(cluster);
}






//***********************************************************
// berechne xMid/yMid, ersten Start fuer Iteration des TrackBuilders
void TQClamClusterProc::ComputeClusterMidpoint(TQClamCluster& cluster)
{
	if(cluster.hits.empty())
	{
		cluster.xMid = -1;
//		cluster.yMid = -1;
		cluster.zPos = -1;
		return;
	}

	double sumX = 0;
//	double sumY = 0;
	double sumZ = 0;
	double sumNormalCoord = 0;
	double sumInvDrifttime = 0;

	for(const TQClamHit* hit : cluster.hits)
	{
		// * Drahtebene holen * /
		const auto* wireplane = fDriftcham.GetWirePlane(hit->layerID);
		if(!wireplane) { continue; }

		// ** Draht holen * /
		const auto* wire = fDriftcham.GetWire(hit->layerID, hit->wireNum);
		if(!wire) { continue; }

		// ** drahtmittelpunkte holen * /
		TVector3 p = wireplane->GetWirePoint(wire);

		/** gewichten mit Driftzeit
		 * --> kurze Driftzeiten nahe Mittelpunkt
		 * --> Gewichtung mit 1/driftzeit */
		sumX += p.X()/hit->drifttime;
//		sumY += p.Y()/hit->drifttime;
		sumZ += p.Z()/hit->drifttime;
		// gewichte Normalkoordinate mit inversen Drifzeiten zur Mittelpunktbestimmung.
		sumNormalCoord += wire->GetNormalCoordinate()/hit->drifttime;

		sumInvDrifttime += 1/hit->drifttime;
	}

	//const double n = static_cast<double>(cluster.numHits);

	cluster.xMid = sumX/sumInvDrifttime;
//	cluster.yMid = sumY/sumInvDrifttime;
	cluster.zPos = sumZ/sumInvDrifttime;
	cluster.meanNormalCoordinate = sumNormalCoord/sumInvDrifttime;
}




//***********************************************************
// pruefe Multi-Wire-Hits
void TQClamClusterProc::ProofMultiWireHits(TQClamCluster& cluster)
{
	std::unordered_map<int,int> count;

	for(int w : cluster.wireNumbers) { count[w]++; }

	cluster.numOfMultiWireHits = 0;
	cluster.hasMultiWireHits = false;

	for(const auto& [wire,c] : count)
	{
		if(c>1)
		{
			cluster.numOfMultiWireHits++;
			cluster.hasMultiWireHits = true;
		}
	}
}





//***********************************************************
// pruefe, ob V-Shape vorliegt
bool TQClamClusterProc::ProofVShape(TQClamCluster& cluster) const
{
	size_t nHits = cluster.numHits;

	/** driftzeiten extrahieren */
	std::vector<double> drifttimes;
	drifttimes.reserve(nHits);
	for(auto* hit : cluster.hits) { drifttimes.push_back(hit->drifttime); }

	/** wenn < 3 hits --> V-Form unmoeglich */
	if(nHits < 3)
	{
		cluster.hasVShape = false;
		/** pruefe de-/ascending */
		ProofDeOrAscending(cluster, drifttimes);

		return false;
	}

	/** index von driftzeit in drifttimes mit min. driftzeit finden */
	int i_minDrifttime = std::distance(drifttimes.begin(), std::min_element(drifttimes.begin(), drifttimes.end()) );

	/** pruefe, ob links von minDrifttime-Index die driftzeit faellt */
	for(size_t i=0; i<i_minDrifttime; i++)
	{
		if(drifttimes[i] < drifttimes[i+1])
		{
			cluster.hasVShape = false;
			ProofDeOrAscending(cluster, drifttimes);
			return false;
		}
	}

	/** falls der hit mit der minimalen driftzeit der Hit mit der hoechsten Drahtnummer ist --> setze descending */
	if(nHits-1==i_minDrifttime)
	{
		cluster.ascending = false;
		cluster.hasVShape = false;
		cluster.descending = true;
		return false;
	}

	/** pruefe, ob rechts von minDrifttime-Index die Driftzeit waechst */
	for(size_t i=i_minDrifttime; i<nHits-1; i++)
	{
		if(drifttimes[i] > drifttimes[i+1])
		{
			cluster.hasVShape = false;
			ProofDeOrAscending(cluster,drifttimes);
			return false;
		}
	}
			
	/** sonst: V-Form gegeben */
	cluster.hasVShape = true;
	cluster.ascending = false;
	cluster.descending = false;

	return true;
}




//***********************************************************
// sortiere Hits nach Drahtebenen-ID
// --> hmm map aendern
std::map<int,std::vector<const TQClamHit*>> TQClamClusterProc::GroupHitsByLayer(const std::vector<TQClamHit>& hits) const
{
	std::map<int,std::vector<const TQClamHit*>> hitsGroupedByLayer; /** Drahtebenen-ID,vectorHitsInEbene */
	for(auto& hit : hits)
	{
		hitsGroupedByLayer[hit.layerID].push_back(&hit);
	}


	/** sortiere hits nach drahtnummmer */
	for(auto& [layer,hitsInLayer] : hitsGroupedByLayer)
	{
		SortHitsByWireNumber(hitsInLayer);
	}

	return hitsGroupedByLayer;
}





//***********************************************************
// sortiere hits nach drahtnummer
void TQClamClusterProc::SortHitsByWireNumber(std::vector<const TQClamHit*>& hits) const
{
	std::sort(hits.begin(), hits.end(), [](auto hit1, auto hit2) {
			return hit1->wireNum < hit2->wireNum;});
}












//***********************************************************
// setze in cluster-struktur ascending/descending
// --> uebergebe nach draht georndete driftzeiten von cluster
void TQClamClusterProc::ProofDeOrAscending(TQClamCluster& cluster, const std::vector<double> & drifttimes) const
{
	if(drifttimes.size() < 2)
	{
		cluster.ascending = false;
		cluster.descending = false;
		return;
	}

	bool ascending = true;
	bool descending = true;

	for(size_t i=1; i<drifttimes.size(); i++)
	{
		if(drifttimes[i] > drifttimes[i-1])
		{
			descending = false;
		}
		else if(drifttimes[i] < drifttimes[i-1])
		{
			ascending = false;
		}

		/** beide false => abbrechen */
		if(!ascending && !descending)
		{
			cluster.ascending = ascending;
			cluster.descending = descending;
			return;
		}
	}

	cluster.ascending = ascending;
	cluster.descending = descending;
}





//***********************************************************
// Fuelle Histogramme vor Anwendung Condition
void TQClamClusterProc::FillHistogramsWithOutCond(const TQClamCluster& cluster)
{
	int layer = cluster.layerID;

	if(layer == 0)
	{
		fHHitNumberInClusterX1->Fill(cluster.numHits);
		fHWireSpanInClusterX1->Fill(cluster.wireSpan);
		fHHitNumberVsWireSpanX1->Fill(cluster.wireSpan,cluster.numHits);

		fHMinDrifttimeInClusterX1->Fill(cluster.minDrifttime);

		fHXMidClusterX1->Fill(cluster.xMid);
		fHWireNumberWithMinDrifttimeInClusterX1->Fill(cluster.wireNumberOfHitWithMinDrifttime);
	}

	else if(layer == 1)
	{
		fHHitNumberInClusterU->Fill(cluster.numHits);
		fHWireSpanInClusterU->Fill(cluster.wireSpan);
		fHHitNumberVsWireSpanU->Fill(cluster.wireSpan,cluster.numHits);

		fHMinDrifttimeInClusterU->Fill(cluster.minDrifttime);

		fHXMidClusterU->Fill(cluster.xMid);
//		fHYMidClusterU->Fill(cluster.yMid);
		fHWireNumberWithMinDrifttimeInClusterU->Fill(cluster.wireNumberOfHitWithMinDrifttime);
	}

	else if(layer == 2)
	{
		fHHitNumberInClusterX2->Fill(cluster.numHits);
		fHWireSpanInClusterX2->Fill(cluster.wireSpan);
		fHHitNumberVsWireSpanX2->Fill(cluster.wireSpan,cluster.numHits);

		fHMinDrifttimeInClusterX2->Fill(cluster.minDrifttime);

		fHXMidClusterX2->Fill(cluster.xMid);
		fHWireNumberWithMinDrifttimeInClusterX2->Fill(cluster.wireNumberOfHitWithMinDrifttime);
	}




//	fHClusterPropertiesBeforeCut->Fill(cluster.hasGaps ? 1 : 0, layer);

	if(layer == 0)
	{
		// Gaps
		if(cluster.hasGaps)
		{
			fCountX1Gap++;
			fHClusterPropertiesBeforeCut->SetBinContent(2,layer+1,double(fCountX1Gap)/double(fEvtCounter));
		}
		else
		{
			fCountX1NoGap++;
			fHClusterPropertiesBeforeCut->SetBinContent(1,layer+1,double(fCountX1NoGap)/double(fEvtCounter));
		}

		// Ascending
		if(cluster.ascending)
		{
			fCountX1Asc++;
			fHClusterPropertiesBeforeCut->SetBinContent(3,layer+1,double(fCountX1Asc)/double(fEvtCounter));
		}

		// Descending
		if(cluster.descending)
		{
			fCountX1Desc++;
			fHClusterPropertiesBeforeCut->SetBinContent(4,layer+1,double(fCountX1Desc)/double(fEvtCounter));
		}

		// VShape
		if(cluster.hasVShape)
		{
			fCountX1VShape++;
			fHClusterPropertiesBeforeCut->SetBinContent(5,layer+1,double(fCountX1VShape)/double(fEvtCounter));
		}
	}

	if(layer == 1)
	{
		// Gaps
		if(cluster.hasGaps)
		{
			fCountUGap++;
			fHClusterPropertiesBeforeCut->SetBinContent(2,layer+1,double(fCountUGap)/double(fEvtCounter));
		}
		else
		{
			fCountUNoGap++;
			fHClusterPropertiesBeforeCut->SetBinContent(1,layer+1,double(fCountUNoGap)/double(fEvtCounter));
		}

		// Ascending
		if(cluster.ascending)
		{
			fCountUAsc++;
			fHClusterPropertiesBeforeCut->SetBinContent(3,layer+1,double(fCountUAsc)/double(fEvtCounter));
		}

		// Descending
		if(cluster.descending)
		{
			fCountUDesc++;
			fHClusterPropertiesBeforeCut->SetBinContent(4,layer+1,double(fCountUDesc)/double(fEvtCounter));
		}

		// VShape
		if(cluster.hasVShape)
		{
			fCountUVShape++;
			fHClusterPropertiesBeforeCut->SetBinContent(5,layer+1,double(fCountUVShape)/double(fEvtCounter));
		}
	}



	if(layer == 2)
	{
		// Gaps
		if(cluster.hasGaps)
		{
			fCountX2Gap++;
			fHClusterPropertiesBeforeCut->SetBinContent(2,layer+1,double(fCountX2Gap)/double(fEvtCounter));
		}
		else
		{
			fCountX2NoGap++;
			fHClusterPropertiesBeforeCut->SetBinContent(1,layer+1,double(fCountX2NoGap)/double(fEvtCounter));
		}

		// Ascending
		if(cluster.ascending)
		{
			fCountX2Asc++;
			fHClusterPropertiesBeforeCut->SetBinContent(3,layer+1,double(fCountX2Asc)/double(fEvtCounter));
		}

		// Descending
		if(cluster.descending)
		{
			fCountX2Desc++;
			fHClusterPropertiesBeforeCut->SetBinContent(4,layer+1,double(fCountX2Desc)/double(fEvtCounter));
		}

		// VShape
		if(cluster.hasVShape)
		{
			fCountX2VShape++;
			fHClusterPropertiesBeforeCut->SetBinContent(5,layer+1,double(fCountX2VShape)/double(fEvtCounter));
		}
	}




}





//***********************************************************
// Fuelle Histogramme nach Anwendung Condition
void TQClamClusterProc::FillHistogramsWithCond(const TQClamCluster& cluster)
{
	int layer = cluster.layerID;

	if(layer == 0)
	{
		fHHitNumberInClusterX1Cut->Fill(cluster.numHits);
		fHWireSpanInClusterX1Cut->Fill(cluster.wireSpan);
		fHHitNumberVsWireSpanX1Cut->Fill(cluster.wireSpan,cluster.numHits);

		fHMinDrifttimeInClusterX1Cut->Fill(cluster.minDrifttime);

		fHXMidClusterX1Cut->Fill(cluster.xMid);
		fHWireNumberWithMinDrifttimeInClusterX1Cut->Fill(cluster.wireNumberOfHitWithMinDrifttime);
	}

	else if(layer == 1)
	{
		fHHitNumberInClusterUCut->Fill(cluster.numHits);
		fHWireSpanInClusterUCut->Fill(cluster.wireSpan);
		fHHitNumberVsWireSpanUCut->Fill(cluster.wireSpan,cluster.numHits);

		fHMinDrifttimeInClusterUCut->Fill(cluster.minDrifttime);

		fHXMidClusterUCut->Fill(cluster.xMid);
//		fHYMidClusterUCut->Fill(cluster.yMid);
		fHWireNumberWithMinDrifttimeInClusterUCut->Fill(cluster.wireNumberOfHitWithMinDrifttime);
	}

	else if(layer == 2)
	{
		fHHitNumberInClusterX2Cut->Fill(cluster.numHits);
		fHWireSpanInClusterX2Cut->Fill(cluster.wireSpan);
		fHHitNumberVsWireSpanX2Cut->Fill(cluster.wireSpan,cluster.numHits);

		fHMinDrifttimeInClusterX2Cut->Fill(cluster.minDrifttime);

		fHXMidClusterX2Cut->Fill(cluster.xMid);
		fHWireNumberWithMinDrifttimeInClusterX2Cut->Fill(cluster.wireNumberOfHitWithMinDrifttime);
	}



	if(layer == 0)
	{
		// Gaps
		if(cluster.hasGaps)
		{
			fCountAfterX1Gap++;
			fHClusterPropertiesAfterCut->SetBinContent(2,layer+1,double(fCountAfterX1Gap)/double(fEvtCounter));
		}
		else
		{
			fCountAfterX1NoGap++;
			fHClusterPropertiesAfterCut->SetBinContent(1,layer+1,double(fCountAfterX1NoGap)/double(fEvtCounter));
		}

		// Ascending
		if(cluster.ascending)
		{
			fCountAfterX1Asc++;
			fHClusterPropertiesAfterCut->SetBinContent(3,layer+1,double(fCountAfterX1Asc)/double(fEvtCounter));
		}

		// Descending
		if(cluster.descending)
		{
			fCountAfterX1Desc++;
			fHClusterPropertiesAfterCut->SetBinContent(4,layer+1,double(fCountAfterX1Desc)/double(fEvtCounter));
		}

		// VShape
		if(cluster.hasVShape)
		{
			fCountAfterX1VShape++;
			fHClusterPropertiesAfterCut->SetBinContent(5,layer+1,double(fCountAfterX1VShape)/double(fEvtCounter));
		}
	}

	if(layer == 1)
	{
		// Gaps
		if(cluster.hasGaps)
		{
			fCountAfterUGap++;
			fHClusterPropertiesAfterCut->SetBinContent(2,layer+1,double(fCountAfterUGap)/double(fEvtCounter));
		}
		else
		{
			fCountAfterUNoGap++;
			fHClusterPropertiesAfterCut->SetBinContent(1,layer+1,double(fCountAfterUNoGap)/double(fEvtCounter));
		}

		// Ascending
		if(cluster.ascending)
		{
			fCountAfterUAsc++;
			fHClusterPropertiesAfterCut->SetBinContent(3,layer+1,double(fCountAfterUAsc)/double(fEvtCounter));
		}

		// Descending
		if(cluster.descending)
		{
			fCountAfterUDesc++;
			fHClusterPropertiesAfterCut->SetBinContent(4,layer+1,double(fCountAfterUDesc)/double(fEvtCounter));
		}

		// VShape
		if(cluster.hasVShape)
		{
			fCountAfterUVShape++;
			fHClusterPropertiesAfterCut->SetBinContent(5,layer+1,double(fCountAfterUVShape)/double(fEvtCounter));
		}
	}



	if(layer == 2)
	{
		// Gaps
		if(cluster.hasGaps)
		{
			fCountAfterX2Gap++;
			fHClusterPropertiesAfterCut->SetBinContent(2,layer+1,double(fCountAfterX2Gap)/double(fEvtCounter));
		}
		else
		{
			fCountAfterX2NoGap++;
			fHClusterPropertiesAfterCut->SetBinContent(1,layer+1,double(fCountAfterX2NoGap)/double(fEvtCounter));
		}

		// Ascending
		if(cluster.ascending)
		{
			fCountAfterX2Asc++;
			fHClusterPropertiesAfterCut->SetBinContent(3,layer+1,double(fCountAfterX2Asc)/double(fEvtCounter));
		}

		// Descending
		if(cluster.descending)
		{
			fCountAfterX2Desc++;
			fHClusterPropertiesAfterCut->SetBinContent(4,layer+1,double(fCountAfterX2Desc)/double(fEvtCounter));
		}

		// VShape
		if(cluster.hasVShape)
		{
			fCountAfterX2VShape++;
			fHClusterPropertiesAfterCut->SetBinContent(5,layer+1,double(fCountAfterX2VShape)/double(fEvtCounter));
		}
	}




}













bool TQClamClusterProc::CleanCluster(TQClamCluster& cluster)
{
	// ------------------------------------------------------------
	// Ziel:
	//
	// Sehr konservatives Cleaning:
	//
	//   1) isolierte lokale Spike-Hits entfernen
	//   2) kleine Gap-Inseln entfernen
	//
	// WICHTIG:
	// - Driftzeit-Residuals NICHT iterativ
	// - Gap-Segmente DÜRFEN iterativ bereinigt werden
	//
	// ------------------------------------------------------------

	if(cluster.hits.size() < 3 || !fClusterConditionPar->fCleanCluster)
	{
		return false;
	}

	const size_t sizeBefore = cluster.hits.size();

	// ------------------------------------------------------------
	// Parameter
	// ------------------------------------------------------------

	const double maxResidual = fClusterConditionPar->fMaxDrifttimeJump[cluster.layerID];

	const int maxAllowedGap = fClusterConditionPar->fMaxAllowedGapInCleaning[cluster.layerID];

	const size_t maxRemovedHits = fClusterConditionPar->fMaxRemovedHits[cluster.layerID];

	const size_t maxIterations = fClusterConditionPar->fMaxCleaningIterations[cluster.layerID];

    
	// ------------------------------------------------------------
    
	// Arbeitskopie
    
	// ------------------------------------------------------------

	std::vector<const TQClamHit*> hits = cluster.hits;

	std::sort(hits.begin(),hits.end(),
			[](const TQClamHit* a,const TQClamHit* b) {
			return a->wireNum < b->wireNum;
			});

	size_t totalRemoved = 0;

	// ============================================================
	// 1) EINMALIGES SPIKE-CLEANING
	// ============================================================

	{
		std::vector<char> remove(hits.size(), 0);

		for(size_t i = 1; i + 1 < hits.size(); ++i)
		{
			if(totalRemoved >= maxRemovedHits) { break; }

			const auto* prev = hits[i - 1];
			const auto* curr = hits[i];
			const auto* next = hits[i + 1];

			// ----------------------------------------------------
			// Nur direkt benachbarte Wires
			// ----------------------------------------------------

			const int gap1 = std::abs(curr->wireNum - prev->wireNum);

			const int gap2 = std::abs(next->wireNum - curr->wireNum);

			if(gap1 != 1 || gap2 != 1) { continue; }

			// ----------------------------------------------------
			// Nachbarn konsistent?
			// ----------------------------------------------------

			const double neighborJump = std::abs(next->drifttime - prev->drifttime);

            

			if(fHistoPar->fBFillHistos && fHDrifttimeJump) { fHDrifttimeJump->Fill(neighborJump); }

			// Große lokale Struktur:
			// vermutlich echte Physik
			if(neighborJump > maxResidual) { continue; }

			// ----------------------------------------------------
			// Residual
			// ----------------------------------------------------
			const double meanNeighbor = 0.5 * (prev->drifttime + next->drifttime);

			const double residual = std::abs(curr->drifttime - meanNeighbor);

            
			if(fHistoPar->fBFillHistos && fHDrifttimeResidualAll) { fHDrifttimeResidualAll->Fill(residual); }

			// ----------------------------------------------------
			// Isolierter Spike?
			// ----------------------------------------------------

			if(residual > maxResidual)
			{
				remove[i] = 1;

				totalRemoved++;

				if(fHistoPar->fBFillHistos && fHDrifttimeResidualRemoved) { fHDrifttimeResidualRemoved->Fill(residual); }
			}
		}

        
		// --------------------------------------------------------
        
		// Spike-Hits entfernen
        
		// --------------------------------------------------------

        
		std::vector<const TQClamHit*> cleanedHits;
        
		cleanedHits.reserve(hits.size());

		for(size_t i = 0; i < hits.size(); ++i)
		{
			if(!remove[i]) { cleanedHits.push_back(hits[i]); }
		}

		hits = cleanedHits;
	}

	// ============================================================
	// 2) ITERATIVES GAP-INSEL-CLEANING
	// ============================================================

	size_t iter = 0;

	while(iter < maxIterations)
	{
		bool changed = false;

		if(hits.size() < 2) { break; }

		// --------------------------------------------------------
		// Segmente bestimmen
		// --------------------------------------------------------

		std::vector<std::pair<size_t,size_t>> segments;

		size_t segStart = 0;

		for(size_t i = 1; i < hits.size(); ++i)
		{
			const int gap = hits[i]->wireNum - hits[i - 1]->wireNum;

			if(gap > maxAllowedGap)
			{
				segments.push_back({segStart, i - 1});

				segStart = i;
			}
		}

		segments.push_back({segStart, hits.size() - 1});

		// --------------------------------------------------------
		// Nur ein Segment -> fertig
		// --------------------------------------------------------

		if(segments.size() <= 1) { break; }

		// --------------------------------------------------------
		// Größtes Segment suchen
		// --------------------------------------------------------

		size_t largestSeg = 0;
		size_t largestSize = 0;

		for(size_t s = 0; s < segments.size(); ++s)
		{
			const size_t segSize = segments[s].second - segments[s].first + 1;

			if(segSize > largestSize)
			{
				largestSize = segSize;
				largestSeg = s;
			}
		}

		// --------------------------------------------------------
		// Alle anderen Segmente entfernen
		// --------------------------------------------------------

		std::vector<const TQClamHit*> cleanedHits;

		for(size_t s = 0; s < segments.size(); ++s)
		{
			const bool keep = (s == largestSeg);

			for(size_t i = segments[s].first; i <= segments[s].second; ++i)
			{
				if(keep)
				{
					cleanedHits.push_back(hits[i]);
				}
				else
				{
					if(totalRemoved < maxRemovedHits)
					{
						totalRemoved++;
						changed = true;
					}
					else
					{
						// Removal-Limit erreicht:
						// Rest behalten
						cleanedHits.push_back(hits[i]);
					}
				}
			}
		}

		hits = cleanedHits;

		// keine Änderungen -> fertig
		if(!changed) { break; }

		iter++;
	}


	// ============================================================
	// 3) MULTI-WIRE CLEANING (Draehte, die mehrfach gefeuert haben?)
	//
	// Strategie:
        //
        // - fuer jeden Draht mit mehreren Hits:
        //      * berechne Driftzeit-Residual jedes Hits
        //      * entferne eindeutig schlechte Hits
        //      * falls keine eindeutige Entscheidung:
        //            -> behalte Hit mit minimaler Driftzeit
        //
	// ============================================================
        {
                std::vector<const TQClamHit*> cleanedHits;
                cleanedHits.reserve(hits.size());


                size_t i = 0;

		bool multiWireHits = false;

		// draehte bereits sortiert
                while(i < hits.size())
                {
                        size_t j = i + 1;


                        // ------------------------------------------------
                        // alle Hits desselben Drahtes finden
                        // ------------------------------------------------
                        while(j < hits.size() && hits[j]->wireNum == hits[i]->wireNum)
			{
				j++;
			}


                        // ------------------------------------------------
                        // Single-Wire-Hit:
                        // direkt übernehmen
                        // ------------------------------------------------
                        if(j - i == 1)
                        {
                                cleanedHits.push_back(hits[i]);
                                i = j;
                                continue; // kein multi-hit -> ueberspringe den rest
                        }

			multiWireHits = true;

                        // ------------------------------------------------
                        // Multi-Wire-Hit:
                        // Hits von i bis j-1 gehören zum gleichen Draht
                        // ------------------------------------------------
                        std::vector<const TQClamHit*> wireHits;

                        for(size_t k=i; k<j; k++)
                        {
                                wireHits.push_back(hits[k]);
                        }


                        // ------------------------------------------------
                        // nach Driftzeit sortieren
                        // ------------------------------------------------
                        std::sort(
                            wireHits.begin(),
                            wireHits.end(),
                            [](const TQClamHit* a,const TQClamHit* b)
                            {
                                return a->drifttime < b->drifttime;
                            });



                        std::vector<double> residuals(wireHits.size(),1e9);



                        // ------------------------------------------------
                        // Nachbarhits des Drahtes suchen
                        // ------------------------------------------------
                        const TQClamHit* leftHit = nullptr;
                        const TQClamHit* rightHit = nullptr;


                        if(i > 0 && hits[i-1]->wireNum == wireHits[0]->wireNum - 1)
                        {
                                leftHit = hits[i-1];
                        }


                        if(j < hits.size() && hits[j]->wireNum == wireHits[0]->wireNum + 1)
                        {
                                rightHit = hits[j];
                        }



                        // ------------------------------------------------
                        // erwartete Driftzeit bestimmen
                        // ------------------------------------------------
                        double expectedDrifttime = -1;


                        if(leftHit && rightHit)
                        {
                                expectedDrifttime = 0.5 * (leftHit->drifttime + rightHit->drifttime);
                        }
                        else if(leftHit)
                        {
				expectedDrifttime = leftHit->drifttime;
                        }
                        else if(rightHit)
                        {
				expectedDrifttime = rightHit->drifttime;
                        }



                        // ------------------------------------------------
                        // Residuen berechnen
                        // ------------------------------------------------
                        if(expectedDrifttime >= 0)
                        {
				for(size_t k=0;k<wireHits.size();k++)
				{
					residuals[k] = std::abs(wireHits[k]->drifttime - expectedDrifttime);
                                }
                        }



                        // ------------------------------------------------
                        // bester und zweitbester Kandidat
                        // ------------------------------------------------
                        size_t best = 0;

                        double bestResidual = residuals[0];
                        //double secondBestResidual = 1e9;


			for(size_t k=1;k<residuals.size();k++)
			{
				if(residuals[k] < bestResidual)
				{
					//secondBestResidual = bestResidual;
					bestResidual = residuals[k];
					best = k;
				}
				/*
				else if(residuals[k] < secondBestResidual)
				{
					secondBestResidual = residuals[k];
                                
				}
				*/
			}


                        // ------------------------------------------------
                        // Entscheidung
                        // ------------------------------------------------
                        bool uniqueBest = false;


                        if(expectedDrifttime >= 0) // && bestResidual < maxResidual && (secondBestResidual - bestResidual) > maxResidual)
			{
				uniqueBest = true;
			}



                        if(uniqueBest)
                        {
                                // eindeutiger guter Hit
                                cleanedHits.push_back(wireHits[best]);

                                totalRemoved += wireHits.size() - 1;


				// TODO evtl...
				/*
                                if(fHMultiWireRemoved)
                                {
					fHMultiWireRemoved->Fill(wireHits.size()-1);
                                }
				*/

                        }
                        else
                        {
                                // keine eindeutige Entscheidung:
                                // kleinste Driftzeit behalten
                                cleanedHits.push_back(wireHits[0]);
                                totalRemoved += wireHits.size() - 1;

				// TODO evtl....
				/*
                                if(fHMultiWireFallback)
                                {
                                        
					fHMultiWireFallback->Fill(wireHits.size()-1);
                                }
				*/
                        }



                        i = j;
                }


                hits = cleanedHits;


//		if(multiWireHits)
		{
			std::sort(hits.begin(),hits.end(),
					[](const TQClamHit* a,const TQClamHit* b)
					{
					return a->wireNum < b->wireNum;
					});
		}


        }




	// ============================================================
	// Cluster aktualisieren
	// ============================================================

	cluster.hits = hits;

	cluster.wireNumbers.clear();
	cluster.drifttimes.clear();
	cluster.normalCoords.clear();

	// ============================================================
	// Histogramme
	// ============================================================

	if(fHistoPar->fBFillHistos)
	{
		if(fHClusterRemovedHits) { fHClusterRemovedHits->Fill(totalRemoved); }

		if(fHClusterSizeBeforeClean) { fHClusterSizeBeforeClean->Fill(sizeBefore); }

		if(fHClusterSizeAfterClean) { fHClusterSizeAfterClean->Fill(cluster.hits.size()); }

		if(fHClusterSizeBeforeVsAfterClean) { fHClusterSizeBeforeVsAfterClean->Fill(sizeBefore,cluster.hits.size()); }

		if(fHClusterShapeChange)
		{
			const int changed = (sizeBefore != cluster.hits.size());

			fHClusterShapeChange->Fill(changed);
		}
	}

	return (totalRemoved > 0);
}



