#ifndef TQCLAMTRIGGERCALIBRATIONPARAMETER_H
#define TQCLAMTRIGGERCALIBRATIONPARAMETER_H

#include "TGo4Parameter.h"
#include "TQClamConstants.h"

#include <utility>

enum class TriggerType
{
        None,
        SzintPmtUp,
        SzintPmtDown,
        CherenkovPmt,
        CoincTrig, /** coincidence */
        QClamTrig, /** evtl. = CoincTrig */
        AccTrig, /** accepted */
        Clock,
        Livetime,
        MbsLivetime,
        FaradayCupChargeCounter
};

class TQClamTriggerCalibrationParameter : public TGo4Parameter
{
        public:
                TQClamTriggerCalibrationParameter();
                TQClamTriggerCalibrationParameter(const char* name);

                /** - Max Dimensionen -*/
                static const Int_t MAX_TDC = QCLAM_MAX_TDC; // Maximale TDC-Anzahl: 32
                static const Int_t MAX_CH = QCLAM_MAX_CH; // Maximale Anzahl Channel/TDC: 128

		/** Leading <-> Trailing vertauschen? */
		Bool_t invertEdges = kFALSE; // Vertausche: Leading <-> Trailing ('Alter' Hardware-Bug)

		/** -- Trigger-Voraussetzung fuer gueltiges Evt */
		Bool_t needTdcTrigger = kFALSE; // Verarbeite Hits nur, wenn Tdc-Trigger existiert

		Bool_t needSzintPmtUp = kFALSE; // Verarbeite nur Hits mit SzintPmtUp-Trigger zu Clustern etc.
		Bool_t needSzintPmtDown = kFALSE; // Verarbeite nur Hits mit SzintPmtDown-Trigger zu Clustern etc.
		Bool_t needCherenkovPmt = kFALSE; // Verarbeite nur Hits mit CherenkovPmt-Trigger zu Clustern etc.
		Bool_t needCoincTrig = kFALSE; // Verarbeite nur Hits mit Coincidence-Trigger zu Clustern etc.
		Bool_t needAccTrig = kFALSE; // Verarbeite nur Hits mit Accepted-Trigger zu Clustern etc.
		Bool_t needClock = kFALSE; // Verarbeite nur Hits mit Clock-Trigger zu Clustern etc.
		Bool_t needLivetime = kFALSE; // Verarbeite nur Hits mit Livetime-Trigger zu Clustern etc.
		Bool_t needMbsLivetime = kFALSE; // Verarbeite nur Hits mit MbsLivetime-Trigger zu Clustern etc.
		Bool_t needFaradayCupChargeCounter = kFALSE; // Verarbeite nur Hits mit FaradayCupChargeCounter-Trigger zu Clustern etc.




                /** --- MAPPING --- */

                /** szint pmt-up */
                Int_t szintPmtUp_tdc = -1; // Mapping SzintPmtUp: TDC (initialisiert mit -1)
                Int_t szintPmtUp_ch = -1; // Mapping SzintPmtDown: Channel (initialisiert mit -1)

                /** szint pmt-down */
                Int_t szintPmtDown_tdc = -1; // Mapping SzintPmtDown: TDC (initialisiert mit -1)
                Int_t szintPmtDown_ch = -1; // Mapping SzintPmtDown: Channel (initialisiert mit -1)

                /** cherenkov pmt */
                Int_t cherenkovPmt_tdc = -1; // Mapping CherenkovPmt: TDC (initialisiert mit -1)
                Int_t cherenkovPmt_ch = -1; // Mapping CherenkovPmt: Channel (initialisiert mit -1)

                /** coinc trig */
                Int_t coincTrig_tdc = -1; // Mapping Coincidence-Trigger: TDC (initialisiert mit -1)
                Int_t coincTrig_ch = -1; // Mapping Coincidence-Trigger: Channel (initialisiert mit -1)

                /** qclam trig */
                Int_t qclamTrig_tdc = -1; // Mapping QClam-Trigger: TDC (initialisiert mit -1)
                Int_t qclamTrig_ch = -1; // Mapping QClam-Trggier: Channel (initialisiert mit -1)

                /** accepted trig */
                Int_t accTrig_tdc = -1; // Mapping Accepted-Trigger: TDC (initialisiert mit -1)
                Int_t accTrig_ch = -1; // Mapping Accepted-Trigger: Channel (initialisiert mit -1)

                /** clock */
                Int_t clock_tdc = -1; // Mapping Clock: TDC (initialisiert mit -1)
                Int_t clock_ch = -1; // Mapping Clock: Channel (initialisiert mit -1)

                /** livetime */
                Int_t livetime_tdc = -1; // Mapping Livetime: TDC (initialisiert mit -1)
                Int_t livetime_ch = -1; // Mapping Livetime: Channel (initialisiert mit -1)

