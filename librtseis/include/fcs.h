/***********************************************************

File Name :
	fcs.h

Programmer:
	Phil Maechling

Description:
	This defines a checksum routine used by the qpager software.

Creation Date:
	9 Septermber 1995

Modification History:


Usage Notes:


**********************************************************/

#ifndef fcs_H
#define fcs_H

#ifdef __cplusplus

extern "C"
{
  unsigned int cksum(char* s);
}

#else
  unsigned int cksum(char* s);


#endif

#endif
