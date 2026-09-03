#include "parameter/TQClamDriftchamGeomParameter.h"

//#include "parameter/TQClamPlaneGeomParameter.h"
#include "TGo4Log.h"

ClassImp(TQClamDriftchamGeomParameter)

//////////////////////////////////////////////////////////////////////

TQClamDriftchamGeomParameter::TQClamDriftchamGeomParameter()
	: TGo4Parameter("DriftchamGeomParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamDriftchamGeomParameter::TQClamDriftchamGeomParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamDriftchamGeomParameter] Create %s", name);
}

//////////////////////////////////////////////////////////////////////


Bool_t TQClamDriftchamGeomParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamDriftchamGeomParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamDriftchamGeomParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamDriftchamGeomParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}


	for(int i = 0; i < MAX_LAYER; i++)
	{
		planeName[i] = other->planeName[i];
		planeType[i] = other->planeType[i];
		nWires[i]    = other->nWires[i];
		zPos[i]      = other->zPos[i];
		xOffset[i]   = other->xOffset[i];
		yOffset[i]   = other->yOffset[i];
		xLength[i]   = other->xLength[i];
		yLength[i]   = other->yLength[i];
		wireDist[i]  = other->wireDist[i];
		angleDeg[i]  = other->angleDeg[i];
	}

	return kTRUE;
}


/*
void TQClamDriftchamGeomParameter::InitPlanes(int n)
{
	planes.clear();
	planes.reserve(n);
	for(int i=0; i<n; i++)
	{
		planes.push_back(new TQClamPlaneGeomParameter(Form("Plane%d", i)));
	}
	nPlanes = n;
}
*/
