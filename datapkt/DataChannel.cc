#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>		        // errno, strerror
#include <regex.h>              // regular expression for channel filter

#include "plog/Log.h"           // plog logging

#include "Duration.h"
#include "DataChannel.h"
#include "readraw.h"
#include "globals.h"
#include "WPProperties.h"
#include "Configuration.h"
#include "Compat.h"

// 01 CAF 2013-08-08 -- ignore all channels if station is turned off

using namespace std;

const string RCSID_DataChannel_cc = "$Id$";

WPMap DataChannel::eewmap = WPMap();
ChannelConfigList DataChannel::chanlist = ChannelConfigList();
ChannelMap DataChannel::chmap = ChannelMap();
ChannelTimedMap DataChannel::chtmap = ChannelTimedMap();
ChannelTimedMap DataChannel::chbookmarks = ChannelTimedMap();
map<string,float> DataChannel::chanthresmap = map<string,float>();
map<string,float> DataChannel::chanazimuthmap = map<string,float>();
map<string,float> DataChannel::chandipmap = map<string,float>();
WPVector DataChannel::eewRec = WPVector();

TimeStamp DataChannel::whitelist_checktime = TimeStamp::current_time();
TimeStamp DataChannel::blacklist_checktime = TimeStamp::current_time();
TimeStamp DataChannel::greylist_checktime = TimeStamp::current_time();
bool DataChannel::whitelist_firstreadflag = false;
bool DataChannel::blacklist_firstreadflag = false;
bool DataChannel::greylist_firstreadflag = false;
int DataChannel::origin_time = 0;
pthread_mutex_t DataChannel::lock = PTHREAD_MUTEX_INITIALIZER;
string DataChannel::channel_file_signature = "";
string DataChannel::greylist_file_signature = "";
string DataChannel::blacklist_file_signature = "";

string ChannelFlagName(ChannelFlag val) {
    if (val == UNDEFINED) return "UNDEFINED";
    else if (val == WHITE) return "WHITE";
    else if (val == GREY) return "GREY";
    else if (val == BLACK) return "BLACK";
    return "INVALID";
}

// returns a string containing space delimited values for network, station, channel, location, latitude, longitude,  elevation, samplerate, gain,
string ChannelToString(Channel chan) {
    char chanstr[MAXSTR];
    sprintf (chanstr, "%s %s %s %s %9.4f %9.4f %7.1f %8.2f %13.6e", chan.network, chan.station, chan.channel, chan.location, chan.latitude, chan.longitude, chan.elevation, chan.samprate, chan.gain);
    return std::string(chanstr);
}

int DataChannel::addChannelsToWPMap(ChannelConfigList channelList, Channel chanZ, WPFactory* pFactory) {
    int numchannels = 0; //number of channels added to the wpmap
    bool isChanE = false;
    bool isChanN = false;
    string dot = ".";
    TimeStamp curtime = TimeStamp::current_time();
    
    
    //Define the E and N channels for every Z channel using only the sncl. Do not clone the metadata (gain, samplerate, lat, lon). 
    //Channel chanE = chanZ;    //This clones the metadata and should not be used. 
    Channel chanE = Channel(chanZ.network, chanZ.station, chanZ.location, chanZ.channel);
    chanE.channel[2] = 'E';
    isChanE = findChannel(channelList, chanE, chanE);

    //Channel chanN = chanZ;    //This clones the metadata and should not be used. 
    Channel chanN = Channel(chanZ.network, chanZ.station, chanZ.location, chanZ.channel);
    chanN.channel[2] = 'N';
    isChanN = findChannel(channelList, chanN, chanN);

    WP *eew = pFactory->createInstance(chanZ,chanE,chanN);

    string sncl_prefix = chanZ.network+dot+chanZ.station+dot+chanZ.location+dot;

    string sncl = sncl_prefix + chanZ.channel;
    LOGD <<"Adding channel "<< sncl << " to the eewmap" << std::endl;
    eewmap[sncl] = eew;
    chbookmarks[sncl] = curtime;
    numchannels++;
    
    //Only if metadata for channel E or N is present then add lookup key to eewmap
    if (isChanE) {
        sncl = sncl_prefix + chanE.channel;
        LOGD <<"Adding channel "<< sncl << " to the eewmap" << std::endl;
        eewmap[sncl] = eew;
        chbookmarks[sncl] = curtime;
        numchannels++;
    }
    else {
        LOGW << "**** Metadata not present for channel " << chanE << ". Not adding to eewmap ****" << endl;
    }
    if (isChanN) {
        sncl = sncl_prefix + chanN.channel;
        LOGD <<"Adding channel "<< sncl << " to the eewmap" << endl;
        eewmap[sncl] = eew;
        chbookmarks[sncl] = curtime;
        numchannels++;
    }
    else {
        LOGW << "**** Metadata not present for channel " << chanN << ". Not adding to eewmap ****" << endl;
    }
    return numchannels;
}// end addChannelsToWPMap

int DataChannel::removeChannelsFromWPMap(Channel chanZ) {
    Channel chanE = chanZ;
    Channel chanN = chanZ;
    string dot = ".";
    int numchannels = 0;
    
    //Not fetching the metadata for E and N because all we plan to do is use the channel name for the erase. 
    chanE.channel[2] = 'E';
    chanN.channel[2] = 'N';
    
    //Remove from the eewmap if present (the Z should be present, but the N and E may not be). 
    //All 3 key entries (when present) have the same WP *eew in the value. Deleting eew once is enough, 
    //trying to do it again raises segmentation fault.
    string sncl_prefix = chanZ.network+dot+chanZ.station+dot+chanZ.location+dot;
    string sncl = sncl_prefix + chanZ.channel;
    
    WPMap::iterator it  = eewmap.find(sncl);
    if (it != eewmap.end()) {
        LOGI << "Removing channel " << sncl << " from eewmap"<<endl;
        delete (*it).second;
        eewmap.erase(it);
        numchannels++;
    }
    sncl = sncl_prefix + chanE.channel;
    it  = eewmap.find(sncl);
    if (it != eewmap.end()) {
        LOGI << "Removing channel " << sncl << " from eewmap"<<endl;
        eewmap.erase(it);
        numchannels++;
    }
    
    sncl = sncl_prefix + chanN.channel;
    it  = eewmap.find(sncl);
    if (it != eewmap.end()) {
        LOGI << "Removing channel " << sncl << " from eewmap"<<endl;
        eewmap.erase(it);
        numchannels++;
    }
    return numchannels;
} // end removeChannelsFromWPMap

DataChannel::DataChannel(){
}

