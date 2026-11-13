#include "parameter/TQClamImageMatrixParameter.h"

#include "TGo4Log.h"


ClassImp(TQClamImageMatrixParameter)

//////////////////////////////////////////////////////////////////////

TQClamImageMatrixParameter::TQClamImageMatrixParameter() : TGo4Parameter("ImageMatrixParam")
{
	TGo4Log::Info("[TQClamImageMatrixParameter] Create ImageMatrixParam.");



	// Setze werte aus Diss Knirsch
	xx = -0.492; // (X|X) mm/mm
	xtheta = 0.000 / mradToDeg; // (X|PHI) cm/mrad (phi und theta vertauscht)
	xdelta = 23.22; // (X|delta) mm/%


	phix = -2.3073 * mradToDeg; // (Theta|X) mrad/mm
	phiphi = -2.022; // (Theta|Theta) mrad/mrad
	phidelta = 9.419 * mradToDeg; // (Theta|delta) mrad/%


	yy = -2.942; // (Y|Y) mm/mm
	ytheta = 0.07 * mradToDeg; // (Y|Phi) mm/mrad


	thetay = -1.1390 * mradToDeg; // (Theta|y) mrad/mm
	thetatheta = -0.305; // (Theta|Theta) mrad/mrad
	


}



//////////////////////////////////////////////////////////////////////


TQClamImageMatrixParameter::TQClamImageMatrixParameter(const char* name) : TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamImageMatrixParameter] Create %s.",name);



	// Setze werte aus Diss Knirsch
	xx = -0.492; // (X|X) mm/mm
	xtheta = 0.000 / mradToDeg; // (X|PHI) cm/mrad (phi und theta vertauscht)
	xdelta = 23.22; // (X|delta) mm/%


	phix = -2.3073 * mradToDeg; // (Theta|X) mrad/mm
	phiphi = -2.022; // (Theta|Theta) mrad/mrad
	phidelta = 9.419 * mradToDeg; // (Theta|delta) mrad/%


	yy = -2.942; // (Y|Y) mm/mm
	ytheta = 0.07 * mradToDeg; // (Y|Phi) mm/mrad


	thetay = -1.1390 * mradToDeg; // (Theta|y) mrad/mm
	thetatheta = -0.305; // (Theta|Theta) mrad/mrad
	



}



Bool_t TQClamImageMatrixParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamImageMatrixParameter] UpdateFrom()");
	auto* other = static_cast<TQClamImageMatrixParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamImageMatrixParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}
	
	xx = other->xx;
	xtheta = other->xtheta;
	xdelta = other->xdelta;

	phix = other->phix;
	phiphi = other->phiphi;
	phidelta = other->phidelta;

	yy = other->yy;
	ytheta = other->ytheta;

	thetay = other->thetay;
	thetatheta = other->thetatheta;


	return kTRUE;
}
