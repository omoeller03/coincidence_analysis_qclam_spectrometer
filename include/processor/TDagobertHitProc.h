#ifndef TDAGOBERTHITPROC_H
#define TDAGOBERTHITPROC_H
/*
 * Belegung der Kanaele:
 *
 * Modul	Kanal		Belegung
 *
 * 0		0		Koinzidenz-Trigger alle LaBr (logisches Oder)
 * 0		1		Szint Down QClam
 * 0		2		Szint Up QClam
 * 0		3		Clock (unwichtig)
 * 0		4		Scaler - logisches ODER Alle LaBr --> WICHTIG
 * 0		5		Scaler - Downscaled QClamTrigger --> UNWICHTIG (alt, Gerhart)
 * 0		6		Scaler - logisches ODER Alle LaBr ausserhalb Totzeit --> WICHTIG
 * 0		7		Scaler - UserOutOr --> UNWICHTIG
 * 0		8		LaBr Det 0 (Tower 1, Det unten)
 * 0		9		LaBr Det 1 (Tower 1, Det mitte)
 * 0		10		LaBr Det 2 (Tower 1, Det oben)
 * 0		11		LaBr Det 3 (Tower 2, Det unten)
 * 0		12		LaBr Det 4 (Tower 2, Det mitte)
 * 0		13		LaBr Det 5 (Tower 2, Det oben)
 * 0		14		unbelegt (?)
 * 0		15		Plastic Szint (waehrend 2025)
 *
 * 1		0		
 * 1		1
 * 1		2
 * 1		3
 * 1		4
 * 1		5
 * 1		6
 * 1		7
 * 1		8		LaBr Det 6 (Tower 3, Det unten)
 * 1		9		LaBr Det 7 (Tower 3, Det mitte)
 * 1		10		LaBr Det 8 (Tower 3, Det oben)
 * 1		11		LaBr Det Vorwaertswinkel
 * 1		12		
 * 1		13
 * 1		14	
 * 1		15		Cherenkov Vorwaertswinkel
 *
 *
 * Zeit von Detektoren relativ zu Mean(SzintUp,SzintDown)
 * Zeit von Detektoren relativ zu Koinzidenz-Trigger (Modul 0 Kanal 0)
 */
#include "TGo4EventProcessor.h"
#include <array>

class TH1;
class TH2;
class TGraph;
class TGo4RollingGraph;
class TGo4Picture;

struct DagHit;
struct RawGammaHit;
struct RawDagobertScaler;


/** parameter */
class TQClamTcpParameter;
class TDagobertCalibrationParameter;
class TQClamHistogramParameter;

/** evt */
class TDagobertHitEvent;





struct ChannelState
{
	uint64_t prevRaw = 0;
	uint64_t prevAccepted = 0;
	uint64_t prevVeto = 0;
	double prevScalerTime = 0.;
	double rateRaw = 0.;
	double rateAccepted = 0.;
	double deadtime = 0.;
};


struct ScalerState
{
	uint32_t prevClock = 0;
	bool initialized = false;

	std::vector<uint32_t> prevRaw;
	std::vector<uint32_t> prevAccepted;
};



class TDagobertHitProc : public TGo4EventProcessor
{
	public:
		TDagobertHitProc();
		TDagobertHitProc(const char* name);
		virtual ~TDagobertHitProc();
		Bool_t BuildEvent(TGo4EventElement* dest) override;


	public:
		static constexpr size_t N_MODULES = 2; /** in config! */
		static constexpr size_t N_CHANNELS = 16; /** in config! */


	private:
		/** Varaiblen */
		double CYCLE_LENGTH = 4.0; /** 4 ns pro Takt --> in parameter */
		std::array<std::array<ChannelState,N_CHANNELS>,N_MODULES> fChan; // ChannelState
		ScalerState fScalerState[2];

		// time
		// TODO save event time and use it for rate calculation

		Bool_t fbTcpIsOpen;
		Int_t fTcpSocket;
		std::string fTcpBuffer;

		ULong64_t fStartTime{0};
		Double_t fGraphTime{0.};
		Bool_t fFirstEventTime{kTRUE};

