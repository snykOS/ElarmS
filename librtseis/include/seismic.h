/***********************************************************

File Name :
	seismic.h

Programmer:
	Phil Maechling

Description:
	These are seismic network related constants and
	definitions. Only seismic related code should have
	to include this file.

Creation Date:
	10 September 1996


Usage Notes:



Modification History:



**********************************************************/

#ifndef seismic_H
#define seismic_H


// Enumeration describing the possible types of phases
//
enum phaseType {PHASE_UNKNOWN, PHASE_P, PHASE_S, PHASE_SKS, PHASE_PCP};
 
typedef enum phaseType phaseType;

// String identifier of a P phase pick
//
const char P_PHASE_PICK_IDENTIFIER[] = "P";

// String identifier of an S phase pick
//
const char S_PHASE_PICK_IDENTIFIER[] = "S";



// String identifier of a Wood-Anderson photographic amplitude
//
const char WOOD_ANDERSON_PHOTO_IDENTIFIER[] = "WA";

// String identifier of a Wood-Anderson synthetic amplitude
//
const char WOOD_ANDERSON_SYNTH_IDENTIFIER[] = "WAS";

// String identifier of a Peak Ground acceleration
//
const char PEAK_GROUND_ACCEL_IDENTIFIER[] = "PGA";

// String identifier of a Peak Ground velocity
//
const char PEAK_GROUND_VEL_IDENTIFIER[] = "PGV";

// String identifier of a Peak Ground displacement
//
const char PEAK_GROUND_DISP_IDENTIFIER[] = "PGD";

// String identifier of a Wood-Anderson corrected amplitude
//
const char WOOD_ANDERSON_CORR_IDENTIFIER[] = "WAC";

// String identifier of a Wood-Anderson uncorrected amplitude
//
const char WOOD_ANDERSON_UNCORR_IDENTIFIER[] = "WAU";

// String identifier of an integral of velocity squared value
//
const char INT_VELOCITY_SQUARED_IDENTIFIER[] = "IV2";

// String identifier of a 0.3 Spectral Peak value
//
const char SPECTRAL_PEAK_03_IDENTIFIER[] = "SP.3";

// String identifier of a 1.0 Spectral Peak value
//
const char SPECTRAL_PEAK_10_IDENTIFIER[] = "SP1.0";

// String identifier of a 3.0 Spectral Peak value
//
const char SPECTRAL_PEAK_30_IDENTIFIER[] = "SP3.0";

// String identifier of a Local Magnitude identifier
//
const char ML_100_IDENTIFIER[] = "ML100";

// String identifier of an Energy Magnitude identifier
//
const char ME_100_IDENTIFIER[] = "ME100";

// String identifier of an Energy value
//
const char ENERGY_IDENTIFIER[] = "EGY";


// Size of a SEED Telemetry Packet
//
const int BYTES_IN_SEED_TELEMETRY_BLOCK = 512;

// Maximum number of channels in a network
//
const int MAX_CHANNELS_IN_NETWORK     =  2048;

// Maximum number of channels in the analog network
// 
const int MAX_CHANNELS_IN_ANALOG_NETWORK = 400;
const int MAX_TRACEBUFS_PER_SEND = 100;
const int MAX_SAMPLES_PER_TRACEBUF = 43;
const int MAX_SAMPLES_IN_ANALOG_CHANNEL_BUFFER =
	   (MAX_SAMPLES_PER_TRACEBUF * MAX_TRACEBUFS_PER_SEND);

// Maximum number of networks
//
const int MAX_NETWORKS                = 10;

// Maximum number of characters in a network type identifier
//
const int MAX_CHARS_IN_NETTYPE_STRING = 10;

// Maximum number of characters in a network identifier
//
const int MAX_CHARS_IN_SUBNET_STRING = 10;

// Maximum number of characters in a processing source identifier
//
const int MAX_CHARS_IN_SOURCE_STRING  = 10;

// Maximum number of characters in a pick type name
//
const int MAX_CHARS_IN_PICKTYPE_STRING  = 10;

// Maximum number of characters in an event/pick remark string
//
const int MAX_CHARS_IN_REMARK_STRING  = 32;

// Maximum number of characters in a first motion string
//
const int MAX_CHARS_IN_FIRSTMO_STRING  = 10;

// Maximum number of characters in a magnitude type string
//
const int MAX_CHARS_IN_MAGTYPE_STRING = 10;

// Maximum number of characters in a "where" string
//
const int MAX_CHARS_IN_WHERE_STRING = 81;

// Maximum number of characters in an amplitude type string
//
const int MAX_CHARS_IN_AMPTYPE_STRING = 10;

// Maximum number of characters in an amplitude type string
//
const int MAX_CHARS_IN_AMP_UNITS_STRING = 10;

// Maximum number of characters in an instrument type string
//
const int MAX_CHARS_IN_INSTTYPE_STRING = 20;


// Maximum number of characters in a filename
//
// NOTE: Should not be in this header file
//
const int MAX_CHARS_IN_FILE_NAME = 256;

// Maximum number of characters in a channel header
//
const int CHARS_IN_CHANNEL_HEADER_STRING = 8;

// Maximum number of characters in a monitor string
//
const int MAX_CHARS_IN_MONITOR_STRING = 40;


// Maximum number of characters in a text file line
//
// NOTE: Should not be in this header file
//
const int MAX_CHARS_IN_FILE_LINE = 120;

// Maximum number of characters in a multicast group identifier
//
// NOTE: Should not be in this header file
//
const int MAX_CHARS_IN_MCAST_GROUP = 50;

// Maximum number of characters in a multicast port identifier
//
// NOTE: Should not be in this header file
//
const int MAX_CHARS_IN_MCAST_PORT  = 25;


const int PROPOGATION_TIME_ACROSS_NETWORK = 90;
const int TELEMETRY_DELAY = 20;
const int CURRENT_CENTURY = 1900;
const int MAXIMUM_NUMBER_OF_AMPS_RETURNED = 300;

const int BROADBAND     = 0;
const int STRONG_MOTION = 1;


//
// Define data types used in richter
//
const int WOOD_ANDERSON = 0;
const int VELOCITY      = 1;
const int ACCELERATION  = 2;

const int SEISMIC_LENGTH_OF_PAGE = 80;

//
// Most of the following settings should be moved to GenLimits.h
// 

// The following cannot be used with leap seconds: the lenght of a day 
// or year is NOT constant with leap seconds
//const int SECONDS_IN_DAY =  86400;
//const int SECONDS_IN_YEAR =  31536000;

const int USECS_PER_SECOND = 1000000;
const int TICKS_PER_SECOND = 10000;
 
const int MAX_INTEGER_VALUE = 2147483647;
const int MIN_INTEGER_VALUE = -2147483647;
 
const int ALL_DATA_MODE = 0;
const int NEW_DATA_ONLY_MODE = 1;

//
//
//

const int CONTIGUOUS_DATA_MODE   =0;
const int ALL_AVAILABLE_DATA_MODE =1;

const int CONVERT_ML100_TO_ML = 0;
const int CONVERT_ME100_TO_ME = 1;

#define MAX_CHARS_IN_GAIN_UNITS_STRING 20


#endif