DataChannel::~DataChannel(){
}

WP* DataChannel::getWPHandle(Channel ch){
  WP* retvalue = NULL;
  char sncl[64];
  sprintf(sncl,"%s.%s.%s.%s",ch.network,ch.station,ch.location,ch.channel);
  pthread_mutex_lock(&lock);
  WPMap::iterator it  = eewmap.find(sncl);
  if(it != eewmap.end()){
    retvalue = (*it).second;
  }
  pthread_mutex_unlock(&lock);
  return retvalue;
}

WP* DataChannel::getWPHandle(string sncl){
  WP* retvalue = NULL;
  pthread_mutex_lock(&lock);
  WPMap::iterator it  = eewmap.find(sncl);
  if(it != eewmap.end()){
    retvalue = (*it).second;
  }
  pthread_mutex_unlock(&lock);
  return retvalue;
}

void DataChannel::getWPList(vector<WP*>& eewlist){
  pthread_mutex_lock(&lock);
  eewlist.clear();
  for(WPMap::iterator it = eewmap.begin();it != eewmap.end(); ++it){
    eewlist.push_back((*it).second);
  }
  pthread_mutex_unlock(&lock);
}

void DataChannel::writePacket_raw(const char* buf) throw(Error){
  unsigned int sec,usec;
  unsigned int nsamp;

  //print_packet(buf);
  //return;

  // print_packet(buf);
  int* data = readraw_data((char*)buf,sec,usec,nsamp);

  timeval samp_time;
  samp_time.tv_sec =  sec;
  samp_time.tv_usec = usec;

  char net[16],sta[16],chan[16],loc[16];
  read_chancomp((char*)buf,net,sta,chan,loc);

  string dot = ".";
  string sncl = string(net)+dot+string(sta)+dot+string(loc)+dot+string(chan);


  //double delay = (double)(TimeStamp::current_time()-TimeStamp(UNIX_TIME,samp_time));
  //LOGD <<sncl<<" TimeStamp : "<<TimeStamp(UNIX_TIME,samp_time)<<" Delay: "<<delay<<endl;


  Channel channel(net,sta,loc,chan);
    try{
        WP* eew = getWPHandle(channel);
	if(eew){
	  //chbookmarks[sncl] = TimeStamp::current_time();
	  //LOGD <<"Putting packet to "<<chan<<" WP . Sample Rate:"<<get_sample_rate(buf)<<endl;
	  //return;
	  eew->setSampleRate(get_sample_rate(buf));
	  eew->process(channel,TimeStamp(UNIX_TIME,samp_time),data,nsamp);
	}
	else{
	  // LOGE <<"ERROR: Can't find "<<channel<<" WP object in the map"<<endl;
	}
    }
    catch(Error& e){
	    LOGE << e.printStr();
    }
}

void DataChannel::writePacket(const char* buf) throw(Error){
    if(!buf)
	return;
    DATA_HDR	*mseed_hdr;	  /* qlib2 mseed_header */
    SDR_HDR       *shdr;
    int           wordorder;
    //char loc[MAX_CHARS_IN_CHANNEL_STRING];
    int rawdata[MAX_SAMPLES_IN_PACKET]; 
    
    int samp;
    INT_TIME itime;

    //sanity checks//
    shdr = (SDR_HDR *)buf;
    if (shdr->data_hdr_ind != DATA_HDR_IND) {
	Error e("Invalid packet type detected");
	throw e;
    }
    if ((wordorder = wordorder_from_time((unsigned char *)&(shdr->time))) == MS_ERROR) {
	Error e("Invalid packet header time detected");
	throw e;
    }


    //#ifdef DEBUG
    //    LOGD <<"Writing packet for "<<getPrintableName()<<endl;
    //#endif

  // Note that the mseed_hdr is a dynamically allocated struct of 
  // approx. 100 bytes
  if( (mseed_hdr=decode_hdr_sdr ((SDR_HDR *)buf,PACKET_SIZE)) == NULL) {
    Error e("Fatal Error decoding trace");
    throw e;
  }

  if(!mseed_hdr){
      throw Error("Unable to decode miniseed header");
  }

  if(!isDataPacket(mseed_hdr)){
    //    LOGD  <<"Packet for "<<getPrintableName()<<" is NOT a data packet"<<endl;
      free_data_hdr(mseed_hdr);      
      throw Error();
  }
  //  LOGD  <<"Packet for "<<getPrintableName()<<" IS a data packet"<<endl;

  // Check for a reasonable number of samples
  if ((mseed_hdr->num_samples <= 0) || 
      (mseed_hdr->num_samples > MAX_SAMPLES_IN_PACKET)) {
    //      char* msg = Compat::Form("In %s Packet has invalid number of samples. %d Samples",getPrintableName(),mseed_hdr->num_samples);
      free_data_hdr(mseed_hdr);
      throw Error();
  }

  // Check for an invalid sample rate
  if (mseed_hdr->sample_rate == 0 || mseed_hdr->sample_rate_mult == 0 ||
      mseed_hdr->num_samples == 0)  {
    /* not true message size! */
    Error e(Compat::Form("Sample rate invalid for %s",mseed_hdr->channel_id));
    free_data_hdr(mseed_hdr);
    throw e;
  }

  /* uncompress the packet */
  samp = ms_unpack(mseed_hdr, mseed_hdr->num_samples, (char *)buf, 
			  rawdata);
  if (samp == MS_ERROR) {
    Error e(Compat::Form("Unable to decompress miniseed packet for %s",mseed_hdr->channel_id));
    free_data_hdr(mseed_hdr);
    throw e;
  }

  if (samp != mseed_hdr->num_samples) {
    Error e(Compat::Form("Number of samples mismatch for %s",mseed_hdr->channel_id));
    free_data_hdr(mseed_hdr);
    throw e;
  }

  //
  // Convert internal time to timeval for stream call
  //
  itime = mseed_hdr->begtime; // internal time
  timeval samp_time;
  samp_time.tv_sec = unix_time_from_int_time(itime);
  samp_time.tv_usec = itime.usec;

  if(samp_time.tv_usec < 0){
      Error e(Compat::Form("MSHEAR Samp_time found to be negative usec : %d",samp_time.tv_usec));
      free_data_hdr(mseed_hdr);
      throw e;
  }


    trim(mseed_hdr->location_id);
    mapLC(mseed_hdr->network_id,mseed_hdr->location_id,MEMORY);
    // string dot = ".";
    //  string sncl = string(mseed_hdr->network_id)+dot+string(mseed_hdr->station_id)+dot+string(mseed_hdr->location_id)+dot+mseed_hdr->channel_id;
    Channel chan(mseed_hdr->network_id,mseed_hdr->station_id,mseed_hdr->location_id,mseed_hdr->channel_id);

    try{
      WP* eew = getWPHandle(chan);
      if(eew){
	//chbookmarks[sncl] = TimeStamp::current_time();
	eew->setSampleRate(sps_rate(mseed_hdr->sample_rate,
				    mseed_hdr->sample_rate_mult));
	// LOGD  <<"Putting SEED packet to "<<chan<<" WP"<<endl;
	eew->process(chan,TimeStamp(UNIX_TIME,samp_time),rawdata,samp);
      }
    }
    catch(Error& e){
	    LOGE << e.printStr();
    }
 
    free_data_hdr(mseed_hdr);
}


