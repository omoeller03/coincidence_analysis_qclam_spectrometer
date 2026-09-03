#include "parameter/TQClamDriftToZOffsetParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamDriftToZOffsetParameter)

//////////////////////////////////////////////////////////////////////

TQClamDriftToZOffsetParameter::TQClamDriftToZOffsetParameter()
	: TGo4Parameter("DriftToZOffsetParam")
{}


//////////////////////////////////////////////////////////////////////

TQClamDriftToZOffsetParameter::TQClamDriftToZOffsetParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamDriftToZOffsetParameter] Create %s",name);

	/** alles nullen */
	Type = -1;

	Folder = "";


	for(size_t l=0; l<MAX_LAYER; l++)
	{
		Files[l] = "";

		NPhiBins[l] = 0;

		for(size_t p=0; p<MAX_PHI_BINS; p++)
		{
			PhiBins[l][p] = -1.;
			NDriftBins[l][p] = 0;
	
			for(size_t d=0; d<MAX_DRIFTTIME_BINS; d++)
			{
				DrifttimeBins[l][p][d] = -1.;
				ZOffsetTable[l][p][d] = -1;
			}
		}
	}
}






// ---------------------------------------------------
// Helper: Unsicherheiten Berechung des z-Offsets
inline double SafeSlope(double z0, double z1, double t0, double t1)
{
	if(std::abs(t1 - t0) < 1e-12) { return 0.0; }
	return (z1 - z0) / (t1 - t0);
}


inline double PropagateLinear(double dzdt, double sigma_t)
{
	return std::abs(dzdt) * sigma_t;
}


inline double sq(double x) { return x * x; }






/** Berechne zOffset
 * ... */
ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffset(const ZOffsetInput& in) const
{
	if(!fBinsBuilt)
	{
		BuildBinCounts();
	}


	switch(fInterpolationMode)
	{
		case 0: { return ComputeZOffsetNearest(in); }

		case 1: { return ComputeZOffsetSliceLinear(in); }

		case 2: { return ComputeZOffsetSliceLinearSafe(in); }

		case 3: { return ComputeZOffsetBilinear(in); }

		case 4: { return ComputeZOffsetBicubic(in); }

		case 5: { return ComputeZOffsetSpline(in); }


		default: { return ComputeZOffsetNearest(in); }

	}
}





