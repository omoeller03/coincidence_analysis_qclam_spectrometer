#include "utils/TQClamTimeWalkLoader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

std::vector<TQClamTimeWalkLoader::TimeWalkParameters>
TQClamTimeWalkLoader::LoadSingleFile(const std::string& filename,
                                     int maxWire)
{
    std::vector<TimeWalkParameters> parameters(maxWire);

    std::ifstream file(filename);
    if(!file)
    {
        throw std::runtime_error(
            "[TimeWalkLoader] Konnte Datei nicht öffnen: " + filename);
    }

    std::string line;
    int filled = 0;

    while(std::getline(file, line))
    {
        if(line.empty())
            continue;

        std::stringstream ss(line);

        int wire;
        double p0, p1, p2;

        if(!(ss >> wire >> p0 >> p1 >> p2))
        {
            std::cerr
                << "[TimeWalkLoader] Ungültige Zeile ignoriert: "
                << line
                << std::endl;
            continue;
        }

        if(wire < 0 || wire >= maxWire)
        {
            std::cerr
                << "[TimeWalkLoader] Wire out of range ignoriert: "
                << wire
                << std::endl;
            continue;
        }

        parameters[wire].p0 = p0;
        parameters[wire].p1 = p1;
        parameters[wire].p2 = p2;

        filled++;
    }

    std::cout
        << "[TimeWalkLoader] Loaded "
        << filled
        << " parameter sets from "
        << filename
        << std::endl;

    return parameters;
}
