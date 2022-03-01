/*************************************************************

  Filename:
	duration.H

  Description:
	This is the duration class. A duration is the signed length
        of an arbitrary time interval.

  Programmer:
	Pete Lombard

  Creation Date:
	15 October 2007

  Modification Date:


**************************************************************/

#ifndef DURATION_H
#define DURATION_H

#include <inttypes.h>
#include <sys/time.h>
#include <iostream>

using std::ostream;

class TimeStamp;
class TimeWindow;

class Duration
{
    friend ostream & operator<<(ostream & ot, const Duration &in);
    friend class TimeStamp;
    friend class TimeWindow;
    
 private:
    int64_t seconds; // since arbitrary start time; a SIGNED quantity
    int     usec;    // microseconds, 0 - 999999
                     // usec values outside this range are allowed in
                     // constructor arguments; the values get normalized
                     // within the constructor.
    bool initialized;
    
    
    // Private constructor: we don't want to expose users to internals
    // of this class.
    Duration(const int64_t in_sec, const int in_usec);
    
    void _normalize();

 public:
    // Constructors:
    Duration(const double sec);
    Duration(const timeval &tv);
    Duration();

    // Arithmetic operators
    // -- and ++ are not included as they imply granularity of time
    TimeStamp  operator+ (const TimeStamp & in) const;
    Duration   operator+ (const Duration  & in) const;
    Duration   operator* (int count) const;
    Duration   operator- (const Duration  & in) const;
    Duration & operator+=(const Duration  & in);
    Duration & operator-=(const Duration  & in);

    // Relational operators
    bool operator==(const Duration &in) const;
    bool operator< (const Duration &in) const;
    bool operator<=(const Duration &in) const;
    bool operator> (const Duration &in) const;
    bool operator>=(const Duration &in) const;

    // Convert a Duration into the number of seconds.
    operator double() const;
    
    // Output a timeval structure
    timeval tv() const;
    
};

#endif
