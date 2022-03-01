/***********************************************************

File Name :
        ChannelReader.C

Original Author:
        Patrick Small

Description:


Creation Date:
        05 April 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <iostream>
#include <cstring>
#include "seismic.h"
#include "RetCodes.h"
#include "ChannelReader.h"


ChannelReader::ChannelReader(char *chanfile)
{
  valid = TN_FALSE;

  config = Configuration(chanfile);
  if (!config) {
    std::cout << 
      "Error (ChannelReader::ChannelReader) Unable to open configuration file"
	 << std::endl;
    return;
  }

  std::strcpy(chanfilename, chanfile);
  valid = TN_TRUE;
}



ChannelReader::~ChannelReader()
{
  valid = TN_FALSE;
}


int ChannelReader::Read(ChannelSet &cs)
{
  return(TN_FAILURE);
}



int operator!(const ChannelReader &cr)
{
  return(!(cr.valid));
}
