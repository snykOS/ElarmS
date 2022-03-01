/***********************************************************

File Name :
	where.h

Programmer:
	Phil Maechling

Description:
	Header file for the fortran where routines.


Creation Date:
	8 September 1995

Modification History:


Usage Notes:


**********************************************************/
#ifndef where_H
#define where_H

#ifdef __cplusplus
extern "C"
{
void set_paths_(const char *townfile, const char *faultfile);
void where_(float* lat, float* longitu, int* res);
void where_fault_(float* lat, float* longitu, int* res);
void get_where_grps_(char *str_array);
void get_where_fault_(char *str_array);

}
#else

void set_paths_(const char *townfile, const char *faultfile);
void where_(float* lat, float* longitu, int* res);
void where_fault_(float* lat, float* longitu, int* res);
void get_where_grps_(char *str_array);
void get_where_fault_(char *str_array);

#endif
#endif
