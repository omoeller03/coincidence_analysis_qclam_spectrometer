#include "parameter/TQClamSieveSlitCalibParameter.h"

#include "TGo4Log.h"

#include <algorithm>
#include <cmath>

ClassImp(TQClamSieveSlitCalibParameter)

//////////////////////////////////////////////////////////////////////

TQClamSieveSlitCalibParameter::TQClamSieveSlitCalibParameter() : TGo4Parameter("SieveSlitCalibParam")
{
	fBUseDefaultSieveSlitCalib = kTRUE;

	LoadDefaultCalibration();
}

//////////////////////////////////////////////////////////////////////

TQClamSieveSlitCalibParameter::TQClamSieveSlitCalibParameter(const char* name) : TGo4Parameter(name)
{
    
	TGo4Log::Info("[TQClamSieveSlitCalibParameter] Create %s",name);

    
	fBUseDefaultSieveSlitCalib = kTRUE;

    
	LoadDefaultCalibration();
}


void TQClamSieveSlitCalibParameter::LoadDefaultCalibration()
{
	Double_t th[kThetaPar] =
	{
		-3.597339501751406488e+00,
	       	1.993156889562384959e-03,
		6.537505743149746307e-02,
	       	1.809713837959300298e-01,
		-1.500587752567322699e-04,
		-1.316196462267098135e-04,
	       	3.912933367404677377e-03,
		2.681824710343194405e-06,
		3.419907047304524343e-07,
		-5.143080260125768542e-05,
		-1.791305368603258574e-03,
		8.856227953378753938e-09,
		-3.032795645527625641e-08,
		3.196346584319172074e-07,
		-6.738927568637568501e-06,
		1.901344601292656678e-06,
		-5.704358108844350610e-05,
		1.182511156420394802e-10,
		2.714609002334158031e-08,
		-9.948781338660779087e-06
	};

	std::copy(th, th+kThetaPar, TH);



	Double_t ph[kPhiPar] =
	{
		2.357073372082745166e+01,
		5.657738329995830838e-03,
		-3.885451115997917704e-03,
		-9.054962691332729685e-01,
		2.211440059590469907e-06,
		4.116536075649579902e-05,
		3.972165054070338642e-04,
		-8.212365994380817725e-08,
		-7.514641107544906407e-07,
		3.126916631557604808e-04,
		1.004001486062289922e-02,
		-2.257349373562700556e-09,
		5.063159375453163460e-08,
		-5.978614004190640757e-08,
		-1.771561350426200266e-05,
		-2.821881747833334729e-06,
		-2.835590878282193424e-06,
		-1.361710960508919699e-10,
		1.236836656718629984e-09,
		3.413156729543821560e-07,
		8.820576246436676733e-10,
		-1.291284093471089686e-06,
		-5.903610073704284808e-05
	};

	std::copy(ph, ph+kPhiPar, PH);





	Double_t tx[kThetaPar] =
	{
		-6.392046124954111264e+01,
		-2.425713378517035598e-02,
		-2.424237124546262390e-01,
		1.757243025277769100e+00,
		5.821481797561937595e-04,
		3.702494290344786240e-05,
		-1.695850957075008270e-02,
		-1.021340803888254740e-05,
		1.409345233352329232e-05,
		-1.197538153213426482e-03,
		-5.131823968674035716e-03,
		-2.625163375484250123e-08,
		-6.889692659345618266e-07,
		-9.332960226504590418e-07,
		1.057637037758363925e-04,
		9.004099591924959772e-06,
		2.349645694646528915e-04,
		2.148808965174288790e-11,
		-1.590561853197669612e-06,
		4.442167237424571769e-05
	};

	std::copy(tx, tx+kThetaPar, TX);





	Double_t ty[kPhiPar] =
	{
		-1.083648465300112775e+02,
		-1.369689290240560034e-02,
		2.695045777978113399e-01,
		4.380544738028256724e+00,
		-5.923278183193683450e-04,
		-8.338033963147348671e-04,
		1.358051716187182005e-02,
		1.074095387339303008e-05,
		5.889362888209646528e-06,
		-1.367343288712788082e-03,
		-4.866091496868329286e-02,
		4.421960980196127792e-08,
		-1.373853654371985302e-08,
		1.321184727733184437e-06,
		4.298985471153714635e-05,
		1.912224534282600875e-05,
		-2.099144626279779284e-04,
		1.148601969005969232e-09,
		-7.068831516793618847e-09,
		-1.239648429883924811e-06,
		-1.048613604779220523e-08,
		-3.302141744681291730e-05,
		2.505510827144676609e-04
	};

	std::copy(ty, ty+kPhiPar, TY);
}







