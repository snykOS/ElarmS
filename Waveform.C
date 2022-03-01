/***********************************************************

File Name :
        Waveform.C

Original Author:
        Patrick Small

Description:


Creation Date:
        15 March 1999


Modification History:


Usage Notes:


**********************************************************/


// Various include files
#include <cstring>
#include <cstdlib>
#include <cmath>
#ifdef __SUNPRO_CC
#include <math.h>
#endif
#include "GenLimits.h"
#include "RetCodes.h"
#include "Waveform.h"
#include "Compat.h"
#include "utils.h"

using namespace std;

// Fraction of sample period error to allow between packet times
const double WAVEFORM_EPSILON = 0.10;


Waveform::Waveform()
{
  valid = TN_FALSE;
  init_qlib2(1);
  // Always make big-endian miniSEED, regardless of local hardware
  set_hdr_wordorder(SEED_BIG_ENDIAN);
  set_data_wordorder(SEED_BIG_ENDIAN);
  nextseg = dslist.begin();
}



Waveform::Waveform(const Waveform &w)
{
  valid = w.valid;
  chan = w.chan;
  dslist = w.dslist;
  nextseg = dslist.begin();
  recsize = w.recsize;
  format = w.format;
  init_qlib2(1);
  // Always make big-endian miniSEED, regardless of local hardware
  set_hdr_wordorder(SEED_BIG_ENDIAN);
  set_data_wordorder(SEED_BIG_ENDIAN);
}



