/***********************************************************

File Name :
        MessageTypesUDP.h

Original Author:
        Patrick Small

Description:

        This header defines the UDP message types which are exchanged 
among processes in the Real-time and Data Acquisition system.


Creation Date:
        09 November 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef messagetypes_udp_H
#define messagetypes_udp_H


// Various include files
#include "seismic.h"
#include "nscl.h"

/**********************************************************

Type : TN_UDP_RNS_Shaking

Description :
     Contains a rapid notification shaking report from the
real-time and data acquisition system.

**********************************************************/

class TN_UDP_RNS_Shaking_Trigger : public nscl{
 public:
    float latitude;
    float longitude;
    double samptime;
    float reading;
};


struct TN_UDP_RNS_Shaking {
    int msgnum;
    double msgtime;
    long eventid;
    int conflevel;
    TN_UDP_RNS_Shaking_Trigger firsttrig;
    int accelavail;
    TN_UDP_RNS_Shaking_Trigger peakaccel;
    int velavail;
    TN_UDP_RNS_Shaking_Trigger peakvel;    
};

/**********************************************************
**********************************************************/


#endif
