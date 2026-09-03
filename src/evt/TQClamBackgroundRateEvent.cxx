#include "evt/TQClamBackgroundRateEvent.h"


ClassImp(TQClamBackgroundRateEvent)

void TQClamBackgroundRateEvent::Clear(Option_t* opt)
{
	TGo4EventElement::Clear(opt);

	fEntonRate = 0;
	fSzintScattChamberRate = 0;
}
