#include "processor/TQClamUnpackProc.h"
#include "evt/TQClamRawEvent.h"

#include "TGo4MbsEvent.h"
#include "TGo4MbsSubEvent.h"
#include "TGo4Log.h"

#include "parameter/TQClamUnpackParameter.h"

#include "readoutmarkers.h"
#include "elb_vfb6_unpacker.h"
#include "struck_sis3316_unpacker.h"

#include "QclamCalibration.h"

#include <inttypes.h>
#include <iomanip>
#include <iostream>

ClassImp(TQClamUnpackProc)

TQClamUnpackProc::TQClamUnpackProc() : TGo4EventProcessor("QClamUnpackProc")
{}


TQClamUnpackProc::TQClamUnpackProc(const char* name) : TGo4EventProcessor(name)
{
        TGo4Log::Info("[TQClamUnpackProc] Create %s.",name);


        //-----------------------------------------------------------
        // 1. initialisiere Parameter
	fUnpackPar = (TQClamUnpackParameter*) GetParameter("UnpackParam");

	fEvtCounter = 0;


        const size_t nTDC = 19;   // oder 18, je nach Setup
                                  //
        /** initialisierungen noetig? */
        for (size_t i = 0; i < nVFB6_TDCs; ++i)
        {
                fVfb6TdcUnpacker[i] = std::make_unique<ElbVfb6Unpacker>();//(i);
        }


        // DNL Kalibrationstabelle --> zur Zeitkorrektur
        for(size_t tdc=0; tdc<nVFB6_TDCs; ++tdc)
        {
                calibTables[tdc] = fVfb6TdcUnpacker[tdc]->getCalibrationTable();
        }

        LoadTdcCalibrationTables();


}



TQClamUnpackProc::~TQClamUnpackProc()
{
    if(fDebugFile.is_open())
        fDebugFile.close();
}


