/*
    Data unpacker for the VME module: CAEN V560 Scaler.


    Copyright 2014 Maxim Singer

    License: MIT (https://opensource.org/licenses/MIT)

*/



#include "caen_v560_unpacker.h"


CaenV560Unpacker::CaenV560Unpacker()
{
    reset();
}

CaenV560Unpacker::~CaenV560Unpacker()
{

}

bool CaenV560Unpacker::analyseFileFormat(const std::vector<uint32_t> &rawData,
                                                                size_t start,
                                                                size_t end)
{
    if(rawData.size()==0)
    {
        return false;
    }

    countersOld = counters; // save counter state from last call

    for(size_t di = start, i = 0; di < end && di < rawData.size(); di++,i++)
    {
        this->counters[i] = rawData.at(di);
    }

    nAnalyseCalls++;

    return true;
}

void CaenV560Unpacker::reset()
{
    counters.fill(0);
    countersOld.fill(0);
    nAnalyseCalls = 0;
}

uint32_t CaenV560Unpacker::getCounter(uint32_t channel) const
{
    if(channel > 15)
    {
        std::cout << "V560Unpacker::getCounter:  channel > 15. return channel 15." << std::endl;
        channel = 15;
    }

    return counters[channel];
}

uint64_t CaenV560Unpacker::getCounter64(uint32_t channel) const
{
    if(channel%2 == 0)
    {
        std::cout << "V560Unpacker::getCounter64:  channel%2 == 0." << std::endl;
        channel = 15;
    }
    if(channel > 15)
    {
        std::cout << "V560Unpacker::getCounter64:  channel > 15. return channel 15." << std::endl;
        channel = 15;
    }

    uint64_t c = counters[channel-1];
    c = c << 32;

    return c + static_cast<uint64_t>(counters[channel]);
}

uint32_t CaenV560Unpacker::getOldCounter(uint32_t channel) const
{
    if(channel > 15)
    {
        std::cout << "V560Unpacker::getOldCounter:  channel > 15. return channel 15." << std::endl;
        channel = 15;
    }

    return countersOld[channel];
}

uint64_t CaenV560Unpacker::getOldCounter64(uint32_t channel) const
{
    if(channel%2 == 0)
    {
        std::cout << "V560Unpacker::getOldCounter64:  channel%2 == 0." << std::endl;
        channel = 15;
    }
    if(channel > 15)
    {
        std::cout << "V560Unpacker::getOldCounter64:  channel > 15. return channel 15." << std::endl;
        channel = 15;
    }

    uint64_t c = countersOld[channel-1];
    c = c << 32;

    return c + static_cast<uint64_t>(countersOld[channel]);
}

float CaenV560Unpacker::getFrequency(uint32_t channel)
{
    return static_cast<float>(getCounter(channel) - getOldCounter(channel))/((getCounter(clockChannel) - getOldCounter(clockChannel))*clockPeriod);
}

float CaenV560Unpacker::getFrequency64(uint32_t channel)
{
    float counts = 0;
    float clocks = 0;

    if(getCounter64(channel) < getOldCounter64(channel))
        counts = (0xFFFFFFFF + getCounter64(channel)) - getOldCounter64(channel);   // handle with buffer overflow
    else
        counts = getCounter64(channel) - getOldCounter64(channel);

    if(getCounter64(clockChannel) < getOldCounter64(clockChannel))
        clocks = (0xFFFFFFFF + getCounter64(clockChannel)) - getOldCounter64(clockChannel);   // handle with buffer overflow
    else
        clocks = getCounter64(clockChannel) - getOldCounter64(clockChannel);

    float ret = counts/(clocks*clockPeriod);
    return ret;
}

void CaenV560Unpacker::setClockChannel(uint32_t channel, double period)
{
    if(channel > 15)
    {
        std::cout << "V560Unpacker::setClockChannel:  channel > 15. set channel=0." << std::endl;
        channel = 0;
    }
    if(period <= 0 )
    {
        std::cout << "V560Unpacker::setClockChannel:  period <= 0. set period=1e-3." << std::endl;
        period = 1e-3;
    }

    clockChannel = channel;
    clockPeriod = period;
}
