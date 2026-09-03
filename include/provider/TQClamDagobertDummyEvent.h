#ifndef TQCLAMDAGOBERTDUMMYEVENT_H
#define TQCLAMDAGOBERTDUMMYEVENT_H

#include "TGo4EventElement.h"

class TQClamDagobertProviderProc;

class TQClamDagobertDummyEvent : public TGo4EventElement
{
	public:
		TQClamDagobertDummyEvent();
		TQClamDagobertDummyEvent(const char* name);
		virtual ~TQClamDagobertDummyEvent();

		Int_t Fill() override;
		void Clear(Option_t* opt = "") override;
		Int_t Init() override;

	private:
		/* nicht streamen!! */
		TQClamDagobertProviderProc *fxProviderProc{nullptr}; //!

	ClassDefOverride(TQClamDagobertDummyEvent,1);
};

#endif