//-----------------------------------------------------------
// Unpack
Bool_t TQClamUnpackProc::BuildEvent(TGo4EventElement* dest)
{
	/** hole output- und input-event */
	auto* out = dynamic_cast<TQClamRawEvent*>(dest);
	auto* mbs = dynamic_cast<TGo4MbsEvent*>(GetInputEvent());
	if(!mbs) { return kFALSE; } /** falls kein input -->*/

	fEvtCounter++;

	out->fEvtCounter = fEvtCounter;


	mbs->ResetIterator();
	TGo4MbsSubEvent* sub = nullptr;

	while((sub = mbs->NextSubEvent()) != nullptr)
	{
		const uint32_t* data32 = reinterpret_cast<const uint32_t*>(sub->GetDataField());
		size_t nwords32 = sub->GetIntLen(); // Anzahl 32‑Bit‑Wörter

		if(nwords32 == 0) { continue; }

		// Packe das Subereignis in einen std::vector<uint32_t>,
		// so wie es QCLAMon an analyseFileFormat übergibt




		std::vector<uint32_t> data(data32, data32 + nwords32);

		size_t   i_start      = 0;
		size_t   i_end        = 0;
		uint32_t datablock_id = 0;
		uint32_t datablock_nr = 0;

		ElbVfb6Unpacker::VFB6_EVENT_LIST evlist;
		evlist.reserve(255);

		for (size_t i = 0; i < data.size(); ++i)
		{
			uint32_t dword = data[i];

			// Block-Header erkennen
			if (dword == ELB_VFB6TDC_ID ||
					dword == READOUTNR_ID   ||
					dword == SCALER_ID      ||
					dword == DAGOBERT_ID)
			{
				// Schutz gegen abgeschnittene Events
				if(data.size() - i < 3) { break; }

				datablock_id = dword;
				datablock_nr = data[++i];   // Modulnummer
				i_start      = i + 1;       // Daten beginnen nach der Modulnummer
				continue;
			}

			// Blockende (TDC/Scaler) oder Ende des Subevents
			if(dword == END_ID || dword == END_ID_OLD || i == data.size() - 1)
			{
				i_end = i;

				switch (datablock_id)
				{
					case READOUTNR_ID: break;

					case SCALER_ID:
					{
						size_t n = i_end - i_start + 1;
						if(n < 10)
						{
							printf("WARNUNG: Scaler-Block zu kurz: %zu Worte!\n", n);
						}
						else
						{
							RawScaler sc;
							sc.scintillatorPMTUp        = data[i_start+0];
							sc.scintillatorPMTDown      = data[i_start+1];
							sc.cherenkovPMT             = data[i_start+2];
							sc.coincidenceTrigger       = data[i_start+3];
							sc.acceptedTrigger          = data[i_start+4];
							sc.clockCounter             = data[i_start+5];
							sc.livetimeCounter          = data[i_start+6];
							sc.mbsLivetimeCounter       = data[i_start+7];
							sc.faradayCupChargeCounter  = data[i_start+8];
							// falsch...sc.TDCTrigger = data[i_start+9];
							//sc.faradayCupChargeCounter  = data[i_start+9];

							/*
							 * --> das nicht die fehlerzaehler....
							 fDebugFile << "TDCTrigger: " << data[i_start+9]
							 << "szintPmtUpErr: " << data[i_start+10]
							 << " - szintPmtDownErr: " << data[i_start+11]
							 << " - cherenkovPmtErr: " << data[i_start+12]
							 << "\ncoincidenceTriggerErr: " << data[i_start+13]
							 << " - acceptedTriggerErr: " << data[i_start+14]
							 << "\n clockCounterErr: " << data[i_start+15]
							 << "\nlivetimeCounterErr: " << data[i_start+16]
							 << " - mbsLiveTimeCounterErr: " << data[i_start+17]
							 << "\nfaradayCupCurrCountErr: " << data[i_start+18]
							 << "\nreserved: " << data[i_start+19] << "\n\n";
			    				*/
							out->scalers.push_back(sc);
						}

						break;
					}

					case ELB_VFB6TDC_ID:
					{
						if(datablock_nr >= fVfb6TdcUnpacker.size()) { break; }

						auto& unpacker = fVfb6TdcUnpacker[datablock_nr];
						evlist.clear();

						unpacker->analyseFileFormat(
								data,
								i_start,
								i_end,
								evlist,
								0x7FFFFFFF,
								true
								);

						for (auto& tev : evlist)
						{
                                                        if (tev.eventNr == -1) { break; }

                                                        // Zeit aus TDC-Modul 0 holen
                                                        if(datablock_nr == 0)
                                                        {
                                                                out->triggerTime = tev.timestamp1mu;
                                                        }

                                                        for (size_t h = 0; h < tev.nHits; ++h)
                                                        {
                                                                uint32_t hitcode = tev.hits[h];
                                                                uint32_t ch      = unpacker->getChannel(hitcode);

                                                                auto& calib_table = calibTablesFromFiles[datablock_nr][ch];
								double tns = 0.;
                                                                if(fUnpackPar->fTimeInterpolationElbVfb6_Random)
                                                                {
                                                                        tns = unpacker->getTime(
                                                                                        hitcode,
                                                                                        calib_table,
                                                                                        ElbVfb6Unpacker::TimeInterpolation::Random
                                                                                        );
                                                                }
                                                                else if(fUnpackPar->fTimeInterpolationElbVfb6_Linear)
                                                                {
                                                                        tns = unpacker->getTime(
                                                                                        hitcode,
                                                                                        calib_table,
                                                                                        ElbVfb6Unpacker::TimeInterpolation::Linear);
                                                                }
                                                                else if(fUnpackPar->fTimeInterpolationElbVfb6_None)
                                                                {
                                                                        tns = unpacker->getTime(
                                                                                        hitcode,
                                                                                        calib_table,
                                                                                        ElbVfb6Unpacker::TimeInterpolation::None);
                                                                }
                                                                else // default: Random
                                                                {
                                                                        tns = unpacker->getTime(
                                                                                        hitcode,
                                                                                        calib_table,
                                                                                        ElbVfb6Unpacker::TimeInterpolation::Random
                                                                                        );
                                                                }

                                                                // emplace_back erzeugt keine Kopier --> effiktiver
                                                                out->hits.emplace_back(RawHit
                                                                                {
                                                                                .accepted = true,
                                                                                .tdcID = static_cast<int>(datablock_nr),
                                                                                .channel = static_cast<int>(ch),
                                                                                .rawTimeInNanoSeconds = tns
                                                                                });

                                                        }
                                                }



						break; /** Ende Elb_VfB-Block */
					}

					case DAGOBERT_ID:
					{
						break;
					}

            
					default: break;
				}

				datablock_id = 0;
			}
		}

	}

	out->eventNumber = mbs->GetCount();
	out->triggerID = mbs->GetTrigger();
	//printf("eventnumber: %u, triggerID: %u\n",out->eventNumber,out->triggerID);
	//out->triggerTime = 0.1;


	if(!out->scalers.empty())
	{
		// Zeit kommt NICHT aus dem Scalerblock, sondern aus TDC0!
		out->scalers[0].timeMicroSeconds = out->triggerTime;
		//printf("timeMicroSec: %f\n",out->triggerTime);
	}

	evt->SetValid(kTRUE);
	return kTRUE;
}









