/*
	alter QCLAMonReader
*/
#include "source/TQClamOfflineRootSource.h"

#include "TGo4Log.h"
#include "TGo4EventEndException.h"
#include "TGo4EventErrorException.h"
#include "TGo4RuntimeException.h"

#include "TObjString.h"

#include "evt/TQClamRawEvent.h"
#include "parameter/TQClamSourceParameter.h"

#include <iostream>
#include <sstream>
#include <regex>
#include <set>

ClassImp(TQClamOfflineRootSource)


/** Konstruktor */
//***********************************************************
TQClamOfflineRootSource::TQClamOfflineRootSource()
	: TGo4EventSource()
{}

//***********************************************************
TQClamOfflineRootSource::TQClamOfflineRootSource(TGo4UserSourceParameter* par)
	: TGo4EventSource("TQClamOfflineRootSource")//, sourcepar)
{
	TGo4Log::Info("[TQClamOfflineRootSource]  ctor, par=%p", par);

        if(!par)
        {
                TGo4Log::Error("[TQClamOflineRootSource] Konstruktur mit NULL Parameter!");
                return;
        }




    // Expression enthält die kommagetrennte Fileliste
    TString list = par->GetExpression();
    if (list.IsNull()) {
        // ältere Go4-Versionen nutzen Name statt Expression
        list = par->GetName();
    }

    std::vector<std::string> filenames;

    TObjArray* arr = list.Tokenize(",");
    for (int i = 0; i < arr->GetEntriesFast(); i++) {
        TString f = ((TObjString*)arr->At(i))->GetString().Strip(TString::kBoth);
        if (!f.IsNull()) {
            filenames.push_back(f.Data());
            TGo4Log::Info("[TQClamOfflineRootSource] File: %s", f.Data());
        }
    }
    delete arr;

    if (filenames.empty()) {
        TGo4Log::Error("[TQClamOfflineRootSource] Keine Input-Files!");
        TGo4RuntimeException();
    }

    init(filenames);


}



/*
 * Destruktor --> default
 */
TQClamOfflineRootSource::~TQClamOfflineRootSource()
{
	Close();
}




void TQClamOfflineRootSource::init(const std::vector<std::string>& filenames)
{

	/*
	if(reader_verbose.verbose>0)
	{
		std::cout << "[DataReader] Auszuwertende Daten:\n" << std::endl;

		for(const auto& elem : filenames)
		{
			std::cout << elem << "\n" << std::endl;
		}
	}
	*/


	// root-files oeffnen
	openFiles(filenames);

	// TDCTrees suchen & pruefen
	findTDCTrees();
	if(tdcTrees.empty())
	{
		throw std::runtime_error("[TQClamOfflineRootSource]: keine TDC-Trees gefunden.");
	}

	// initialisiere vektor mit filename fuer ausgabedateien
	collectAllRunIDs(filenames);

	// TDCTree-Daten einlesen (in Struktur TDCTree
	setupTDCTreeBranches();

	// ScalerTrees suchen & pruefen
	// nur in einem der zusammengehoerenden Rohdatenfiles ist ein scaler-tree
	findScalerTrees();
	if(!scalerTrees.empty()) { setupScalerTreeBranches(); }

	// sortiere TDCTrees
	std::sort(tdcTrees.begin(), tdcTrees.end(),
			[](const TDCTree& a, const TDCTree& b)
			{
			if (a.runID != b.runID) return a.runID < b.runID;
			if (a.fileNumber != b.fileNumber) return a.fileNumber < b.fileNumber;
			return a.tdcID < b.tdcID;
			}
			);

	// zaehler setzen
	currentRunPosition = 0;
	currentFilePositionInRun = 0;

	// setze currentRunID und currentFileNumber
	currentRunID = runIDs[0];
	currentFileNumber = filesInRun[currentRunID][0];
	

	// setze Zaehler
	currentEntry = -1;
	currentSubEvent = -1;

	// referenz tree zaehler
	nEntriesRef = 0; // wird in loadReferenceTree gesetzt
	nSubEventsRef = 0; // wird in loadReferenceTree gesetzt

	// lade Referenz-Tree --> setze nEntriesRef, nSubEventsRef
	loadReferenceTree();
}





