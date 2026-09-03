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

// ==========================================================================
// Fit-Ergebnis pro Winkel
// ==========================================================================
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

// ==========================================================================
// Modell
// ==========================================================================
double NormalCDF(double x, double mu, double sigma)
{
	if(sigma <= 0.0) { return 0.0; }
	return 0.5 * (1.0 + TMath::Erf((x - mu) / (std::sqrt(2.0) * sigma)));
}

double SkewedGaussian(double *x, double *par)
{
	const double t     = x[0];
	const double Ap    = par[0];
	const double t0    = par[1];

	const double sigma = par[2];
	const double s     = par[3];

	return Ap *
		std::exp(-0.5 * std::pow((t - t0) / sigma, 2.0)) *
		(1.0 + TMath::Erf(s * (t - t0) /
				  (std::sqrt(2.0) * sigma)));
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
	const double peak =
		Ap *
		std::exp(-0.5 * std::pow((t - t0) / sigma, 2.0)) *
		(1.0 + TMath::Erf(s * (t - t0) /
				  (std::sqrt(2.0) * sigma)));

	const double plateau =
		At *
		NormalCDF(t, t0, sigma_e) *
		(1.0 - NormalCDF(t, t0 + L, sigma_t));

	return peak + plateau;
}

double DriftModel(double *x, double *par)
{
	return DriftModelValue(
			x[0],
			par[0], par[1], par[2], par[3],
			par[4], par[5], par[6], par[7]);
}

// ==========================================================================
// Quadratischer Fit Ap/At(phi)
// ==========================================================================
double QuadFunc(double phi, double a, double b, double c)
{
	return a + b * phi + c * phi * phi;
}

// ==========================================================================
// Minimizer-Hilfsfunktionen
// ==========================================================================

