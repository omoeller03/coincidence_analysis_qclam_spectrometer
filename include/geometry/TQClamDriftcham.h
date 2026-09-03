// TQClamDriftchamber.h (container fuer die ebenen, draehte werden von ebenen erzeugt)
#ifndef TQCLAMDRIFTCHAM_h
#define TQCLAMDRIFTCHAM_h

/** #include "Rtypes.h" geometrie linken, ggf entkommentieren */
#include <vector>
#include <string>

#include "geometry/TQClamWirePlane.h"
#include "geometry/TQClamWire.h"

class TQClamDriftcham
{
	public:
		TQClamDriftcham(int id, // Driftkammer ID
				const std::string& gen); // 3Gen/4Gen, eigentlich unwichtig, naja

		~TQClamDriftcham() = default;

	
		/** Getter */
		int GetID() const; // Kammer ID ausgeben
		const std::vector<TQClamWirePlane>& GetWirePlanes() const; // Drahtebenen ausgeben
		const TQClamWirePlane* GetWirePlane(int layerID) const;
		const TQClamWire* GetWire(int layerID, int wireNumber) const;

		// --- Ebenen hinzufuegen ---
		void AddWirePlane(TQClamWirePlane layer); // Drahtebene hizufuegen

		// --- Debug ---
		void Print() const; // Kammerart ausgeben

	// Variablen
	private:
		// Kammerspezifische ID
		// 3Gen-Kammer -> 3
		// 4Gen-Kammer -> 4
		int driftchamID;
//		const VerboseLevel& kammer_verbose;

		std::string driftchamGen; // 3Gen/4Gen (3/4)

		// Ebenen in Driftkammer
		// 3Gen-Kammer -> X1-U-X2
		// 4Gen-Kammer -> X1-U1-X2-U2
		std::vector<TQClamWirePlane> wirePlanes;

/*	ClassDef(TQClamDriftcham,1); geometrie linken, ggf. entkommentieren */

};

#endif
