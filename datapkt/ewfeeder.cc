
/***********************************************************

File Name :
	ewfeeder.cc

Programmer:
	Kalpesh Solanki. Caltech

Description:
	This reads tracebuf2 messages from an Earthworm ring,
	and Calculates TauC/PD for EEW.

Creation Date:
        Jan, 2010

Modification History:

Usage Notes:
        Works with 4-byte, integer waveforms only.

**********************************************************/

#include <fcntl.h>
#include <errno.h>
#include <algorithm>
#include "plog/Log.h"       // plog logging library
#include "WPProperties.h"
#include "ewfeeder.h"
#include "notify_eew.h"


using namespace std;
using namespace earthworm;

const string RCSID_ewfeeder_cc = "$Id: ewfeeder.cc $";

bool ewfeeder::is_offline;
unsigned long ewfeeder::offline_timestamp_offset;
bool ewfeeder::offline_first_packet;
int packet_latency_threshold;
bool ewfeeder::record;

static long nowMillisecs() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (long)tp.tv_sec * 1000L + (long)tp.tv_usec / 1000L;
}

extern "C" {
static void* process_packet_wrapper(void* data)
{
    return ewfeeder::process_packet(data);
} // process_packet_wrapper
} // extern "C"


void ewfeeder::start(string config, int orgtime, WPFactory* pFactory) throw(Error)
{

  debug = TN_FALSE;
    char Buffer[BUF_SIZE];           // Tracebuf2 message container
    TRACE2_HEADER *trh = (TRACE2_HEADER *)&Buffer[0];

    char     errMsg[150];
    time_t   timeNow;
    time_t   timeLastBeat;
    int      res;
    long     recsize;
    int numthreads;
    MSG_LOGO reclogo;
    bool first_packet = true;
    long last_packet_queued;
    long last_packet_endtime;
    string channel_file;
    FILE *fp = NULL;
    gzFile zfd = Z_NULL;
    
    WPProperties* prop;
    try{
      prop = WPProperties::getInstance();
      prop->init(config);
    }
    catch(Error& e){
      LOGE << "Problem getting properties: " << e.str() << std::endl;
      throw e;
    }
 
#ifdef NOTIFY_EEW
    if ( prop->getCMSConfigFile().empty() || strcmp(prop->getCMSConfigFile().c_str(), "false") == 0 ||
            prop->getEEWChannel().empty() || strcmp(prop->getEEWChannel().c_str(), "false") == 0 ) {
        LOGW << std::endl << "*** Notify_EEW disabled because either CMSConfigFile or EEWChannel is empty or false ***" << std::endl;
    } else {
        //Start CMS Thread//
        LOGI << "ewfeeder is calling start_notify_eew_listener...";
        start_notify_eew_listener_thread();
    }
#else
    LOGW << "*** Support for NOTIFY_EEW disabled at compile time  ***";
#endif

    //Initialization
    total_thread_count = 0;
    nLogo = 0;
    offline_timestamp_offset = 0;
    offline_first_packet = false;
    is_offline = false;

    if(orgtime > 0){
      DataChannel::origin_time = orgtime;
      is_offline = true;
    }

    packet_latency_threshold = prop->getPacketLatencyThreshold();

    // check for optional filters
    string pattern = prop->getChannelFilter();
    if (pattern.length() > 0)
        DataChannel::SetChannelFilter(pattern);

    //bool replaying = false;
    bool replaying = prop->getDoReplay();
    record = prop->getFLdebug();
    bool replay_real_time = false;
    string replay_file;
    try {
	//replay_file = prop->getString("Replay");
	replay_file = prop->getReplayFile();
	replaying = !replay_file.empty();
	if(replaying) {
	    //replay_real_time = prop->getBool("ReplayRealTime");
	    replay_real_time = prop->getReplayRealTime();
	}
    }
    catch (Error e) {}

    if( replaying ) {
	//channel_file = prop->getString("ChannelFile");
	channel_file = prop->getChannelFile();
	DataChannel::readChannels(pFactory, channel_file);
    }
    //populate channels, read black and grey list files//
    else if (!DataChannel::readChannelData(pFactory)) {
        LOGF << "Error reading channel metadata.  Aborting!";
        exit(2);
    }

    numthreads = prop->getThreadCount();
  //set all queue pointers to NULL
    for(int i=0;i<MAX_THREADS;i++){
      pktqmap[i] = NULL;
    }

    myPid = getpid();
    if (myPid == -1) {
        LOGF << " Cannot get my pid. Exiting";
        throw Error();
    }

    if( !replaying ) {
	// Connect to Earthworm transport ring
	// Do this early so we can start beating our heart during the long
	// startup procedure

	r2w_config((char*)prop->getEWConfig().c_str());
	LOGI << "RingName: " << RingName;
	r2w_lookup(); 

	LOGI << "RingKey:" << RingKey;
	tport_attach( &Region, RingKey );
	timeLastBeat = time(&timeNow) - HeartBeatInterval - 1;
	if  ( time(&timeNow) - timeLastBeat  >=  HeartBeatInterval ) {
	    timeLastBeat = timeNow;
	    r2w_status( TypeHeartBeat, 0, (char *)"" ); 
	}
    
        // 
        // Read in list of channels to import.
        //

    
        // Flush the transport ring
        //
        while ( tport_getmsg( &Region, GetLogo, nLogo, &reclogo, &recsize,
			  Buffer, sizeof(Buffer)-1 ) != GET_NONE );
    }
    else {
	fp = NULL;
	zfd = Z_NULL;
	int n = replay_file.length();
	if(n > 3 && replay_file.substr(n-3).compare(".gz") == 0) {
	    int fd;
            if((fd = open(replay_file.c_str(), O_RDONLY)) == -1) {
                LOGF << "Cannot open tracebuf file: " << replay_file.c_str() << std::endl 
                     << strerror(errno) << std::endl;
                exit(-1);
            }
            zfd = gzdopen(fd, "rb");
	}
        else {
            if((fp = fopen(replay_file.c_str(), "r")) != NULL) {
                LOGI << "Reading tracebuf file: " << replay_file;
                LOGI << "Using chanfile: " << channel_file;
            }
            else {
                LOGF << "Cannot open tracebuf file: " << replay_file.c_str() << std::endl
                     << strerror(errno) << std::endl;
                exit(-1);
            }
	}
	first_packet = true;
    }
    datatype_error_cnt = 0;

    //
    // Declare some stuff
    //
    //
    // Get Earthworm tracebuf2 messages
    //    
    while ( isRocknRoll() )
    {
	if(!replaying) {
	    // See if termination has been requested
	    pid_t flag = (pid_t) tport_getflag(&Region);
        if ( flag == TERMINATE || flag == myPid) {
            LOGI << std::endl << std::endl;
            LOGI << "************" << std::endl;
            LOGI << __FILE__ << ":" << __LINE__ << ": received EW request to terminate." << std::endl;
            LOGI << "************" << std::endl << std::endl;

            // now change flag so we can start shutting down
            setRocknRoll(false);
            break;
        }
	
	    // Send heartbeat to statmgr
	    if  ( time(&timeNow) - timeLastBeat  >=  HeartBeatInterval ) {
            timeLastBeat = timeNow;
            r2w_status( TypeHeartBeat, 0, (char *)"" ); 
	    }
    
	    //refresh channels, read black and grey list files. 
	    if (!DataChannel::readChannelData(pFactory)) {
		    LOGW << "Error refreshing channel metadata. Continue using previously loaded metadata";
	    }
	}
    
	while ( isRocknRoll() )      // Process all new messages
	{
	    if(!replaying) {
	        // Get msg & check return code from transport
	        res = tport_getmsg( &Region, GetLogo, nLogo, &reclogo, &recsize,
				Buffer, sizeof(Buffer)-1 );
                           	    
	        if ( res == GET_NONE ) {          // No more new messages
		    break;
	        }
	        else if ( res == GET_TOOBIG )    // Next message too big for buffer
            {
                snprintf(errMsg, sizeof(errMsg),
                        "Retrieved msg[%ld] (i%u m%u t%u) too big for Buffer[%ld]",
                        recsize, reclogo.instid, reclogo.mod, reclogo.type, 
                        (long)sizeof(Buffer)-1 );
                r2w_status( TypeError, ERR_TOOBIG, errMsg );
                LOGE << __FILE__ << ":" << __LINE__ << ": " << errMsg;
                continue;
            }
	        else if ( res == GET_MISS )      // Got a msg, but missed some
            {
                sprintf( errMsg,
                        "Missed msg(s)  i%u m%u t%u  %s.",
                        reclogo.instid, reclogo.mod, 
                        reclogo.type, RingName );
                r2w_status( TypeError, ERR_MISSMSG, errMsg );
                LOGE << __FILE__ << ":" << __LINE__ << ": " << errMsg;
                continue;
            }
	        else if ( res == GET_NOTRACK )   // Got msg; can't tell if some missed
            {
                snprintf(errMsg, sizeof(errMsg),
                        "Msg received (i%u m%u t%u); transport.h NTRACK_GET exceeded",
                        reclogo.instid, reclogo.mod, reclogo.type );
                r2w_status( TypeError, ERR_NOTRACK, errMsg );
                LOGE << __FILE__ << ":" << __LINE__ << ": " << errMsg;
            }
	    } else {
	        if( !readPacketFromFile(Buffer, trh, replay_file, fp, zfd) ) {
                    if(zfd != Z_NULL) gzclose(zfd);
                    else fclose(fp);
                    zfd = Z_NULL;
                    fp = NULL;
                    break;
                }
                if(first_packet) {
                    first_packet = false;
                    last_packet_queued = nowMillisecs();
		    last_packet_endtime = (long)(trh->endtime*1000 + 0.5);
                }
                else if(replay_real_time) {
                    // wait 
                    long this_packet_endtime = (long)(trh->endtime*1000 + 0.5);
                    long delayMs = this_packet_endtime - last_packet_endtime;
                    if(delayMs > 0) {
                        long nowMs = nowMillisecs();
                        long sleep_time = delayMs + last_packet_queued - nowMs;
                        if(sleep_time > 0) sleep_ew(sleep_time);
			last_packet_queued = nowMs;
			last_packet_endtime = this_packet_endtime;
                    }
                }
	    }

	    //
	    // Four-byte integer data only
	    //
	    if ( strcmp(trh->datatype, "i4") != 0 &&
		 strcmp(trh->datatype, "s4") != 0 ) continue;

	    //   if(strcmp(trh->sta,eewPropertiesST::getInstance()->getStation().c_str())!=0)
	    //  continue;
	    //LOGD << "Received Packet from " << trh->net << "." << trh->sta << "." << trh->chan << "." << trh->loc;
	    

	    //
	    // Search for this sncl in the list of known sncl's.
	    // If not found, put it in the not-found list.
	    //

	    //
	    // If necessary, swap bytes in tracebuf2 message
	    //
	    if ( WaveMsg2MakeLocal(trh) < 0 )
	      {
              LOGE << "Tracebuf2 waveform type is unknown.";
              continue;
	      }

	    //
	    // Found a sncl we want.  Load it into the wda.
	    //
	    // datatype is restricted above to "i4" or "s4" which are both 4-bytes
	    int data_size = 4;
      	    char *samples = (char *)(&Buffer[0] + sizeof(TRACE2_HEADER));	    
 	    char stasig[32];
	    snprintf(stasig,sizeof(stasig),"%s.%s.%s",trh->net,trh->sta,mapLC(trh->net,trh->loc,ASCII));
 	    Channel chn = Channel(trh->net,trh->sta,trh->loc,trh->chan);

 	    PacketInfo pinfo;
 	    pinfo.ch = chn;
	    pinfo.pbuf.arrivaltime = TimeStamp::current_time();
	    memcpy(&pinfo.trh,trh,sizeof(TRACE2_HEADER));
	    // LOGD << "Size of samples:" << sizeof(waveform_data_type)*(trh->nsamp);
       	    memcpy(pinfo.pbuf.get(),samples,data_size*(trh->nsamp));

 	    //	    string stasig = string(trh->net)+string(trh->sta)+string(trh->loc);
 	    unsigned long hashcode = hash((unsigned char*)stasig);
	    // LOGD << "Hash for " << stasig << " is " << hashcode;
 	    int tindex = hashcode%numthreads;
	    // LOGD << "Index in the queue array: " << tindex;

	    PacketQueue* pktq = pktqmap[tindex];
	    if(pktq==NULL){
	        pktq = new PacketQueue();
	        if(pktq==NULL){
                    LOGW << "Can't create packet queue";
                    throw Error();
                }

	        total_thread_count++;
                LOGD << "Creating a new thread for channel " << chn << ". Total Thread Count: " << total_thread_count;
	        if(pthread_create(&thread_array[total_thread_count-1],NULL,process_packet_wrapper,(void*)pktq)<0){
                    LOGF << "ERROR: Error Creating thread" << std::endl;
                    delete pktq;
                    throw Error();
                }
	        pktqmap[tindex] = pktq;
	    }
	    if(replaying) {
		// slow down if the queue is getting big
		while(pktq->size() > 400) {
		    sleep_ew( 200 );
		}
	    }
	    pktq->push(pinfo,BLOCKED);
	}
	if(!replaying) {
	    sleep_ew( 200 );
	}
	else {
	    break;
	}
    }
    if(!replaying) {
	for(int i=0;i<MAX_THREADS;i++){
	    pktqmap[i] = NULL;
	}
	tport_detach( &Region );
    }
    else {
	// wait until packet queues are empty
	int num_empty = 0;
	while(num_empty < MAX_THREADS) {
	    num_empty = 0;
	    for (int i = 0; i < MAX_THREADS; i++) {
		if(pktqmap[i] == NULL || (int)pktqmap[i]->size() == 0) num_empty++;
	    }
	    if(num_empty < total_thread_count) sleep(1);
        }

	setRocknRoll(false);

        if (record) {
            for (WPVector::iterator it=DataChannel::eewRec.begin(); it!=DataChannel::eewRec.end(); it++) {
                WP* eew = *it;
                eew->stoprecord();
            }
        }

	// release all threads from the pktqmap[i].pop() lock and they will exit.
	for (int i = 0; i < MAX_THREADS; i++) if(pktqmap[i] != NULL) {
	    PacketInfo pinfo;
	    pktqmap[i]->push(pinfo, BLOCKED); // insert a "null" packet so the pop() will return
	}

	for (int i = 0; i < total_thread_count; i++) {
//	    LOGD << "Waiting for thread " << i<< " to join...";
//	    int rtn = pthread_join(thread_array[i], NULL);
	    pthread_join(thread_array[i], NULL);
//	    LOGD << "Done. Return value : " << rtn;
	}
    }

} // ewfeeder::start