Waveform::Waveform(const DataSegmentList &dl)
{
  DATA_HDR *dh;
  DataSegmentList::iterator dsp;
  DataSegmentList::iterator dspnext;
  int chanfound = TN_FALSE;
  int recsizefound = TN_FALSE;
  int formatfound = TN_FALSE;
  Channel curchan;
  int curformat;
  TimeStamp endtime;
  TimeWindow trimtime;
  double epsilon;
  Duration epsilon_dur;
  
  valid = TN_FALSE;
  dslist = dl;

  init_qlib2(1);
  // Always make big-endian miniSEED, regardless of local hardware
  set_hdr_wordorder(SEED_BIG_ENDIAN);
  set_data_wordorder(SEED_BIG_ENDIAN);

  // Sort the segment list
  dslist.sort();

  // Perform some verification on the segment list
  dsp = dslist.begin();
  dspnext = dsp;
  dspnext++;
  while (dsp != dslist.end()) {
    dh = decode_hdr_sdr((SDR_HDR *)((*dsp).data), (*dsp).len);
    if (dh == NULL) {
      std::cout << "Error (Waveform::Waveform): Unable to decode SEED header" 
	   << std::endl;
      return;
    }
    trim (dh->network_id);
    trim (dh->station_id);
    trim (dh->channel_id);
    trim (dh->location_id);
    curchan = Channel(dh->network_id, dh->station_id, mapLC(dh->network_id,dh->location_id,MEMORY),dh->channel_id);

    // Determine the sample rate
    if ((dh->sample_rate > 0) && (dh->sample_rate_mult > 0)) {
      curchan.samprate = (double)(dh->sample_rate * dh->sample_rate_mult);
    } else if ((dh->sample_rate > 0) && (dh->sample_rate_mult < 0)) {
      curchan.samprate = (double) ((double) dh->sample_rate) / 
	((double)(-1.0 * dh->sample_rate_mult));
    } else if ((dh->sample_rate < 0) && (dh->sample_rate_mult > 0)) {
      curchan.samprate = (double)((1.0/(double)(-1.0 * dh->sample_rate)) * 
				  dh->sample_rate_mult);
    } else if ((dh->sample_rate < 0) && (dh->sample_rate_mult < 0)) {
      curchan.samprate = (double)(1.0/(double)(-1.0 * dh->sample_rate)) / 
	(double)(-1.0 * dh->sample_rate_mult);
    } else if ((dh->sample_rate == 0) || (dh->sample_rate_mult == 0)) {
      std::cout << "Error (Waveform::Waveform): Invalid sample rate detected" 
	   << std::endl;
      free_data_hdr(dh);
      return;
    }

    curformat = dh->data_type;

    // Free the data header resources
    free_data_hdr(dh);

    // Verify the channel name and sample rate
    if (chanfound == TN_FALSE) {
      chan = curchan;
      epsilon = (1.0 / chan.samprate) * WAVEFORM_EPSILON;
	epsilon_dur = Duration(epsilon);
      chanfound = TN_TRUE;
    } else {
      if (!(chan == curchan)) {
	std::cout << "Error (Waveform::Waveform): Channel mismatch in segment list"
	     << std::endl;
	return;
      }

      if(lround(chan.samprate) != lround(curchan.samprate)) {
	std::cout << 
	  "Error (Waveform::Waveform): Sample rate mismatch in segment list" 
	     << std::endl;
	return;
      }

    }

    // Verify that there is data in the segment
    if ((*dsp).numsamp <= 0) {
      std::cout << "Error (Waveform::Waveform): Invalid number of samples in segment list" << std::endl;
      return;
    }

    // Verify the record size
    if (recsizefound == TN_FALSE) {
      switch ((*dsp).len) {
      case MINISEED_REC_512:
	recsize = MINISEED_REC_512;
	break;
      case MINISEED_REC_4096:
	recsize = MINISEED_REC_4096;
	break;
      default:
	std::cout << 
	  "Error (Waveform::Waveform): Unsupported record size of " 
	     << (*dsp).len << std::endl;
	return;
	break;
      };
      recsizefound = TN_TRUE;
    } else if ((*dsp).len != recsize) {
      std::cout << 
	"Error (Waveform::Waveform): Record size mismatch in segment list" 
	   << std::endl;
      return;
    }

    switch (curformat) {
    case STEIM1:
      curformat = MINISEED_FORMAT_STEIM1;
	break;
    case STEIM2:
      curformat = MINISEED_FORMAT_STEIM2;
      break;
    default:
      std::cout << 
	  "Error (Waveform::Waveform): Unsupported data format of " 
	   << curformat << std::endl;
      return;
      break;
    };
    
    // Verify the data format
    if (formatfound == TN_FALSE) {
      format = curformat;
      formatfound = TN_TRUE;
    } else {
      if (curformat != format) {
	std::cout << 
	  "Error (Waveform::Waveform): Data format mismatch in segment list" 
	     << std::endl;
	return;
      }
    }

    // If this packet and next overlap, erase ONLY the overlapping packet
    // and display a warning and go on to the next packet.
    // Gaps between packets are OK.
    if (dspnext != dslist.end()) {
      endtime = (*dsp).start + Duration((*dsp).numsamp / chan.samprate);
      if ((endtime > (*dspnext).start) 
	  && (fabs(Duration(endtime - (*dspnext).start)) > (double)epsilon_dur)) { //leapsecond change
	std::cout << "Warning (Waveform::Waveform): Adjacent packets overlap" 
	     << std::endl;
	std::cout << "First Packet  : " << (*dsp).start << "->" << endtime 
	     << "\tNum Samples : " << (*dsp).numsamp << std::endl;
	std::cout << "Second Packet : " << (*dspnext).start << "->..." 
	     << "\tNum Samples : " << (*dspnext).numsamp << std::endl;
	// dslist.erase(dspnext, dslist.end());
        DataSegmentList::iterator temp = dspnext;
        dspnext++;
        dslist.erase(temp);
	//break;
        continue;
      }
    }
    dsp++;
    dspnext++;
  }

  // Set the beginning of the next segment to the first packet in the list
  nextseg = dslist.begin();

  if (dslist.size() == 0) {
    std::cout << "Error (Waveform::Waveform): Waveform trimmed to zero segments"
	 << std::endl;
    return;
  }
  valid = TN_TRUE;
}


