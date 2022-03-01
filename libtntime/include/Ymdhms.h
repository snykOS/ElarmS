/***********************************************************

File Name :
	Ymdhms

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

#ifndef ymdhms_h
#define ymdhms_h

#include "TimeStamp.h"

struct ymdhms
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  float seconds;
  char month_str[4];
  char am_pm[4];
  char timezone[8];
};


int year_mdhms(const TimeStamp &intime,
	       struct ymdhms& utc_outtime,
	       struct ymdhms& us_pacific_outtime);

#endif
