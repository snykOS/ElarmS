/***********************************************************

File Name :
	sdecode.c

Programmer:
	Phil Maechling

Description:
	This is a static decode version of Doug N's decode header routine.
	It ignores blockettes.

Creation Date:
	22 July 1996

Usage Notes:



Modification History:

**********************************************************/
#include    <stdio.h>
#include    <stdlib.h>
#include    <memory.h>
#include    <string.h>
#include    <time.h>
#include    <math.h>
#include    "qlib2.h"
#include    "nscl.h"
//#include    "sdr_utils.h"

//#define	DATA_HDR_IND	'D'
//#define	VOL_HDR_IND	'V'
#define TN_SUCCESS       0
#define TN_FAILURE      -1

int  herrno;			/*  errno from header routines.		*/

/************************************************************************/
/*  decode_hdr_sdr_static:						*/
/*	Decode SDR header stored with each DRM data block,		*/
/*	All processing uses static memory allocations */
/*      Only works on 512 byte Steim 1 packets */
/************************************************************************/
int decode_hdr_sdr_static (DATA_HDR* ohdr, SDR_HDR* ihdr, int* pblksize)
{
    char		tmp[80];
    char		*pc;
    int			i, next_seq;
    int			seconds, usecs;

    /* Perform data integrity check, and pick out pertinent header info.*/
    herrno=0;

    if (!(ihdr->data_hdr_ind == DATA_HDR_IND || 
          ihdr->data_hdr_ind == VOL_HDR_IND)) 
    {
	/*  Don't have a DATA_HDR_IND.  See if the entire header is	*/
	/*  composed of NULLS.  If so, print warning and return NULL.	*/
	/*  Some early Quanterras output a spurious block with null	*/
	/*  header info every 16 blocks.  That block should be ignored.	*/
	if (allnull((char *)ihdr, sizeof(SDR_HDR))) 
        {
	    return(TN_FAILURE);
	}
	else 
        {
	    herrno = 1;
	    return (TN_FAILURE);
	}
    }

    /* Handle volume header */
    if (ihdr->data_hdr_ind == VOL_HDR_IND) 
    {
	return (TN_FAILURE);
    }

    memset ((void *)ohdr, 0, sizeof(DATA_HDR));
    ohdr->seq_no = atoi (charncpy (tmp, ihdr->seq_no, 6) );

    charncpy (ohdr->station_id, ihdr->station_id, 5);
    charncpy (ohdr->location_id, ihdr->location_id, 2);
    charncpy (ohdr->channel_id, ihdr->channel_id, 3);
    charncpy (ohdr->network_id, ihdr->network_id, 2);
    trim (ohdr->station_id);
    trim (ohdr->location_id);
    trim (ohdr->channel_id);
    trim (ohdr->network_id);
    ohdr->hdrtime = ohdr->begtime = decode_time_sdr(ihdr->time,
						    SEED_BIG_ENDIAN);

    if( (ohdr->begtime.year < 0 ) ||
        (ohdr->begtime.second < 0) ||
        (ohdr->begtime.usec < 0) )
    {

      fprintf(stdout,"MSHEAR Time decoded invalidly with : %s %s %s %s %d %d \n",
		ohdr->network_id,ohdr->station_id,ohdr->channel_id,mapLC(ohdr->network_id,ohdr->location_id,ASCII),
	        ohdr->begtime.second,
                ohdr->begtime.usec);
      
      mapLC(ohdr->network_id,ohdr->location_id,MEMORY);
      fflush(stdout);
      return(TN_FAILURE);
    }

	
    ohdr->num_samples = ihdr->num_samples;

    if( ohdr->num_samples < 1)
    {
      fprintf(stdout,"MSHEAR Found too few samples %s %s %s %s %d :\n",
		ohdr->network_id,ohdr->station_id,ohdr->channel_id,mapLC(ohdr->network_id,ohdr->location_id,ASCII),
	        ohdr->num_samples);
      mapLC(ohdr->network_id,ohdr->location_id,MEMORY);
      fflush(stdout);
      return(TN_FAILURE);
    }

    ohdr->sample_rate_mult = ihdr->sample_rate_mult;
    ohdr->sample_rate = eval_rate(ihdr->sample_rate_factor, 
				  ihdr->sample_rate_mult);

    /*	WARNING - may need to convert flags to independent format	*/
    /*	if we ever choose a different flag format for the DATA_HDR.	*/

    ohdr->activity_flags = ihdr->activity_flags;
    ohdr->io_flags = ihdr->io_flags;
    ohdr->data_quality_flags = ihdr->data_quality_flags;

    ohdr->num_blockettes = ihdr->num_blockettes;
    ohdr->num_ticks_correction = ihdr->num_ticks_correction;
    ohdr->first_data = ihdr->first_data;
    ohdr->first_blockette = ihdr->first_blockette;
    ohdr->data_type = 0;		/* assume unknown datatype.	*/

    /*	If the time correction has not already been added, we should	*/
    /*	add it to the begtime.  Do NOT change the ACTIVITY flag, since	*/
    /*	it refers to the hdrtime, NOT the begtime/endtime.		*/

    if ( ohdr->num_ticks_correction != 0 && 
	((ohdr->activity_flags & ACTIVITY_TIME_GAP) == 0) ) 
    {
	ohdr->begtime = add_dtime (ohdr->begtime, 
				   (double) ohdr->num_ticks_correction *
					USECS_PER_TICK);
    }

    time_interval2(ohdr->num_samples - 1, ohdr->sample_rate,
		   ohdr->sample_rate_mult,
		   &seconds, &usecs);

    if((seconds < 0) || (usecs < 0))
    {
      fprintf(stdout,"MSHEAR calculated negative usec with : %s %s %s %s %d %d \n",
		ohdr->network_id,ohdr->station_id,ohdr->channel_id,mapLC(ohdr->network_id,ohdr->location_id,ASCII),seconds,usecs);
      mapLC(ohdr->network_id,ohdr->location_id,MEMORY);
      fflush(stdout);
      return(TN_FAILURE);
    }

    ohdr->endtime = add_time(ohdr->begtime, seconds, usecs);

    /*	Process any blockettes that follow the fixed data header.	*/
    /*	If a blockette 1000 exists, fill in the datatype.		*/
    /*	Otherwise, leave the datatype as unknown.			*/

    ohdr->data_type = STEIM1;
    ohdr->num_blockettes = 0;

    *pblksize = 512;

    /* Return NULL if we don't have a data block. */

    if (ihdr->data_hdr_ind != DATA_HDR_IND) 
    {
      return(TN_FAILURE);
    }
    return(TN_SUCCESS);
}
