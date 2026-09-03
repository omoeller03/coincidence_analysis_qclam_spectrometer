#ifndef TQCLAMTIMEOFFSETLOADER_H
#define TQCLAMTIMEOFFSETLOADER_H

#include <vector>
#include <string>

class TQClamTimeOffsetLoader
{
	public:

		static std::vector<double> LoadSingleFile(const std::string& filename,int maxWire);
};

#endif