/*
 * Getter fuer Vektor mit allen IDs (aus filename: <name>_<evtl Cratenummer>_<ID>_<FilenummerProID>.root)
 */
const std::vector<int>& TQClamOfflineRootSource::getAllRunIDs() const
{
	return runIDs;
}




/*
 * Oeffne uebergebene files, pruefe, ob sie leer sind/wirklich existieren
 * --> wenn nicht leer und existent --> fuege sie zu internen vektor von files hinzu - andernfalls runtime error
 */
void TQClamOfflineRootSource::openFiles(const std::vector<std::string>& filenames)
{
	// files nacheinander einlesen
	// TDCTree (Struktur) erzeugen, fuellen & vector tdcTrees hinzufuegen
	// ScalerTree (Struktur) erzeugen, fuellen & vector scalerTrees hinzufuegen
//	if(reader_verbose.verbose>0)
//	{
	TGo4Log::Info("[TQClamOfflineRootSource] Files werden eingelesen:");
//	}
	int i = 1;
	for(const auto& fname : filenames)
	{
		std::cout << std::to_string(i) << ".: "  << fname << std::endl;
		auto f = std::make_unique<TFile>(fname.c_str(), "READ");
		if(!f || f->IsZombie())
		{
			throw std::runtime_error("[TQClamOfflineRootSource] Konnte Datei nicht oeffnen: " + fname);
		}

		auto [runID,fileNumber] = readID(fname);

		files[runID][fileNumber].push_back(std::move(f));
	}

	/*
	if(reader_verbose.verbose>0)
	{
		std::cout << "\n" << std::endl;
	}
	*/

}


/*
 * Lese RunID
 */
std::pair<int,int> TQClamOfflineRootSource::readID(const std::string& filename)
{
	// Dateinamen ohne Pfad extrahieren
	std::string path = filename;
	auto pos = path.find_last_of("/\\");
	std::string base;
	std::string file;
	if(pos == std::string::npos) { file = path; }
       	else
	{
		base = path.substr(0, pos);
		file = path.substr(pos + 1); 
	}

	// nun habe ich das root file vom path extrahiert in 'file'
	// split bei '_'
	std::vector<std::string> parts;
	std::stringstream ss(file);
	std::string item;

	// suche nach '_c1_' oder '_c2_' etc. in dem file
	// --> alte DAQ funktioniert nur mit getrennt voneinander laufenden crates --> '_c<zahl>_' pro Crate
	// --> neue DAQ mached die Crates und schreibt sie gemeinsam weg --> kein '_c<zahl>_'
	std::regex pattern("_c[0-9]+_");
	std::smatch match; //---------------------------------------- speichert alle matches von 'pattern' ab: match[0], mathc[1] --> wenn nichts gefunden --> match.size() == 0

//	if(reader_verbose.verbose>0)
//	{
//		if(std::regex_search(file, match, pattern)) { std::cout << "File von alter DAQ --> gesplittet auf mehrere Crates." << std::endl; }
//		else { std::cout << "[DataReader] File von neuer DAQ --> Crates wurden in DAQ gemached." << std::endl; }
//	}

	// suche '_' in 'file'
	while(std::getline(ss, item, '_')) { parts.push_back(item); }

	// kein '_c<zahl>_' gefunden --> file aus neuer DAQ
	if(match.size() == 0)
	{

		// das file muss mind aus zwei teilen, getrennt von '_' bestehen: <name>_<ID>_<fileProID>
		if(parts.size() < 2) { throw std::runtime_error("Kein gueltiger Filename: " + filename); } // kein gueltiger name

		// dritte komponente ist File (ID) --> vierte komponente koennte auch noch ausgelesen werden...
		// TODO
		auto pointVal = parts[parts.size()-1].find('.');
		int fileNumber = std::stoi(parts.back().substr(0,pointVal));
//		int fileNumber = std::stoi(parts[parts.size()-1].substr(0, pointVal-1));
		int runID = std::stoi(parts[parts.size()-2]);
		return {runID, fileNumber};
	}

	if(parts.size()<3) { throw std::runtime_error("[TQClamOfflineRootSource] Kenne Filenamenstruktur nicht"); }

	// splitte '.root' von letztem parts ab --> dies die Filenummer einer Run ID
	auto pointVal = parts[parts.size()-1].find('.');
//	int fileNumber = std::stoi(parts[parts.size()-1].substr(0, pointVal-1));
	int fileNumber = std::stoi(parts.back().substr(0,pointVal));
	int runID = std::stoi(parts[parts.size()-2]);

	return {runID, fileNumber};
}


