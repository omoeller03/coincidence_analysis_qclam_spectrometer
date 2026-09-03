#ifndef TQCLAMCONSTANTS_H
#define TQCLAMCONSTANTS_H

#include "Rtypes.h"


#define SYNC_BY_MBS_TRIGGER true
#define USE_COUPLED_MBS_SYSTEM true
#define DEBUG_MBS_HEADERS false
#define DEBUG_EVENT_SYNCHRONIZATION false
#if USE_COUPLED_MBS_SYSTEM
    const size_t nDataStreams = 1;  // New coupled system: single MBS client
#else
    const size_t nDataStreams = 3;  // Legacy system: three separate MBS clients
#endif
const size_t MAX_MBS_CLIENTS = 3;
#define NEW_FILENAME_FORMAT true // parse run files with new format c_RUNNR_000i.lmd. otherwise, use historic format q_RUNNR.lmd


/** Clock-Periode QCLAM --> zur Raten-Berechnung!!! */
static constexpr double CLOCK_PERIOD_QCLAM = 2.4e-7; // sec per tick
/** Clock-FRQUENCY QCLAM --> zur Raten-Berechnung!!! */
static constexpr double CLOCK_FREQUENCY_DAGOBERT = 1.e7; // 10 MHz
static constexpr double CYCLE_LENGTH_DAGOBERT = 4.; // length of cycle = 4 ns

/** Dagobert Module/Kanaele */
static constexpr size_t N_MODULES_DAGOBERT = 2; // 2 x SIS Module
static constexpr size_t N_CHANNELS_DAGOBERT = 16; // 16 x Kanaele je SIS-Modul


/** max anzahl files */
static constexpr size_t QCLAM_MAX_FILES = 1000;

/** Maximale Dimensionen von Arrays Double[QCLAM_MAX_TDC] ... */
static constexpr size_t QCLAM_MAX_TDC = 32;
static constexpr size_t QCLAM_MAX_CH = 128;
static constexpr size_t QCLAM_MAX_LAYER = 3;//8;
static constexpr size_t QCLAM_MAX_WIRE = 512;

/** Umgang mit Conditions */
static constexpr size_t QCLAM_COND_OFF = 0; // nutze Condition nicht
static constexpr size_t QCLAM_COND_FLAG_ONLY = 1; // setze Flag accepted (true/false), processe weiter
static constexpr size_t QCLAM_COND_FILTER = 2; // filter alles ausserhalb Condition

/** Maximale Anzahl Subevents pro Event --> wieviele Tdc-Trigger pro Event */
static constexpr size_t QCLAM_MAX_SUBEVENTS = 20;

/** Source-Typen */
static constexpr size_t QCLAM_OFFLINE_ROOT = 0; /** --> TQClamOfflineRootSource */
static constexpr size_t QCLAM_OFFLINE_LMD = 1; /** --> TQClamOfflineLmdSource */
static constexpr size_t QCLAM_MBS_STREAM = 2; /** --> MbsStream im QCLAM_SINGLE Modus */

static constexpr size_t QCLAM_DAGOBERT_OFFLINE_LMD = 3; /** --> TQClamOfflineLmdSource */
static constexpr size_t QCLAM_DAGOBERT_MBS_STREAM = 4; /** MbsStream im QCLAM_DAGOBERT Modus */

static constexpr size_t QCLAM_OFFLINE_ROOT_DEBUG = 10; /** --> TQClamOfflineRootSource */
static constexpr size_t QCLAM_OFFLINE_LMD_DEBUG = 11; /** --> TQClamOfflineLmdSource */

/** Konstanten fuer Korrektur-Tabellen Grid-Groesse */
/* nicht zu gross machen:
 * Double_t -> 8 Byte
 * 2 x Tabellen mit je QCLAM_CORR_..._PHI_BINS * QCLAM_..._DRIFTTIME_BINS vielen eintraegen
 * Bsp.: beide 1000 eintraege
 *
 * --> speicherplatz = 8 Byte * 2 * (1000 * 1000) = 16 MB */
static constexpr size_t QCLAM_CORR_TABLE_MAX_PHI_BINS = 1e3;
static constexpr size_t QCLAM_CORR_TABLE_MAX_DRIFTTIME_BINS= 1e3;

static constexpr size_t DAGOBERT_NUMBER_DETECTORS = 10; /** 9 x in Tuermen, 1 x Vorwaertswinkel */

#endif
