#include "processor/TDagobertUnpackProc.h"

#include "struck_sis3316_unpacker.h"
#include "elb_vfb6_unpacker.h"
#include "readoutmarkers.h"

#include "TGo4MbsEvent.h"
#include "evt/TQClamRawEvent.h"
#include "TGo4MbsSubEvent.h"
#include "TGo4Log.h"


//ClassImp(TDagobertUnpackProc)

TDagobertUnpackProc::TDagobertUnpackProc() : TGo4EventProcessor("DagobertUnpackProc")
{}

TDagobertUnpackProc::TDagobertUnpackProc(const char* name) : TGo4EventProcessor(name)
{
	TGo4Log::Info("[TDagobertUnpackProc] Create %s.",name);

	/** initialisierungen noetig? */
}


TDagobertUnpackProc::~TDagobertUnpackProc()
{
}




Bool_t TDagobertUnpackProc::BuildEvent(TGo4EventElement* dest)
{
        /** hole output- und input-event */
        auto* out = dynamic_cast<TQClamRawEvent*>(dest);
        auto* mbs = dynamic_cast<TGo4MbsEvent*>(GetInputEvent());
        if(!mbs) { return kFALSE; } /** falls kein input -->*/


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

                                                break; /** Scaler-Block */
                                        }


                                        case ELB_VFB6TDC_ID:
					{
						break;
                                        }

					case DAGOBERT_ID:
					{
						/*
						size_t n = i_end - i_start + 1;
						RawDagobert dg;
						dg.moduleID = datablock_nr;

						dg.words.assign(data.begin()+i_start,data.begin()+i_end+1);


						out->dagobert.push_back(std::move(dg));


						fSis3316Unpacker.decode(
								data,
								i_start-1,
								i_end,
								out->dagobert,
								false); // kein verbose 
						*/

						std::array<std::array<std::vector<StruckSis3316Unpacker::Sis3316Data>,
                                                        StruckSis3316Unpacker::nChannels>,
                                                        StruckSis3316Unpacker::nModules> tmpDagobert;

                                                StruckSis3316Unpacker fDagobertUnpacker;
                                                fDagobertUnpacker.decode(
                                                                data,
                                                                i_start-1,
                                                                i_end,
                                                                tmpDagobert,
                                                                false);

                                                for(int m=0; m<StruckSis3316Unpacker::nModules; ++m)
                                                {
                                                        for(int ch=0; ch<StruckSis3316Unpacker::nChannels; ++ch)
                                                        {
                                                                for(const auto& ev : tmpDagobert[m][ch])
                                                                {
                                                                        printf("DAGOBERT: module=%d channel=%d timestamp=%llu\n"
                                                                                       "gate1=%u gate2=%u\n"
                                                                                       "maw_max=%u maw_before=%u max_after=%u\n"
                                                                                       "start_energyFIR=%u max_energyFIR=%u\n"
                                                                                       "flags=%u\n\n",
                                                                                        ev.module,
                                                                                        ev.channel,
                                                                                        (unsigned long long)ev.timestamp,
                                                                                        ev.firstGateSum,
                                                                                        ev.secondGateSum,
                                                                                        ev.maxMAW,
                                                                                        ev.beforeMAW,
                                                                                        ev.afterMAW,
                                                                                        ev.startEnergyFIR,
                                                                                        ev.maxEnergyFIR,
                                                                                        ev.flags);
                                                                }
                                                        }
                                                }



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

	out->SetValid(kTRUE);
        return kTRUE;
}

