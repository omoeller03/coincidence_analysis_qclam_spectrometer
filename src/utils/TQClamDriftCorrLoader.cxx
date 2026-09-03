#include "utils/TQClamDriftCorrLoader.h"

#include "TFile.h"
#include "TTree.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>





CorrData TQClamDriftCorrLoader::LoadSingleROOTFile(const std::string& filename)
{
        std::vector<double> phis;
        std::vector<std::vector<double>> drifttimes;
        std::vector<std::vector<double>> zOffsets;

        /// ROOT-file oeffnen
        //std::string filename = layerCorr.second;
        TFile* file = TFile::Open(filename.c_str());
        // existiert ROOT-file?
        if(!file || file->IsZombie())
        {
                throw std::runtime_error("[DriftCorrTable2D] Konnte Driftkorrektur-ROOT-File nicht oeffnen: " + filename);
        }
        // Driftkorrekturtree holen
        TTree* treeCorr = (TTree*)file->Get("DriftKorrTree");

        // einlesen vorbereiten
        double phiDeg, drifttime, zOffset;
        double sigma_phiDeg, sigma_drifttime, sigma_zOffset;
        int zSideStartDrift;

        treeCorr->SetBranchAddress("trackAngleDeg", &phiDeg);
        treeCorr->SetBranchAddress("drifttime_MinDT", &drifttime);
        treeCorr->SetBranchAddress("driftlength_Corrected", &zOffset);
        treeCorr->SetBranchAddress("drifttimeStdw_MinDT", &sigma_drifttime);
        treeCorr->SetBranchAddress("ySideStartDrift_MinDT", &zSideStartDrift);

        std::cout << "Lese simuliertes ROOT-Driftkorrektur File ein." << std::endl;

        Long64_t n = treeCorr->GetEntries();

        // -- erste innere for-schleife: --
        // sammle alle Winkel phi, um anschliessen eindeutige Winkelliste zu erzeugen
        // Tree sieht so aus:
        // *    Row   * trackAngl * drifttime * driftleng *
        // ************************************************
        // *        0 *        30 * 173.97091 * 1.1133974 *
        // *        1 *        30 * 173.57789 * 1.1109974 *
        // *        2 *        30 * 173.18488 * 1.1085974 *
        // *        3 *        30 * 172.79191 * 1.1061974 *
        // *        4 *        30 * 172.39900 * 1.1037974 *
        // ...
        // *      999 *        30 * 218.37139 * 1.2842025 *
        // Type <CR> to continue or q to quit ==>
        // *     1000 *        31 * 173.01592 * 1.1098709 *
        // *     1001 *        31 * 172.62309 * 1.1074709 *
        // *     1002 *        31 * 172.23029 * 1.1050709 *
        // *     1003 *        31 * 171.83755 * 1.1026709 *
        // *     1004 *        31 * 171.44483 * 1.1002709 *
        // *     1005 *        31 * 171.05217 * 1.0978709 *
        // ...
        // => es kommen mehrfach gleiche Winkel vor
        // => will eindeutige Winkelliste => jeder Winkel nur einmal
        std::vector<double> tmpPhi;
        tmpPhi.reserve(n);

        for(Long64_t i=0; i<n; i++)
        {
                treeCorr->GetEntry(i);
                if(zSideStartDrift<0) { continue; }
                // phi aus treeCorr in deg -> umrechnung in rad
                tmpPhi.push_back(phiDeg * M_PI/180);
        }

        // sortiere tmpPhi (eigentlich nicht notwendig... sicherheitshalbe) und entferne Duplikate
        std::sort(tmpPhi.begin(), tmpPhi.end());
        tmpPhi.erase(std::unique(tmpPhi.begin(), tmpPhi.end()), tmpPhi.end());

        size_t nPhi = tmpPhi.size();

        // -- zweite innere Schleife: --
        // fuelle phiBins und reserviere drifttimeBins und innere vektoren von zOffsetTable

        // reserve vektoren:
        phis.reserve(nPhi);
        drifttimes.reserve(nPhi);
        zOffsets.reserve(nPhi);

        // phiBins fuellen und zOffsetTable vorbereiten
        // (sollten unterschiedlich viele driftzeiten pro Winkel existieren
        for(double phi : tmpPhi)
        {
                phis.push_back(phi);
                //
                drifttimes.emplace_back();
                //
                drifttimes.back().reserve(n);
                // in zOffset[layer_index] neuen std::vector<std::vector<double>> anlegen
                zOffsets.emplace_back();
                // in zOffset[layer_index] groesse von inneren std::vector<double> reservieren
                zOffsets.back().reserve(n);
        }

        // -- dritte innere for-schleife: --
        // fuelle drifttimeBins und zOffsetTable
        for(Long64_t i=0; i<n; i++)
        {
                treeCorr->GetEntry(i);
                if(zSideStartDrift<0) { continue; }

                double phiRad = phiDeg*M_PI/180;
                // phi-index bestimmen
                auto it = std::lower_bound(tmpPhi.begin(), tmpPhi.end(), phiRad);
                size_t phi_index = std::distance(tmpPhi.begin(), it);

                drifttimes[phi_index].push_back(drifttime);
                zOffsets[phi_index].push_back(zOffset);
        }


        file->Close();
        delete file;

        CorrData data;
        data.phiBins = phis;
        data.drifttimeBins = drifttimes;
        data.zOffsetTable = zOffsets;

        return data;
}