/** nearest neighbor
 * */
ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetNearest(const ZOffsetInput& in) const
{
	ZOffsetResult out;

	int layer_index = -1;

	for(int k=0; k<MAX_LAYER; k++)
	{
		if(LayerIDs[k] == in.layerID)
		{
			layer_index = k;
			break;
		}
	}

	/** layerID existiert nicht
	 * -> gebe 'invalid' ZOffsetResult aus */
	if(layer_index < 0) { return out; }

	int i = FindNearestIndex(PhiBins[layer_index],NPhiBins[layer_index]/*MAX_PHI_BINS*/, in.phi);
	int j = FindNearestIndex(DrifttimeBins[layer_index][i], NDriftBins[layer_index][i]/*MAX_DRIFTTIME_BINS*/, in.drifttime);

	out.zOffset = ZOffsetTable[layer_index][i][j];
	out.sigma_zOffset = 0.;
	out.valid = true;

	return out;
}





ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetSliceLinear(const ZOffsetInput& in) const
{
	ZOffsetResult out;

	// -----------------------------
	// Layer finden
	// -----------------------------
	int layer_index = -1;
	for(int k=0; k<MAX_LAYER; k++)
	{
		if(LayerIDs[k] == in.layerID)
		{
			layer_index = k;
			break;
		}
	}

	if(layer_index < 0) { return out; }

    
	// -----------------------------
    
	// Phi: NEAREST (kein Interpolieren!)
    
	// -----------------------------
    
	int iphi = FindNearestIndex(
			PhiBins[layer_index],
			NPhiBins[layer_index],
			in.phi);

	if(iphi < 0 || iphi >= NPhiBins[layer_index]) { return out; }



	// -----------------------------
	// Driftzeit: lineare Interpolation
	// -----------------------------
	int id0 = FindLowerIndex(
			DrifttimeBins[layer_index][iphi],
			NDriftBins[layer_index][iphi],
			in.drifttime);

	int id1 = id0 + 1;

	if(id0 < 0 || id1 >= NDriftBins[layer_index][iphi]) { return ComputeZOffsetNearest(in); }

	double t0 = DrifttimeBins[layer_index][iphi][id0];
	double t1 = DrifttimeBins[layer_index][iphi][id1];

	double z0 = ZOffsetTable[layer_index][iphi][id0];
	double z1 = ZOffsetTable[layer_index][iphi][id1];

	// Schutz gegen kaputte bins
	if(std::abs(t1 - t0) < 1e-12) { return ComputeZOffsetNearest(in); }



	double dt = (t1 - t0);
	if(std::abs(dt) < 1e-12) { return ComputeZOffsetNearest(in); }

	double w = (in.drifttime - t0) / dt;

	double z = z0 + w * (z1 - z0);

	// --------- derivative dz/dt ----------
	double dzdt = (z1 - z0) / dt;

	// --------- error propagation ----------
	double sigma_t = in.sigma_drifttime;



/*
	double w = (in.drifttime - t0) / (t1 - t0);

	double z = z0 + w * (z1 - z0);
*/
	// -----------------------------
	// Output
	// -----------------------------
	out.zOffset = z;
	out.sigma_zOffset = PropagateLinear(dzdt, sigma_t);
	out.valid = true;

	return out;
}






ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetSliceLinearSafe(const ZOffsetInput& in) const
{
	ZOffsetResult out;

	// -----------------------------
	// Layer suchen
	// -----------------------------
	int layer = -1;
	for(int k=0; k<MAX_LAYER; k++)
	{
		if(LayerIDs[k] == in.layerID)
		{
			layer = k;
			break;
		}
	}

    	if(layer < 0) { return out; }
    	const size_t nphi = NPhiBins[layer];
	if(nphi == 0) { return out; }

	// -----------------------------
	// PHI: exakt wie Stable
	// -----------------------------
	int iphi = FindNearestIndex(PhiBins[layer], nphi, in.phi);

	if(iphi < 0 || iphi >= (int)nphi) { return ComputeZOffsetNearest(in); }

	// Sicherheitscheck (wichtig!)
	const size_t nd = NDriftBins[layer][iphi];
	if(nd < 2) { return ComputeZOffsetNearest(in); }

	// -----------------------------
	// DRIFT: lineare Interpolation (wie Stable)
	// -----------------------------
	int id0 = FindLowerIndex(PhiBins[layer] /* dummy */, 0, 0); // verhindert misuse
	id0 = FindLowerIndex(DrifttimeBins[layer][iphi], nd, in.drifttime);
	int id1 = id0 + 1;

	if(id0 < 0 || id1 >= (int)nd) { return ComputeZOffsetNearest(in); }

	double t0 = DrifttimeBins[layer][iphi][id0];
	double t1 = DrifttimeBins[layer][iphi][id1];

	double z0 = ZOffsetTable[layer][iphi][id0];
	double z1 = ZOffsetTable[layer][iphi][id1];

	if(std::abs(t1 - t0) < 1e-12) { return ComputeZOffsetNearest(in); }

	double w = (in.drifttime - t0) / (t1 - t0);

	// Clamp (wichtig gegen -1e-185 Effekte)
	if(w < 0) { w = 0; }
	if(w > 1) { w = 1; }

	double z = z0 + w * (z1 - z0);
	double dt = (t1 - t0);
	double dzdt = (z1 - z0) / dt;


	if(!std::isfinite(z)) { return ComputeZOffsetNearest(in); }

	out.zOffset = z;
	out.sigma_zOffset = std::abs(dzdt) * in.sigma_drifttime;
	out.valid = true;

	return out;
}




