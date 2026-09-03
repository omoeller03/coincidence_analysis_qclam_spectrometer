#ifndef TQCLAMBACKGROUNDRATEPROC_H
#define TQCLAMBACKGROUNDRATEPROC_H

#include "TGo4EventProcessor.h"

class TGo4RollingGraph;

class TQClamBackgroundRateProc : public TGo4EventProcessor
{
	public:
		TQClamBackgroundRateProc();
		TQClamBackgroundRateProc(const char* name);
		virtual ~TQClamBackgroundRateProc() {}


		Bool_t CheckEventClass(TClass* cl) override;
		Bool_t BuildEvent(TGo4EventElement* dest) override;


		/** Histogramme/Rollinggraphen */
		TGo4RollingGraph* fGEnton;
		TGo4RollingGraph* fGSzintScattChamber;



	ClassDefOverride(TQClamBackgroundRateProc,1);
};

#endif
