/*
    Data unpacker for the VME module: Struck SIS3302 Flash ADC.


    Copyright 2014 Maxim Singer

    License: MIT (https://opensource.org/licenses/MIT)

*/

#include "struck_sis3302_unpacker.h"


StruckSis3302Unpacker::StruckSis3302Unpacker()
{
    init();
}

StruckSis3302Unpacker::~StruckSis3302Unpacker()
{

}


void StruckSis3302Unpacker::init()
{
    nLastEventSamples = 0;
}


int StruckSis3302Unpacker::analyseFileFormat(const std::vector<uint32_t> &data,
                                        size_t start,
                                        size_t end,
                                        std::array< std::vector<std::shared_ptr <Sis3302DataFormat>>, 8> &dataformat_list)
{
    if(data.size() == 0)
    {
        std::cout << "SIS3302_Unpacker::analyseDataformat: not data." << std::endl;
        return -1;
    }

    if(data[start]==0xd640d64)  // fill dword to align to the 8 byte border -> skip dword
    {
        start++;
    }

    int channel = 0;
    size_t deadbeef_pos = 0;

/*
    std::cout << std::endl;
    for(size_t di = start; di < end && di < data.size();di++)
    {
        std::cout << std::dec << di << "\t"<< std::hex << data[di] << std::endl;

    }
*/

    for(size_t di = start; di < end && di < data.size();) // one loop = one trigger event
    {
        Sis3302DataFormat dataformat;

        if((data[di] & 0xFFFF0000) == 0xCCCCFF00) // no memory during the readout process -> skip event
        {
            return -2;  // TODO: better behaviour
        }

        // seek for channel number
        if((data[di] & 0xFFFF0000) == 0xCCCC0000)
        {
            channel = data[di++] & 0xFFFF;
        }

        // skip the fill dword that align to the 8 byte border
        if(data[di]==0xd640d64)
        {
            di++;
        }

        dataformat.eventHeader = data[di] & 0xFFFF;
        uint64_t timestamp47_32 = (data[di++] & 0xFFFF0000) >> 16;
        uint64_t timestamp31_0 = data[di++];
        dataformat.timestamp = ((4294967296.0 * timestamp47_32)+(timestamp31_0));

        deadbeef_pos = di+3+ceil((float)nLastEventSamples/2.0f);    // prediction for 0xdeadbeef to minimize search costs
        if(data[deadbeef_pos] != 0xdeadbeef)
        {
            auto res = std::find(data.begin()+di, data.end(), 0xdeadbeef);  // search for 0xdeadbeef
            if(res != data.end())
            {
                deadbeef_pos = std::distance(data.begin(), res);
            }
            else
            {
                std::cout << "SIS3302_Unpacker::analyseDataformat: can't find 0xdeadbeef. file format is corrupt. ch=" << channel
                          << " exit." << std::endl;
                return -3;
            }
        }

        if(deadbeef_pos-di < 3)
        {
            std::cout << "SIS3302_Unpacker::analyseDataformat: no data before 0xdeadbeef. file format is corrupt." << std::endl;
            di = deadbeef_pos + 1;
            continue;
        }
        nLastEventSamples = deadbeef_pos - di - 3;
        dataformat.nSamples = nLastEventSamples;

        dataformat.samples.reserve(dataformat.nSamples);
        for(size_t i = 0; i < nLastEventSamples; i++)
        {
            const unsigned short val1 =  data[di] & 0x0000FFFF;
            const unsigned short val2 = (data[di] & 0xFFFF0000) >> 16;

            dataformat.samples.push_back(val1);
            dataformat.samples.push_back(val2);

            di++;
        }

        dataformat.maxEnergyValue = data[di++];
        dataformat.startEnergyValue = data[di++];
        dataformat.flags = data[di++];

        if(data[di] != 0xdeadbeef)  // end of event
        {
            std::cout << "SIS3302_Unpacker::analyseDataformat: the last dword is not 0xdeadbeef. brocken file format. exit." << std::endl;
            return -1;
        }

        di++;

        dataformat_list.at(channel).push_back(std::shared_ptr <Sis3302DataFormat>(new Sis3302DataFormat(dataformat)));
    }


    return 0;
}
