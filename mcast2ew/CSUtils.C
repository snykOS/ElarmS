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
#include <math.h>       // ldexp
#include <cstdio>
#include <ctime>
#include <cmath>
#include "RetCodes.h"
#include "CSUtils.h"



// Format specifier for data header time formatting
const int TIME_FMT = 1;


// Size of a standard miniseed packet
const int MSEED_PACKET_SIZE = 512;

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
dump_data_hdr (ewConfig &cfg, DATA_HDR *hdr)
{
  int  bl;
  
  logit((char *) "et", (char *) "%s: seqno = %d\n", cfg.Progname, hdr->seq_no);
  logit((char *) "et", (char *) "%s: station = %s\n", cfg.Progname, hdr->station_id);
  logit((char *) "et", (char *) "%s: location = %s\n", 
	cfg.Progname, mapLC(hdr->network_id, hdr->location_id,ASCII));
  mapLC(hdr->network_id, hdr->location_id,MEMORY);
  logit((char *) "et", (char *) "%s: channel = %s\n", cfg.Progname, hdr->channel_id);
  logit((char *) "et", (char *) "%s: network = %s\n", cfg.Progname, hdr->network_id);
  logit((char *) "et", (char *) "%s: hdrtime = %s\n", 
	cfg.Progname, date_string(hdr->hdrtime,(char*)"N/A"));
  logit((char *) "et", (char *) "%s: begtime = %s\n", 
	cfg.Progname, date_string(hdr->begtime, (char*)"N/A"));
  logit((char *) "et", (char *) "%s: endtime = %s\n", 
	cfg.Progname, date_string(hdr->endtime, (char*)"N/A"));
  logit((char *) "et", (char *) "%s: sample_rate = %d\n", cfg.Progname, hdr->sample_rate);
  logit((char *) "et", (char *) "%s: activity_flags = 0x%02x\n", cfg.Progname, 
	hdr->activity_flags);
  logit((char *) "et", (char *) "%s: io_flags = 0x%02x\n", cfg.Progname, hdr->io_flags);
  logit((char *) "et", (char *) "%s: data_quality_flags = 0x%02x\n", cfg.Progname, 
	hdr->data_quality_flags);
  logit((char *) "et", (char *) "%s: num_ticks_correction = %d\n", cfg.Progname, 
	hdr->num_ticks_correction);
  logit((char *) "et", (char *) "%s: num_blockettes = %d\n", cfg.Progname, hdr->num_blockettes);
  logit((char *) "et", (char *) "%s: first_blockette = %d\n", cfg.Progname, 
	hdr->first_blockette);
  logit((char *) "et", (char *) "%s: num_samples = %d\n", cfg.Progname, hdr->num_samples);
  logit((char *) "et", (char *) "%s: num_data_frames = %d\n", cfg.Progname, 
	hdr->num_data_frames);
  logit((char *) "et", (char *) "%s: x0 = %d\n", cfg.Progname, hdr->x0);
  logit((char *) "et", (char *) "%s: xn = %d\n", cfg.Progname, hdr->xn);

  // dump out blockettes
  if (hdr->pblockettes != NULL) {
    int next;
    int l;
    int type;
    BS *bs = hdr->pblockettes;
    do {
      BLOCKETTE_HDR *bh = (BLOCKETTE_HDR *)(bs->pb);
      switch (type=bs->type) {
      case 100:	
      case 200:	
      case 201:	
      case 300:	
      case 310:	
      case 320:	
      case 390:	
      case 400:	
      case 405:	
	logit((char *) "et", (char *) "%s: blockette %d\n", cfg.Progname, type);
	break;
      case 1000: 
	logit((char *) "et", 
	      (char *) "%s: blockette %d, fmt = %d (%s), order = %d (%s), len = %d (%d)\n",
	      cfg.Progname, type, 
	      ((BLOCKETTE_1000 *)bh)->format, 
	      (((BLOCKETTE_1000 *)bh)->format == STEIM1) ? "STEIM1" :
	      (((BLOCKETTE_1000 *)bh)->format == STEIM2) ? "STEIM2" : 
	      "unknown",
	      ((BLOCKETTE_1000 *)bh)->word_order, 
	      (((BLOCKETTE_1000 *)bh)->word_order == SEED_BIG_ENDIAN) ? "BIG_ENDIAN" :
	      (((BLOCKETTE_1000 *)bh)->word_order == SEED_LITTLE_ENDIAN) ? "LITTLE_ENDIAN" : 
	      "unknown",
	      ((BLOCKETTE_1000 *)bh)->data_rec_len, 
	      roundoff ( ldexp(1.,((BLOCKETTE_1000 *)bh)->data_rec_len) ));
		break;
      case 1001: 
	logit((char *) "et", (char *) "%s: blockette %d, clock_quality = %d, usec99 = %d, frame_count=%d\n",
	      cfg.Progname, type, 
	      ((BLOCKETTE_1001 *)bh)->clock_quality, 
	      ((BLOCKETTE_1001 *)bh)->usec99, 
	      ((BLOCKETTE_1001 *)bh)->frame_count);
	break;
      default:
	logit((char *) "et", (char *) "%s: unknown blockette %d, skipping\n", cfg.Progname, 
	      bh->type);
      }
      bs = bs->next;
    } while (bs != (BS *)NULL);
  }
  return;
}



