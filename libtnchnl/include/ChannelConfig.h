/***********************************************************

File Name :
        ChannelConfig.h

Original Author:
        Patrick Small

Description:


Creation Date:
        14 June 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef channel_config_H
#define channel_config_H

// Various include files
#include <map>
#include <iostream>
using std::ostream;
#include "GenLimits.h"

class ChannelConfig
{
 private:

 public:
    char config[MAXSTR];

    ChannelConfig();
    ChannelConfig(const ChannelConfig &c);
    ~ChannelConfig();

    ChannelConfig& operator=(const ChannelConfig &c);
    friend ostream& operator<<(ostream &os, const ChannelConfig &c);
};


#endif

