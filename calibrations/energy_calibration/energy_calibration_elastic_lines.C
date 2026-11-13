#include "TGraph.h"
#include "TF1.h"
#include "TCanvas.h"

#include <iostream>
#include <fstream>
#include <vector>

// ================================
//
// Energiekalbrierung 
//
// nutze elastische Linien, die durch die Fokalebene geschoben werden
// benoetige:
// - Strahlenergie
// - Sollstrahlenergie je Messwert (in MSc Arbeit von Gerhart ermittelte Formel -> siehe Grafana etwa)
// - rekonstruierte Position in Fokalebene
// - Anregungsenergie (bzw. deponierte Energie Rueckstoss...)
//
//
//
// Vorgehen: 
//
// 1) Annahme: Impulsablage durch Polynom beschreiben
// delta p / p0 = sum_{j=0}^{n} a_j * x^j
// mit:
// - delta p = p - p0,
// - p = Elektron-Impuls gemessen,
// - p0 = Soll-Impuls Elektron
//
// 2) Vernachlaessige Elektronen-Masse (realtivistische Elektronen)
// delta p / p = E_S / E0(i) - ( 1 + E_x(i) / E0(i) )
// mit:
// - E_S = Strahlenergie
// - E0(i) = Sollstrahlenergie der 'i'-ten Linie (Abh.: Magnetfeldeinstellung)
// - B(i) = Dipol-Magnetfeld bei 'i'-ter Linie in mT
// - E_x(i) = Anregungsenergie der 'i'-ten Linie (Abh.: Zustand, Target)
//
// 3) Minimierung/Fit:
// y =  E_S / E(i) - ( 1 + E_x(i) / E(i) )
//
// sum_{j=0}^{n} a_j * x^j - y = 0
//
//
//
// Benoetige Input-File in folgender Form:
//
// x	B_mT	E_x_MeV
// ...
//
//
// Sollstrahl-Energie hier im Skript setzen
//
// ================================

//std::string data = "energy_calibration_data.txt";
//double beam_energy = 65.0;

// === masterarbeit gerhart ===
double par_1 = 0.20; // MeV/mT
double par_2 = 1.73; // mT

double soll_strahl_energie_MeV(double B_mT)
{
	return par_1 * (B_mT - par_2);
}

void energy_calibration_elastic_lines(const char* data = "energy_calibration_data.txt",double beam_energy = 65.0)
{
	const double E_S = beam_energy; // Strahlenergie in MeV
	

	// Datei einlesen
	std::ifstream file(data);

	std::vector<double> x_vals;
	std::vector<double> y_vals;

	double x, B, E_x;

	while( file >> x >> B >> E_x)
	{
		double E0 = soll_strahl_energie_MeV(B);
		std::cout << E0 << endl;
		double y = E_S / E0 - (1 + E_x/E0);
		x_vals.push_back(x);
		y_vals.push_back(y);
	}

	int n = x_vals.size();


	// ==== TGraph ====
	TGraph* gr = new TGraph(n);

	for(int i=0; i<n; i++)
	{
		gr->SetPoint(i, x_vals[i], y_vals[i]);
	}

	gr->SetTitle("Energiekalibrierung; x; #Delta p / p_{0}");


	// Fit-Funktion
	TF1* fit = new TF1("fit","[0] + [1] * x + [2] * x * x",0,1000);

	gr->Fit(fit, "R");


	// Parameter ausgeben
	double a0 = fit->GetParameter(0);
	double a1 = fit->GetParameter(1);
	double a2 = fit->GetParameter(2);


	std::cout << "Fit-Parameter: \n"
		<< "a0 = " << a0 << "\n"
		<< "a1 = " << a1 << "\n"
		<< "a2 = " << a2 << "\n\n"
		<< "Funktion: E_Rest = E0 * ( 1 + a0 + a1 * x + a2 * x * x)"
		<< std::endl;


	// Plot
	TCanvas* c = new TCanvas("c","Kalibrierung",800,600);
	gr->SetMarkerStyle(2);
	gr->SetMarkerSize(3.5);
	gr->Draw("AP");
	fit->Draw("same");
}

