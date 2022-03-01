/*************************************************************

  Filename:
	TimeStamp.C

  Description:
	This is the TimeStamp class. Represents a point in time;
	is able to represent times during leap seconds.

  Programmer:
	Pete Lombard

  Creation Date:
	15 October 2007

  Modification Date:
	This is a complete rewrite of the old TimeStamp and TimeRep classes
	which were not capable of representing leap seconds.

    15 May 2011 PNL: fixed rounding of usec in accessors.

    8 Feb 2012 PNL: fixed normalization for negative usecs.

   25 Feb 2017 PNL: changed minDtime from 1970/01/01 to 1900/01/01

   23 Aug 2019 CAF: Added input stream operator, minor cleanup for compiler warnings


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

// Define the static members
double TimeStamp::maxDtime = 32503690000.0;  // a little after 3000/01/01, to 
                                             // allow for lots of leap seconds.
double TimeStamp::minDtime = -2208988800.0;  // 1900/01/01


// Constructors:
TimeStamp::TimeStamp() 
{
    _seconds = 0;
    usec = 0;
    initialized = false;
}


TimeStamp::TimeStamp(const TimeStamp &intime) 
{
    _seconds = intime._seconds;
    usec = intime.usec;
    initialized = intime.initialized;
    
    _normalize();
}

TimeStamp::TimeStamp(const int year_in, const int month_in, const int day_in, 
		     const int hour_in, const int min_in, const int sec_in, 
		     const int usec_in)
{
    EXT_TIME et;
    INT_TIME it;
    
    et.year = year_in;
    // qlib2's normalization believes doy, NOT month and day
    et.doy = mdy_to_doy(month_in, day_in, year_in);
    et.hour = hour_in;
    et.minute = min_in;
    et.second = sec_in;
    // Put usec_in into et so it will get normalized with other inputs
    et.usec = usec_in;
    
    it = ext_to_int(et);  // qlib2 does normalization here
    usec = it.usec;
    it.usec = 0;
    _seconds = (int64_t) int_to_tepoch(it);
    initialized = true;
}



TimeStamp::TimeStamp(const int year_in, const int dayofyear_in, 
		     const int hour_in, const int min_in, const int sec_in, 
		     const int usec_in)
{
    EXT_TIME et;
    INT_TIME it;
    
    et.year = year_in;
    et.doy = dayofyear_in;
    et.hour = hour_in;
    et.minute = min_in;
    et.second = sec_in;
    // Put usec_in into et so it will get normalized with other inputs
    et.usec = usec_in;
    
    it = ext_to_int(et);  // qlib2 does normalization here
    usec = it.usec;
    it.usec = 0;
    _seconds = (int64_t) int_to_tepoch(it);
    initialized = true;
}


TimeStamp::TimeStamp(const TimeStyle &style, const double &intime)
{
    initialized = false;
    if (intime < minDtime) {
	std::cerr << "TimeStamp constructor input: " << intime 
		  << "; below minimum allowed" << std::endl;
    }
    else if (intime > maxDtime) {
	std::cerr << "TimeStamp constructor input: " << intime 
		  << "; above maximum allowed" << std::endl;
    }
    else if (isnan(intime)) {
	std::cerr << "TimeStamp constructor input is NaN" << std::endl;
    }
    else {
	switch (style) {
	case NOMINAL_EPOCH_TIME:
	    {
		double t_seconds = nepoch_to_tepoch(intime);
		_seconds = (int64_t) t_seconds;
		usec = (int)((t_seconds - _seconds) * 1.0e+6 + 0.5);
	    }
	    break;
	case TRUE_EPOCH_TIME:
	    _seconds = (int64_t)intime;
	    usec = (int)((intime - _seconds) * 1.0e+6 + 0.5);
	    break;
	}
	_normalize();
	initialized = true;
    }
}

TimeStamp::TimeStamp(const TimeStyle &style, const timeval &intime)
{
    initialized = false;
    if ((double)intime.tv_sec < minDtime) {
	std::cerr << "TimeStamp constructor input: " << intime.tv_sec 
		  << "; below minimum allowed" << std::endl;
    }
    else if ((double)intime.tv_sec > maxDtime) {
	std::cerr << "TimeStamp constructor input: " << intime.tv_sec
		  << "; above maximum allowed" << std::endl;
    }
    else {
	switch (style) {
	case NOMINAL_EPOCH_TIME:
	    _seconds = (int64_t) nepoch_to_tepoch((double)intime.tv_sec);
	    usec = intime.tv_usec;
	    break;
	case TRUE_EPOCH_TIME:
	    _seconds = intime.tv_sec;
	    usec = intime.tv_usec;
	    break;
	}
	_normalize();
	initialized = true;
    }
}


// Private constructor: we don't want to expose users to internals
// of this class.
TimeStamp::TimeStamp(const int64_t in_sec, const int in_usec) 
{
    _seconds = in_sec;
    usec = in_usec;
    _normalize();
    initialized = true;
}

// Set to current time
TimeStamp TimeStamp::current_time()
{
    // unused -- int res;
    timeval tval;
    
    /*res =*/ gettimeofday(&tval, NULL);
    
    return TimeStamp(NOMINAL_EPOCH_TIME, tval);
}

