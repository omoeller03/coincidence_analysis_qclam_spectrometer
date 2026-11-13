#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <memory>
#include <string>

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
#include "TF1.h"
#include "TGraph.h"
#include "TMath.h"

#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/Minimizer.h"

using namespace std;

std::string gInputDirectory = "/d/d04-1/ag_pi/QCLAM_EEPG_CE140_2026/ana/unpacked_data/calibrations_go4/data/drifttime_length";
std::string gFilePrefix = "DriftlengthCalibStore_qclam_dago_";
std::string gTreeName = "Track";

std::string gOutputDir = ".";

int gRunFirst = 34;
int gRunLast = 40;




// fit ergebnis je kalibrierten winkel
struct FitResult
{
	double Ap = 0.0;
	double t0 = 0.0;
	double sigma = 0.0;
	double s = 0.0;

	double At = 0.0;
	double sigma_e = 0.0;
	double sigma_t = 0.0;
	double L = 0.0;
};


// fit-fkt
double NormalCDF(double x, double mu, double sigma)
{
	if(sigma <= 0.0) { return 0.0; }

	return 0.5 * (1.0 + TMath::Erf((x - mu) / (std::sqrt(2.0) * sigma)));
}


double SkewedGaussianValue(
		double t,
		double Ap,
		double t0,
		double sigma,
		double s)
{
	if(sigma <= 0.0) { return 0.0; }

	return Ap * std::exp(-0.5 * std::pow((t - t0) / sigma, 2.0)) * (1.0 + TMath::Erf( s * (t - t0) / (std::sqrt(2.0) * sigma)));
}


double DriftModelValue(
		double t,
		double Ap,
		double t0,
		double sigma,
		double s,
		double At,
		double sigma_e,
		double sigma_t,
		double L)
{
	const double peak = SkewedGaussianValue(t, Ap, t0, sigma, s);

	const double plateau = At * NormalCDF(t, t0, sigma_e) * (1.0 - NormalCDF(t, t0 + L, sigma_t));

	return peak + plateau;
}


double SkewedGaussian(double *x, double *par)
{
	return SkewedGaussianValue(
			x[0],
			par[0],
			par[1],
			par[2],
			par[3]);
}


double DriftModel(double *x, double *par)
{
	return DriftModelValue(
			x[0],
			par[0],
			par[1],
			par[2],
			par[3],
			par[4],
			par[5],
			par[6],
			par[7]);
}


// quadr fit Ap/At
double QuadFunc(
		double phi,
		double a,
		double b,
		double c)
{
	return a + b * phi + c * phi * phi;
}


// plateau-fit
bool RunPlateauFit(
		const vector<vector<double>>& x,
		const vector<vector<double>>& y,
		const vector<double>& t0Init,
		double fit2Min,
		double fit2Max,
		vector<double>& At,
		double& sigma_e,
		double& sigma_t,
		double& L)
{
	const int nPhi = static_cast<int>(x.size());

	const int nPar = nPhi + 3;

	auto objective = [&](const double *p)
	{
		double chi2 = 0.0;

		const double se = p[nPhi];
		const double st = p[nPhi + 1];
		const double l  = p[nPhi + 2];

		for(int i = 0; i < nPhi; ++i)
		{
			const double ati = p[i];
			const double t0  = t0Init[i];

			for (size_t j = 0; j < x[i].size(); ++j)
			{
				const double t = x[i][j];

				if(t < fit2Min || t > fit2Max) { continue; }

				const double model = ati *
					NormalCDF(t, t0, se) *
					(1.0 - NormalCDF(t, t0 + l, st));

				const double r = y[i][j] - model;

				chi2 += r * r;
			}
		}

		return chi2;
	};


	unique_ptr<ROOT::Math::Minimizer> min(ROOT::Math::Factory::CreateMinimizer("Minuit2","Migrad"));

	if(!min) { return false; }


	ROOT::Math::Functor f(objective,nPar);

	min->SetFunction(f);

	min->SetMaxFunctionCalls(200000);
	min->SetMaxIterations(20000);
	min->SetTolerance(1e-7);
	min->SetPrintLevel(0);


	// At(phi)
	for(int i = 0; i < nPhi; ++i)
	{
		double guess = 0.1;
		double maxY = 0.0;

		for(size_t j = 0; j < x[i].size(); ++j)
		{
			if(x[i][j] >= fit2Min && x[i][j] <= fit2Max)
			{
				maxY = std::max(maxY, y[i][j]);
			}
		}

		if(maxY > 0.0) { guess = maxY; }

		min->SetVariable(i, Form("At_%d", i), guess, 0.01);

		min->SetVariableLimits(i, 0.0, 10.0);
	}


	// Globale Parameter
	min->SetVariable(nPhi, "sigma_e", 5.0, 0.05);
	min->SetVariable(nPhi + 1, "sigma_t", 5.0, 0.05);
	min->SetVariable(nPhi + 2, "L", 100.0, 0.2);

	min->SetVariableLimits(nPhi, 0.1, 50.0);
	min->SetVariableLimits(nPhi + 1, 0.1, 50.0);
	min->SetVariableLimits(nPhi + 2, 10.0, 400.0);


	const bool ok = min->Minimize();

	if(!ok) { return false; }


	const double *p = min->X();


	At.resize(nPhi);

	for(int i = 0; i < nPhi; ++i)
	{
		At[i] = p[i];
	}

	sigma_e = p[nPhi];
	sigma_t = p[nPhi + 1];
	L = p[nPhi + 2];


	return true;
}


