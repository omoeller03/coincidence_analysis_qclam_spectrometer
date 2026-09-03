#ifndef TQCLAMTCPEVENT_H
#define TQCLAMTCPEVENT_H

#include "TGo4EventElement.h"


class TQClamTcpEvent : public TGo4EventElement
{
	public:
		TQClamTcpEvent(const char* name="TcpEvent") : TGo4EventElement(name)
		{}
		virtual ~TQClamTcpEvent() {}

		Double_t fEntonRate;
		Double_t fSzintScattChamberRate;

		virtual void Clear(Option_t* opt="") override;




	ClassDefOverride(TQClamTcpEvent,1);
};

#endif