Waveform::Waveform(const DataArrayList &dl, Channel &newChan, 
		   int data_fmt = STEIM2, int mseed_blksize = 512)
{
    DataArrayList::const_iterator dap;
    DataSegment ds;    
    TimeStamp endtime;
    DATA_HDR *hdr, *dh;
    char *p_ms = NULL;
    int npacked, k, nblocks;
    int sample_rate_mult = 1;
    int sample_rate = 0;
    int seqno = 1;
    int ns;
    
    init_qlib2(1);
    // Always make big-endian miniSEED, regardless of local hardware
    set_hdr_wordorder(SEED_BIG_ENDIAN);
    set_data_wordorder(SEED_BIG_ENDIAN);

    valid = TN_FALSE;
    if (mseed_blksize > DS_MAX_SIZE) {
	std::cout << Compat::Form("Error (Waveform): mseed_blksize %d too large; max is %d", 
				  mseed_blksize, DS_MAX_SIZE);
	return;
    }
    if (TN_SUCCESS != _BuildRate(newChan.samprate, sample_rate, 
				 sample_rate_mult)) {
	std::cout << "Error (Waveform): Unable to build rate and rate_mult for "
		  << newChan.samprate << std::endl;
	return;
    }

    chan = newChan;
    recsize = mseed_blksize;
    format = data_fmt;
    
    int i(0);
    for (dap = dl.begin(); dap != dl.end(); dap++) {
	hdr = new_data_hdr();
	hdr->seq_no = seqno;
	strncpy (hdr->station_id, newChan.station, DH_STATION_LEN);
	strncpy (hdr->network_id, newChan.network, DH_NETWORK_LEN);
	strncpy (hdr->channel_id, newChan.channel, DH_CHANNEL_LEN);
	if (newChan.location[0] != ' ' && newChan.location[0] != '-')
	    strncpy (hdr->location_id, newChan.location, DH_LOCATION_LEN);
	hdr->hdrtime = tepoch_to_int(dap->start.ts_as_double(TRUE_EPOCH_TIME));
	hdr->begtime = hdr->hdrtime;
	hdr->sample_rate = sample_rate;
	hdr->sample_rate_mult = sample_rate_mult;
	hdr->data_type = data_fmt;
	hdr->blksize = mseed_blksize;
	hdr->xm1 = 0;

	npacked = ms_pack_data (hdr, 
				NULL, 
				dap->data.size(),
				const_cast<int *>(&(*dap->data.begin())), 
				&nblocks, 
				&p_ms, 
				0, 
				NULL);
	if (npacked != dap->data.size()) {
	    std::cout << "Error (Waveform): packing miniseed, sent " 
		      << dap->data.size()
		      << " samples, packed " << npacked << " samples" 
		      << std::endl;
	}
	
	for (ns = 0; ns < nblocks; ns++) {
	    dh = decode_hdr_sdr((SDR_HDR *)(p_ms + ns * mseed_blksize),
				mseed_blksize);
	    ds.len = mseed_blksize;
	    ds.numsamp = dh->num_samples;
	    ds.start = TimeStamp(TRUE_EPOCH_TIME, int_to_tepoch(dh->begtime));
	    memcpy(ds.data, p_ms + ns * mseed_blksize, mseed_blksize);
	    dslist.push_back(ds);
	    free_data_hdr(dh);
	}
	free_data_hdr(hdr);
	free(p_ms);
	p_ms = NULL;
	seqno += nblocks;
    }
    nextseg = dslist.begin();
    valid = TN_TRUE;
    
    return;
}

int Waveform::_BuildRate(double drate, int &rate, int &rate_mult)
{
    if (drate > 0.99) {
	/* Assume you want integer samples/second */
	rate = (int)(drate + .1);
	rate_mult = 1;
    }
    else {
	/* assume you want integer second/sample */
	drate = 1.0/drate + .1;
	rate = -1 * (int)drate;
	rate_mult = 1;
    }
    return(TN_SUCCESS);
}