// joint fit
bool RunJointFit(
		const vector<vector<double>>& xFit,
		const vector<vector<double>>& yFit,
		vector<FitResult>& result,
		double& sigma,
		double& s,
		double& sigma_e,
		double& sigma_t,
		double& L)
{
	const int nPhi = static_cast<int>(result.size());

	const int nPar = 3 * nPhi + 5;


	auto objective = [&](const double *p)
	{
		double chi2 = 0.0;

		const double sig = p[3 * nPhi + 0];

		const double skw = p[3 * nPhi + 1];

		const double se = p[3 * nPhi + 2];

		const double st = p[3 * nPhi + 3];

		const double l = p[3 * nPhi + 4];


		for(int i = 0; i < nPhi; ++i)
		{
			const double Ap = p[3 * i + 0];

			const double t0 = p[3  * i + 1];

			const double At = p[3 * i + 2];


			for(size_t j = 0; j < xFit[i].size(); ++j)
			{
				const double model = DriftModelValue(
						xFit[i][j],
						Ap,
						t0,
						sig,
						skw,
						At,
						se,
						st,
						l);


				const double r = yFit[i][j] - model;

				// soft_l1, f_scale = 0.1
				const double fScale = 0.1;

				const double u = r / fScale;

				chi2 +=
					2.0 *
					fScale *
					fScale *
					(std::sqrt(1.0 + u * u) - 1.0);
			}
		}

		return chi2;
	};


	unique_ptr<ROOT::Math::Minimizer> min(ROOT::Math::Factory::CreateMinimizer("Minuit2","Migrad"));

	if(!min) { return false; }


	ROOT::Math::Functor f(objective,nPar);

	min->SetFunction(f);

	min->SetMaxFunctionCalls(500000);
	min->SetMaxIterations(50000);
	min->SetTolerance(1e-7);
	min->SetPrintLevel(0);


	// Winkelabhängige Parameter
	for(int i = 0; i < nPhi; ++i)
	{
		min->SetVariable(
				3 * i + 0,
				Form("Ap_%d", i),
				result[i].Ap,
				0.001);

		min->SetVariable(
				3 * i + 1,
				Form("t0_%d", i),
				result[i].t0,
				0.01);

		min->SetVariable(
				3 * i + 2,
				Form("At_%d", i),
				result[i].At,
				0.001);


		min->SetVariableLimits(
				3 * i + 0,
				0.0,
				10.0);

		// [30, 70]
		min->SetVariableLimits(
				3 * i + 1,
				30.0,
				70.0);

        
		min->SetVariableLimits(
				3 * i + 2,
				0.0,
				10.0);
	}


	// Globale Parameter
	min->SetVariable(
			3 * nPhi + 0,
			"sigma",
			sigma,
			0.05);

	min->SetVariable(
			3 * nPhi + 1,
			"s",
			s,
			0.02);

	min->SetVariable(
			3 * nPhi + 2,
			"sigma_e",
			sigma_e,
			0.05);

	min->SetVariable(
			3 * nPhi + 3,
			"sigma_t",
			sigma_t,
			0.05);

	min->SetVariable(
			3 * nPhi + 4,
			"L",
			L,
			0.2);


	min->SetVariableLimits(
			3 * nPhi + 0,
			0.1,
			50.0);

	min->SetVariableLimits(
			3 * nPhi + 1,
			-5.0,
			5.0);

   
	min->SetVariableLimits(
			3 * nPhi + 2,
			0.1,
			50.0);

	min->SetVariableLimits(
			3 * nPhi + 3,
			0.1,
			50.0);

	min->SetVariableLimits(
			3 * nPhi + 4,
			10.0,
			400.0);


	const bool ok = min->Minimize();

	if(!ok) { return false; }


	const double *p = min->X();


	for(int i = 0; i < nPhi; ++i)
	{
		result[i].Ap = p[3 * i + 0];

		result[i].t0 = p[3 * i + 1];

		result[i].At = p[3 * i + 2];

		result[i].sigma = p[3 * nPhi + 0];

		result[i].s = p[3 * nPhi + 1];

		result[i].sigma_e = p[3 * nPhi + 2];

		result[i].sigma_t = p[3 * nPhi + 3];

		result[i].L = p[3 * nPhi + 4];
	}

	sigma = p[3 * nPhi + 0];

	s = p[3 * nPhi + 1];

	sigma_e = p[3 * nPhi + 2];

	sigma_t = p[3 * nPhi + 3];

	L = p[3 * nPhi + 4];


	return true;
}







