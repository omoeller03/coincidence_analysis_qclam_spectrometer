#ifndef SUBTRACT_MULTILABR_STRING_C
#define SUBTRACT_MULTILABR_STRING_C

#include "Riostream.h"
#include <vector>
#include <sstream>

Bool_t SubtractMultiplePromptBkgrdMatrices(const char* inputList, Bool_t draw = kTRUE)
{
	if(!inputList)
	{
		std::cout << "Empty input list" << std::endl;
		return kFALSE;
	}

	if(!TGo4AbstractInterface::Instance() || go4 != TGo4AbstractInterface::Instance())
	{
		std::cout << "FATAL: not in Go4 GUI" << std::endl;
		return kFALSE;
	}

	// ------------------------------------------------------------
	// Parse string -> vector<int>
	// accepts "0,5,6" or "0 5 6"
	// ------------------------------------------------------------

	std::vector<Int_t> labrs;

	TString s(inputList);
	s.ReplaceAll(",", " ");

	std::stringstream ss(s.Data());
	Int_t val;

	while(ss >> val) { labrs.push_back(val); }

	if(labrs.empty())
	{
		std::cout << "No valid indices in input" << std::endl;
		return kFALSE;
	}

	// ------------------------------------------------------------
	// Accumulate
	// ------------------------------------------------------------

	TH2* sum = nullptr;
	Double_t normTotal = 0;

	for(size_t i=0;i<labrs.size();i++)
	{
		Int_t labr = labrs[i];

		TString hP = Form("EEPGMatrixPrompt_LaBr%d", labr);
		TString hB = Form("EEPGMatrixBkgrd_LaBr%d", labr);

		TH2* hp = dynamic_cast<TH2*>(go4->GetObject(go4->FindItem(hP),1000));

		TH2* hb = dynamic_cast<TH2*>(go4->GetObject(go4->FindItem(hB),1000));

		if(!hp || !hb)
		{
			std::cout << "Missing histogram LaBr " << labr << std::endl;
			continue;
		}

		TString condName = Form("CondArrCoincPrompt%06d", labr);

		TGo4WinCond* win = dynamic_cast<TGo4WinCond*>(go4->GetObject(go4->FindItem(condName),1000));

		if(!win)
		{
			std::cout << "Missing condition LaBr " << labr << std::endl;
			continue;
		}

		Double_t nTrue  = win->TrueCounts();
		Double_t nTotal = win->Counts();
		Double_t nBk    = nTotal - nTrue;

		if(nTrue <= 0 || nBk <= 0)
		{
			std::cout << "Bad stats LaBr " << labr << std::endl;
			continue;
		}

		Double_t alpha = nTrue / nBk;

		std::cout << "LaBr " << labr
			<< " True=" << nTrue
			<< " alpha=" << alpha
			<< std::endl;

		if(!sum)
		{
			sum = (TH2*)hp->Clone("sumPromptBkgrd");
			sum->Reset();
			sum->SetDirectory(nullptr);
		}

		sum->Add(hp);
		sum->Add(hb, -alpha);

		normTotal += nTrue;
	}

	if(!sum || normTotal <= 0)
	{
		std::cout << "No valid data accumulated" << std::endl;
		return kFALSE;
   
	}

	// ------------------------------------------------------------
	// Normalize
	// ------------------------------------------------------------

	sum->Scale(1.0 / normTotal);

	// ------------------------------------------------------------
	// Save to Go4
	// ------------------------------------------------------------

	TString rname = go4->SaveToMemory("Sums", sum, kTRUE);

	std::cout << "Saved: " << rname.Data() << std::endl;

	if(draw)
	{
		ViewPanelHandle vpanel = go4->StartViewPanel();
		go4->DrawItem(rname, vpanel);
	}

	return kTRUE;
}

#endif
