/***********************************************************

File Name :
        Waveform.h

Original Author:
        Patrick Small

Description:


Creation Date:
        12 March 1999


Modification History:


Usage Notes:


**********************************************************/

#ifndef waveform_H
#define waveform_H

// Various include files
#include <list>
#include "TimeWindow.h"
#include "DataArray.h"
#include "DataSegment.h"
#include "DataCounts.h"
#include "Channel.h"
#include "MiniSEED.h"
#include "qlib2.h"


// Definition of a DataSegmentList
//
typedef std::list<DataSegment, std::allocator<DataSegment> > DataSegmentList;


// Definition of a DataCountList
//
typedef std::list<DataCounts, std::allocator<DataCounts> > DataCountList;

// Definition of a DataArrayList
//
typedef std::list<DataArray, std::allocator<DataArray> > DataArrayList;

class Waveform
{

 private:
    int valid;
    Channel chan;
    DataSegmentList dslist;
    DataSegmentList::iterator nextseg;
    int recsize;
    int format;

    int _InsertCounts(DataCountList &dl, DataCounts &dc);
    int _TrimPacket(DataSegment &ds, DataSegment &dsprev,
		    const TimeWindow &tw);
    int _GetSamples(DataSegment &ds, DataCounts &dc);
    int _BuildRate(double drate, int &rate, int &rate_mult);

 public:
    Waveform();
    Waveform(const Waveform &w);
    Waveform(const DataSegmentList &dl);
    Waveform(const DataArrayList &dl, Channel &chan, int data_fmt, 
	     int mseed_blksize);
    ~Waveform();

    int GetSEED(DataSegmentList &dl, int rsize = MINISEED_REC_ORIGINAL,
		int dform = MINISEED_FORMAT_ORIGINAL);
    int GetCounts(DataCountList &dl);
    double GetSampleRate();

    int getSizeInBytes();
    int NumberOfSegments();
    int SetFirstSegment();
    int Next(Waveform &w);
    int Trim(const TimeWindow &tw);
    int StartTime(TimeStamp &starttime);
    int EndTime(TimeStamp &endtime);

    Waveform& operator=(const Waveform &w);
    friend int operator!(const Waveform &w);
};


#endif
