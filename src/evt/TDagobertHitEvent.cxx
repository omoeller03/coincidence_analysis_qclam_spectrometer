#include "evt/TDagobertHitEvent.h"

#include "TGo4Log.h"


TDagobertHitEvent::TDagobertHitEvent()
	: TGo4EventElement("DagobertHit")
{}


TDagobertHitEvent::TDagobertHitEvent(const char* name)
	: TGo4EventElement(name)
{}


void TDagobertHitEvent::Clear(Option_t* opt)
{
	TGo4EventElement::Clear(opt);
/*
        // LaBr 0..8
        for(int i = 0; i < 9; ++i)
        {
                laBr[i].module = -1;
                laBr[i].channel = -1;
                laBr[i].energy = 0.;

                laBr[i].time = 0.;
                laBr[i].dtToMeanSzintTime = 0.;
                laBr[i].dtToLaBrCoincTime = 0.;
                laBr[i].pileup = false;

                laBr[i].rawScaler = 0;
                laBr[i].accScaler = 0;
                laBr[i].vetoScaler = 0;

                laBr[i].rateRaw = 0.;
                laBr[i].rateAccepted = 0.;
                laBr[i].deadtime = 0.;
        }

        forwardLaBr = DagHit();
        cherenkov = DagHit();
        plastic = DagHit();



        rateEnton = -1.;
        rateSzintScattChamber = -1.;
        rateLaBrOR = -1.;
        rateLaBrORNoDead = -1.;

        coincTimeLaBr = -1.;
        szintPmtUpTime = -1.;
        szintPmtDownTime = -1.;
        meanSzintTime = -1.;
*/

	// LaBr 0..8
	for(int i = 0; i < 9; ++i)
	{
		laBr[i].module = -1;
		laBr[i].channel = -1;
		laBr[i].energy = NAN;
        
		laBr[i].calibrated_energy = NAN;
		laBr[i].time = NAN;
		laBr[i].dtToMeanSzintTime = NAN;
		laBr[i].dtToLaBrCoincTime = NAN;
		laBr[i].pileup = false;

		laBr[i].rawScaler = 0;
		laBr[i].accScaler = 0;
		laBr[i].vetoScaler = 0;

		laBr[i].rateRaw = NAN;
		laBr[i].rateAccepted = NAN;
		laBr[i].deadtime = NAN;

	}

	forwardLaBr = DagHit();
	cherenkov = DagHit();
	plastic = DagHit();

    

	rateEnton = -1.;
	rateSzintScattChamber = -1.;
	rateLaBrOR = -1.;
	rateLaBrORNoDead = -1.;
	
	coincTimeLaBr = -1.;
	szintPmtUpTime = -1.;
	szintPmtDownTime = -1.;
	meanSzintTime = -1.;
}
