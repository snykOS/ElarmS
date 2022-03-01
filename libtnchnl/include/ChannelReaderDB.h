/***********************************************************

File Name :
        ChannelReaderDB.h

Original Author:
        Patrick Small

Description:


Creation Date:
        14 June 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef channel_reader_db_H
#define channel_reader_db_H

// Various include files
#include "Channel.h"
#include "ChannelConfig.h"
#include "ChannelReader.h"
#include "Database.h"

// Definition of a list of a configured channels
typedef std::map<Channel, ChannelConfig, std::less<Channel>, std::allocator<std::pair<const Channel, ChannelConfig> > > ChannelConfigList;

class ChannelReaderDB : public Database
{
 private:
    
 public:
    ChannelReaderDB();
    ChannelReaderDB(const char *dbs, const char *dbu, const char *dbp);
    ~ChannelReaderDB();

    // Updated channel specification: NET-STA-CHAN-LOC 
    int GetChannels(const char *progname, ChannelConfigList &cl);
    int GetActiveChannels(ChannelSet &cl);

    // New channel specification: NET-STA-LOC-CHAN -..... SHOULD NOT BE USED -Kalpesh
    //    int GetChannels4Comp(const char *progname, ChannelConfigList &cl);
    //    int GetActiveChannels4Comp(ChannelSet &cl);
};


#endif

