/* adaptiert aus TGo4ExampleMesh von JAM */
#ifndef TQCLAMDAGOBERTPROVIDERPROC_H
#define TQCLAMDAGOBERTPROVIDERPROC_H

#include "TGo4EventProcessor.h"

class TQClamDagobertProviderProc : public TGo4EventProcessor
{
	public:
		TQClamDagobertProviderProc();
		TQClamDagobertProviderProc(const char* name);
		virtual ~TQClamDagobertProviderProc();

		/** pruefe, ob input-event korrekt und falls dem so ist:
		 * setzt event-pointer fuer folgende steps */
		void SetRealInput();

	ClassDefOverride(TQClamDagobertProviderProc,1);
};


#endif
