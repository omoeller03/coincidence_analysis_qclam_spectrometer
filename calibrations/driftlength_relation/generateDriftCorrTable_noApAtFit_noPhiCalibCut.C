#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <iomanip>

#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TString.h"

using namespace std;

//==========================================================================
// Hilfsstruktur zum Speichern der Fit-Parameter
//==========================================================================
struct FitResult
{
	double Ap;
	double t0;
	double sigma;
	double At;
	double s;
	double sigma_e;
	double sigma_t;
	double L;
};


//==========================================================================
// Hilfsmethoden & Fit-Funktion
//==========================================================================
double NormalCDF(double x,
		double mu,
		double sigma)
{
	return 0.5*(1.0+TMath::Erf((x-mu)/(std::sqrt(2.)*sigma)));
}

double SkewedGaussian(double *x,double *par)
{
	double t=x[0];

	double Ap=par[0];
	double t0=par[1];
	double sigma=par[2];
	double s=par[3];

	return Ap*exp(-0.5 * pow((t-t0)/sigma,2.0)) * (1.+TMath::Erf( s*(t-t0)/(sqrt(2.)*sigma) ));
}



double DriftModel(double *x,double *par)
{
	double t=x[0];

	double Ap = par[0];
	double t0 = par[1];
	double sigma = par[2];
	double s = par[3];

	double At = par[4];
	double sigma_e = par[5];
	double sigma_t = par[6];
	double L = par[7];

	double peak = Ap * exp(-0.5*pow((t-t0)/sigma,2.0)) * (1.+TMath::Erf( s*(t-t0)/(sqrt(2.)*sigma) ));

	double plateau = At * NormalCDF(t,t0,sigma_e) * (1.-NormalCDF(t,t0+L,sigma_t));

	return peak+plateau;
}





double EvalDriftModel(
		double t,
		double phi,
		const FitResult &fit,
		TF1 *ratioFit)
{
	double ratio=ratioFit->Eval(phi);

	double Ap=ratio*fit.At;

	double peak = Ap * exp(-0.5*pow((t-fit.t0)/fit.sigma,2.)) * (1.+TMath::Erf(fit.s*(t-fit.t0) / (sqrt(2.)*fit.sigma)));

	double plateau = fit.At * NormalCDF(t,fit.t0,fit.sigma_e) * (1.-NormalCDF(t,fit.t0+fit.L,fit.sigma_t));

	return peak+plateau;
}