Waveform::~Waveform()
{
  // Clear out all entries from the data segment list
  dslist.clear();
  valid = TN_FALSE;
}



int Waveform::_InsertCounts(DataCountList &dl, DataCounts &dc)
{
  DataCountList::iterator dcp;
  CountList::iterator cp;
  TimeStamp endtime;
  double epsilon;

  if (dl.size() > 0) {
    // Allow 1/10 sample period error before treating the difference
    // as a time gap.
    epsilon = (1.0 / chan.samprate) * WAVEFORM_EPSILON;
    dcp = dl.end();
    dcp--;
    endtime = (*dcp).start + Duration((*dcp).numsamp / chan.samprate);
    if (fabs(Duration(dc.start - endtime)) < epsilon) {
      // Append these samples to the end of the last element of the list
      for (cp = dc.data.begin(); cp != dc.data.end(); cp++) {
	(*dcp).data.insert((*dcp).data.end(), (*cp));
      }
      (*dcp).numsamp += dc.numsamp;
      return(TN_SUCCESS);
    }
  }
  
  // Append a new DataCounts element
  dl.insert(dl.end(), dc);

  return(TN_SUCCESS);
}



int Waveform::_GetSamples(DataSegment &ds, DataCounts &dc)
{
  DATA_HDR *dh;
  int retval;
  int *samples;
  int i;

  // Clear out any existing data counts
  dc.data.clear();

  // Unpack the SEED data header from the previous packet
  dh = decode_hdr_sdr((SDR_HDR *)(ds.data), ds.len);
  if (dh == NULL) {
    std::cout << 
      "Error (Waveform::_GetSamples): Unable to decode SEED header" 
	 << std::endl;
    return(TN_FAILURE);
  }

  // Unpack the SEED packet into an array of counts
  samples = new int[ds.numsamp];
  if (samples == NULL) {
    std::cout << "Error (Waveform::_GetSamples): Cannot allocated sample array" 
	 << std::endl;
    free_data_hdr(dh);
    return(TN_FAILURE);
  }
  retval = ms_unpack(dh, ds.numsamp, ds.data, (void *)(samples));

  // Release the resources used by the data header
  free_data_hdr(dh);

  if (retval != ds.numsamp) {
    std::cout << "Error (Waveform::_GetSamples): Unable to unpack all counts in previous packet" << std::endl;
    delete(samples);
    return(TN_FAILURE);
  }

  // Pack the data count object
  dc.start = ds.start;
  dc.numsamp = ds.numsamp;
  for (i = 0; i < ds.numsamp; i++) {
    dc.data.insert(dc.data.end(), samples[i]);
  }
  delete(samples);
  return(TN_SUCCESS);
};



