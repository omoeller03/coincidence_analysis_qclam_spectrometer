#ifndef TQCLAMOFFLINELMDSOURCE_H
#define TQCLAMOFFLINELMDSOURCE_H

#include "TGo4EventSource.h"
#include <vector>
#include <atomic>
#include <string>

#include "elb_vfb6_unpacker.h"

class TGo4UserSourceParameter;
class TClass;

class TQClamOfflineLmdSource : public TGo4EventSource
{
	public:
		TQClamOfflineLmdSource();
		TQClamOfflineLmdSource(TGo4UserSourceParameter* sourcepar);
		virtual ~TQClamOfflineLmdSource();

		virtual Int_t  Open();
		virtual Int_t  Close();
		Bool_t BuildEvent(TGo4EventElement* dest) override;
		Bool_t CheckEventClass(TClass* cl) override;

	private:
		std::string fFilename;
		int fEventNumber {0};

		// MBS-API Pointer (opaque)
		void* fChannel    {nullptr};  // s_evt_channel*
		void* fFileHeader {nullptr};  // s_filhe*
		void* fBufHeader  {nullptr};  // s_bufhe*

		ElbVfb6Unpacker fVfb6;
		ElbVfb6Unpacker::CALIBRATION_TABLE fCalib{};

		std::atomic_bool fStop {false};

	ClassDefOverride(TQClamOfflineLmdSource,1);
};

#endif

