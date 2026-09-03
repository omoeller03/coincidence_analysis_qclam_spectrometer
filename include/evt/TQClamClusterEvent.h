#ifndef TQCLAMCLUSTEREVENT_H
#define TQCLAMCLUSTEREVENT_H

#include "TGo4EventElement.h"

#include "structs/TQClamMbsHeader.h"
#include "structs/TQClamScaler.h"
#include "structs/TQClamTriggerDetectors.h"
#include "TQClamConstants.h"

#include <bitset>
#include <vector>

struct TQClamHit;
class TQClamWire;
enum class WirePlaneType;






struct TQClamCluster
{
	bool accepted = false; /** Cluster alle Conditions passiert */
	std::vector<const TQClamHit*> originalHits; /** alle Hits, vor aussortieren */

	bool cleaned = false; /** wurde der Cluster durch CleanCluster veraendert? */
	size_t numRemovedHits = 0; /** anzahl entfernter Hits eines Clusters innerhalb CleanCluster */
	std::vector<const TQClamHit*> removedHits; /** in CleanCluster entfernte Hits */

	std::vector<const TQClamHit*> hits; /** liste aller hits in cluster */
	std::vector<int> wireNumbers; /** liste aller drahtnummern von draehten, die gefeuert haben */
	std::vector<double> drifttimes; /** liste aller driftzeiten */
	std::vector<double> tot; /** liste aller tots */
	std::vector<double> normalCoords; /** liste der normalen Koordinaten der Draehte: x/u */

	int layerID = -1; /** ebenen ID */
	int minWireNumber = -1; /** kleinste drahtnummer */
	int maxWireNumber= -1; /** groesste drahtnummer */

	size_t wireSpan = 0; /** drahtspanne = | maxDrahtnummer - minDrahtnummer | + 1 */
	size_t numHits = 0; /** anzahl hits in cluster */

	bool hasMultiWireHits = false; /** haben draehte in cluster mehrfach gefeuert? */
	int numOfMultiWireHits = 0; /** anzahl multi-wire-hits */

	bool hasGaps = false; /** haben Draehte zw minDraht und maxDraht in Cluster nicht gefeuert? */
	bool hasVShape = false; /** bilden die Driftzeiten im Cluster aufgetragen gegen Drahtnummer V-Form? */
	bool ascending = false; /** steigen die Driftzeiten aufgetragen gegen Drahtnummer? */
	bool descending = false; /** fallen die Driftzeiten aufgetragen gegen Drathnummer? */

	double minDrifttime = -1; /** minimale Driftzeit */
	double meanDrifttime = -1; /** mittlere Driftzeit */
	double meanDriftlength = -1; /** mittlerer Driftweg --> berechnet mit globaler driftgeschw (fuer grobe Schaetzungen */

	double xMid = -1; /** mittlere x-Pos in Cluster: grob fuer Schaeutzung */
//	double yMid = -1; /** mittlere y-Pos in Cluster (nur fuer U-Ebene): grob fuer Schaetzung */
	double zPos = -1; /** z-Pos in Cluster: grob fuer Schaetzung */

	double meanNormalCoordinate = -1; /** mittlere Normal-Koordinate in Cluster  */

	int wireNumberOfHitWithMinDrifttime; /** drahtnummer von hit mit min driftzeit */
	const TQClamHit* hitWithMinDrifttime = nullptr;
};






class TQClamClusterEvent : public TGo4EventElement
{
	public:
		TQClamClusterEvent() {}
		TQClamClusterEvent(const char* name) 
			: TGo4EventElement(name)
		{}

		virtual ~TQClamClusterEvent() {}


		void Clear(Option_t* opt = "") override;


		/** parameter */
		MbsHeader header; //! nicht streamen
		TQClamScaler scaler; //! nicht streamen
		TQClamTriggerDetectors trigDet; //! nicht streamen

		int layerID = -1;
		int fEvtCounter{0};
		WirePlaneType planeType; //!
		std::bitset<QCLAM_MAX_LAYER> layerHasCluster; /** layerID hat Cluster -> 0/1 */


		std::vector<const TQClamHit*> hits; /** uebergebene hits */
		std::vector<TQClamCluster> clusters; /** gebaute cluster-strukturen */


	ClassDefOverride(TQClamClusterEvent,1);
};
		

#endif
