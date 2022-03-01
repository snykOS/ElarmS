/*************************************************************

  Filename:
	TimeWindow.C

  Description:
	Represents an arbitrary half-open time interval by its
   	start time and end time. The represented interval starts 
	at the start time and runs up to but not including the end time.

  Programmer:
	Pete Lombard

  Creation Date:
	15 October 2007

  Modification Date:


**************************************************************/

#include <cstring>
#include "Compat.h"
#include "TimeWindow.h"
#include "ByteSwap.h"
#include "RetCodes.h"

struct wireTimeWindow
{
    double start;
    double end;
};


typedef wireTimeWindow wireTimeWindow;


TimeWindow::TimeWindow(const char *buf)
{
    wireTimeWindow w;
    
    // Copy into wireTimeWindow structure to avoid data alignment problems
    std::memcpy(&w, buf, sizeof(wireTimeWindow));
    
    start = TimeStamp(TRUE_EPOCH_TIME, ntohd(w.start));
    end = TimeStamp(TRUE_EPOCH_TIME, ntohd(w.end));
}



TimeWindow::TimeWindow(const TimeWindow &tw)
{
  start = tw.start;
  end = tw.end;
}



TimeWindow::TimeWindow(const TimeStamp &s, const TimeStamp &e)
{
  start = s;
  end = e;
}

TimeWindow::TimeWindow(const TimeStamp &s, const Duration &dur)
{
    start = s;
    end = s + dur;
}


TimeWindow::TimeWindow()
{
}


TimeWindow::~TimeWindow()
{
}

int TimeWindow::Serialize(char *buf, int &buflen) const
{
    wireTimeWindow w;
    
    if ((unsigned)buflen < sizeof(wireTimeWindow)) {
	std::cout << "Error (TimeWindow::Serialize): Buffer is too small" << 
	    std::endl;
	return(TN_FAILURE);
    }

    w.start = htond(start.ts_as_double(TRUE_EPOCH_TIME));
    w.end = htond(end.ts_as_double(TRUE_EPOCH_TIME));
    std::memcpy(buf, &w, sizeof(wireTimeWindow));
    buflen = sizeof(wireTimeWindow);
    return(TN_SUCCESS);
}


bool TimeWindow::Overlap(const TimeWindow &tw) const
{
    if ((start <= tw.start) && (end > tw.start)) {
	return(true);
    }
    if ((start >= tw.start) && (start < tw.end)) {
	return(true);
    }
    
    return(false);
}


TimeWindow& TimeWindow::operator=(const TimeWindow &tw)
{
    start = tw.start;
    end = tw.end;
    return(*this);
}



bool TimeWindow::operator==(const TimeWindow &tw) const
{
    return ((start == tw.start) && (end == tw.end));
}

ostream & operator <<(ostream & os, const TimeWindow &tw)
{
  os << tw.start << "->" << tw.end;
  return(os);
}
    

// The following "less than" operator does not make physical sense,
// but Caltech has requested it be defined here anyway.
bool TimeWindow::operator<(const TimeWindow &tw) const
{
  if ((start < tw.start) 
      || ((start == tw.start) && (end < tw.end))) {
    return (true);
  } else {
    return(false);
  }
}


TimeWindow::operator Duration() const
{
    return end - start;
}
