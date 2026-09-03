#ifndef TQCLAMGEOMETRY_H
#define TQCLAMGEOMETRY_H

#include "geometry/TQClamDriftcham.h"

class TQClamDriftchamGeomParameter;

class TQClamGeometry
{
	public:
		static TQClamGeometry& Instance()
		{
			static TQClamGeometry inst;
			return inst;
		}

		~TQClamGeometry();


		void BuildDriftchamFromParameter(const TQClamDriftchamGeomParameter* par);

		const TQClamDriftcham& GetDriftcham() const
		{
			return driftcham;
		}

	private:
		TQClamGeometry() = default;

		TQClamDriftcham driftcham {3, "3Gen"};
};

#endif
