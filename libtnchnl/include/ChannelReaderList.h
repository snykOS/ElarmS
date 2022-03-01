/***********************************************************

File Name :
        ChannelReaderList.h

Original Author:
        Patrick Small

Description:


Creation Date:
        05 April 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef channel_reader_list_H
#define channel_reader_list_H

// Various include files
#include "ChannelReader.h"


class ChannelReaderList : public ChannelReader
{
 private:
    
 public:
    ChannelReaderList(char *chanfile);
    ~ChannelReaderList();

    int Read(ChannelSet &cs);
};


#endif

