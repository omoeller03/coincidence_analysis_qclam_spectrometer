#ifndef TQCLAMBACKGROUNDRATEEVENT_H
#define TQCLAMBACKGROUNDRATEEVENT_H

#include "TGo4EventElement.h"


class TQClamBackgroundRateEvent : public TGo4EventElement
{
	public:
		TQClamBackgroundRateEvent() : TQClamBackgroundRateEvent("TQClamBackgroundRateEvent") {}
		TQClamBackgroundRateEvent(const char* name = "BackgroundRateEvent") : TGo4EventElement(name)
		{}

		virtual ~TQClamBackgroundRateEvent()
		{}

		virtual void Clear(Option_t* opt="") override;

		Double_t fEntonRate;
		Double_t fSzintScattChamberRate;


	ClassDefOverride(TQClamBackgroundRateEvent,1);
};


#endif
