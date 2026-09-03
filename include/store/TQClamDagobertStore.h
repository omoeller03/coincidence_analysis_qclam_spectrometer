#ifndef TQCLAMDAGOBERTSTORE_H
#define TQCLAMDAGOBERTSTORE_H

#include "TGo4EventStore.h"

class TTree;
class TFile;
class TGo4UserStoreParameter;

/// evts/... speichern?
// class TQClamHitEvent;
// ...

class TQClamDagobertStore : public TGo4EventStore
{
	public:
		TQClamDagobertStore();
		TQClamDagobertStore(TGo4UserStoreParameter* par);
		virtual ~TQClamStore();

		//Int_t Store(TGo4EventElement* event) override;

	private:
		TFile* fxFile{nullptr}; //!
		TTree* fxTree{nullptr}; //!
		// TQClamHitEvent* fxHitEvent{nullptr}; //!

		TQClamTrackEvent* fQClamTrackEvent{nullptr};
		TDagobertHitEvent* fDagoHitEvent{nullptr};
		Bool_t fbBranchExists{kFALSE};
	
	
	ClassDefOverride(TQClamStore,1);
};

#endif
