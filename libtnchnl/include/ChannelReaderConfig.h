/***********************************************************

File Name :
        ChannelReaderConfig.h

Original Author:
        Patrick Small

Description:


Creation Date:
        23 June 1999


Modification History:


Usage Notes:


**********************************************************/

//NOTE: NOT UPDATED FOR LOCATION CODE -LOOKS LIKE THIS FILE IS A RELIC- KALPESH 04/06/2005


#ifndef channel_reader_config_H
#define channel_reader_config_H

#ifdef 0 
// Various include files
#include <map>
#include "ChannelReader.h"
#include "ChannelConfig.h"

using namespace std;

// Definition of a list of a configured channels
typedef std::map<Channel, ChannelConfig, std::less<Channel>, 
    std::allocator<std::pair<const Channel, ChannelConfig> > > ChannelConfigList;


class ChannelReaderConfig : public ChannelReader
{
 private:
    
 public:
    ChannelReaderConfig(char *chanfile);
    ~ChannelReaderConfig();

    int Read(ChannelSet &cs);
    int Read(ChannelConfigList &cl);
};

#endif

#endif

