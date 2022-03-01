/*************************************************************

  Filename:
	Duration.C

  Description:
	This is the duration class. A duration is the signed length
        of an arbitrary time interval.

  Programmer:
	Pete Lombard

  Creation Date:
	15 October 2007

  Modification Date:

    8 Feb 2012 PNL: fixed normalization for negative usecs.

**************************************************************/

#include <cassert>
#include <cmath>
#ifdef __SUNPRO_CC
#include <math.h>
#endif
#include "Compat.h"
#include "TimeStamp.h"
#include "Duration.h"
#include "qlib2.h"

const int MAX_USEC = 1000000;


// Constructors:
Duration::Duration()
{
    seconds = 0;
    usec = 0;
    initialized = false;
}

Duration::Duration(const double sec)
{
    seconds = (int64_t) sec;
    usec = (int) lround((sec-(int64_t) sec) * 1000000);
    _normalize();    
    initialized = true;
}
    
Duration::Duration(const timeval &tv)
{
    seconds = (int64_t) tv.tv_sec;
    usec = tv.tv_usec;
    _normalize();
    initialized = true;
}


// Private constructor: we don't want to expose users to internals
// of this class.
Duration::Duration(const int64_t sec, const int usec_in)
{
    seconds = sec;
    usec = usec_in;
    _normalize();
    initialized = true;
}


// Arithmetic operators
// -- and ++ are not included as they imply granularity of time
TimeStamp  Duration::operator+ (const TimeStamp & in) const 
{
    assert(in.initialized && initialized);
    return TimeStamp(seconds + in._seconds, usec + in.usec);
}

Duration   Duration::operator+ (const Duration  & in) const
{
    assert(in.initialized && initialized);
    return Duration(seconds + in.seconds, usec + in.usec);
}

Duration   Duration::operator* (int count) const
{
    assert(initialized);

    int64_t scaled_useconds = (int64_t)usec * count;
    int64_t scaled_seconds = seconds * count;  // may overflow

    scaled_seconds += scaled_useconds / 1000000;
    scaled_useconds %= 1000000;

    return Duration(scaled_seconds, (int)scaled_useconds);
}


Duration   Duration::operator- (const Duration  & in) const
{
    assert(in.initialized && initialized);
    return Duration(seconds - in.seconds, usec - in.usec);
}

Duration & Duration::operator+=(const Duration  & in) 
{
    assert(in.initialized && initialized);
    seconds += in.seconds;
    usec += in.usec;
    _normalize();
    return *this;
}

Duration & Duration::operator-=(const Duration  & in)
{
    assert(in.initialized && initialized);
    seconds -= in.seconds;
    usec -= in.usec;
    _normalize();
    return *this;
}


// Relational operators
bool Duration::operator==(const Duration &in) const
{
    assert(in.initialized && initialized);
    return (seconds == in.seconds && usec == in.usec);
}

bool Duration::operator< (const Duration &in) const
{
    assert(in.initialized && initialized);
    if (seconds == in.seconds) return (usec < in.usec);
    return seconds < in.seconds;
}

bool Duration::operator<=(const Duration &in) const
{
    assert(in.initialized && initialized);
    if (seconds == in.seconds) return (usec <= in.usec);
    return (seconds < in.seconds);
}

bool Duration::operator> (const Duration &in) const
{
    assert(in.initialized && initialized);
    if (seconds == in.seconds) return (usec > in.usec);
    return (seconds > in.seconds);
}

bool Duration::operator>=(const Duration &in) const
{
    assert(in.initialized && initialized);
    if (seconds == in.seconds) return (usec >= in.usec);
    return (seconds > in.seconds);
}

// Convert a Duration into the number of seconds.
Duration::operator double() const
{
    assert(initialized);
    return (double)seconds + usec * 1.0e-6;
}


timeval Duration::tv() const
{
    timeval tv;
    tv.tv_sec = (time_t) seconds;
    tv.tv_usec = (suseconds_t) usec;
    return tv;
}



ostream & operator<<(ostream & os, const Duration &in)
{
    assert(in.initialized);
    double d = (double)in.seconds + 1.0e-6 * in.usec;
    
    os << Compat::Form("%.6f seconds", d);
    return os;
}


void Duration::_normalize() 
{
    while (usec >= MAX_USEC) {
	usec -= MAX_USEC;
	seconds++;
    }
    while (usec < 0) {
	usec += MAX_USEC;
	seconds--;
    }
    return;
}
