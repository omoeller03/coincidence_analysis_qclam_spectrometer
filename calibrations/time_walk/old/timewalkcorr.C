#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TF1.h"






TF1* fittimewalk(TH2D* h2 = nullptr)
{
	if(!h2)
	{
		std::cout << "TH2D als Argument uebergeben!" << std::endl;
		return nullptr;
	}

	TAxis* xax = h2->GetXaxis();
	int nx = xax->GetNbins();

	std::vector<double> tot_vals;
	std::vector<double> peak_vals;

	for(int ix=1; ix<=nx; ix++)
	{
		TH1D* py = h2->ProjectionY(Form("py_%d", ix), ix,ix);

		if(py->GetEntries()<10) { continue; } /** zu wenig statistik */

		/** max finden */
		int maxbin = py->GetMaximumBin();
		double peak = py->GetXaxis()->GetBinCenter(maxbin);

		double tot = xax->GetBinCenter(ix);

		tot_vals.push_back(tot);
		peak_vals.push_back(peak);
	}

	TGraph* gr = new TGraph(tot_vals.size(), &tot_vals[0], &peak_vals[0]);
	gr->SetTitle("peak drifttime vs ToT;ToT/ns;Peak drifttime/ns");
	gr->SetMarkerStyle(20);
	gr->Draw("AP");

	/** time-walk-fkt fitten */
	TF1* f = new TF1("f", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", 20,55);
	gr->Fit(f,"R");
	std::cout << "Fit-Parameter" << std::endl;
	f->Print();


	return f;
}



TH2D* apply_timewalkfit(TH2D* h2, TF1* f, double tref=30.0)
{
        if(!h2 || !f)
        {
                std::cout << "Histogramm oder Fitfunktion fehlt" << std::endl;
                return nullptr;
        }

        TH2D* hcorr = (TH2D*)h2->Clone("h2_corr");
        hcorr->Reset();
	hcorr->GetXaxis()->SetTitle("Time-over-Threshold/ns");
	hcorr->GetXaxis()->CenterTitle();
	hcorr->GetYaxis()->SetTitle("Time-Walk korrigierte Driftzeit/ns");
	hcorr->GetYaxis()->CenterTitle();

        TAxis* xax = h2->GetXaxis();
        TAxis* yax = h2->GetYaxis();

        for(int ix=1; ix<=xax->GetNbins(); ix++)
        {
                double tot = xax->GetBinCenter(ix);

                for(int iy=1; iy<=yax->GetNbins(); iy++)
                {
                        int content = h2->GetBinContent(ix,iy);
                        if(content == 0) { continue; }

                        double t = yax->GetBinCenter(iy);


                        double t_corr = t - ( f->Eval(tot) - 0);

                        hcorr->Fill(tot,t_corr,content);
                }
        }

	TCanvas* c2 = new TCanvas("c2", "c2", 700,500);
	hcorr->Draw("COLZ");

        return hcorr;
}







TH2D* timewalkcorr(TH2D* h2 = nullptr, double tref = 30.0)
{
	if(!h2)
	{
		std::cout << "TH2D als Argument uebergeben!" << std::endl;
		return nullptr;
	}

	TF1* fcorr = fittimewalk(h2);
	TH2D* hcorr = apply_timewalkfit(h2, fcorr, tref);

	return hcorr;
}

