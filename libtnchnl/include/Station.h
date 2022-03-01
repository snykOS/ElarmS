/***********************************************************

File Name :
        Station.h

Original Author:
        Patrick Small

Description:


Creation Date:
        21 December 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef station_H
#define station_H

// Various include files
#include <iostream>
using std::ostream;
#include "seismic.h"
#include "nscl.h"


class Station
{
 private:

 public:
    char network[MAX_CHARS_IN_NETWORK_STRING];
    char station[MAX_CHARS_IN_STATION_STRING];
    double latitude;
    double longitude;
    double elevation;

    Station();
    Station(const Station &s);
    Station(const char *net, const char *sta);
    ~Station();

    Station& operator=(const Station &s);
    friend int operator<(const Station &s1, const Station &s2);
    friend int operator==(const Station &s1, const Station &s2);
    friend ostream& operator<<(ostream &os, const Station &s);
};


#endif