// This method trims a SEED packet to the specified time window. The
// Time window and packet times are assumed to overlap.
int Waveform::_TrimPacket(DataSegment &ds, DataSegment &dsprev,
			  const TimeWindow &tw)
{
  DATA_HDR *dh;
  DATA_HDR *newdh;
  int *samples;
  int retval;
  struct timeval t;
  Duration timediff;
  TimeStamp newstart;
  int startsamp;
  int numsamp;
  INT_TIME ttime;
  int numblocks;
  char *msbuf;
  char **msbufptr;
  char errmsg[MAXSTR];
  int blksize;
  DataCounts dc;
  CountList::iterator clp;

  msbufptr = &msbuf;
  strcpy(errmsg, "");

  // Unpack the SEED data header
  dh = decode_hdr_sdr((SDR_HDR *)(ds.data), ds.len);
  if (dh == NULL) {
    std::cout << "Error (Waveform::_TrimPacket): Unable to decode SEED header" 
	 << std::endl;
    return(TN_FAILURE);
  }

  blksize = dh->blksize;

  // Configure the new SEED data header
  newdh = new_data_hdr();
  if (newdh == NULL) {
    std::cout << 
      "Error (Waveform::_TrimPacket): Unable to allocate new SEED header" 
	 << std::endl;
    free_data_hdr(dh);
    return(TN_FAILURE);
  }
  if(copy_data_hdr(newdh, dh)==NULL){ //Modified by Kalpesh
    std::cout << 
      "Error (Waveform::_TrimPacket): Unable to copy SEED header" 
	 << std::endl;
    free_data_hdr(dh);
    free_data_hdr(newdh);
    return(TN_FAILURE);    
  }

  // Unpack the SEED packet into an array of counts
  samples = new int[ds.numsamp];
  retval = ms_unpack(dh, ds.numsamp, ds.data, (void *)samples);

  // Release the resources used by the data header
  free_data_hdr(dh);

  if (retval != ds.numsamp) {
    std::cout << 
      "Error (Waveform::_TrimPacket): Unable to unpack all counts in packet" 
	 << std::endl;
    delete(samples);
    free_data_hdr(newdh);
    return(TN_FAILURE);
  }

  // Calculate new start sample, number of samples, and start time

  // CASE 1: Window overlaps with oldest samples in segment
  if (tw.start < ds.start) {
    startsamp = 0;
    newstart = ds.start;
    timediff = tw.end - ds.start;
    numsamp = ceil(fabs((double)timediff) * chan.samprate); //leapsecond change.

    // CASE 2: Window falls within packet, or overlaps with its
    // newest samples
  } else  {
    timediff = tw.start - ds.start;
    startsamp = ceil(fabs((double)timediff) * chan.samprate); //leapsecond change
    newstart = ds.start + (startsamp / chan.samprate);
    timediff = tw.end - ds.start;
    numsamp = ceil(fabs((double)timediff) * chan.samprate) - startsamp;  //leapsecond change
    // Adjust numsamp if window extends past newest samples
    if ((numsamp + startsamp) > ds.numsamp) {
      numsamp = ds.numsamp - startsamp;
    }
  }

  // Check if we have nothing to repack
  if (numsamp == 0) {
    ds.start = TimeStamp(TRUE_EPOCH_TIME, 0.0);
    ds.numsamp = 0;
    ds.len = 0;
    delete(samples);
    free_data_hdr(newdh);
    return(TN_SUCCESS);
  }

  // Initialize the changed fields within the new data header structure
  newdh->num_samples = numsamp;
  ttime = tepoch_to_int(newstart.ts_as_double(TRUE_EPOCH_TIME));
  newdh->begtime = ttime;
  newdh->hdrtime = ttime;

  // If there is a previous packet, we must record the last two samples
  // contained within it so that QLIB2 can compute a valid difference list
  // for the samples in the trimmed packet.
  if (!(ds == dsprev)) {
    if (this->_GetSamples(dsprev, dc) != TN_SUCCESS) {
      std::cout << "Error (Waveform::_TrimPacket): Unable to unpack counts in previous packet" << std::endl;
      delete(samples);
      free_data_hdr(newdh);
      return(TN_FAILURE);
    }
    clp = dc.data.end();
    clp--;
    newdh->xm1 = (*clp);
    if (dsprev.numsamp > 1) {
      clp--;
      newdh->xm2 = (*clp);
    } else {
      newdh->xm2 = newdh->xm1;
    }
  }

  *msbufptr = NULL;

  // Repack the trimmed data
  retval = ms_pack_data(newdh, NULL, numsamp, samples + startsamp, 
			&numblocks, msbufptr, 0, errmsg);

  delete(samples);
  free_data_hdr(newdh);

  if (*msbufptr == NULL) {
    std::cout << "Error (Waveform::_TrimPacket): MiniSEED buffer is NULL" << std::endl;
    return(TN_FAILURE);
  }
  
  if (retval != numsamp) {
    std::cout << "Error (Waveform::_TrimPacket): Error packing MiniSEED records - " 
	 << errmsg << std::endl;
    free(*msbufptr);
    return(TN_FAILURE);
  }

  // Update the data segment with the new sample segment
  ds.start = newstart;
  ds.numsamp = numsamp;
  ds.len = blksize;
  memcpy(ds.data, *msbufptr, ds.len);
  free(*msbufptr);
  return(TN_SUCCESS);
}



