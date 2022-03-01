/***********************************************************

File Name :
        ChannelReaderList.C

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
#include <iostream>
#include <string.h>
#include "seismic.h"
#include "RetCodes.h"
#include "ChannelReaderList.h"


ChannelReaderList::ChannelReaderList(char *chanfile) : ChannelReader(chanfile)
{
}



ChannelReaderList::~ChannelReaderList()
{
}


int ChannelReaderList::Read(ChannelSet &cs)
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

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  while (1) {
    retval = config.next(line);
    config.getLineNum(curline);
    if(retval != TN_SUCCESS) {
      break;
    } else {
      retval = sscanf(line, "%s %s %s", network, station, channel);
      if (retval != 3) {
	std::cout << 
	  "Error (ChannelReaderList::Read): Invalid channel entry on line "
	     << curline << std::endl;
        config.getFullLine(origline);
	std::cout << "Line: " << origline << std::endl;
        return(TN_FAILURE);
      }
      chan = Channel(network, station, channel);
      cspair = cs.insert(chan);
      if (cspair.second != TN_TRUE) {
	std::cout << "Error (ChannelReaderList::Read): Duplicate channel " << chan 
	     << " found in channel file" << std::endl;
	return(TN_FAILURE);
      }
    }
  }
  if (retval != TN_EOF) {
    std::cout << 
      "Error (ChannelReaderList::Read): Error occurred reading channel file "
	 << chanfilename << ", line " << curline << std::endl;
    return(TN_FAILURE);
  }

  return(TN_SUCCESS);
}