void* ewfeeder::process_packet(void* _pq)
{
  PacketInfo pinfo;
  PacketQueue *pktq = (PacketQueue*)(_pq);
  if(pktq==NULL){
      LOGF << "ERROR: Received NULL packet queue pointer";
      return NULL;
  }

  while( isRocknRoll() ){
    pktq->pop(pinfo,BLOCKED);
    _packet_statistics.read++;
    
    try {

        WP* eew = DataChannel::getWPHandle(pinfo.ch);
        if(eew){

            // Check that it is not black listed. 
            if (DataChannel::getChannelFlag(pinfo.ch) != BLACK) {

                struct timeval curtime;
                gettimeofday(&curtime,NULL);	  

                eew->setSampleRate(pinfo.trh.samprate);
                if (record) {
                    WPVector::iterator it;
                    it = std::find(DataChannel::eewRec.begin(), DataChannel::eewRec.end(), eew);
                    if (it == DataChannel::eewRec.end()) {
                        DataChannel::eewRec.push_back(eew);
                    }
                }
                //LOGD << "Putting packet to " << chn << " EEW , eew->samplerate:" << eew->samplerate << " trh->samprate:" << trh->samprate << " nsamps:" << trh->nsamp;
                timeval time_of_first_sample;
                if(offline_first_packet==false && is_offline==true){
                    offline_first_packet = true;

                    offline_timestamp_offset = curtime.tv_sec - (long)pinfo.trh.starttime;
                }

                time_of_first_sample.tv_sec  = (long) pinfo.trh.starttime + offline_timestamp_offset;
                time_of_first_sample.tv_usec = (long)(1000000.0 * fmod(pinfo.trh.starttime,1.0));

                if (packet_latency_threshold != 0 && fabs(curtime.tv_sec - pinfo.trh.starttime) > fabs(packet_latency_threshold) ) {
                    // debug print if threshold is negative
                    if (packet_latency_threshold < 0) {
                        LOGI << "ewfeeder.process_packet: " << pinfo.ch << " latency is " << (long)(curtime.tv_sec - pinfo.trh.starttime);
                    }
                    _packet_statistics.latent++;
                } else if(!eew->useRawPacket()) {
                   if ( eew->process(pinfo.ch,TimeStamp(UNIX_TIME,time_of_first_sample),(int*)pinfo.pbuf.get(),pinfo.trh.nsamp) ) _packet_statistics.processed++;
		} else {
		    RawPacket raw(pinfo.ch, pinfo.trh.nsamp, (int*)pinfo.pbuf.get(), pinfo.trh.starttime,
                                pinfo.trh.samprate, pinfo.pbuf.arrivaltime.ts_as_double(UNIX_TIME),
				(double)curtime.tv_sec + 1.e-06*curtime.tv_usec);
		    if ( eew->process(raw) ) _packet_statistics.processed++;
		}
            } else {
                _packet_statistics.blacklisted++;
            }
        } else {
            _packet_statistics.dropped++;
        }
    }
    catch(Error& e){
      LOGE << __func__ << " Caught unexpected error: " << e.str() << std::endl;
    }
  }

  //Terminate PacketQueue and wait for the threads//
  return NULL;
} // ewfeeder::process_packet


