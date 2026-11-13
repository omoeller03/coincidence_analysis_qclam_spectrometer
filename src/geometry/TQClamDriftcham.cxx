// TQClamDriftchamber
#include "geometry/TQClamDriftcham.h"

#include "TGo4Log.h"
#include <iostream>



// Konstruktor
TQClamDriftcham::TQClamDriftcham(int id, const std::string& gen)
	: driftchamID(id),
	driftchamGen(gen)
//	chamber_verbose(verbose)
{
	TGo4Log::Info("[QClamDriftcham] Create %s Driftchamber.", driftchamGen.c_str());
}



// --- Getter ---

// driftkammerID ausgeben
int TQClamDriftcham::GetID() const
{
	return driftchamID;
}


// alle Drahtebenen als vector ausgeben
const std::vector<TQClamWirePlane>& TQClamDriftcham::GetWirePlanes() const
{
	return wirePlanes;
}


// ebene mit ebeneID ausgeben
const TQClamWirePlane* TQClamDriftcham::GetWirePlane(int layerID) const
{
	if(layerID < 0 || layerID >= static_cast<int>(wirePlanes.size()))
	{
	       	std::cerr << "[Driftcham] Ungueltige layerID: " << layerID
			<< " (nEbenen: " << wirePlanes.size() << ")\n";
		return nullptr;
	}

	return &wirePlanes.at(layerID);
}


const TQClamWire* TQClamDriftcham::GetWire(int layerID, int wireNumber) const
{
	if(layerID < 0 || layerID >= static_cast<int>(wirePlanes.size()))
	{
		std::cerr << "[Driftkammer] Unguelige layerID: " << std::to_string(layerID)
			<< " (nEbenen: " << std::to_string(wirePlanes.size())
			<< ")\n" << std::endl;
		return nullptr;
	}

	return wirePlanes.at(layerID).GetWireByWireNumber(wireNumber);
}





/// fuege der Driftkammer eine ebene hinzu
void TQClamDriftcham::AddWirePlane(TQClamWirePlane layer)
{
	layer.BuildWires(); 			// ebene soll draehte bauen

	wirePlanes.push_back(std::move(layer)); 	// speichere ebene in driftkammer
}


/** debug - parameter printen */
void TQClamDriftcham::Print() const
{
	switch(driftchamID)
	{
		case 3:
			std::cout << "Kammer Generation 3 (Driftkammer-ID: " << driftchamID << ")" << std::endl;
			break;
		case 4:
			std::cout << "Kammer Generation 4 (Driftkammer-ID: " << driftchamID << ")" << std::endl;
			break;
		default:
			std::cout << "Diesen Kammertypen kenne ich nicht (Driftkammer-ID: " << driftchamID << ")" << std::endl;
	}
}