		//double fSumLaBr0Accepted, fSumLaBr1Accepted, fSumLaBr2Accepted, fSumLaBr3Accepted, fSumLaBr4Accepted, fSumLaBr5Accepted, fSumLaBr6Accepted, fSumLaBr7Accepted, fSumLaBr8Accepted;
		//double fSumLaBr0Deadtime, fSumLaBr1Deadtime, fSumLaBr2Deadtime,fSumLaBr3Deadtime,fSumLaBr4Deadtime,fSumLaBr5Deadtime,fSumLaBr6Deadtime,fSumLaBr7Deadtime,fSumLaBr8Deadtime;
		std::array<double, 9> fSumLaBrAccepted{};
		std::array<double, 9> fSumLaBrRaw{};
		std::array<double, 9> fSumLaBrDeadtime{};
		double fSumForwardAccepted, fSumForwardDeadtime, fSumForwardRaw;
		double fSumCherenkovAccepted, fSumCherenkovDeadtime, fSumCherenkovRaw;
		double fSumPlasticAccepted, fSumPlasticDeadtime, fSumPlasticRaw;



	private:
		/** --- Parameter --- */
		TQClamTcpParameter* fTcpPar{nullptr};
		TDagobertCalibrationParameter* fDagoCaliPar{nullptr};
		TQClamHistogramParameter* fHistoPar{nullptr};


		/** --- Histogramme --- */
		/** LaBr-Detektoren Energie, unkalibriert*/
		TH1* fHRawEnergyLaBr0{nullptr};
		TH1* fHRawEnergyLaBr1{nullptr};
		TH1* fHRawEnergyLaBr2{nullptr};
		TH1* fHRawEnergyLaBr3{nullptr};
		TH1* fHRawEnergyLaBr4{nullptr};
		TH1* fHRawEnergyLaBr5{nullptr};
		TH1* fHRawEnergyLaBr6{nullptr};
		TH1* fHRawEnergyLaBr7{nullptr};
		TH1* fHRawEnergyLaBr8{nullptr};
		/** Vorwaerts-LaBr Energie unkalibriert*/
		TH1* fHRawEnergyLaBrForward{nullptr};

		/** LaBr-Detektoren Energie-kalibriert */
		TH1* fHCaliEnergyLaBr0{nullptr};
		TH1* fHCaliEnergyLaBr1{nullptr};
		TH1* fHCaliEnergyLaBr2{nullptr};
		TH1* fHCaliEnergyLaBr3{nullptr};
		TH1* fHCaliEnergyLaBr4{nullptr};
		TH1* fHCaliEnergyLaBr5{nullptr};
		TH1* fHCaliEnergyLaBr6{nullptr};
		TH1* fHCaliEnergyLaBr7{nullptr};
		TH1* fHCaliEnergyLaBr8{nullptr};
		/** Vorwaerts-LaBr Energie-kalibriert*/
		TH1* fHCaliEnergyLaBrForward{nullptr};
		/** timing relativ zu Mean(SzintUp,SzintDown) */
		TH1* fHLaBr0VsMeanSzint{nullptr};
		TH1* fHLaBr1VsMeanSzint{nullptr};
		TH1* fHLaBr2VsMeanSzint{nullptr};
		TH1* fHLaBr3VsMeanSzint{nullptr};
		TH1* fHLaBr4VsMeanSzint{nullptr};
		TH1* fHLaBr5VsMeanSzint{nullptr};
		TH1* fHLaBr6VsMeanSzint{nullptr};
		TH1* fHLaBr7VsMeanSzint{nullptr};
		TH1* fHLaBr8VsMeanSzint{nullptr};
		/** timing relativ zur ODER LaBr-Koinzidenz */
		TH1* fHLaBr0VsCoinc{nullptr};
		TH1* fHLaBr1VsCoinc{nullptr};
		TH1* fHLaBr2VsCoinc{nullptr};
		TH1* fHLaBr3VsCoinc{nullptr};
		TH1* fHLaBr4VsCoinc{nullptr};
		TH1* fHLaBr5VsCoinc{nullptr};
		TH1* fHLaBr6VsCoinc{nullptr};
		TH1* fHLaBr7VsCoinc{nullptr};
		TH1* fHLaBr8VsCoinc{nullptr};
		/** timing vorwaertsdetektoren */
		TH1* fHForwardLaBrVsMeanSzint{nullptr};
		TH1* fHForwardLaBrVsCoinc{nullptr};
		TH1* fHCherenkovVsMeanSzint{nullptr};
		TH1* fHCherenkovVsCoinc{nullptr};
		TH1* fHPlasticVsMeanSzint{nullptr};
		TH1* fHPlasticVsCoinc{nullptr};
		/** energy-shift: Energie LaBr vs. Time */
		TH2* fHLaBr0EnergyVsTime;
		TH2* fHLaBr1EnergyVsTime;
		TH2* fHLaBr2EnergyVsTime;
		TH2* fHLaBr3EnergyVsTime;
		TH2* fHLaBr4EnergyVsTime;
		TH2* fHLaBr5EnergyVsTime;
		TH2* fHLaBr6EnergyVsTime;
		TH2* fHLaBr7EnergyVsTime;
		TH2* fHLaBr8EnergyVsTime;
		/** Summen Histogramme */
		TH1* fHCaliEnergyLaBrSum{nullptr};
		TH1* fHLaBrVsMeanSzintSum{nullptr};
		TH1* fHLaBrVsCoincSum{nullptr};