int Waveform::GetSEED(DataSegmentList &dl, int rsize, int dform)
{
  // Clear out any existing data segment entries
  dl.clear();

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }
  if (((rsize == recsize) && (dform == format))
      || ((rsize == MINISEED_REC_ORIGINAL) 
	  && (dform == MINISEED_FORMAT_ORIGINAL))) {
    dl = dslist;
    return(TN_SUCCESS);
  } else {
    std::cout << "Error (Waveform::GetSEED): Only ORIGINAL record size and data format supported by Waveform class" << std::endl;
    return(TN_FAILURE);
  }
}



int Waveform::GetCounts(DataCountList &dl)
{  
  DataSegmentList::iterator dsp;
  DataCounts dc;

  // Clear out any existing data in the return list
  dl.clear();

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  for (dsp = dslist.begin(); dsp != dslist.end(); dsp++) {
    if (this->_GetSamples((*dsp), dc) != TN_SUCCESS) {
      std::cout << "Error (Waveform::GetCounts): Unable to retrieve data counts" 
	   << std::endl;
      return(TN_FAILURE);
    }
    // Store the counts into the DataCount list
    if (this->_InsertCounts(dl, dc) != TN_SUCCESS) {
      std::cout << 
	"Error (Waveform::GetCounts): Unable to insert counts into count list"
	   << std::endl;
      return(TN_FAILURE);
    }
  }
  return(TN_SUCCESS);
}



double Waveform::GetSampleRate()
{
  double retval = 0.0;

  if (valid != TN_TRUE) {
    return(retval);
  }
  retval = chan.samprate;
  return(retval);
}



int Waveform::NumberOfSegments()
{
  DataSegmentList::iterator dsp;
  DataSegmentList::iterator dspold;
  double epsilon;
  TimeStamp endtime;
  int numseg = 0;
  
  if (valid != TN_TRUE) {
    return(numseg);
  }

  if (dslist.size() == 0) {
    return(numseg);
  }

  // Allow 1/10 sample period error before treating the difference
  // as a time gap.
  epsilon = (1.0 / chan.samprate) * WAVEFORM_EPSILON;  

  dspold = dslist.begin();
  dsp = dspold;
  dsp++;
  numseg++;
  while (dsp != dslist.end()) {
    endtime = (*dspold).start + Duration((*dspold).numsamp / chan.samprate);
    if (fabs(Duration((*dsp).start - endtime)) >= epsilon) {
      numseg++;
    }
    dspold = dsp;
    dsp++;
  };
  
  return(numseg);
}



int Waveform::SetFirstSegment()
{
  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }
  nextseg = dslist.begin();
  return(TN_SUCCESS);
}



int Waveform::Next(Waveform &w)
{
  DataSegmentList::iterator dsp;
  DataSegmentList::iterator dspold;
  double epsilon;
  TimeStamp endtime;

  w = Waveform();

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  if (nextseg == dslist.end()) {
    return(TN_EOF);
  }

  // Allow 1/10 sample period error before treating the difference
  // as a time gap.
  epsilon = (1.0 / chan.samprate) * WAVEFORM_EPSILON;  

  // Find the start and end packet of the next contiguous segment
  dspold = nextseg;
  dsp = nextseg;
  dsp++;
  for (; dsp != dslist.end(); dsp++) {
    endtime = (*dspold).start + Duration((*dspold).numsamp / chan.samprate);
    if (fabs(Duration((*dsp).start - endtime)) >= epsilon) {
      break;
    }
    dspold = dsp;
  }
  w = Waveform(DataSegmentList(nextseg, dsp));
  if (!w) {
    return(TN_FAILURE);
  }
  nextseg = dsp;

  return(TN_SUCCESS);
}



