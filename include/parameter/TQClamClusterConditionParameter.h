#ifndef TQCLAMCLUSTERCONDITIONPARAMETER_H
#define TQCLAMCLUSTERCONDITIONPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"
#include "TQClamConstants.h"

class TQClamClusterConditionParameter : public TGo4Parameter
{
	public:
		TQClamClusterConditionParameter();
		TQClamClusterConditionParameter(const char* name);

		/** Max Dimensionen */
		static const Int_t MAX_LAYER = QCLAM_MAX_LAYER; // Maximale Anzahl Ebenen: 8

		static constexpr Int_t kOff = QCLAM_COND_OFF; // Condition-Mode 0: Condition hat keine Auswirkung
		static constexpr Int_t kFlagOnly = QCLAM_COND_FLAG_ONLY; // Condition-Mode 1: Condition setzt Event-Flag accepted true/false (Default)
		static constexpr Int_t kFilter = QCLAM_COND_FILTER; // Condition-Mode 2: Condition verwirft Event
	
		Int_t minWireSpanInCluster[MAX_LAYER]; // Minimale Drahtspanne: | max_Drahtnummer - min Drahtnummer |
		Int_t maxWireSpanInCluster[MAX_LAYER]; // Maximale Drahtspanne: | max_Drahtnummer - min_Drahtnummer |
		Int_t WireSpanCondMode = kFlagOnly; // Condition-Mode Drahtspanne-Fenster: (minWireSpan, maxWireSpan)

		Int_t minHitNumberInCluster[MAX_LAYER]; // Minimale Anzahl Hits in Cluster
		Int_t maxHitNumberInCluster[MAX_LAYER]; // Maximale Anzahl Hits in Cluster
		Int_t HitNumberCondMode = kFlagOnly; // Condition-Mode Hit-Anzahl-Fenster: (minHitNumber, maxHitNumber)

		Bool_t requireVShape[MAX_LAYER]; // Nur Cluster mit V-Form (Driftzeit vs. Drahtnummer) verwenden
		Bool_t requireAscending[MAX_LAYER]; // Nur Cluster mit ansteigender Driftzeit (gegen Drahtnummer aufgetragen) verwenden
		Bool_t requireDescending[MAX_LAYER]; // Nur Cluster mit abfallender Driftzeit (gegen Drahtnummer aufgetragen) verwenden
		Bool_t requireClusterWithoutGaps[MAX_LAYER]; // Nur Cluster ohne Drathloecher verwenden

		/** Parameter fuer Methode CleanCluster
		 * -> fCleanCluster -> fuehre CleanCluster
		 * -> fMaxAllowedGapInCleaning -> wenn gap so und so gross -> kann nicht als ein cluster gezaehlt werden */
		Bool_t fCleanCluster = kFALSE; // Versuche Hit-Ausreisser zu entfernen.
		Int_t fMaxAllowedGapInCleaning[MAX_LAYER]; // CleanCluster: Gap-Groesse in Hits einer Ebene, um dies nicht als einen Cluster zaehlen zu koennen --> entscheide, welche Hits Cluster sind und sortiere weit entfernte Hits aus
		Double_t fMaxDrifttimeJump[MAX_LAYER]; // CleanCluster: Driftzeit-Differenz zwischen Hits benachbarter Draehte duerfen max. um fMaxDrifttimeJump abweichen --> ansonsten rauswerfen
		Int_t fMaxRemovedHits[MAX_LAYER]; // CleanCluster: nicht zu aggressiv -> entferne maximal MaxRemovedHits je Cluster
		Int_t fMaxCleaningIterations[MAX_LAYER]; // CleanCluster: Iterativ Cluster-Parameter berechnen und dann Cluster saeubern --> Max. Anzahl Iteration (andernfalls Abbruch, wenn keine Hits rausgeworfen wurden)



		/** bei GUI-Update */
		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	ClassDefOverride(TQClamClusterConditionParameter, 1);
};

#endif