/*********************************************************************
 *  r2w_lookup( )   Look up important info from earthworm.h tables.  *
 *********************************************************************/
void ewfeeder::r2w_lookup( void )
{
   
    // Look up keys to shared memory regions
    LOGI << "RING NAME:" << RingName;
    if ( ( RingKey = GetKey(RingName) ) == -1 ) {
	    LOGF << "Invalid ring name <" << RingName << ">. Exiting.";
	    exit( -1 );
    }
    
    // Look up installations of interest
    if ( GetLocalInst( &InstId ) != 0 ) {
	    LOGF << "error getting local installation id. Exiting.";
	    exit( -1 );
    }
    
    // Look up modules of interest
    if ( GetModId(MyModName, &MyModId ) != 0 ) {
	    LOGF << "Invalid module name <" << MyModName << ">. Exiting.";
	    exit( -1 );
    }
    
    // Look up message types of interest
    if ( GetType( (char *)"TYPE_HEARTBEAT", &TypeHeartBeat ) != 0 ) {
	    LOGF << "Invalid message type <TYPE_HEARTBEAT>. Exiting.";
	    exit( -1 );
    }
    if ( GetType( (char *)"TYPE_ERROR", &TypeError ) != 0 ) {
	    LOGF << "Invalid message type <TYPE_ERROR>. Exiting.";
	    exit( -1 );
    }
    return;
} 


