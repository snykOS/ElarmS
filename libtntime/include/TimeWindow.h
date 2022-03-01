/***********************************************************

File Name :
        TimeWindow.h

Original Author:
        Pete Lombard

Description:
	Represents an arbitrary half-open time interval by its
   	start time and end time. The represented interval starts 
	at the start time and runs up to but not including the end time.

Creation Date:
        15 October 2007


Modification History:


Usage Notes:


**********************************************************/

#ifndef time_window_H
#define time_window_H

#include <iostream>
#include "TimeStamp.h"
#include "Duration.h"

// Maximum size of a time window in wire format (in bytes)
//
// Minimum:
// 8 byte start time + 8 byte end time
//
const int TW_MAX_WIRE = 32;

class TimeWindow
{
    friend ostream& operator<<(ostream &os, const TimeWindow &tw);

 public:
    TimeStamp start;
    TimeStamp end;

    TimeWindow(const char *buf);
    TimeWindow(const TimeWindow &tw);
    TimeWindow(const TimeStamp &s, const TimeStamp &e);
    TimeWindow(const TimeStamp &s, const Duration &dur);
    TimeWindow();
    ~TimeWindow();

    int Serialize(char *buf, int &buflen) const;
    bool Overlap(const TimeWindow &tw) const;

    TimeWindow& operator=(const TimeWindow &tw);
    bool operator==(const TimeWindow &tw) const;
    bool operator<(const TimeWindow &tw) const;
    operator Duration() const;
};

#endif
