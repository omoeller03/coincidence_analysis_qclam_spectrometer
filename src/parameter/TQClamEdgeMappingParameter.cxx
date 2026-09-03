#include "parameter/TQClamEdgeMappingParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamEdgeMappingParameter)

//////////////////////////////////////////////////////////////////////

TQClamEdgeMappingParameter::TQClamEdgeMappingParameter()
	: TGo4Parameter("EdgeMappingParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamEdgeMappingParameter::TQClamEdgeMappingParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamEdgeParameter] Create %s", name);

//	needTdcTrigger = kTRUE; /** benoetige Tdc-Trigger fuer Hit-Erzeugung */
//	invertEdges = kFALSE; /** invertiere: leading <-> trailing */
	for(Int_t tdc=0; tdc<MAX_TDC; tdc++)
	{
		tdcTriggerChannel[tdc] = -1;

		for(Int_t ch=0; ch<MAX_CH; ch++)
		{
			leadingToTrailing[tdc][ch] = -1;
			trailingToLeading[tdc][ch] = -1;

                        /** EdgeType NONE/LEADING/TRAILING */
                        edgeType[tdc][ch] = EdgeType::NONE;
		}
	}
}


/** pruefe, ob (tdc,ch) ein tdc-trigger kanal ist */
bool TQClamEdgeMappingParameter::IsTdcTrigger(Int_t tdc, Int_t ch) const
{
	return tdcTriggerChannel[tdc] == ch;
}


int TQClamEdgeMappingParameter::GetTrailingForLeading(Int_t tdc, Int_t leadCh) const
{
	return leadingToTrailing[tdc][leadCh];
}


int TQClamEdgeMappingParameter::GetLeadingForTrailing(Int_t tdc, Int_t trailCh) const
{
	return trailingToLeading[tdc][trailCh];
}

/** Mapping (tdc,ch) --> EdgeType */
bool TQClamEdgeMappingParameter::IsNone(int tdc, int ch) const
{
        return edgeType[tdc][ch] == EdgeType::NONE;
}
bool TQClamEdgeMappingParameter::IsLeading(int tdc, int ch) const
{
        return edgeType[tdc][ch] == EdgeType::LEADING;
}
bool TQClamEdgeMappingParameter::IsTrailing(int tdc, int ch) const
{
        return edgeType[tdc][ch] == EdgeType::TRAILING;
}



/** - Update - */
Bool_t TQClamEdgeMappingParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamEdgeMappingParameter] UpdateFrom()");

	auto* other = dynamic_cast<TQClamEdgeMappingParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamEdgeMappingParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

//	needTdcTrigger = other->needTdcTrigger;
//	invertEdges = other->invertEdges;

	for(Int_t tdc=0; tdc<MAX_TDC; tdc++)
	{
		tdcTriggerChannel[tdc] = other->tdcTriggerChannel[tdc];

		for(Int_t ch=0; ch<MAX_CH; ch++)
		{
			leadingToTrailing[tdc][ch] = other->leadingToTrailing[tdc][ch];
			trailingToLeading[tdc][ch] = other->trailingToLeading[tdc][ch];

                        /** EdgeType NONE/LEADING/TRAILING */
                        edgeType[tdc][ch] = other->edgeType[tdc][ch];
		}
	}

	return kTRUE;
}