/*********************************************************************
 * r2w_status() builds a heartbeat or error message & puts it into   *
 *              shared memory.  Writes errors to screen.             *
 *********************************************************************/
void ewfeeder::r2w_status( unsigned char type, short ierr, char *note )
{
    MSG_LOGO    logo;
    char        msg[256] = {0};
    long        size;
    long        t;
    
    // Build the message
    logo.instid = InstId;
    logo.mod    = MyModId;
    logo.type   = type;
    
    time( &t );
    
    if ( type == TypeHeartBeat )
	    sprintf( msg, "%ld %ld\n", t, (long int)myPid);
    else if ( type == TypeError ) {
        sprintf( msg, "%ld %hd %s\n", t, ierr, note);
        LOGI <<  note;
    }
    
    size = strlen( msg );
    
    // Write message to shared memory
    if ( tport_putmsg( &Region, &logo, size, msg ) != PUT_OK )
    {
        if ( type == TypeHeartBeat ) {
            LOGE << "Error sending heartbeat.";
        }
        if ( type == TypeError ) {
            LOGE << "Error sending error: " << ierr;
        }
    }
    return;
}

/*******************************************************************
 *  r2w_config() processes command file(s) using kom.c functions.  *
 *                    Exits if any errors are encountered.         *
 *******************************************************************/
