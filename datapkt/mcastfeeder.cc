#include "WPProperties.h"
#include "mcastfeeder.h"
#include "notify_eew.h"


using namespace std;

const string RCSID_mcastfeeder_cc = "$Id$";

int  total_thread_count;
pthread_t  thread_array[512];
  
PacketBuffer sb;
PacketQueueMap pktqmap;

pcap_t * handle;
PacketBufferQueue* pktq;


extern "C" {
static void* process_packet_wrapper(void* data)
{
    return mcastfeeder::process_packet(data);
} // process_packet_wrapper
} // extern "C"


extern "C" {
void got_packet(u_char *args, const struct pcap_pkthdr *header,
		const u_char *packet){
  
  struct iphdr *ipheader = (struct iphdr*)(packet + sizeof(struct ethernethdr));
  struct udphdr *udpheader = (struct udphdr*)((int)ipheader + (int)sizeof(struct iphdr));
  struct in_addr ip_addr;
  struct in_addr ip_addr_src;
  static int pktcount = 0;
  static queue< PacketBuffer >* qptr = NULL;
  WPProperties*   prop = WPProperties::getInstance();
  
  if(ipheader->ip_p==IP_P_UDP){
    //  LOGD <<"Got packet! "<<endl;
    //  return;

      bzero(&ip_addr,sizeof(struct in_addr));
      ip_addr.s_addr = ipheader->ip_dst;
      ip_addr_src.s_addr = ipheader->ip_src;

      string dstip = string(inet_ntoa(ip_addr));

      sb.arrivaltime = TimeStamp::current_time();
      if(dstip == prop->getMulticastAddress()){
	sb.type = SEED_PACKET;
	sb.size = COMP_PACKET_SIZE;
      }
      else if(dstip == prop->getQMAMulticastAddress()){
	sb.type = QMA_PACKET;
	sb.size = UNCOMP_PACKET_SIZE;
      }
      else{
	return;
      }

      memcpy(sb.get(),(char*)((int)udpheader + sizeof(struct udphdr)),sb.size);
      //int dstport = htons(udpheader->uh_dport);
      //int numthreads = prop->getThreadCount();
      //int index = dstport%numthreads;

      Channel ch;
      try{
	if(sb.type == SEED_PACKET){
	  ch = getChannelFromMSEED(sb.get());
	  // return;
	}
	else if(sb.type == QMA_PACKET){
	  ch = getChannelFromRaw(sb.get());
	  //LOGD <<"Got Station Name: "<<ch<<endl;
	  // return;
	}
      }catch(Error &e){
	e.print();
	return;
      }

      char sn_str[64];
      sprintf(sn_str,"%s.%s.%s",ch.network,ch.station,mapLC(ch.network,ch.location,ASCII));
      string staname = sn_str;
      
      PacketQueueMap::iterator pit = pktqmap.find(staname);
      if(pit == pktqmap.end()){
	if(total_thread_count == 0){
	  //	  pktq = new PacketQueue();
// 	  if(pktq==NULL){
// 	    LOGW <<"WARNING: Can't create packet queue"<<endl;
// 	    return;
// 	  }
	  
	  total_thread_count++;
      LOGI <<"Creating a new thread for "<<staname<<", Total Thread Count: "<<total_thread_count<<endl;
	  if(pthread_create(&thread_array[total_thread_count-1],NULL,process_packet_wrapper,(void*)pktq)<0){
        LOGF <<"ERROR: Error Creating thread"<<endl;
        delete pktq;
        return;
	  }
	}
	pktqmap[staname] = pktq;
      }
      else{
	pktq  = (*pit).second;
      }
      pktq->push(sb,BLOCKED);      
  }
} // got_packet
} // extern "C"


