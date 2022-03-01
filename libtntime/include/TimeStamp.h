/*************************************************************

  Filename:
	TimeStamp.H

  Description:
	This is the timestamp class. Represents a point in time;
	is able to represent times during leap seconds.

  Programmer:
	Pete Lombard

  Creation Date:
	15 October 2007

  Modification History:
	This is a complete rewrite of the old TimeStamp and TimeRep classes
	which were not capable of representing leap seconds.

**************************************************************/

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <inttypes.h>
#include <sys/time.h>
#include <iostream>

using std::ostream;
using std::istream;

class Duration;
class TimeWindow;

enum TimeStyle
    {
	TRUE_EPOCH_TIME,
	NOMINAL_EPOCH_TIME
    };

// Some aliases for TimeStyles
const TimeStyle UNIX_TIME = NOMINAL_EPOCH_TIME;
const TimeStyle ANTELOPE_TIME = NOMINAL_EPOCH_TIME;
const TimeStyle EARTHWORM_TIME = NOMINAL_EPOCH_TIME;
const TimeStyle LEAPSECOND_TIME = TRUE_EPOCH_TIME;

class TimeStamp
{
    friend ostream& operator<<(ostream &os, const TimeStamp &in);
    friend istream& operator>>(istream &is, TimeStamp &out);
    friend class Duration;
    friend class TimeWindow;
    
 private:
    int64_t _seconds; // since 00:00:00 UTC 1/1/1970 including leapseconds
    int usec;         // microseconds, 0 - 999999
                      // usec values outside this range are allowed in
                      // constructor arguments; the values get normalized
                      // within the constructor.
    bool initialized;
    static double maxDtime;  // maximum allowed time as double for constructor
    static double minDtime;  // minimum allowed time as double for constructor

    // Private constructor: we don't want to expose users to internals
    // of this class.
    TimeStamp(const int64_t in_sec, const int in_usec);

    void _normalize();

 public:
    // Constructors:
    TimeStamp(const TimeStamp &intime);  // copy constructor
    TimeStamp(const int year, const int month, const int day, const int hour,
	      const int min, const int sec, const int usec);
    TimeStamp(const int year, const int dayofyear, const int hour,
	      const int min, const int sec, const int usec);
    TimeStamp(const TimeStyle &style, const double &intime);
    TimeStamp(const TimeStyle &style, const timeval &intime);
    TimeStamp();      // default constructor

    // Set to current time
    static TimeStamp current_time();

    // Arithmetic operators:
    Duration    operator -(const TimeStamp & in) const;
    TimeStamp   operator +(const Duration & in) const;
    TimeStamp   operator -(const Duration & in) const;
    TimeStamp & operator+=(const Duration & in);
    TimeStamp & operator-=(const Duration & in);

    // Relational operators:
    bool operator==(const TimeStamp &in) const;
    bool operator< (const TimeStamp &in) const;
    bool operator<=(const TimeStamp &in) const;
    bool operator> (const TimeStamp &in) const;
    bool operator>=(const TimeStamp &in) const;
    bool operator !() const;

    // Accessor methods
    int year() const;
    int month_of_year() const;
    int day_of_year() const;
    int day_of_month() const;
    int hour_of_day() const;
    int minute_of_hour() const;
    int second_of_minute() const;
    int milliseconds() const;
    int tenth_milliseconds() const;
    int u_seconds() const;
    int64_t seconds(const TimeStyle &style) const;
    double ts_as_double(const TimeStyle &style) const;

    // Bounds set and get functions
    static void getBounds(double &min, double &max);
    static void setBounds(double &min, double &max);
};

#endif

