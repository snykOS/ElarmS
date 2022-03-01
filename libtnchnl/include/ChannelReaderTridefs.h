/***********************************************************

File Name :
        ChannelReaderTridefs.h

Original Author:
        Patrick Small

Description:


Creation Date:
        05 May 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef channel_reader_tridefs_H
#define channel_reader_tridefs_H

// Various include files
#include "ChannelReader.h"


class ChannelReaderTridefs : public ChannelReader
{
 private:
    
 public:
    ChannelReaderTridefs(char *chanfile);
    ~ChannelReaderTridefs();

    int Read(ChannelSet &cs);
};


#endif

