// TQClamWirePlane.h
#ifndef TQCLAMWIREPLANE_h
#define TQCLAMWIREPLANE_h

//#include <configs/VerboseLevel.hh>

#include <vector>
#include <map>
#include "TVector3.h"

class TQClamWire;
enum class WirePlaneType;


class TQClamWirePlane
{
	public:
		// Kontruktor
		TQClamWirePlane(WirePlaneType type, // U-/X-Ebene (enum class EbeneTyp ist in Draht.hh definiert)
				double zPosition, // z-Koordinate der Ebene
				double x_offset, // unterster x-Wert der Drahtakzeptanz dieser Ebene
				double y_offset, // linke Seite der Drahtakzeptanz
				double x_length, // Laenge der Drahtakzeptanz in x-Rtg
				double y_length, // Laenge der Drahtakzeptanz in y-Rtg
				double wire_ist, // Abstand benachbarter Draehte in x-Rtg
				double wire_angle, // Winkel zwischen Draht und der y-Achse/rad
				int n_wires, // Anzahl Draehte in der Ebene
				int id);//, // Drahtebenen ID
				//const VerboseLevel& verbose);

		// Destruktor
		~TQClamWirePlane() = default;

	
		/* Methoden */
		// Getter
		WirePlaneType GetType() const;
		double GetZPosition() const;
		double GetXOffset() const;
		double GetYOffset() const;
		double GetXLength() const;
		double GetYLength() const;
		double GetNormalWireDist() const; // DrahtDist / normaler Abstand
		double GetWireDist() const;
		double GetWireAngleRad() const;
		double GetWireAngleDeg() const;
		int GetID() const; // Ebenen-ID ausgeben
		int GetNumberOfWires() const;
		const std::vector<TQClamWire>& GetWires() const; // Draehte ausgeben
		const TQClamWire* GetWireByWireNumber(int wireNumber) const;

		// Geometrie
		TVector3 GetWirePoint(const TQClamWire* wire) const;
		//TVector3 GetWirePoint(const TQClamWire& wire) const;

		// Bauen
		void BuildWires();

		// Debug
		void Print() const;

	private:
		WirePlaneType layerType; /** X-/U-Ebene */
//		VerboseLevel ebene_verbose;

		double layerZPosition; /** z-Koordinate der Ebene */

		double layerXOffset; /** x-Wert Beginn der Drahtakzeptanz (unten) */
		double layerYOffset; /** y-Wert Beginn Drahtakzeptanz (links) */

		double layerXLength; /** Laenge der Drahtakzeptanz in x-Richtung */
		double layerYLength; /** Laenge der Drahtakzeptanz in y-Richtung */

		double layerWireDist; /** Abstand benachbarter Draehte in x-Rtg */
		double layerWireAngle; /** Winkel/rad zwischen Draht und der y-Achse in der x-y-Ebene */

		int layerNWires; /** Anzahl Draehte */
		int layerID; /** Drahtebenen ID */
		
		std::vector<TQClamWire> layerWires; /** Draehte */
		/*
		std::map<int, Draht*> ebene_drahtMapByWireNumber; // Draehte (Drahtnummmer) --> Draht
		std::map<int, Draht*> ebene_drahtMapByID; // Draehte (drahtID) --> Draht
		*/

		std::map<int, std::size_t> mapWireNumberToWireListIndex; // mapping: Drahtnummmer --> Index in 'layerWires'
		std::map<int, std::size_t> mapWireIDToWireListIndex; // mapping: DrahtID --> Index in 'layerWires'


/*	ClassDef(TQClamWirePlane,1); geometrie linken, ggf auskommentieren */
};


#endif
