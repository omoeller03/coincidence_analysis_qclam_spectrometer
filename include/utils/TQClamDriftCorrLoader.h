#ifndef TQCLAMDRIFTCORRLOADER_H
#define TQCLAMDRIFTCORRLOADER_H

#include <string>
#include <vector>


/** Hilfsstruktur beim Laden der Daten */
struct CorrData
{
        // alle phi-winkel
        std::vector<double> phiBins;
        // driftzeit-vektoren je winkel phi
        // [[phi1: driftzeit1, driftzeit2, ...], [phi2: driftzeit1, driftzeit2, ...], ...]
        std::vector<std::vector<double>> drifttimeBins;
        // 
        std::vector<std::vector<double>> zOffsetTable;
};




class TQClamDriftCorrLoader
{
	public:
		TQClamDriftCorrLoader() = default;

		static CorrData LoadSingleROOTFile(const std::string& filename);
		static CorrData LoadSingleTXTFile(const std::string& filename);

		// TODO
		// schreibe Interpolation
		// --> vielleicht Fit an die Daten moeglich?
};


#endif
