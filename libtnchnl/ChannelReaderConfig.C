/***********************************************************

File Name :
        ChannelReaderConfig.C

Original Author:
        Patrick Small

Description:


Creation Date:
        05 April 1999


Modification History:


Usage Notes:


**********************************************************/

//NOTE: NOT UPDATED FOR LOCATION CODE - KALPESH 04/06/2005

// Various include files
#include <iostream>
#include <string.h>
#include "seismic.h"
#include "RetCodes.h"
#include "ChannelReaderConfig.h"


ChannelReaderConfig::ChannelReaderConfig(char *chanfile) 
  : ChannelReader(chanfile)
{
}



ChannelReaderConfig::~ChannelReaderConfig()
{
}



int ChannelReaderConfig::Read(ChannelSet &cs)
{
  cout << "Error (ChannelReaderConfig::Read): Unsupported method" << endl;
  return(TN_FAILURE);
}



int ChannelReaderConfig::Read(ChannelConfigList &cl)
{
  pair<ChannelConfigList::iterator, bool> clpair;
  char line[CONFIG_MAX_STRING_LEN];
  char origline[CONFIG_MAX_STRING_LEN];
  int curline;
  Channel chan;
  ChannelConfig chaninfo;
  int retval;
  char *strpos;
  int i;

  // Parameters in channel definition file
  char network[MAX_CHARS_IN_NETWORK_STRING];
  char station[MAX_CHARS_IN_STATION_STRING];
  char channel[MAX_CHARS_IN_CHANNEL_STRING];
  char location[MAX_CHARS_IN_LOCATION_STRING];

  char configinfo[MAXSTR];

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  while (1) {
    retval = config.next(line);
    config.getLineNum(curline);
    if(retval != TN_SUCCESS) {
      break;
    } else {
      i = 0;
      strcpy(configinfo, "");
      strpos = strtok(line, " \t");
      while (strpos != NULL) {
	switch (i) {
	case 0:
	  strcpy(network, strpos);
	  break;
	case 1:
	  strcpy(station, strpos);
	  break;
	case 2:
	  strcpy(channel, strpos);
	  break;
	default:
	  if (i > 3) {
	    strcat(configinfo, " ");
	  }
	  strcat(configinfo, strpos);
	  break;
	};
	i++;
	strpos = strtok(NULL, " \t");
      }
      if (i < 2) {
	cout << 
	  "Error (ChannelReaderConfig::Read): Invalid channel entry on line "
	     << curline << endl;
        config.getFullLine(origline);
	cout << "Line: " << origline << endl;
        return(TN_FAILURE);
      }

      chan = Channel(network, station, channel);
      strcpy(chaninfo.config, configinfo);
      clpair = cl.insert(ChannelConfigList::value_type(chan, chaninfo));
      if (clpair.second != TN_TRUE) {
	cout << "Error (ChannelReaderConfig::Read): Duplicate channel " 
	     << chan << " found in channel file" << endl;
	return(TN_FAILURE);
      }
    }
  }
  if (retval != TN_EOF) {
    cout << 
      "Error (ChannelReaderConfig::Read): Error occurred reading channel file "
	 << chanfilename << ", line " << curline << endl;
    return(TN_FAILURE);
  }

  return(TN_SUCCESS);

}