// output stream operator
ostream& operator<<(ostream &os, const TimeStamp &in)
{
    assert(in.initialized);
    INT_TIME it = tepoch_to_int((double)in._seconds + 
				1.0e-6 * in.usec);

    os << time_to_str(it, MONTHS_FMT_1);
    return os;
}

// input stream operator
std::istream& operator>> (std::istream& is, TimeStamp& out) {
    std::string str;
    is  >> str;

    int year, mon, day, hour, min, sec, usec; char dummy;

    // try parsing with forward slash in date
    if (sscanf(str.c_str(), "%d/%d/%d%c%d:%d:%d.%d", &year, &mon, &day, &dummy, &hour, &min, &sec, &usec) == 8) {
        TimeStamp time(year, mon, day, hour, min, sec, usec * 100);
        out = time;
        return is;
    }
    // try parsing with dash in date
    if (sscanf(str.c_str(), "%d-%d-%d%c%d:%d:%d.%d", &year, &mon, &day, &dummy, &hour, &min, &sec, &usec) == 8) {
        TimeStamp time(year, mon, day, hour, min, sec, usec * 100);
        out = time;
        return is;
    }

    // try parsing with forward slash in date and without usec
    if (sscanf(str.c_str(), "%d/%d/%d%c%d:%d:%d", &year, &mon, &day, &dummy, &hour, &min, &sec) == 7) {
        TimeStamp time(year, mon, day, hour, min, sec, 0);
        out = time;
        return is;
    }
    // try parsing with dash in date and without usec
    if (sscanf(str.c_str(), "%d-%d-%d%c%d:%d:%d", &year, &mon, &day, &dummy, &hour, &min, &sec) == 7) {
        TimeStamp time(year, mon, day, hour, min, sec, 0);
        out = time;
        return is;
    }

    // otherwise parsing was not sucessful
    TimeStamp badTime;
    out = badTime;
    return is;
} // operator>>


// Arithmetic operators:
Duration    TimeStamp::operator -(const TimeStamp & in) const
{
    assert(in.initialized && initialized);
    return Duration(_seconds - in._seconds, usec - in.usec);
}
    

TimeStamp   TimeStamp::operator +(const Duration & in) const
{
    assert(in.initialized && initialized);
    return TimeStamp(_seconds + in.seconds, usec + in.usec);
}

TimeStamp   TimeStamp::operator -(const Duration & in) const
{
    assert(in.initialized && initialized);
    return TimeStamp(_seconds - in.seconds, usec - in.usec);
}

TimeStamp & TimeStamp::operator+=(const Duration & in)
{
    assert(in.initialized && initialized);
    *this = TimeStamp(_seconds + in.seconds, usec + in.usec);
    return *this;
}

TimeStamp & TimeStamp::operator-=(const Duration & in)
{
    assert(in.initialized && initialized);
    *this = TimeStamp(_seconds - in.seconds, usec - in.usec);
    return *this;
}


