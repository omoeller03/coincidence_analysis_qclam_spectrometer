#include "evt/TQClamDagobertCoincidenceEvent.h"

#include "TGo4Log.h"


ClassImp(TQClamDagobertCoincidenceEvent)

TQClamDagobertCoincidenceEvent::TQClamDagobertCoincidenceEvent()
	: TGo4EventElement("CoincidenceEvent")
{}


TQClamDagobertCoincidenceEvent::TQClamDagobertCoincidenceEvent(const char* name)
	: TGo4EventElement(name)
{}



TQClamDagobertCoincidenceEvent::~TQClamDagobertCoincidenceEvent()
{}



void TQClamDagobertCoincidenceEvent::Clear(Option_t* opt)
{
        TGo4EventElement::Clear(opt);

	header.file_name = "";
	header.run_number = -1;
	header.file_number = -1;

	header.user = "";
	header.time = "";
	header.label = "";

	header.mbs_event_count = 0;
	header.mbs_trigger = 0;
	
	header.mbs_buffer_number = 0;

	header.mbs_time_sec = 0;
	header.mbs_time_subsec = 0;

	fEvtCounter = 0;
	fTrackCounter = 0;

	fLaBrORCounter = 0;
	fLaBr0Counter = 0;
	fLaBr1Counter = 0;
	fLaBr2Counter = 0;
	fLaBr3Counter = 0;
	fLaBr4Counter = 0;
	fLaBr5Counter = 0;
	fLaBr6Counter = 0;
	fLaBr7Counter = 0;
	fLaBr8Counter = 0;



	// ===============================
	// Track-Parameter
	// ===============================
	phi_rad = NAN;
	phi_deg = NAN;
	phi_deg_corr = NAN;

	theta_rad = NAN;
	theta_deg = NAN;

	xD_X1 = NAN;
	xD_U = NAN;
	xD_X2 = NAN;
	uD_U = NAN;
	yD_U = NAN;

	xD_X1_corr = NAN;
	yD_U_corr = NAN;

	el_channel = NAN;
	el_energy_MeV = NAN;

	scattX = NAN;
	scattY = NAN;
	scattZ = NAN;
	scattPhi = NAN;
	scattTheta = NAN;



	// ===============================
	// LaBr-Parameter (uebernommen)
	// ===============================
	rateEnton = NAN;
	rateSzintScattChamber = NAN;
	rateLaBrOR = NAN;
	rateLaBrORNoDead = NAN;

	coincTimeLaBr = NAN;
	szintPmtUpTime = NAN;
	szintPmtDownTime = NAN;
	meanSzintTime = NAN;

	// uebernommen + erweitert hier
        // LaBr 0..8
	for(int i = 0; i < 9; ++i)
	{
		laBr[i].module = -1;
		laBr[i].channel = -1;
		laBr[i].energy = 0.;

		laBr[i].time = NAN;
		laBr[i].dtToMeanSzintTime = NAN;
		laBr[i].dtToLaBrCoincTime = NAN;
		laBr[i].pileup = false;

		laBr[i].rawScaler = 0;
		laBr[i].accScaler = 0;
		laBr[i].vetoScaler = 0;

		laBr[i].rateRaw = NAN;
		laBr[i].rateAccepted = NAN;
		laBr[i].deadtime = NAN;

		// berechnet
		laBr[i].timewalk = NAN;
		laBr[i].dtToMeanSzintTime_Corr = NAN;
		laBr[i].dtToLaBrCoincTime_Corr = NAN;
	}




	// ===============================
	// berechnet
	// ===============================
	tof = NAN;


}
