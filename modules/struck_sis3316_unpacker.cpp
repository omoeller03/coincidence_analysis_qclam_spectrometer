/*
 Data unpacker for the VME module: Struck SIS3316 .
 21-08-25 adjusted by JAM <j.adamczewski@gsi.de> following schema by Maxim Singer

 First try to just extract gate sums and scaler values (no traces) for live display in QClaMon

 */

#include "struck_sis3316_unpacker.h"

// JAM- to be safe we again try to use some nice macros:
#define SIS3316UNP_NEXTINDEX_WITHCHECK  \
    di++; \
    if(di>= indata.size()){  \
      std::cout << "Sis3316_Unpacker::decode: WARNING- no full event at end of data buffer! last data is" << std::hex << indata.back()<< std::endl;\
      break; \
    }

StruckSis3316Unpacker::StruckSis3316Unpacker()
{
  init();
}

StruckSis3316Unpacker::~StruckSis3316Unpacker()
{

}

void StruckSis3316Unpacker::init()
{

}


int StruckSis3316Unpacker::decode(const std::vector<uint32_t> &indata, size_t start, size_t end,
   // std::array<std::vector<StruckSis3316Unpacker::Sis3316Data>, StruckSis3316Unpacker::nChannels> &dataList,
   std::array<std::array<std::vector<StruckSis3316Unpacker::Sis3316Data>, StruckSis3316Unpacker::nChannels>,StruckSis3316Unpacker::nModules> &dataList,
    bool doTrace /** trace entpacken */, bool verbose /** verbose */)