TChain* BuildInputChain()
{
        TChain* chain = new TChain(gTreeName.c_str());

        int nFiles = 0;

        std::cout
                << std::endl
                << "=================================================="
                << std::endl
                << "Input-Dateien"
                << std::endl
                << "=================================================="
                << std::endl;

        for(int run = gRunFirst; run <= gRunLast; run++)
        {
                TString pattern =
                        Form(
                                        "%s/%s%03d_*.root",
                                        gInputDirectory.c_str(),
                                        gFilePrefix.c_str(),
                                        run
                                        );

                std::cout
                        << "Run "
                        << run
                        << ": "
                        << pattern
                        << std::endl;

                int nAdded =
                        chain->Add(pattern);

                std::cout
                        << "  -> "
                        << nAdded
                        << " Dateien"
                        << std::endl;

                nFiles += nAdded;
        }


        std::cout
                << std::endl
                << "Insgesamt "
                << nFiles
                << " ROOT-Dateien hinzugefuegt."
                << std::endl;

        if(nFiles == 0)
        {
                std::cout
                        << "FEHLER: Keine Input-Dateien gefunden!"
                        << std::endl;

                delete chain;
                return nullptr;
        }

        std::cout
                << "Tree: "
                << gTreeName
                << std::endl;

        std::cout
                << "Entries: "
                << chain->GetEntries()
                << std::endl;

        std::cout
                << "=================================================="
                << std::endl
                << std::endl;

        return chain;
}











