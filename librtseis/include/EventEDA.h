/***********************************************************

File Name :
	event.h

Programmer:
	Phil Maechling

Description:
	This is the structure of the data in the eda buffer.

Creation Date:
	27 August 1997


Usage Notes:



Modification History:
	10 Dec 2007 Pete Lombard - Modified to use leap-second-aware 
	tntime classes.



**********************************************************/
#ifndef eventEDA_H
#define eventEDA_H

#include "TimeStamp.h"
#include "nscl.h"

class EventEDA : public nscl{
public:
  TimeStamp	origin_time;
  float  latitude;
  float  longitude;
  float  peak_amplitude;
  int    number_of_confirming_stations;
  float  amplitude_threshold; 

EventEDA();
~EventEDA();
};

#endif
