#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2D.h>
#include <TPolyLine.h>
#include <TBox.h>
#include <TMath.h>
#include <vector>
#include <iostream>

// =========================
// ROOT GUI
// =========================
#include <TGClient.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TSystem.h>

// =========================
// GLOBALS
// =========================
TTree* tree = nullptr;
TCanvas* cEvent = nullptr;

std::vector<int>*    res_layerID    = nullptr;
std::vector<int>*    res_wireNumber = nullptr;
std::vector<double>* z_measured     = nullptr;

double phi = 0;
double xD_X1 = 0;
double xD_U = 0;
double xD_X2 = 0;
double yD_U = 0;
double yD_U_corr = 0;

Double_t gamma_angle = 26.5*M_PI/180.;

Int_t nBinsX = 1200;
Double_t minX = -100.;
Double_t maxX = 1000.;
Int_t nBinsZ = 240;
Double_t minZ = -12.;
Double_t maxZ = 12.;

TString tree_name = "Track";
TString layerID_Branch = "res_layerID";
TString wireNumber_Branch = "res_wireNumber";
TString z_measured_Branch = "z_measured";

TString phi_Branch = "phi";
TString xD_X1_Branch = "xD_X1";
TString xD_U_Branch = "xD_U";
TString xD_X2_Branch = "xD_X2";
TString yD_U_Branch = "yD_U";
TString yD_U_Corr_Branch = "yD_U_corr";

// =========================
// GEOMETRY (IMPORTANT: RESTORED)
// =========================
const double wirePitch_X = 3.0;
const double wirePitch_U = 3.35220115432; //3.0/std::cos(26.5*M_PI/180.);

double xOffset[3] = {0.0, /*-wirePitch_U/2. 0.0*/ 50.0, -88.0};

// =========================
// HISTOGRAMS
// =========================
TH2D* hLayer[3] = {nullptr,nullptr,nullptr};

// =========================
// NAVIGATION
// =========================
Long64_t currentEvent = 0;
TGNumberEntry* eventBox = nullptr;

// =========================
// DRAW EVENT
// =========================
void DrawEvent(Long64_t entry)
{
	tree->GetEntry(entry);
	currentEvent = entry;

	std::cout << "\nEVENT " << entry << "\n";

	double phiRad = phi * TMath::Pi() / 180.0;
	double tanPhi = tan(phiRad);
	if(fabs(tanPhi) < 1e-8) { tanPhi = 1e-8; }

	// =========================
	// INIT HISTOS
	// =========================
	for (int l = 0; l < 3; l++)
	{
		if (!hLayer[l])
		{
			hLayer[l] = new TH2D(
					Form("hL_%d", l),
					Form("Layer %d;X [mm];Z [mm]", l),
					nBinsX,minX,maxX,//1000, 0, 1000,
					nBinsZ,minZ,maxZ //120, -12, 12
					);
	    
			hLayer[l]->GetXaxis()->CenterTitle();
	    
			hLayer[l]->GetYaxis()->CenterTitle();
            
			hLayer[l]->SetStats(0);
        
		}
        
		hLayer[l]->Reset();
    
	}

	// =========================
	// CLUSTERS
	// =========================
	for (size_t i = 0; i < res_layerID->size(); i++)
	{
		int l = res_layerID->at(i);
		if (l < 0 || l > 2) { continue; }

		double wire = res_wireNumber->at(i);
		double z = z_measured->at(i);

		double x = 0;
		if(l == 1)
		{
			double u = wirePitch_X * wire;
			double yD = -yD_U;
			//x = yD * std::tan(gamma_angle) + u / std::cos(gamma_angle);
			//x = yD + u;
			x = wirePitch_U * wire + yD_U_corr*tan(gamma_angle) + xOffset[l];
		}
		else
		{
			x = wirePitch_X * wire + xOffset[l];
		}

		hLayer[l]->Fill(x, z);
	}

	// =========================
	// DRAW
	// =========================
	cEvent->Clear();
	cEvent->Divide(1,3);

	for (int l = 0; l < 3; l++)
	{
		cEvent->cd(l+1);
		hLayer[l]->Draw("COL");
	}

	for (int l = 0; l < 3; l++)
	{
		cEvent->cd(l+1);

		double xMin = hLayer[l]->GetXaxis()->GetXmin();
		double xMax = hLayer[l]->GetXaxis()->GetXmax();

		TLine* zeroLine = new TLine(xMin, 0.0, xMax, 0.0);
		zeroLine->SetLineColor(kBlue);
		zeroLine->SetLineStyle(2);
		zeroLine->SetLineWidth(2);
		zeroLine->Draw("SAME");
	}

	// =========================
	// CLUSTER BARS (FULL BLACK)
	// =========================
	for (int l = 0; l < 3; l++)
	{
		cEvent->cd(l+1);

		for (size_t i = 0; i < res_layerID->size(); i++)
		{
			if (res_layerID->at(i) != l) { continue; }

			double wire = res_wireNumber->at(i);
			double z = z_measured->at(i);

			double x = 0.;
			double dx = 0.;
			/*
			if(l == 1)
			{
				x = wirePitch_U * wire + xOffset[l];
				dx = wirePitch_U * 0.9;
			}
			*/
			if(l == 1)
			{
				double u = wirePitch_X * wire;
				double yD = -yD_U;
				x = yD * std::sin(gamma_angle) + u; //yD * std::tan(gamma_angle) + u / std::cos(gamma_angle);
				//x = wirePitch_U * wire;
				//x = yD + u;
				//std::cout << "u=" << u << " y=" << yD << std::endl;
				dx = wirePitch_U * 1.; //0.9;
				//x = wirePitch_U * wire + xOffset[l];
				x = wirePitch_U * wire - yD_U_corr*tan(gamma_angle) + xOffset[l];
			}
			else
			{
				x = wirePitch_X * wire + xOffset[l];
				dx = wirePitch_X * 1.; //0.9;
			}

//			double x = wirePitch * wire + xOffset[l];

			//double dx = wirePitch * 0.9;

			double z0 = -fabs(z);
			double z1 =  fabs(z);

			TBox* b = new TBox(x - dx/2, z0, x + dx/2, z1);

			b->SetFillColor(kBlack);
			b->SetFillStyle(1001);
			b->SetLineColor(kBlack);

			b->Draw("SAME");
		}
	}

	// =========================
	// TRACK (UNCHANGED LOGIC)
	// =========================
	for(int l = 0; l < 3; l++)
	{
		cEvent->cd(l+1);

		double xD = (l == 0) ? xD_X1 : ( (l == 1) ? xD_U : xD_X2);

		double zmin = -12;
		double zmax =  12;

		double x1 = xD + zmin / tanPhi;
		double x2 = xD + zmax / tanPhi;

		TPolyLine* line = new TPolyLine(2);
		line->SetPoint(0, x1, zmin);
		line->SetPoint(1, x2, zmax);

		line->SetLineColor(kRed);
		line->SetLineWidth(2);
		line->Draw("L SAME");
	}

	cEvent->Modified();
	cEvent->Update();

	if(eventBox) { eventBox->SetNumber(entry); }
}

