#include "utils/TQClamTimeOffsetLoader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

std::vector<double> TQClamTimeOffsetLoader::LoadSingleFile(const std::string& filename, int maxWire)
{
	std::vector<double> offsets(maxWire, 0.0);

	std::ifstream file(filename);
	if(!file)
	{
		throw std::runtime_error("[TimeOffsetLoader] Konnte Datei nicht öffnen: " + filename);
	}

	std::string line;
	int filled = 0;

	while(std::getline(file, line))
	{
		if(line.empty()) { continue; }

		std::stringstream ss(line);

		int wire;
		double offset;

		if(!(ss >> wire >> offset))
		{
			std::cerr << "[TimeOffsetLoader] Ungültige Zeile ignoriert: " << line << std::endl;
			continue;
		}

		if(wire < 0 || wire >= maxWire)
		{
			std::cerr << "[TimeOffsetLoader] Wire out of range ignoriert: " << wire << std::endl;
			continue;
		}

		offsets[wire] = offset;
		filled++;
	}

	std::cout
		<< "[TimeOffsetLoader] Loaded "
		<< filled
		<< " offsets from "
		<< filename
		<< std::endl;

	return offsets;
}
