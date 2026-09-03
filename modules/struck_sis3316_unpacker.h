/*
    Data unpacker for the VME module: Struck SIS3316 .
    21-08-25 adjusted by JAM <j.adamczewski@gsi.de> following schema by Maxim Singer

    First try to just extract gate sums and scaler values (no traces) for live display in QClaMon
    see overview slides at https://www.struck.de/sis3316-2014-03-20.pdf
*/


#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <memory>
#include <cmath>
#include <algorithm>



/********
 *  sample of subevent from c_027_0000.lmd (JAM 21-08-2025)

SubEv ID      3 Type/Subtype    10     1 Length    64[w] Control 12 Subcrate  0
  aaaa.aaa1 0006.000e d406.1119 0000.453d 0000.f89f 0800.71ce 0800.438b 0800.3865
  ffff.fffa 0000.737a e000.0000 0006.002e d406.111a 0000.4724 0000.9b87 0800.15df
  0800.0b38 0800.092c ffff.e25e 0000.2ca6 e400.0000 0006.009e d406.1119 0000.45fe
  0001.4337 0800.4b2f 0800.2e8e 0800.2534 0000.0004 0000.93f1 e000.0000

===============
- aaaa.aaa1  :identifier from f_user.c, last nibble is trigger number

- 0006.000e  : header 1 as in vme_struc_sis3316.spec (ucesb)
              UINT32 header1
                 {
                    0_3:   format;
                    4_5:   chanNo;
                    6_7:   groupNo;
                    8_15:  moduleNo = MATCH(base);
                    16_31: timeSH;
                    ..
                    }
               from this we have format 3, channel 0 and group 0, module 0, timestamp high is 0x0006

- d406.1119 : header 2 as in vme_struc_sis3316.spec (ucesb)
               UINT32 header2
                 {
                  0_31:  timeSL;
                  ..
                  }
                so timestamp lo is 0xd4061119, full timestamp 0x6d4061119

- 0000.453d :   sum (energy) of gate 7 (gateG) as described in vme_struc_sis3316.spec (ucesb)
                  UINT32 gate7
                  {
                     0_27:  sum;
                     28_31: 0;
                  }
-  0000.f89f :  sum (energy) of gate 8 (gateH) as described in vme_struc_sis3316.spec (ucesb)
                  UINT32 gate8
                  {
                     0_27:  sum;
                     28_31: 0;
                  }

 - 0800.71ce
   0800.438b
   0800.3865   : maw1 (max), maw2 (before), maw3 (after)  (CFD), like
                   UINT32 maw1
                    {
                       0_27:  max;
                       28_31: 0;
                        ..
                    }
//////////////////////////// variant 1:
 -ffff.fffa  : ?? scaler identifier as in vme_struc_sis3316_scaler.spec

                 UINT32 check NOENCODE
                   {
                      0_3:   moduleNo = MATCH(base);
                      4_31:  scalerIdentifier = MATCH(0xFFFFFFF);
                     ..
                   }
                However, base a is never written in f_user.c, only scalers 0,1,2 !!!!!!!


 -0000.737a  : ??? scaler data would be more than this...
////////////////////////////// variant
               more likely this is still part of regular ADC/SIS3316_FIR_ENERGY

  -ffff.fffa
                UINT32 start_energy1
                  {
                     0_31:  startE;
                  }

    -0000.737a
                  UINT32 max_energy1
                  {
                     0_31:  maxE;
                  }



e000.0000  :         header 2 as in vme_struc_sis3316.spec (ucesb)
                  UINT32 header3
                     {
                        0_25:  sampleLength;
                        26:    pileupOrRepileup1;
                        27:    mawFlag;
                        28_31: 0xE;
                        ....
                     }
                  Here we have sample length 0, so no trace data follows! So this is not trailer, but  sampleLength words of traces could follow


**/

