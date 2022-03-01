/***********************************************************

File Name :
        CS2Utils.h

Programmer:
        Patrick Small

Based Upon the EW Module:

        q2ew - quanterra two earthworm 

                (designed and tested using Q730 dataloggers)

        COPYRIGHT 1998, 1999: Instrumental Software Technologies, Inc.
        ALL RIGHTS RESERVED. Please contact the authors for use
        of this code. It is free to all Academic institutions and
        Federal Government Agencies.

        This code requires the COMSERV libraries of Quanterra Inc. and
        the QLIB2 libraries of Univ. California Berkeley, both of which
        may be obtained at the ftp site provided by Berkeley:
        ftp://quake.geo.berkeley.edu

        Authors: Paul Friberg & Sid Hellman 

        Contact: support@isti.com


Creation Date:
        18 May 2000


Modification History:


Usage Notes:


**********************************************************/

#ifndef csutils_H
#define csutils_H

// Various include files
#include "trace_buf.h"
#include "EWConfig.h"


int handleStatus(int status, long station);
int MSEEDToTrace(ewConfig &cfg, void *bytes, int len,
		 TracePacket &trace_buffer, long *out_message_size);

#endif
