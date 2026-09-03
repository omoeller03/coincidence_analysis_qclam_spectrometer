#include "TH2D.h"
#include "TF1.h"

TH2D* apply_timewalkcorr(TH2D* h2, TF1* f, double tref=30.0)
{
	if(!h2 || !f)
	{
		std::cout << "Histogramm oder Fitfunktion fehlt" << std::endl;
		return nullptr;
	}

	TH2D* hcorr = (TH2D*)h2->Clone("h2_corr");
	hcorr->Reset();

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


			double t_corr = t - ( f->Eval(tot) - f->Eval(tref));

			hcorr->Fill(tot,t_corr,content);
		}
	}

	return hcorr;
}