//-----------------------------------------------------------
// Lade alle Kalibrationstabellen zur Fine-Time Korrektur
void TQClamUnpackProc::LoadTdcCalibrationTables()
{
        for(size_t tdc = 0; tdc < nVFB6_TDCs; ++tdc)
        {
                TString twoDigit = (tdc < 10 ? Form("0%lu", tdc) : Form("%lu", tdc));
                TString filename = Form("%s/%s/tdc_%s_calibration_table.txt",PROJECT_SOURCE_DIR,fUnpackPar->fFolderTdcCalibration.Data(),twoDigit.Data());

                readTextFileTo2DVectorDouble(timeCalibrationTableFPGA[tdc], filename);


                for(size_t ch = 0; ch < ElbVfb6Unpacker::nChannels; ++ch)
                {
                        for(size_t bin = 0; bin < ElbVfb6Unpacker::nTimeBins; ++bin)
                        {
                                calibTablesFromFiles[tdc][ch][bin] = timeCalibrationTableFPGA[tdc][ch][bin];
                        }
                }

        }
}








//-----------------------------------------------------------
// Lese Kalibrationstabelle zur Fine-Time Korrektur ein
void TQClamUnpackProc::readTextFileTo2DVectorDouble(std::vector<std::vector<double>>& lines, const TString& filename)
{
        std::ifstream file(filename.Data());
        if(!file.good())
        {
                std::cout << "ERROR reading calibration file " << filename << "\n";
                return;
        }

        std::string line;
        while(std::getline(file, line))
        {
                std::stringstream ss(line);
                std::vector<double> row;
                double v;
                while(ss >> v) row.push_back(v);
                lines.push_back(row);
        }
}










//-----------------------------------------------------------
// NOT USED!!!
double TQClamUnpackProc::ComputeFineTime(
        int tdcID,
        unsigned short channel,
        unsigned short clock200MHz,
        unsigned short flag400MHz,
        unsigned short fpgaCounter)
{
        double tns = 0.0;

        // 1) 200 MHz clock → 5 ns Schritte
        tns += static_cast<double>(clock200MHz) * period200MHzInNanoSeconds;

        // 2) 400 MHz flag → 2.5 ns Schritte
        tns += static_cast<double>(flag400MHz) * period400MHzInNanoSeconds;

        // 3) FineTime aus FPGA‑Kalibrationstabelle
        const auto& table = timeCalibrationTableFPGA[tdcID][channel];

        // Zufallswert zwischen zwei Kalibrationspunkten
        static thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<double> dist(
                        table[fpgaCounter],
                        table[fpgaCounter + 1]
                        );

        double fine = dist(gen);

        // Tabelle ist normiert auf 400 MHz → multiplizieren
        tns += fine * period400MHzInNanoSeconds;

        return tns;
}


























void TQClamUnpackProc::DecodeElbBlock(const std::vector<uint32_t>& block,
                                      TQClamRawEvent* out,
                                      int tdcID)
{

    if(tdcID < 0 || tdcID >= N_TDC)  // passend zu deiner Konfiguration
    {
        printf("DecodeElbBlock: ungueltige TDCID=%d, Block verworfen (len=%zu)\n",
               tdcID, block.size());
        return;
    }


    ElbVfb6Unpacker::VFB6_EVENT_LIST list;
    list.reserve(255);

    ElbVfb6Unpacker vfb6;

    // block ist schon "sauber": nur VFB6‑Daten
    vfb6.analyseFileFormat(block, 0, block.size()-1, list, 250, false);

    for(const auto& tev : list)
    {
        printf("        Event: tdcID=%d, nHits=%d trigReq=%ld ts=%ld µs\n",
               tdcID, tev.nHits, tev.triggerReguest, tev.timestamp1mu);

        for(size_t h = 0; h < tev.nHits; ++h)
        {
            uint32_t hit = tev.hits[h];
            uint32_t ch  = vfb6.getChannel(hit);

            // optional: halbe Hits rauswerfen
            // if(ch == 0 && hit < 0x10000) continue;

            double t_ns_rand = vfb6.getTime(hit, ElbVfb6Unpacker::Random);

            std::cout << "        hit " << h
                      << ": ch=" << ch
                      << "  t=" << t_ns_rand << " ns"
                      << "  (0x" << std::hex << hit << std::dec << ")\n";


	    RawHit rawhit;
	    rawhit.channel = ch;
	    rawhit.tdcID = tdcID;
	    rawhit.rawTimeInNanoSeconds = t_ns_rand;
	    rawhit.accepted = true;

	    out->hits.push_back(rawhit);
        }
    }
}











