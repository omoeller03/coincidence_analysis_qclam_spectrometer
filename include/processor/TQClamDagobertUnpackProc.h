#ifndef TQCLAMDAGOBERTUNPACKPROC_H
#define TQCLAMDAGOBERTUNPACKPROC_H

#include "TGo4EventProcessor.h"
#include <vector>
#include <memory>
#include <fstream>

#include "elb_vfb6_unpacker.h"
#include "struck_sis3316_unpacker.h"

class TQClamDagobertRawEvent;
class TQClamUnpackParameter;
//class ElbVfb6Unpacker;

class TQClamDagobertUnpackProc : public TGo4EventProcessor
{
        public:
                TQClamDagobertUnpackProc();
                TQClamDagobertUnpackProc(const char* name);
                virtual ~TQClamDagobertUnpackProc();

                Bool_t BuildEvent(TGo4EventElement* dest) override;

		static constexpr double period200MHzInNanoSeconds = 5.0; // 200 MHz Clock
		static constexpr double period400MHzInNanoSeconds = 2.5; // 400 MHz Clock


		int fEvtCounter{0};
	private:
		TQClamUnpackParameter* fUnpackPar{nullptr};

        private:

		bool fFirstScalerTime = true;
		double prev_timeMicroSeconds = 0.;
		double sum_timeMicroSeconds = 0.;

		bool fInElbBlock{false};
		std::vector<uint32_t> fElbBuffer;
		int currentTdcID;
		static constexpr size_t nVFB6_TDCs = 19;   // du hast 19 Module
		std::array<std::unique_ptr<ElbVfb6Unpacker>,nVFB6_TDCs> fVfb6TdcUnpacker;
		std::array<std::array<ElbVfb6Unpacker::CALIBRATION_TABLE,ElbVfb6Unpacker::nChannels>,nVFB6_TDCs> calibTables;

		StruckSis3316Unpacker fDagobertUnpacker;
		std::array<std::array<std::vector<StruckSis3316Unpacker::Sis3316Data>,StruckSis3316Unpacker::nChannels>,StruckSis3316Unpacker::nModules> tmpDagobert;

		std::array<std::vector<std::vector<double>>, nVFB6_TDCs> timeCalibrationTableFPGA;
		std::array<std::array<ElbVfb6Unpacker::CALIBRATION_TABLE, ElbVfb6Unpacker::nChannels>,
			nVFB6_TDCs> calibTablesFromFiles;


                bool firstZeroSeen[19]{false};

		std::ofstream fDebugFile;


		/** Methoden */
		void LoadTdcCalibrationTables();
		void readTextFileTo2DVectorDouble(std::vector<std::vector<double>>& lines, const TString& filename);
		double ComputeFineTime(int tdcID,
				unsigned short channel,
				unsigned short clock200MHz,
				unsigned short flag400MHz,
				unsigned short fpgaCounter);




        ClassDefOverride(TQClamDagobertUnpackProc,1)
};

#endif

