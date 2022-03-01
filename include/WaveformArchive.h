/***********************************************************

File Name :
        WaveformWarchive.h

Original Author:
        Patrick Small

Description:


Creation Date:
        17 June 1999

Modification History:


Usage Notes:


**********************************************************/

#ifndef waveform_archive_H
#define waveform_archive_H

// Various include files
#include "Channel.h"
#include "DatabaseLimits.h"
#include "TimeWindow.h"
#include "Logfile.h"
#include "StatusManager.h"

// TEMPORARY: this goes in tndb/include/DatabaseLimits.h:
// Maximum length of a subdir name (waveform schema; subdir table)
const int DB_MAX_SUBDIRNAME_LEN = 64;

// Enumeration of the valid types of waveform archives
typedef enum {WAVE_UNKNOWN, WAVE_TRIG, WAVE_CONT} waveType;

// Definition of the maximum waveform type enumeration
const int WAVE_MAX_TYPE = 3;

// String descriptions of each waveform type
const char waveTypeStrings[WAVE_MAX_TYPE][16] = {"Unknown", "Triggered", 
						 "Continuous"};

// Database string descriptions of each waveform type
const char waveTypeDBStrings[WAVE_MAX_TYPE][DB_MAX_WAVETYPE_LEN+1] = {"U", 
								      "T", 
								      "C"};

// Enumeration of the valid types of formats
typedef enum {WAVE_FORMAT_UNKNOWN, WAVE_FORMAT_BINARY, WAVE_FORMAT_MSEED,
	      WAVE_FORMAT_PSEGY, WAVE_FORMAT_USEGY} formatType;

// Definition of the maximum format type enumeration
const int WAVE_FORMAT_MAX_TYPE = 5;

// String descriptions of each format type
const char formatTypeStrings[WAVE_FORMAT_MAX_TYPE][16] = {"Unknown", 
							  "Binary",
							  "MiniSEED",
							  "Passcal SEGY",
							  "UCB SEGY"};

// Database number descriptions of each format type
const int formatTypeDBNums[WAVE_FORMAT_MAX_TYPE] = {1, 1, 2, 3, 4};


// Enumeration of the valid types of waveform archives
typedef enum {WAVE_STATUS_EMPTY, WAVE_STATUS_TEMP, 
	      WAVE_STATUS_ARCH} statusType;

// Definition of the maximum waveform type enumeration
const int WAVE_MAX_STATUS = 3;

// String descriptions of each waveform type
const char statusTypeStrings[WAVE_MAX_STATUS][16] = {"Empty", "Temporary", 
						     "Archive"};

// Database string descriptions of each waveform type
const char statusTypeDBStrings[WAVE_MAX_STATUS][DB_MAX_WAVESTAT_LEN+1] = {"E", 
									  "T", 
									  "A"};


// Enumeration of the valid data formats
typedef enum {WAVE_DATAFORMAT_UNKNOWN, WAVE_DATAFORMAT_16BIT,
	      WAVE_DATAFORMAT_24BIT, WAVE_DATAFORMAT_32BIT,
	      WAVE_DATAFORMAT_IEEE_FPOINT, WAVE_DATAFORMAT_IEEE_DPREC,
	      WAVE_DATAFORMAT_STEIM1, WAVE_DATAFORMAT_STEIM2,
	      WAVE_DATAFORMAT_GEO_MLX24, WAVE_DATAFORMAT_GEO_MLX16_3EXP,
	      WAVE_DATAFORMAT_GEO_MLX16_4EXP, WAVE_DATAFORMAT_USNN,
	      WAVE_DATAFORMAT_CDSN, WAVE_DATAFORMAT_GRAEF,
	      WAVE_DATAFORMAT_IPG, WAVE_DATAFORMAT_SRO,
	      WAVE_DATAFORMAT_HGLP, WAVE_DATAFORMAT_DWWSSN,
	      WAVE_DATAFORMAT_RSTN} dataFormatType;

// Definition of the maximum data format enumeration
const int WAVE_MAX_DATAFORMAT = 19;

// String descriptions of each data format
const char dataFormatTypeStrings[WAVE_MAX_DATAFORMAT][32] = {"Unknown", 
							     "16b INT", 
							     "24b INT",
							     "32b INT",
							     "IEEE Float",
							     "IEEE Double",
							     "STEIM 1", 
							     "STEIM 2",
							     "GEO Mx 24b INT",
							     "GEO Mx 16b INT, 3e",
							     "GEO Mx 16b INT, 4e",
							     "US National Network",
							     "CDSN 16b",
							     "Graefenberg 16b",
							     "IPG-Strasbourg 16b",
							     "SRO", "HGLP",
							     "DWWSSN", "RSTN"};

// Database numeric descriptions of each data format
const char dataFormatTypeDBNums[WAVE_MAX_DATAFORMAT] = {0, 1, 2, 3, 4, 5, 10, 
							11, 12, 13, 14, 15, 
							16, 17, 18, 30, 31, 
							32, 33};


// Enumeration of the valid word orders
typedef enum {WAVE_WORDORDER_UNKNOWN, WAVE_WORDORDER_LITTLE_ENDIAN,
	      WAVE_WORDORDER_BIG_ENDIAN} orderType;

// Definition of the maximum word order enumeration
const int WAVE_MAX_WORDORDER = 3;

// String descriptions of each order order
const char orderTypeStrings[WAVE_MAX_DATAFORMAT][32] = {"Unknown", 
							"Little Endian", 
							"Big Endian"};

// Database numeric descriptions of each data format
const char orderTypeDBNums[WAVE_MAX_DATAFORMAT] = {-1, 0, 1};



class WaveformArchive
{
 private:

 protected:

 public:
    Channel chan;
    TimeStamp reqstart; // Original start time of cont. waveform request
    TimeWindow win;     // Actual start and end times of waveform data
    char archloc[DB_MAX_ARCHIVE_LEN+1];
    char filename[DB_MAX_FILENAME_LEN+1];
    char subdir[DB_MAX_SUBDIRNAME_LEN+1];
    unsigned long fileid;
    int filesize;
    int offset;
    int length;
    waveType wtype;
    formatType format;
    dataFormatType dataformat;
    orderType wordorder;
    int recsize;
    statusType status;
    unsigned long wfid;
    char rec_type;

    WaveformArchive();
    WaveformArchive(const WaveformArchive &w);
    ~WaveformArchive();

    WaveformArchive& operator=(const WaveformArchive &w);
};


#endif