bool DataChannel::isDataPacket(DATA_HDR *hdr){
    BS *bs = hdr->pblockettes;
    int itype = DAT_INDEX;
    int b1000 = 0;
    int b2000 = 0;
    int n = -1;

    /* Determine the type of packet based on the blockette types.   */
    /* Assume it is a data packet unless proven otherwise.	    */
    /* Check packets in this order:				    */
    /* 2xx => events		*/
    /* 3xx => calibration	*/
    /* 5xx => timing		*/
    /* sample rate = 0 => log	*/
    while (bs != (BS *)NULL) {
        n = bs->type;
	if (n >= 200 && n < 300) {
	    itype = DET_INDEX;
	    return false;
	}
	else if (n >= 300 && n <= 400) {
	    itype = CAL_INDEX;
	    return false;
	}
	else if (n >= 500 && n < 600) {
	    itype = CLK_INDEX;
	    return false;
	}
	else if (n == 1000) {
	    /* Note that we found a blockette 1000, but keep scanning.	*/
	    b1000 = 1;
	}
	else if (n == 2000) {
	    /* Note that we found a blockette 2000, but keep scanning.	*/
	    b2000 = 1;
	}
	else {
	    /* Unknown or unimportant blockette - skip it */
	}
	bs = bs->next;
    }
    /* LOG channel is any channel still identified as a data channel	*/
    /* but with a sample rate of 0 and non-zero sample count.		*/
    if (itype == DAT_INDEX && hdr->sample_rate == 0 && hdr->num_samples != 0) {
	itype = LOG_INDEX;
	return false;
    }
    if (itype == DAT_INDEX && hdr->sample_rate == 0 && hdr->num_samples == 0 && b2000) {
	itype = BLK_INDEX;
	return false;
    }
    /* EVERY data packet should have a blockette 1000.			*/
    if (itype == DAT_INDEX && b1000 == 0) {
	    LOGF << "Unknown blockette/packet type " << n << " for " << hdr->station_id << " "
             << hdr->location_id << " " << hdr->channel_id << " " << hdr->network_id 
             << std::endl;
	    return false;
    }
    /* Ignore empty end-of-detection packets.	    */
    if (itype == DAT_INDEX && hdr->sample_rate == 0 && hdr->num_samples == 0){
      return false;
    }

    return true;
}

void DataChannel::setChannelFlags(string filename,ChannelFlag type) throw(int){
    Configuration cfg(filename.c_str());
    char line[1024];
    ValueList vl;
    
    if(!cfg){
      LOGF <<"ERROR: File IO error occured with " << filename<<endl;
      throw (-1);
    }
    //Clear channels with channelflag of passed in type
    ChannelMap::iterator cmi = chmap.begin();
    while (cmi != chmap.end()){
        if ((*cmi).second == type) {
            ChannelMap::iterator erasecmi = cmi;
            ++cmi;
            chmap.erase(erasecmi);
        } else {
            ++cmi;
        }
    }

    int ret;
    while((ret=cfg.next(line))==TN_SUCCESS){
        string chanstr = line;

        // trim leading and trailing white spaces as well as comment '#'
        size_t first = chanstr.find_first_not_of(" \t");
        size_t end   = chanstr.find_first_of(" \t#");
        chanstr = chanstr.substr(first, (end - first));

        if(chanstr.size()<=12){
            //log format error//
            LOGW << "Invalid line in Channel List file " << filename << endl;
            continue;//Because its not a critical error//
        }
        
        chmap[chanstr] = type;
        LOGI <<"Reading ["<<chanstr<<"] into "<< ChannelFlagName(type) << " list" << endl;
    }
}

ChannelFlag DataChannel::getChannelFlag(Channel ch){ 

//Print Current Channels
//   TimeStamp curtime = TimeStamp::current_time();
//   for(ChannelTimedMap::iterator it = chbookmarks.begin(); it != chbookmarks.end(); ++it){
//     if((double)(curtime - (*it).second) < 5.0){
//       LOGD  <<"CURRENT AT "<<curtime<<" "<<(*it).first<<endl;
//     }
//   }

  char chname[64];  
  sprintf(chname,"%s.%s.%s.%s",ch.network,ch.station,ch.channel,ch.location);
  ChannelFlag cf = WHITE;

  ChannelMap::iterator i = chmap.find(chname);
  // LOGD  <<" Trying to find "<<chname<<" from channel map"<<endl;
  if(i != chmap.end()){
    cf = (*i).second;
//    LOGD <<"FOUND "<<ch<<" in the flag list , mode:"<<cf<<endl;
  }

#if 0  // 01 CAF 2013-08-08 -- ignore all channels if station is turned off
  char chan_prefix[3];
  memset(chan_prefix,'\0',sizeof(chan_prefix));
  strncpy(chan_prefix,ch.channel,sizeof(chan_prefix)-1);
  chan_prefix[2] = '\0';
  if(string(chan_prefix) == "HH")
#endif // 01 CAF 2013-08-08 -- ignore all channels if station is turned off

  if(isStationOff(ch.network,ch.station)==true){
//      LOGD <<"Station "<<ch<<" is OFF. Returning BLACK channel flag"<<endl;
      return BLACK;
  }
  return cf;
}

// This is here temporarily till dependent code is updated. Use the other variant 'bool getClipping(Channel ch, float& clip)'
float DataChannel::getClipping(Channel ch){
    char sncl[64];
    float clip;
    sprintf(sncl,"%s.%s.%s.%s",ch.network,ch.station,ch.channel,ch.location);
    
    //If unable to find a matching sncl return NOT_CLIPPED=-1 to mean 'unknown clipping'.
    pthread_mutex_lock(&lock);
    if (chanthresmap.find(sncl) == chanthresmap.end()) {
        clip = NOT_CLIPPED;
    }
    else {
        clip = chanthresmap[sncl];
    }
    pthread_mutex_unlock(&lock);
    return clip;
}

