#ifndef TQCLAMSPECTRUMCALIBRATIONPARAMETER_H
#define TQCLAMSPECTRUMCALIBRATIONPARAMETER_H

#include "TGo4Parameter.h"

class TQClamSpectrumCalibrationParameter : public TGo4Parameter
{
	public:
		TQClamSpectrumCalibrationParameter();
		TQClamSpectrumCalibrationParameter(const char* name);

		void CorrectPhi(double x_raw, // x unkorrigiert
				double sigma_x_raw, // std dev x unkorrigiert
				double phi_raw, // phi unkorrigiert
				double sigma_phi_raw, // std dev phi unkorrigiert
				double& phi_corr, // phi korrigerit -> wird gesetzt
				double& sigma_phi_corr // std dev phi korrigiert -> wird gesetzt
				) const;
		void CorrectX(double x_raw, // x unkorrigiert
				double sigma_x_raw, // std dev x unkorrigiert
				double phi_corr, // phi korrigiert 
				double sigma_phi_corr, // std dev phi korrigiert
				double& x_corr, // x korrigiert -> wird gesetzt
				double& sigma_x_corr // std dev x korrigiert -> wird gesetzt
				) const;
		void Calibrate(double x_corr, // x korrigiert
				double sigma_x_corr, // sigma x korrigiert
				double& e_MeV, // energie/MeV -> wird gesetzt
				double& sigma_e_MeV // sigma energie/MeV -> wird gesetzt
			      ) const;

		// =============================
		// u-Ursprung -> verschiebe u-Koordinaten, damit y-Spektrum um 0 zentriert
		// =============================
		Double_t u_org = -83.0;


		// =============================
		// phi-Korrektur Parameter
		// =============================
		Double_t a_phi_corr = 0.; // Phi-Korrektur: Konstante
		Double_t b_phi_corr = 0.; // Phi-Korrektur: Steigung (1. Ordn)
		Double_t c_phi_corr = 0.; // Phi-Korrektur: Kruemmung (2. Ordn)

		// =============================
		// x-Korrektur Parameter
		// =============================
		// 0. Ordnung
		Double_t a_0_0 = 0.; // X-Korrektur: a_0_0 * x**0 * phi_corr**0 (0. Ordnung)
		// 1. Ordnung
		Double_t a_1_0 = 0.; // X-Korrektur: a_1_0 * x**1 * phi_corr**0 (1. Ordnung)
		Double_t a_0_1 = 0.; // X-Korrektur: a_0_1 * x**0 * phi_corr**1 (1. Ordnung)
		Double_t a_1_1 = 0.; // X-Korrektur: a_1_1 * x**1 * phi_corr**1 (1. Ordnung)
		// 2. Ordnung
		Double_t a_2_0 = 0.; // X-Korrektur: a_2_0 * x**2 * phi_corr**0 (2. Ordnung)
		Double_t a_2_1 = 0.; // X-Korrektur: a_1_1 * x**2 * phi_corr**1 (2. Ordnung)
		Double_t a_0_2 = 0.; // X-Korrektur: a_0_2 * x**0 * phi_corr**2 (2. Ordnung)
		Double_t a_1_2 = 0.; // X-Korrektur: a_1_2 * x**1 * phi_corr**2 (2. Ordnung)
		Double_t a_2_2 = 0.; // X-Korrektur: a_2_2 * x**2 * phi_corr**2 (2. Ordnung)

		// =============================
		// Energie-Kalibrierung
		// =============================
		Double_t eS_corr = 0.; // Energie-Kalibrierung: Strahlenergie / MeV
		Double_t e0_corr = 0.; // Energie-Kalibrierung: Sollstrahl-Energie / MeV
		Double_t a_e_corr = 0.; // Energie-Kalibrierung: Konstante (0. Ordnung)
		Double_t b_e_corr = 0.; // Energie-Kalibrierung: Steigung (1. Ordnung)
		Double_t c_e_corr = 0.; // Energie-Kalibrierung: Kruemmung (2. Ordnung)

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


	ClassDefOverride(TQClamSpectrumCalibrationParameter,1);
};


#endif
