#ifndef TQCLAMSTORE_H
#define TQCLAMSTORE_H

#include "TGo4EventStore.h"

class TTree;
class TFile;
class TGo4UserStoreParameter;

/// evts/... speichern?
// class TQClamHitEvent;
// ...

class TQClamStore : public TGo4EventStore
{
	public:
		TQClamStore();
		TQClamStore(TGo4UserStoreParameter* par);
		virtual ~TQClamStore();

		//Int_t Store(TGo4EventElement* event) override;

	private:
		TFile* fxFile{nullptr};
		TTree* fxTree{nullptr};
		// TQClamHitEvent* fxHitEvent{nullptr};
		Bool_t fbBranchExists{kFALSE};
	
	
	ClassDefOverride(TQClamStore,1);
};

#endif