// Return true if clipping is set for a channel. False if channel is missing from map or there is no default set (e.g. acceleration and displacement)
bool DataChannel::getClipping(Channel ch, float& clip){
    char sncl[64];
    bool found = false;
    sprintf(sncl,"%s.%s.%s.%s",ch.network,ch.station,ch.channel,ch.location);
    
    //If unable to find a matching sncl return false to mean 'unknown clipping'.
    pthread_mutex_lock(&lock);
    if (chanthresmap.find(sncl) != chanthresmap.end() && chanthresmap[sncl] != NOT_CLIPPED) {
        clip = chanthresmap[sncl];
        found = true;
    }

    pthread_mutex_unlock(&lock);
    return found;

}
// Return true if azimuth is set for a channel. False if channel is missing from map or the value for azimuth is missing.
bool DataChannel::getAzimuth(Channel ch, float& azimuth ){
    char sncl[64];
    bool found = false;
    sprintf(sncl,"%s.%s.%s.%s",ch.network,ch.station,ch.channel,ch.location);
    
    //Check that it exists and is set before setting the reference to azimuth
    pthread_mutex_lock(&lock);
    if (chanazimuthmap.find(sncl) != chanazimuthmap.end() && chanazimuthmap[sncl] != FLOAT_NOTSET ) {
        azimuth = chanazimuthmap[sncl];
        found = true;
    }
    pthread_mutex_unlock(&lock);
    return found;
}

// Return true if dip is set for a channel. False if channel is missing from map or the value for dip is missing.
bool DataChannel::getDip(Channel ch, float& dip){
    char sncl[64];
    bool found = false;
    sprintf(sncl,"%s.%s.%s.%s",ch.network,ch.station,ch.channel,ch.location);
    
    //Check that it exists and is set before setting the reference to dip
    pthread_mutex_lock(&lock);
    if (chandipmap.find(sncl) != chandipmap.end() && chandipmap[sncl] != FLOAT_NOTSET ) {
        dip = chandipmap[sncl];
        found = true;
    }
    pthread_mutex_unlock(&lock);
    return found;
}

bool DataChannel::readChannelData(WPFactory* pFactory) {
    WPProperties * prop;
    prop = WPProperties::getInstance();
    
    //Read files if they have changed 
    pthread_mutex_lock(&lock);    
    
    // check if it is first time file is read, else ensure that file read interval !=0 (0 means disable refresh) and that the interval has passed since last check. 
    if (!whitelist_firstreadflag || (prop->getReadIntervalWhiteList() != 0 &&
          (double)(TimeStamp::current_time() - whitelist_checktime) > prop->getReadIntervalWhiteList())) {
//        LOGI <<"Checking white list file at " << TimeStamp::current_time() << endl;

        string newchannel_file_signature = GetFileSignature(prop->getChannelFile());
        if (channel_file_signature.compare(newchannel_file_signature) != 0) { 
            LOGI <<"First load, or change detected in channel file. Refresh channel metadata" <<endl;
            int status = readChannels(pFactory, prop->getChannelFile());
            
            //Check that the channel config file has been read at least once. Subsequent failure to read file is not an error.
            if (status == TN_FAILURE) {
                if (!whitelist_firstreadflag){
                    LOGF << "ERROR: Error reading channel file." << endl;
                    pthread_mutex_unlock(&lock); 
                    return false;
                }
                else {
                    LOGW << "No channel config available. Continue using previously loaded channels" << endl;
                }
            }
            else {
                channel_file_signature = newchannel_file_signature;
                whitelist_firstreadflag = true;
            }
        } // end change detected in file
        whitelist_checktime = TimeStamp::current_time();

    } // end interval check and reading a whitelist channel file
    
    // Do not read file if filename equals NONE
    if (strcasecmp(prop->getChannelGreyList().c_str(), "none") == 0) {
        greylist_file_signature = "NONE";
    }
    // check if it is first time file is read, else ensure that file read interval !=0 (which means disable refresh) and that the interval has passed since last read. 
    else if (!greylist_firstreadflag || (prop->getReadIntervalBlackGreyList() != 0 &&
          (double)(TimeStamp::current_time() - greylist_checktime) > prop->getReadIntervalBlackGreyList())) {
//        LOGI << "Checking grey list file at " << TimeStamp::current_time() << std::endl;
                
        string newgreylist_file_signature = GetFileSignature (prop->getChannelGreyList());
        try{
            if (greylist_file_signature.compare(newgreylist_file_signature) != 0) {
                setChannelFlags(prop->getChannelGreyList(),GREY);
                greylist_file_signature = newgreylist_file_signature;
                greylist_firstreadflag = true;
            }
        }
        catch(int& e){
          LOGW << "Unable to read channel grey list files" << std::endl;
        }
        greylist_checktime = TimeStamp::current_time();        
    }
    // Do not read file if filename equals NONE
    if (strcasecmp(prop->getChannelBlackList().c_str(), "none") == 0) {
        blacklist_file_signature = "NONE";
    }
    else if (!blacklist_firstreadflag || (prop->getReadIntervalBlackGreyList() != 0 &&
          (double)(TimeStamp::current_time() - blacklist_checktime) > prop->getReadIntervalBlackGreyList())) {
//        LOGI << "Checking black list file at " << TimeStamp::current_time() << std::endl;
        string newblacklist_file_signature = GetFileSignature (prop->getChannelBlackList());
        try {
            if (blacklist_file_signature.compare(newblacklist_file_signature) != 0) {
                setChannelFlags(prop->getChannelBlackList(),BLACK);
                blacklist_file_signature = newblacklist_file_signature;
                blacklist_firstreadflag = true;
            }
        }
        catch(int& e){
          LOGI << "WARNING: Unable to read channel black list files" << std::endl;
        }
        blacklist_checktime = TimeStamp::current_time();
    }
    pthread_mutex_unlock(&lock);     
    return true;
} // DataChannel::readChannelData

