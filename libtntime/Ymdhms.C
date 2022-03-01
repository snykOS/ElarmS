/***********************************************************

File Name :
	Ymdhms.C

Programmer:
	Phil Maechling

Description:
	Converts a TimeStamp to ymdhms structures.
	One of the output structures represents UTC, the other
	represents US/pacific time.

Creation Date:
	22 September 1998


Usage Notes:



Modification History:
	15 October 2007 - Pete Lombard
	Revised description to more accurately reflect the code
	Changed input parameter from double to TimeStamp.
	Time as a double is ambiguous; it needs to be tied to a specific
	epoch for it to have any meaning.


**********************************************************/
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
#ifdef __SUNPRO_CC
#include <stdlib.h>
#endif
#include "y2k.h"
#include "Ymdhms.h"
#include "RetCodes.h"

using namespace std;

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

int year_mdhms(const TimeStamp &intime,
	       struct ymdhms& utc_outtime,
	       struct ymdhms& us_pacific_outtime)
{

    char monstr[12][4] = {"Jan",
			  "Feb",
			  "Mar",
			  "Apr",
			  "May",
			  "Jun",
			  "Jul",
			  "Aug",
			  "Sep",
			  "Oct",
			  "Nov",
			  "Dec"};
  
    //
    // Initialize the output structures
    //

    strcpy(utc_outtime.am_pm,"");
    strcpy(us_pacific_outtime.am_pm,"");

    strcpy(utc_outtime.timezone,"UTC");
    strcpy(us_pacific_outtime.timezone,"");

    struct timeval origtval;
    origtval.tv_sec = intime.seconds(UNIX_TIME);
    origtval.tv_usec = 0;

    double dummy, dtime;
    dtime = intime.ts_as_double(UNIX_TIME);
    double dfract = modf(dtime,&dummy);
    float fraction = (float) dfract;

    //
    // Set the time conversion and time printouts to use US/Pacfic
    //

    //
    // Set the environment variable to be the local time
    //

    // Save the old TZ so we can put it back
    char oldtz[PATH_MAX];
    oldtz[0] = 0;
    size_t len = 0;
    if (getenv("TZ") != NULL && (len = strlen(getenv("TZ"))) > 0) {
	strncpy(oldtz, getenv("TZ"), PATH_MAX -1);
	oldtz[PATH_MAX-1] = 0;
    }
    
    char ist[] = "TZ=US/Pacific";
    putenv(ist);

    struct tm* evtime;
    evtime       = localtime(&origtval.tv_sec);

    //
    // Determine whether it is Daylight or Standard time
    //

    if (evtime->tm_isdst != 0) {
	strcpy(us_pacific_outtime.timezone,"PDT");
    } else {
	strcpy(us_pacific_outtime.timezone,"PST");
    }

    //
    // Determine if the local time is AM or PM
    //

    if(evtime->tm_hour < 12) {
	strcpy(us_pacific_outtime.am_pm,"AM");
    } else {
	strcpy(us_pacific_outtime.am_pm,"PM");
    }

    //
    // Change the hours to 12 hour time
    //

    int twelvehour;

    if(evtime->tm_hour > 12) {
	twelvehour = evtime->tm_hour - 12;
    } else {
	twelvehour = evtime->tm_hour;
    }

    //
    //
    //
    strcpy(us_pacific_outtime.month_str,monstr[evtime->tm_mon]); 

    int fullyear = find_four_digit_year(evtime->tm_year);
    us_pacific_outtime.year = fullyear;
    us_pacific_outtime.month  = evtime->tm_mon + 1;
    us_pacific_outtime.day = evtime->tm_mday;
    us_pacific_outtime.hour = twelvehour;
    us_pacific_outtime.minute = evtime->tm_min;
    us_pacific_outtime.seconds = (float) evtime->tm_sec;
    us_pacific_outtime.seconds = us_pacific_outtime.seconds + fraction;

    //
    // Now Repeat the processing for the UTC Time
    //

    strcpy(utc_outtime.month_str,monstr[intime.month_of_year() - 1]); 
    utc_outtime.year = intime.year();
    utc_outtime.month  = intime.month_of_year();
    utc_outtime.day = intime.day_of_month();
    utc_outtime.hour = intime.hour_of_day();
    utc_outtime.minute = intime.minute_of_hour();
    utc_outtime.seconds = (float) intime.second_of_minute() + 
	1.0e-6 * intime.u_seconds();

    // Put the old timezone back into effect
    if (strlen(oldtz) > 0) {
	putenv(oldtz);
	tzset();
    }
    
    return(TN_SUCCESS);
}