// =========================
// NAVIGATION
// =========================
void SetEvent(Long64_t e)
{
	if(!tree) { return; }

	if(e < 0) { e = 0; }
	if(e >= tree->GetEntries()) { e = tree->GetEntries() - 1; }

	DrawEvent(e);
}

void NextEvent() { SetEvent(currentEvent + 1); }
void PrevEvent() { SetEvent(currentEvent - 1); }

void GoToEvent()
{
	SetEvent((Long64_t)eventBox->GetNumber());
}

// =========================
// GUI
// =========================
void BuildGUI()
{
	TGMainFrame *f = new TGMainFrame(gClient->GetRoot(), 200, 60);

	TGHorizontalFrame *h = new TGHorizontalFrame(f);

	TGTextButton *b1 = new TGTextButton(h, "<- Prev");
	//b1->Connect("Clicked()", "TObject", nullptr, "PrevEvent()");
	b1->Connect("Clicked()",0,0, "PrevEvent()");
	h->AddFrame(b1);

	eventBox = new TGNumberEntry(h, 0, 6, -1, TGNumberFormat::kNESInteger);
	h->AddFrame(eventBox);

	TGTextButton *b2 = new TGTextButton(h, "Go");
	b2->Connect("Clicked()",0,0, "GoToEvent()");
	h->AddFrame(b2);

	TGTextButton *b3 = new TGTextButton(h, "Next ->");
	b3->Connect("Clicked()",0,0, "NextEvent()");
	h->AddFrame(b3);

	f->AddFrame(h);
	f->SetWindowName("Event Navigation");
	f->MapSubwindows();
	f->Resize();
	f->MapWindow();
}

// =========================
// INIT
// =========================
void Init()
{
	tree->SetBranchAddress(/*"res_layerID"*/layerID_Branch, &res_layerID);
	tree->SetBranchAddress(/*"res_wireNumber"*/wireNumber_Branch, &res_wireNumber);
	tree->SetBranchAddress(/*"z_measured"*/z_measured_Branch, &z_measured);

	tree->SetBranchAddress(/*"phi"*/ phi_Branch, &phi);
	tree->SetBranchAddress(/*"xD_X1"*/ xD_X1_Branch, &xD_X1);
	tree->SetBranchAddress(/*"xD_U"*/ xD_U_Branch, &xD_U);
	tree->SetBranchAddress(/*"xD_X2"*/ xD_X2_Branch, &xD_X2);
	tree->SetBranchAddress(/*"xD_U"*/ yD_U_Branch, &yD_U);
	tree->SetBranchAddress(/*"xD_U"*/ yD_U_Corr_Branch, &yD_U_corr);
}

// =========================
// MAIN
// =========================
void TrackEventViewer(const char* file="TrackStore.root")
{
	TFile* f = TFile::Open(file);
	if(!f || f->IsZombie())
	{
		std::cout << "File error\n";
		return;
	}

	f->GetObject(tree_name/*"Track"*/, tree);
	if(!tree)
	{
		std::cout << "Tree not found\n";
		return;
	}

	Init();

	cEvent = new TCanvas("cEvent", "Track Event Viewer", 1100, 900);

	DrawEvent(0);
	BuildGUI();

	std::cout << "\nREADY\n";
}