/*
 * sammle alle IDs der Uebergabefiles in runIDs
 */
void TQClamOfflineRootSource::collectAllRunIDs(const std::vector<std::string>& filenames)
{
	runIDs.clear();
	filesInRun.clear();

	for(const auto& filename : filenames)
	{
		auto [runID, fileNumber] = readID(filename);
//		if(reader_verbose.verbose>0)
//		{
//			std::cout << "Run ID: " << std::to_string(runID) << ", filenumber: " << std::to_string(fileNumber) << std::endl;
//		}
		runIDs.push_back(runID);
		filesInRun[runID].push_back(fileNumber);
	}

	// sortiere vector runIDs und lasse jede runID nur einmal drin
	std::sort(runIDs.begin(), runIDs.end());
	runIDs.erase(std::unique(runIDs.begin(), runIDs.end()), runIDs.end());

	// sortiere fileNumber je runID innerhalb von filesInRun: 
	for(auto& [runID, fileList] : filesInRun)
	{
		std::sort(fileList.begin(), fileList.end());
	}
}




/*
 * suche TDCTrees aus eingelesenen files
 */
void TQClamOfflineRootSource::findTDCTrees()
{
//	if(reader_verbose.verbose>0)
//	{
//		std::cout << "Trees in Files werden eingelesen:" << std::endl;
//	}
	// hole nacheinander files und erzeuge TDCTree-Strukturen
	int i=1; // zum zaehlen der files

	// separiere nach Run ID
	for(auto& [runID, fileListRun] : files)
	{
		// separiere nach File Nummer in Run
		for(auto& [fileNumber, fileList] : fileListRun)
		{
			// falls alte DAQ --> zwei Crates --> zwei Files pro Run & Filenummer
			for(auto& fpointer : fileList)
			{
				// hole file
				TFile* f = fpointer.get();
				// erstelle liste/iterator aus allen Objekten in Root-File
				TIter nextKey(f->GetListOfKeys());

//				if(reader_verbose.verbose)
//				{
//					std::cout << "File Nummer " << std::to_string(i) << ".: " << f->GetName() << std::endl;
//				}

				// fuer root-backup-cycles --> darf trees nicht mehrfach ueberschreiben
				std::set<std::string> seenTrees;
	
				//while(TKey* key = (TKey*)nextKey())
				while(TKey* key = static_cast<TKey*>(nextKey()))
				{
					// extrahiere keys (Objekte), die Trees sind
					if(std::string(key->GetClassName()) != "TTree") { continue; }
	
					// fuer jedes ELB-TDC Modul gibt es einen Tree
					// --> die Trees im file heissen: tdc_<nummer>
					// --> nummer = tdcID
					std::string name = key->GetName();
					if(name.rfind("tdc_", 0) != 0) { continue; } // pruefe, ob Tree mit 'tdc_' beginnt

					// gleiche namen (backup-cycles) nur einmal zulassen
					if(seenTrees.count(name)) { continue; }
					seenTrees.insert(name);
	
					// lese Tree ein
					//TTree* tree  = (TTree*)key->ReadObj();
					//TTree* tree = dynamic_cast<TTree*>(key->ReadObj());
					//
					//kein ReadObj, sondern neuesten Cycle holen
					TTree* tree = dynamic_cast<TTree*>(f->Get(name.c_str()));
					if(!tree) { continue; }
	
					// extrahiere tdcID --> tdc_<tdcID>
					int tdcID = std::stoi(name.substr(name.find('_') + 1)); 
					//std::cout << "TDCID einlesen: " << std::to_string(tdcID) << std::endl;

//					if(reader_verbose.verbose>0)
//					{
//						std::cout << "Tree-TDC ID: " << std::to_string(tdcID) << std::endl;
//					}
		
					// Struktur erzeugen & mit ersten werten fuellen --> driftzeit etc spaeter
					TDCTree T;
					T.runID = runID;
					T.fileNumber = fileNumber;
					T.tdcID = tdcID;
					T.tree = tree;
					T.nEntries = tree->GetEntries();

		
					// fuege TDCTree Struktur vector tdcTrees hinzu
					tdcTrees.push_back(T);
				}

				i++;
			}
		}

	}

//	if(reader_verbose.verbose>0)
//	{
//		std::cout << "\n" << std::endl;
//	}
}



