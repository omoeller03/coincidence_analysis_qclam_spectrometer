/*
    Data unpacker for the VME module: Struck SIS3302 Flash ADC.


    Copyright 2014 Maxim Singer

    License: MIT (https://opensource.org/licenses/MIT)

*/


#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <memory>
#include <cmath>
#include <algorithm>


class StruckSis3302Unpacker
{
public:
    StruckSis3302Unpacker();
    ~StruckSis3302Unpacker();

    class Sis3302DataFormat
    {
    public:
        Sis3302DataFormat()
        {
            timestamp = 0.0;
            startEnergyValue = 0;
            maxEnergyValue = 0;
            nSamples = 0;
            flags = 0;
            eventHeader = 0;
        }

        double timestamp;

        long startEnergyValue;  // (Energy value from first value of Trigger Gate )
        long maxEnergyValue;    // (during Trigger Gate active )


        unsigned long flags;

        unsigned short eventHeader;

        unsigned long nSamples;
        std::vector<unsigned short> samples;
    };

    // Zeit zwischen dem ersten und letzten Event in Sekunden
    uint32_t getMeasuringTime() const { return (lastTraceTimestamp-firstTraceTimestamp)*nanosecondsPerSample*1e-9;}

    int analyseFileFormat(const std::vector<uint32_t> &rawData,
                          size_t start,
                          size_t end,
                          std::array<std::vector<std::shared_ptr<Sis3302DataFormat> >, 8> &dataformatList);


 private:

    void init();

    bool firstScalerReadout;

    uint64_t firstTraceTimestamp;
    uint64_t lastTraceTimestamp;

    const int nanosecondsPerSample = 40;
    size_t nLastEventSamples;
};
