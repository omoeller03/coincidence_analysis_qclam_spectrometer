#include "parameter/TQClamDriftchamCalibrationParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamDriftchamCalibrationParameter)


//////////////////////////////////////////////////////////////////////

TQClamDriftchamCalibrationParameter::TQClamDriftchamCalibrationParameter()
        : TGo4Parameter("DriftchamCalibParam")
{}


//////////////////////////////////////////////////////////////////////

TQClamDriftchamCalibrationParameter::TQClamDriftchamCalibrationParameter(const char* name)
        : TGo4Parameter(name)
{
        TGo4Log::Info("[TQClamDriftchamCalibrationParameter] Create %s", name);

        /** parameter initialisieren */

	/** parameter Mapping */
        numTDC = -1;
	numLayer = -1;

        for(Int_t tdc=0; tdc<MAX_TDC; tdc++)
        {
                numCh[tdc] = -1;
                for(Int_t ch=0; ch<MAX_CH; ch++)
                {
                        layerMapLead[tdc][ch] = -1;
			layerMapTrail[tdc][ch] = -1;

                        wireMapLead[tdc][ch] = -1;
			wireMapTrail[tdc][ch] = -1;

                        /** Standard: Kanaele deaktiviert */
                        leadChannelDisabled[tdc][ch] = kTRUE;
			trailChannelDisabled[tdc][ch] = kTRUE;
                }
        }

        /** dito */
        for(Int_t layer=0; layer<MAX_LAYER; layer++)
        {
		numWire[layer] = -1;

                for(Int_t wire=0; wire<MAX_WIRE; wire++)
                {
                        /** Standard: Draehte deaktiviert */
                        wireDisabled[layer][wire] = kTRUE;
                }
        }




	/** parameter Driftzeit */
	useGlobOffset = kFALSE;
	useLayerOffset = kFALSE;
	useTdcOffset = kFALSE;
	useWireOffset = kFALSE;
	useWireOffsetFiles = kTRUE;
	useTdcChannelOffset = kFALSE;

        globOffset = 0.0;
        invert = kFALSE;

        for(Int_t tdc=0; tdc<MAX_TDC; tdc++)
        {
                tdcOffset[tdc] = 0.0;

                for(Int_t ch=0; ch<MAX_CH; ch++)
                {
                        tdcChannelOffset[tdc][ch] = 0.0;
                }
        }

        for(Int_t layer=0; layer<MAX_LAYER; layer++)
        {
                layerOffset[layer] = 0.0;

                for(Int_t wire=0; wire<MAX_WIRE; wire++)
                {
                        wireOffset[layer][wire] = 0.0;
                }
        }


	/** Parameter Time-Walk Korrektur */
	useTimeWalkCorrectedDrifttime = kFALSE;
	useGlobalTimeWalk = kTRUE;
	useWireSpecificTimeWalk = kTRUE;
	tref = 0.0;
	p0 = 0.0;
	p1 = 0.0;
	p2 = 0.0;
	//p3 = 0.0;


	for(int i=0; i<MAX_LAYER; i++)
	{
		for(int j=0; j<MAX_WIRE; j++)
		{
			p0_spec[i][j] = 0.;
			p1_spec[i][j] = 0.;
			p2_spec[i][j] = 0.;
			//p3_spec[i][j] = 0.;
		}
	}

}



/** -- Methoden Mapping -- */

/** Draht aktiviert? */
Bool_t TQClamDriftchamCalibrationParameter::IsWireActive(Int_t layer, Int_t wire) const
{
        return !wireDisabled[layer][wire];
}

/** Kanal aktiviert? */
Bool_t TQClamDriftchamCalibrationParameter::IsChannelActive(Int_t tdc, Int_t ch) const
{
        if(!HasMapping(tdc,ch)) { return kFALSE; }
        return !leadChannelDisabled[tdc][ch];
}

/** Existiert Mapping fuer (tdc,ch)? */
Bool_t TQClamDriftchamCalibrationParameter::HasMapping(Int_t tdc, Int_t ch) const
{
        return layerMapLead[tdc][ch] >= 0;
}

/** Fuehre Mapping durch (tdc,ch) --> (layer,wire) */
std::pair<Int_t,Int_t> TQClamDriftchamCalibrationParameter::GetWire(Int_t tdc, Int_t ch) const
{
        return { layerMapLead[tdc][ch], wireMapLead[tdc][ch] };
}





/** -- Methoden Driftzeit -- */

