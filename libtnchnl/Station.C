/***********************************************************

File Name :
        Station.C

Original Author:
        Patrick Small

Description:


Creation Date:
        21 December 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <cstring>
#include "RetCodes.h"
#include "Station.h"

using namespace std;

Station::Station()
{
  strcpy(network, "");
  strcpy(station, "");
  latitude = 0.0;
  longitude = 0.0;
  elevation = 0.0;
}


Station::Station(const Station &s)
{
  strcpy(network, s.network);
  strcpy(station, s.station);
  latitude = s.latitude;
  longitude = s.longitude;
  elevation = s.elevation;
}



Station::Station(const char *net, const char *sta)
{
  strcpy(network, net);
  strcpy(station, sta);
  latitude = 0.0;
  longitude = 0.0;
  elevation = 0.0;
}



Station::~Station()
{
}


Station& Station::operator=(const Station &s)
{
 strcpy(network, s.network);
  strcpy(station, s.station);
  latitude = s.latitude;
  longitude = s.longitude;
  elevation = s.elevation;
  return(*this);
}


int operator<(const Station &s1, const Station &s2)
{
  if (strcmp(s1.network, s2.network) < 0) {
    return(TN_TRUE);
  }
  if ((strcmp(s1.network, s2.network) == 0) && 
      (strcmp(s1.station, s2.station) < 0)) {
    return(TN_TRUE);
  }

  return(TN_FALSE);
}


int operator==(const Station &s1, const Station &s2)
{
  if ((strcmp(s1.network, s2.network) == 0) && 
      (strcmp(s1.station, s2.station) == 0)) {
    return(TN_TRUE);
  } else {
    return(TN_FALSE);
  }
}


ostream& operator<<(ostream &os, const Station &s)
{
  os << s.network << " " << s.station;
  return(os);
}