/*
ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetBilinear(const ZOffsetInput& in) const
{
    ZOffsetResult out;

    // -----------------------------
    // Layer finden
    // -----------------------------
    int layer = -1;
    for(int k=0; k<MAX_LAYER; k++)
    {
        if(LayerIDs[k] == in.layerID)
        {
            layer = k;
            break;
        }
    }

    if(layer < 0) return out;

    const size_t nphi = NPhiBins[layer];
    if(nphi < 2) return ComputeZOffsetSliceLinear(in);

    // -----------------------------
    // phi bracket
    // -----------------------------
    int iphi0 = FindLowerIndex(PhiBins[layer], nphi, in.phi);
    int iphi1 = iphi0 + 1;

    if(iphi0 < 0 || iphi1 >= (int)nphi)
        return ComputeZOffsetSliceLinear(in);

    double phi0 = PhiBins[layer][iphi0];
    double phi1 = PhiBins[layer][iphi1];

    if(std::abs(phi1 - phi0) < 1e-12)
        return ComputeZOffsetSliceLinear(in);

    double wphi = (in.phi - phi0) / (phi1 - phi0);
    if(wphi < 0) wphi = 0;
    if(wphi > 1) wphi = 1;

    // -----------------------------
    // helper: 1D drift interpolation
    // -----------------------------
    auto interpSlice = [&](int iphi) -> double
    {
        const size_t nd = NDriftBins[layer][iphi];
        if(nd < 2)
            return ZOffsetTable[layer][iphi][0];

        int j0 = FindLowerIndex(DrifttimeBins[layer][iphi], nd, in.drifttime);
        int j1 = j0 + 1;

        if(j0 < 0 || j1 >= (int)nd)
            return ZOffsetTable[layer][iphi][j0];

        double t0 = DrifttimeBins[layer][iphi][j0];
        double t1 = DrifttimeBins[layer][iphi][j1];

        double z0 = ZOffsetTable[layer][iphi][j0];
        double z1 = ZOffsetTable[layer][iphi][j1];

        if(std::abs(t1 - t0) < 1e-12)
            return z0;

        double wt = (in.drifttime - t0) / (t1 - t0);

        if(wt < 0) wt = 0;
        if(wt > 1) wt = 1;

        return z0 + wt * (z1 - z0);
    };

    // -----------------------------
    // evaluate 2 slices
    // -----------------------------
    double z0 = interpSlice(iphi0);
    double z1 = interpSlice(iphi1);

    // -----------------------------
    // final phi interpolation
    // -----------------------------
    double z = z0 + wphi * (z1 - z0);

    if(!std::isfinite(z))
        return ComputeZOffsetSliceLinear(in);

    out.zOffset = z;
    out.sigma_zOffset = 0.0;
    out.valid = true;

    return out;
}
*/

ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetBilinear(const ZOffsetInput& in) const
{
    ZOffsetResult out;

    int layer = -1;
    for(int k=0; k<MAX_LAYER; k++)
        if(LayerIDs[k] == in.layerID) layer = k;

    if(layer < 0) return out;

    const size_t nphi = NPhiBins[layer];
    if(nphi < 2) return ComputeZOffsetSliceLinear(in);

    int iphi0 = FindLowerIndex(PhiBins[layer], nphi, in.phi);
    int iphi1 = iphi0 + 1;

    if(iphi0 < 0 || iphi1 >= (int)nphi)
        return ComputeZOffsetSliceLinear(in);

    double phi0 = PhiBins[layer][iphi0];
    double phi1 = PhiBins[layer][iphi1];

    if(std::abs(phi1 - phi0) < 1e-12)
        return ComputeZOffsetSliceLinear(in);

    double wphi = (in.phi - phi0) / (phi1 - phi0);
    wphi = std::clamp(wphi, 0.0, 1.0);

    auto interpSlice = [&](int iphi) -> std::pair<double,double>
    {
        const size_t nd = NDriftBins[layer][iphi];
        if(nd < 2)
            return {ZOffsetTable[layer][iphi][0], 0.0};

        int j0 = FindLowerIndex(DrifttimeBins[layer][iphi], nd, in.drifttime);
        int j1 = j0 + 1;

        if(j0 < 0 || j1 >= (int)nd)
            return {ZOffsetTable[layer][iphi][j0], 0.0};

        double t0 = DrifttimeBins[layer][iphi][j0];
        double t1 = DrifttimeBins[layer][iphi][j1];

        double z0 = ZOffsetTable[layer][iphi][j0];
        double z1 = ZOffsetTable[layer][iphi][j1];

        double dt = t1 - t0;
        if(std::abs(dt) < 1e-12)
            return {z0, 0.0};

        double w = (in.drifttime - t0) / dt;
        w = std::clamp(w, 0.0, 1.0);

        double z = z0 + w * (z1 - z0);
        double dzdt = (z1 - z0) / dt;

        double sz = std::abs(dzdt) * in.sigma_drifttime;

        return {z, sz};
    };

    auto [z0, s0] = interpSlice(iphi0);
    auto [z1, s1] = interpSlice(iphi1);

    double z = z0 + wphi * (z1 - z0);

    double dz_dphi = (z1 - z0) / (phi1 - phi0);
    double s_phi = std::abs(dz_dphi) * in.sigma_phi;

    out.zOffset = z;
    out.sigma_zOffset = std::sqrt(
        sq((1 - wphi) * s0) +
        sq(wphi * s1) +
        sq(s_phi)
    );

    out.valid = true;
    return out;
}


