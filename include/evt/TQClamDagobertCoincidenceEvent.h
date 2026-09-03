#ifndef TQCLAMDAGOBERTCOINCIDENCEEVENT_H
#define TQCLAMDAGOBERTCOINCIDENCEEVENT_H

#include "TGo4EventElement.h"

#include "structs/TQClamMbsHeader.h"
#include "structs/TQClamScaler.h"
#include "structs/TQClamTriggerDetectors.h"




struct DagCoincHit
{
	// ===============================
	// uebernommen
	// ===============================
        int module = -1;
        int channel = -1;

        double energy = NAN;
        double calibrated_energy = NAN;
        double time = NAN;
        double dtToMeanSzintTime = NAN;
        double dtToLaBrCoincTime = NAN;
        bool pileup = false;


        uint64_t rawScaler = 0;
        uint64_t accScaler = 0;
        uint64_t vetoScaler = 0;
        bool hasScaler = false;

        double rateRaw = NAN;
        double rateAccepted = NAN;
        double deadtime = NAN;

	// ===============================
	// berechnet
	// ===============================
	double timewalk = NAN;

	double dtToMeanSzintTime_Corr = NAN;
	double dtToLaBrCoincTime_Corr = NAN;
};



class TQClamDagobertCoincidenceEvent : public TGo4EventElement
{
	public:
		TQClamDagobertCoincidenceEvent();
		TQClamDagobertCoincidenceEvent(const char* name);

		virtual ~TQClamDagobertCoincidenceEvent();



                void Clear(Option_t* opt="") override;


		MbsHeader header; //! nicht streamen
		TQClamScaler scaler; //! nicht streamen
		TQClamTriggerDetectors trigDet; //! nicht streamen

		int fEvtCounter = 0;
		int fTrackCounter = 0; // (nur konvergierte Tracks)

		int fLaBrORCounter = 0;
		int fLaBr0Counter = 0;
		int fLaBr1Counter = 0;
		int fLaBr2Counter = 0;
		int fLaBr3Counter = 0;
		int fLaBr4Counter = 0;
		int fLaBr5Counter = 0;
		int fLaBr6Counter = 0;
		int fLaBr7Counter = 0;
		int fLaBr8Counter = 0;

		// ===============================
		// Track-Parameter
		// ===============================
		double phi_rad{NAN};
		double phi_deg{NAN};
		double phi_deg_corr{NAN};

		double theta_rad{NAN};
		double theta_deg{NAN};

		double xD_X1{NAN};
		double xD_U{NAN};
		double xD_X2{NAN};
		double uD_U{NAN};
		double yD_U{NAN};

		double xD_X1_corr{NAN};
		double yD_U_corr{NAN};

		double el_channel{NAN};
		double el_energy_MeV{NAN};

		double scattX{NAN};
		double scattY{NAN};
		double scattZ{NAN};
		double scattPhi{NAN};
		double scattTheta{NAN};




		// ===============================
		// LaBr-Parameter (uebernommen)
		// ===============================
		double rateEnton{NAN};
		double rateSzintScattChamber{NAN};
		double rateLaBrOR{NAN};
		double rateLaBrORNoDead{NAN};

		double coincTimeLaBr{NAN};
		double szintPmtUpTime{NAN};
		double szintPmtDownTime{NAN};
		double meanSzintTime{NAN};

		// uebernommen + erweitert hier
		DagCoincHit laBr[9]; //!


		// ===============================
		// berechnet
		// ===============================
		double tof = NAN;


	ClassDefOverride(TQClamDagobertCoincidenceEvent,1);
};


#endif
