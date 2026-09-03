/*
    Data unpacker for the VME module: CAEN V560 Scaler.


    Copyright 2014 Maxim Singer

    License: MIT (https://opensource.org/licenses/MIT)

*/



#pragma once

#include <array>
#include <vector>
#include <iostream>

class CaenV560Unpacker
{
public:
    CaenV560Unpacker();
    ~CaenV560Unpacker();

    bool analyseFileFormat(const std::vector<uint32_t> &rawData,
                                                        size_t start,
                                                        size_t end);

    void reset();

    uint32_t getCounter(uint32_t channel) const;
    uint64_t getCounter64(uint32_t channel) const;

    uint32_t getOldCounter(uint32_t channel) const;
    uint64_t getOldCounter64(uint32_t channel) const;

    float getFrequency(uint32_t channel);
    float getFrequency64(uint32_t channel);

    void setClockChannel(uint32_t channel, double period);

    double getClockPeriod() const { return clockPeriod; }

    size_t analyseCalls() const { return nAnalyseCalls; }

private:
    std::array<uint32_t, 16> counters;
    std::array<uint32_t, 16> countersOld;

    uint8_t clockChannel;
    double clockPeriod;
    uint64_t nAnalyseCalls = 0;
};
