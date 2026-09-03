#ifndef TQCLAMUNPACKPROC_H
#define TQCLAMUNPACKPROC_H

#include "TGo4EventProcessor.h"
#include <vector>
#include <memory>
#include <fstream>

#include "elb_vfb6_unpacker.h"

class TQClamRawEvent;
class TQClamUnpackParameter;

class TQClamUnpackProc : public TGo4EventProcessor
{
	public:
		TQClamUnpackProc();
		TQClamUnpackProc(const char* name);
		virtual ~TQClamUnpackProc();

		Bool_t BuildEvent(TGo4EventElement* dest) override;

		static constexpr double period200MHzInNanoSeconds = 5.0; // 200 MHz Clock
		static constexpr double period400MHzInNanoSeconds = 2.5; // 400 MHz Clock

		int fEvtCounter{0};
	private:
		void LoadTdcCalibrationTables();
		void readTextFileTo2DVectorDouble(std::vector<std::vector<double>>& lines, const TString& filename);
		double ComputeFineTime(int tdcID,
                                unsigned short channel,
                                unsigned short clock200MHz,
                                unsigned short flag400MHz,
                                unsigned short fpgaCounter);
	private:
		TQClamUnpackParameter* fUnpackPar{nullptr};

	private:
		bool fInElbBlock{false};
		std::vector<uint32_t> fElbBuffer;
		int currentTdcID;
		static constexpr size_t nVFB6_TDCs = 19;   // du hast 19 Module
		std::array<std::unique_ptr<ElbVfb6Unpacker>,nVFB6_TDCs> fVfb6TdcUnpacker;

		std::array<std::array<ElbVfb6Unpacker::CALIBRATION_TABLE,ElbVfb6Unpacker::nChannels>,nVFB6_TDCs> calibTables;

                std::array<std::vector<std::vector<double>>, nVFB6_TDCs> timeCalibrationTableFPGA;
                std::array<std::array<ElbVfb6Unpacker::CALIBRATION_TABLE, ElbVfb6Unpacker::nChannels>,
                        nVFB6_TDCs> calibTablesFromFiles;




		void DecodeElbBlock(const std::vector<uint32_t>& block, TQClamRawEvent* out,int tdcID);
		void DecodeElbBlock2(const std::vector<uint32_t>& block, TQClamRawEvent* out,int tdcID);
		void DecodeElbBlockNew(const std::vector<uint32_t>& block, TQClamRawEvent* out,int tdcID);

		bool firstZeroSeen[19]{false};

		std::ofstream fDebugFile;


	ClassDefOverride(TQClamUnpackProc,1)
};

#endif

