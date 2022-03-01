/***********************************************************

File Name :
	CSUtils.C

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


Limitations or Warnings:
	

Creation Date:
	18 May 2000

Modification History:
        Location code upgrade -- Kalpesh Solanki -- 04.08.2005

Usage Notes:

**********************************************************/

// Various include files
#include <cstdio>
#include <ctime>
#include <cmath>
#include "RetCodes.h"
#include "CSUtils.h"
#include "TimeStamp.h"
#include "Duration.h"

// Format specifier for data header time formatting
const int TIME_FMT = 1;


/************************************************************************/
/*  date_string:							*/
/*	Return the string for a specified date.				*/
/************************************************************************/
char *date_string(INT_TIME time, char *dflt_str) 
{
  char    *p;
  if (missing_time(time)) return (dflt_str);
  p = time_to_str(time,TIME_FMT);
  return ((p!=NULL) ? p : dflt_str);
}



/************************************************************************/
/*  dump_data_hdr:							*/
/*	Dump the specified block header information.			*/
/************************************************************************/
void
dump_data_hdr (ewConfig &cfg, struct onesec_pkt *pkt)
{
  int  bl;
  
  struct timeval tv;
  tv.tv_sec = ntohl(pkt->timestamp_sec) + Q330_TIME_OFFSET_FROM_UNIX;
  tv.tv_usec = ntohl(pkt->timestamp_usec);
  TimeStamp ts (EARTHWORM_TIME, tv);

  logit((char *) "et", (char *) "%s: SNCL=%s.%s.%s.%s rate=%d ts=%s,%s = %04d/%02d/%02d,%02d:%02d:%02d.%06d",
	pkt->station, pkt->net, pkt->channel, pkt->location, ntohl(pkt->rate), 
	tv.tv_sec, tv.tv_usec, ts.year(), ts.month_of_year(), ts.day_of_month(),
	ts.hour_of_day(), ts.minute_of_hour(), ts.second_of_minute(),
	ts.u_seconds());
  return;
}



int onesec_pktToTrace(ewConfig &cfg, void *bytes, int len,
		   TracePacket &trace_buffer, long *out_message_size) 
{
  TRACE2_HEADER	*trace_hdr;	  /* ew trace header */
  int		num_samples;	  /* num samples from uncompression routine */
  int		pin_from_scn;	  /* return value to use from PINmap */
  SDR_HDR       *shdr;
  struct onesec_pkt *pkt = (struct onesec_pkt *)bytes;

  struct timeval tv;

  tv.tv_sec = ntohl(pkt->timestamp_sec) + Q330_TIME_OFFSET_FROM_UNIX;
  tv.tv_usec = ntohl(pkt->timestamp_usec);
  TimeStamp ts (EARTHWORM_TIME, tv);
  
  int rate = ntohl(pkt->rate);

  // Check for an invalid sample rate
  if (rate < 1 || rate > MAX_ONESEC_PKT_RATE) {
    	return(TN_FAILURE);
  }

  trace_hdr = (TRACE2_HEADER *) &trace_buffer.trh;
  memset((void*) trace_hdr, 0, sizeof(TRACE2_HEADER));
  trace_hdr->pinno = 0;		/* Unknown item */
  trace_hdr->nsamp = rate;
  trace_hdr->starttime = ts.ts_as_double(EARTHWORM_TIME);
  Duration dur = 1.0 - (1.0/rate);
  TimeStamp te = ts + (Duration)(dur);
  trace_hdr->endtime = te.ts_as_double(EARTHWORM_TIME);

  // Verify that the endtime is after the start time
  if (trace_hdr->endtime < trace_hdr->starttime) {
    *out_message_size=0;
    logit((char *) "et", (char *) "%s: Packet end time is before start time\n", cfg.Progname);
    if (cfg.Verbose) {
      dump_data_hdr(cfg, pkt);
    }
    return(TN_FAILURE);
  }

  trace_hdr->samprate = rate;
  strncpy(trace_hdr->sta,pkt->station, std::min(TRACE2_STA_LEN, ONESEC_PKT_STATION_LEN));
  strncpy(trace_hdr->net,pkt->net, std::min(TRACE2_NET_LEN,ONESEC_PKT_NET_LEN));
  strncpy(trace_hdr->chan,pkt->channel, std::min(TRACE2_CHAN_LEN,ONESEC_PKT_CHANNEL_LEN));
  strncpy(trace_hdr->loc,pkt->location, std::min(TRACE2_NET_LEN,ONESEC_PKT_NET_LEN));
  mapLC(trace_hdr->net, trace_hdr->loc,ASCII);
  strcpy(trace_hdr->datatype,(my_wordorder == SEED_BIG_ENDIAN) ? "s4" : "i4");
  trace_hdr->quality[1] = 0;

  trace_hdr->version[0] = TRACE2_VERSION0;
  trace_hdr->version[1] = TRACE2_VERSION1;

  /* finally, get the pinno from the linked list */
  if ( (pin_from_scn = getPinFromSCNL(trace_hdr->sta,
				     trace_hdr->chan,trace_hdr->net,trace_hdr->loc, 
				     cfg)) != -1) {
    trace_hdr->pinno = pin_from_scn;
  }

  // Convert data from network byte order to our byteorder
  int *Trace_Data = (int *)(trace_buffer.msg+sizeof(TRACE2_HEADER));
  for (int i=0; i<rate; i++) {
    Trace_Data[i] = ntohl(pkt->samples[i]);
  }
  *out_message_size = sizeof (TRACE2_HEADER) + (rate * sizeof(int));

  return(TN_SUCCESS);
} 