int Waveform::Trim(const TimeWindow &tw)
{
  DataSegmentList::iterator dsp;
  DataSegmentList::iterator dspprev;
  DataSegmentList::iterator dsptmp;
  TimeStamp endtime;
  double epsilon;
  Duration duration_zero = 0.0;

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }
  
  if ((tw.start <= TimeStamp(TRUE_EPOCH_TIME, 0.0)) || 
      (tw.end <= TimeStamp(TRUE_EPOCH_TIME, 0.0)) || (tw.end <= tw.start)) {
    std::cout << 
      "Error (Waveform::Trim): Invalid duration - time cannot be represented" 
	 << std::endl;
    return(TN_FAILURE);
  }

  epsilon = (1.0 / chan.samprate) * WAVEFORM_EPSILON;
  
  for (dsp = dslist.begin(); dsp != dslist.end();) {
    // Endtime is the time of the last sample in the packet
    endtime = (*dsp).start + Duration(((*dsp).numsamp - 1) / chan.samprate);

    // CASE 1: Packet falls outside of time window
    if ((tw.end <= (*dsp).start) || (tw.start > endtime)) {
      // Move nextseg iterator if it points to the deleted packet
      if (nextseg == dsp) {
	nextseg++;
      }
      dsptmp = dsp;
      dsp++;
      dslist.erase(dsptmp);
      
      // CASE 2: Packet falls completely within time window
    } else if ((tw.start <= (*dsp).start) && (tw.end > endtime)) {
      dsp++;
      
      // CASE 3: Packet partially overlaps with time window
    } else {

      // Find the previous intersecting packet. We will need it in order
      // to trim the current packet properly.
      dspprev = dsp;
      if (dsp != dslist.begin()) {
	dspprev--;
	endtime = (*dspprev).start + 
	  Duration((*dspprev).numsamp / chan.samprate);
	if (fabs(Duration((*dsp).start - endtime)) >= epsilon) {
	  dspprev = dsp;
	}
      }
      if (this->_TrimPacket((*dsp), (*dspprev), tw) != TN_SUCCESS) {
	std::cout << 
	  "Error (Waveform::Trim): Unable to trim data segment to time window"
	     << std::endl;
	return(TN_FAILURE);
      }
      
      // Check if packet was trimmed to have no remaining samples
      if ((*dsp).numsamp == 0) {
	// Move nextseg iterator if it points to the deleted packet
	if (nextseg == dsp) {
	  nextseg++;
	}
	dsptmp = dsp;
	dsp++;
	dslist.erase(dsptmp);
      } else {
	dsp++;
      }
    }
  }
  
  if (dslist.size() == 0) {
    valid = TN_FALSE;
    nextseg = dslist.begin();
  }
  return(TN_SUCCESS);
}


int Waveform::StartTime(TimeStamp &starttime)
{
  DataSegment ds;

  starttime = TimeStamp(TRUE_EPOCH_TIME, 0.0);

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  if (dslist.size() > 0) {
    ds = dslist.front();
    starttime = ds.start;
  }
  return(TN_SUCCESS);
}


int Waveform::EndTime(TimeStamp &endtime)
{
  DataSegment ds;

  endtime = TimeStamp(TRUE_EPOCH_TIME, 0.0);

  if (valid != TN_TRUE) {
    return(TN_FAILURE);
  }

  if (dslist.size() > 0) {
    ds = dslist.back();
    endtime = ds.start + Duration((ds.numsamp - 1) / chan.samprate);
  }
  return(TN_SUCCESS);
}


int Waveform::getSizeInBytes(){
    return (DS_MAX_WIRE)*dslist.size();
}


Waveform& Waveform::operator=(const Waveform &w)
{
  valid = w.valid;
  chan = w.chan;
  dslist = w.dslist;
  nextseg = dslist.begin();
  recsize = w.recsize;
  format = w.format;
  return(*this);
}



int operator!(const Waveform &w)
{
  return(!(w.valid));
}
