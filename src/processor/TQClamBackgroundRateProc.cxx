#include "processor/TQClamBackgroundRateProc.h"

#include "evt/TQClamTcpEvent.h"
#include "evt/TQClamBackgroundRateEvent.h"

#include "TGo4Log.h"
#include "TGo4RollingGraph.h"

#include "TClass.h"


ClassImp(TQClamBackgroundRateProc)


TQClamBackgroundRateProc::TQClamBackgroundRateProc()
	: TGo4EventProcessor("BackgroundRateProc")
{}


TQClamBackgroundRateProc::TQClamBackgroundRateProc(const char* name)
	: TGo4EventProcessor(name)
{
	TGo4Log::Info("[TQClamBackgroundRateProc] Create %s.",name);


	fGEnton = MakeRollingGraph("EntonRate","Enton Rate",10000);
	fGSzintScattChamber = MakeRollingGraph("SzintScattChamberRate","Szint Streukammer Rate",10000);
}



Bool_t TQClamBackgroundRateProc::CheckEventClass(TClass* cl)
{
	return cl->InheritsFrom(TQClamBackgroundRateEvent::Class());
}



Bool_t TQClamBackgroundRateProc::BuildEvent(TGo4EventElement* dest)
{
	auto* evt = dynamic_cast<TQClamBackgroundRateEvent*>(dest);
	auto* in = dynamic_cast<TQClamTcpEvent*>(GetInputEvent());

	if(!in)
	{
		TGo4Log::Error("[TQClamBackgroundProc] Fehler mit In-/Ouput-Event!");
		return kFALSE;
	}


	evt->fEntonRate = in->fEntonRate;
	evt->fSzintScattChamberRate = in->fSzintScattChamberRate;

	fGEnton->Fill(evt->fEntonRate);
	fGSzintScattChamber->Fill(in->fSzintScattChamberRate);



	return kTRUE;
}
