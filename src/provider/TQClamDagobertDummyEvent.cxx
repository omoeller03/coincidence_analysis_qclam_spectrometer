/** adaptiert aus TGo4ExampleMesh von JAM */
#include "provider/TQClamDagobertDummyEvent.h"

#include "provider/TQClamDagobertProviderProc.h"


ClassImp(TQClamDagobertDummyEvent)

TQClamDagobertDummyEvent::TQClamDagobertDummyEvent()
	: TGo4EventElement("DummyEvent")
{}


TQClamDagobertDummyEvent::TQClamDagobertDummyEvent(const char* name)
	: TGo4EventElement(name)
{}


TQClamDagobertDummyEvent::~TQClamDagobertDummyEvent()
{}


Int_t TQClamDagobertDummyEvent::Init()
{
	/* check for different source types */
	Int_t rev = 0;
	Clear();
	if(CheckEventSource("TQClamDagobertProviderProc"))
	{
		fxProviderProc = dynamic_cast<TQClamDagobertProviderProc*>(GetEventSource());
	}
	else
	{
		rev = 1;
	}

	return rev;
}


Int_t TQClamDagobertDummyEvent::Fill()
{
        /* check for different source types */
	Int_t rev = 0;
	if(fxProviderProc)
	{
		fxProviderProc->SetRealInput();
	}
	else
	{
		rev = 1;
	}

	return rev;
}



void TQClamDagobertDummyEvent::Clear(Option_t*)
{}