void mcastfeeder::start(string config, int orgtime, WPFactory* pFactory) throw(Error){
  char *dev;			 
  char errbuf[PCAP_ERRBUF_SIZE]; 
  struct bpf_program fp;	 
  char filter_exp[128];	         
  bpf_u_int32 mask;		 
  bpf_u_int32 net;		 
  struct pcap_pkthdr header;	 
  const u_char *packet;		 

  WPProperties* prop;
  try{
    prop = WPProperties::getInstance();
    prop->init(config);
  }
  catch(Error& e){
    e.print();
    throw e;
  }

#ifdef NOTIFY_EEW
    //Start CMS Thread//
    LOGI << "mcastfeeder is calling start_notify_eew_listener..." << endl;
    start_notify_eew_listener_thread();
#else
    LOGW << "*** NOTIFY_EEW thread disabled ***" << endl;
#endif

  
  //Initialization
  try{
    DataChannel::setChannelFlags(prop->getChannelGreyList(),GREY);
    DataChannel::setChannelFlags(prop->getChannelBlackList(),BLACK);
  }
  catch(int& e){
    LOGF <<"ERROR: Unable to read channel list files"<<endl;
    throw e;
  }
  pktq = new PacketBufferQueue();

  //set all queue pointers to NULL
  //  for(int i=0;i<MAX_THREADS;i++){
  //  pktqmap[i] = NULL;
  //}

  // check for optional filters
  string pattern = prop->getChannelFilter();
  if (pattern.length() > 0)
      DataChannel::SetChannelFilter(pattern);

  //populate channels//
  int status = DataChannel::readChannels(pFactory, prop->getChannelFile());
  if (status == TN_FAILURE) {
      LOGF << "ERROR: No channel config available.  Aborting!" << endl;
      exit(2);
  }

  init_qlib2(1);

  string IFID = prop->getIFID();
  dev = (char*)IFID.c_str();
  
  int pid_A = fork();
  string destip = "";
  if(pid_A<0){
    LOGF <<"Error forking the process A"<<endl;
    throw Error();
  }
  
#if 1
  if(pid_A > 0){
      destip = prop->getQMAMulticastAddress();
  }
  else{
    destip = prop->getMulticastAddress();
  }
#else
  if(pid_A > 0){
    int pid_B = fork();
    if(pid_B<0){
      LOGF <<"Error forking the process B"<<endl;
      throw Error();
    }    
    if(pid_B > 0){
      while(1){
	sleep(1);
      }
      if(kill(pid_A,SIGTERM)<0){
	perror("WARNING: Child process is still running. Please kill it manually");
      }
      if(kill(pid_B,SIGTERM)<0){
	perror("WARNING: Child process is still running. Please kill it manually");
      }
      return;
    }
    else{
      destip = prop->getQMAMulticastAddress();
    }
  }
  else{
    destip = prop->getMulticastAddress();
  }
#endif
  
  sprintf(filter_exp,"ip dst %s",destip.c_str());

  //  sprintf(filter_exp,"ip dst %s",prop->getMulticastAddress().c_str());
  LOGI << "Filter is [" << filter_exp << "]" << std::endl;;
  LOGI << "Found " << dev << " network device." << std::endl; 
  //////

  int pcapbufsize = 64000; //16 MB

#if 0 // CAF not needed here?
//  pcap_set_chunk_size(pcapbufsize*1024);
//pcap_t *ptr;
//pcap_set_buffer_size(ptr, pcapbufsize*1024);
#endif // CAF not needed here?
  //Setup pcap//
  if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
    LOGE << "Couldn't get netmask for device " << dev << ": " << errbuf <<std::endl;
    net = 0;
    mask = 0;
  }

  handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
  if (handle == NULL) {
    LOGE << "Couldn't open device " << dev << ": " << errbuf <<std::endl;
    return;
  }

  if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
    LOGE << "Couldn't parse filter " << filter_exp << ": " << pcap_geterr(handle) <<std::endl;
    return;
  }
  if (pcap_setfilter(handle, &fp) == -1) {
    LOGE << "Couldn't install filter " << filter_exp << ": " << pcap_geterr(handle) <<std::endl;
    return;
  }

    if(pcap_set_buffer_size(handle,pcapbufsize*1024) == -1){
      LOGE <<"Error: Couldn't set buffer size for the Ethernet buffer"<<std::endl;
      return; 
    }

    //Start capturing packets//
    pcap_loop(handle,0,got_packet,NULL);
    pcap_close(handle);
    return;
} // start

