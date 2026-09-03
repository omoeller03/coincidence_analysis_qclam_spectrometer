#ifndef TQCLAMFOCALPLANEPARAMETER_H
#define TQCLAMFOCALPLANEPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"
#include "TVector3.h"

class TQClamFocalPlaneParameter : public TGo4Parameter
{
	public:
		TQClamFocalPlaneParameter();
		TQClamFocalPlaneParameter(const char* name);

		Bool_t QuadraticFocalPlane = kFALSE;
		Double_t a = 0.;
		Double_t b = 0.;
		Double_t c = 0.;

		TVector3 Project(const TVector3& pos, double phi, double theta) const; // const TVector3& dir) const;

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	private:
		Double_t FindIntersectionX(double xPos, double zPos, double phi) const; //const TVector3& pos, const TVector3& dir) const;
		Double_t QuadraticFocalFunction(Double_t xPos) const;

	ClassDefOverride(TQClamFocalPlaneParameter,1);
};

#endif
