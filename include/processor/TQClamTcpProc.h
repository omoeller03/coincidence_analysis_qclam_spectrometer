#ifndef TQCLAMTCPPROC_H
#define TQCLAMTCPPROC_H

#include "TGo4EventProcessor.h"


class TQClamTcpProc : public TGo4EventProcessor
{
	public:
		TQClamTcpProc();
		TQClamTcpProc(const char* name);

		~virtual TQClamTcpProc();


		Bool_t CheckEventClass(TClass* cl) override;
		Bool_t BuildEvent(TGo4EventElement* dest) override;

	ClassDefOverride(TQClamTcpProc,1);
};


#endif
