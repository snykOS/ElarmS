/***********************************************************

File Name :
        CS2EW.h

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

Description:

	This is a comserv client that reads packets from a comserv
	memory area and writes the packets into an EW ring.

Creation Date:
        18 May 2000


Modification History:


Usage Notes:


**********************************************************/

#ifndef mcast2ew_H
#define mcast2ew_H

// Maxmimun number of channels per station
const int MAX_CHANNELS_PER_STATION = 12;

// Maximum number of channel selectors per station
const int MAX_SELECTORS = MAX_CHANNELS_PER_STATION + 2;


// Amount of time to sleep between polls
const int POLL_NANO = 500000000;

// Comserv setup constants
char CLIENT_NAME[] = "CS2EW" ;
char SERVER_NAME[] = "*" ;
const int COM_OUT_BUF_SIZE = 6000;


#endif