void TQClamOfflineRootSource::setupTDCTreeBranches()
{
	for(auto& T : tdcTrees)
	{
		// in der Struktur TDCTree liegt der eingelesene tdcTree tree --> bereite Trees zum Einlesen der Daten vor (setze Branch Adressen)
		T.tree->SetBranchAddress("eventNumber", &T.eventNumber);
		T.tree->SetBranchAddress("triggerRequestCounter", &T.triggerRequestCounter);
		T.tree->SetBranchAddress("timestampInMicroSeconds", &T.timestampInMicroSeconds);
		T.tree->SetBranchAddress("channel", &T.channel);
		T.tree->SetBranchAddress("timeRawInNanoSeconds", &T.timeRawInNanoSeconds);




		/*
					if(T.tdcID == 0)
					{
    						std::cout << "\n=== DEBUG: tdc_0 Rohdatenprüfung ===\n";

    						// Lies einen Entry, von dem du weißt, dass er im Reader falsche Werte liefert
    						Long64_t testEntry = 977;   // oder 985, 986, 1005 – was du willst

    						std::cout << "Lese Entry " << testEntry << " aus tdc_0\n";

						T.tree->GetEntry(testEntry);
						
						if (!T.channel || !T.timeRawInNanoSeconds)
						{
        						std::cout << "FEHLER: Branchpointer ist NULL!\n";
    						} else {
        						std::cout << "channel->size() = " << T.channel->size() << "\n";
        						std::cout << "timeRawInNanoSeconds->size() = " << T.timeRawInNanoSeconds->size() << "\n";

        						// Wir wissen: eventNumber.size() == 1 → also SubEvent 0
        						if (T.channel->size() > 0)
							{
            							const auto& ch = T.channel->at(0);
            							const auto& dt = T.timeRawInNanoSeconds->at(0);

            							std::cout << "SubEvent 0: " << ch.size() << " Hits\n";

            							for (size_t i = 0; i < ch.size(); ++i)
								{
                							std::cout << "  Hit " << i
                          							<< "  Kanal=" << ch[i]
                          							<< "  dt=" << dt[i]
                          							<< "\n";
            							}
        						}
    						}

						std::cout << "=== ENDE DEBUG ===\n\n";
					}

		*/

	}
}



/*
 * Scaler-Tree aus file holen und ScalerTree Struktur erzuegen
 */
