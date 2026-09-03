#include "evt/TQClamTcpEvent.h"



ClassImp(TQClamTcpEvent)

void TQClamTcpEvent::Clear(Option_t* opt)
{
	TGo4EventElement::Clear(opt);

	fEntonRate = 0;
	fSzintScattChamberRate = 0;
}