{
  if (indata.size() == 0)
  {
    std::cout << "Sis3316_Unpacker::decode: no data." << std::endl;
    return -1;
  }
  Sis3316Data outEvent;
  for (size_t di = start; di < end && di < indata.size(); ++di)    // one loop = one trigger event
  {
    outEvent.Clear();
    // TODO: optionally treat scaler events!

    // first look for header 1, like  0006.000e  last nibble identifies it
    if ((indata[di] & 0x0000000F) == 0x0000000E)
    {
      outEvent.marker = indata[di];
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode found start marker " << std::hex << indata[di] << std::endl;

      outEvent.module = (indata[di] >> 8) & 0xFF;
      outEvent.channel = (indata[di] >> 4) & 0xF;    // use full channel (chan * group)
       if (verbose)
        std::cout << "Sis3316_Unpacker::decode for module:"<<  outEvent.module<<", ch:"<< std::hex << outEvent.channel << std::endl;

      outEvent.timestamp = (indata[di] >> 16) & 0xFFFF;    // high part of timestamp here
      outEvent.timestamp = (outEvent.timestamp << 32);
      SIS3316UNP_NEXTINDEX_WITHCHECK
      outEvent.timestamp |= indata[di];
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has timestamp: " << std::hex << outEvent.timestamp << std::endl;

      SIS3316UNP_NEXTINDEX_WITHCHECK
      // gate 1 and gate 2:
      outEvent.firstGateSum = indata[di] & 0xFFFFFFF;
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has first gate: " << std::hex << outEvent.firstGateSum << std::endl;
      SIS3316UNP_NEXTINDEX_WITHCHECK
      outEvent.secondGateSum = indata[di] & 0xFFFFFFF;
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has second gate: " << std::hex << outEvent.secondGateSum << std::endl;
      SIS3316UNP_NEXTINDEX_WITHCHECK
      // maw 1, 2, 3 values:
      outEvent.maxMAW = indata[di] & 0xFFFFFFF;
      ;    // CFD moving average window maximum
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has MAW max: " << std::hex << outEvent.maxMAW << std::endl;
      SIS3316UNP_NEXTINDEX_WITHCHECK
      outEvent.beforeMAW = indata[di] & 0xFFFFFFF;
      ;    // CFD moving average window before
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has MAW before: " << std::hex << outEvent.beforeMAW << std::endl;

      SIS3316UNP_NEXTINDEX_WITHCHECK
      outEvent.afterMAW = indata[di] & 0xFFFFFFF;
      ;    // CFD moving average window after
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has MAW after: " << std::hex << outEvent.afterMAW << std::endl;

      // fir energies:
      SIS3316UNP_NEXTINDEX_WITHCHECK
      outEvent.startEnergyFIR = indata[di];    // (Energy value from first value of Trigger Gate )
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has startEnergyFIR: " << std::hex << outEvent.startEnergyFIR
            << std::endl;
      SIS3316UNP_NEXTINDEX_WITHCHECK
      outEvent.maxEnergyFIR = indata[di];    // (during Trigger Gate active )
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode has maxEnergyFIR: " << std::hex << outEvent.maxEnergyFIR << std::endl;

      SIS3316UNP_NEXTINDEX_WITHCHECK
      //header3 with optionally traces following it:
      if ((indata[di] & 0xF0000000) != 0xE0000000)
      {
        if (verbose)
          std::cout << "Sis3316_Unpacker::decode could not find  trailing marker, found " << std::hex << indata[di]
              << std::endl;
        continue;    // try to find next block with header 1
      }

      outEvent.flags = indata[di];    // put full information of header3 here
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode found channel  trailer " << std::hex << outEvent.flags << std::endl;
      // for traces, if any
      outEvent.nSamples = indata[di] & 0x3FFFFFF;
      if (verbose && outEvent.nSamples > 0)
        std::cout << "Sis3316_Unpacker::decode getting traces of length " << std::hex << outEvent.nSamples << std::endl;

      for (uint32_t t = 0; t < outEvent.nSamples; ++t)
      {
        SIS3316UNP_NEXTINDEX_WITHCHECK
	if(doTrace)
	{
	/*
	 * SPAETER EINEN BOOL BAUEN -> TRACE DATEN entpacken ja/nein!!!
        outEvent.samples1.push_back(indata[di] & 0xFFFF);
        outEvent.samples2.push_back((indata[di] >> 16) & 0xFFFF);
	*/
	}
      }

    }
    else if ((indata[di] & 0xFFFFFFF0) == 0xFFFFFFF0)
    {
      outEvent.marker = indata[di];
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode found scaler  block, marker" << std::hex << indata[di] << std::endl;
      // scaler number is last nibble
      uint32_t id = indata[di] & 0xF;


      //
      // TEST OSKAR:
      /*
      std::cout << "[SCALER HEADER]"
          << " word=0x" << std::hex << indata[di]
          << " id=" << std::dec << id
          << std::endl;
      */
      // ENDE TEST

      if (id > StruckSis3316Unpacker::nScalers)
      {
        std::cout << "WARNING Sis3316_Unpacker::decode - invalid scaler id " << id << "from scaler marker  " << std::hex << indata[di] << std::endl;
        continue;
      }
      outEvent.channel=0; // all scaler information is in pseudo channel 0
      outEvent.module=id; //OSKAR TEST 0; // and assign to module 0 block (independent of scaler id)
      outEvent.hasScaler=true;
      Sis3316ScalerData &theScaler = outEvent.scaler[id];
      theScaler.id = id;
      // TODO: loop over all channels
      for (uint32_t c = 0; c < StruckSis3316Unpacker::nChannels; ++c)
      {
        if (verbose)
          std::cout << "\tscaler chan \t" << c << std::endl;
        SIS3316UNP_NEXTINDEX_WITHCHECK
        if (verbose)
          std::cout << "\t scaler raw \t" << std::hex << indata[di]  << std::endl;
        theScaler.raw.push_back(indata[di]);
        SIS3316UNP_NEXTINDEX_WITHCHECK
        if (verbose)
          std::cout << "\t scaler accepted \t" << std::hex << indata[di]  << std::endl;
        theScaler.accepted.push_back(indata[di]);
        SIS3316UNP_NEXTINDEX_WITHCHECK
        if (verbose)
          std::cout << "\t scaler deadtime \t" << std::hex << indata[di]  << std::endl;
        theScaler.deadtime.push_back(indata[di]);
        SIS3316UNP_NEXTINDEX_WITHCHECK
        if (verbose)
          std::cout << "\t scaler pileup   \t" << std::hex << indata[di]  << std::endl;
        theScaler.pileup.push_back(indata[di]);
        SIS3316UNP_NEXTINDEX_WITHCHECK
        if (verbose)
          std::cout << "\t scaler veto \t" << std::hex << indata[di]  << std::endl;
        theScaler.veto.push_back(indata[di]);
        SIS3316UNP_NEXTINDEX_WITHCHECK
        if (verbose)
          std::cout << "\t scaler suppressed \t" << std::hex << indata[di]  << std::endl;
        theScaler.suppressed.push_back(indata[di]);
      }
      //
      // TEST OSKAR
      /*
      std::cout << "[SCALER DATA]"
          << " id=" << id
          << " raw=" << theScaler.raw.size()
          << " accepted=" << theScaler.accepted.size()
          << " deadtime=" << theScaler.deadtime.size()
          << std::endl;
      */
      // TEST ENDE
      //

    }
    else
    {
      if (verbose)
        std::cout << "Sis3316_Unpacker::decode looking for next start block marker, found " << std::hex << indata[di]
            << std::endl;
      continue;
    }

    ////////////////////
    // publish completed data
    dataList.at(outEvent.module).at(outEvent.channel).push_back(outEvent);
  } // for
  return 0;
}





