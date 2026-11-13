#ifndef TQCLAMIMAGEMATRIXPARAMETER_H
#define TQCLAMIMAGEMATRIXPARAMETER_H

#include "TGo4Parameter.h"

class TQClamImageMatrixParameter : public TGo4Parameter
{
	public:
		TQClamImageMatrixParameter();
		TQClamImageMatrixParameter(const char* name);

	private:
		const Double_t mradToDeg = 1e-3 * 180. / M_PI;

	public:

		/// Abbildungsmatrix-Parameter
		// Werte aus Diss Knirsch
		//
		// Achtung: in Diss-Knirsch sind PHI und THETA vertauscht (aber sowohl Targte- als auch Bildkoordinate)


		// 1) x
		// x <- (X|X) * x + (X|PHI) * phi + (x|delta) * delta
		// --> Delta x_target ~ sigma_x / |(X|X)|
		//
		// eigentlich: 
		// Delta x_target ~ 1/(X|X)^2 * (sigma_x^2 + ((X|PHI)*sigma_PHI)^2 + ((X|delta)*sigma_delta)^2 )
		Double_t xx = -0.492; // (X|X) mm/mm
		Double_t xtheta = 0.000 / mradToDeg; // (X|PHI) cm/mrad (phi und theta vertauscht)
		Double_t xdelta = 23.22; // (X|delta) mm/%


		// 2) phi
		// phi = (Phi|X) * x + (Phi|Phi) * Phi + (Phi|delta) * delta
		Double_t phix = -2.3073 * mradToDeg; // (Theta|X) mrad/mm
		Double_t phiphi = -2.022; // (Theta|Theta) mrad/mrad
		Double_t phidelta = 9.419 * mradToDeg; // (Theta|delta) mrad/%


		/// 3) y
		// y <- (Y|Y) * y + (Y|Theta) * theta
		// --> Delta y_target ~ sigma_y / |(Y|Y)|
		Double_t yy = -2.942; // (Y|Y) mm/mm
		Double_t ytheta = 0.07 * mradToDeg; // (Y|Phi) mm/mrad


		// 4) theta
		// theta = (Theta|y) * y + (Theta|Theta) * theta
		Double_t thetay = -1.1390 * mradToDeg; // (Theta|y) mrad/mm
		Double_t thetatheta = -0.305; // (Theta|Theta) mrad/mrad
	


		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamImageMatrixParameter,1);
};


#endif