/*
ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetBicubic(const ZOffsetInput& in) const
{
    ZOffsetResult out;

    // -------------------------
    // layer finden
    // -------------------------
    int layer_index = -1;
    for(int k=0; k<MAX_LAYER; k++)
    {
        if(LayerIDs[k] == in.layerID)
        {
            layer_index = k;
            break;
        }
    }

    if(layer_index < 0) return out;

    const int nphi = NPhiBins[layer_index];
    if(nphi < 4) return ComputeZOffsetBilinear(in);

    // -------------------------
    // phi index (zentral)
    // -------------------------
    int iphi = FindLowerIndex(PhiBins[layer_index], nphi, in.phi);

    // clamp für 4er Fenster
    iphi = std::clamp(iphi, 1, nphi - 3);

    // -------------------------
    // drift index (wir nehmen phi0 später)
    // -------------------------
    const int nd0 = NDriftBins[layer_index][iphi];
    if(nd0 < 4) return ComputeZOffsetBilinear(in);

    int idrift = FindLowerIndex(DrifttimeBins[layer_index][iphi], nd0, in.drifttime);
    idrift = std::clamp(idrift, 1, nd0 - 3);

    // -------------------------
    // lokale Koordinaten
    // -------------------------
    double phi0 = PhiBins[layer_index][iphi];
    double phi1 = PhiBins[layer_index][iphi+1];

    double t0 = DrifttimeBins[layer_index][iphi][idrift];
    double t1 = DrifttimeBins[layer_index][iphi][idrift+1];

    if(std::abs(phi1 - phi0) < 1e-12 || std::abs(t1 - t0) < 1e-12)
        return ComputeZOffsetBilinear(in);

    double tx = (in.phi - phi0) / (phi1 - phi0);
    double ty = (in.drifttime - t0) / (t1 - t0);

    // -------------------------
    // Catmull-Rom helper
    // -------------------------
    auto cubic = [](double p0, double p1, double p2, double p3, double t)
    {
        double a0 = -0.5*p0 + 1.5*p1 - 1.5*p2 + 0.5*p3;
        double a1 =  p0 - 2.5*p1 + 2.0*p2 - 0.5*p3;
        double a2 = -0.5*p0 + 0.5*p2;
        double a3 =  p1;
        return ((a0*t + a1)*t + a2)*t + a3;
    };

    // -------------------------
    // 4x4 patch laden
    // -------------------------
    double p[4][4];

    for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
    {
        int ii = iphi + i - 1;
        int jj = idrift + j - 1;

        ii = std::clamp(ii, 0, nphi-1);
        jj = std::clamp(jj, 0, NDriftBins[layer_index][ii]-1);

        p[i][j] = ZOffsetTable[layer_index][ii][jj];
    }

    // -------------------------
    // erst drift (y)
    // -------------------------
    double tmp[4];

    for(int i=0;i<4;i++)
    {
        tmp[i] = cubic(p[i][0], p[i][1], p[i][2], p[i][3], ty);
    }

    // -------------------------
    // dann phi (x)
    // -------------------------
    double z = cubic(tmp[0], tmp[1], tmp[2], tmp[3], tx);

    out.zOffset = z;
    out.valid = true;
    out.sigma_zOffset = 0.0;

    return out;
}
*/

ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetBicubic(const ZOffsetInput& in) const
{
    ZOffsetResult out;

    int layer = -1;
    for(int k=0; k<MAX_LAYER; k++)
        if(LayerIDs[k] == in.layerID) layer = k;

    if(layer < 0) return out;

    const int nphi = NPhiBins[layer];
    if(nphi < 4) return ComputeZOffsetBilinear(in);

    int iphi = FindLowerIndex(PhiBins[layer], nphi, in.phi);
    iphi = std::clamp(iphi, 1, nphi - 3);

    const int nd = NDriftBins[layer][iphi];
    if(nd < 4) return ComputeZOffsetBilinear(in);

    int id = FindLowerIndex(DrifttimeBins[layer][iphi], nd, in.drifttime);
    id = std::clamp(id, 1, nd - 3);

    auto eval = [&](double phi, double t)
    {
        ZOffsetInput tmp = in;
        tmp.phi = phi;
        tmp.drifttime = t;
        return ComputeZOffsetBilinear(tmp).zOffset;
    };

    double eps_phi = in.sigma_phi;
    double eps_t   = in.sigma_drifttime;

    double z0 = eval(in.phi, in.drifttime);
    double zp = eval(in.phi + eps_phi, in.drifttime);
    double zm = eval(in.phi - eps_phi, in.drifttime);

    double zt_p = eval(in.phi, in.drifttime + eps_t);
    double zt_m = eval(in.phi, in.drifttime - eps_t);

    double dz_dphi = 0.5 * (zp - zm);
    double dz_dt   = 0.5 * (zt_p - zt_m);

    out.zOffset = z0;
    out.sigma_zOffset = std::sqrt(
        sq(dz_dphi) + sq(dz_dt)
    );

    out.valid = true;
    return out;
}