// Relational operators:
bool TimeStamp::operator==(const TimeStamp &in) const
{
    assert(in.initialized && initialized);
    return (_seconds == in._seconds && usec == in.usec);
}

bool TimeStamp::operator< (const TimeStamp &in) const
{
    assert(in.initialized && initialized);
    if (_seconds == in._seconds) return (usec < in.usec);
    return _seconds < in._seconds;
}

bool TimeStamp::operator<=(const TimeStamp &in) const
{
    assert(in.initialized && initialized);
    if (_seconds == in._seconds) return (usec <= in.usec);
    return (_seconds < in._seconds);
}

bool TimeStamp::operator> (const TimeStamp &in) const
{
    assert(in.initialized && initialized);
    if (_seconds == in._seconds) return (usec > in.usec);
    return (_seconds > in._seconds);
}

bool TimeStamp::operator>=(const TimeStamp &in) const
{
    assert(in.initialized && initialized);
    if (_seconds == in._seconds) return (usec >= in.usec);
    return (_seconds > in._seconds);
}

bool TimeStamp::operator !() const
{
    return( ! initialized );
}


// Accessor methods
int TimeStamp::year() const
{
    assert(initialized);
    INT_TIME it = tepoch_to_int((double)_seconds + 1.0e-6 * usec);
    return it.year;
}

int TimeStamp::month_of_year() const
{
    assert(initialized);
    EXT_TIME et = int_to_ext(tepoch_to_int((double)_seconds + 1.0e-6 * usec));
    return et.month;
}

int TimeStamp::day_of_year() const
{
    assert(initialized);
    EXT_TIME et = int_to_ext(tepoch_to_int((double)_seconds + 1.0e-6 * usec));
    return et.doy;
}

int TimeStamp::day_of_month() const
{
    assert(initialized);
    EXT_TIME et = int_to_ext(tepoch_to_int((double)_seconds + 1.0e-6 * usec));
    return et.day;
}

int TimeStamp::hour_of_day() const
{
    assert(initialized);
    EXT_TIME et = int_to_ext(tepoch_to_int((double)_seconds + 1.0e-6 * usec));
    return et.hour;
}

int TimeStamp::minute_of_hour() const
{
    assert(initialized);
    EXT_TIME et = int_to_ext(tepoch_to_int((double)_seconds + 1.0e-6 * usec));
    return et.minute;
}

int TimeStamp::second_of_minute() const
{
    assert(initialized);
    EXT_TIME et = int_to_ext(tepoch_to_int((double)_seconds + 1.0e-6 * usec));
    return et.second;
}

int TimeStamp::milliseconds() const
{
    assert(initialized);
    return (int)(usec / 1000);
}

int TimeStamp::tenth_milliseconds() const
{
    assert(initialized);
    return (int)(usec / 100.0);
}

int TimeStamp::u_seconds() const
{
    assert(initialized);
    return usec;
}

int64_t TimeStamp::seconds(const TimeStyle &style) const
{
    assert(initialized);
    switch (style) {
    case TRUE_EPOCH_TIME:
	return _seconds;
	break;
    case NOMINAL_EPOCH_TIME:
	return (int64_t) tepoch_to_nepoch((double)_seconds);
	break;
    }
    return 0.0;  // never reached    
}


double TimeStamp::ts_as_double(const TimeStyle &style) const
{
    assert(initialized);
    switch (style) {
    case TRUE_EPOCH_TIME:
	return (double)_seconds + 1.0e-6 * usec;
	break;
    case NOMINAL_EPOCH_TIME:
	return tepoch_to_nepoch((double)_seconds + 1.0e-6 * usec);
	break;
    }
    return 0.0;  // never reached    
}


void TimeStamp::_normalize() 
{
    while (usec >= MAX_USEC) {
	usec -= MAX_USEC;
	_seconds++;
    }
    while (usec < 0) {
	usec += MAX_USEC;
	_seconds--;
    }
    return;
}

    
void TimeStamp::getBounds(double &min, double &max)
{
    min = minDtime;
    max = maxDtime;
    return;
}
			  
void TimeStamp::setBounds(double &min, double &max)
{
    minDtime = min;
    maxDtime = max;
    return;
}
