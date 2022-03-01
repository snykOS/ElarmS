/***********************************************************

File Name :
	tt.h

Programmer:
	Phil Maechling

Description:
	Header file for the travel_time fortran routines.
	This returns the pn and pg time, and sn and sg times.

Creation Date:
	12 September 1995

Modification History:


Usage Notes:
	I compilied in the s-cal velocity structure,
	to speed up processing.
   	Recompilation required if this structure changes.

**********************************************************/
#ifndef tt_H
#define tt_H

#ifdef __cplusplus
extern "C"
{

  void travel_t_(float* dist,
		 float* p1,
		 float* p2,
		 float* s1,
		 float* s2);
}
#else

travel_t_(float* dist,
	  float* p1,
	  float* p2,
	  float* s1,
	  float* s2);

#endif
#endif