void TQClamOfflineRootSource::findScalerTrees()
{
	// separiere nach runID
	for(auto& [runID, fileListRun] : files)
	{
		// separiere nach fileNummer in run
		for(auto& [fileNumber, fileList] : fileListRun)
		{
			// falls alte DAQ --> zwei Crates --> zwei Files
			for(auto& fpointer : fileList)
			{
				// tree holen und pruefen, ob er existiert
				TTree* tree = dynamic_cast<TTree*>(fpointer->Get("scaler"));
				if(!tree) { continue; }

				// ScalerTree Struktur erzeugen und mit ersten Werten fuellen
				ScalerTree S;
				S.runID = runID;
				S.fileNumber = fileNumber;
				S.tree = tree;
				S.nEntries = tree->GetEntries();
		
				// ScalerTree Struktur in vector scalerTrees hinzufuegen
				scalerTrees.push_back(S);
			}
		}
	}
}



/*
 * lese Scaler-Tree Branches in ScalerTree Struktur ein
 */
void TQClamOfflineRootSource::setupScalerTreeBranches()
{
	for(auto& S : scalerTrees)
	{
		// in der Struktur ScalerTree liegt der eingelesene scalerTree tree --> bereite Trees zum Einlesen der Daten vor (setze Branch Adressen)
		S.tree->SetBranchAddress("TDCTrigger", &S.TDCTrigger);
		S.tree->SetBranchAddress("scintillatorPMTUp", &S.scintillatorPMTUp);
		S.tree->SetBranchAddress("scintillatorPMTDown", &S.scintillatorPMTDown);
		S.tree->SetBranchAddress("cherenkovPMT", &S.cherenkovPMT);
		S.tree->SetBranchAddress("coincidenceTrigger", &S.coincidenceTrigger);
		S.tree->SetBranchAddress("acceptedTrigger", &S.acceptedTrigger);
		S.tree->SetBranchAddress("clockCounter", &S.clockCounter);
		S.tree->SetBranchAddress("livetimeCounter", &S.livetimeCounter);
		S.tree->SetBranchAddress("mbsLivetimeCounter", &S.mbsLivetimeCounter);
		S.tree->SetBranchAddress("faradayCupChargeCounter", &S.faradayCupChargeCounter);
	}
}









void TQClamOfflineRootSource::loadReferenceTree()
{
	// lade Referenz Tree --> erster Tree mit currentRunID und currentFileNumber in tdcTrees
	auto it = std::find_if(tdcTrees.begin(), tdcTrees.end(),
			[&](const auto& t) {
			return t.runID == currentRunID && t.fileNumber == currentFileNumber;
			});

	if(it == tdcTrees.end())
	{
		throw std::runtime_error("[TQClamOfflineRootSource] Finde keinen ReferenzTree.");
	}

	Tref = &(*it);
	// lade Variablen in Tref
	nEntriesRef = Tref->nEntries;


	// zu Beginn currentEntry = -1 (konstruktor)
	if(currentEntry >= 0)
	{
		Tref->tree->GetEntry(currentEntry);
		nSubEventsRef = Tref->eventNumber ? static_cast<int>(Tref->eventNumber->size()) : 0;
	}

	else
	{
		nSubEventsRef = 0;
	}

	/*
	if(reader_verbose.verbose>0)
	{
		std::cout << "Reference Tree, nEntriesRef: " << nEntriesRef << ", nSubEventsRef: " << nSubEventsRef << std::endl;
	}
	*/

}




/*
 * lade SubEvent von Referenz-Tree (referenz Tree gibt anzahl entries, subevents etc vor)
 * SubEvent ist ein SzintillatorTrigger mit allen Hits etc --> eine physikalische Elektronentrajektorie
 */
