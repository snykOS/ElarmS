/***********************************************************

File Name :
        ChannelReader.h

Original Author:
        Patrick Small

Description:


Creation Date:
        05 April 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef channel_reader_H
#define channel_reader_H

// Various include files
#include <set>
#include "Channel.h"
#include "Configuration.h"


// Definition of the less-than comparator for Channel objects
typedef std::less<Channel> lessChan;


// Definition of a set of channels
typedef std::set<Channel, lessChan, std::allocator<Channel> > ChannelSet;


class ChannelReader
{
 private:

 protected:
    int valid;
    Configuration config;
    char chanfilename[FILENAME_MAX];
    
 public:
    ChannelReader(char *chanfile);
    ~ChannelReader();

    virtual int Read(ChannelSet &cs);

    friend int operator!(const ChannelReader &cr);
};


#endif

