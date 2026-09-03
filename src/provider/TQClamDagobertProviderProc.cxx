/*
 * adaptiert aus go4-bsp TGo4ExampleMesh von JAM
 */
#include "provider/TQClamDagobertProviderProc.h"


#include "TGo4Log.h"
#include "TGo4Analysis.h"
#include "TGo4EventElement.h"
#include "TGo4UserException.h"


ClassImp(TQClamDagobertProviderProc)


TQClamDagobertProviderProc::TQClamDagobertProviderProc(const char* name)
	: TGo4EventProcessor(name)
{
	TGo4Log::Info("[TQClamDagobertProvider] Create %s.",name);
}

TQClamDagobertProviderProc::~TQClamDagobertProviderProc()
{}



void TQClamDagobertProviderProc::SetRealInput()
{
	TGo4EventElement* myinput = GetInputEvent();
	if(!myinput || myinput->CheckEventSource("TGo4EventProcessor"))
	{
		/**
		 * source des input-events ist ein prozessor, kein file/stream:
		 * --> resette die framework-funktion, dass das vorherige event als input dient
		 * --> noetig fuer nicht-lineare analysen (mehrere branches)
		 * trick:
		 * prozessor hat namen von event, das ich mir holen moechte + '_<beliebig>': 
		 * prozessorname = <inputevent>_<beliebig>
		 * --> hole mir input event
		 * --> ueberschreibe input event des naechsten steps
		 */
		TString eventname = GetName();
		eventname.Resize(eventname.Index("_",1));
		myinput = TGo4Analysis::Instance()->GetEventStructure(eventname.Data());
		if(!myinput)
		{
			throw TGo4UserException(3,"[TQClamDagobertProviderProc] FEHLER in 'SetRealInput()' des input-events %s",GetName());
		}
		else { SetInputEvent(myinput); }
	}
   // else if(myinput && myinput->CheckEventSource("TGo4FileSource"))
   //    {
   //       // this provider reads from file. ok
   //
   //    }
   // else
   //    {
   //       // neither processor or file input: something is very wrong
   //       throw TGo4UserException(3,"No source found for provider %s", GetName());
   //    }
   //  we skip the last checks for performance reasons...
}
