#ifndef TQCLAMDRIFTCHAMPARAMETER_H
#define TQCLAMDRIFTCHAMPARAMETER_H

#include "TGo4Parameter.h"
#include "TQClamConstants.h"

#include "Rtypes.h"


class TQClamDriftchamGeomParameter : public TGo4Parameter
{
	public:
		TQClamDriftchamGeomParameter();
		TQClamDriftchamGeomParameter(const char* name);

		/** Max Dimension */
		static const Int_t MAX_LAYER = QCLAM_MAX_LAYER; // Maximale Anzahl Ebenen: 8

		Int_t driftchamID = 0; // Driftkammer-ID: nur pro-forma
		TString generation = ""; // Driftkammer-Generation: nur pro-forma
		Int_t nPlanes = 0; // Anzahl Drahtebenen

		static constexpr Int_t NONE = -1; // Ebenen-Typ NONE = -1 (--> keine Ebene.)
		static constexpr Int_t X = 0; // Ebenen-Typ X = 0: Unterscheidung X/U relevant fuer Tracking
		static constexpr Int_t U = 1; // Ebenen-Typ U = 1: Unterscheidung X/U relevant fuer Tracking

		TString planeName[MAX_LAYER]; // Ebenen-Namen --> kann etwa in Histogrammen verwendet werden
		Int_t planeType[MAX_LAYER]; // Ebenen-Typen X/U (Eintraege nach Ebenenposition sortiert: X1 - U - ...)
		Int_t nWires[MAX_LAYER]; // Anzahl Draehte in Ebene (Eintraege nach Ebenenposition sortiert: X1 - U ...)
		Double_t zPos[MAX_LAYER]; // z-Position/cm Ebenen (Eintraege nach Ebenenposition sortiert: X1 - U ...)
		Double_t xOffset[MAX_LAYER]; // x-Offset/cm Ebenen (Eintraege nach Ebenenposition sortiert: X1 - U ...)
		Double_t yOffset[MAX_LAYER]; // y-Offset/cm Ebenen (Eintraege nach Ebenenposition sortiert: X1 - U ...)
		Double_t xLength[MAX_LAYER]; // x-Laenge/cm Ebenen (Eintraege nach Ebenenposition sortiert: X1 - U ...)
		Double_t yLength[MAX_LAYER]; // y-Laenge/cm Ebenen (Eintraege nach Ebenenposition sortiert: X1 - U ...)
		Double_t wireDist[MAX_LAYER]; // Abstand benachbarte Draehte/cm je Ebene (Eintraege nach Ebenenposition sortiert: X1 - U ...)
		Double_t angleDeg[MAX_LAYER]; // Winkel/deg Draehte zur Y-Achse je Ebene (Eintraege nach Ebenenposition sortiert: X1 - U ...)

		Bool_t UpdateFrom(TGo4Parameter* rhs) override;


//		std::vector<TQClamPlaneGeomParameter*> planes;

		/** erzeuge PlaneParameter */
//		void InitPlanes(int n);
	
	ClassDefOverride(TQClamDriftchamGeomParameter,1);
};

#endif
