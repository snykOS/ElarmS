/***********************************************************

File Name :
	m1002m.h

Programmer:
	Phil Maechling

Description:
	Header file describing the m1002m routine.
	This program converts a m100 to a regular magnitude.

Creation Date:
	8 December 1996


Usage Notes:



Modification History:



**********************************************************/
#ifndef m1002m_H
#define m1002m_H

#ifdef __cplusplus

extern "C"
{
  void m1002m_(float* am100,
	       float* am,
	       float* dist,
	       float* depth,
	       int* iflag);
  
  void m1002m_hyp_(float* am100,
		   float* am,
		   float* dist,
		   int* iflag);
}

#else

  m1002m_(float* am100,
          float* am,
          float* dist,
	  float* depth,
          int* iflag);

  m1002m_hyp_(float* am100,
	      float* am,
	      float* dist,
	      int* iflag);

#endif
#endif
