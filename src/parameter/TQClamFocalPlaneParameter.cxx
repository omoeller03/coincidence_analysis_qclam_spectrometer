#include "parameter/TQClamFocalPlaneParameter.h"

#include "TGo4Log.h"


ClassImp(TQClamFocalPlaneParameter)

//////////////////////////////////////////////////////////////////////

TQClamFocalPlaneParameter::TQClamFocalPlaneParameter()
	: TGo4Parameter("FocalPlaneParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamFocalPlaneParameter::TQClamFocalPlaneParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamFocalPlaneParameter] Create %s.",name);

	QuadraticFocalPlane = kFALSE;
	a = 0.;
	b = 0.;
	c = 0.;
}



Bool_t TQClamFocalPlaneParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamFocalPlaneParameter] UpdateFrom().");
	auto* other = static_cast<TQClamFocalPlaneParameter*>(rhs);

	if(!other)
	{
		TGo4Log::Error("[TQClamFocalPlaneParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	QuadraticFocalPlane = other->QuadraticFocalPlane;
	a = other->a;
	b = other->b;
	c = other->c;


	return kTRUE;
}


static bool IsMultipleOfPi(double phi)
{
	return std::abs(std::fmod(phi,M_PI)) < 1.e-12;
}


// ***********************************************************
// Finde Schnittpunkt zwischen Fokalebene und gefitteten Track (Gerade) mit:
// (xPos,zPos) = Position des Tracks in X1-Ebene
// phi = Winkel, tan(phi) = dz/dx
Double_t TQClamFocalPlaneParameter::FindIntersectionX(double xPos, double zPos, double phi) const //const TVector3& pos, const TVector3& dir) const
{
	/*
	Double_t x0 = pos.X();
	Double_t z0 = pos.Z();

	Double_t dx = dir.X();
	Double_t dz = dir.Z();
	*/

	double x0 = xPos;
	double z0 = zPos;
	double m = -std::atan(phi); //dz/dx;


	//if(std::abs(dx) == 0) { return x0; }
	if(IsMultipleOfPi(phi)) { return x0; }
	/**
	 z(x) = a + b x + c x^2
	 m (x - x0) + z0 = a + b x + c x^2
	 => c x^2 + (b - m)x + (a - z0 + m x0) = 0
	 => A x^2 + B x + C = 0
	 => (x + B/(2 A))^2 = -C/A + B^2 / (2 A)^2
	 => x = +/- sqrt( - C/A + B^2/(2 A)^2 ) - B/(2 A)
	      = 1/(2 A)  (-B +/- sqrt(B^2 - 4 A C))
	      = 1/(2 A) (-B +/- sqrt(D))
	*/

	double A = c;
	double B = b - m;
	double C = a - z0 + m * x0;

	if(std::abs(A) < 1e-12)
	{
		/**
		 Fokalebene effektiv linear: z = a + b x
		 m (x - x0) + z0 = a + b x
		 => (m - b)x = a - z0 + m x0
		*/
		double denom = (m - b);
		if(std::abs(denom) < 1e-12) return x0; /** nahezu parallel */
		return (a - z0 + m * x0) / denom;
	}

	double D = B*B - 4*A*C;
	/** kein reeller Schnitt -> fallback */
	if(D < 0) { return x0; }

	double sqrtD = std::sqrt(D);
	double x1 = (-B + sqrtD) / (2*A);
	double x2 = (-B - sqrtD) / (2*A);

	// Steigung der Geraden
	bool rising = (m > 0);

	// Parabelöffnung
	bool opensUp = (A > 0);

	// parabel nach unten geöffnet (c < 0)
	if(!opensUp)
	{
		/** Gerade steigt -> physikalischer Schnittpunkt rechts */
		if(rising) { return std::max(x1, x2); }
		/** Gerade fällt -> physikalischer Schnittpunkt links */
		else { return std::min(x1, x2); }
	}

	/** Parabel nach oben geöffnet (c > 0) */
	else
	{
		/** Gerade steigt -> physikalischer Schnittpunkt links */
		if(rising) { return std::min(x1, x2); }
		/** Gerade fällt -> physikalischer Schnittpunkt rechts */
		else { return std::max(x1, x2); }
	}
}
/*
	double t1 = (x1 - x0) / dx;
	double t2 = (x2 - x0) / dx;

	bool ok1 = t1 > 0;
	bool ok2 = t2 > 0;

	if(ok1 && !ok2) return x1;
	if(ok2 && !ok1) return x2;

	// Falls beide >0 oder beide <0 -> nimm den kleineren positiven t
	if(ok1 && ok2) return (t1 < t2 ? x1 : x2);

	// Falls beide <0 -> nimm den größeren (am wenigsten negativen)
	return (t1 > t2 ? x1 : x2);


}
*/

/*
Double_t TQClamFocalPlaneParameter::FindIntersectionX(const TVector3& pos, const TVector3& dir) const
{
	Double_t x0 = pos.X();
	Double_t y0 = pos.Y();
	Double_t z0 = pos.Z();

	Double_t dx = dir.X();
	Double_t dy = dir.Y();
	Double_t dz = dir.Z();

//	TGo4Log::Info("dx=%f",dx);

	if(std::abs(dx) == 0) { return x0; }

	double m = dz/dx;

        // fokalebene: z(x) = a + b*x + c*x*x
        // m * (x-x0) + z0 = a + b*x + c*x*x
        // 0 = (a - z0 + m*x0) + x*(b-m) + c*x*x
        // 0 = (a - z0 + m*x0)/c + x*(b-m)/c + x*x
        // 0 = (a - z0 + m*x0)/c + (x + (b-m)/(2*c))**2 - ((b-m)/(2*c))**2
        // (x + (b-m)/(2*c))**2 = ((b-m)/(2*c))**2 - (a - z0 + m*x0)/c
        // x = +-sqrt( ((b-m)/(2*c))**2 - (a - z0 + m*x0)/c ) - (b-m)/(2*c)


	double par = (b-m)/(2*c);

	double x_1 = +std::sqrt( par*par - (a - z0 + m*x0) / c) -par;
	double x_2 = -std::sqrt( par*par - (a - z0 + m*x0) / c) - par;

//	 Fokalebene nach unten geoffnet (negative z) 
	if(c>0)
	{
		// vergleiche rtg-vektoren
		if(m>0) { return std::max(x_1,x_2); }

		// m < 0 
		return std::min(x_1,x_2);
	}

	// parabel nach unten --> c>0 .. eigentlich nicht
	if(m>0) { return std::min(x_1,x_2); }

	// m<0 
	return std::max(x_1,x_2);
}
*/



Double_t TQClamFocalPlaneParameter::QuadraticFocalFunction(Double_t xPos) const
{
	/** berechne zPos auf Fokalebene fuer gegebenes xPos, xPos/mm angeben */
	return a + xPos * (b + c * xPos);
}


// phi in rad
TVector3 TQClamFocalPlaneParameter::Project(const TVector3& pos, double phi, double theta) const //const TVector3& dir) const
{
	double x_hit = FindIntersectionX(pos.X(), pos.Z(), phi); //pos,dir);

//	TGo4Log::Info("x0=%f, dx=%f, dz=%f, x_hit=%f", pos.X(), dir.X(), dir.Z(), x_hit);

	double z_hit = QuadraticFocalFunction(x_hit);
	//double y_hit = pos.Y() + (dir.Y()/dir.X()) * (x_hit - pos.X());
	double y_hit = pos.Y();

	return TVector3(x_hit,y_hit,z_hit);
}




