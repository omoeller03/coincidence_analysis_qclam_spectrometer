#ifndef TQCLAMOFFLINEROOTSOURCE_h
#define TQCLAMOFFLINEROOTSOURCE_h

#include "TGo4EventSource.h"

//#include <configs/VerboseLevel.hh>

//#include <io/reader/IDataReader.hh>
//#include <reco/objects/RawEvent.hh>

#include <TFile.h>
#include <TTree.h>
#include <TKey.h>

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <deque>

class TQClamRawEvent; // zu fuellendes Eventelement

// TODO Parameterklassen
class TGo4UserSourceParameter;

/*
 * mein verstaendnis:
 *
 * DAQ liefert 
 * Ebene 1) Scaler-Tree
 * Ebene 1) TDC-Trees (0 ... 17)
 *
 *  |_ Ebene 2) in TDC-Trees mehrere Entries
 *  	jeder Entry ist ein Block mit mehreren physikalischen Ereignissen (= Szint-trigger mit ausgelesenen TDC-Hits)
 *  	Entry sieht so aus (Liste von Subevents):
 *  	eventNumber = {100, 101, 102, ...}
 *  	channel = {{1,2}, {5,1,2}, {...}, ...}
 *  	drifttime = {{...}, {...}, {...}, ... }
 *  	...
 *
 *  	|_ Ebene 3) in einem Entry stecken somit mehrere SubEvents:
 *  		eventNumber[0] = 100
 *  		channel[0} = {1,2}
 *  		drifttime[0] = {...}
 *  		...
 *
 *  		--> ein Subevent ist also ein Szint-Trigger mit allen TDC-Hit Informationen
 *  		--> ein Subevent wird also zu einem RawEvent
 *
 *  		|_ Ebene 4) einzelner Hit --> einzelner TDC-Kanal
 *  			channel[0][0] = 1
 *  			drifttime[0][0] = t0
 *  			...
 *
 * also muessen die SubEvents aus Entrys verschiedener TDC-Trees zusammengefuehrt und in einem RawEvent abgespeichert werden
 * --> das zusammenfuehren geschieht uber EntryNummer (currentEntry) und SubEventNummer (currentSubEvent)
 *
 * Info: Entry existiert vmtl nur, weil die Hardware blockweise schreibt
 * --> Hardware schreibt mehrere SubEvents (= reale Events) auf einmal
 *
 *
 * je TDC:
 * - channel 0 --> tdc-trigger --> auslese kann gestartet werden (delayter szintillator-trigger)
 * - channel 1 - 48 --> leading-edges fuer kanal 1 - 48 (konfigurierbar)
 * - channel 51 - 98 --> trailing-edges fuer kanal 1 - 48 (konfigurierbar)
 * ->> schreibe erst einmal zeiten und kanaele gleichermassen in RawEvent
 * ->> keine Unterscheidung leading/trailing edge (erst in TQClamHitProc)
 *
 */


class TQClamOfflineRootSource : public TGo4EventSource
{
	public:
		/** konstruktor mit direkter parameter-uebergabe 
		 * --> nicht implementiert aktuell
		 * --> waere fuer stream von root-daten... nicht sinnvoll... */
		//TQClamOfflineRootSource(const char* name, const char* args, Int_t port);

		TQClamOfflineRootSource();
		/** konstruktor ueber go4-parameter */
		TQClamOfflineRootSource(TGo4UserSourceParameter* sourcepar);

		/** destruktor */
		virtual ~TQClamOfflineRootSource();


		/** oeffne file */ 
		virtual Int_t Open();

		/** schliesse file */
		virtual Int_t Close();

		/** baue event & fuelle target-event klasse */
		virtual Bool_t BuildEvent(TGo4EventElement* dest) override;

		/** pruefe, ob event-class passend fuer source */
		Bool_t CheckEventClass(TClass* cl) override;

//		bool hasNext() const override;
//		RawEvent next() override;

		// gebe alle IDs der Inputfiles aus --> zur benennung der monitoring und outpufiles
		const std::vector<int>& getAllRunIDs() const;

	private:
		std::vector<int> runIDs; // speichere FileIDs fuer Output- und Monitoring-filenamen
		std::map<int, std::vector<int>> filesInRun; // map mit [[RunID1 -> (fileNummer1, fileNummer2,...)], [RunID2 -> (...)], ... ]
		int currentRunID; // RunID, die gerade ausgewertet wird
		int currentFileNumber; // File Nummer, die gerade ausgewertet wird
		int currentRunPosition = -1; // laufindex der runnumern, die ich gerade auswerte --> werte 1./2./... run aus
		int currentFilePositionInRun = -1; // laufindex der files in einem run. falls mehrere crates (alte DAQ): alle files mit gleicher run- und filenummer zaehlen zu einer currentFile..Position
		// Methoden, die inputIDs sammelt
		void collectAllRunIDs(const std::vector<std::string>& filenames);
		std::pair<int,int> readID(const std::string& filename);