                /** mbsLivetime */
                Int_t mbsLivetime_tdc = -1; // Mapping MbsLivetime: TDC (initialisiert mit -1)
                Int_t mbsLivetime_ch = -1; // Mapping MbsLivetime: Channel (initialisiert mit -1)

                /** faradayCupChargeCounter */
                Int_t faradayCupChargeCounter_tdc = -1; // Mapping FaradayCupChargeCounter: TDC (initialisiert mit -1)
                Int_t faradayCupChargeCounter_ch = -1; // Mapping FaradayCupChargeCounter: Channel (initialisiert mit -1)


		/** - Methoden Trigger-Voraussetzung - */
                inline bool ProofHit(bool isTdcTrig) const
                {
                        /** brauche keinen TdcTrigger */
                        if(!needTdcTrigger) { return true; }

                        /** brauche TdcTrigger */
                        return isTdcTrig;
                }



		inline bool ProofEvent(bool isSzintPmtUp, bool isSzintPmtDown, bool isCherenkovPmt,
				bool isCoincTrig, bool isAccTrig, bool isClock, bool isLivetime,
				bool isMbsLivetime, bool isFaradayCupChargeCounter) const
		{
			if(needSzintPmtUp) /** brauche szintPmtUp? */
			{
				/** habe SzintPmtUp? */
				if(!isSzintPmtUp) { return false; }
			}
			if(needSzintPmtDown) /** brauche SzintPmtDown? */
			{
				if(!isSzintPmtDown) { return false; }
			}
			if(needCherenkovPmt) /** brauche CherenkovPmt */
			{
				if(!isCherenkovPmt) { return false; }
			}
			if(needCoincTrig) /** brauche CoincTrig? */
			{
				if(!isCoincTrig) { return false; }
			}
			if(needAccTrig) /** brauche AccTrig? */
			{
				if(!isAccTrig) { return false; }
			}
			if(needClock) /** brauche Clock? */
			{
				if(!isClock) { return false; }
			}
			if(needLivetime) /** brauche Livetime? */
			{
				if(!isLivetime) { return false; }
			}
			if(needMbsLivetime) /** brauche MbsLivetime? */
			{
				if(!isMbsLivetime) { return false; }
			}
			if(needFaradayCupChargeCounter) /** brauche FaradayCupChargeCounter? */
			{
				if(!isFaradayCupChargeCounter) { return false; }
			}
			return true;
		}
						

		/** - Methoden Mapping - */
                bool IsTrigger(int tdc, int ch) const;
                TriggerType GetTriggerType(int tdc, int ch) const;

		/** Pruefe, ob entsprechender Leading-Kanal vorliegt */
		inline bool IsSzintPmtUp(int tdc, int ch) const
		{
			return (tdc == szintPmtUp_tdc && ch == szintPmtUp_ch);
		}
		inline bool IsSzintPmtDown(int tdc, int ch) const
		{
			return (tdc == szintPmtDown_tdc && ch == szintPmtDown_ch);
		}
		inline bool IsCherenkovPmt(int tdc, int ch) const
		{
			return (tdc == cherenkovPmt_tdc && ch == cherenkovPmt_ch);
		}
		inline bool IsCoincTrig(int tdc, int ch) const
		{
			return (tdc == coincTrig_tdc && ch == coincTrig_ch);
		}
		inline bool IsAccTrig(int tdc, int ch) const
		{
			return (tdc == accTrig_tdc && ch == accTrig_ch);
		}
		inline bool IsClock(int tdc, int ch) const
		{
			return (tdc == clock_tdc && ch == clock_ch);
		}
		inline bool IsLivetime(int tdc, int ch) const
		{
			return (tdc == livetime_tdc && ch == livetime_ch);
		}
		inline bool IsMbsLivetime(int tdc, int ch) const
		{
			return (tdc == mbsLivetime_tdc && ch == mbsLivetime_ch);
		}
		inline bool IsFaradayCupChargeCounter(int tdc, int ch) const
		{
			return (tdc == faradayCupChargeCounter_tdc && ch == faradayCupChargeCounter_ch);
		}

		/** --- UMRECHNUNG Scalerzeit --- */

                /** Zeit nach Offset-Anwendung invertieren: *(-1) */
                Bool_t invert = kFALSE;

                /** Offsets */
                Double_t globOffset = 0.0;
                Double_t tdcOffset[MAX_TDC];
                Double_t tdcChannelOffset[MAX_TDC][MAX_CH];


		/** - Methoden Umrechnung Scalerzeit - */
                double CorrectTime(Double_t rawT, Int_t tdc, Int_t ch) const;



		/** - Update - */
                Bool_t UpdateFrom(TGo4Parameter* rhs) override;



        ClassDefOverride(TQClamTriggerCalibrationParameter,1);
};


#endif