		/** --- TGraphen --- */
		// LaBr 0..8 Accepted
		TGraph* fGLaBr0Accepted{nullptr};
		TGraph* fGLaBr1Accepted{nullptr};

		TGraph* fGLaBr2Accepted{nullptr};
		TGraph* fGLaBr3Accepted{nullptr};
		TGraph* fGLaBr4Accepted{nullptr};
		TGraph* fGLaBr5Accepted{nullptr};
		TGraph* fGLaBr6Accepted{nullptr};
		TGraph* fGLaBr7Accepted{nullptr};
		TGraph* fGLaBr8Accepted{nullptr};
		
		// LaBr 0..8 Raw
		TGraph* fGLaBr0Raw{nullptr};
		TGraph* fGLaBr1Raw{nullptr};

		TGraph* fGLaBr2Raw{nullptr};
		TGraph* fGLaBr3Raw{nullptr};
		TGraph* fGLaBr4Raw{nullptr};
		TGraph* fGLaBr5Raw{nullptr};
		TGraph* fGLaBr6Raw{nullptr};
		TGraph* fGLaBr7Raw{nullptr};
		TGraph* fGLaBr8Raw{nullptr};
		

		// LaBr 0..8 Deadtime
		TGraph* fGLaBr0Deadtime{nullptr};
		TGraph* fGLaBr1Deadtime{nullptr};
		TGraph* fGLaBr2Deadtime{nullptr};
		TGraph* fGLaBr3Deadtime{nullptr};
		TGraph* fGLaBr4Deadtime{nullptr};
		TGraph* fGLaBr5Deadtime{nullptr};
		TGraph* fGLaBr6Deadtime{nullptr};
		TGraph* fGLaBr7Deadtime{nullptr};
		TGraph* fGLaBr8Deadtime{nullptr};
		
		// Forward LaBr
		TGraph* fGForwardLaBrAccepted{nullptr};
		TGraph* fGForwardLaBrDeadtime{nullptr};
		TGraph* fGForwardLaBrRaw{nullptr};
		
		// Cherenkov
		TGraph* fGCherenkovAccepted{nullptr};
		TGraph* fGCherenkovDeadtime{nullptr};
		TGraph* fGCherenkovRaw{nullptr};
	
		// Plastic
		TGraph* fGPlasticAccepted{nullptr};
		TGraph* fGPlasticDeadtime{nullptr};
		TGraph* fGPlasticRaw{nullptr};





		/** --- Rolling Graphen --- */
		// LaBr 0..8 Accepted
		TGo4RollingGraph* fGRLaBr0Accepted{nullptr};
		TGo4RollingGraph* fGRLaBr1Accepted{nullptr};