// == main ==
void generateDriftCorrTable() //const char* inputFile,const char* outputDir = ".")
{
	gStyle->SetOptStat(0);

	const char* outputDir = gOutputDir.c_str();

	/// KONFIGURATION

	// Driftzelle
	const double driftCellHeight = 12.0;


	// Zeitachse
	const double tMin = 0.0;
	const double tMax = 250.0;
	const double dt   = 0.5;


	// NUR DIESE WINKEL WERDEN TATSÄCHLICH KALIBRIERT.
	// Die restlichen aus Parameter-Fit und Extrapolation
	const double calibPhiMin = 33.0;
	const double calibPhiMax = 53.0;

	const double phiBinWidth = 1.0;


	// Winkelbereich der AUSGABETABELLE
	const double tablePhiMin = 25.0;
	const double tablePhiMax = 65.0;
	const double tablePhiStep = 1.0;


	// Absolute Phi-Werte verwenden?
	const bool useAbsolutePhi = true;


	// PDFs
	const bool savePDF = true;


	// Fitbereiche
	const double fit1Min = 30.0;
	const double fit1Max = 80.0;

	const double fit2Min = 100.0;
	const double fit2Max = 225.0;


	// DRAHTBEREICHE
	vector<pair<int,int>> wireRanges =
	{
		{0, 350}
	};


	set<int> wireList =
	{
		// z.B.
		// 68, 69, 70, ...
	};


	set<int> excludedWires =
	{
		// z.B.
		// 100, 101
	};


	// cuts
	bool usePullCut = false;
	double maxPull = 3.0;


	bool useResidualCut = false;
	double maxResidual = 0.5;


	bool useTOTCut = true;
	double minTOT = 10.0;
	double maxTOT = 400.0;


	// layer
	const int nLayers = 3;

	const char* layerName[nLayers] =
	{
		"X1",
		"U",
		"X2"
	};


	gSystem->mkdir(outputDir,true);


    
	// KALIBRIER-WINKEL ERZEUGEN
    
	vector<double> phiBins;

    
	for(double phi = calibPhiMin; phi <= calibPhiMax + 1e-9; phi += phiBinWidth)
	{
		phiBins.push_back(phi);
	}

	const int nPhiBins = static_cast<int>(phiBins.size());


	cout << endl;
	cout << "==================================================" << endl;
	cout << "Drift calibration configuration" << endl;
	cout << "==================================================" << endl;

	cout << "Calibration phi range : "
		<< calibPhiMin
		<< " ... "
		<< calibPhiMax
		<< " deg"
		<< endl;

	cout << "Table phi range       : "
		<< tablePhiMin
		<< " ... "
		<< tablePhiMax
		<< " deg"
		<< endl;

	cout << "Number of fit angles  : "
		<< nPhiBins
		<< endl;

	cout << "Fit region 1          : "
		<< fit1Min
		<< " ... "
		<< fit1Max
		<< " ns"
		<< endl;

	cout << "Fit region 2          : "
		<< fit2Min
		<< " ... "
		<< fit2Max
		<< " ns"
		<< endl;

	cout << "==================================================" << endl;


	// TABELLE-WINKEL
	vector<double> tablePhiBins;

	for(double phi = tablePhiMin; phi <= tablePhiMax + 1e-9; phi += tablePhiStep)
	{
		tablePhiBins.push_back(phi);
	}


        // Input Chain aufbauen
        TChain* tree = BuildInputChain();

        if(!tree)
        {
                std::cout << "Tree nicht gefunden!" << std::endl;
                return;
        }



	if(tree == nullptr)
	{
		cout << "Tree Track not found." << endl;

		//fin->Close();

		return;
	}



    
	cout << "Entries    : "
         
		<< tree->GetEntries()
         
		<< endl;


	// Reader
	TTreeReader reader(tree);


	TTreeReaderValue<double> phi(reader,"phi");

	TTreeReaderValue<vector<int>> res_layerID(reader,"res_layerID");

	TTreeReaderValue<vector<int>> res_wireNumber(reader,"res_wireNumber");

	TTreeReaderValue<vector<double>> drifttimes(reader,"drifttimes");

	TTreeReaderValue<vector<double>> tot(reader,"tot");

	TTreeReaderValue<vector<double>> residual(reader,"residual");

	TTreeReaderValue<vector<double>> pull(reader,"pull");


	// HISTOGRAMME
	// verwende 501 pkte, 0.0, 0.5 ... 250.0
	const int nTimeBins = static_cast<int>(std::round((tMax - tMin) / dt)) + 1;


	vector<vector<TH1D*>> hist(nLayers, vector<TH1D*>(nPhiBins,nullptr));


	for(int layer = 0; layer < nLayers; ++layer)
	{
		for (int p = 0; p < nPhiBins; ++p)
		{
			TString name;

			name.Form("h_%s_%03d",layerName[layer],p);


			const double hMin = tMin - dt / 2.0;
			const double hMax = tMax + dt / 2.0;


			hist[layer][p] = new TH1D(
					name,
					name,
					nTimeBins,
					hMin,
					hMax);
		}
	}


	// Draht akzeptiert
	auto acceptWire = [&](int wire) -> bool
		{
			if(excludedWires.count(wire)) { return false; }
			if(!wireList.empty()) { return wireList.count(wire); }

			for(const auto& r : wireRanges)
			{
				if(wire >= r.first && wire <= r.second) { return true; }
			}


			return false;
		};


	// tree einlesen
	Long64_t acceptedHits = 0;
	Long64_t acceptedTracks = 0;


	while(reader.Next())
	{
		double phiDeg = *phi;

		if(useAbsolutePhi) { phiDeg = std::fabs(phiDeg); }


		// nur im kalibrierbereich
		if(phiDeg < calibPhiMin || phiDeg > calibPhiMax)
		{
			continue;
		}


		const int phiIndex = static_cast<int>(std::round((phiDeg - calibPhiMin) / phiBinWidth));


		if(phiIndex < 0 || phiIndex >= nPhiBins)
		{
			continue;
		}


		acceptedTracks++;

		const size_t nHits = drifttimes->size();


		for(size_t i = 0; i < nHits; ++i)
		{
			const int layer = (*res_layerID)[i];


			if(layer < 0 || layer >= nLayers)
			{
				continue;
			}


			const int wire = (*res_wireNumber)[i];

			if(!acceptWire(wire)) { continue; }


			const double dtime = (*drifttimes)[i];

			if(dtime < tMin || dtime > tMax) { continue; }


            
			if(usePullCut && std::fabs((*pull)[i]) > maxPull) { continue; }

			if(useResidualCut && std::fabs((*residual)[i]) > maxResidual) { continue; }


			if(useTOTCut)
			{
				const double t = (*tot)[i];

				if(t < minTOT || t > maxTOT)
				{
					continue;
				}
			}


			hist[layer][phiIndex]->Fill(dtime);


			acceptedHits++;
		}
	}


	cout << endl;
	cout << "Accepted tracks : "
		<< acceptedTracks
		<< endl;

	cout << "Accepted hits   : "
		<< acceptedHits
		<< endl;


	// histogramme normieren
	for(int layer = 0; layer < nLayers; ++layer)
	{
		for (int p = 0; p < nPhiBins; ++p)
		{
			TH1D* h = hist[layer][p];

			const double integral = h->Integral("width");


			if(integral > 0.0)
			{
				h->Scale(1.0 / integral);
			}
		}
	}


	// fit ergebnisse
	vector<vector<FitResult>> fitResults(nLayers,vector<FitResult>(nPhiBins));

	for(int layer = 0; layer < nLayers; ++layer)
	{
		// pdf
		TString pdfName;

		pdfName.Form("%s/histograms_%s.pdf", outputDir,layerName[layer]);


		TCanvas c(
				Form("c_%s", layerName[layer]),
				Form("c_%s", layerName[layer]),
				900,
				600);

		if(savePDF) { c.Print(pdfName + "["); }


		vector<vector<double>> allX(nPhiBins);

		vector<vector<double>> allY(nPhiBins);

		vector<double> t0Init(nPhiBins, 0.0);

		for (int p = 0; p < nPhiBins; ++p)
		{
			TH1D* h = hist[layer][p];


			double maxY = -1.0;
			double maxT = 0.0;


			for(int b = 1; b <= h->GetNbinsX(); ++b)
			{
				const double t = h->GetBinCenter(b);
				const double y = h->GetBinContent(b);


				allX[p].push_back(t);
				allY[p].push_back(y);


				if(y > maxY)
				{
					maxY = y;
					maxT = t;
				}
			}


            t0Init[p] = maxT;
        }


		/// plateau fit
		vector<double> At(nPhiBins,0.0);


		double sigma_e = 5.0;
		double sigma_t = 5.0;
		double L = 100.0;


		const bool plateauOK = RunPlateauFit(
				allX,
				allY,
				t0Init,
				fit2Min,
				fit2Max,
				At,
				sigma_e,
				sigma_t,
				L);


		cout << endl;
		cout << "==================================================" << endl;
		cout << "Layer "
			<< layerName[layer]
			<< endl;

		cout << "Plateau fit: "
			<< (plateauOK ? "SUCCESS" : "FAILED")
			<< endl;

		cout << "sigma_e = "
			<< sigma_e
			<< endl;

		cout << "sigma_t = "
			<< sigma_t
			<< endl;

		cout << "L       = "
			<< L
			<< endl;

		cout << "==================================================" << endl;


		// plateau abziehen & peak fitten
		vector<vector<double>> xFit(nPhiBins);
		vector<vector<double>> yFit(nPhiBins);


		for(int p = 0; p < nPhiBins; ++p)
		{
			vector<double>& x = allX[p];
			vector<double>& y = allY[p];

			double maxCorrected = 0.0;
			double peakT = t0Init[p];

			vector<double> xPeak;
			vector<double> yPeak;


			for(size_t j = 0; j < x.size(); ++j)
			{
				// originaldaten fuer joint-fit
				if((x[j] >= fit1Min && x[j] <= fit1Max) || (x[j] >= fit2Min &&  x[j] <= fit2Max))
				{
					xFit[p].push_back(x[j]);
					yFit[p].push_back(y[j]);
				}


				// plateau abziehen
				if(x[j] < fit1Min || x[j] > fit1Max)
				{
					continue;
				}


				const double plateau = At[p] *
					NormalCDF(x[j],t0Init[p],sigma_e) *
					(1.0 -
					 NormalCDF(x[j], t0Init[p] + L,sigma_t));


				const double yc = std::max(0.0, y[j] - plateau);


				xPeak.push_back(x[j]);
				yPeak.push_back(yc);


				if(yc > maxCorrected)
				{
					maxCorrected = yc;
					peakT = x[j];
				}
			}


			FitResult& r = fitResults[layer][p];

			r.At = At[p];
			r.t0 = peakT;
			r.sigma = 5.0;
			r.s = 0.5;
			r.sigma_e = sigma_e;
			r.sigma_t = sigma_t;
			r.L = L;
			r.Ap = maxCorrected;

			// Skewed-Gaussian-Fit
			if(xPeak.size() >= 5 && maxCorrected > 0.0)
			{
				TGraph gr(
						static_cast<int>(xPeak.size()),
						xPeak.data(),
						yPeak.data());


				TF1 peakFit(
						Form("peak_%s_%03d",layerName[layer],p),
						SkewedGaussian,
						fit1Min,
						fit1Max,
						4);


				peakFit.SetParameters(
						maxCorrected,
						peakT,
						5.0,
						0.5);


				peakFit.SetParLimits(
						0,
						0.0,
						10.0);

				peakFit.SetParLimits(
						1,
						fit1Min,
						fit1Max);

				peakFit.SetParLimits(
						2,
						0.1,
						50.0);

				peakFit.SetParLimits(
						3,
						-5.0,
						5.0);

				gr.Fit(&peakFit,"Q0");


				r.Ap = peakFit.GetParameter(0);
				r.t0 = peakFit.GetParameter(1);
				r.sigma = peakFit.GetParameter(2);
				r.s = peakFit.GetParameter(3);
			}
		}


	
		// globale startwerte fuer joint-fit
        
		vector<double> sigmaValues;
        
		vector<double> sValues;


        
		for(const auto& r : fitResults[layer])
        
		{
            
			sigmaValues.push_back(
                
					r.sigma);

            
			sValues.push_back(
                
					r.s);
        
		}


        
		std::sort(
            
				sigmaValues.begin(),
            
				sigmaValues.end());

        
		std::sort(
            
				sValues.begin(),
            
				sValues.end());


        
		double sigmaGlobal = 0.0;
        
		double sGlobal = 0.0;


        
		if(!sigmaValues.empty())
        
		{
            
			const size_t n =
                
				sigmaValues.size();

            
			if(n % 2 == 0)
            
			{
                
				sigmaGlobal =
                    
					0.5 *
                    
					(sigmaValues[n/2 - 1] +
                     
					 sigmaValues[n/2]);
            
			}
            
			else
            
			{
                
				sigmaGlobal =
                    
					sigmaValues[n/2];
            
			}
        
		}


        
		if(!sValues.empty())
        
		{
            
			const size_t n =
                
				sValues.size();

            
			if(n % 2 == 0)
            
			{
                
				sGlobal =
                    
					0.5 *
                    
					(sValues[n/2 - 1] +
                     
					 sValues[n/2]);
            
			}
            
			else
            
			{
                
				sGlobal =
                    
					sValues[n/2];
            
			}
        
		}



		// joint fit
		cout << endl;
		cout << "Joint fit..." << endl;


		const bool jointOK =
			RunJointFit(
					xFit,
					yFit,
					fitResults[layer],
					sigmaGlobal,
					sGlobal,
					sigma_e,
					sigma_t,
					L);

		cout << "Joint fit: "
			<< (jointOK ? "SUCCESS" : "FAILED")
			<< endl;


		cout << "sigma   = "
			<< sigmaGlobal
			<< endl;

		cout << "s       = "
			<< sGlobal
			<< endl;

		cout << "sigma_e = "
			<< sigma_e
			<< endl;

		cout << "sigma_t = "
			<< sigma_t
			<< endl;

		cout << "L       = "
			<< L
			<< endl;


		// fit plots
		for(int p = 0; p < nPhiBins; ++p)
		{
            
			const FitResult& r = fitResults[layer][p];

            
			TH1D* h = hist[layer][p];

			h->SetLineWidth(2);


			TString title;
			title.Form("%s   phi = %.1f deg",layerName[layer],phiBins[p]);

			h->SetTitle(title);
			h->GetXaxis()->SetTitle("Drift time (ns)");
			h->GetYaxis()->SetTitle("Normalized counts");


			// gesamtfit
			h->Draw("hist");


			TF1 totalFit(
					Form("total_%s_%03d", layerName[layer], p),
					DriftModel,
					tMin,
					tMax,
					8);


			totalFit.SetParameters(
					r.Ap,
					r.t0,
					r.sigma,
					r.s,
					r.At,
					r.sigma_e,
					r.sigma_t,
					r.L);


			totalFit.SetLineWidth(2);
			totalFit.Draw("same");

			if(savePDF) { c.Print(pdfName); }

			// residuen
			TH1D hResidual(
					Form("residual_%s_%03d", layerName[layer], p),
					title,
					static_cast<int>( (fit2Max - fit1Min) / dt),
					fit1Min, fit2Max);


			for(int b = 1; b <= h->GetNbinsX(); ++b)
			{
				const double t = h->GetBinCenter(b);


				if(!( (t >= fit1Min &&  t <= fit1Max) || (t >= fit2Min && t <= fit2Max)))
				{
					continue;
				}


				const double y = h->GetBinContent(b);

				const double yf = DriftModelValue(
						t,
						r.Ap,
						r.t0,
						r.sigma,
						r.s,
						r.At,
						r.sigma_e,
						r.sigma_t,
						r.L);

				hResidual.Fill(t, y - yf);
			}

			hResidual.SetTitle(Form("Residuals %s, phi = %.1f deg",layerName[layer],phiBins[p]));

			hResidual.GetXaxis()->SetTitle("Drift time (ns)");
			hResidual.GetYaxis()->SetTitle("Data - fit");

			hResidual.Draw("hist");


			if(savePDF) { c.Print(pdfName); }


			// fit-komponenten
			h->SetTitle(
					Form(
						"%s, phi = %.1f deg - fit components",
						layerName[layer],
						phiBins[p]));


			h->Draw("hist");


			TF1 totalComp(
					Form("totalComp_%s_%03d", layerName[layer],p),
					DriftModel,
					tMin,
					tMax,
					8);


			totalComp.SetParameters(
					r.Ap,
					r.t0,
					r.sigma,
					r.s,
					r.At,
					r.sigma_e,
					r.sigma_t,
					r.L);


			TF1 peakComp(
					Form("peakComp_%s_%03d", layerName[layer],p),
					SkewedGaussian,
					tMin,
					tMax,
					4);


			peakComp.SetParameters(
					r.Ap,
					r.t0,
					r.sigma,
					r.s);


			TF1 plateauComp(
					Form("plateauComp_%s_%03d", layerName[layer],p),
					[](double *x, double *par)
					{
					return
					par[0] *
					NormalCDF(x[0],par[1],par[2]) *
					(1.0 - NormalCDF( x[0],par[1] + par[3],par[4]));
					},
					tMin,
					tMax,
					5);


			plateauComp.SetParameters(
					r.At,
					r.t0,
					r.sigma_e,
					r.L,
					r.sigma_t);

			totalComp.SetLineWidth(2);
			peakComp.SetLineStyle(2);
			plateauComp.SetLineStyle(3);

			totalComp.Draw("same");
			peakComp.Draw("same");
			plateauComp.Draw("same");


			TLegend leg(0.60,0.65,0.88,0.88);
			leg.AddEntry(h,"Data","l");
			leg.AddEntry(&totalComp,"Total fit","l");
			leg.AddEntry(&peakComp,"Peak","l");
			leg.AddEntry(&plateauComp,"Plateau","l");

			leg.Draw();

			if(savePDF) { c.Print(pdfName); }
		}

		if(savePDF) { c.Print(pdfName + "]"); }


		// Ap/At vs phi -> fit -> extrapolieren
		TCanvas ratioCanvas(
				Form("ratioCanvas_%s",layerName[layer]),
				Form("Ap/At %s",layerName[layer]),
				900,
				600);


		TGraph ratioGraph(nPhiBins);

		for(int p = 0; p < nPhiBins; ++p)
		{
			const FitResult& r = fitResults[layer][p];
			const double ratio = (r.At > 0.0) ? r.Ap / r.At : 0.0;
			ratioGraph.SetPoint(p, phiBins[p], ratio);
		}


		TF1 ratioFit(
				Form("ratioFit_%s", layerName[layer]),
				"[0]+[1]*x+[2]*x*x",
				calibPhiMin,
				calibPhiMax);


		ratioFit.SetParameters(0.0,0.0,0.0);

		ratioGraph.Fit(&ratioFit,"Q");

		ratioGraph.SetTitle(Form("%s: Ap/At vs. phi",layerName[layer]));
		ratioGraph.GetXaxis()->SetTitle("phi (deg)");
		ratioGraph.GetYaxis()->SetTitle("Ap/At");

		ratioGraph.SetMarkerStyle(20);
		ratioGraph.Draw("AP");
		ratioFit.Draw("same");


		// quadr. fit ueber kompletten tabellenbereich
		TF1 ratioTableFit(
				Form("ratioTableFit_%s",layerName[layer]),
				"[0]+[1]*x+[2]*x*x",
				tablePhiMin,
				tablePhiMax);


		ratioTableFit.SetParameters(
				ratioFit.GetParameter(0),
				ratioFit.GetParameter(1),
				ratioFit.GetParameter(2));


		ratioTableFit.SetLineStyle(2);
		ratioTableFit.Draw("same");


		if(savePDF)
		{
			TString ratioPDF;
			ratioPDF.Form("%s/ApAt_%s.pdf",outputDir,layerName[layer]);

			ratioCanvas.Print(ratioPDF);
		}


		cout << endl;
		cout << "Ap/At quadratic fit "
			<< layerName[layer]
			<< ":" << endl;

		cout << "a = "
			<< ratioFit.GetParameter(0)
			<< endl;

		cout << "b = "
			<< ratioFit.GetParameter(1)
			<< endl;

		cout << "c = "
			<< ratioFit.GetParameter(2)
			<< endl;


		// driftweg-tabelle
		//
		TString outname;
		outname.Form("%s/driftlength_relation_%s.txt",outputDir,layerName[layer]);

		ofstream out(outname.Data());

		out << fixed;
		out << "t(ns)";


		for(double p : tablePhiBins)
		{
			out << "\t" << p;
		}


		out << endl;


		// globale fitparameter
		const double sigmaGlobalFinal = sigmaGlobal;
		const double sGlobalFinal = sGlobal;
		const double sigma_e_final = sigma_e;
		const double sigma_t_final = sigma_t;
		const double L_final = L;

		// driftwege berechnen
		const int nTableTimes = static_cast<int>(std::round((tMax - tMin) / dt)) + 1;
		vector<vector<double>> driftLength(tablePhiBins.size(), vector<double>(nTableTimes,0.0));

		vector<double> totalIntegral( tablePhiBins.size(), 0.0);


		for(size_t k = 0; k < tablePhiBins.size(); ++k)
		{
			const double tablePhi = tablePhiBins[k];

			int sourceIndex = -1;


			// Exakter Treffer
			for(int p = 0; p < nPhiBins; ++p)
			{
				if(std::fabs(phiBins[p] - tablePhi) < 1e-9)
				{
					sourceIndex = p;
					break;
				}
			}


			if(sourceIndex < 0)
			{
				if(tablePhi < phiBins.front())
				{
					sourceIndex = 0;
				}
				else
				{
					sourceIndex = nPhiBins - 1;
				}
			}


			const FitResult& source = fitResults[layer][sourceIndex];


			const double t0 = source.t0;
			const double AtValue = source.At;


			// Ap/At aus quadr. fit
			const double fPhi = ratioFit.Eval(tablePhi);


			const double ApValue = fPhi * AtValue;


			// Modell
			auto model = [&](double t)
			{
				return
					DriftModelValue(
							t,
							ApValue,
							t0,
							sigmaGlobalFinal,
							sGlobalFinal,
							AtValue,
							sigma_e_final,
							sigma_t_final,
							L_final);
			};


			// Gesamtintegral 0 ... 250 ns
			// Trapezregel
			double integral = 0.0;

			double previousT = tMin;
			double previousValue = model(previousT);

			for(double t = tMin + dt; t <= tMax + 1e-9; t += dt)
			{
				const double currentValue = model(t);
				integral += 0.5 * (previousValue + currentValue) * dt;

				previousT = t;
				previousValue = currentValue;
			}


			totalIntegral[k] = integral;


			// Laufendes Integral
			double running = 0.0;
			driftLength[k][0] = 0.0;

			for(int j = 1; j < nTableTimes; ++j)
			{
				const double t = tMin + j * dt;
				const double tPrevious = t - dt;
				const double yPrevious = model(tPrevious);
				const double yCurrent = model(t);

				running += 0.5 * (yPrevious + yCurrent) * dt;

				if(integral > 0.0)
				{
					driftLength[k][j] = driftCellHeight * running / integral;
				}
				else
				{
					driftLength[k][j] = 0.0;
				}
			}
		}


		// tabelle schreiben
		for(int j = 0; j < nTableTimes; ++j)
		{
			const double t = tMin + j * dt;
			out << setprecision(1) << t;

			for(size_t k = 0; k < tablePhiBins.size(); ++k)
			{
				out << "\t"
					<< setprecision(10)
					<< driftLength[k][j];
			}

			out << endl;
		}
		out.close();

		// driftlaenge vs driftzeit 
		TCanvas driftCanvas(
				Form("driftCanvas_%s",layerName[layer]),
				Form("Drift length %s",layerName[layer]),
				900,
				600);


		TLegend driftLegend(0.62,0.65,0.88,0.88);
		bool drewAny = false;

		vector<double> requestedAngles =
		{
			25.0,
			45.0,
			65.0
		};


		for(double requestedPhi : requestedAngles)
		{
			int pFound = -1;

			for(size_t k = 0; k < tablePhiBins.size(); ++k)
			{
				if(std::fabs(tablePhiBins[k] - requestedPhi)
						< 1e-9)
				{
					pFound = static_cast<int>(k);
					break;
				}
			}


			if(pFound < 0) { continue; }

			TGraph* g = new TGraph(nTableTimes);

			for(int j = 0; j < nTableTimes; ++j)
			{
				const double t = tMin + j * dt;

				g->SetPoint(j,t,driftLength[pFound][j]);
			}

			g->SetLineWidth(2);

			if(!drewAny)
			{
				g->SetTitle(Form("%s: Drift length vs. drift time",layerName[layer]));
				g->GetXaxis()->SetTitle("Drift time (ns)");
				g->GetYaxis()->SetTitle("Drift length (mm)");

				g->Draw("AL");
				drewAny = true;
			}
			else
			{
				g->Draw("L same");
			}

			driftLegend.AddEntry(g, Form("%.0f deg",requestedPhi), "l");
		}


		if(drewAny)
		{
			driftLegend.Draw();

			if(savePDF)
			{
				TString driftPDF;
				driftPDF.Form("%s/driftlength_vs_drifttime_%s.pdf",outputDir,layerName[layer]);

				driftCanvas.Print(driftPDF);
			}
		}



		// fitparameter ausgeben
		TString paramName;
		paramName.Form("%s/fit_parameters_%s.txt",outputDir,layerName[layer]);

		ofstream paramOut(paramName.Data());

		paramOut << fixed;
		paramOut
			<< "phi\tAp\tt0\tsigma\ts\tAt"
			<< "\tsigma_e\tsigma_t\tL\tAp/At"
			<< endl;


		for(int p = 0; p < nPhiBins; ++p)
		{
			const FitResult& r =fitResults[layer][p];

			paramOut
				<< setprecision(6)
				<< phiBins[p]
				<< "\t"
				<< r.Ap
				<< "\t"
				<< r.t0
				<< "\t"
				<< r.sigma
				<< "\t"
				<< r.s
				<< "\t"
				<< r.At
				<< "\t"
				<< r.sigma_e
				<< "\t"
				<< r.sigma_t
				<< "\t"
				<< r.L
				<< "\t"
				<< ((r.At > 0.0) ? r.Ap / r.At : 0.0)
				<< endl;
		}

		paramOut.close();


		// quadratische Ap/At-Koeff. speichern
		TString ratioParamName;
		ratioParamName.Form("%s/ApAt_quadratic_%s.txt",outputDir,layerName[layer]);


		ofstream ratioOut(ratioParamName.Data());
		ratioOut << fixed;

		ratioOut
			<< "Calibration phi range: "
			<< calibPhiMin
			<< " "
			<< calibPhiMax
			<< endl;

		ratioOut
			<< "Table phi range: "
			<< tablePhiMin
			<< " "
			<< tablePhiMax
			<< endl;

		ratioOut
			<< "a = "
			<< setprecision(12)
			<< ratioFit.GetParameter(0)
			<< endl;

		ratioOut
			<< "b = "
			<< setprecision(12)
			<< ratioFit.GetParameter(1)
			<< endl;

		ratioOut
			<< "c = "
			<< setprecision(12)
			<< ratioFit.GetParameter(2)
			<< endl;


		ratioOut.close();
	}


	//fin->Close();

	cout << endl;
	cout << "==================================================" << endl;
	cout << "Drift calibration finished." << endl;
	cout << "Calibration range : "
	       	<< calibPhiMin
		<< " ... "
		<< calibPhiMax
		<< " deg"
		<< endl;

	cout << "Table range       : "
		<< tablePhiMin
		<< " ... "
		<< tablePhiMax
		<< " deg"
		<< endl;

	cout << "Output directory  : "
		<< outputDir
		<< endl;

	cout << "==================================================" << endl;
}
