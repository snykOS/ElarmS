/***********************************************************

File Name :
	Decimal Degrees to Degrees Minutes

Programmer:
	Phil Maechling

Description:
	Converts a decimal degree to a degress minutes.
  	It will preserve the input sign of the lat or long.
	The minutes will always return postive.

Creation Date:
	12 Jan 2000


Usage Notes:



Modification History:



**********************************************************/
#include <iostream>
#include <cstring>
#include <cmath>
#include "Dd2Dm.h"
#include "RetCodes.h"

using namespace std;

int DecDeg2DegMin(struct lat_long& location)
{

double f,lof;
double ih,loih;
float dtemp;
float flat_min, flong_min;
int startof,endof,lenof;

/* Find degree's minutes */

  f = modf((double) location.lat_dd,&ih);

/* Find Degree minutes */

  dtemp = (float) ((fabs(f) * ((double)60.0)));
  flat_min = dtemp;


/* Longitude */

  lof = modf((double) location.lon_dd,&loih);

/* Longitude minutes */

  dtemp = (float) (fabs(lof) * ((double) 60.0));
  flong_min = dtemp;

  location.lat_dm_deg = (int) ih;
  location.lat_dm_min = flat_min;

  location.lon_dm_deg = (int) loih;
  location.lon_dm_min = flong_min;

  return(TN_SUCCESS);
}