/******************************** from vme_struck_sis3316_scaler.spec
       *  // loop over number of active channels
       list (0<=i<CHANNEL_NUMBER)
       {
       UINT32 raw NOENCODE
       {
       0_31: v;
       ENCODE(raw_count[i], (value=v));
       }
       UINT32 accepted NOENCODE
       {
       0_31: v;
       ENCODE(acc_count[i], (value=v));
       }
       UINT32 deadtime NOENCODE
       {
       0_31: v;
       ENCODE(dt_count[i], (value=v));
       }
       UINT32 pileup NOENCODE
       {
       0_31: v;
       ENCODE(pileup_count[i], (value=v));
       }
       UINT32 veto NOENCODE
       {
       0_31: v;
       ENCODE(veto_count[i], (value=v));
       }
       UINT32 Hi_energy_suppress NOENCODE
       {
       0_31: v;
       ENCODE(suppressed_count[i], (value=v));
       }
       }
       ******************************************* */

class StruckSis3316Unpacker
{
public:
    StruckSis3316Unpacker();
    virtual ~StruckSis3316Unpacker();

    const static uint32_t nChannels = 16;      // defined by firmware
    const static uint32_t nScalers = 2;
    const static uint32_t nModules = 2;

    ////////////////////////////////////
    class Sis3316ScalerData
    {
    public:
      Sis3316ScalerData():id(-1)
      {
        Clear();

      }

      void Clear()
        {
          id=-1;
          raw.clear();;
          accepted.clear();;
          deadtime.clear();;
          pileup.clear();;
          veto.clear();
          suppressed.clear();
        }

      int id; // scaler number
      std::vector<uint32_t> raw;
      std::vector<uint32_t> accepted;
      std::vector<uint32_t> deadtime;
      std::vector<uint32_t> pileup;
      std::vector<uint32_t> veto;
      std::vector<uint32_t> suppressed;
    };



    ////////////////
    class Sis3316Data
    {
    public:
        Sis3316Data():
          module(0), channel(0), timestamp(0),startEnergyFIR(0),maxEnergyFIR(0),firstGateSum(0),secondGateSum(0),maxMAW(0), beforeMAW(0),afterMAW(),flags(0),nSamples(0), hasScaler(false)
      {
          for(uint32_t s=0; s<StruckSis3316Unpacker::nScalers; ++s)
          {
            scaler.push_back(Sis3316ScalerData());
          }
      }


        bool IsPileupOrRepileup() const
        {
          return ((flags >> 26) & 0x1) == 0x1;
        }

        bool IsMawFlag() const
        {
          return ((flags >> 27) & 0x1) == 0x1;
        }

        void Clear()
        {
          samples1.clear();
          samples2.clear();

          for(uint32_t s=0; s<StruckSis3316Unpacker::nScalers; ++s)
          {
            scaler[s].Clear();
          }
        }

	uint32_t marker = 0; // marker scaler/physics event

        uint16_t module; // module identifier in crate
        uint16_t channel; // ADC channel and group -> full channel id
        uint64_t timestamp;

        uint32_t startEnergyFIR;  // (Energy value from first value of Trigger Gate )
        uint32_t maxEnergyFIR;    // (during Trigger Gate active )
        uint32_t firstGateSum; // sum (energy) of gate 7 (gateG)
        uint32_t secondGateSum;// sum (energy) of gate 8 (gateH)

        uint32_t maxMAW;    // CFD moving average window maximum
        uint32_t beforeMAW; // CFD moving average window before
        uint32_t afterMAW;  // CFD moving average window after


        uint32_t flags; // put information of header3 here

    //    unsigned short eventHeader;

     // for traces, if any
        uint32_t nSamples;
        std::vector<uint16_t> samples1;
        std::vector<uint16_t> samples2;

     // for scaler data, if any
        bool hasScaler;
      std::vector<Sis3316ScalerData> scaler;

        // Sis3316ScalerData scaler[StruckSis3316Unpacker::nScalers];

    };
////////////////////////////////////

    int decode(const std::vector<uint32_t> &rawData,
                          size_t start,
                          size_t end,
                          std::array<std::array<std::vector<StruckSis3316Unpacker::Sis3316Data>, StruckSis3316Unpacker::nChannels>,StruckSis3316Unpacker::nModules> &dataList,
                          bool verbose=false);


 private:

    void init();

};