/*
ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetSpline(const ZOffsetInput& in) const
{
    ZOffsetResult out;

    int layer_index = -1;
    for(int k=0;k<MAX_LAYER;k++)
        if(LayerIDs[k]==in.layerID)
            layer_index = k;

    if(layer_index < 0) return out;

    int nphi = NPhiBins[layer_index];
    if(nphi < 4) return ComputeZOffsetBilinear(in);

    int iphi = FindLowerIndex(PhiBins[layer_index], nphi, in.phi);
    iphi = std::clamp(iphi, 1, nphi - 3);

    const int nd = NDriftBins[layer_index][iphi];
    if(nd < 4) return ComputeZOffsetBilinear(in);

    int id = FindLowerIndex(DrifttimeBins[layer_index][iphi], nd, in.drifttime);
    id = std::clamp(id, 1, nd - 3);

    // norm coords
    double phi0 = PhiBins[layer_index][iphi];
    double phi1 = PhiBins[layer_index][iphi+1];

    double t0 = DrifttimeBins[layer_index][iphi][id];
    double t1 = DrifttimeBins[layer_index][iphi][id+1];

    if(std::abs(phi1-phi0)<1e-12 || std::abs(t1-t0)<1e-12)
        return ComputeZOffsetBilinear(in);

    double tx = (in.phi - phi0)/(phi1 - phi0);
    double ty = (in.drifttime - t0)/(t1 - t0);

    auto cubic = [](double p0,double p1,double p2,double p3,double t)
    {
        double a0 = -0.5*p0 + 1.5*p1 - 1.5*p2 + 0.5*p3;
        double a1 =  p0 - 2.5*p1 + 2.0*p2 - 0.5*p3;
        double a2 = -0.5*p0 + 0.5*p2;
        double a3 =  p1;
        return ((a0*t + a1)*t + a2)*t + a3;
    };

    double p[4][4];

    for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
    {
        int ii = iphi + i - 1;
        int jj = id + j - 1;

        ii = std::clamp(ii, 0, nphi-1);
        jj = std::clamp(jj, 0, NDriftBins[layer_index][ii]-1);

        p[i][j] = ZOffsetTable[layer_index][ii][jj];
    }

    double tmp[4];

    for(int i=0;i<4;i++)
        tmp[i] = cubic(p[i][0],p[i][1],p[i][2],p[i][3],ty);

    double z = cubic(tmp[0],tmp[1],tmp[2],tmp[3],tx);

    out.zOffset = z;
    out.valid = true;
    return out;
}
*/


ZOffsetResult TQClamDriftToZOffsetParameter::ComputeZOffsetSpline(const ZOffsetInput& in) const
{
    ZOffsetResult out;

    int layer = -1;
    for(int k=0;k<MAX_LAYER;k++)
        if(LayerIDs[k]==in.layerID) layer = k;

    if(layer < 0) return out;

    const int nphi = NPhiBins[layer];
    if(nphi < 4) return ComputeZOffsetBilinear(in);

    auto eval = [&](double phi, double t)
    {
        ZOffsetInput tmp = in;
        tmp.phi = phi;
        tmp.drifttime = t;
        return ComputeZOffsetBilinear(tmp).zOffset;
    };

    double eps_phi = in.sigma_phi;
    double eps_t   = in.sigma_drifttime;

    double z0 = eval(in.phi, in.drifttime);

    double dz_dphi = (eval(in.phi + eps_phi, in.drifttime)
                    - eval(in.phi - eps_phi, in.drifttime)) * 0.5;

    double dz_dt   = (eval(in.phi, in.drifttime + eps_t)
                    - eval(in.phi, in.drifttime - eps_t)) * 0.5;

    out.zOffset = z0;
    out.sigma_zOffset = std::sqrt(
        sq(dz_dphi) +
        sq(dz_dt)
    );

    out.valid = true;
    return out;
}