Double_t TQClamSieveSlitCalibParameter::EvalThetaPolynomial(
		const Double_t* p,
		double x,
		double y,
		double phi) const
{
	return

		p[0]

		+ x   * p[1]
		+ y   * p[2]
		+ phi * p[3]

		+ x*y   * p[4]
		+ x*phi * p[5]
		+ y*phi * p[6]

		+ x*y*phi * p[7]

		+ x*x     * p[8]
		+ y*y     * p[9]
		+ phi*phi * p[10]

		+ x*x*y   * p[11]
		+ x*x*phi * p[12]
		+ y*y*x   * p[13]
		+ y*y*phi * p[14]
		+ phi*phi*x * p[15]
		+ phi*phi*y * p[16]

		+ x*x*y*y * p[17]
		+ y*y*phi*phi * p[18]

		+ y*y*y * p[19];
}




Double_t TQClamSieveSlitCalibParameter::EvalPhiPolynomial(
		const Double_t* p,
		double x,
		double y,
		double phi) const
{
	return
		p[0]

		+ x   * p[1]
		+ y   * p[2]
		+ phi * p[3]

		+ x*y   * p[4]
		+ x*phi * p[5]
		+ y*phi * p[6]

		+ x*y*phi * p[7]

		+ x*x     * p[8]
		+ y*y     * p[9]
		+ phi*phi * p[10]

		+ x*x*y   * p[11]
		+ x*x*phi * p[12]
		+ y*y*x   * p[13]
		+ y*y*phi * p[14]
		+ phi*phi*x * p[15]
		+ phi*phi*y * p[16]

		+ x*x*y*y     * p[17]
		+ x*x*phi*phi * p[18]
		+ y*y*phi*phi * p[19]

		+ x*x*x * p[20]
		+ y*y*y * p[21]
		+ phi*phi*phi * p[22];
}




void TQClamSieveSlitCalibParameter::TransformToScattCoords(
		double x,
		double y,
		double phi,
		double& phi_scatt,
		double& theta_scatt) const
{
	if(!fBUseDefaultSieveSlitCalib || !std::isfinite(y))
	{
		phi_scatt = NAN;
		theta_scatt = NAN;
		return;
	}

	theta_scatt = EvalThetaPolynomial(TH,x,y,phi);
	phi_scatt   = EvalPhiPolynomial(PH,x,y,phi);
}





void TQClamSieveSlitCalibParameter::TransformToTargetCoords(
		double x,
		double y,
		double phi,
		double& xTarget,
		double& yTarget) const
{
	if(!fBUseDefaultSieveSlitCalib || !std::isfinite(y))
	{
		xTarget = NAN;
		yTarget = NAN;
		return;
	}

	xTarget = EvalThetaPolynomial(TX,x,y,phi);
	yTarget = EvalPhiPolynomial(TY,x,y,phi);
}




Bool_t TQClamSieveSlitCalibParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamSieveSlitCalibParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamSieveSlitCalibParameter*>(rhs);

	if(!other)
	{
		TGo4Log::Error("[TQClamSieveSlitCalibParameter] UpdateFrom failed!");
		return kFALSE;
	}

	fBUseDefaultSieveSlitCalib = other->fBUseDefaultSieveSlitCalib;

	std::copy(other->TH,other->TH+kThetaPar,TH);

	std::copy(other->PH,other->PH+kPhiPar,PH);

	std::copy(other->TX,other->TX+kThetaPar,TX);

	std::copy(other->TY,other->TY+kPhiPar,TY);

	return kTRUE;
}
