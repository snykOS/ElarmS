/***********************************************************

File Name :
	adjave.h

Programmer:
	Phil Maechling

Description:
	This is the statistics routine used to dropped glitches
	and outliers from magnitude processing.

Creation Date:
	12 December 1996


Usage Notes:



Modification History:



**********************************************************/
#ifndef adjave_H
#define adjave_H

#ifdef __cplusplus

extern "C"
{
  void adjave_(float* x,
	       int * n,
	       float* av,
	       float* sv,
	       int* j,
	       int* nj,
	       float* thr);
  
}
#else

  adjave_(float* x,
          int * n,
	  float* av,
	  float* sv,
	  int* j,
	  int* nj,
	  float* thr);

#endif
#endif
