#ifndef TDAGOBERTCALIBRATIONPARAMETER_H
#define TDAGOBERTCALIBRATIONPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"
#include "TQClamConstants.h"

class TDagobertCalibrationParameter : public TGo4Parameter
{
	public:
		TDagobertCalibrationParameter();
		TDagobertCalibrationParameter(const char* name);

		static constexpr size_t N_DETECTORS = DAGOBERT_NUMBER_DETECTORS; // Anzahl Detektoren (gesetzt in TQClamConstants)

		Double_t minCoincTimeWin[N_DETECTORS];
		Double_t maxCoincTimeWin[N_DETECTORS];

		Bool_t fDoToFCorr{kFALSE};
		Bool_t fDoTimeWalkCorr{kFALSE};
		Bool_t fDoRateCorr{kFALSE};

		TString declaration = "E[i](ch) = a[i] + b[i]*ch + c[i]*ch^2 + d[i]*ch^3 + e[i]*ch^4 + f[i]*ch^5"; // Dient der Erklaerung der folgenden Parameter zu Energiekalibrieung der gamma-Detektoren i = 0..8
		Double_t a[N_DETECTORS]; // Energiekalibrierung: Offset-Parameter je Detektor
		Double_t b[N_DETECTORS];
		Double_t c[N_DETECTORS];
		Double_t d[N_DETECTORS];
		Double_t e[N_DETECTORS];
		Double_t f[N_DETECTORS];

		double CalibrateEnergy(/*int*/ double channel, int n) const;

                // =======================================
                // ToF Korrektur
                //
                // tof(x,phi) = c 
                //      + x1 * x + phi1 * phi
                //      + phi2 * phi² + x1phi1 * x*phi + x2 * x²
                //      + phi3 * phi³ x1phi2 *x*phi² + x2phi1 * x²*phi + x3 * x³
                //      + phi4 * phi⁴ +  + x2phi2 * x²*phi² 
                // =======================================
                Double_t tof_c{0};
                Double_t tof_x1{0};
                Double_t tof_phi1{0};
                Double_t tof_phi2{0};
                Double_t tof_x1phi1{0};
                Double_t tof_x2{0};
                Double_t tof_phi3{0};
                Double_t tof_x1phi2{0};
                Double_t tof_x2phi1{0};
                Double_t tof_x3{0};
                Double_t tof_phi4{0};
                Double_t tof_x2phi2{0};

                // ToF-Korrektur der LaBr-Zeit
                void CorrectToF(double x, double phi, double& tof);


                // =======================================
                // Raten-Korrektur Energie
                //
                // E_ratecorr(E,R) = (1 + 1 / (pow(R,p0) + p1*pow(R,p2)) )) * p3 * E
                // =======================================
                Double_t rate_p0[N_DETECTORS];
                Double_t rate_p1[N_DETECTORS];
                Double_t rate_p2[N_DETECTORS];
                Double_t rate_p3[N_DETECTORS];

                // Raten-Korrektur der LaBr-Energie
                void CorrectRateDependence(int detector, double E_raw, double rate, double& E_corr);

                // =======================================
                // Time-Walk Korrektur
                //
                // time_walk(E) = c + E1 * E + E2 * E²
                // =======================================
                Double_t timewalk_c[N_DETECTORS];
                Double_t timewalk_E1[N_DETECTORS];
                Double_t timewalk_E2[N_DETECTORS];

                // Time-Walk Korrektur der LaBr Zeiten
                void CorrectTimeWalk(int detector, double E, double& t_corr);


		// wende tof und timewalk auf koinzidenzzeit an (falls konfiguriert)
                void CorrectLaBrTimeTotal(double tof, double timewalk,  double& t);





		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TDagobertCalibrationParameter,1);
};

#endif
