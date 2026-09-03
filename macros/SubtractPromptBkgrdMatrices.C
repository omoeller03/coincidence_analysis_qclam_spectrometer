#ifndef SUBTRACTPROMPTBKGRD_MATRICES_C
#define SUBTRACTPROMPTBKGRD_MATRICES_C

#include "Riostream.h"

Bool_t SubtractPromptBkgrdMatrices(Int_t labr,
                                   Bool_t draw = kTRUE)
{
	if(labr < 0 || labr > 8)
	{
		std::cout << "LaBr index must be 0..8" << std::endl;
		return kFALSE;
	}


	if(!TGo4AbstractInterface::Instance() || go4 != TGo4AbstractInterface::Instance())
	{
		std::cout << "FATAL: not running inside Go4 GUI" << std::endl;
		return kFALSE;
	}

	// ------------------------------------------------------------
	// Histogram names
	// ------------------------------------------------------------

	TString hPromptName = Form("EEPGMatrixPrompt_LaBr%d", labr);
	TString hBkgrdName  = Form("EEPGMatrixBkgrd_LaBr%d", labr);

	TString fullPrompt = go4->FindItem(hPromptName);
	TString fullBkgrd  = go4->FindItem(hBkgrdName);

	TH2* hPrompt = dynamic_cast<TH2*>(go4->GetObject(fullPrompt,1000));
	TH2* hBkgrd  = dynamic_cast<TH2*>(go4->GetObject(fullBkgrd,1000));

	if(!hPrompt || !hBkgrd)
	{
		std::cout << "Could not load histograms for LaBr " << labr << std::endl;
		return kFALSE;
	}

	// ------------------------------------------------------------
	// Condition (single element of array)
	// ------------------------------------------------------------

	TString condName = Form("CondArrCoincPrompt%06d", labr);

	TString fullCond = go4->FindItem(condName);

	std::cout << "Looking for condition: "
		<< condName.Data()
		<< std::endl;

	if(fullCond.IsNull())
	{
		std::cout << "Condition not found: "
			<< condName.Data()
			<< std::endl;
		return kFALSE;
	}

	TGo4WinCond* win = dynamic_cast<TGo4WinCond*>(go4->GetObject(fullCond,1000));

	if(!win)
	{
		std::cout << "Could not cast condition to TGo4WinCond: "
			<< condName.Data()
			<< std::endl;
		return kFALSE;
	}

	// ------------------------------------------------------------
	// Condition statistics, normalization
	// ------------------------------------------------------------

	Double_t nTrue  = win->TrueCounts();
	Double_t nTotal = win->Counts();
	Double_t nBkgrd = nTotal - nTrue;

	if(nTrue <= 0 || nBkgrd <= 0)
	{
		std::cout << "Invalid condition statistics!"
			<< " True=" << nTrue
			<< " Total=" << nTotal
			<< std::endl;
		return kFALSE;
	}

	Double_t alpha = nTrue / nBkgrd;

	std::cout << "LaBr " << labr
		<< " True=" << nTrue
		<< " Total=" << nTotal
		<< " alpha=" << alpha
		<< std::endl;

	// ------------------------------------------------------------
	// Build result histogram
	// ------------------------------------------------------------

	TString resultName  = Form("EEPGMatrixSub_LaBr%d", labr);
	TString resultTitle = Form("Prompt - scaled Background LaBr%d", labr);

	TH2* result = (TH2*)hPrompt->Clone(resultName);

	result->SetDirectory(nullptr);
	result->SetName(resultName);
	result->SetTitle(resultTitle);
	result->Sumw2();

	// ------------------------------------------------------------
	// Normalization:
	// H = Prompt - alpha * Background
	// optional normalization by TrueCounts
	// ------------------------------------------------------------

	result->Scale(1.0 / nTrue);
	result->Add(hBkgrd, -alpha / nTrue);

	// ------------------------------------------------------------
	// Store in Go4
	// ------------------------------------------------------------

	TString rname = go4->SaveToMemory("Sums", result, kTRUE);

	std::cout << "Saved result: "
		<< rname.Data()
		<< std::endl;

	if(draw)
	{
		ViewPanelHandle vpanel = go4->StartViewPanel();
		go4->DrawItem(rname, vpanel);
	}

	return kTRUE;
}

#endif
