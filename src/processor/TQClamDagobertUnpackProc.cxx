#include "struck_sis3316_unpacker.h"
#include "elb_vfb6_unpacker.h"
#include "readoutmarkers.h"

#include "evt/TQClamDagobertRawEvent.h"

#include "processor/TQClamDagobertUnpackProc.h"
#include "parameter/TQClamUnpackParameter.h"

#include "TGo4MbsSubEvent.h"
#include "TGo4MbsEvent.h"
#include "TGo4Log.h"

#include <sstream>
#include <iostream>
#include <regex>

ClassImp(TQClamDagobertUnpackProc)

	
TQClamDagobertUnpackProc::TQClamDagobertUnpackProc() : TGo4EventProcessor("UnpackProc")
{}
//-----------------------------------------------------------
// Konstruktor
TQClamDagobertUnpackProc::TQClamDagobertUnpackProc(const char* name) : TGo4EventProcessor(name)
{
	TGo4Log::Info("[TQClamDagobertUnpackProc] Create %s.",name);
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
TQClamDagobertUnpackProc::~TQClamDagobertUnpackProc()
{
}
//-----------------------------------------------------------
// Unpack
Bool_t TQClamDagobertUnpackProc::BuildEvent(TGo4EventElement* dest)
{
        /** hole output- und input-event */
        auto* out = dynamic_cast<TQClamDagobertRawEvent*>(dest);
        auto* mbs = dynamic_cast<TGo4MbsEvent*>(GetInputEvent());
        if(!mbs) { return kFALSE; } /** falls kein input -->*/

	fEvtCounter++;

	out->fEvtCounter = fEvtCounter;




	auto* bufhe = mbs->GetMbsBufferHeader();
	auto* filhe = mbs->GetMbsSourceHeader();

	static int run_number = -1;
	static int file_number = -1;

	if(filhe)
	{
		std::string file_name = filhe->filhe_file;
		out->header.file_name = file_name;

		/// filename:
		// *_XXX_YYYY.lmd
		//
		// XXX = run_number
		// YYYY = file_number

		std::regex re(R"(.*_(\d+)_(\d+)\.lmd$)");
		std::smatch match;

		try
		{
			if(!std::regex_match(file_name,match,re))
			{
				throw std::runtime_error("Filename has wrong format");
			}

			run_number = std::stoi(match[1].str());
			file_number = std::stoi(match[2].str());
		}
		catch(const std::exception& )
		{
			run_number = -1;
			file_number = -1;
		}
			
		out->header.run_number = run_number;
		out->header.file_number = file_number;

		/*
		size_t pos = file_name.find(".lmd");
		
		//while((pos = file_name.find(".lmd")) != std::string::npos)
		if(pos != std::string::npos)
		{
			run_number = std::stoi(file_name.substr(pos-9,3).c_str());
			file_number = std::stoi(file_name.substr(pos-4,4).c_str());

			out->header.run_number = run_number;
			out->header.file_number = file_number;
		}
		*/


		out->header.user = filhe->filhe_user;
		out->header.time = filhe->filhe_time;
		out->header.label = filhe->filhe_label;
	}

	if(bufhe)
	{
		out->header.mbs_event_count = mbs->GetCount();
		out->header.mbs_evts_in_buffer = bufhe->l_evt;
		out->header.mbs_time_sec = bufhe->l_time[0];
		out->header.mbs_time_subsec = bufhe->l_time[1];
	}




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

						/*
						std::cout << "\n[SCALER UNPACKER]\n";
						std::cout << "module=" << datablock_nr
							<< " nWords=" << n
							<< " i_start=" << i_start
							<< " i_end=" << i_end
							<< "\n";

						for(size_t k = i_start; k <= i_end && k < data.size(); ++k)
						{
							std::cout << "  data[" << k << "]=" << data[k] << "\n";
						}
						*/


                                                if(n < 10)
                                                {
                                                        printf("WARNUNG: Scaler-Block zu kurz: %zu Worte!\n", n);
                                                }
                                                else
                                                {
							/*
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
							*/

                                                        out->scaler.szintillatorPmtUp        = data[i_start+0];
                                                        out->scaler.szintillatorPmtDown      = data[i_start+1];
                                                        out->scaler.cherenkovPmt             = data[i_start+2];
                                                        out->scaler.coincidenceTrigger       = data[i_start+3];
                                                        out->scaler.acceptedTrigger          = data[i_start+4];
                                                        out->scaler.clockCounter             = data[i_start+5];
                                                        out->scaler.livetimeCounter          = data[i_start+6];
                                                        out->scaler.mbsLivetimeCounter       = data[i_start+7];

                                                        out->scaler.faradayCupChargeCounter  = data[i_start+8];



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
                                                        //out->scalers.push_back(sc);
                                                }
                                                break; /** Scaler-Block */
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
								// timestamp1mu ist in tev.timestamp1mu (oder äquivalent)
								//fDebugFile << "\ntimestamp1mu: " << tev.timestamp1mu << "\n\n";
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
						break;
                                        }
					case DAGOBERT_ID:
					{
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
									// emplace_back --> keine Kopie erzeugen
//									std::cout << "module=" << m << " channel=" << ch << std::endl;
									out->gammaHits.emplace_back(RawGammaHit{

									//RawGammaHit& gh = out->gammaHits.back();
											.module = static_cast<size_t>(m),
											.channel = static_cast<size_t>(ch),
											.timestamp = static_cast<double>(ev.timestamp),
											.startEnergyFIR = ev.startEnergyFIR,
											.maxEnergyFIR = ev.maxEnergyFIR,
											.firstGateSum = ev.firstGateSum,
											.secondGateSum = ev.secondGateSum,
											.maxMAW = ev.maxMAW,
											.beforeMAW = ev.beforeMAW,
											.afterMAW = ev.afterMAW,
											.flags = ev.flags,
											.pileup = ev.IsPileupOrRepileup(),
											.mawFlag = ev.IsMawFlag(),
											// wird direkt in-place konstruiert
											.samples1 = ev.samples1,   
											.samples2 = ev.samples2,
											.hasScaler = ev.hasScaler,
											// werden unten gefüllt
											.scaler_raw = {},
											.scaler_accepted = {},
											.scaler_deadtime = {},
											.scaler_pileup = {},
											.scaler_veto = {},
											.scaler_suppressed = {}
											});
									// Referenz auf das gerade erzeugte Element
									RawGammaHit& gh = out->gammaHits.back();

									if(ev.hasScaler)
									{
										const auto& sc = ev.scaler[m];

										/*
										out->dagobertScalers.emplace_back(
												RawDagobertScaler
												{
												.module = static_cast<size_t>(m),
												.timestamp = static_cast<double>(ev.timestamp),
												.raw = sc.raw,
												.accepted = sc.accepted,
												.deadtime = sc.deadtime,
												.veto = sc.veto
												.suppressed = sc.suppressed
												}
												);
										*/
										RawDagobertScaler rawScaler;

										rawScaler.module = m;
										rawScaler.timestamp = ev.timestamp;

										rawScaler.raw        = sc.raw;
										rawScaler.accepted   = sc.accepted;
										rawScaler.deadtime   = sc.deadtime;
										rawScaler.veto       = sc.veto;
										rawScaler.suppressed = sc.suppressed;

										out->dagobertScalers.push_back(rawScaler);


										/*
										 * ALT: Scaler noch nicht unabhaengig von GammaHits
										const auto& sc = ev.scaler[m];


										gh.scaler_raw = sc.raw;
										gh.scaler_accepted = sc.accepted;
										gh.scaler_deadtime = sc.deadtime;
										gh.scaler_pileup = sc.pileup;
										gh.scaler_veto = sc.veto;
										gh.scaler_suppressed = sc.suppressed;

										static int nPrint = 0;

										if (nPrint < 30)
										{
											std::cout << "\n=== SCALER EVENT ===\n";
											std::cout << "SIS module = " << m
												<< "  SIS channel = " << ch
												<< "  timestamp = " << ev.timestamp
												<< std::endl;

											for(size_t i=0;i<sc.raw.size();i++)
											{
												std::cout
													<< "raw[" << i << "]=" << sc.raw[i]
													<< " accepted=" << sc.accepted[i]
													<< std::endl;
											}

											nPrint++;
										}
										*/

									}

									/*
									if(true) //ev.hasScaler)
									{	
										for(const auto& sc : ev.scaler)
										{
										
//											std::cout << "scaler_raw=" << sc.raw.end() << std::endl;
											gh.scaler_raw.insert(
													gh.scaler_raw.end(),
													sc.raw.begin(),
													sc.raw.end());
											gh.scaler_accepted.insert(
													gh.scaler_accepted.end(),
													sc.accepted.begin(),
													sc.accepted.end());
											gh.scaler_deadtime.insert(
													gh.scaler_deadtime.end(),
													sc.deadtime.begin(),
													sc.deadtime.end());
											gh.scaler_pileup.insert(
													gh.scaler_pileup.end(),
													sc.pileup.begin(),
													sc.pileup.end());
											gh.scaler_veto.insert(
													gh.scaler_veto.end(),
													sc.veto.begin(),
													sc.veto.end());
											gh.scaler_suppressed.insert(
													gh.scaler_suppressed.end(),
													sc.suppressed.begin(),
													sc.suppressed.end());

										}
									}
									*/
									/*
									std::cout << "[SCALER COPY]"
									<< " event module m=" << m
									<< " channel ch=" << ch
									<< " ev.scaler.size()=" << ev.scaler.size()
									<< std::endl;

									for(size_t i = 0; i < ev.scaler.size(); ++i)
									{
									std::cout << "    scaler[" << i << "]"
									<< " raw=" << ev.scaler[i].raw.size()
									<< " accepted=" << ev.scaler[i].accepted.size()
									<< " deadtime=" << ev.scaler[i].deadtime.size()
									<< std::endl;
									}


									if(ev.scaler.size() > 0)
									{
									const auto& sc = ev.scaler[0];

									gh.scaler_raw = sc.raw;
									gh.scaler_accepted = sc.accepted;
									gh.scaler_deadtime = sc.deadtime;
									gh.scaler_pileup = sc.pileup;
									gh.scaler_veto = sc.veto;
									gh.scaler_suppressed = sc.suppressed;
									}
									*/


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

	/*
        if(!out->scalers.empty())
        {
                // Zeit kommt NICHT aus dem Scalerblock, sondern aus TDC0!
                out->scalers[0].timeMicroSeconds = out->triggerTime;
                //printf("timeMicroSec: %f\n",out->triggerTime);
        }
	*/


	if(fFirstScalerTime)
	{
		prev_timeMicroSeconds = double(out->triggerTime);
		sum_timeMicroSeconds = double(out->triggerTime);
		fFirstScalerTime = false;
	}
	else
	{
		if(double(out->triggerTime) < prev_timeMicroSeconds)
		{
			sum_timeMicroSeconds += double(out->triggerTime);
			prev_timeMicroSeconds = double(out->triggerTime);
		}

		else
		{
			sum_timeMicroSeconds += (double(out->triggerTime) - prev_timeMicroSeconds);
			prev_timeMicroSeconds = double(out->triggerTime);
		}
	}

	out->scaler.timeMicroSeconds = double(sum_timeMicroSeconds);

	//std::cout << "timeMicroSeconds = " << out->scaler.timeMicroSeconds << std::endl;

	out->SetValid(kTRUE);
        return kTRUE;
}
//-----------------------------------------------------------
// Lade Fine-Time Kalibrations-Tabellen
void TQClamDagobertUnpackProc::LoadTdcCalibrationTables()
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
// Lese Fine-Time Kalibrationstabelle ein
void TQClamDagobertUnpackProc::readTextFileTo2DVectorDouble(std::vector<std::vector<double>>& lines, const TString& filename)
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
double TQClamDagobertUnpackProc::ComputeFineTime(
        int tdcID,
        unsigned short channel,
        unsigned short clock200MHz,
        unsigned short flag400MHz,
        unsigned short fpgaCounter)
{
	double tns = 0.0;
	// 1) 200 MHz clock -> 5 ns Schritte
	tns += static_cast<double>(clock200MHz) * period200MHzInNanoSeconds;
	// 2) 400 MHz flag -> 2.5 ns Schritte
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
	// Tabelle ist normiert auf 400 MHz -> multiplizieren
	tns += fine * period400MHzInNanoSeconds;
	return tns;
}