void ewfeeder::r2w_config( char *configfile )
{
    int  ncommand = 4;  // # of required commands you expect to process 
    char init[10];      // Init flags, one for each required command
    char *com;
    char *str;
    int  success;
    
    // Set to zero one init flag for each required command
    for ( int i=0; i<ncommand; i++ )  init[i] = 0;
    
    // Open the main configuration file
    int nfiles = k_open( configfile ); 
    if ( nfiles == 0 )
    {
		LOGF << "ew2eew: Error opening command file <" << configfile << ">. Exiting.";
	    exit( -1 );
    }
    
    // Process all nested config files
    while (nfiles > 0) {      // While there are config files open
	while (k_rd()) {       // Read next line from active file
	    com = k_str();      // Get first token from line
	    
	    // Ignore blank lines & comments
	    if ( !com )           continue;
	    if ( com[0] == '#' )  continue;
	    
	    // Open a nested configuration file
	    if ( com[0] == '@' ) {
		success = nfiles+1;
		nfiles  = k_open(&com[1]);
		if ( nfiles != success ) {
		    LOGF << "ew2eew: Error opening command file <" << &com[1] << ">. Exiting.";
		    exit( -1 );
		}
		continue;
	    }
	    
	    // Process anything else as a command
	    if ( k_its((char *)"LogFile") ) {
		LogSwitch = k_int();
		init[0] = 1;
	    }
	    else if ( k_its((char *)"MyModuleId") ) {
		str = k_str();
		if (str) strcpy( MyModName, str );
		init[1] = 1;
	    }
	    else if ( k_its((char *)"RingName") ) {
		str = k_str();
		if (str) {
		    if (strlen(str) >= MAX_RING_STR) {
                LOGF << "RingName too long; max " << 
                    MAX_RING_STR-1 << " chars";
                exit(-1);
            }
		    strcpy( RingName, str );
		    init[2] = 1;
		}
	    }
//             else if ( k_its((char *)"ConfigFile") ) {
//               str = k_str();
//               if (str) {
//                 if(strlen(str) >= MAX_FILENAME){
//                   LOGD << "ConfigFile too long";
//                   exit(-1);
//                 }
//                 strcpy(ConfigFile,str);
//                 try{
//                   WPProperties* prop = WPProperties::getInstance();
//                   prop->init(string(ConfigFile));
//                   }
//                   catch(Error& e){
//                     e.print();
//                   }
                
                
//                 init[5] = 1;
//               }
//             }
	    else if ( k_its((char *)"HeartBeatInterval") ) {
		HeartBeatInterval = k_long();
		init[3] = 1;
	    }

	    // Installation & module to get tracebuf2 messages from
	    else if ( k_its((char *)"GetTraceFrom") ) {
		if ( nLogo+1 >= MAXLOGO ) {
		    LOGF << "Too many <GetTraceFrom> commands in <" << configfile << ">; max="
                << MAXLOGO << ". Exiting.";
		    exit( -1 );
		}
		if ( ( str=k_str() ) ) {
		    if ( GetInst( str, &GetLogo[nLogo].instid ) != 0 ) {
			LOGF << "Invalid installation name <" << str << "> in <GetTraceFrom> cmd. Exiting.";
			exit( -1 );
		    }
		}
		if ( ( str=k_str() ) ) {
		    if ( GetModId( str, &GetLogo[nLogo].mod ) != 0 ) {
			LOGF << "Invalid module name <" << str << "> in <GetTraceFrom> cmd. Exiting.";
			exit( -1 );
		    }
		}
		if ( GetType( (char *)"TYPE_TRACEBUF2", &GetLogo[nLogo].type ) != 0 ) {
		    LOGF << "Invalid message type <TYPE_TRACEBUF2>. Exiting.";
		    exit( -1 );
		}
		nLogo  += 1;
		init[4] = 1;
	    }	    
	    else if ( k_its((char *)"Debug") )
		debug = TN_TRUE;

	    // Unknown command
	    else {
            LOGW << "Unknown command <" << com <<  "> in " << configfile << std::endl;
            continue;
        }
	    
	    // See if there were any errors processing the command
	    if ( k_err() ) {
            LOGF << "Bad <" << com << "> command in " << configfile << ". Exiting.";
            exit( -1 );
        }
	}
	nfiles = k_close();
    }
    
    // After all files are closed, check init flags for missed commands
    int  nmiss = 0;
    for ( int i=0; i<ncommand; i++ )
	if ( !init[i] ) nmiss++;
    if ( nmiss )
    {

    ostringstream ostrm;
	ostrm << "ERROR, no ";
	if ( !init[0] ) ostrm << "<LogFile> ";
	if ( !init[1] ) ostrm << "<MyModuleId> ";
	if ( !init[2] ) ostrm << "<RingName> ";
	if ( !init[3] ) ostrm << "<HeartBeatInterval> ";
	if ( !init[4] ) ostrm << "<GetTraceFrom> ";	        
	ostrm << "command(s) in <" << configfile << ">. Exiting.";
    LOGE << ostrm.str();
	exit( -1 );
    }    
    return;
}