void TQClamOfflineRootSource::loadReferenceSubEvent()
{

	// Lade Entry des Referenz-TDC (tdc_0 oder kleinste ID)
	Tref->tree->GetEntry(currentEntry);

	nSubEventsRef = Tref->eventNumber ? static_cast<int>(Tref->eventNumber->size()) : 0;

	// Falls dieser Entry keine SubEvent enthaelt
	// --> springe zu naechsten Entry
	while(nSubEventsRef==0)
	{
		currentEntry++;
		currentSubEvent = 0;

		if(currentEntry >= nEntriesRef)
		{
			nSubEventsRef = 0;
			return;
//			throw std::runtime_error("[TQClamOfflineRootSource] Keine weiteren Subevents verfuegbar");
		}

		Tref->tree->GetEntry(currentEntry);
	}
}



/*
 * gehe zu naechstem SubEvent, wenn kein weiteres SubEvent, dann naechster Entry:
 *
 * Entry 0 - SubEvent 0
 * advancePosition()
 * Entry 0 - SubEvent 1
 * advancePosition()
 * Entry 1 - SubEvent 0
 * ...
 */
bool TQClamOfflineRootSource::advancePosition()
{
	if(currentEntry >= nEntriesRef)
	{
//		if(reader_verbose.verbose>0)
//		{
//			std::cout << "advacePosition: false" << std::endl;
//		}

		return false;
	}

	
	// 1. SubEvent laesst sich erhoehen --> tun und raus
	if(currentSubEvent + 1 < nSubEventsRef)
	{
		++currentSubEvent;

//		if(reader_verbose.verbose)
//		{
//			std::cout << "currentSubEvent: " << currentSubEvent << std::endl;
//		}

		return true;
	} 

	// 2. ausgewertetes SubEvent war letztes im aktuellen Entry --> currentEntry erhohen, wenn moeglich
	if(currentEntry + 1 < nEntriesRef)
	{
		++currentEntry;

		Tref->tree->GetEntry(currentEntry);
		nSubEventsRef = Tref->eventNumber ? static_cast<int>(Tref->eventNumber->size()) : 0;
		currentSubEvent = 0;

		return true;
	}

	// 3. alle Entries im file durch --> naechstes File in Run, wenn moeglich
	if(currentFilePositionInRun + 1 < filesInRun[currentRunID].size())
	{
		currentFilePositionInRun++;
		currentFileNumber = filesInRun[currentRunID][currentFilePositionInRun];
	
		currentEntry = 0;
		currentSubEvent = 0;
	
		// neues File --> neuer Referenz Tree und neue ReferenzVariablen
		loadReferenceTree();

		return true;
	}

	// 4. alle Files in aktuellen Run durch --> Run erhohen, wenn moeglich
	if(currentRunPosition + 1 < runIDs.size())
	{
		++currentRunPosition;
		currentRunID = runIDs[currentRunPosition];

		currentFilePositionInRun = 0;
		currentFileNumber = filesInRun[currentRunID][currentFilePositionInRun];

		currentEntry = 0;
		currentSubEvent = 0;

		// neues File --> neuer Referenz Tree und neue ReferenzVariablen
		loadReferenceTree();
		
		return true;
	}

	// falls das alles nicht zutrifft --> false ausgeben
	return false;


}



/*
 * RawEvent-Metadaten fuellen:
 *
 * - EventNummer (welches Event)
 * - TriggerID (identisch zu EventNummer, aber aus anderem Branch, dort triggerRequestCounter)
 * - TriggerZeit (absolute Eventzeit)
 */
