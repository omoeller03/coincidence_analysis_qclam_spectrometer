#ifndef TQCLAMFARADAYCUPCURRENTCALIBRATIONPARAMETER_H
#define TQCLAMFARADAYCUPCURRENTCALIBRATIONPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"

class TQClamFaradayCupCurrentCalibrationParameter : public TGo4Parameter
{
	public:
		TQClamFaradayCupCurrentCalibrationParameter();
		TQClamFaradayCupCurrentCalibrationParameter(const char* name);

		/** parameter */
		TString description=""; // current -> <currentCorr> = <invert> * (<scalefactorCurrent> *Ladung/Zeit  + <offsetCurrent>) --> Default: Ladung/fC, Zeit/µs
		Double_t offsetCurrent; // offsetCurrent (see description)
		Double_t scalefactorCurrent; // scalefactorCurrent (see description)
		Bool_t invert; // invert: kTRUE --> *(+1) || kFALSE --> *(-1) (see description)

		inline Double_t CorrectCurrent(double current) const
		{
			return invert ? (-1)*(scalefactorCurrent*current + offsetCurrent)
				: scalefactorCurrent*current + offsetCurrent;
		}

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamFaradayCupCurrentCalibrationParameter,1);
};

#endif
