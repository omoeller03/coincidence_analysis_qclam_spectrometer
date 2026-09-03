#include "TGo4AbstractInterface.h"
#include "TH1.h"
#include "TString.h"
#include "TNamed.h"
#include "TTimeStamp.h"
#include <iostream>

// ------------------------------------------------------------
// SNAPSHOT + CLEAR MACRO
// ------------------------------------------------------------
Bool_t CopyAndClearTimePeakHist()
{
	const char* histname = "Time_LaBr0_vs_MeanSzint";

	auto go4 = TGo4AbstractInterface::Instance();
	if(!go4)
	{
		std::cout << "ERROR: no Go4 instance\n";
		return kFALSE;
	}

	// ========================================================
	// 1) FIND HISTOGRAM IN ANALYSIS
	// ========================================================
	TString full = go4->FindItem(histname);

	if(full.IsNull())
	{
		std::cout << "ERROR: histogram not found: " << histname << std::endl;
		return kFALSE;
	}

	TObject* obj = go4->GetObject(full, 1000);
	TH1* h = dynamic_cast<TH1*>(obj);

	if(!h)
	{
		std::cout << "ERROR: not a TH1: " << histname << std::endl;
		return kFALSE;
	}

	// ========================================================
	// 2) CREATE TIMESTAMPED COPY
	// ========================================================
	TTimeStamp ts;
	TString stamp = ts.AsString("lc"); // local compact string

	stamp.ReplaceAll(" ", "_");
	stamp.ReplaceAll(":", "-");

	TString newname = Form("%s_%s", histname, stamp.Data());

	TH1* hcopy = (TH1*)h->Clone(newname);
	hcopy->SetDirectory(nullptr);

	// ========================================================
	// 3) SAVE COPY INTO WORKSPACE
	// ========================================================
	TString saved = go4->SaveToMemory("Workspace", hcopy, kTRUE);

	std::cout << "Saved snapshot: " << saved << std::endl;

	// ========================================================
	// 4) CLEAR ORIGINAL (SAFE RESET!)
	// ========================================================
	if(h)
	{
		std::cout << "Resetting original histogram in Analysis: " << histname << std::endl;
		h->Reset();   // <-- IMPORTANT: Go4-safe clear
	}

	go4->SendCommand("ResetObject", histname);

	// ========================================================
	// 5) OPTIONAL INFO OBJECT (DEBUG / LOG)
	// ========================================================
	TNamed* info = new TNamed(Form("%s_snapshot_info", histname), Form("Snapshot created: %s", stamp.Data()));

	go4->SaveToMemory("Workspace", info, kTRUE);

	std::cout << "Done.\n";

	return kTRUE;
}