void TQClamOfflineRootSource::fillEventMetadata(TQClamRawEvent& evt)
{


	if(currentSubEvent >= Tref->eventNumber->size())
	{
//		if(reader_verbose.verbose>0)
//		{
//			std::cout << "\n[TQClamOfflineRootSource] currentSubEvent groesser als eventNumber von Referenz-Tree Tref.\n" << std::endl;
//		}

		return;
	}
	if(currentSubEvent >= Tref->triggerRequestCounter->size())
	{
//		if(reader_verbose.verbose>0)
//		{
//			std::cout <<  "\n[TQClamOfflineRootSource] currentSubEvent groesser als triggerRequestCounter von Referenz-Tree Tref.\n" << std::endl;
//		}

		return;
	}
	if(currentSubEvent >= Tref->timestampInMicroSeconds->size())
	{
//		if(reader_verbose.verbose>0)
//		{
//		std::cout <<  "\n[TQClamOfflineRootSource] currentSubEvent groesser als timestampInMicroSeconds von Referenz-Tree Tref.\n" << std::endl;
//		}

		return;
	}

	evt.eventNumber = Tref->eventNumber->at(currentSubEvent);
	evt.triggerID = Tref->triggerRequestCounter->at(currentSubEvent); // TODO noch einmal ueberlegen, ob triggerRequestCounter korrekt ist

	// Zeitstempel in sek umgerechnet
	ULong64_t ts_us = Tref->timestampInMicroSeconds->at(currentSubEvent);
	evt.triggerTime = static_cast<double>(ts_us);
}



/*
 * erzeuge und fuelle RawHits und packe sie in RawEvent.
 */

void TQClamOfflineRootSource::fillTDCHits(TQClamRawEvent& evt)
{
	//int id = 0;
	for(const auto& T : tdcTrees)
	{
		if(T.runID != currentRunID || T.fileNumber != currentFileNumber) { continue; }

		T.tree->GetEntry(currentEntry);
		//std::cout << "schleife nummer: " << i << std::endl;


		if(currentSubEvent >= T.eventNumber->size())
		{

//			if(reader_verbose.verbose>0)
//			{
//				std::cout << "\n[TQClamOfflineRootSource] currentSubEvent groesser als eventNumber von aktuellem Tree T.\n" << std::endl;
//			}

			continue;
		}
		if(currentSubEvent >= T.channel->size())
		{
//			if(reader_verbose.verbose>0)
//			{
//				std::cout <<  "\n[TQClamOfflineRootSource] currentSubEvent groesser als triggerRequestCounter von aktuellem Tree T.\n" << std::endl;
//			}

			continue;
		}
		if(currentSubEvent >= T.timeRawInNanoSeconds->size())
		{
//			if(reader_verbose.verbose>0)
//			{
//				std::cout <<  "\n[TQClamOfflineRootSource] currentSubEvent groesser als timestampInMicroSeconds von aktuellem Tree T.\n" << std::endl;
//			}

			continue;
		}


		
		// wenn TDC weniger subevents hat --> nicht beitragen --> weiter (currentSubEvent startet bei 0)
		if(currentSubEvent >= T.eventNumber->size()) { continue; }

		
		const auto& ch = T.channel->at(currentSubEvent); //----------------- Kanaele (vektor)
		const auto& dt = T.timeRawInNanoSeconds->at(currentSubEvent); //---- unkorrigierte, rohe Driftzeiten (vektor)
		

		size_t nHits = std::min(ch.size(), dt.size());

		// TODO
		for(size_t i=0; i<nHits; i++)
		{
			//std::cout << "Hitnummer: " << std::to_string(i) << std::endl;
			RawHit h;
			h.tdcID = T.tdcID;
			h.channel = ch[i];
			h.rawTimeInNanoSeconds = dt[i];
			evt.hits.push_back(h);
		}
		

		//id++;
	}
}




/*
 * lese Scaler-Daten in RawScaler ein und packe RawScaler in RawEvent
 */
