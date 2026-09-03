#ifndef TQCLAMTIMEWALKLOADER_H
#define TQCLAMTIMEWALKLOADER_H

#include <string>
#include <vector>

class TQClamTimeWalkLoader
{
	public:

		struct TimeWalkParameters
		{
			double p0 = 0.0;
			double p1 = 0.0;
			double p2 = 0.0;
		};

		static std::vector<TimeWalkParameters> LoadSingleFile(const std::string& filename, int maxWire);
};

#endif
