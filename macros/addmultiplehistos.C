#ifndef ADDMULTIPLEHISTOS_C
#define ADDMULTIPLEHISTOS_C

//#include "addmultiplehistos.h"

#include "Riostream.h"

///////////////////////////////////////////////////////////////////
//////// Go4 GUI example script addhistos.C
//          J.Adamczewski, gsi, May 2006
//          JAM May 2015: added support for 2d histograms
//
//          Angepasst O.Moeller, 09.06.2026
//
// NOTE: to be run in Go4 GUI local command line only!
//       NEVER call this script in remote analysis process!!!
/////// Functionality:
// adds histogram of name2 to histogram of name1
/////// Usage:
// histogram name2 will be scaled by factor.
// (e.g. if factor==-1, his2 is substracted from his1)
// The draw flag switches if the results are displayed each time this makro is called
// if display is switched off, the result histogram is just updated in browser and existing displays
///////
Bool_t addmultiplehistos(Int_t n, const char* names[], Double_t factor, Bool_t draw, const char* resultname="sum", const char* resulttitle="sum")
{
	// brauche min 2 x files zum addieren
	if(n < 2)
	{
		std::cout << "Need at least 2 histograms. END NOW." << std::endl;
		return kFALSE;
	}

	// erreiche ich analyse?
	if(!TGo4AbstractInterface::Instance() || go4 != TGo4AbstractInterface::Instance())
	{
		std::cout <<"FATAL: Go4 gui macro executed outside Go4 GUI!! returning." << std::endl;
		return kFALSE;
	}
   
	TH1* result = nullptr;
	TString firstFullName;

	for(Int_t i=0; i<n; i++)
	{
		TString fullname = go4->FindItem(names[i]);
		TObject* obj = go4->GetObject(fullname,1000); // 1000 = timeout/ms, um objekt aus analyse zu kriegen

		TH1* hist = nullptr;
		if(obj && obj->InheritsFrom("TH1"))
		{
			hist = (TH1*)obj;
		}

		if(!hist)
		{
			std::cout << "Could not get histogram " << names[i] << std::endl;
			return kFALSE;
		}

		if(i==0)
		{
			firstFullName = fullname;
			
			result = (TH1*)hist->Clone(resultname);
			result->SetName(resultname);
			result->SetTitle(resulttitle);
			result->Reset();
			result->Sumw2();
			result->Add(hist);
		}
		else
		{
			if(result->GetDimension() != hist->GetDimension())
			{
				std::cout << "Dimension mismatch for " << names[i] << std::endl;

				delete result;
				return kFALSE;
			}

			result->Add(hist,factor);
		}
	} // end for

	// Summenhistogramm wird ins Workspace geschrieben
	result->SetDirectory(nullptr);

	TString rname = go4->SaveToMemory("Sums",result,kTRUE);

	std::cout << "Saved result histogram to " << rname.Data() << std::endl;

	if(draw)
	{
		ViewPanelHandle vpanel = go4->StartViewPanel();

		go4->DrawItem(rname,vpanel);
	}

	return kTRUE;
}

#endif