int MSEEDToTrace(ewConfig &cfg, void *bytes, int len,
		   TracePacket &trace_buffer, long *out_message_size) 
{
  DATA_HDR	*mseed_hdr;	  /* qlib2 mseed_header */
  TRACE2_HEADER	*trace_hdr;	  /* ew trace header */
  int		num_samples;	  /* num samples from uncompression routine */
  int		pin_from_scn;	  /* return value to use from PINmap */
  SDR_HDR       *shdr;
  int           wordorder;

  // Check that the packet length is equal to that of a MiniSEED packet
  if (len != MSEED_PACKET_SIZE) {
    *out_message_size = 0;
    logit((char *) "et", (char *) "%s: Invalid MiniSEED packet length\n", cfg.Progname);
    return(TN_FAILURE);
  }

  // The SEED packet may be corrupted, so we have to perform several
  // fundamental checks before decoding it. The QLIB2 library handles 
  // errors by exiting the program, which is behavior that we do not want.
  shdr = (SDR_HDR *)bytes;
  if (shdr->data_hdr_ind != DATA_HDR_IND) {
    *out_message_size = 0;
    logit((char *) "et", (char *) "%s: Invalid packet type detected\n", cfg.Progname);
    return(TN_FAILURE);
  }

//   if (wordorder_from_time_safe((unsigned char *)&(shdr->time), 
// 				&wordorder) != TRUE) {
//     *out_message_size = 0;
//     logit((char *) "et", (char *) "%s: Invalid packet header time detected\n", cfg.Progname);
//     return(TN_FAILURE);
//   }

  //Modified by Kalpesh//
  wordorder = wordorder_from_time((unsigned char *)&(shdr->time));
  if (wordorder==MS_ERROR){
    *out_message_size = 0;
    logit((char *) "et", (char *) "%s: Invalid packet header time detected\n", cfg.Progname);
    return(TN_FAILURE);
  }

  // Note that the mseed_hdr is a dynamically allocated struct of 
  // approx. 100 bytes
  if( (mseed_hdr=decode_hdr_sdr ((SDR_HDR *)bytes, len)) == NULL) {
    *out_message_size=0;
    logit((char *) "et", (char *) "%s: Fatal Error decoding trace\n", cfg.Progname);
    return(TN_FAILURE);
  }

  // Check for a reasonable number of samples
  if ((mseed_hdr->num_samples <= 0) || (mseed_hdr->num_samples > 
      ((MAX_TRACEBUF_SIZ - sizeof(TRACE2_HEADER)) / sizeof(int)))) {
    *out_message_size = 0;
    if (cfg.Verbose) {
	// moved the logit complaint inside here to minimize bogus messages
      logit((char *) "et", (char *) "%s: %s.%s.%s Packet has invalid number of samples for conversion (%d)\n", 
	  cfg.Progname, mseed_hdr->network_id, mseed_hdr->station_id, mseed_hdr->channel_id, mseed_hdr->num_samples);
      dump_data_hdr(cfg, mseed_hdr);
    }
    free_data_hdr(mseed_hdr);
    return(TN_FAILURE);
  }

  *out_message_size = sizeof(TRACE2_HEADER) +
    (sizeof(int)*mseed_hdr->num_samples);

  // Check for an invalid sample rate
  if (mseed_hdr->sample_rate == 0 || mseed_hdr->sample_rate_mult == 0 ||
      mseed_hdr->num_samples == 0)  {
    if (strcmp(mseed_hdr->channel_id, "ACE")==0 || strcmp(mseed_hdr->channel_id, "LOG")==0) {
    	free_data_hdr(mseed_hdr);
    	return(TN_FAILURE);
    }
    /* not true message size! */
    *out_message_size = 80;
    logit((char *) "et", (char *) "%s: %s.%s.%s Sample rate invalid srate=%d srate_mult=%d nsamp=%d\n", 
		cfg.Progname, mseed_hdr->network_id, mseed_hdr->station_id, mseed_hdr->channel_id,
		mseed_hdr->sample_rate, mseed_hdr->sample_rate_mult, mseed_hdr->num_samples);
    if (cfg.Verbose) {
      dump_data_hdr(cfg, mseed_hdr);
    }
    free_data_hdr(mseed_hdr);
    return(TN_FAILURE);
  }

  /* uncompress the packet */
  num_samples = ms_unpack(mseed_hdr, mseed_hdr->num_samples, (char *)bytes, 
			  &trace_buffer.msg[sizeof(TRACE2_HEADER)]);

  //Modified by Kalpesh//
  if (num_samples<=0) {
    *out_message_size=0;
    logit((char *) "et", (char *) "%s: Unable to decompress miniseed packet\n", cfg.Progname);
    if (cfg.Verbose) {
      dump_data_hdr(cfg, mseed_hdr);
    }
    free_data_hdr(mseed_hdr);
    return(TN_FAILURE);
  }

  if (num_samples != mseed_hdr->num_samples) {
    *out_message_size=0;
    logit((char *) "et", (char *) "%s: Number of samples mismatch\n", cfg.Progname);
    if (cfg.Verbose) {
      dump_data_hdr(cfg, mseed_hdr);
    }
    free_data_hdr(mseed_hdr);
    return(TN_FAILURE);
  }

  trace_hdr = (TRACE2_HEADER *) &trace_buffer.trh;
  memset((void*) trace_hdr, 0, sizeof(TRACE2_HEADER));
  trace_hdr->pinno = 0;		/* Unknown item */
  trace_hdr->nsamp = mseed_hdr->num_samples;
  // note that unix_time_from_int_time() does not handle leap_seconds 
  // secs=60 should this miraculously occur on the start time of a 
  // data packet... 
  trace_hdr->starttime = (double)unix_time_from_int_time(mseed_hdr->begtime) +
    ((double)(mseed_hdr->begtime.usec)/USECS_PER_SEC);
  trace_hdr->endtime = (double)unix_time_from_int_time(mseed_hdr->endtime) +
    ((double)(mseed_hdr->endtime.usec)/USECS_PER_SEC);

  // Verify that the endtime is after the start time
  if (trace_hdr->endtime < trace_hdr->starttime) {
    *out_message_size=0;
    logit((char *) "et", (char *) "%s: Packet end time is before start time\n", cfg.Progname);
    if (cfg.Verbose) {
      dump_data_hdr(cfg, mseed_hdr);
    }
    free_data_hdr(mseed_hdr);
    return(TN_FAILURE);
  }

  trace_hdr->samprate = sps_rate(mseed_hdr->sample_rate,
				 mseed_hdr->sample_rate_mult);
  strcpy(trace_hdr->sta,trim(mseed_hdr->station_id));
  strcpy(trace_hdr->net,trim(mseed_hdr->network_id));
  strcpy(trace_hdr->chan,trim(mseed_hdr->channel_id));
  trim(mseed_hdr->location_id);
  mapLC(mseed_hdr->network_id, mseed_hdr->location_id,MEMORY);
  strcpy(trace_hdr->loc,mseed_hdr->location_id);
  strcpy(trace_hdr->datatype,(my_wordorder == SEED_BIG_ENDIAN) ? "s4" : "i4");
  trace_hdr->quality[1] = (char)mseed_hdr->data_quality_flags;

  trace_hdr->version[0] = TRACE2_VERSION0;
  trace_hdr->version[1] = TRACE2_VERSION1;

  /* finally, get the pinno from the linked list */
  if ( (pin_from_scn = getPinFromSCNL(trace_hdr->sta,
				     trace_hdr->chan,trace_hdr->net,trace_hdr->loc, 
				     cfg)) != -1) {
    trace_hdr->pinno = pin_from_scn;
  }

  free_data_hdr(mseed_hdr);

  return(TN_SUCCESS);
} 
