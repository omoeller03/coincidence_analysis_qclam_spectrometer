#ifndef TQCLAMDISPLAYSETTINGPARAMETER_H
#define TQCLAMDISPLAYSETTINGPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"

class TQClamDisplaySettingParameter : public TGo4Parameter
{
	public:
		TQClamDisplaySettingParameter();
		TQClamDisplaySettingParameter(const char* name);

		
		//-----------------------------------------------------------
		// Histogramme/Graphen HitProc

		/** Trigger */
		Int_t RollGraphTriggerIntegrationWindow; // Rolling-Graph von Faraday-Cup-Current/Szint-/Cherenkov-/...-Trigger: Gibt an, wie viele Events integriert werden, bis der Graph geupdatet wird: Strom = dLadung(100xEvt)/dZeit(100xEvt)


		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamDisplaySettingParameter,1);
};


#endif
