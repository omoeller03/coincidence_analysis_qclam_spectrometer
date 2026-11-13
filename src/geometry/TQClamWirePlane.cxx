// TQClamWirePlane
#include "geometry/TQClamWirePlane.h"

#include "geometry/TQClamWire.h"

#include <iostream>


TQClamWirePlane::TQClamWirePlane(WirePlaneType type, double z_position, double x_offset, double y_offset, double x_length, double y_length, double wire_dist, double wire_angle, int n_wires, int id)//, const VerboseLevel& verbose)
	: layerType(type),
	layerZPosition(z_position), // z-Koordinate der Ebene
	layerXOffset(x_offset), // x-Wert Beginn Drahtakzeptanz (unten)
	layerYOffset(y_offset), // y-Wert Beginn Drahtakzeptanz (links)
	layerXLength(x_length), // Laenge Drahtakzeptanz in x-Rtg
	layerYLength(y_length), // Laenge Drahtakzeptanz in y-Rtg
	layerWireDist(wire_dist), // Abstand benachbarter Draehte in x-Rtg (mm)
	layerWireAngle(wire_angle), // Winkel Draehte zur y-Achse (rad)
	layerNWires(n_wires), // Anzahl Draehte
	layerID(id) // Drahtebenen ID
//	ebene_verbose(verbose)
{
//	Print();
}




// --- Getter ---

WirePlaneType TQClamWirePlane::GetType() const
{
	return layerType;
}


double TQClamWirePlane::GetZPosition() const
{
	return layerZPosition;
}


double TQClamWirePlane::GetXOffset() const
{
	return layerXOffset;
}


double TQClamWirePlane::GetYOffset() const
{
	return layerYOffset;
}


double TQClamWirePlane::GetXLength() const
{
	return layerXLength;
}


double TQClamWirePlane::GetYLength() const
{
	return layerYLength;
}


double TQClamWirePlane::GetWireDist() const
{
	return layerWireDist;
}

double TQClamWirePlane::GetNormalWireDist() const
{
	return layerWireDist * std::cos(layerWireAngle);
}

double TQClamWirePlane::GetWireAngleRad() const
{
	return layerWireAngle;
}

double TQClamWirePlane::GetWireAngleDeg() const
{
	return layerWireAngle*180./M_PI;
}



int TQClamWirePlane::GetID() const
{
	return layerID;
}


int TQClamWirePlane::GetNumberOfWires() const
{
	return layerNWires;
}


const std::vector<TQClamWire>& TQClamWirePlane::GetWires() const
{
	return layerWires;
}


const TQClamWire* TQClamWirePlane::GetWireByWireNumber(int wireNumber) const
{
	/*
	if(ebene_verbose.verbose>3)
	{
		//std::cout << "verbose = " << std::to_string(ebene_verbose.verbose) << std::endl;
		std::cout << "[getWireByWireNumber] layerID=" << layerID
			<< " request wireNumber=" << wireNumber
			<< " nDraehte=" << layerNWires << std::endl;
	}
	*/


	auto it = mapWireNumberToWireListIndex.find(wireNumber);

	if(it==mapWireNumberToWireListIndex.end())
	{
		std::cerr << "[TQClamWirePlane] Unguelige wireNumber: " << std::to_string(wireNumber)
			<< " (Ebene-ID: " << std::to_string(layerID)
			<< ", nDrahte: " << std::to_string(layerNWires) << ")\n" << std::endl;
		return nullptr;
	}

	return &layerWires[it->second];
}


// --- Draehte bauen ---
/*
 * platziere Draehte in der Ebene
 *
 * die normale Draht-Koordinate entsprecht der Koordinate relativ zur Ebene.
 * die globale Drahtkoordinate liefert dann getWirePoint()
 */
void TQClamWirePlane::BuildWires()
{
	layerWires.clear();
	mapWireNumberToWireListIndex.clear();
	mapWireIDToWireListIndex.clear();

	layerWires.reserve(layerNWires);

	for (int i = 0; i < layerNWires; ++i)
	{
		// TODO --> beachte Offset korrekt zur berechnung der normalen Koordinate
		// --> wie bei u?
		double normalWireCoordinate = i * layerWireDist - layerXOffset;
		int id = layerID * 1000 + i;

		// baue Draht und fuege ihn zu 'layerWires' hinzu
		layerWires.emplace_back(
				layerType,
				layerZPosition,
				normalWireCoordinate,
				id,
				i,
				layerID,
				layerWireAngle,
				layerWireDist);

		std::size_t idx = layerWires.size() - 1;
		mapWireNumberToWireListIndex[i] = idx;
		mapWireIDToWireListIndex[id] = idx;
	}
}


// --- Geometrie ---
/*
 * berechnet Punkt auf (verlaengerter) Drahtlinie (global, nicht relativ zur Ebene)
 *
 * --> rechnet einfach x-/u-Koordinate des Drahts in x- und y-Projektion um
 *  z = z_Drahtebene
 */
TVector3 TQClamWirePlane::GetWirePoint(const TQClamWire* wire) const
{
	double normalCoordinate = wire->GetNormalCoordinate();
	double angle = wire->GetAngle();

	// Projektionen von normalCoordinate auf x und y
	double xProj = normalCoordinate * std::cos(angle);
	double yProj = normalCoordinate * std::sin(angle);

	// Punkt auf dem Draht
	double xPos = layerXOffset + xProj;
	double yPos = layerYOffset + yProj;
	double zPos = layerZPosition;

	return TVector3(xPos, yPos, zPos);
}
/*
TVector3 TQClamWirePlane::GetWirePoint(const TQClamWire& wire) const
{
	double normalCoordinate = wire.GetNormalCoordinate();
	double angle = wire.GetAngle();

	// Projektionen von normalCoordinate auf x und y
	double xProj = normalCoordinate * std::cos(angle);
	double yProj = normalCoordinate * std::sin(angle);

	// Punkt auf dem Draht
	double xPos = layerXOffset + xProj;
	double yPos = layerYOffset + yProj;
	double zPos = layerZPosition;

	return TVector3(xPos, yPos, zPos);
}
*/



// --- Debug ---
void TQClamWirePlane::Print() const
{
	std::cout << (layerType == WirePlaneType::U ? "U" : "X") 
		<< "-Drahtebene, ID: " << layerID
		<< "\nPosition: z = " << layerZPosition
		<< "\nAnzahl Draehte: " << layerNWires
		<< "\nWinkel Draehte zur y-Achse (in rad): " << layerWireAngle
		<< "\nAbstand zwischen benachbarten Draehten (in x-Richtung): " << layerWireDist
		<< "\nx-Laenge Ebene (mm): " << layerXLength
		<< "\nmit x-Start (mm): " << layerXOffset
		<< "\ny-Laenge Ebene (mm): " << layerYLength
		<< "\nmit y-Start (mm): " << layerYOffset << std::endl;
}
