/***********************************************************

File Name :
        DataSegment.h

Original Author:
        Patrick Small

Description:


Creation Date:
        16 February 1999


Modification History:
	15 October 2004: increased DS_MAX_SIZE to 4096 from 512. PNL, UCB

Usage Notes:


**********************************************************/

#ifndef data_segment_H
#define data_segment_H

// Various include files
#include "TimeStamp.h"
#include "Duration.h"


// Maximum size of a buffer that can be stored in a data segment
const int DS_MAX_SIZE = 4096;


// Maximum size of a data segment in wire format (in bytes)
//
// Minimum:
// 8 byte start time + 4 byte numsamp + 4 bytes len + DS_MAX_SIZE bytes data
//
// When DS_MAX_SIZE was 512, DS_MAX_WIRE was 564 = 512 + 52
// No explanation was given for this number: PNL 10/15/2004

const int DS_MAX_WIRE = DS_MAX_SIZE + 52;


class DataSegment
{
 private:

 public:
    TimeStamp start;
    int numsamp;
    int len;
    char data[DS_MAX_SIZE];

    DataSegment();
    DataSegment(const char *buf);
    DataSegment(const DataSegment &ds);
    ~DataSegment();

    int Serialize(char *buf, int &buflen);

    DataSegment& operator=(const DataSegment &ds);
    friend int operator<(const DataSegment &ds1, const DataSegment &ds2);
    friend int operator==(const DataSegment &ds1, const DataSegment &ds2);
};


#endif

