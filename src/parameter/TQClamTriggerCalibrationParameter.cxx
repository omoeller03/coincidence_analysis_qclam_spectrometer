#include "parameter/TQClamTriggerCalibrationParameter.h"

#include "TGo4Log.h"

enum class ScalerType;

ClassImp(TQClamTriggerCalibrationParameter)

//////////////////////////////////////////////////////////////////////

TQClamTriggerCalibrationParameter::TQClamTriggerCalibrationParameter()
        : TGo4Parameter("TriggerCalibParam")
{}

//////////////////////////////////////////////////////////////////////

TQClamTriggerCalibrationParameter::TQClamTriggerCalibrationParameter(const char* name)
        : TGo4Parameter(name)
{
        TGo4Log::Info("[TQClamTriggerParameter] Create %s", name);


	/** -- Trigger-Voraussetzung fuer gueltiges Evt */
	needTdcTrigger = kTRUE;

	needSzintPmtUp = kFALSE;
	needSzintPmtDown = kFALSE;
	needCherenkovPmt = kFALSE;
	needCoincTrig = kFALSE;
	needAccTrig = kFALSE;
	needClock = kFALSE;
	needLivetime = kFALSE;
	needMbsLivetime = kFALSE;
	needFaradayCupChargeCounter = kFALSE;


	/** Leading <-> Trailing vertauschen? */
	invertEdges = kFALSE;

	/** reset mapping */
	szintPmtUp_tdc = -1;
	szintPmtUp_ch = -1;

	szintPmtDown_tdc = -1;
	szintPmtDown_ch = -1;

	cherenkovPmt_tdc = -1;
	cherenkovPmt_ch = -1;

	coincTrig_tdc = -1;
	coincTrig_ch = -1;

	qclamTrig_tdc = -1;
	qclamTrig_ch = -1;

	accTrig_tdc = -1;
	accTrig_ch = -1;

	clock_tdc = -1;
	clock_ch = -1;

	livetime_tdc = -1;
	livetime_ch = -1;

	mbsLivetime_tdc = -1;
	mbsLivetime_ch = -1;

	faradayCupChargeCounter_tdc = -1;
	faradayCupChargeCounter_ch = -1;


	/** reset zeit-offsets */
        globOffset = 0.0;

        for(int tdc=0; tdc<MAX_TDC; tdc++)
        {
                tdcOffset[tdc] = 0.0;

                for(int ch = 0; ch<MAX_CH; ch++)
                {
                        tdcChannelOffset[tdc][ch] = 0.0;
                }
        }
}

/** - Methoden Mapping - */

/** pruefen, ob ein Scaler vorliegt */
bool TQClamTriggerCalibrationParameter::IsTrigger(int tdc, int ch) const
{
        return GetTriggerType(tdc,ch) != TriggerType::None;
}


/** TriggerTyp holen */
TriggerType TQClamTriggerCalibrationParameter::GetTriggerType(int tdc, int ch) const
{
        if(tdc == szintPmtUp_tdc && ch == szintPmtUp_ch) { return TriggerType::SzintPmtUp; }
        else if(tdc == szintPmtDown_tdc && ch == szintPmtDown_ch) { return TriggerType::SzintPmtDown; }
        else if(tdc == cherenkovPmt_tdc && ch == cherenkovPmt_ch) { return TriggerType::CherenkovPmt; }
        else if(tdc == coincTrig_tdc && ch == coincTrig_ch) { return TriggerType::CoincTrig; }
        else if(tdc == qclamTrig_tdc && ch == qclamTrig_ch) { return TriggerType::QClamTrig; }
        else if(tdc == accTrig_tdc && ch == accTrig_ch) { return TriggerType::AccTrig; }
        else if(tdc == clock_tdc && ch == clock_ch) { return TriggerType::Clock; }
        else if(tdc == livetime_tdc && ch == livetime_ch) { return TriggerType::Livetime; }
        else if(tdc == mbsLivetime_tdc && ch == mbsLivetime_ch) { return TriggerType::MbsLivetime; }
        else if(tdc == faradayCupChargeCounter_tdc && ch == faradayCupChargeCounter_ch) { return TriggerType::FaradayCupChargeCounter; }

        return TriggerType::None;
}


/** - Methoden Scalerzeit-Umrechnung - */

/** Umrechnung: rohe Scalerzeit --> korrigierte Scalerzeit */
double TQClamTriggerCalibrationParameter::CorrectTime(Double_t rawT, Int_t tdc, Int_t ch) const
{
        if(invert) { return -1 * (rawT + tdcOffset[tdc] + tdcChannelOffset[tdc][ch]); }

        return rawT + tdcOffset[tdc] + tdcChannelOffset[tdc][ch];
}



/** - Update - */
Bool_t TQClamTriggerCalibrationParameter::UpdateFrom(TGo4Parameter* rhs)
{
        TGo4Log::Info("[TQClamTriggerCalibrationParameter] UpdateFrom()");

        auto* other = dynamic_cast<TQClamTriggerCalibrationParameter*>(rhs);
        if(!other)
        {
                TGo4Log::Error("[TQClamTriggerCalibrationParameter] UpdateFrom() nicht erfolgreich!");
                return kFALSE;
        }

	/** update conditions */
	needTdcTrigger = other->needTdcTrigger;

	needSzintPmtUp = other->needSzintPmtUp;
	needSzintPmtDown = other->needSzintPmtDown;
	needCherenkovPmt = other->needCherenkovPmt;
	needCoincTrig = other->needCoincTrig;
	needAccTrig = other->needAccTrig;
	needClock = other->needClock;
	needLivetime = other->needLivetime;
	needMbsLivetime = other->needMbsLivetime;
	needFaradayCupChargeCounter = other->needFaradayCupChargeCounter;


	invertEdges = other->invertEdges;


	/** update mapping */
	szintPmtUp_tdc = other->szintPmtUp_tdc;
	szintPmtUp_ch = other->szintPmtUp_ch;

	szintPmtDown_tdc = other->szintPmtDown_tdc;
	szintPmtDown_ch = other->szintPmtDown_ch;

	cherenkovPmt_tdc = other->cherenkovPmt_tdc;
	cherenkovPmt_ch = other->cherenkovPmt_ch;

	coincTrig_tdc = other->coincTrig_tdc;
	coincTrig_ch = other->coincTrig_ch;

	qclamTrig_tdc = other->qclamTrig_tdc;
	qclamTrig_ch = other->qclamTrig_ch;

	accTrig_tdc = other->accTrig_tdc;
	accTrig_ch = other->accTrig_ch;

	clock_tdc = other->clock_tdc;
	clock_ch = other->clock_ch;

	livetime_tdc = other->livetime_tdc;
	livetime_ch = other->livetime_ch;

	mbsLivetime_tdc = other->mbsLivetime_tdc;
	mbsLivetime_ch = other->mbsLivetime_ch;

	faradayCupChargeCounter_tdc = other->faradayCupChargeCounter_tdc;
	faradayCupChargeCounter_ch = other->faradayCupChargeCounter_ch;

	/** update scalerzeit-umrechnung */
        invert = other->invert;
        globOffset = other->globOffset;

        for(Int_t tdc=0; tdc<MAX_TDC; tdc++)
        {
                tdcOffset[tdc] = other->tdcOffset[tdc];

                for(Int_t ch=0; ch<MAX_CH; ch++)
                {
                        tdcChannelOffset[tdc][ch] = other->tdcChannelOffset[tdc][ch];
                }
        }

        return kTRUE;
}

