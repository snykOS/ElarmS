/***********************************************************

File Name :
	dstaz.h

Programmer:
	Patrick Small

Description:
	Header file for the travel_time fortran routines.

Creation Date:
	28 June 1999

Modification History:


Usage Notes:


**********************************************************/
#ifndef dstaz_H
#define dstaz_H

#ifdef __cplusplus
extern "C"
{
  void distaz_(float* lt1, float* ln1, float* dep, float* lt2, float* ln2, 
	       float* dist, float* az, float* ptime);
  void parrtime_(float* deg, float* edepth, float* ptime);

}
#else

distaz_(float* lt1, float* ln1, float* dep, float* lt2, float* ln2, 
	float* dist, float* az, float* ptime);
parrtime_(float* deg, float* edepth, float* ptime);

#endif
#endif
