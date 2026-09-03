#include "parameter/TQClamTrackConditionParameter.h"

#include "TGo4Log.h"

ClassImp(TQClamTrackConditionParameter)

//////////////////////////////////////////////////////////////////////

TQClamTrackConditionParameter::TQClamTrackConditionParameter()
	: TGo4Parameter("TrackConditionParam")
{
	TGo4Log::Info("[TQClamTrackConditionParameter] Create TrackConditionPar.");

	for(size_t i=0; i<MAX_LAYER; i++)
	{
		AllowedLayersForTrack[i] = kFALSE;
		RequiredLayersForTrack[i] = kFALSE;
	}

	min_x_X1 = -1e9;
	max_x_X1 = 1e9;

	min_phi_deg = 0.;
	max_phi_deg = 360.;



	fMinIter = 2;
	fMaxIter = 50;
	RelaxationAlpha = 1.;
	EpsilonPhi = 1e-5;
	EpsilonTheta = 1.e-3;
	EpsilonChi2 = 1e-3;

	EpsilonPhiStagnation = 1e-12;

	UseOutlierRejection = kFALSE;
	OutlierRejectionLimit = 3.0;
	UpperOutlierRejectionLimit = 3.;

	fMaxRejectedHitsX1PerIter = 2;
	fMaxRejectedHitsUPerIter = 2;
	fMaxRejectedHitsX2PerIter = 2;
}

//////////////////////////////////////////////////////////////////////

TQClamTrackConditionParameter::TQClamTrackConditionParameter(const char* name)
	: TGo4Parameter(name)
{
	TGo4Log::Info("[TQClamTrackConditionParameter] Create %s.", name);


	for(size_t i=0; i<MAX_LAYER; i++)
	{
		AllowedLayersForTrack[i] = kFALSE;
		RequiredLayersForTrack[i] = kFALSE;
	}

	min_x_X1 = -1e9;
	max_x_X1 = 1e9;

	min_phi_deg = 0.;
	max_phi_deg = 360.;


	fMinIter = 2;
	fMaxIter = 50;
	RelaxationAlpha = 1.;
	EpsilonPhi = 1e-5;
	EpsilonTheta = 1.e-3;
	EpsilonChi2 = 1e-3;

	EpsilonPhiStagnation = 1e-12;

	UseOutlierRejection = kFALSE;
	OutlierRejectionLimit = 3.0;
	UpperOutlierRejectionLimit = 3.;

	fMaxRejectedHitsX1PerIter = 2;
	fMaxRejectedHitsUPerIter = 2;
	fMaxRejectedHitsX2PerIter = 2;

	ConvergenceCriterion_Phi = kTRUE;
	ConvergenceCriterion_RMS = kFALSE;
	ConvergenceCriterion_PhiAndRMS = kFALSE;
	ConvergenceCriterion_PhiOrRMS = kFALSE;

	EConvergenceCriterion ConvergenceCriterion = EConvergenceCriterion::Phi; // Default



}


Bool_t TQClamTrackConditionParameter::Test_X_X1(double x) const
{
	if(x >= min_x_X1 && x <= max_x_X1)
	{
		return kTRUE;
	}

	return kFALSE;
}




Bool_t TQClamTrackConditionParameter::Test_Phi_Deg(double phi) const
{
	if(phi >= min_phi_deg && phi <= max_phi_deg)
	{
		return kTRUE;
	}

	return kFALSE;
}




Bool_t TQClamTrackConditionParameter::UpdateFrom(TGo4Parameter* rhs)
{
	TGo4Log::Info("[TQClamTrackConditionParameter] UpdateFrom()");
	auto* other = static_cast<TQClamTrackConditionParameter*>(rhs);
	if(!other)
	{
		TGo4Log::Error("[TQClamTrackConditionParameter] UpdateFrom() nicht erfolgreich!");
		return kFALSE;
	}

	for(size_t i=0; i<MAX_LAYER; i++)
	{
		AllowedLayersForTrack[i] = other->AllowedLayersForTrack[i];
		RequiredLayersForTrack[i] = other->RequiredLayersForTrack[i];
	}


	min_x_X1 = other->min_x_X1;
	max_x_X1 = other->max_x_X1;

	min_phi_deg = other->min_phi_deg;
	max_phi_deg = other->max_phi_deg;


	fMinIter = other->fMinIter;
	fMaxIter = other->fMaxIter;
	RelaxationAlpha = other->RelaxationAlpha;
	EpsilonPhi = other->EpsilonPhi;
	EpsilonTheta = other->EpsilonTheta;
	EpsilonChi2 = other->EpsilonChi2;
	EpsilonRMS = other->EpsilonRMS;

	EpsilonPhiStagnation = other->EpsilonPhiStagnation;

	UseOutlierRejection = other->UseOutlierRejection;
	OutlierRejectionLimit = other->OutlierRejectionLimit;
	UpperOutlierRejectionLimit = other->UpperOutlierRejectionLimit;

	fMaxRejectedHitsX1PerIter = other->fMaxRejectedHitsX1PerIter;
	fMaxRejectedHitsUPerIter = other->fMaxRejectedHitsUPerIter;
	fMaxRejectedHitsX2PerIter = other->fMaxRejectedHitsX2PerIter;

	ConvergenceCriterion_Phi = other->ConvergenceCriterion_Phi;
        if(ConvergenceCriterion_Phi)
        {
                ConvergenceCriterion = EConvergenceCriterion::Phi;
        }

	ConvergenceCriterion_RMS = other->ConvergenceCriterion_RMS;
        if(ConvergenceCriterion_RMS)
        {
                ConvergenceCriterion = EConvergenceCriterion::RMS;
        }

	ConvergenceCriterion_PhiAndRMS = other->ConvergenceCriterion_PhiAndRMS;
        if(ConvergenceCriterion_PhiAndRMS)
        {
                ConvergenceCriterion = EConvergenceCriterion::PhiAndRMS;
        }

	ConvergenceCriterion_PhiOrRMS = other->ConvergenceCriterion_PhiOrRMS;
        if(ConvergenceCriterion_PhiOrRMS)
        {
                ConvergenceCriterion = EConvergenceCriterion::PhiOrRMS;
        }





	return kTRUE;
}
