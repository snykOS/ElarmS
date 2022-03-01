/***********************************************************

File Name :
        DatabaseLimits.h

Original Author:
        Patrick Small

Description:


Creation Date:
        17 June 1999

Modification History:


Usage Notes:


**********************************************************/

#ifndef database_limits_H
#define database_limits_H


// Maximum length of the program name field
const int DB_MAX_NAME_LEN = 16;

// Maximum length of the auth field
const int DB_MAX_AUTH_LEN = 15;

// Maximum length of the subsource field
const int DB_MAX_SUBSOURCE_LEN = 8;

// Maximum length of the event type field
const int DB_MAX_EVENTTYPE_LEN = 7;

// Maximum length of the geographic type field
const int DB_MAX_GEOGTYPE_LEN = 1;

// Maximum length of the mag type field
const int DB_MAX_MAGTYPE_LEN = 6;

// Maximum length of the amp type field
const int DB_MAX_AMPTYPE_LEN = 8;

// Maximum length of the arrival type field
const int DB_MAX_ARRTYPE_LEN = 8;

// Maximum length of the unit type field
const int DB_MAX_UNITTYPE_LEN = 4;

// Maximum length of the filename field
const int DB_MAX_FILENAME_LEN = 32;

// Maximum length of the archive name field
const int DB_MAX_ARCHIVE_LEN = 8;

// Maximum length of the wave type field
const int DB_MAX_WAVETYPE_LEN = 1;

// Maximum length of the wave status field
const int DB_MAX_WAVESTAT_LEN = 1;

// Maximum length of the channelsrc field
const int DB_MAX_CHANSRC_LEN = 8;

// Maximum length of the channelsrc field
const int DB_MAX_SEEDCHAN_LEN = 3;

// Maximum length of the local evend id field
const int DB_MAX_LOCEVID_LEN = 24;

// Maximum length of a comment field
const int DB_MAX_COMMENT_LEN = 80;

// Maximum length of an onscale field
const int DB_MAX_ONSCALE_LEN = 2;

// Maximum length of the waveform request type field
const int DB_MAX_REQTYPE_LEN = 1;

// Maximum length of the rflag (human/automatic/finalized) field
const int DB_MAX_RFLAG_LEN = 1;

// Maximum length of the cflag (clipped flag) field
const int DB_MAX_CFLAG_LEN = 2;

// Maximum length of the channel configuration field
const int DB_MAX_CONFIG_LEN = 64;

// Maximum length of the first motion field
const int DB_MAX_FIRSTMO_LEN = 2;

// Maximum length of the iphase field
const int DB_MAX_IPHASE_LEN = 8;

// Maximum length of a channel location field
const int DB_MAX_LOCATION_LEN = 2;

// Maximum length of an alarm action string field
const int DB_MAX_ALARM_ACTION_LEN = 15;

// Maximum length of an action state field
const int DB_MAX_ACTION_STATE_LEN = 15;

// Maximum length of an action state field
const int DB_MAX_PRIMARY_SYSTEM_LEN = 5;

// Maximum length of a system/peer status field
const int DB_MAX_SYSTEM_STATUS_LEN = 15;

// Maximum length of the algorithm field
const int DB_MAX_ALGORITHM_LEN = 15;

// Maximum length of the location type field
const int DB_MAX_TYPE_LEN = 2;

// Maximum length of the fixed depth field
const int DB_MAX_FDEPTH_LEN = 1;

// Maximum length of the fixed epicenter field
const int DB_MAX_FEPI_LEN = 1;

// Maximum length of the fixed time field
const int DB_MAX_FTIME_LEN = 1;

// Maximum length of the signal onset quality field
const int DB_MAX_QUAL_LEN = 1;

// Maximum length of the trigger type flag field
const int DB_MAX_TRIGFLAG_LEN = 1;

// Maximum length of the gain units field
const int  DB_MAX_GAIN_UNITS_LEN = 20;

// Maximum length of nettrig.subnet_code field
const int  DB_MAX_SUBNET_CODE_LEN = 20;

#endif
