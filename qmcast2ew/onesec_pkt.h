#ifndef __onesec_pkt_h
#define __onesec_pkt_h

#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#define Q330_TIME_OFFSET_FROM_UNIX  946684800

#define MAX_ONESEC_PKT_RATE 1000

#define	ONESEC_PKT_NET_LEN	4
#define ONESEC_PKT_STATION_LEN	16
#define	ONESEC_PKT_CHANNEL_LEN	16
#define	ONESEC_PKT_LOCATION_LEN	4

struct onesec_pkt{
  char net[ONESEC_PKT_NET_LEN];
  char station[ONESEC_PKT_STATION_LEN];
  char channel[ONESEC_PKT_CHANNEL_LEN];
  char location[ONESEC_PKT_LOCATION_LEN];
  uint32_t rate;
  uint32_t timestamp_sec;
  uint32_t timestamp_usec;
  int32_t samples[MAX_ONESEC_PKT_RATE];
};

#endif
