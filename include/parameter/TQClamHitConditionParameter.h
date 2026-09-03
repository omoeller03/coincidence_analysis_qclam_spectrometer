#ifndef TQCLAMHITCONDITIONPARAMETER_H
#define TQCLAMHITCONDITIONPARAMETER_H

#include "TGo4Parameter.h"
#include "TQClamConstants.h"
#include "Rtypes.h"

class TQClamHitConditionParameter : public TGo4Parameter
{
	public:
		TQClamHitConditionParameter();
		TQClamHitConditionParameter(const char* name);

		static const Int_t MAX_LAYER = QCLAM_MAX_LAYER;

		/** -- Konstanten fuer Condition-Modi
		  * wie soll mit Condition verfahren werden
		  * --> dies fuer jede Condition einzeln */
		static constexpr Int_t kOff = QCLAM_COND_OFF; // Condition-Mode 0: Condition hat keine Auswirkung
		static constexpr Int_t kFlagOnly = QCLAM_COND_FLAG_ONLY; // Condition-Mode 1:  Condition setzt Event-Flag accepted true/false (Default)
		static constexpr Int_t kFilter = QCLAM_COND_FILTER; // Condition-Mode 2:  Condition  verwirft Event


		/** time-over-threshold fenster */
		Double_t minToT_ns[MAX_LAYER]; // minimale time-over-threshold/ns (trailing - leading)
		Double_t maxToT_ns[MAX_LAYER]; // maximale time-over-threshold/ns (trailing - leading)
		Int_t ToTCondMode = kFlagOnly; // Condition-Mode ToT-Fenster: (minToT_ns, maxToT_ns)

		/** driftzeit fenster */
		Double_t minDrifttime_ns[MAX_LAYER]; // minimale Driftzeit/ns (nach Umrechnung: Offsets, invertiert...)
		Double_t maxDrifttime_ns[MAX_LAYER]; // maximale Driftzeit/ns (nach Umrechnung: Offsets, invertiert...)
		Int_t DrifttimeCondMode = kFlagOnly; // Condition-Mode Driftzeit-Fenster: (minDrifttime_ns, maxDrifttime_ns)

		/** bedingungen */
		Bool_t requireTdcTrig = kFALSE; // verwende nur Hits mit TDC-Trigger?
		Bool_t requireLead = kFALSE; // verwende nur Hits mit Leading-Flanke?
		Bool_t requireTrail = kFALSE; // verwende nur Hits mit Trailing-Flanke?

	

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


	ClassDefOverride(TQClamHitConditionParameter,1);
};

#endif