bool RunPlateauFit(
		const vector<vector<double>>& x,
		const vector<vector<double>>& y,
		const vector<double>& t0Init,
		vector<double>& At,
		double& sigma_e,
		double& sigma_t,
		double& L)
{
	const int nPhi = static_cast<int>(x.size());

	// Parameter:
	// [0 ... nPhi-1] = At(phi)
	// [nPhi]         = sigma_e
	// [nPhi+1]       = sigma_t
	// [nPhi+2]       = L
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

			for(size_t j = 0; j < x[i].size(); ++j)
			{
				const double t = x[i][j];

				if(t < 100.0 || t > 225.0) { continue; }

				const double model =
					ati *
					NormalCDF(t, t0, se) *
					(1.0 - NormalCDF(t, t0 + l, st));

				const double r = y[i][j] - model;
				chi2 += r * r;
			}
		}

		return chi2;
	};

	unique_ptr<ROOT::Math::Minimizer> min(
			ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad"));

	if(!min) { return false; }

	ROOT::Math::Functor f(objective, nPar);
	min->SetFunction(f);
	min->SetMaxFunctionCalls(200000);

	min->SetMaxIterations(20000);
 
	min->SetTolerance(1e-7);
	min->SetPrintLevel(0);

	for(int i = 0; i < nPhi; ++i)
	{
		double guess = 0.1;

		double maxY = 0.0;
		for(size_t j = 0; j < x[i].size(); ++j)
		{
			if(x[i][j] >= 100.0 && x[i][j] <= 225.0)
			{
				maxY = std::max(maxY, y[i][j]);
			}
		}

		if(maxY > 0.0) { guess = maxY; }

		min->SetVariable(i, Form("At_%d", i), guess, 0.01);
		min->SetVariableLimits(i, 0.0, 10.0);
	}

	min->SetVariable(nPhi,     "sigma_e", 5.0, 0.05);
	min->SetVariable(nPhi + 1, "sigma_t", 5.0, 0.05);
	min->SetVariable(nPhi + 2, "L",       100.0, 0.2);

	min->SetVariableLimits(nPhi,     0.1, 50.0);
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
	L       = p[nPhi + 2];

	return true;
}

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

	// [3*i+0] = Ap
	// [3*i+1] = t0
	// [3*i+2] = At
	// last five = sigma, s, sigma_e, sigma_t, L
	const int nPar = 3 * nPhi + 5;

	auto objective = [&](const double *p)
	{
		double chi2 = 0.0;

		const double sig = p[3 * nPhi + 0];
		const double skw = p[3 * nPhi + 1];
		const double se  = p[3 * nPhi + 2];
		const double st  = p[3 * nPhi + 3];
		const double l   = p[3 * nPhi + 4];

		for(int i = 0; i < nPhi; ++i)
		{
			const double Ap = p[3 * i + 0];
			const double t0 = p[3 * i + 1];
			const double At = p[3 * i + 2];

			for(size_t j = 0; j < xFit[i].size(); ++j)
			{
				const double model =
					DriftModelValue(
							xFit[i][j],
							Ap, t0, sig, skw,
							At, se, st, l);

				const double r = yFit[i][j] - model;

				const double fScale = 0.1;
				const double u = r / fScale;
				chi2 += 2.0 * fScale * fScale *
					(std::sqrt(1.0 + u * u) - 1.0);
			}
		}

		return chi2;
	};

	unique_ptr<ROOT::Math::Minimizer> min(
			ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad"));

	if(!min) { return false; }

	ROOT::Math::Functor f(objective, nPar);
	min->SetFunction(f);
	min->SetMaxFunctionCalls(500000);
	min->SetMaxIterations(50000);
	min->SetTolerance(1e-7);
	min->SetPrintLevel(0);

	for(int i = 0; i < nPhi; ++i)
	{
		min->SetVariable(3 * i + 0, Form("Ap_%d", i),
				result[i].Ap, 0.001);
		min->SetVariable(3 * i + 1, Form("t0_%d", i),
				result[i].t0, 0.01);
		min->SetVariable(3 * i + 2, Form("At_%d", i),
				result[i].At, 0.001);

		min->SetVariableLimits(3 * i + 0, 0.0, 10.0);
		min->SetVariableLimits(3 * i + 1, 20.0, 90.0);
		min->SetVariableLimits(3 * i + 2, 0.0, 10.0);
	}

	min->SetVariable(3 * nPhi + 0, "sigma",   sigma,   0.05);
	min->SetVariable(3 * nPhi + 1, "s",       s,       0.02);
	min->SetVariable(3 * nPhi + 2, "sigma_e", sigma_e, 0.05);
	min->SetVariable(3 * nPhi + 3, "sigma_t", sigma_t, 0.05);
	min->SetVariable(3 * nPhi + 4, "L",       L,       0.2);

	min->SetVariableLimits(3 * nPhi + 0, 0.1, 50.0);
	min->SetVariableLimits(3 * nPhi + 1, -5.0, 5.0);
	min->SetVariableLimits(3 * nPhi + 2, 0.1, 50.0);
	min->SetVariableLimits(3 * nPhi + 3, 0.1, 50.0);
	min->SetVariableLimits(3 * nPhi + 4, 10.0, 400.0);

	const bool ok = min->Minimize();

	if(!ok) { return false; }

	const double *p = min->X();

	for(int i = 0; i < nPhi; ++i)
	{
		result[i].Ap = p[3 * i + 0];
		result[i].t0 = p[3 * i + 1];
		result[i].At = p[3 * i + 2];

		result[i].sigma = p[3 * nPhi + 0];
		result[i].s     = p[3 * nPhi + 1];
		result[i].sigma_e = p[3 * nPhi + 2];
		result[i].sigma_t = p[3 * nPhi + 3];
		result[i].L       = p[3 * nPhi + 4];
	}

	sigma   = p[3 * nPhi + 0];
	s       = p[3 * nPhi + 1];
	sigma_e = p[3 * nPhi + 2];
	sigma_t = p[3 * nPhi + 3];
	L       = p[3 * nPhi + 4];

	return true;
}

