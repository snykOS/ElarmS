#ifndef nscl_h
#define nscl_h

#include "qlib2.h"

static int BINARY   = 0;
static int MEMORY   = 1;
static int ASCII    = 2;
static int FILENAME = 3;
static int DATABASE = 4;

#ifdef __cplusplus

// Maximum number of characters in a network identifier
//
const int MAX_CHARS_IN_NETWORK_STRING = DH_NETWORK_LEN+1;//10;

// Maximum number of characters in a station identifier
//
const int MAX_CHARS_IN_STATION_STRING = DH_STATION_LEN+1;//10;

// Maximum number of characters in a channel identifier
//
const int MAX_CHARS_IN_CHANNEL_STRING = DH_CHANNEL_LEN+1;//10;

// Maximum number of characters in a location identifier
//
const int MAX_CHARS_IN_LOCATION_STRING = DH_LOCATION_LEN+1;//10;


class nscl{
 public:
    char network[MAX_CHARS_IN_NETWORK_STRING];
    char station[MAX_CHARS_IN_STATION_STRING];
    char channel[MAX_CHARS_IN_CHANNEL_STRING];
    char location[MAX_CHARS_IN_LOCATION_STRING];
};

#endif

#ifdef __cplusplus
extern "C"{
    char* mapLC(char* network, char* lc, const int d);
}
char* mapLC(nscl*, const int d);

#else
char* mapLC(char* network, char* lc, const int d);
#endif



#endif
