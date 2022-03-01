/***********************************************************

File Name :
        ChannelReaderTridefs.C

Original Author:
        Patrick Small

Description:


Creation Date:
        05 April 1999


Modification History:


Usage Notes:


**********************************************************/
//NOT UPDATED FOR LOCATION CODES -Kalpesh 04.06.2005

// Various include files
//#include <stream.h>
#include <string.h>
#include "seismic.h"
#include "RetCodes.h"
#include "ChannelReaderTridefs.h"


ChannelReaderTridefs::ChannelReaderTridefs(char *chanfile) 
  : ChannelReader(chanfile)
{
}



ChannelReaderTridefs::~ChannelReaderTridefs()
{
}


int ChannelReaderTridefs::Read(ChannelSet &cs)
{
  std::pair<ChannelSet::iterator, bool> cspair;
  char line[CONFIG_MAX_STRING_LEN];
  char origline[CONFIG_MAX_STRING_LEN];
  int curline;
  Channel chan;
  int retval;

  // Parameters in channel definition file
  char network[MAX_CHARS_IN_NETWORK_STRING];
  char station[MAX_CHARS_IN_STATION_STRING];
  char channel[MAX_CHARS_IN_CHANNEL_STRING];
  char telemetry_type;
  double latitude, longitude, elevation;
  double sample_period;
  int calibrated;
  double gain_factor, mlcor, mecor;

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  while (1) {
    retval = config.next(line);
    config.getLineNum(curline);
    if(retval != TN_SUCCESS) {
      break;
    } else {
      retval = sscanf(line, "%s %s %s %c %lf %lf %lf %lf %lf %lf %lf %d", 
		      network, station, channel, &telemetry_type, 
		      &latitude, &longitude, &elevation, 
		      &sample_period, &gain_factor, &mlcor, 
		      &mecor, &calibrated);
      if (retval != 12) {
	std::cout << 
	  "Error (ChannelReaderTridefs::Read): Invalid channel entry on line "
	     << curline << std::endl;
        config.getFullLine(origline);
	std::cout << "Line: " << origline << std::endl;
        return(TN_FAILURE);
      }
      chan = Channel(network, station, channel);
      chan.teltype = telemetry_type;
      chan.samprate = (1.0 / sample_period);
      chan.latitude = latitude;
      chan.longitude = longitude;
      chan.elevation = elevation;
      chan.gain = gain_factor;
      chan.mlcor = mlcor;
      chan.mecor = mecor;
      cspair = cs.insert(chan);
      if (cspair.second != TN_TRUE) {
	std::cout << 
	  "Error (ChannelReaderTridefs::Read): Duplicate channel " << chan 
	     << " found in channel file" << std::endl;
	return(TN_FAILURE);
      }
    }
  }
  if (retval != TN_EOF) {
    std::cout << "Error (ChannelReaderTridefs::Read): Error occurred reading channel file " << chanfilename << ", line " << curline << std::endl;
    return(TN_FAILURE);
  }

  return(TN_SUCCESS);
}

