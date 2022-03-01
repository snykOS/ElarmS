/***********************************************************

File Name :
        ChannelConfig.C

Original Author:
        Patrick Small

Description:


Creation Date:
        14 June 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <cstring>
#include "RetCodes.h"
#include "ChannelConfig.h"

using namespace std;

ChannelConfig::ChannelConfig()
{
  strcpy(config, "");
}


ChannelConfig::ChannelConfig(const ChannelConfig &c)
{
  strcpy(config, c.config);
}


ChannelConfig::~ChannelConfig()
{
}


ChannelConfig& ChannelConfig::operator=(const ChannelConfig &c)
{
  strcpy(config, c.config);	
  return(*this);
}


ostream& operator<<(ostream &os, const ChannelConfig &c)
{
  os << c.config;
  return(os);
}
