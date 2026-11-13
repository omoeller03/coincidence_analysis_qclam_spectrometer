#include "geometry/TQClamWire.h"

#include <iostream>



TQClamWire::TQClamWire(WirePlaneType type, double z_coordinate, double normal_coordinate, int globalID, int wireNumber, int layerID, double angle, double deltaN)
	: wireLayerType(type),
	zCoordinate(z_coordinate),
	normalCoordinate(normal_coordinate),
	globalID(globalID),
	wireNumber(wireNumber),
	layerID(layerID),
	wireAngle(angle),
	wireDist(deltaN)
{
//	Print();
}



// --- Getter ---

WirePlaneType TQClamWire::GetType() const
{
	return wireLayerType;
}


// 
double TQClamWire::GetZCoordinate() const
{
	return zCoordinate;
}


double TQClamWire::GetNormalCoordinate() const
{
	return normalCoordinate;
}


double TQClamWire::GetAngle() const
{
	return wireAngle;
}


double TQClamWire::GetWireDist() const
{
	return wireDist;
}


int TQClamWire::GetGlobalID() const
{
	return globalID;
}


int TQClamWire::GetWireNumber() const
{
	return wireNumber;
}


int TQClamWire::GetLayerID() const
{
	return layerID;
}


// --- Geometrie ---
TVector3 TQClamWire::GetDirection() const
{
	// Richtung des Drahts (in x-y Ebene)
	// X-Ebene: angle = 0° --> Draht parallel zur y-Achse
	// U-Ebene: angle != 0° --> Draht gedreht, orthogonal zu U-Achse
	return TVector3(std::sin(wireAngle), std::cos(wireAngle), 0);
}



// --- Debug ---
void TQClamWire::Print() const
{
	std::cout << (wireLayerType==WirePlaneType::X ? "X" : "U")
		<< "-Draht: normal coordinate: " << normalCoordinate
		<< "\nangle: " << wireAngle << " rad, " << wireAngle*180./M_PI << " deg"
	       	<< "\n global Draht-ID: " << globalID
		<< "\nlocal Draht-ID in layer (wireNumber): " << wireNumber
		<< "\nDraht in layer with ID: " << layerID << std::endl;
}