	public:
		// TDC und Scaler Strukturen
		struct TDCTree {
			int runID = -1;	//------ runID
			int fileNumber = -1; //----- FileID in run
			int tdcID = -1;
			TTree* tree = nullptr;

			std::vector<unsigned short>* eventNumber = nullptr; // aktuelle SubEvents
			std::vector<unsigned short>* triggerRequestCounter = nullptr;
			std::vector<ULong64_t>* timestampInMicroSeconds = nullptr;
			std::vector<std::vector<unsigned short>>* channel = nullptr;
			std::vector<std::vector<double>>* timeRawInNanoSeconds = nullptr; // rohe driftzeit in ns (Triggerzeit Hit relativ zu TDC-Trigger) --> alle Zeiten gesammelt
			std::vector<std::vector<double>>* tdcTrigger = nullptr; // Triggerzeit TDC (Kanal 0)
			std::vector<std::vector<double>>* leadingEdgeNanoSeconds = nullptr; // Leading Edge Signal (Kanal 1 - 48 je TDC)
			std::vector<std::vector<double>>* trailingEdgeNanoSeconds = nullptr; // Trailing Edge Signal (Kanal 49 - 96 je TDC)
			
			Long64_t nEntries = 0;
		};

		struct ScalerTree
		{
			int runID = -1; //----------- runID
			int fileNumber = -1; //--------- file ID in run
			TTree* tree = nullptr;

			// Zeitstempel in us
			ULong64_t scalerTimeMicroSeconds = 0;

			// Hauptzaehler
			ULong64_t TDCTrigger = 0;
			ULong64_t scintillatorPMTUp = 0;
			ULong64_t scintillatorPMTDown = 0;
			ULong64_t cherenkovPMT = 0;
			ULong64_t coincidenceTrigger = 0;
			ULong64_t acceptedTrigger = 0;
			ULong64_t clockCounter = 0;
			ULong64_t livetimeCounter = 0;
			ULong64_t mbsLivetimeCounter = 0;
			ULong64_t faradayCupChargeCounter = 0;

			// Fehlerzaehler
			ULong64_t errorTDCTrigger = 0;
			ULong64_t errorScintillatorPMTUp = 0;
			ULong64_t errorScintillatorPMTDown = 0;
			ULong64_t errorCherenkovPMT = 0;
			ULong64_t errorCoincidenceTrigger = 0;
			ULong64_t errorAcceptedTrigger = 0;
			ULong64_t errorClockCounter = 0;
			ULong64_t errorLivetimeCounter = 0;
			ULong64_t errorMBSLivetimeCounter = 0;
			ULong64_t errorFaradayCupChargeCounter = 0;


			Long64_t nEntries = 0;
		};

	private:
//		VerboseLevel reader_verbose;
		// Variablen & Hilfsmethoden
		//std::vector<std::unique_ptr<TFile>> files;
		std::map<int, std::map<int, std::vector<std::unique_ptr<TFile>>>> files; //--- [runID, [fileNumber, [file_c1, file_c2]]]
		//std::vector<TDCTree> tdcTrees;
		std::deque<TDCTree> tdcTrees;
		//std::vector<ScalerTree> scalerTrees;
		std::deque<ScalerTree> scalerTrees;
		TDCTree* Tref = nullptr;

		Long64_t currentEntry = -1;
		int currentSubEvent = -1;
		Long64_t nEntriesRef = -1;
		int nSubEventsRef = -1;

		bool nextPosition = true;

		void init(const std::vector<std::string>& filenames);
		void openFiles(const std::vector<std::string>& filenames);
		void findTDCTrees();
		void setupTDCTreeBranches();

		void findScalerTrees();
		void setupScalerTreeBranches();

		bool advancePosition();
		void loadReferenceSubEvent(); //---------------------------- lade Referenz-Tree (kleinste tdcID) und stelle sicher, 
					      //---------------------------- dass currentEntry/currentSubEvent auf gueltiges SubEvent zeigen
		void loadReferenceTree();
		void fillEventMetadata(TQClamRawEvent& evt); //-- fuelle eventNumber, triggerID & triggerTime in RawEvent
		void fillTDCHits(TQClamRawEvent& evt); //-------- erzeuge & fuelle RawHit Strukturen und packe sie in RawEvent
		void fillScalerData(TQClamRawEvent& evt); //----- erzeugt & fuelle RawScaler Strukturen und packe sie in RawEvent


		/*
		int currentEventIndex;
		int totalEvents;
		*/

	/*
		Event readNexEvent()
		{
			Event evt(currentEventID++);

			int wireID;
			double absTime, driftTime;

			while(...)
			{
				Draht* wire = geometry.findWire(wireID);
				double driftLength = calibrateDrift(driftTime);
				Hit hit(wireID, absTime, driftTime, driftLength, wire);
				evt.addHit(hit);
			}

			return evt;
		}


	private:
		int currentEventID = 0;
		Geometry geometry;
	*/
	


	ClassDefOverride(TQClamOfflineRootSource, 1);
};



#endif