void* mcastfeeder::process_packet(void* _pq){
  PacketBuffer sbuf;
  char* bytes;
  
  PacketBufferQueue *pktq = (PacketBufferQueue*)(_pq);
  if(pktq==NULL){
    LOGF <<"ERROR: Received NULL packet queue pointer"<<endl;
    return NULL;
  }

  while( isRocknRoll() ) {
    pktq->pop(sbuf,BLOCKED);
    bytes = sbuf.get();
    
    try{
      switch(sbuf.type){
      case QMA_PACKET:
	//LOGD <<" Received QMA_PACKET"<<endl;
	//print_packet(bytes);
	DataChannel::writePacket_raw(bytes);
	break;
      case SEED_PACKET:
	//LOGD <<"Received SEED_PACKET"<<endl;
	DataChannel::writePacket(bytes);
	break;
      default:
	break;
      }
      // double procdur = (double)(TimeStamp::current_time() - sbuf.arrivaltime);
      //LOGD <<"Total Processing Time:"<<procdur<<endl;
      //if(procdur >= 0.3){
	//	LOGW <<"Processing is taking too much time :"<<procdur<<" seconds"<<endl;
      //}
    }
    catch(Error& e){
      e.print();
    }
  }

  //Terminate PacketBufferQueue and wait for the threads//
  return NULL;
} // mcastfeeder::process_packet
  
Channel getChannelFromMSEED(const char* buf) throw(Error){
    DATA_HDR	*mseed_hdr;	  /* qlib2 mseed_header */
    SDR_HDR       *shdr;
    int           wordorder;
    char loc[MAX_CHARS_IN_CHANNEL_STRING];
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


#ifdef DEBUG
    LOGD<<"Writing packet for "<<getPrintableName()<<endl;
#endif

  // Note that the mseed_hdr is a dynamically allocated struct of 
  // approx. 100 bytes
  if( (mseed_hdr=decode_hdr_sdr ((SDR_HDR *)buf,PACKET_SIZE)) == NULL) {
    Error e("Fatal Error decoding trace");
    throw e;
  }

  if(!mseed_hdr){
      throw Error("Unable to decode miniseed header");
  }

  if(!DataChannel::isDataPacket(mseed_hdr)){
    //    LOGD <<"Packet for "<<getPrintableName()<<" is NOT a data packet"<<endl;
      free_data_hdr(mseed_hdr);      
      throw Error();
  }
  //  LOGD <<"Packet for "<<getPrintableName()<<" IS a data packet"<<endl;

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
    //    mapLC(mseed_hdr->network_id,mseed_hdr->location_id,MEMORY);

    Channel ch = Channel(mseed_hdr->network_id,mseed_hdr->station_id,mapLC(mseed_hdr->network_id,mseed_hdr->location_id,MEMORY),mseed_hdr->channel_id);

    //   char sn_str[64];
    //sprintf(sn_str,"%s.%s.%s",mseed_hdr->network_id,mseed_hdr->station_id,mapLC(mseed_hdr->network_id,mseed_hdr->location_id,ASCII));

    //    string sn = sn_str;

    free_data_hdr(mseed_hdr);

    return ch;
} // getChannelFromMSEED

Channel getChannelFromRaw(const char* buf) throw(Error){

  unsigned int sec,usec;
  unsigned int nsamp;
  int* data = readraw_data((char*)buf,sec,usec,nsamp);

    timeval samp_time;
    samp_time.tv_sec =  sec;
    samp_time.tv_usec = usec;
  
  //  LOGD <<*this<<" TimeStamp : "<<TimeStamp(samp_time)<<" Delay: "<<(double)(TimeStamp::current_time()-TimeStamp(samp_time))<<endl;

  char net[16],sta[16],chan[16],loc[16];
  read_chancomp((char*)buf,net,sta,chan,loc);

  Channel ch = Channel(net,sta,mapLC(net,loc,MEMORY),chan);

  //  char sn_str[64];
  //sprintf(sn_str,"%s.%s.%s",net,sta,mapLC(net,loc,ASCII));

  //string sn = sn_str;
  return ch;
} // getChannelFromRaw