// ==========================================================================
// Hauptmethode
// ==========================================================================
void generateDriftCorrTable(
		const char* inputFile,
		const char* outputDir = ".")
{
	gStyle->SetOptStat(0);

	// ========================================================================
	// Konfiguration
	// ========================================================================

	const double driftCellHeight = 12.0;

	const double tMin = 0.0;
	const double tMax = 250.0;
	const double dt   = 0.5;


	const double phiMin = 20.0;
	const double phiMax = 65.0;
	const double phiBinWidth = 1.0;

	const bool useAbsolutePhi = true;
	const bool savePDF = true;

	// Fitbereiche wie im Python-Skript
	const double fit1Min = 30.0;
	const double fit1Max = 80.0;
	const double fit2Min = 100.0;
	const double fit2Max = 225.0;

	// ========================================================================
	// Drahtbereiche
	// ========================================================================

	vector<pair<int,int>> wireRanges =
	{
		{0, 350}
	};

	set<int> wireList =
	{
	};

  set<int> excludedWires =
  {
  };

  // ========================================================================
  // Qualitätscuts
  // ========================================================================

  bool usePullCut = false;
  double maxPull = 3.0;

  bool useResidualCut = false;
  double maxResidual = 0.5;

  bool useTOTCut = true;
  double minTOT = 10.0;
  double maxTOT = 500.0;

  // ========================================================================
  // Layer
  // ========================================================================

  const int nLayers = 3;

  const char* layerName[nLayers] =
  {
    "X1",
    "U",
    "X2"
  };

  gSystem->mkdir(outputDir, true);

  // ========================================================================
  // Phi-Bins
  // ========================================================================

  vector<double> phiBins;

  for (double phi = phiMin;
       phi <= phiMax + 1e-9;
       phi += phiBinWidth)
  {
    phiBins.push_back(phi);
  }

  const int nPhiBins = static_cast<int>(phiBins.size());

  cout << endl;
  cout << "Phi bins: " << nPhiBins << endl;

  // ========================================================================
  // ROOT öffnen
  // ========================================================================

  TFile *fin = TFile::Open(inputFile);

  if (fin == nullptr || fin->IsZombie())
  {
    cout << "Cannot open " << inputFile << endl;
    return;
  }

  TTree *tree = dynamic_cast<TTree*>(fin->Get("Track"));

  if (tree == nullptr)
  {
    cout << "Tree Track not found." << endl;
    fin->Close();
    return;
  }

  cout << "Entries : " << tree->GetEntries() << endl;

  // ========================================================================
  // TreeReader
  // ========================================================================

  TTreeReader reader(tree);

  TTreeReaderValue<double> phi(reader, "phi");

  TTreeReaderValue<vector<int>> res_layerID(
      reader, "res_layerID");

  TTreeReaderValue<vector<int>> res_wireNumber(
      reader, "res_wireNumber");

  TTreeReaderValue<vector<double>> drifttimes(
      reader, "drifttimes");

  TTreeReaderValue<vector<double>> tot(
      reader, "tot");

  TTreeReaderValue<vector<double>> residual(
      reader, "residual");

  TTreeReaderValue<vector<double>> pull(
      reader, "pull");

  // ========================================================================
  // Histogramme
  // ========================================================================

  vector<vector<TH1D*>> hist(nLayers,
                              vector<TH1D*>(nPhiBins, nullptr));

  for (int layer = 0; layer < nLayers; ++layer)
  {
    for (int p = 0; p < nPhiBins; ++p)
    {
      TString name;
      name.Form("h_%s_%03d", layerName[layer], p);

      hist[layer][p] = new TH1D(
          name,
          name,
          static_cast<int>((tMax - tMin) / dt),
          tMin,
          tMax);
    }
  }

  // ========================================================================
  // Hilfsfunktion: Draht akzeptiert?
  // ========================================================================

  auto acceptWire = [&](int wire) -> bool
  {
    if (excludedWires.count(wire))
      return false;

    if (!wireList.empty())
      return wireList.count(wire);

    for (const auto& r : wireRanges)
    {
      if (wire >= r.first && wire <= r.second)
        return true;
    }

    return false;
  };

  // ========================================================================
  // Tree einlesen
  // ========================================================================

  Long64_t acceptedHits = 0;
  Long64_t acceptedTracks = 0;

  while (reader.Next())
  {
    double phiDeg = *phi;

    if (useAbsolutePhi)
      phiDeg = std::fabs(phiDeg);

    if (phiDeg < phiMin || phiDeg > phiMax)
      continue;

    const int phiIndex =
        static_cast<int>(
            std::round((phiDeg - phiMin) / phiBinWidth));

    if (phiIndex < 0 || phiIndex >= nPhiBins)
      continue;

    acceptedTracks++;

    const size_t nHits = drifttimes->size();

    for (size_t i = 0; i < nHits; ++i)
    {
      const int layer = (*res_layerID)[i];

      if (layer < 0 || layer >= nLayers)
        continue;

      const int wire = (*res_wireNumber)[i];

      if (!acceptWire(wire))
        continue;

      const double dtime = (*drifttimes)[i];

      if (dtime < tMin || dtime > tMax)
        continue;

      if (usePullCut &&
          std::fabs((*pull)[i]) > maxPull)
        continue;

      if (useResidualCut &&
          std::fabs((*residual)[i]) > maxResidual)
        continue;

      if (useTOTCut)
      {
        const double t = (*tot)[i];

        if (t < minTOT || t > maxTOT)
          continue;
      }

      hist[layer][phiIndex]->Fill(dtime);
      acceptedHits++;
    }
  }

  cout << endl;
  cout << "Accepted tracks : " << acceptedTracks << endl;
  cout << "Accepted hits   : " << acceptedHits << endl;

  // ========================================================================
  // Histogramme normieren
  // ========================================================================

  for (int layer = 0; layer < nLayers; ++layer)
  {
    for (int p = 0; p < nPhiBins; ++p)
    {
      TH1D* h = hist[layer][p];

      const double integral = h->Integral("width");

      if (integral > 0.0)
        h->Scale(1.0 / integral);
    }
  }

  // ========================================================================
  // PDF für Histogramme/Fits
  // ========================================================================

  vector<vector<FitResult>> fitResults(
      nLayers,
      vector<FitResult>(nPhiBins));

  for (int layer = 0; layer < nLayers; ++layer)
  {
    TString pdfName;
    pdfName.Form("%s/histograms_%s.pdf",
                 outputDir,
                 layerName[layer]);

    TCanvas c(Form("c_%s", layerName[layer]),
              Form("c_%s", layerName[layer]),
              900, 600);

    if (savePDF)
      c.Print(pdfName + "[");

    // ----------------------------------------------------------------------
    // Datenvektoren
    // ----------------------------------------------------------------------

    vector<vector<double>> allX(nPhiBins);
    vector<vector<double>> allY(nPhiBins);

    vector<double> t0Init(nPhiBins, 0.0);

    for (int p = 0; p < nPhiBins; ++p)
    {
      TH1D* h = hist[layer][p];

      double maxY = -1.0;
      double maxT = 0.0;

      for (int b = 1; b <= h->GetNbinsX(); ++b)
      {
        const double t = h->GetBinCenter(b);
        const double y = h->GetBinContent(b);

        allX[p].push_back(t);
        allY[p].push_back(y);

        if (y > maxY)
        {
          maxY = y;
          maxT = t;
        }
      }

      t0Init[p] = maxT;
    }

    // ----------------------------------------------------------------------
    // 1) Plateau-Fit
    // ----------------------------------------------------------------------

    vector<double> At(nPhiBins, 0.0);

    double sigma_e = 5.0;
    double sigma_t = 5.0;
    double L       = 100.0;

    const bool plateauOK =
        RunPlateauFit(
            allX,
            allY,
            t0Init,
            At,
            sigma_e,
            sigma_t,
            L);

    cout << endl;
    cout << "==================================================" << endl;
    cout << "Layer " << layerName[layer] << endl;
    cout << "Plateau fit: "
         << (plateauOK ? "SUCCESS" : "FAILED") << endl;

    cout << "sigma_e = " << sigma_e << endl;
    cout << "sigma_t = " << sigma_t << endl;
    cout << "L       = " << L << endl;
    cout << "==================================================" << endl;

    // ----------------------------------------------------------------------
    // 2) Plateau abziehen + Peak separat fitten
    //
    // Wichtig:
    //   xFit/yFit enthalten die ORIGINALDATEN in beiden Fitregionen.
    //   Genau diese Daten werden später im Joint-Fit verwendet.
    //
    //   Für den separaten Peak-Fit verwenden wir dagegen zusätzlich
    //   xPeak/yPeak, also die um das Plateau korrigierten Daten.
    // ----------------------------------------------------------------------

    vector<vector<double>> xFit(nPhiBins);
    vector<vector<double>> yFit(nPhiBins);

    for (int p = 0; p < nPhiBins; ++p)
    {
      vector<double>& x = allX[p];
      vector<double>& y = allY[p];

      double maxCorrected = 0.0;
      double peakT = t0Init[p];

      vector<double> xPeak;
      vector<double> yPeak;

      for (size_t j = 0; j < x.size(); ++j)
      {
        // ORIGINALDATEN für den späteren Joint-Fit:
        if ((x[j] >= fit1Min && x[j] <= fit1Max) ||
            (x[j] >= fit2Min && x[j] <= fit2Max))
        {
          xFit[p].push_back(x[j]);
          yFit[p].push_back(y[j]);
        }

        // Plateau abziehen und nur Peakbereich fitten:
        if (x[j] < fit1Min || x[j] > fit1Max)
          continue;

        const double plateau =
            At[p] *
            NormalCDF(x[j], t0Init[p], sigma_e) *
            (1.0 -
             NormalCDF(x[j],
                       t0Init[p] + L,
                       sigma_t));

        const double yc =
            std::max(0.0, y[j] - plateau);

        xPeak.push_back(x[j]);
        yPeak.push_back(yc);

        if (yc > maxCorrected)
        {
          maxCorrected = yc;
          peakT = x[j];
        }
      }

      FitResult& r = fitResults[layer][p];

      r.At       = At[p];
      r.t0       = peakT;
      r.sigma    = 5.0;
      r.s        = 0.5;
      r.sigma_e  = sigma_e;
      r.sigma_t  = sigma_t;
      r.L        = L;
      r.Ap       = maxCorrected;

      if (xPeak.size() >= 5 && maxCorrected > 0.0)
      {
        TGraph gr(
            static_cast<int>(xPeak.size()),
            xPeak.data(),
            yPeak.data());

        TF1 peakFit(
            Form("peak_%s_%03d",
                 layerName[layer], p),
            SkewedGaussian,
            fit1Min,
            fit1Max,
            4);

        peakFit.SetParameters(
            maxCorrected,
            peakT,
            5.0,
            0.5);

        peakFit.SetParLimits(0, 0.0, 10.0);
        peakFit.SetParLimits(1, fit1Min, fit1Max);
        peakFit.SetParLimits(2, 0.1, 50.0);
        peakFit.SetParLimits(3, -5.0, 5.0);

        gr.Fit(&peakFit, "Q0");

        r.Ap    = peakFit.GetParameter(0);
        r.t0    = peakFit.GetParameter(1);
        r.sigma = peakFit.GetParameter(2);
        r.s     = peakFit.GetParameter(3);
      }
    }

    // ----------------------------------------------------------------------
    // 3) Gemeinsamer Joint-Fit über alle Winkel
    // ----------------------------------------------------------------------

    double sigmaGlobal = 0.0;
    double sGlobal = 0.0;

    for (const auto& r : fitResults[layer])
    {
      sigmaGlobal += r.sigma;
      sGlobal += r.s;
    }

    sigmaGlobal /= nPhiBins;
    sGlobal /= nPhiBins;

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
         << (jointOK ? "SUCCESS" : "FAILED") << endl;

    cout << "sigma   = " << sigmaGlobal << endl;
    cout << "s       = " << sGlobal << endl;
    cout << "sigma_e = " << sigma_e << endl;
    cout << "sigma_t = " << sigma_t << endl;
    cout << "L       = " << L << endl;

    // ----------------------------------------------------------------------
    // Fitplots
    // ----------------------------------------------------------------------

    for (int p = 0; p < nPhiBins; ++p)
    {
      const FitResult& r = fitResults[layer][p];

      // ------------------------------------------------------------
      // Gesamtverteilung + Gesamtfit + Komponenten
      // ------------------------------------------------------------

      TH1D* h = hist[layer][p];

      h->SetLineWidth(2);

      TString title;
      title.Form("%s   phi = %.1f deg",
                 layerName[layer],
                 phiBins[p]);

      h->SetTitle(title);
      h->GetXaxis()->SetTitle("Drift time (ns)");
      h->GetYaxis()->SetTitle("Normalized counts");
      h->Draw("hist");

      TF1 totalFit(
          Form("total_%s_%03d",
               layerName[layer], p),
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

      if (savePDF)
        c.Print(pdfName);

      // ------------------------------------------------------------
      // Residuen im Fitbereich
      // ------------------------------------------------------------

      TH1D hResidual(
          Form("residual_%s_%03d",
               layerName[layer], p),
          title,
          static_cast<int>((fit1Max - fit1Min) / dt +
                           (fit2Max - fit2Min) / dt),
          0.0,
          fit2Max - fit1Min);

      for (int b = 1; b <= h->GetNbinsX(); ++b)
      {
        const double t = h->GetBinCenter(b);

        if (!((t >= fit1Min && t <= fit1Max) ||
              (t >= fit2Min && t <= fit2Max)))
          continue;

        const double y = h->GetBinContent(b);
        const double yf =
            DriftModelValue(
                t,
                r.Ap,
                r.t0,
                r.sigma,
                r.s,
                r.At,
                r.sigma_e,
                r.sigma_t,
                r.L);

        // Einfacher Residuen-Plot über die tatsächliche Zeitachse
        hResidual.Fill(t - fit1Min, y - yf);
      }

      hResidual.SetTitle(
          Form("Residuals %s, phi = %.1f deg",
               layerName[layer],
               phiBins[p]));
      hResidual.GetXaxis()->SetTitle("Drift time (ns)");
      hResidual.GetYaxis()->SetTitle("Data - fit");
      hResidual.Draw("hist");

      if (savePDF)
        c.Print(pdfName);

      // ------------------------------------------------------------
      // Komponenten-Overlay
      // ------------------------------------------------------------

      h->SetTitle(
          Form("%s, phi = %.1f deg - fit components",
               layerName[layer],
               phiBins[p]));
      h->Draw("hist");

      TF1 totalComp(
          Form("totalComp_%s_%03d",
               layerName[layer], p),
          DriftModel,
          tMin,
          tMax,
          8);

      totalComp.SetParameters(
          r.Ap, r.t0, r.sigma, r.s,
          r.At, r.sigma_e, r.sigma_t, r.L);

      TF1 peakComp(
          Form("peakComp_%s_%03d",
               layerName[layer], p),
          SkewedGaussian,
          tMin,
          tMax,
          4);

      peakComp.SetParameters(
          r.Ap, r.t0, r.sigma, r.s);

      TF1 plateauComp(
          Form("plateauComp_%s_%03d",
               layerName[layer], p),
          [](double *x, double *par)
          {
            return par[0] *
                   NormalCDF(x[0], par[1], par[2]) *
                   (1.0 -
                    NormalCDF(x[0] + 0.0,
                              par[1] + par[3],
                              par[4]));
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

      TLegend leg(0.60, 0.65, 0.88, 0.88);
      leg.AddEntry(h, "Data", "l");
      leg.AddEntry(&totalComp, "Total fit", "l");
      leg.AddEntry(&peakComp, "Peak", "l");
      leg.AddEntry(&plateauComp, "Plateau", "l");
      leg.Draw();

      if (savePDF)
        c.Print(pdfName);
    }

    if (savePDF)
      c.Print(pdfName + "]");

    // ======================================================================
    // Ap/At vs phi + quadratischer Fit
    // ======================================================================

    TCanvas ratioCanvas(
        Form("ratioCanvas_%s", layerName[layer]),
        Form("Ap/At %s", layerName[layer]),
        900, 600);

    TGraph ratioGraph(nPhiBins);

    for (int p = 0; p < nPhiBins; ++p)
    {
      const FitResult& r = fitResults[layer][p];

      const double ratio =
          (r.At > 0.0) ? r.Ap / r.At : 0.0;

      ratioGraph.SetPoint(
          p,
          phiBins[p],
          ratio);
    }

    TF1 ratioFit(
        Form("ratioFit_%s", layerName[layer]),
        "[0]+[1]*x+[2]*x*x",
        phiBins.front(),
        phiBins.back());

    ratioFit.SetParameters(0.0, 0.0, 0.0);

    ratioGraph.Fit(&ratioFit, "Q");

    ratioGraph.SetTitle(
        Form("%s: Ap/At vs. phi",
             layerName[layer]));
    ratioGraph.GetXaxis()->SetTitle("phi (deg)");
    ratioGraph.GetYaxis()->SetTitle("Ap/At");
    ratioGraph.SetMarkerStyle(20);
    ratioGraph.Draw("AP");
    ratioFit.Draw("same");

    if (savePDF)
    {
      TString ratioPDF;
      ratioPDF.Form("%s/ApAt_%s.pdf",
                    outputDir,
                    layerName[layer]);

      ratioCanvas.Print(ratioPDF);
    }

    cout << endl;
    cout << "Ap/At quadratic fit " << layerName[layer] << ":" << endl;
    cout << "a = " << ratioFit.GetParameter(0) << endl;
    cout << "b = " << ratioFit.GetParameter(1) << endl;
    cout << "c = " << ratioFit.GetParameter(2) << endl;

    // ======================================================================
    // Driftlängentabelle
    //
    // WICHTIG:
    // Es wird KEINE phi-Extrapolation gemacht.
    // Die Tabelle enthält ausschließlich die tatsächlich verwendeten phiBins.
    // ======================================================================

    TString outname;
    outname.Form("%s/driftlength_relation_%s.txt",
                 outputDir,
                 layerName[layer]);

    ofstream out(outname.Data());

    out << fixed;
    out << "t(ns)";

    for (double p : phiBins)
      out << "\t" << p;

    out << endl;

    // Gesamtintegrale
    vector<double> totalIntegral(nPhiBins, 0.0);

    for (int p = 0; p < nPhiBins; ++p)
    {
      double previousT = tMin;

      double previousValue =
          DriftModelValue(
              previousT,
              fitResults[layer][p].Ap,
              fitResults[layer][p].t0,
              fitResults[layer][p].sigma,
              fitResults[layer][p].s,
              fitResults[layer][p].At,
              fitResults[layer][p].sigma_e,
              fitResults[layer][p].sigma_t,
              fitResults[layer][p].L);

      for (double t = tMin + dt;
           t <= tMax + 1e-9;
           t += dt)
      {
        const double currentValue =
            DriftModelValue(
                t,
                fitResults[layer][p].Ap,
                fitResults[layer][p].t0,
                fitResults[layer][p].sigma,
                fitResults[layer][p].s,
                fitResults[layer][p].At,
                fitResults[layer][p].sigma_e,
                fitResults[layer][p].sigma_t,
                fitResults[layer][p].L);

        totalIntegral[p] +=
            0.5 * (previousValue + currentValue) * dt;

        previousT = t;
        previousValue = currentValue;
      }
    }

    vector<double> runningIntegral(nPhiBins, 0.0);

    // Speichern der Driftlänge auch für den Plot
    vector<vector<double>> driftLength(
        nPhiBins,
        vector<double>(
            static_cast<int>((tMax - tMin) / dt) + 1,
            0.0));

    int timeIndex = 0;

    for (double t = tMin;
         t <= tMax + 1e-9;
         t += dt)
    {
      out << setprecision(1) << t;

      for (int p = 0; p < nPhiBins; ++p)
      {
        if (t > tMin)
        {
          const double previousValue =
              DriftModelValue(
                  t - dt,
                  fitResults[layer][p].Ap,
                  fitResults[layer][p].t0,
                  fitResults[layer][p].sigma,
                  fitResults[layer][p].s,
                  fitResults[layer][p].At,
                  fitResults[layer][p].sigma_e,
                  fitResults[layer][p].sigma_t,
                  fitResults[layer][p].L);

          const double currentValue =
              DriftModelValue(
                  t,
                  fitResults[layer][p].Ap,
                  fitResults[layer][p].t0,
                  fitResults[layer][p].sigma,
                  fitResults[layer][p].s,
                  fitResults[layer][p].At,
                  fitResults[layer][p].sigma_e,
                  fitResults[layer][p].sigma_t,
                  fitResults[layer][p].L);

          runningIntegral[p] +=
              0.5 * (previousValue + currentValue) * dt;
        }

        double dl = 0.0;

        if (totalIntegral[p] > 0.0)
        {
          dl =
              driftCellHeight *
              runningIntegral[p] /
              totalIntegral[p];
        }

        driftLength[p][timeIndex] = dl;

        out << "\t"
            << setprecision(10)
            << dl;
      }

      out << endl;
      timeIndex++;
    }

    out.close();

    // ======================================================================
    // Driftlänge vs Driftzeit
    //
    // Nur Winkel, die tatsächlich in phiBins vorhanden sind.
    // ======================================================================

    TCanvas driftCanvas(
        Form("driftCanvas_%s", layerName[layer]),
        Form("Drift length %s", layerName[layer]),
        900, 600);

    TLegend driftLegend(0.62, 0.65, 0.88, 0.88);

    bool drewAny = false;

    vector<double> requestedAngles =
    {
      25.0,
      45.0,
      65.0
    };

    for (double requestedPhi : requestedAngles)
    {
      int pFound = -1;

      for (int p = 0; p < nPhiBins; ++p)
      {
        if (std::fabs(phiBins[p] - requestedPhi) < 1e-6)
        {
          pFound = p;
          break;
        }
      }

      // Winkel nicht vorhanden -> NICHT extrapolieren.
      if (pFound < 0)
        continue;

      TGraph* g =
          new TGraph(
              static_cast<int>(driftLength[pFound].size()));

      for (size_t j = 0;
           j < driftLength[pFound].size();
           ++j)
      {
        const double t =
            tMin + static_cast<double>(j) * dt;

        g->SetPoint(
            static_cast<int>(j),
            t,
            driftLength[pFound][j]);
      }

      g->SetLineWidth(2);

      if (!drewAny)
      {
        g->SetTitle(
            Form("%s: Drift length vs. drift time",
                 layerName[layer]));
        g->GetXaxis()->SetTitle("Drift time (ns)");
        g->GetYaxis()->SetTitle("Drift length (mm)");
        g->Draw("AL");
        drewAny = true;
      }
      else
      {
        g->Draw("L same");
      }

      driftLegend.AddEntry(
          g,
          Form("%.0f deg", phiBins[pFound]),
          "l");
    }

    if (drewAny)
    {
      driftLegend.Draw();

      if (savePDF)
      {
        TString driftPDF;
        driftPDF.Form(
            "%s/driftlength_vs_drifttime_%s.pdf",
            outputDir,
            layerName[layer]);

        driftCanvas.Print(driftPDF);
      }
    }

    // ======================================================================
    // Fitparameter ausgeben
    // ======================================================================

    TString paramName;
    paramName.Form(
        "%s/fit_parameters_%s.txt",
        outputDir,
        layerName[layer]);

    ofstream paramOut(paramName.Data());

    paramOut << fixed;
    paramOut
        << "phi\tAp\tt0\tsigma\ts\tAt\tsigma_e\tsigma_t\tL\tAp/At"
        << endl;

    for (int p = 0; p < nPhiBins; ++p)
    {
      const FitResult& r = fitResults[layer][p];

      paramOut
          << setprecision(6)
          << phiBins[p] << "\t"
          << r.Ap << "\t"
          << r.t0 << "\t"
          << r.sigma << "\t"
          << r.s << "\t"
          << r.At << "\t"
          << r.sigma_e << "\t"
          << r.sigma_t << "\t"
          << r.L << "\t"
          << ((r.At > 0.0) ? r.Ap / r.At : 0.0)
          << endl;
    }

    paramOut.close();
  }

  fin->Close();

  cout << endl;
  cout << "==================================================" << endl;
  cout << "Drift calibration finished." << endl;
  cout << "Output directory: " << outputDir << endl;
  cout << "==================================================" << endl;
}
