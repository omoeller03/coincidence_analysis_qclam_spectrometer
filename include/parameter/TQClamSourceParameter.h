#ifndef TQCLAMSOURCEPARAMETER_H
#define TQCLAMSOURCEPARAMETER_H

#include "TGo4UserSourceParameter.h"
#include "TQClamConstants.h"
#include "Rtypes.h"


class TQClamSourceParameter : public TGo4UserSourceParameter
{
	public:
		static const Int_t kQClamOfflineRoot = QCLAM_OFFLINE_ROOT; // SourceType OfflineRoot: 0

		static const Int_t kQClamOfflineLmd = QCLAM_OFFLINE_LMD; // SourceType OfflineLmd: 1
		static const Int_t kQClamMbsStream = QCLAM_MBS_STREAM; // SourceType MbsStream: 2
								       //
		static const Int_t kQClamDagobertOfflineLmd = QCLAM_DAGOBERT_OFFLINE_LMD; // SourceType OfflineLmd: 3
		static const Int_t kQClamDagobertMbsStream = QCLAM_DAGOBERT_MBS_STREAM; // SourceType MbsStream: 4

		/** nicht mehr benoetigt --> nicht mehr nutzbar*/
		static const Int_t kQClamOfflineRootDebug = QCLAM_OFFLINE_ROOT_DEBUG; // SourceType OfflineRoot: 10
		static const Int_t kQClamOfflineLmdDebug = QCLAM_OFFLINE_LMD_DEBUG; // SourceType OfflineLmd: 11

		static const Int_t MAX_LMD_FILES = QCLAM_MAX_FILES;

		TQClamSourceParameter();
		TQClamSourceParameter(const char* name);
		virtual ~TQClamSourceParameter();

		std::string file_directory;
		std::string store_directory;

		TString fLmdFiles[MAX_LMD_FILES];

		TString fName; // fuer --file --> not implemented 
		std::vector<TString> fFiles; // Input-Files TODO: TString[...] fFiles


		/** source typ */
		/* 0 -> TQClamOfflineRootSource
		 * 1 -> TQClamOfflineLmdSource
		 * ... weiteres? */
		Int_t fSourceType; // SourceType (OfflineRoot: 0 / OfflineLmd: 1 / MbsStream: 2)

		TString fStreamServer; // Stream-Server: LIPC-5



		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


	ClassDefOverride(TQClamSourceParameter, 1);
};

#endif
