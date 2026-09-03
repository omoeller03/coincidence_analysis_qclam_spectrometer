#ifndef TQCLAMUNPACKPARAMETER_H
#define TQCLAMUNPACKPARAMETER_H

#include "TGo4Parameter.h"

class TQClamUnpackParameter : public TGo4Parameter
{
	public:
		TQClamUnpackParameter();
		TQClamUnpackParameter(const char* name);

		TString fFolderTdcCalibration;

		Bool_t fTimeInterpolationElbVfb6_Random = kTRUE;
		Bool_t fTimeInterpolationElbVfb6_Linear = kFALSE;
		Bool_t fTimeInterpolationElbVfb6_None = kFALSE;




		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamUnpackParameter,1);
};


#endif
