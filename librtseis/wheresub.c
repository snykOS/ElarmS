/***********************************************************

File Name :
	wheresub.c

Programmer:
	Phil Maechling

Description:
	This is a c interface to a fortran where routine.

Creation Date:
	21 September 1998


Usage Notes:



Modification History:



**********************************************************/

#include <string.h>
#include <strings.h>

#define MAX_WHERE_STR 80

extern where_();
extern get_where_grps_();
extern get_where_fault_();


char *wheresub(float flat, float flong,
	       char ws1[MAX_WHERE_STR + 1],
               char ws2[MAX_WHERE_STR + 1],
               char ws3[MAX_WHERE_STR + 1],
	       char ws4[MAX_WHERE_STR + 1],
	       char ws5[MAX_WHERE_STR + 1])
{
  float lat_deg,long_deg;
  int  ires;
  char str_array[8][MAX_WHERE_STR];  /* Eight strings with 81 chars each */
  char flt_name[MAX_WHERE_STR];

  memset(ws1,0,81); /*  Null out the return string to be sure they */
		    /* end with a null char */
  memset(ws2,0,81);
  memset(ws3,0,81);
  memset(ws4,0,81);
  memset(ws5,0,81);

  lat_deg  = flat;  
  long_deg =  flong;

  where_(&lat_deg, &long_deg, &ires); /* note: lon must be negative */
  get_where_grps_(&str_array);

  strcpy(ws1,str_array[0]);
  strcpy(ws2,str_array[1]);
  strcpy(ws3,str_array[2]);
  strcpy(ws4,str_array[3]);


  where_fault_(&lat_deg, &long_deg, &ires); /* note: lon must be negative */
  get_where_fault_(&flt_name);
  strcpy(ws5,flt_name);

  return(str_array[0]);
}