CorrData TQClamDriftCorrLoader::LoadSingleTXTFile(const std::string& filename)
{
        CorrData data;

        // file sieht so aus:
        // * driftzeit/ns \t zOffset/mm fuer phi=25 deg \t zOffset/mm fuer phi=26 deg \t zOffset/mm fuer phi=65 deg\n
        // ****************************************************************************************************************
        // ...
        // 249.5 11.9960722867 11.9960766028 11.9960804005 ... 11.9959289480$
        // ...
        // => es kommen mehrfach gleiche Winkel vor
        // => will eindeutige Winkelliste => jeder Winkel nur einmal



        /// file oeffnen
        std::ifstream file(filename);
        // existiert file?
        if(!file)
        {
                throw std::runtime_error("[DriftCorrTable2D] Konnte Driftkorrektur-File nicht oeffnen: " + filename);
        }


	/*
	 * ALT, NUN AUCH NEUE CONFIGS, IN DENEN ICH MEHR WINKEL KALIBRIERE UND DIESE IN DIE ERSTE ZEILE SCHREIBE
	 *
        // bestimme laenge einer zeile --> anzahl winkel = laenge zeile - 1
        std::string line0;
        size_t n_phi=0;
        if(std::getline(file, line0))
        {
                std::string word;
                std::stringstream s(line0);
                while(s >> word) { n_phi++; }
                // eins abziehen, da erster wert driftzeit
                n_phi -= 1;
        }
        else
        {
                throw std::runtime_error("[DriftCorrTable2D] Konnte erste Zeile des Driftwegkorrektur txt-files nicht lesen.");
        }

        // TODO --> config?
        std::vector<double> phis;

	for(int i=0; i<n_phi; i++)
	{
		double phi_deg = 25.0 + i;
		phis.push_back(phi_deg * M_PI / 180.);
	}
	*/

	/*
	 *
	 * NEU MIT MEHR WINKELN MOEGLICH UND ERSTER ZEILE, DIE DIE GENAUEN WINKEL ANGIBT
	 *
	 */
	// erste Zeile lesen

	std::string line0;

	if(!std::getline(file, line0))
	{
		throw std::runtime_error("[DriftCorrTable2D] Konnte erste Zeile des Driftwegkorrektur txt-files nicht lesen.");
	}

	std::stringstream ss0(line0);
	std::string firstToken;
	ss0 >> firstToken;

	std::vector<double> phis;
	size_t n_phi = 0;

	// Prüfen, ob erstes Token numerisch ist.
	// Falls nein (z.B. "t(ns)"), enthält die erste Zeile die Winkel.
	char* endptr = nullptr;
	std::strtod(firstToken.c_str(), &endptr);

	bool hasHeader = (*endptr != '\0');

	if(hasHeader)
	{
	        double phi_deg;
	        while(ss0 >> phi_deg)
	        {
	                phis.push_back(phi_deg * M_PI / 180.);
	        }

	        n_phi = phis.size();

		if(n_phi == 0)
		{
			throw std::runtime_error("[DriftCorrTable2D] Header gefunden, aber keine Winkel eingelesen.");
		}
	}
	else
	{
	        // altes Format:
	        // erste Zeile ist bereits Datenzeile
	        std::stringstream tmp(line0);
	        double value;
	        while(tmp >> value) { ++n_phi; }
	
	        // erste Spalte = Driftzeit
	        --n_phi;
	
	        for(size_t i=0; i<n_phi; ++i)
	        {
	                phis.push_back((25.0 + i) * M_PI / 180.);
	        }
	}
	/*
	 * ENDE NEU
	 *
	 */





        /// hole alle werte aus tabelle: driftzeiten und zOffsets
        std::string line;
        // innerer vektor entspricht einer zeile: [driftzeit,zOffset fuer phi1, zOffset fuer phi2, ...]
        std::vector<std::vector<double>> values;

	if(hasHeader)
	{ /** nix todo, header schon gelesen */ }
	else
	{
        	// file wieder auf zeile 0 setzen.
		// ALTES FORMAT: erste zeile bereits daten
        	file.clear();
        	file.seekg(0);
	}


        while(std::getline(file,line))
        {
                std::stringstream ss(line);
                double value;

       	        values.emplace_back();
       	        while(ss >> value)
       	        {
       	                values.back().push_back(value);
       	        }
       	}

        std::vector<double> drifttimes;

        // [[driftzeit 1: zoffset_phi1, zoffset_phi2, ...], [driftzeit 2: zoffset_phi1, ...], ...]
        std::vector<std::vector<double>> zOffsets;

        for(int i=0; i<values.size(); i++)
        {
                // erster wert aus values ist driftzeit
                drifttimes.push_back(values[i][0]);

                // der rest sind die offsets der gleichen dirftzeit verschiedener phis
                zOffsets.emplace_back();
                for(int j=1; j<values[i].size(); j++)
                {
                        zOffsets.back().push_back(values[i][j]);
                }
        }


        // 1. habe phiBins:
        data.phiBins = phis;
        // 2. drifttimeBins:
        // driftzeiten in liste fuer jedes phi die gleichen
        // --> muss einfahc [drifttimes, drifttimes, ...] n_phi mal machen

        std::vector<std::vector<double>> drifttimeBins;
        for(int i=0; i<n_phi; i++)
        {
                drifttimeBins.push_back(drifttimes);
        }
        data.drifttimeBins = drifttimeBins;

        // 3. zOffsetTable:
        // ich habe zOffsets pro driftzeit fuer verschiedene winkel
        // --> [driftzeit1: [zOffset_phi1, zOffset_phi2, ...], driftzeit2: [...], ...]
        // brauche aber zOffsets pro winkel fuer verschiedene driftzeiten
        // --> muss meinen vektor transponieren

        size_t rows = zOffsets.size();
        size_t cols = zOffsets[0].size();

        std::vector<std::vector<double>> zOffsetTable(cols, std::vector<double>(rows));

        for(int i=0; i<rows; i++)
        {
                for(int j=0; j<cols; j++)
                {
                        zOffsetTable[j][i] = zOffsets[i][j];
                }
        }

        data.zOffsetTable = zOffsetTable;

        return data;
}


