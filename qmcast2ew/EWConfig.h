/***********************************************************

File Name :
        EWConfig.h

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

	Used for mcast2ew by Paul Friberg 7/12/2005


Usage Notes:


**********************************************************/

#ifndef ewconfig_H
#define ewconfig_H

// Various include files
#include <set>
#include <map>
#include <cstdlib>
#include <cstring>
#include "GenLimits.h"
#include "Channel.h"
#include "mcastutil.h"
#include "StationMcast.h"

// Qlib include files
//#include "qlib2.h"

// Earmworm include files
extern "C" {
#include "earthworm.h"
#include "kom.h"
#include "transport.h"
}



// Net-STa-Channel maximum length constants
const int SEED_SITE_MAX_CHAR = 5;
const int SEED_CHAN_MAX_CHAR = 3;
const int SEED_NET_MAX_CHAR = 2;
const int SEED_LOC_MAX_CHAR = 2;


// Net-Sta-Chan-Location mapping structure
typedef struct _SCNL {
    char site[SEED_SITE_MAX_CHAR+1];        /* site name SEED */
    char chan[SEED_CHAN_MAX_CHAR+1];        /* channel name SEED */
    char net[SEED_NET_MAX_CHAR+1];          /* network code SEED */
    char loc[SEED_LOC_MAX_CHAR+1];       /* location code, to be used later */
    char comb[SEED_SITE_MAX_CHAR+SEED_CHAN_MAX_CHAR+SEED_NET_MAX_CHAR+SEED_LOC_MAX_CHAR+1];
    int pinno;                              /* EW PinNumber */
    struct _SCNL *next;                      /* linked list to next mapping */
} SCNL;

struct LocalLessStr
{
        bool operator() (const char* s1, const char *s2) const
        {
                return strcmp(s1, s2) < 0;
        }
}; 

typedef std::map<const char*, long, LocalLessStr, std::allocator<std::pair<const char *, long> > >  Name2LongMap;
typedef std::map<const char*, SHM_INFO *, LocalLessStr, std::allocator<std::pair<const char *, SHM_INFO *> > >  Name2RegionMap;
typedef std::map<const char*, StationMcast, LocalLessStr, std::allocator<std::pair<const char *, StationMcast> > >  Name2StationMcast;

// Type definition for a channel list
typedef std::set<Channel, std::less<Channel>, std::allocator<Channel> > ChannelList;


// Structure which stores the configuration settings for this program
struct ewConfig {
    char Progname[MAXSTR];
    char Config[MAXSTR];
    char mcast_addr[MAXSTR];
    char gapdir[MAXSTR];
    char latdir[MAXSTR];
    int  latlog_period;
    unsigned char QModuleId;
    long RingKey;
    int HeartbeatInt;
    int LogFile;
    int ComservSeqBuf;
    char ChannelFile[MAXSTR];
    SCNL *SCNL_head;
    int Verbose;
    int maxstation;
    unsigned char TypeHB;
    unsigned char TypeTrace;
    unsigned char TypeErr;
    MSG_LOGO DataLogo;
    unsigned char InstId;
    SHM_INFO  Region;
    Name2LongMap   RingName2Key;
    Name2RegionMap RingName2Region;
    Name2StationMcast sta_list;
    ChannelList cl;
};


typedef struct ewConfig ewConfig;


// Reads the EW config file and populates the ewConfig structure
int GetConfig(char *configfile, ewConfig &cfg);
int getPinFromSCNL( char * S, char *C, char *N, char *L,ewConfig &cfg);

#endif
