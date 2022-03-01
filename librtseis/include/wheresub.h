/***********************************************************

File Name :
	wheresub.h

Programmer:
	Phil Maechling

Description:
	These are c interface routines to fortran where routines.

Creation Date:
	8 September 1995

Modification History:


Usage Notes:


**********************************************************/
#ifndef wheresub_H
#define wheresub_H

#include "where.h"

#define MAX_WHERE_STR 80

#ifdef __cplusplus
extern "C"
{
char *wheresub(float flat, float flong,
		char ws1[MAX_WHERE_STR + 1],
	        char ws2[MAX_WHERE_STR + 1],
	        char ws3[MAX_WHERE_STR + 1],
		char ws4[MAX_WHERE_STR + 1],
		char ws5[MAX_WHERE_STR + 1]);
}
#else
char *wheresub(float flat, float flong,
		char ws1[MAX_WHERE_STR + 1],
	        char ws2[MAX_WHERE_STR + 1],
	        char ws3[MAX_WHERE_STR + 1],
		char ws4[MAX_WHERE_STR + 1],
		char ws5[MAX_WHERE_STR + 1]);
#endif
#endif
