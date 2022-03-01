/***********************************************************

File Name :
	ByteSwap.h

Programmer:
	Pete Lombard

Description:
	Byte-swapping routines beyond standard htons, htonl, etc.

Creation Date:
	3 April 2006

Modification History:
	2013/01/11 Renamed from byteswap.h due to name conflict on linux.

Usage Notes:


**********************************************************/
#ifndef byteswap_H
#define byteswap_H

#include <netinet/in.h>


#ifdef __cplusplus
extern "C"
{
#endif
    float htonf(float);
    float ntohf(float);
    double htond(double);
    double ntohd(double);
#ifdef __cplusplus
}
#endif

#endif
