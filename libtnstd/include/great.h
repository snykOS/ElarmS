/***********************************************************

File Name :
	great.h

Programmer:
	Phil Maechling

Description:
	This is a great circle distance calculation routine.
	It was written by Hiroo Kanamori.

Creation Date:
	12 September 1995

Modification History:
	Modified by Phil Maechling for use in terrascope work.

Usage Notes:


**********************************************************/
#ifndef great_H
#define great_H

#ifdef __cplusplus

extern "C" {

#endif

  void great_(float* OLAT1,
	      float* OLON1,
	      float* OLAT2,
	      float* OLON2,
	      float* ODIST,
	      float* ODISD,
	      float* OGC,
	      float* OZ12,
	      float* OZ21);

#ifdef __cplusplus

}

#endif

#endif
