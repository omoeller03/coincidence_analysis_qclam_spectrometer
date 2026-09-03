#include "geometry/TQClamGeometry.h"

#include "geometry/TQClamWirePlane.h"
#include "geometry/TQClamWire.h"

#include "parameter/TQClamDriftchamGeomParameter.h"

#include "TGo4Log.h"


TQClamGeometry::~TQClamGeometry() = default;


void TQClamGeometry::BuildDriftchamFromParameter(const TQClamDriftchamGeomParameter* par)
{
	driftcham = TQClamDriftcham(par->driftchamID, par->generation.Data());

	for(int i=0; i<par->nPlanes; i++)
	{
		TQClamWirePlane plane(
				par->planeType[i] == TQClamDriftchamGeomParameter::X ? WirePlaneType::X : WirePlaneType::U,
				par->zPos[i],
				par->xOffset[i],
				par->yOffset[i],
				par->xLength[i],
				par->yLength[i],
				par->wireDist[i],
				par->angleDeg[i] * M_PI/180.0, /** umrechnung in rad */
				par->nWires[i],
				i /** id */);

				driftcham.AddWirePlane(plane);

	TGo4Log::Info("GEOM-BUILD: layer=%d | zPos=%f | xOffset=%f | wireDist=%f",
              i,
              par->zPos[i],
              par->xOffset[i],
              par->wireDist[i]);
	}

}
