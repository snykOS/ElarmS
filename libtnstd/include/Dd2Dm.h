/***********************************************************

File Name :
	Decimal Degrees to Degrees Minutes

Programmer:
	Phil Maechling

Description:
	Converts a double time to ymdhms with seconds as float.

Creation Date:
	22 September 1998


Usage Notes:



Modification History:



**********************************************************/

#ifndef dd2dm_h
#define dd2dm_h

struct lat_long
{
  float lat_dd;
  float lon_dd;
  int   lat_dm_deg;
  float lat_dm_min;
  int   lon_dm_deg;
  float lon_dm_min;

};

int DecDeg2DegMin(struct lat_long& location);

#endif