		TGo4RollingGraph* fGRLaBr2Accepted{nullptr};
		TGo4RollingGraph* fGRLaBr3Accepted{nullptr};
		TGo4RollingGraph* fGRLaBr4Accepted{nullptr};
		TGo4RollingGraph* fGRLaBr5Accepted{nullptr};
		TGo4RollingGraph* fGRLaBr6Accepted{nullptr};
		TGo4RollingGraph* fGRLaBr7Accepted{nullptr};
		TGo4RollingGraph* fGRLaBr8Accepted{nullptr};
		
		// LaBr 0..8 Raw
		TGo4RollingGraph* fGRLaBr0Raw{nullptr};
		TGo4RollingGraph* fGRLaBr1Raw{nullptr};

		TGo4RollingGraph* fGRLaBr2Raw{nullptr};
		TGo4RollingGraph* fGRLaBr3Raw{nullptr};
		TGo4RollingGraph* fGRLaBr4Raw{nullptr};
		TGo4RollingGraph* fGRLaBr5Raw{nullptr};
		TGo4RollingGraph* fGRLaBr6Raw{nullptr};
		TGo4RollingGraph* fGRLaBr7Raw{nullptr};
		TGo4RollingGraph* fGRLaBr8Raw{nullptr};
		

		// LaBr 0..8 Deadtime
		TGo4RollingGraph* fGRLaBr0Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr1Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr2Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr3Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr4Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr5Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr6Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr7Deadtime{nullptr};
		TGo4RollingGraph* fGRLaBr8Deadtime{nullptr};
		
		// Forward LaBr
		TGo4RollingGraph* fGRForwardLaBrAccepted{nullptr};
		TGo4RollingGraph* fGRForwardLaBrDeadtime{nullptr};
		TGo4RollingGraph* fGRForwardLaBrRaw{nullptr};
		
		// Cherenkov
		TGo4RollingGraph* fGRCherenkovAccepted{nullptr};
		TGo4RollingGraph* fGRCherenkovDeadtime{nullptr};
		TGo4RollingGraph* fGRCherenkovRaw{nullptr};
	
		// Plastic
		TGo4RollingGraph* fGRPlasticAccepted{nullptr};
		TGo4RollingGraph* fGRPlasticDeadtime{nullptr};
		TGo4RollingGraph* fGRPlasticRaw{nullptr};

		TGo4RollingGraph* fGRLaBr0RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr1RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr2RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr3RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr4RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr5RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr6RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr7RawDivEnton{nullptr};
		TGo4RollingGraph* fGRLaBr8RawDivEnton{nullptr};
		
		// Counter für Intervall
		int fRateCounter = 0;
	
		double fFirstTime = -1.;
		/** Pictures */
		TGo4Picture* fPDagEnergyRaw{nullptr};
		TGo4Picture* fPDagEnergyCali{nullptr};
		TGo4Picture* fPDagEnergyShift{nullptr};
		TGo4Picture* fPTimePeak_MeanSzint{nullptr};
		TGo4Picture* fPTimePeak_Coinc{nullptr};
		TGo4Picture* fPSumSpectra{nullptr};
		TGo4Picture* fPForwardDetectors{nullptr};
		TGo4Picture* fPDagoRateMonitor{nullptr};
		TGo4Picture* fPDagoRateMonitorRollingGraph{nullptr};
	private:
		/** Methoden */
		DagHit BuildHit(const RawGammaHit& gh);
		double CorrectTimestamp(uint64_t ts,
				uint32_t maxMAW,
				uint32_t beforeMAW,
				uint32_t afterMAW) const;
		double EnergyFromGates(uint32_t gateG, uint32_t gateH) const;
		void ProcessScalerSnapshot(const RawDagobertScaler& sc);
		void FillHistograms(TDagobertHitEvent* evt);
		Bool_t ProcessTcpEvents(TDagobertHitEvent* evt);
		Int_t OpenTcpConnection();
		Int_t CloseTcpConnection();
		Bool_t ReadLine(std::string& out);

	ClassDefOverride(TDagobertHitProc,1);
};
#endif
