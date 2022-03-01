/***********************************************************

File Name :
        Channel.C

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <cstdlib>
#include <cstring>
#include "RetCodes.h"
#include "Channel.h"
#include "nscl.h"

using namespace std;

// Structure definining the wire format of a Channel object
class wireChannel : public nscl
{
public:
  double samprate;
  double latitude;
  double longitude;
  double elevation;
  double gain;
  double mlcor;
  double mecor;
  inst_type instrument_type;
  char gain_units[MAX_CHARS_IN_GAIN_UNITS_STRING];
};


//typedef wireChannel wireChannel;


Channel::Channel()
{
  strcpy(network, "");
  strcpy(station, "");
  strcpy(channel, "");
  strcpy(location, "");
  strcpy(gain_units, "");
  instrument_type = SENSOR_UNKNOWN;
  samprate = 0.0;
  latitude = 0.0;
  longitude = 0.0;
  elevation = 0.0;
  gain = 0.0;
  mlcor = 0.0;
  mecor = 0.0;
}


Channel::Channel(const char *buf)
{
  wireChannel *w;

  w = (wireChannel *)buf;
  strncpy(network, w->network, MAX_CHARS_IN_NETWORK_STRING);
  strncpy(station, w->station, MAX_CHARS_IN_STATION_STRING);
  strncpy(channel, w->channel, MAX_CHARS_IN_CHANNEL_STRING);
  strncpy(location, mapLC((nscl*)w,MEMORY), MAX_CHARS_IN_LOCATION_STRING);
  instrument_type = w->instrument_type;
  samprate = w->samprate;
  latitude = w->latitude;
  longitude = w->longitude;
  elevation = w->elevation;
  gain = w->gain;
  mlcor = w->mlcor;
  mecor = w->mecor;
}



Channel::Channel(const Channel &c)
{
  strcpy(network, c.network);
  strcpy(station, c.station);
  strcpy(channel, c.channel);
  strcpy(location, c.location);
  strcpy(gain_units, c.gain_units);
  instrument_type= c.instrument_type;
  samprate = c.samprate;
  latitude = c.latitude;
  longitude = c.longitude;
  elevation = c.elevation;
  gain = c.gain;
  mlcor = c.mlcor;
  mecor = c.mecor;
}


Channel::Channel(const char *net, const char *sta, const char *loc, 
		 const char *chan)
{
  if (strlen(net) > MAX_CHARS_IN_NETWORK_STRING) {
	std::cerr << "Channel Constructor Fatal Error for "<< net<<"."<<sta<<"."<<chan<<"."<<loc<<std::endl;
	std::cerr << "FATAL ERROR, network string length is too big for Channel Object: "<<net<<std::endl;
	exit(1);
  }
  if (strlen(sta) > MAX_CHARS_IN_STATION_STRING) {
	std::cerr << "Channel Constructor Fatal Error for "<< net<<"."<<sta<<"."<<chan<<"."<<loc<<std::endl;
	std::cerr << "FATAL ERROR, station string length is too big for Channel Object: "<<sta<<std::endl;
	exit(1);
  }
  if (strlen(loc) > MAX_CHARS_IN_LOCATION_STRING) {
	std::cerr << "Channel Constructor Fatal Error for "<< net<<"."<<sta<<"."<<chan<<"."<<loc<<std::endl;
	std::cerr << "FATAL ERROR, location string length is too big for Channel Object: "<<loc<<std::endl;
	exit(1);
  }
  if (strlen(chan) > MAX_CHARS_IN_CHANNEL_STRING) {
	std::cerr << "Channel Constructor Fatal Error for "<< net<<"."<<sta<<"."<<chan<<"."<<loc<<std::endl;
	std::cerr << "FATAL ERROR, channel string length is too big for Channel Object: "<<chan<<std::endl;
	exit(1);
  }
  strcpy(network, net);
  strcpy(station, sta);
  strcpy(channel, chan);
  strcpy(location, loc);
  mapLC(network,location,MEMORY);

  instrument_type= SENSOR_UNKNOWN;
  samprate = 0.0;
  latitude = 0.0;
  longitude = 0.0;
  elevation = 0.0;
  gain = 0.0;
  mlcor = 0.0;
  mecor = 0.0;
}



Channel::~Channel()
{
}


int Channel::Serialize(char *buf, int &buflen)
{
  wireChannel w;

  if (buflen < sizeof(wireChannel)) {
    std::cout << "Error (Channel::Serialize): Buffer is too small" << std::endl;
    return(TN_FAILURE);
  }
  strcpy(w.network, network);
  strcpy(w.station, station);
  strcpy(w.channel, channel);
  strcpy(w.location, location);
  strcpy(w.gain_units, gain_units);
  w.instrument_type = instrument_type;
  w.samprate = samprate;
  w.latitude = latitude;
  w.longitude = longitude;
  w.elevation = elevation;
  w.gain = gain;
  w.mlcor = mlcor;
  w.mecor = mecor;
  memcpy(buf, &w, sizeof(wireChannel));
  buflen = sizeof(wireChannel);
  return(TN_SUCCESS);
}


char* Channel::getPrintableName() const{
    sprintf((char*)fullname,"%s.%s.%s.%s",network,station,channel,mapLC((nscl*)this,ASCII));
    mapLC((nscl*)this,MEMORY);
    return (char*)fullname;
}

Channel& Channel::operator=(const Channel &c)
{
  strcpy(network, c.network);
  strcpy(station, c.station);
  strcpy(channel, c.channel);
  strcpy(location, c.location);
  strcpy(gain_units, c.gain_units);
  instrument_type = c.instrument_type;
  samprate = c.samprate;
  latitude = c.latitude;
  longitude = c.longitude;
  elevation = c.elevation;
  gain = c.gain;
  mlcor = c.mlcor;
  mecor = c.mecor;
  return(*this);
}


int operator<(const Channel &c1, const Channel &c2)
{
  if (strcmp(c1.network, c2.network) < 0) {
    return(TN_TRUE);
  }
  if ((strcmp(c1.network, c2.network) == 0) && 
      (strcmp(c1.station, c2.station) < 0)) {
    return(TN_TRUE);
  }

  //I DONOT WANT TO BE BACKWARDS COMPATIBLE -KALPESH


  // NOTE: To be backwards compatible with the old channel
  // naming scheme of NET-STA-CHA, we will check to see if
  // a location code is specified before doing a compare on
  // that field.
  //  if ((strlen(c1.location) > 0) && (strlen(c2.location) > 0)) {
    // Compare the location codes

    if ((strcmp(c1.network, c2.network) == 0) && 
	(strcmp(c1.station, c2.station) == 0) &&
	(strcmp(c1.location, c2.location) < 0)) {
      return(TN_TRUE);
    }
    
    if ((strcmp(c1.network, c2.network) == 0) && 
	(strcmp(c1.station, c2.station) == 0) && 
	(strcmp(c1.location, c2.location) == 0) && 
	(strcmp(c1.channel, c2.channel) < 0)) {
      return(TN_TRUE);
    }

    //  } 

//  else {
//     // One or both location codes not specified. Compare just NET-STA-CHA

//     if ((strcmp(c1.network, c2.network) == 0) && 
// 	(strcmp(c1.station, c2.station) == 0) &&
// 	(strcmp(c1.channel, c2.channel) < 0)) {
//       return(TN_TRUE);
//     }

//  }

  return(TN_FALSE);
}


int operator==(const Channel &c1, const Channel &c2)
{

  // NOTE: To be backwards compatible with the old channel
  // naming scheme of NET-STA-CHA, we will check to see if
  // a location code is specified before doing a compare on
  // that field.
    //  if ((strlen(c1.location) > 0) && (strlen(c2.location) > 0)) {
    // Compare the location codes
    if ((strcmp(c1.network, c2.network) == 0) && 
	(strcmp(c1.station, c2.station) == 0) && 
	(strcmp(c1.channel, c2.channel) == 0) &&
	(strcmp(c1.location, c2.location) == 0)) {
      return(TN_TRUE);
    } else {
      return(TN_FALSE);
    }

//   } else {

//     // One or both location codes not specified. Compare just NET-STA-CHA
//     if ((strcmp(c1.network, c2.network) == 0) && 
// 	(strcmp(c1.station, c2.station) == 0) && 
// 	(strcmp(c1.channel, c2.channel) == 0)) {
//       return(TN_TRUE);
//     } else {
//       return(TN_FALSE);
//     }

//   }

}


// ostream& operator<<(ostream &os, const Channel &c)
// {
//   os << c.network << " " << c.station << " " << c.channel << " '" <<mapLC((nscl*)&c,ASCII)<<"'";
//     return(os);
// }

ostream& operator<<(ostream &os, const Channel &c)
{
  char location[MAX_CHARS_IN_LOCATION_STRING];
  strcpy(location, c.location);

  os << c.network << " " << c.station << " " << c.channel << " '"
     << mapLC((char*)c.network,(char*)location,ASCII) << "'";
    return(os);
}