/** Driftzeit durch Offsets und Invertieren korrigieren */
double TQClamDriftchamCalibrationParameter::ApplyTimeOffsets(double rawT, int tdc, int ch, int layer, int wire) const
{
        if(invert)
        {
		double corrT = rawT;
		if(useGlobOffset) { corrT += globOffset; }
		if(useTdcOffset) { corrT += tdcOffset[tdc]; }
		if(useLayerOffset) { corrT += layerOffset[layer]; }
		if(useTdcChannelOffset) { corrT += tdcChannelOffset[tdc][ch]; }
		if(useWireOffset) { corrT += wireOffset[layer][wire]; }

                return -1 * corrT; // (rawT + globOffset + tdcOffset[tdc] + tdcChannelOffset[tdc][ch] + layerOffset[layer] + wireOffset[layer][wire]);
        }

	double corrT = rawT;
	if(useGlobOffset) { corrT += globOffset; }
	if(useTdcOffset) { corrT += tdcOffset[tdc]; }
	if(useLayerOffset) { corrT += layerOffset[layer]; }
	if(useTdcChannelOffset) { corrT += tdcChannelOffset[tdc][ch]; }
	if(useWireOffset) { corrT += wireOffset[layer][wire]; }

	return corrT;
        //return rawT + globOffset + tdcOffset[tdc] + tdcChannelOffset[tdc][ch] + layerOffset[layer] + wireOffset[layer][wire];
}


/** Time-Walk-Korrekturen auf Driftzeit anwenden */
double TQClamDriftchamCalibrationParameter::ApplyTimeWalkCorrection(int layer, int wireNumber, double rawT, double tot) const
{
	if(useGlobalTimeWalk)
	{
		//return (rawT - ( p0 + tot * (p1 + tot * ( p2 + tot * p3) )  - tref)); OLD AND WRONG, corrected wrong switching leading <-> trailing
		return (rawT - ( p0 * exp(-p1*tot) + p2 - tref));
	}
	else if(useWireSpecificTimeWalk)
	{
		//return (rawT - ( p0_spec[layer][wireNumber] + tot * (p1_spec[layer][wireNumber] + tot * ( p2_spec[layer][wireNumber] + tot * p3_spec[layer][wireNumber]) )  - tref));
		return (rawT - ( p0_spec[layer][wireNumber] * exp(-p1_spec[layer][wireNumber] * tot) + p2_spec[layer][wireNumber] )  - tref);
	}
	else
	{
		return rawT;
	}
}


/** -- Update -- */
Bool_t TQClamDriftchamCalibrationParameter::UpdateFrom(TGo4Parameter* rhs)
{
        TGo4Log::Info("[TQClamDriftchamCalibrationParameter] UpdateFrom()");
        auto* other = dynamic_cast<TQClamDriftchamCalibrationParameter*>(rhs);
        if(!other)
        {
                TGo4Log::Error("[TQClamDriftchamCalibrationParameter] UpdateFrom() nicht erfolgreich!");
                return kFALSE;
        }

	/** -- Mapping -- */

        numTDC = other->numTDC;
	numLayer = other->numLayer;

        for(Int_t tdc=0; tdc<MAX_TDC; tdc++)
        {
                numCh[tdc] = other->numCh[tdc];

                for(Int_t ch=0; ch<MAX_CH; ch++)
                {
                        layerMapLead[tdc][ch] = other->layerMapLead[tdc][ch];
			layerMapTrail[tdc][ch] = other->layerMapTrail[tdc][ch];

                        wireMapLead[tdc][ch] = other->wireMapLead[tdc][ch];
			wireMapTrail[tdc][ch] = other->wireMapTrail[tdc][ch];

                        /** Standard: Kanaele deaktiviert */
                        leadChannelDisabled[tdc][ch] = other->leadChannelDisabled[tdc][ch];
			trailChannelDisabled[tdc][ch] = other->trailChannelDisabled[tdc][ch];
                }
        }

        /** dito */
        for(Int_t layer=0; layer<MAX_LAYER; layer++)
        {
		numWire[layer] = other->numWire[layer];
		offsetFiles[layer] = other->offsetFiles[layer];

                for(Int_t wire=0; wire<MAX_WIRE; wire++)
                {
                        /** Standard: Draehte deaktiviert */
                        wireDisabled[layer][wire] = other->wireDisabled[layer][wire];
                }
        }


	/** -- Driftzeit-Offsets in ns -- */

	useGlobOffset = other->useGlobOffset;
	useTdcOffset = other->useTdcOffset;
	useLayerOffset = other->useLayerOffset;
	useTdcChannelOffset = other->useTdcChannelOffset;
	useWireOffsetFiles = other->useWireOffsetFiles;
	useWireOffset = other->useWireOffset;

        globOffset = other->globOffset;
        invert = other->invert;

        for(Int_t tdc=0; tdc<MAX_TDC; tdc++)
        {
                tdcOffset[tdc] = other->tdcOffset[tdc];

                for(Int_t ch=0; ch<MAX_CH; ch++)
                {
                        tdcChannelOffset[tdc][ch] = other->tdcChannelOffset[tdc][ch];
                }
        }

        for(Int_t layer=0; layer<MAX_LAYER; layer++)
        {
                layerOffset[layer] = other->layerOffset[layer];

                for(Int_t wire=0; wire<MAX_WIRE; wire++)
                {
                        wireOffset[layer][wire] = other->wireOffset[layer][wire];
                }
        }

        return kTRUE;
}