unsigned long ewfeeder::hash(unsigned char *str)
{
  unsigned long hash = 5381;
  int c;
  
  while ( (c = *str++) )
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  
  return hash;
}

bool ewfeeder::readPacketFromFile(char *Buffer, TRACE2_HEADER *trh, string tracebuf_file, FILE *fp, gzFile zfd)
{
    if(zfd != Z_NULL) {
        if(gzread(zfd, Buffer, sizeof(TRACE2_HEADER)) <= 0) {
            LOGI << "ewfeeder: finished reading " << tracebuf_file;
            return false;
        }
    }
    else {
        if(fread(Buffer, sizeof(TRACE2_HEADER), 1, fp) != 1) {
            LOGI << "ewfeeder: finished reading " << tracebuf_file;
            return false;
        }
    }

    if( !okDataType(trh) ) return false;

    if(zfd != Z_NULL) {
        if(gzread(zfd, Buffer+sizeof(TRACE2_HEADER), trh->nsamp*sizeof(int)) <= 0) {
            LOGE << "ewfeeder: read error " << tracebuf_file;
            return false;
        }
    }
    else {
        if((int)fread(Buffer+sizeof(TRACE2_HEADER), sizeof(int), trh->nsamp, fp) != trh->nsamp) {
            LOGE << "ewfeeder: read error." << tracebuf_file;
            return false;
        }
    }
    return true;
}

bool ewfeeder::okDataType(TRACE2_HEADER *trh)
{
    // Four-byte integer data only
    if ( strcmp(trh->datatype, "i4") != 0 && strcmp(trh->datatype, "s4") != 0 ) {
        if(++datatype_error_cnt < 20) {
            LOGE << "ewfeeder: bad datatype: " << trh->datatype << " Must 'i4' or 's4'";
        }
        else if(datatype_error_cnt == 20) {
            LOGE << "ewfeeder: bad datatype (last warning): " << trh->datatype << " Must 'i4' or 's4'";
        }
        return false;
    }

    // If necessary, swap bytes in tracebuf2 message
    if ( WaveMsg2MakeLocal(trh) < 0 ) {
        LOGI << "Tracebuf2 waveform type is unknown.";
        if(++datatype_error_cnt < 20) {
            LOGE << "ewfeeder: Tracebuf2 waveform type is unknown.";
        }
        else if(datatype_error_cnt == 20) {
            LOGE << "ewfeeder: Tracebuf2 waveform type is unknown. (last warning)";
        }
        return false;
    }
    return true;
}

// end of file: ewfeeder.cc
