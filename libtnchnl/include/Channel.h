/***********************************************************

File Name :
        Channel.h

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef channel_H
#define channel_H

// Various include files
#include <iostream>
#include "seismic.h"
#include "nscl.h"


using std::ostream;
using std::istream;

// Maximum size of a channel in wire format (in bytes)
//
// Minumum:
//
const int CHANNEL_MAX_WIRE = 128;

typedef enum {SENSOR_BROADBAND, SENSOR_STRONG_MOTION, SENSOR_SHORT_PERIOD, SENSOR_UNKNOWN} inst_type;

class Channel : public nscl
{
 private:
    char fullname[MAX_CHARS_IN_NETWORK_STRING +
		  MAX_CHARS_IN_STATION_STRING +
		  MAX_CHARS_IN_CHANNEL_STRING +
		  MAX_CHARS_IN_LOCATION_STRING];

 public:
    inst_type instrument_type;
    char gain_units[20];

    double samprate;
    double latitude;
    double longitude;
    double elevation;
    double gain;
    double mlcor;
    double mecor;

    Channel();
    Channel(const char *buf);
    Channel(const Channel &c);
    //Channel(const char *net, const char *sta, const char *chan);
    Channel(const char *net, const char *sta, const char *loc, 
	    const char *chan);
    ~Channel();

    int Serialize(char *buf, int &buflen);
    char* getPrintableName() const;

    
    Channel& operator=(const Channel &c);
    friend int operator<(const Channel &c1, const Channel &c2);
    friend int operator==(const Channel &c1, const Channel &c2);
    friend ostream& operator<<(ostream &os, const Channel &c);
};


#endif