int DataChannel::readChannels(WPFactory* pFactory, string channelfile){
    
    LOGI << "Reading channel file " << channelfile << std::endl;
    
    //Save the new channel list in a local var. 
    ChannelConfigList newchanlist = ChannelConfigList();
    
    //chanlist contains old channel list now, and will contain current list at the end of this function
    try {
        GetChannels(channelfile, newchanlist);
    }
    catch(Error& e){
      LOGF << e.printStr();
      return TN_FAILURE;
    }
    
    if(newchanlist.size()<=0){
      LOGF << "NO channels found" << std::endl;
      return (TN_FAILURE);
    }
    
    //Compare the z channels in chanlist with newchanlist and get 3 ChannelLists 
    //- addzlist, updzlist, delzlist since that is the starting point for the 3-tuple.
    //Current known issues : HN* is cloned into HL*. Can the cloning 
    // done in getChannels be removed?
    // Only checks for changes in Z channels, since that is the starting point for the eewmap. Ignores changes if made only in E or N channels. 
    // If changes to N and E are to be checked, then what should happen if no change to Z, but add/upd/del is done for N or E. Effect on eewmap to be considered.

    ChannelList oldzlist, newzlist, addzlist, updzlist, delzlist;
    
    //first load, chanlist empty, all new channels are to be added      
    if(chanlist.size() <= 0) {
        getAllZ(newchanlist, addzlist);        
    }
    else {
        getAllZ(chanlist, oldzlist);
        getAllZ(newchanlist, newzlist);
        
        //present in old, but missing in new, save in delzlist
        for(ChannelList::iterator it = oldzlist.begin(); it != oldzlist.end(); it++) {
            Channel oldz = (*it);
            Channel newz = Channel();
            
            if (!findChannel(newchanlist, oldz, newz)) {
                delzlist.push_back(oldz);
                LOGI << "Channel to be deleted: " << ChannelToString(oldz) << std::endl;
            }
        }// end loop that checks for deleted channels
        
        //present in new, could be 'no changes' or 'update' or 'new'. 
        for(ChannelList::iterator it = newzlist.begin(); it != newzlist.end(); it++) {
            Channel newz = (*it);
            Channel oldz = Channel();
            if (findChannel(chanlist, newz, oldz)) {
                //check if there is change in metadata 
                if (!(newz.latitude == oldz.latitude && newz.longitude == oldz.longitude && newz.samprate == oldz.samprate && newz.gain == oldz.gain)) {
                    updzlist.push_back(newz);
                    LOGI << "Channel to be updated: " << ChannelToString(newz) << std::endl;
                }
            }
            else {
                addzlist.push_back(newz);
                LOGI << "Channel to be added: " << ChannelToString(newz) << std::endl;
            }
        }// end loop that checks for new or updated channels
    }

    //Go over newly added channels, create factory instance and add to eewmap
    for(ChannelList::iterator it = addzlist.begin();it!=addzlist.end();it++){
        Channel chanZ = (*it);
        addChannelsToWPMap(newchanlist, chanZ, pFactory); 
    } 

    //Go over updated channels
    for(ChannelList::iterator it = updzlist.begin();it!=updzlist.end();it++){
        Channel chanZ = (*it);

        //Delete from the eewmap and then create again. There is no function currently to update WP instance.
        removeChannelsFromWPMap(chanZ);
        addChannelsToWPMap(newchanlist, chanZ, pFactory);         
    }

    //Go over deleted channels and remove them from the eewmap. This assumes that NE are also to be removed alongwith Z.
    for(ChannelList::iterator it = delzlist.begin();it!=delzlist.end();it++){
        Channel chanZ = (*it);
        removeChannelsFromWPMap(chanZ);
    }

    //point chanlist to the newlist.
    chanlist = newchanlist;
    LOGI << "Number of channels in chanlist:" << chanlist.size() << endl;
    LOGI << "Number of entries in eewmap:" << eewmap.size() << endl;

    return (TN_SUCCESS);
}//end readChannels


void DataChannel::getAllZ(ChannelConfigList &cl,ChannelList& clres){
  //  LOGD  <<"Reading Z Channels"<<endl;
    for(ChannelConfigList::iterator it = cl.begin();it!=cl.end();it++){
	Channel chnl = (*it).first;
	if(chnl.channel[2] == 'Z'){
	  //LOGD  <<"Adding to list:  "<<chnl<<endl;
	  clres.push_back(chnl);	
	}
    }  
}

// Search for a channel with sncl matching what is in chansearch. If a channel is found then it returns true and the found channel is passed by reference in chanmatch
bool DataChannel::findChannel(ChannelConfigList &cl,Channel chansearch, Channel &chanmatch){

    for(ChannelConfigList::iterator it = cl.begin();it!=cl.end();it++){
        Channel chnl = (*it).first;
        if(chnl == chansearch) {
            chanmatch = chnl;
            return true;
        }
    }
    return false;
}
/* Commented out because its behavior is erratic. If no channel matches chan then it returns the channel it matched the last time. 
Channel DataChannel::findChannel(ChannelConfigList &cl,Channel chan){
    
    for(ChannelConfigList::iterator it = cl.begin();it!=cl.end();it++){
        Channel chnl = (*it).first;
        if(chnl == chan)
            return chnl;
    }
}
*/
regex_t* DataChannel::pChannel_regex = NULL;

void DataChannel::SetChannelFilter(const string pattern) {
    const char* str = pattern.c_str();
    if (strcasecmp(str, "none") == 0 || strcasecmp(str, "off") == 0) {
        if (pChannel_regex != NULL) regfree(pChannel_regex);
        LOGI << "ChannelFilter is off" << endl;
    } else {
        LOGI << "ChannelFilter is " << pattern << endl;
        pChannel_regex = (regex_t*)malloc(sizeof(regex_t));
        // regcomp returns 0 if ok
        if (regcomp(pChannel_regex, pattern.c_str(), REG_EXTENDED|REG_NOSUB|REG_ICASE) != 0) {
            LOGE << endl << "Error trying to compile channel filter '" << pattern << "'" << endl;
            exit(0);
        }
    }

} // DataChannel.SetChannelFilter

