// Draht.h
#ifndef TQCLAMWIRE_h
#define TQCLAMWIRE_h

#include "TVector3.h"

// definiere Typ der Ebene, in welcher der Draht liegt
enum class WirePlaneType { NONE, X, U };


class TQClamWire
{
	public:
		// Konstruktor
		TQClamWire(WirePlaneType type, // Draht in U-/X-Ebene
				double z_coordinate,
				double normal_coordinate, // u-/x-Koordinate (Normale zum Draht)
				int globalID, // Draht-ID (global in Driftkammer, ueber alle Ebenen)
				int wireNumber, // Draht-Nummer (lokal in Drahtebene => ID in Ebene)
				int layerID, // globale Ebenen-ID
				double angle, // anstellwinkel draht zur y-achse (rad)
				double deltaN);

		// Destruktor
		~TQClamWire() = default;

		
		/* Methoden */
		// Getter
		// x = normalcoordinate * cos(angle)
		// y = normalcoordinate * sin(angle)
		WirePlaneType GetType() const;
		double GetZCoordinate() const;
		double GetNormalCoordinate() const;
		double GetAngle() const;
		double GetWireDist() const;
		int GetGlobalID() const;
		int GetWireNumber() const;
		int GetLayerID() const;

		// Geometrie
		TVector3 GetDirection() const;

		// Debug
		void Print() const;

	// Variablen
	private:
		WirePlaneType wireLayerType; // U-/X-Ebene

		double zCoordinate;
		double normalCoordinate; // u-/x-Koordinate Draht
		double wireAngle; // winkel des drahts zur y-achse (in rad)
		double wireDist; // normalerAbstand zu benachbarten Draht

		int globalID; // Draht-ID (global)
		int wireNumber; // Draht-Nummer (lokal in Drahtebene)
		int layerID; // Ebenen-ID


/*	ClassDef(TQClamWire,1); geometrie linken ggf auskommentieren */
};



#endif
