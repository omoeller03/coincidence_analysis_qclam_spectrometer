#include "parameter/TQClamSourceParameter.h"
#include "TGo4Log.h"
#include "TQClamConstants.h"

ClassImp(TQClamSourceParameter)

//////////////////////////////////////////////////////////////////////

TQClamSourceParameter::TQClamSourceParameter()
	: TGo4UserSourceParameter("SourceParam")
{
	TGo4Log::Error("[TQClamSourceParameter] Dummy Konstruktor!");
}

//////////////////////////////////////////////////////////////////////

TQClamSourceParameter::TQClamSourceParameter(const char* name)
	: TGo4UserSourceParameter(name)
{
	TGo4Log::Info("[TQClamSourceParameter] Create %s", GetName());
	//if(!par->GetName().IsNull()) { filenames.push_back(par->GetName()); }

	file_directory = "";
	store_directory = "";
	
	fName = "";
	/*
	for(size_t n=0; n<MAX_FILES;n++)
	{
		fFiles[n] = "";
	}
	*/
	fFiles.clear();

	for(size_t i=0; i<MAX_LMD_FILES; i++)
	{
		fLmdFiles[i] = "";
	}
}

//////////////////////////////////////////////////////////////////////

TQClamSourceParameter::~TQClamSourceParameter()
{
	TGo4Log::Info("[TQClamSourceParameter] Delete %s", GetName());
}


//////////////////////////////////////////////////////////////////////


/** - Update - */
Bool_t TQClamSourceParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamSourceParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamSourceParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamSourceParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}


	fName = other->fName;
	/*
	for(size_t n=0; n<MAX_FILES; n++)
	{
		fFiles[n] = fFiles[n];
	}
	*/


	for(size_t i=0; i<MAX_LMD_FILES; i++)
	{
		fLmdFiles[i] = other->fLmdFiles[i];
	}


	fFiles = other->fFiles; /* TODO */
	fStreamServer = other->fStreamServer;
	fSourceType = other->fSourceType;

	return kTRUE;
}
