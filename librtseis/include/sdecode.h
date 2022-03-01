/***********************************************************

File Name :
	sdecode.h

Programmer:
	Phil Maechling

Description:
	Static Decode.
	This is a static version of Doug N's decode_hdr.h

Creation Date:
	22 July 1996


Usage Notes:



Modification History:



**********************************************************/
/*  Routines in sdecode.c */
#ifndef sdecode_H
#define sdecode_H

#include    "timedef.h"
#include    "sdr.h"
#include    "data_hdr.h"
#include    "sdr_utils.h"

#ifdef __cplusplus
extern "C"
{
int decode_hdr_sdr_static(DATA_HDR *, SDR_HDR *, int *);
}
#else
int         decode_hdr_sdr_static (DATA_HDR* ,SDR_HDR *, int *);
#endif
#endif