//==========================================================================
// HAUPTMETHODE
//==========================================================================
void generateDriftCorrTable_noApAtFit(
		const char* inputFile,
		const char* outputDir = ".")
{

    
	gStyle->SetOptStat(0);

	//==========================================================================
	// Konfiguration
	//==========================================================================

	const double driftCellHeight = 12.0;

	double tMin = 0.0;
	double tMax = 250.0;
	double dt   = 0.5;

	double phiMin = 20.0;
	double phiMax = 65.0;
	double phiBinWidth = 1.0;

	bool useAbsolutePhi = true;

	bool savePDF = true;

	//==========================================================================
	// Drahtbereiche
	//==========================================================================
    
	vector<pair<int,int>> wireRanges =
	{
		{0,350}
	};

	//==========================================================================
	// Nur diese Drähte benutzen.
	// leer = alle Drähte aus wireRanges
	//==========================================================================

	set<int> wireList =
	{
	};

	//==========================================================================
	// Ausgeschlossene Drähte
	//==========================================================================

	set<int> excludedWires =
	{
	};

	//==========================================================================
	// Qualitätscuts
	//==========================================================================

	bool usePullCut = false;
	double maxPull = 3.0;

	bool useResidualCut = false;
	double maxResidual = 0.5;

	bool useTOTCut = true;
	double minTOT = 10.0;
	double maxTOT = 500.0;

	//==========================================================================
	// Layer
	//==========================================================================

	const int nLayers = 3;

	const char* layerName[nLayers] =
	{
		"X1",
		"U",
		"X2"
	};

	//==========================================================================
	// Ausgabeordner erzeugen
	//==========================================================================

	gSystem->mkdir(outputDir,true);

	//==========================================================================
	// Winkel vorbereiten
	//==========================================================================

	vector<double> phiBins;

	for(double phi=phiMin; phi<=phiMax+1e-9; phi+=phiBinWidth)
	{
		phiBins.push_back(phi);
	}

	//const int nPhiBins = phiBins.size();
	const int nPhiBins = static_cast<int>(std::round((phiMax - phiMin) / phiBinWidth)) + 1;;

	cout << endl;
	cout << "Phi bins: " << nPhiBins << endl;

	//==========================================================================
	// ROOT öffnen
	//==========================================================================

    
	TFile *fin = TFile::Open(inputFile);

	if(fin==nullptr || fin->IsZombie())
	{
		cout<<"Cannot open "<<inputFile<<endl;
		return;
	}

    
	TTree *tree = dynamic_cast<TTree*>(fin->Get("Track"));

	if(tree==nullptr)
	{
		cout<<"Tree Track not found."<<endl;
		return;
	}

    
	cout<<"Entries : "<<tree->GetEntries()<<endl;

	//==========================================================================
	// TreeReader
	//==========================================================================

	TTreeReader reader(tree);

	TTreeReaderValue<double> phi(reader,"phi");

	TTreeReaderValue<vector<int>>    res_layerID(reader,"res_layerID");
	TTreeReaderValue<vector<int>>    res_wireNumber(reader,"res_wireNumber");

	TTreeReaderValue<vector<double>> drifttimes(reader,"drifttimes");
	TTreeReaderValue<vector<double>> tot(reader,"tot");
	TTreeReaderValue<vector<double>> residual(reader,"residual");
	TTreeReaderValue<vector<double>> pull(reader,"pull");

	//==========================================================================
	// Histogramme
	//==========================================================================

	vector<vector<TH1D*> > hist;

	hist.resize(nLayers);

	for(int layer=0; layer<nLayers; layer++)
	{
		hist[layer].resize(nPhiBins);

		for(int p=0; p<nPhiBins; p++)
		{
			TString name;

			name.Form("h_%s_%03d",
					layerName[layer],
					p);

			hist[layer][p]=new TH1D(
					name,
					name,
					int((tMax-tMin)/dt),
					tMin,
					tMax);
		}
	}





	//==========================================================================
	// Hilfsfunktion: Draht akzeptiert?
	//==========================================================================

	auto acceptWire = [&](int wire)->bool
	{
		if(excludedWires.count(wire)) { return false; }

		if(!wireList.empty()) { return wireList.count(wire); }

		for(const auto& r : wireRanges)
		{
			if(wire>=r.first && wire<=r.second) { return true; }
		}

		return false;
	};

	//==========================================================================
	// Tree einlesen
	//==========================================================================

	Long64_t acceptedHits = 0;
	Long64_t acceptedTracks = 0;

	while(reader.Next())
	{
		double phiDeg = *phi;

		if(useAbsolutePhi) { phiDeg = fabs(phiDeg); }

		if(phiDeg < phiMin) { continue; }

		if(phiDeg > phiMax) { continue; }

		int phiIndex = static_cast<int>(std::round((phiDeg-phiMin)/phiBinWidth));

		if(phiIndex<0) { continue; }

		if(phiIndex>=nPhiBins) { continue; }

		acceptedTracks++;

		size_t nHits = drifttimes->size();

		for(size_t i=0;i<nHits;i++)
		{
			int layer = (*res_layerID)[i];

			if(layer<0 || layer>=nLayers) { continue; }

			int wire = (*res_wireNumber)[i];

			if(!acceptWire(wire)) { continue; }

			double dtime = (*drifttimes)[i];

			if(dtime<tMin) { continue; }

			if(dtime>tMax) { continue; }

			if(usePullCut)
			{
				if(fabs((*pull)[i])>maxPull) { continue; }
			}

			if(useResidualCut)
			{
				if(fabs((*residual)[i])>maxResidual) { continue; }
			}

			if(useTOTCut)
			{
				double t = (*tot)[i];

				if(t<minTOT) { continue; }

				if(t>maxTOT) { continue; }
			}

			hist[layer][phiIndex]->Fill(dtime);

			acceptedHits++;
		}
	}

	cout<<endl;
	cout<<"Accepted tracks : "<<acceptedTracks<<endl;
	cout<<"Accepted hits   : "<<acceptedHits<<endl;

	//==========================================================================
	// Histogramme normieren
	//==========================================================================

	for(int layer=0; layer<nLayers; layer++)
	{
		for(int p=0; p<nPhiBins; p++)
		{
			TH1D* h = hist[layer][p];

			double integral = h->Integral("width");

			if(integral>0) { h->Scale(1.0/integral); }
		}
	}

	//==========================================================================
	// PDF-Ausgabe
	//==========================================================================

	// deklariere Vektor, dem die Fit-Ergebnisse übergeben werden
	std::vector<std::vector<FitResult>> fitResults(nLayers, std::vector<FitResult>(nPhiBins));
	if(savePDF)
	{
		for(int layer=0; layer<nLayers; layer++)
		{
			TString pdfName;

			pdfName.Form("%s/histograms_%s.pdf",
					outputDir,
					layerName[layer]);

			TCanvas c("c","c",900,600);

			c.Print(pdfName+"[");

			for(int p=0;p<nPhiBins;p++)
			{
				std::cout << std::endl;
				std::cout << Form("phi = %.f", phiBins[p]) << std::endl;;
				hist[layer][p]->SetLineWidth(2);


				// plots machen
				TString title;
				title.Form("%s   phi = %.1f deg",
						layerName[layer],
						phiBins[p]);
				hist[layer][p]->SetTitle(title);
				hist[layer][p]->GetXaxis()->SetTitle("Drift time (ns)");
				hist[layer][p]->GetYaxis()->SetTitle("Normalized counts");
				hist[layer][p]->Draw("hist");

				// FIT
				TString fname = ("fit_%s_%.1f",layerName[layer],phiBins[p]);

				TF1* f = new TF1(
						fname,
						DriftModel,
						tMin,
						tMax,
						8);

				f->SetParNames(
						"Ap",
						"t0",
						"sigma",
						"s",
						"At",
						"sigma_e",
						"sigma_t",
						"L");

				f->SetParameters(
						0.006, // Ap
						45., // t0
						10., // sigma
						2., // s
						0.002, // At
						15., // sigma_e
						20., // sigma_t
						190. // L
						); 

				f->SetParLimits(0,0,10);
				f->SetParLimits(1,20,80);
				f->SetParLimits(2,1,20);
				f->SetParLimits(3,-5,5);
				f->SetParLimits(4,0,10);
				f->SetParLimits(5,0.5,30);
				f->SetParLimits(6,0.5,30);
				f->SetParLimits(7,20,300);

				hist[layer][p]->Fit(f);//,"RQ0");

				f->Draw("same");

				// fit-daten speichern
				fitResults[layer][p].Ap = f->GetParameter(0);
				fitResults[layer][p].t0 = f->GetParameter(1);
				fitResults[layer][p].sigma = f->GetParameter(2);
				fitResults[layer][p].s = f->GetParameter(3);
				fitResults[layer][p].At = f->GetParameter(4);
				fitResults[layer][p].sigma_e = f->GetParameter(5);
				fitResults[layer][p].sigma_t = f->GetParameter(6);
				fitResults[layer][p].L = f->GetParameter(7);


				c.Print(pdfName);
			}

			c.Print(pdfName+"]");
		}
	}




	std::vector<TF1*> ratioFits(nLayers);

	for(int layer=0; layer<nLayers; layer++)
	{
		TGraph *g=new TGraph;

		int n=0;

		for(int p=0;p<nPhiBins;p++)
		{
			double ratio= fitResults[layer][p].Ap / fitResults[layer][p].At;

			g->SetPoint(
					n++,
					phiBins[p],
					ratio);
		}

		TString name;
		name.Form("ratio_%s",layerName[layer]);

		ratioFits[layer]=new TF1(
				name,
				"[0]+[1]*x+[2]*x*x",
				phiMin,
				phiMax);

		g->Fit(ratioFits[layer],"Q");
	}




	//=====================================================================
	// Extrapolation
	//=====================================================================

	std::vector<std::vector<FitResult>> tableFits(nLayers,std::vector<FitResult>(nPhiBins));

	for(int layer=0; layer<nLayers; layer++)
	{
		for(int p=0; p<nPhiBins; p++)
		{
			tableFits[layer][p]=fitResults[layer][p];
		}

		//--------------------------------------------------
		// links extrapolieren
		//--------------------------------------------------

		int firstGood=0;

		while(firstGood<nPhiBins-1 &&
				fitResults[layer][firstGood].At<=0)
		{
			firstGood++;
		}

		for(int p=0;p<firstGood;p++)
			tableFits[layer][p]=fitResults[layer][firstGood];

		//--------------------------------------------------
		// rechts extrapolieren
		//--------------------------------------------------

		int lastGood=nPhiBins-1;

		while(lastGood>0 &&
				fitResults[layer][lastGood].At<=0)
		{
			lastGood--;
		}

		for(int p=lastGood+1;p<nPhiBins;p++)
			tableFits[layer][p]=fitResults[layer][lastGood];
	}



	//==========================================================================
	// Driftlängentabellen schreiben
	//==========================================================================

	for(int layer=0; layer<nLayers; layer++)
	{
		TString outname;
		outname.Form("%s/driftlength_relation_%s.txt",
				outputDir,
				layerName[layer]);

		std::ofstream out(outname.Data());

		out << std::fixed;

		// Header
		out << "t(ns)";

		for(double phi=phiMin; phi<=phiMax+1e-9; phi+=phiBinWidth)
		{
			out << "\t" << phi;
		}

		out << std::endl;

		/*
		// Für jeden Zeitpunkt
		for(double t=tMin; t<=tMax+1e-9; t+=dt)
		{
			out << std::setprecision(1) << t;

			// Für jeden Winkel
			for(int p=0; p<nPhiBins; p++)
			{
				//----------------------------------------------------------
				// Gesamtintegral
				//----------------------------------------------------------

				double total = 0.0;

				for(double tt=tMin; tt<=tMax; tt+=0.1)
				{
//					total += EvalDriftModel(
//							tt,
//							phiBins[p],
//							fitResults[layer][p],
//							ratioFits[layer]) * 0.1;
					total += EvalDriftModel(
							tt,
							phiBins[p],
							tableFits[layer][p],
							ratioFits[layer]) * 0.1;
				}

				//----------------------------------------------------------
				// Integral bis t
				//----------------------------------------------------------

				double part = 0.0;

				for(double tt=tMin; tt<=t; tt+=0.1)
				{
					part += EvalDriftModel(
							tt,
							phiBins[p],
							fitResults[layer][p],
							ratioFits[layer]) * 0.1;
				}

				//----------------------------------------------------------
				// Driftlänge
				//----------------------------------------------------------

				double driftLength = 0.0;

				if(total>0.0) { driftLength = driftCellHeight * part / total; }

				out << "\t"
					<< std::setprecision(10)
					<< driftLength;
			}

			out << std::endl;
		}
	
		*/


		//======================================================================
		// Gesamtintegrale einmal berechnen
		//======================================================================

		/*
		std::vector<double> totalIntegral(nPhiBins);

		for(int p=0; p<nPhiBins; p++)
		{
			totalIntegral[p]=0.0;

			for(double tt=tMin; tt<=tMax; tt+=0.1)
			{
				totalIntegral[p] +=
					EvalDriftModel(
							tt,
							phiBins[p],
							tableFits[layer][p],
							ratioFits[layer]) * 0.1;
			}
		}
		*/
		std::vector<double> totalIntegral(nPhiBins, 0.0);

for(int p=0; p<nPhiBins; p++)
{
    double previousT =
        tMin;

    double previousValue =
        EvalDriftModel(
            previousT,
            phiBins[p],
            tableFits[layer][p],
            ratioFits[layer]);

    for(double t=tMin+dt; t<=tMax+1e-9; t+=dt)
    {
        double currentValue =
            EvalDriftModel(
                t,
                phiBins[p],
                tableFits[layer][p],
                ratioFits[layer]);

        totalIntegral[p] +=
            0.5 * (previousValue + currentValue) * dt;

        previousT = t;
        previousValue = currentValue;
    }
}

		//======================================================================
		// Laufende Integrale
		//======================================================================

std::vector<double> runningIntegral(nPhiBins, 0.0);

for(double t=tMin; t<=tMax+1e-9; t+=dt)
{
    out << std::setprecision(1) << t;

    for(int p=0; p<nPhiBins; p++)
    {
        double driftLength = 0.0;

        if(t > tMin)
        {
            double previousValue =
                EvalDriftModel(
                    t-dt,
                    phiBins[p],
                    tableFits[layer][p],
                    ratioFits[layer]);

            double currentValue =
                EvalDriftModel(
                    t,
                    phiBins[p],
                    tableFits[layer][p],
                    ratioFits[layer]);

            runningIntegral[p] +=
                0.5 * (previousValue + currentValue) * dt;
        }

        if(totalIntegral[p] > 0.0)
        {
            driftLength =
                driftCellHeight *
                runningIntegral[p] /
                totalIntegral[p];
        }

        out << "\t"
            << std::setprecision(10)
            << driftLength;
    }

    out << std::endl;
}
/*
		std::vector<double> runningIntegral(nPhiBins,0.0);

		//======================================================================
		// Tabelle schreiben
		//======================================================================

		for(double t=tMin; t<=tMax+1e-9; t+=dt)
		{
			out << std::setprecision(1) << t;

			for(int p=0; p<nPhiBins; p++)
			{
				//----------------------------------------------------------
				// Integral bis aktuelle Zeit erweitern
				//----------------------------------------------------------

				runningIntegral[p] +=
					EvalDriftModel(
							t,
							phiBins[p],
							tableFits[layer][p],
							ratioFits[layer]) * dt;

				//----------------------------------------------------------
				// Driftlänge
				//----------------------------------------------------------

				double driftLength = 0.0;

				if(totalIntegral[p]>0.0)
				{
					driftLength =
						driftCellHeight *
						runningIntegral[p] /
						totalIntegral[p];
				}

				out << "\t"
					<< std::setprecision(10)
					<< driftLength;
			}

			out << std::endl;
		}

	*/
		out.close();
	}






}





