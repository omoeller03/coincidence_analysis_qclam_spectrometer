#ifndef TQCLAMEDGEMAPPINGPARAMETER_H
#define TQCLAMEDGEMAPPINGPARAMETER_H

#include "TGo4Parameter.h"
#include "TQClamConstants.h"
#include "Rtypes.h"


enum class EdgeType
{
        NONE = 0,
        LEADING = 1,
        TRAILING = 2
};


class TQClamEdgeMappingParameter : public TGo4Parameter
{
	public:
		TQClamEdgeMappingParameter();
		TQClamEdgeMappingParameter(const char* name);

		/** Max Dimensionen */
		static const Int_t MAX_TDC = QCLAM_MAX_TDC;
		static const Int_t MAX_CH = QCLAM_MAX_CH;

		/** - Voraussetznug an Hits - */
//		Bool_t needTdcTrigger = kFALSE; // Process Hits Only If TdcTrigger Exists

//		Bool_t invertEdges = kFALSE; // Swap: Leading <-> Trailing (Old Hardware-Bug)

		/** - Mapping - */
		Int_t leadingToTrailing[MAX_TDC][MAX_CH]; // Mapping: Leading-(tdc,ch) --> Trailing-(ch)
		Int_t trailingToLeading[MAX_TDC][MAX_CH]; // Mapping: Trailing-(tdc,ch) --> Leading-(ch)

                EdgeType edgeType[MAX_TDC][MAX_CH]; // Mapping: (tdc,ch) --> EdgeType (NONE/LEADING/TRAILING = 0/1/2)

		Int_t tdcTriggerChannel[MAX_TDC]; // Mapping: TDC --> Trigger-Kanal



		/** - Methoden - */
		int GetLeadingForTrailing(Int_t tdc, Int_t trailCh) const;

		int GetTrailingForLeading(Int_t tdc, Int_t leadCh) const;

                /** Mapping (tdc,ch) --> EdgeType */
                bool IsNone(int tdc, int ch) const;
                bool IsLeading(int tdc, int ch) const;
                bool IsTrailing(int tdc, int ch) const;

//		inline bool ProofHit(bool isTdcTrig) const
//		{
//			/** brauche keinen TdcTrigger */
//			if(!needTdcTrigger) { return true; }
//			
//			/** brauche TdcTrigger */
//			return isTdcTrig;
//		}

		bool IsTdcTrigger(Int_t tdc, Int_t ch) const;

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


	ClassDefOverride(TQClamEdgeMappingParameter,1);
};

#endif
