#ifndef TQCLAMHITPROCDEBUGOUTPUTPARAMETER_H
#define TQCLAMHITPROCDEBUGOUTPUTPARAMETER_H

#include "TGo4Parameter.h"

#include "Rtypes.h"

class TQClamHitProcDebugOutputParameter : public TGo4Parameter
{
	public:
		TQClamHitProcDebugOutputParameter();
		TQClamHitProcDebugOutputParameter(const char* name);

		Bool_t WriteRaw = kFALSE; // Rohdaten (input) des HitProcs in ein File schreiben?
		Bool_t WriteProcessed = kFALSE; // output-Daten des HitProcs in ein File schreiben?
		TString OutputFileRaw = "debug_output_hitproc_raw.txt"; // File-Name Rohdaten
		TString OutputFileProcessed = "debug_output_hitproc_processed.txt"; // File-Name output-Daten

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamHitProcDebugOutputParameter, 1);
};

#endif