void DataChannel::GetChannels(string channel_file, ChannelConfigList &cl) throw(Error)
{
    ChannelConfig config;
    Channel chan;
    char line[MAXSTR];
    string net, sta, location, channel, gain_units;
    float lat, lon, elev, samprate;
    double gain, gain_cgs;
    int numread;
    const char *configstr = "";
    FILE* fp;
    vector<string> columns;
    map<string, string> values;
    char emsg[MAXSTR];
    float threshold, azimuth, dip;
    string thres;
    bool ismks;
    
    map<string,bool> chandupcheckmap; //Map of channels used to check for duplicate entries. 
    
    // Get default values for ground motion threshold
    float velthreshold, accthreshold, dispthreshold;
    velthreshold = WPProperties::getInstance()->getVelocityThreshold();
    
    //Property velthreshold missing. Report a warning. 
    if (!velthreshold) {
        velthreshold = NOT_CLIPPED; //  prevent clip = 0 error
        LOGW << "*** Velocity threshold config property not set." << endl;
    }

    accthreshold = NOT_CLIPPED;  //Do not to assign a default for acceleration. Instead use -1 to indicate that this channel is NOT CLIPPED.
    dispthreshold = NOT_CLIPPED; //Do not to assign a default for displacement. Instead use -1 to indicate that this channel is NOT CLIPPED.
    if((fp = fopen(channel_file.c_str(), "r")) == NULL) {
        throw Error("Cannot open channel file: " + channel_file + "\n" + strerror(errno));
    }

    int count = 0; int skipped = 0; int errskipped =0;
    int inserted = 0;
    int lineno = 0;
    int overwritten = 0;
    
    while(fgets(line, sizeof(line), fp)) {
        //strip the newline char at the end of the line.
        size_t linelen = strlen(line);
        if (line[linelen - 1] == '\n' || line[linelen - 1] == '\r') line[linelen - 1] = '\0';
        if (line[linelen - 2] == '\n' || line[linelen - 2] == '\r') line[linelen - 2] = '\0';
        
        lineno++;
        threshold = 0;
        if (strstr(line, "#columns:")) {
            LOGI << "Columns:" << line << " [gain in cgs]" << endl;
            getColumns(line+9, columns);        // override columns
            continue;
        }
        if(line[0] == '#' or strlen(line) <= 1) {
            LOGI << line << endl;
            continue;
        }
        //Make a copy of line, because readLine modifies it.
        char tknbuf[MAXSTR];
        memset(tknbuf, '\0', sizeof(tknbuf));
        strncpy(tknbuf, line, sizeof(tknbuf)-1);

        if (columns.size() == 0) {
            const char* default_columns = "network station location channel latitude longitude elevation samplerate gain units";
            LOGI << "# Forcing default columns" << endl;
            LOGI << "# " << default_columns << " [gain in cgs]" << endl;

            strcpy(line, default_columns);
            getColumns(line, columns);
        }

        if (not readLine(tknbuf, columns, values)) {
            snprintf(emsg, sizeof(emsg), "Error: Missing columns on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }        
        net = values["network"];
        sta = values["station"];
        location = values["location"];
        channel = values["channel"];
        

        numread = sscanf(values["latitude"].c_str(), "%f", &lat);
        if (!numread) {
            snprintf(emsg, sizeof(emsg), "Error: Invalid latitude on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }

        numread = sscanf(values["longitude"].c_str(), "%f", &lon);
        if (!numread) {
            snprintf(emsg, sizeof(emsg), "Error: Invalid longitude on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }

        numread = sscanf(values["elevation"].c_str(), "%f", &elev);
        if (!numread) {
            snprintf(emsg, sizeof(emsg), "Error: Invalid elevation on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }
        numread = sscanf(values["samplerate"].c_str(), "%f", &samprate);
        if (!numread) {
            snprintf(emsg, sizeof(emsg), "Error: Invalid samplerate on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }

        // store original value of gain as read from file to print to log.
        numread = sscanf(values["gain"].c_str(), "%lf", &gain);
        if (!numread) {
            snprintf(emsg, sizeof(emsg), "Error: Invalid gain on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }

        // Convert gain into cgs system and store in gain_cgs
        // Currently expecting only earth units in the particular format described below. Update it units change.
        gain_units = values["units"];
        //Check if MKS unit convert to CGS
        if(!strcasecmp(gain_units.c_str(), "DU/M/S") 
                || !strcasecmp(gain_units.c_str(), "DU/M/S**2")
                || !strcasecmp(gain_units.c_str(), "DU/M")){
          gain_cgs = gain * 0.01;
          ismks = true;
        }
        // check if cgs, not an error, and no conversion required.
        else if(!strcasecmp(gain_units.c_str(), "counts/(cm/sec)") 
                || !strcasecmp(gain_units.c_str(), "counts/(cm/sec2)")
                || !strcasecmp(gain_units.c_str(), "counts/(cm)")) {
          gain_cgs = gain;
          ismks = false;
        }
        else {
            //not mks or cgs. raise  an error - unexpected unit
            snprintf(emsg, sizeof(emsg), "Error: Invalid gain unit '%s' on line %d: %s. ", gain_units.c_str(), lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }
        
        //Check that gain <> 0
        if (!gain_cgs) {
            snprintf(emsg, sizeof(emsg), "Error: Gain is zero on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }
        
        
        // Finished converting required columns and checking for errors. Create the channel object and sncl
        chan = Channel((char *)net.c_str(), (char *)sta.c_str(), mapLC((char *)net.c_str(), (char *)location.c_str(), ASCII), (char *)channel.c_str());
        char sncl[64];
        sprintf(sncl,"%s.%s.%s.%s",chan.network,chan.station,chan.channel,chan.location);

        chan.latitude = lat;
        chan.longitude = lon;
        chan.elevation = elev;
        chan.samprate = samprate;
        chan.gain = gain_cgs;
 
        // Read in the optional columns and save each in a map
        thres = values["groundmotionclip"];
        if (thres.size() == 0) thres = "UNKNOWN";           // if optional channel is not present, then assume UNKNOWN

        // If groundmotionclip value not present in file then use default value from properties file based on measurement type
        // groundmotionclip will have earth units (m, m/s, m/sec2, cm, cm/s, cm/sec2) in the same system as the gain.
        if (!strcasecmp(thres.c_str(), "UNKNOWN")) {
            if (!strcasecmp(gain_units.c_str(), "DU/M/S")
                || !strcasecmp(gain_units.c_str(), "counts/(cm/sec)")) {
                threshold = velthreshold;
            }
            else if (!strcasecmp(gain_units.c_str(), "DU/M/S**2")
                || !strcasecmp(gain_units.c_str(), "counts/(cm/sec2)")) {
                threshold = accthreshold;
            }
            else if (!strcasecmp(gain_units.c_str(), "DU/M")
                || !strcasecmp(gain_units.c_str(), "counts/(cm)")) {
                threshold = dispthreshold;
            }
        }
        else {
            numread = sscanf(thres.c_str(), "%f", &threshold);
            if (!numread) {
                snprintf(emsg, sizeof(emsg), "Error: Invalid threshold on line %d: %s", lineno, line);
                LOGE << emsg << endl;
                errskipped++;
                continue;
            }        
            // Convert mks values from file to cgs by multiplying by 100.            
            if (ismks) {
                threshold = threshold * 100;
            }
        }
        
        //Check that groundmotionclip <> 0
        if (!threshold) {
            snprintf(emsg, sizeof(emsg), "Error: Ground motion clip threshold is zero on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }
        chanthresmap[sncl] = threshold;
        
        //Check for duplicate sncl. 
        if (chandupcheckmap.find(sncl) == chandupcheckmap.end()) {
            chandupcheckmap[sncl] = true;
            
        }
        else {
            snprintf(emsg, sizeof(emsg), "Error: Duplicate entry for channel on line %d: %s", lineno, line);
            LOGE << emsg << endl;
            errskipped++;
            continue;
        }
        if (values.find("azimuth") == values.end()) {
            azimuth = FLOAT_NOTSET; 
        }
        else {
            numread = sscanf(values["azimuth"].c_str(), "%f", &azimuth);
            if (!numread) {
                snprintf(emsg, sizeof(emsg), "Error: Invalid azimuth on line %d: %s", lineno, line);
                LOGE << emsg << endl;
                errskipped++;
                continue;
            }
        }
        if (values.find("dip") == values.end()) {
            dip = FLOAT_NOTSET; 
        }
        else {
            numread = sscanf(values["dip"].c_str(), "%f", &dip);
            if (!numread) {
                snprintf(emsg, sizeof(emsg), "Error: Invalid dip on line %d: %s", lineno, line);
                LOGE << emsg << endl;
                errskipped++;
                continue;
            }
        }
        chanazimuthmap[sncl] = azimuth;
        chandipmap[sncl] = dip;
        
        strcpy(config.config, configstr);
        char s[MAXSTR];
        snprintf(s, sizeof(s), " [%13.6e]", chan.gain);

        // chan_prefix is the first two characters of the channel code, 
        // (the final third character indicates orientation)
        char chan_prefix[3];
        memset(chan_prefix,'\0',sizeof(chan_prefix));
        strncpy(chan_prefix,chan.channel,sizeof(chan_prefix)-1);
        chan_prefix[2] = '\0';

        // check for optional filtering
        // JA 2016/05/05 updating to use chan_prefix instead of full channel, 
        // otherwise can incorrectly match
        int skip = 0;
        if (pChannel_regex != NULL) {
            // regexec return 0 if match
            //if (skip = !(regexec(pChannel_regex, channel.c_str(), (size_t)0, NULL, 0) == 0)) {
            if ( (skip = !(regexec(pChannel_regex, chan_prefix, (size_t)0, NULL, 0)) == 0)) {
                strcat(s, " -- skipping channel");
                skipped++;
            }
        }
        if (!whitelist_firstreadflag) {
            LOGI << "[Processed]#" << lineno << ":" << line << s << endl;
        }
        if (skip) continue;

	// JA 2016/05/04, set instrument_type based on chan_prefix 
	// note type is set as SENSOR_UNKNOWN from constructor by default
        if (string(chan_prefix) == "HH" || string(chan_prefix) == "BH") {
            chan.instrument_type = SENSOR_BROADBAND;
        } else if (string(chan_prefix) == "HN" || string(chan_prefix) == "HL" || string(chan_prefix) == "EN" || string(chan_prefix) == "SN") {
            chan.instrument_type = SENSOR_STRONG_MOTION;
        } else if (string(chan_prefix) == "EH" || string(chan_prefix) == "SH") {
            chan.instrument_type = SENSOR_SHORT_PERIOD;
        }
        
        //HN* channels are cloned into HL*s. But if the file contains HL* channels then remove the cloned
        //channels and use the one from the datafile. 
        // cl.insert does not overwrite values. cl.erase has to be called first to remove old value.
        Channel chan2;
        if (findChannel(cl,chan, chan2)) {
            LOGI << "   Removed old (cloned) value for channel " << ChannelToString(chan2) << endl;
            cl.erase(chan);
            LOGI << "   Replaced with values read from file    " << ChannelToString(chan) <<endl;
            overwritten++;
        }
        cl.insert(ChannelConfigList::value_type(chan, config));

        if(string(chan_prefix) == "HN"){
            strncpy(chan.channel,"HL",2);
            //Do not clone if the channel is present in the ChannelConfigList
            if (!findChannel(cl, chan,chan2)){
                cl.insert(ChannelConfigList::value_type(chan, config));
                inserted++;
            }
        }

        count++;
    }
    LOGI << "DataChannel got " << count << " rows from the file" << endl;
    LOGI << "Inserted " << inserted << " rows due to HN->HL mapping" << endl;
    LOGI << "Overwrote " << overwritten << " rows due to distinct HL entries" << endl;
    LOGI << "Skipped " << skipped << " rows due to filters" << endl;
    LOGI << "Skipped " << errskipped << " rows due to error reading line" << endl;
    fclose(fp);
    if (errskipped > 0) {
       throw Error( "Error parsing file: " + channel_file);
    }
    chandupcheckmap.clear();
} // DataChannel::GetChannels(channel_file, cl)

bool DataChannel::getColumns(char *line, vector<string> &columns) throw(Error)
{
    map<string, int> names;
    map<string, int>::iterator it;
    char *tok, *last, *c;
    int n = 0;

    columns.clear();

#define REQUIRED -1
#define OPTIONAL -2

    tok = line;
    names[string("network")] = REQUIRED;
    names[string("station")] = REQUIRED;
    names[string("location")] = REQUIRED;
    names[string("channel")] = REQUIRED;
    names[string("latitude")] = REQUIRED;
    names[string("longitude")] = REQUIRED;
    names[string("elevation")] = REQUIRED;
    names[string("samplerate")] = REQUIRED;
    names[string("gain")] = REQUIRED;
    names[string("units")] = REQUIRED;
    names[string("groundmotionclip")] = OPTIONAL;
    names[string("azimuth")] = OPTIONAL;
    names[string("dip")] = OPTIONAL;
    

    while( (c = strtok_r(tok, " \t\n", &last)) ) {
        tok = NULL;
        if((it = names.find(c)) != names.end()) {
            if((*it).second >= 0) {
                throw( Error("DataChannel::GetColumns: column name repeated: " + string(c)) );
            }
            (*it).second = n;
        }
        columns.push_back(string(c));
        n++;
    }
    bool missing = false;
    std::stringstream os;
    for(it = names.begin(); it != names.end(); it++) {
        if((*it).second < 0) {
            os << (*it).first << " ";
            if((*it).second == REQUIRED)
                missing = true;
        }
    }
    if(missing) {
        throw( Error("DataChannel::GetColumns: missing column(s): " + os.str()) );
    }

    if (os.str().length() >> 0) {
        LOGW << endl << endl << "*** DataChannel::GetColumns optional column(s) not found: " + os.str() << endl << endl;
    }

    // print columns so we see what we're going to be parsing
//    LOGD << "New column order:";
//    for (int i = 0; i < columns.size(); i++) LOGD << " " << columns[i];
//    LOGD << endl;

    return true;
}

bool DataChannel::readLine(char *line, vector<string> &columns, map<string, string> &values)
{
    values.clear();
    int n = 0;      //column number
    char *tok, *c, *last;

    tok = line;

    while( (c = strtok_r(tok, " \t\n", &last)) != NULL && n < (int)columns.size()) {
        tok = NULL;
        values[columns[n]] = c;
        n++;
    }
    return (n == (int)columns.size()) ? true : false;
}


void DataChannel::turnStationOff(string net,string sta){
  TimeStamp remove_time = TimeStamp::current_time();

  //multithreaded- add lock here
  pthread_mutex_lock(&lock);
  chtmap[net+"."+sta] = remove_time;
  pthread_mutex_unlock(&lock);
}

void DataChannel::turnStationOn(string net,string sta){
  ChannelTimedMap::iterator it;
  string key = net+"."+sta;

  //multithreaded- add lock here
  pthread_mutex_lock(&lock);
  it = chtmap.find(key);
  if(it == chtmap.end()){
    pthread_mutex_unlock(&lock);
    return;
  }
  chtmap.erase(it);
  pthread_mutex_unlock(&lock);
}


bool DataChannel::isStationOff(string net,string sta){
  string key = net+"."+sta;
  bool isOff = true;
  ChannelTimedMap::iterator it;
  pthread_mutex_lock(&lock);

  it = chtmap.find(key);
  if(it == chtmap.end()){
    isOff = false;
  }
  pthread_mutex_unlock(&lock);
  return isOff;
}

string DataChannel::GetFileSignature(string filename) {
    char id_time[MAXSTR];
    struct stat buf;
    stat(filename.c_str(), &buf);    
    //combine deviceid, inode and modified time into a : separated string
    sprintf(id_time, "%u:%u:%ju", (unsigned int)buf.st_dev, (unsigned int)buf.st_ino, (uintmax_t)buf.st_mtime);
    return std::string(id_time);
}

string DataChannel::GetFileSignature(const ChannelFlag flag) {
    static WPProperties* prop = NULL;
    if (!prop) {
        prop = WPProperties::getInstance();
    }
    switch (flag) {
        case WHITE: return channel_file_signature;
        case GREY: return greylist_file_signature;
        case BLACK: return blacklist_file_signature;
        default: {
                     char errmsg[100];
                     sprintf(errmsg, "%s:%d:%s called with invalid channel flag %d", 
                             __FILE__, __LINE__, __func__, flag);
                     throw Error(errmsg);
                 }
    } // switch on flag
} // DataChannel::GetFileSignature

#ifdef UTDC
bool DataChannel::readChannelsUT(const string filename)
{
    try {
        ChannelConfigList myList = ChannelConfigList();
        GetChannels(filename, myList);
        /*
        This chunk of commented code is useful to write out the contents of the ChannelConfigList if required
        LOGI << "Channel list contents:" <<endl;
        for(ChannelConfigList::iterator it = myList.begin();it!=myList.end();it++){
            Channel chnl = (*it).first;
            ChannelConfig config = (*it).second;
            LOGI <<"    "<< chnl << " Gain:"<< chnl.gain << endl; 
        }
        */
        
        return true;
    } catch(Error& e) {
        LOGE <<"Error occurred when reading channels: " << e.printStr() << endl; 
    }
    return false;
} // DataChannel::readChannelsUT

class DC : public WP{   
  public:
    bool createModules();
    void reset();
    void record(int d);
    void stoprecord();
    
    DC(Channel Z,Channel E,Channel N);
    virtual ~DC();
    class Factory : public WPFactory {
        DC* createInstance(Channel z, Channel e, Channel n) {
            return new DC(z, e, n);
        }
    }; // class DC::Factory
}; // class DC


DC::DC(Channel Z,Channel E,Channel N) :WP(Z,E,N){
    return;
}

bool DC::createModules(){
    return true;
}

void DC::reset(){
    return;
}

void DC::stoprecord(){
    return;
}

void DC::record(int d) {
    return;
}
DC::~DC() {
    LOGD << __FILE__ << ":" << __LINE__ << ": destructor called" << endl;
}
//If one arg then read channels once. 
// if 2 args then 2nd arg tells how many seconds to run the script. Max is 5 minutes
int main (int argc, char* argv[]) {

    cout << "Unit test for " << RCSID_DataChannel_cc << endl;
    if(argc < 2) {
      cout << "Usage: " << argv[0] << " <chanfile.dat>" <<endl;
      cout << "    or " << argv[0] << " <wp_onsite.cfg> <duration in seconds>" <<endl;
      exit(0);
    }
    
    // enable print locking to minimize corrupted logs
    //pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
    //WPLib::setPrintLock(&print_lock);
    
    if (argc == 2) {
        LOGD << "DataChannel: Start unit test to read channels " << endl;

        string filename(argv[1]);
        bool okay = DataChannel::readChannelsUT(filename);
        LOGD << "DataChannel: read of file " << filename << " is " << (!okay ? "not " : "") << "okay" << endl;
        exit( okay ? 0 : 1);
    }

    LOGD << "DataChannel: Start unit test to monitor channel files " << endl;
     
    try {
        WPProperties* prop;
        prop = WPProperties::getInstance();
        prop->init(argv[1]);
        WPFactory* pFactory = new DC::Factory();      
        int duration = 300;
        if (argc == 3) {
            duration = atoi(argv[2]); 
            if (duration > 300) duration = 300;
        }
        string whiteSig = "undefined";
        string greySig = "undefined";
        string blackSig = "undefined";
        for (int i=0; i<duration; i++) {
            string wSig = DataChannel::GetFileSignature(WHITE);
            string gSig = DataChannel::GetFileSignature(GREY);
            string bSig = DataChannel::GetFileSignature(BLACK);
            if (wSig != whiteSig || gSig != greySig || bSig != blackSig) {
                LOGD << "File signatures white|grey|black: " << wSig << "|" << gSig << "|" << bSig << endl;
                whiteSig = wSig; greySig = gSig; blackSig = bSig;
            }
            if ( !DataChannel::readChannelData(pFactory) ) {
                LOGF << "ERROR: Error reading channel metadata.  Aborting!" << endl;
                exit(1);
            }
            sleep(1);
        }
    }
    catch(Error& e) {
        LOGF <<"Error occurred when reading channels: " << e.printStr() << std::endl; 
        exit(1);
    }

    LOGD << "Finished unit test successfully."<<endl;
    
    exit(0);
}

#endif // UTDC