void TQClamOfflineRootSource::fillScalerData(TQClamRawEvent& evt)
{
	//for(size_t i=0; i<scalerTrees.size(); i++)
	int id = 0;
	for(const auto& S : scalerTrees)
	{
		if(S.runID != currentRunID || S.fileNumber != currentFileNumber) { continue; }

		if(currentEntry >= S.nEntries) { continue; }

		S.tree->GetEntry(currentEntry);

		RawScaler scaler;
		scaler.scalerID = static_cast<int>(id);

		// Zeitstempel (falls vorhanden)
		if(S.scalerTimeMicroSeconds != 0)// && !S.scalerTimeMicroSeconds->empty())
		{
			scaler.timeMicroSeconds = S.scalerTimeMicroSeconds;
		} else {
			scaler.timeMicroSeconds = evt.triggerTime;
		}

		// alle Zaehler uebernehmen
		scaler.TDCTrigger = S.TDCTrigger;
		scaler.scintillatorPMTUp = S.scintillatorPMTUp;
		scaler.scintillatorPMTDown = S.scintillatorPMTDown;
		scaler.cherenkovPMT = S.cherenkovPMT;
		scaler.coincidenceTrigger = S.coincidenceTrigger;
		scaler.acceptedTrigger = S.acceptedTrigger;
		scaler.clockCounter = S.clockCounter;
		scaler.livetimeCounter = S.livetimeCounter;
		scaler.mbsLivetimeCounter = S.mbsLivetimeCounter;
		scaler.faradayCupChargeCounter = S.faradayCupChargeCounter;

		// alle error uebernehmen
		scaler.errorTDCTrigger = S.errorTDCTrigger;
		scaler.errorScintillatorPMTUp = S.errorScintillatorPMTUp;
		scaler.errorScintillatorPMTDown = S.errorScintillatorPMTDown;
		scaler.errorCherenkovPMT = S.errorCherenkovPMT;
		scaler.errorCoincidenceTrigger = S.errorCoincidenceTrigger;
		scaler.errorAcceptedTrigger = S.errorAcceptedTrigger;
		scaler.errorClockCounter = S.errorClockCounter;
		scaler.errorLivetimeCounter = S.errorLivetimeCounter;
		scaler.errorMBSLivetimeCounter = S.errorMBSLivetimeCounter;
		scaler.errorFaradayCupChargeCounter = S.errorFaradayCupChargeCounter;

		evt.scalers.push_back(scaler);

		id++;
	}
}




Int_t TQClamOfflineRootSource::Open()
{
	// TODO
	return 0;
}


Bool_t TQClamOfflineRootSource::CheckEventClass(TClass* cl)
{
	return kTRUE;
	/*
	if(!cl) { return kFALSE; }

	if(cl == TQClamRawEvent::Class()) { return kTRUE; }

	TGo4Log::Error("TQClamOfflineRootSource: Unsupported event class %s", cl->GetName());
	return kFALSE;
	*/
}




Bool_t TQClamOfflineRootSource::BuildEvent(TGo4EventElement* dest)
{
	//TGo4Log::Info("[TQClamOfflineRootSource] BuildEvent called.");
	/** gibt es weiteres Event?
	  * & Zaehler mit advancePosition() inkrementieren */
	if(!advancePosition())
	{
		throw TGo4EventEndException(this);
		return kFALSE; /** beende EventLoop */
	}

	if(currentEntry >= nEntriesRef)
	{
		TGo4Log::Error("[TQClamOfflineRootSource] BuildEvent: Referenzzaehler ueberschritten!");
		throw TGo4EventErrorException(this);
	}

	/** Target-Evt casten */
	auto* evt = dynamic_cast<TQClamRawEvent*>(dest);

	/** Referenz-Subevent laden */
	loadReferenceSubEvent();

	/** Metadaten fuellen (eventNumber, triggerID, triggerTime */
	fillEventMetadata(*evt);

	/** TDC-Hits fuellen (Driftkammerhits, keine Unterscheidung leading/trailing) */
	fillTDCHits(*evt);

	/** Scaler-Daten fuellen (Szint Up/Down, Cherenkov, TDC-Trigger, Zaehler, Fehlerzaehler... */
	fillScalerData(*evt);

	/** Event gefuellt */
	return kTRUE;
}



Int_t TQClamOfflineRootSource::Close()
{
	// TODO
	TGo4Log::Info("[TQClamOfflineRootSource] Delete %s",GetName());
	return 0;
}