//--------------------------------------------------
// Hilfsmethoden
//--------------------------------------------------



/** Finde index in bins mit zu val betraglich naechstem wert in bins
 * => gebe naechsten Bin ohne Interpolation zurueck */
int TQClamDriftToZOffsetParameter::FindNearestIndex(const double* bins, size_t n, double val) const
{
	/** 1. leeres array */
	if(n == 0) { return 0; }

	/** 2. binaere suche: erstes element >= val */
	const double* it = std::lower_bound(bins,bins+n,val);

	/** 3. val kleiner als alle werte */
	if(it == bins) { return 0; }

	/** 4. val groesser als alle werte */
	if(it == bins + n) { return n-1; }

	/** 5. sonst: nachbarvergleich */
	size_t upperIdx = it - bins;
	size_t lowerIdx = upperIdx - 1;

	if(std::abs(bins[upperIdx] - val) < std::abs(bins[lowerIdx] - val)) { return upperIdx; }

	return lowerIdx;
}




/** Finde index in bins dessen wert eins kleiner als val ist
 * => gebe unteren Bin ohne Interpolation zurueck */
int TQClamDriftToZOffsetParameter::FindLowerIndex(const double* bins, size_t n, double val) const
{
	if(n < 2) { return 0; }

	/** binaere suche: erstes element >= val */
	const double* it = std::lower_bound(bins,bins+n,val);

	/** val kleiner als alle werte */
	if(it == bins) { return 0; }

	/** val groesser als alle werte */
	if(it == bins + n) { return n-2; }

	/** sonst: */
	return (it - bins) - 1;
}




/** Zaehle phi-bins und drifttime-bins */
void TQClamDriftToZOffsetParameter::BuildBinCounts() const
{
	TGo4Log::Info("[TQClamDriftToZOffsetParameter] BuildBinCounts().");

	for(int l = 0; l < MAX_LAYER; l++)
	{
		NPhiBins[l] = 0;

		for(int p = 0; p < MAX_PHI_BINS; p++)
		{
			if(PhiBins[l][p] >= 0) { NPhiBins[l]++; }
			else { break; }
		}

		for(int p = 0; p < NPhiBins[l]; p++)
		{
			NDriftBins[l][p] = 0;

			for(int d = 0; d < MAX_DRIFTTIME_BINS; d++)
			{
				if(DrifttimeBins[l][p][d] >= 0) { NDriftBins[l][p]++; }
				else { break; }
			}
		}
	}

	fBinsBuilt = true;
	TGo4Log::Info("[TQClamDriftToZOffsetParameter] BuildBinCounts() erfolgreich.");
}

/** Update... */
Bool_t TQClamDriftToZOffsetParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamDriftToZOffsetParameter] UpdateFrom().");

	auto other = static_cast<TQClamDriftToZOffsetParameter*>(rhs);

	if(!other)
	{
		TGo4Log::Error("[TQClamDriftToZOffsetParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	Type = other->Type;
	Folder = other->Folder;
	fInterpolationMode = other->fInterpolationMode;


	/*
	 * das macht die Korrektur kaputt. muesste file komplett neu einlesen, gaube ich, noch nicht implementiert...
	 * auch nicht unbedingt noetig, das stellt man einmal vorher ein...
	for(size_t l=0; l<MAX_LAYER; l++)
	{
		Files[l] = other->Files[l];
		for(size_t p=0; p<MAX_PHI_BINS; p++)
		{
			PhiBins[l][p] = other->PhiBins[l][p];

			for(size_t d=0; d<MAX_DRIFTTIME_BINS; d++)
			{
				DrifttimeBins[l][p][d] = other->DrifttimeBins[l][p][d];
				ZOffsetTable[l][p][d] = other->ZOffsetTable[l][p][d];
			}
		}
	}
	*/


	/** rebuild NPhiBins & NDriftBins */
	fBinsBuilt = kFALSE;

	return kTRUE;
}





