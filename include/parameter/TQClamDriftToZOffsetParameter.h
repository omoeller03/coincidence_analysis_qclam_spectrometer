#ifndef TQCLAMDRIFTTOZOFFSETPARAMETER_H
#define TQCLAMDRIFTTOZOFFSETPARAMETER_H

#include "TGo4Parameter.h"
#include "Rtypes.h"

#include "TQClamConstants.h"


struct ZOffsetInput
{
        int layerID;
        double drifttime = -1e9;
        double sigma_drifttime = -1e9;
        double xPos = -1e9;
        double sigma_xPos = -1e9;
        double phi = -1e9;
        double sigma_phi = -1e9;
        double theta = -1e9;
        double sigma_theta = -1e9;
};

struct ZOffsetResult
{
        double zOffset;
        double sigma_zOffset;
        bool valid = false;
};




class TQClamDriftToZOffsetParameter : public TGo4Parameter
{
	public:
		TQClamDriftToZOffsetParameter();
		TQClamDriftToZOffsetParameter(const char* name);

		static const Int_t MAX_LAYER = QCLAM_MAX_LAYER;
		static const Int_t MAX_PHI_BINS = QCLAM_CORR_TABLE_MAX_PHI_BINS;
		static const Int_t MAX_DRIFTTIME_BINS = QCLAM_CORR_TABLE_MAX_DRIFTTIME_BINS;

		mutable bool fBinsBuilt = false;

		Int_t Type; // Korrektur-Typ: 0 -> gemessene Korrekturtabelle, erzeugt mit QCLAMon, 1 -> mit Garfield++ simulierte Korrekturtabelle

		TString Folder; // Pfad zum Ordner der Korrektur-Dateien

		TString Files[MAX_LAYER]; // Korrektur-Datei fuer jede Ebene

		Int_t fInterpolationMode = 1; // Berechnung des z-Offsets: 0. nearest neighbor; interpolation: 1. SliceLinear, 2. SliceLinearSafe, 3. Bilinear, 4. Bicubic, 5. Spline


		mutable Int_t NPhiBins[MAX_LAYER];
		mutable Int_t NDriftBins[MAX_LAYER][MAX_PHI_BINS];


		Int_t LayerIDs[MAX_LAYER];
		Double_t PhiBins[MAX_LAYER][MAX_PHI_BINS]; // phi = PhiBins[layer][phi_bin]
		Double_t DrifttimeBins[MAX_LAYER][MAX_PHI_BINS][MAX_DRIFTTIME_BINS]; // driftzeit = DrifttimeBins[layer][phi_bin][driftzeit_bin]
		Double_t ZOffsetTable[MAX_LAYER][MAX_PHI_BINS][MAX_DRIFTTIME_BINS]; // zOffset = ZOffsetTable[layer][phi_bin][driftzeit_bin]







		/** berechne z-Offset */
		ZOffsetResult ComputeZOffset(const ZOffsetInput& in) const;

	private:
		ZOffsetResult ComputeZOffsetNearest(const ZOffsetInput& in) const; /** berechnung z-offset ohne interpolation */
		ZOffsetResult ComputeZOffsetSliceLinear(const ZOffsetInput& in) const;
		ZOffsetResult ComputeZOffsetSliceLinearSafe(const ZOffsetInput& in) const;
		ZOffsetResult ComputeZOffsetBilinear(const ZOffsetInput& in) const; /** berechnung z-offset mit interpolation */
		ZOffsetResult ComputeZOffsetBicubic(const ZOffsetInput& in) const; /** berechnung z-offset mit interpolation */
		ZOffsetResult ComputeZOffsetSpline(const ZOffsetInput& in) const; /** berechnung z-offset mit interpolation */


		Bool_t UpdateFrom(TGo4Parameter* rhs) override;

	private:
		void BuildBinCounts() const;
		int FindNearestIndex(const double* bins, size_t n, double val) const;
		int FindLowerIndex(const double* bins, size_t n, double val) const;

	ClassDefOverride(TQClamDriftToZOffsetParameter,1);
};

#endif
