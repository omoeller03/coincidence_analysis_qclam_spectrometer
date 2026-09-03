#ifndef TDAGOBERTHITEVENT_H
#define TDAGOBERTHITEVENT_H

#include "TGo4EventElement.h"
#include "structs/TQClamMbsHeader.h"


struct DagHit
{
	int module = -1;
	int channel = -1;

	double energy = NAN;
	double calibrated_energy = NAN;
	double time = NAN;
	double dtToMeanSzintTime = NAN;
	double dtToLaBrCoincTime = NAN;
	bool pileup = false;


	uint64_t rawScaler = 0;
	uint64_t accScaler = 0;
	uint64_t vetoScaler = 0;
	bool hasScaler = false;

	double rateRaw = NAN;
	double rateAccepted= NAN;
	double deadtime = NAN;
};


class TDagobertHitEvent : public TGo4EventElement
{
	public:
		TDagobertHitEvent();
		TDagobertHitEvent(const char* name);


		virtual ~TDagobertHitEvent()
		{}

		MbsHeader header; //! nicht streamen

		DagHit laBr[9]; // 0 .. 8
		DagHit forwardLaBr; //! vorwaertswinkel labr
		DagHit cherenkov; //! vorwaertswinkel cherenkov
		DagHit plastic; //! vorwaertswinkel ehemals

		/** raten */
		double rateEnton = -1.;
		double rateSzintScattChamber = -1.;
		double rateLaBrOR = -1.;
		double rateLaBrORNoDead = -1.;

		/** zeiten */
		double coincTimeLaBr = -1.;
		double szintPmtUpTime = -1.;
		double szintPmtDownTime = -1;
		double meanSzintTime = -1.;

		void Clear(Option_t* opt="") override;

	ClassDefOverride(TDagobertHitEvent,1);
};


#endif
