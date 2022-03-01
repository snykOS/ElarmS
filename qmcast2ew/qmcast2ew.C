/***********************************************************

File Name :
	qmcast2ew.C
        Receive 1 second qmaserv2 multicast data packets from Q330,
	convert to EW TRACEBUF2 packets, and insert into EW ring(s).

Programmer:
        Doug Neuhauser / Paul Friberg / Sid Hellman

Based Upon the EW Module mcast2ew which is really based on:

        q2ew - quanterra to earthworm 

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

        Receive 1 second Q330 qmaserv2 multicast data packets,
	convert to EW TRACEBUF2 packets and insert into EW ring(s).

Limitations or Warnings:
	

Creation Date:
	v0.0.1 2012.04.11 - DSN - from mcast2ew v0.1.4 2008-05-19.

Modification History:
        v0.0.3 2014.09.16 - DSN - add call to get_my_wordorder() to initialize my_wordorder.
	v0.0.2 2012.04.16 - DSN - fix RingName error reporting in EWConfig.C
	v0.0.1 2012.04.11 - DSN - Initial code, converted from mcast2ew
		- Fixed EWConfig to allow different channels for same Net.Site
		  to be received on different multicast ports 
		  (ie multiple dataloggers for a single Net.Site).
								
Usage Notes:
**********************************************************/

// this next wait timeout should ideally be less than the heartbeat interval...by a lot!
#define WAIT_ON_SELECT_SECS 5

#define VERSION_QMCAST2EW "v0.0.3 2014.09.16"
#define PROGNAME "qmcast2ew"
#include <iostream>
#include <map>
#include <string>
#include <fstream>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/resource.h>

// Various include files
#include "StationMcast.h"
#include "RetCodes.h"
#include "GenLimits.h"
#include "Configuration.h"
#include "EWConfig.h"
#include "CSUtils.h"
#include "TimeStamp.h"
#include "Duration.h"
#include "nscl.h"
#include "Logfile.h"

using namespace std;

// Termination flag
int ShutMeDown;


// Time to sleep (milliseconds) when no data to be read
const int SLEEP_TIME = 500;


// Loop interval between checks of the HB time
const int HEARTBEAT_CHECK_INTERVAL = 20;

class ChanBookmark{
public:
  double prev_pkt_time;
  double prev_lat_check_time;
  Logfile latfile;
};

map<string,ChanBookmark*> ChanBookmarks;

// Function declarations
int handleOptions(int argc, char ** argv, ewConfig &cfg);
int init(int argc, char ** argv, ewConfig &cfg);
void usage(ewConfig &cfg);
int loadChannels(ewConfig &cfg);
void messageSend(ewConfig &cfg, unsigned char, short, char *);
void sendHeartbeat(ewConfig &cfg, char *str);
void shutdown(ewConfig &cfg, char * str);
void gap_check(ewConfig &cfg, ChanBookmark* cbm, TracePacket& buf);
void lat_log(ewConfig &cfg, ChanBookmark* cbm, TracePacket& buf);

/************************************************************************/
/* signal handler that intiates a shutdown */
extern "C" {
void sigHandler(int sigval) {
    signal(sigval, sigHandler);
    fprintf(stderr, "%s: Received signal %d\n", PROGNAME, sigval);
    ShutMeDown = TN_TRUE;
    return;
}
}


// this number was 5000 and it made some solaris installations of mcast2ew complain....
// we should hopefully not run more than 1024 stations through any instance of this program.
#define FD_LIMIT_REQUEST 1024

// Make EW config object external so that it is initialized.
  ewConfig cfg;

int main (int argc, char ** argv)
{
  int i, sta_index, scan_station;     /* indexes */
  bool	alert, okay;		      /* flags */
  TracePacket ew_trace_buf;	      /* earthworm trace buffer pointer */
  long  ew_trace_len;		      /* length in bytes of the trace buffer */
  time_t TSLastBeat;
  time_t now;
  int hbtime;
  int retval;
  TimeStamp curtime;
  TimeStamp packtime;
  Duration lat;
  char *data;
  fd_set rset;
  Name2RegionMap::iterator reg_iter;
  Name2StationMcast::iterator sta_iter;
  char * ringname;

  alert = TN_FALSE;
  okay = TN_TRUE;
  hbtime = 1;

  // Find the length of the two double times which are at the front
  // of the user data returned from a comserv memory area
  int timehdrinfo = (sizeof(double) * 2); 


  init_qlib2(1);//Added by Kalpesh.
  get_my_wordorder();
  
  rlimit rl;
  rl.rlim_cur = FD_LIMIT_REQUEST;
  rl.rlim_max = FD_LIMIT_REQUEST;
  if(setrlimit(RLIMIT_NOFILE,&rl)<0){
    fprintf(stderr,"Error (main): Unable to increase the file descriptor limit");
    perror("Error");
    exit(-1);
  }


  // Get the Comserv shared memory areas, and establish EW transport
  if (init(argc, argv, cfg) != TN_SUCCESS) {
    fprintf(stderr, "Error (main): Unable to initialize program\n");
    exit(-1);
  }

  ShutMeDown = TN_FALSE;
  time(&TSLastBeat);  // Initialize the time that we have started

  FD_ZERO(&rset);

  //set fd limit
  

  // Main thread loop acquiring DATA MSGs from COMSERV */
  while (okay) {
    struct timeval timeout;	// timeout to wait on select
    int max_fd=0; 		// max socket file descriptor 
    int sreturn;  		// return from select call

    // setup the select call
    sta_iter = cfg.sta_list.begin();
    while (sta_iter != cfg.sta_list.end()) {
      int sock;

 	sock = ((*sta_iter).second).sockfd;
#ifdef DEBUG
      fprintf(stderr, "DEBUG: FD_SET socket %d for %s\n", sock, ((*sta_iter).second).site);
#endif
      FD_SET(sock, &rset);
      if (sock > max_fd) {
         max_fd = sock;
      }
      sta_iter++;
    }
    max_fd++;
#ifdef DEBUG
    fprintf(stderr, "DEBUG: FD_SET max_fd %d\n", max_fd);
#endif

    timeout.tv_sec = WAIT_ON_SELECT_SECS;
    timeout.tv_usec = 0;
    
    // got any data pending?
    sreturn = select(max_fd, &rset, NULL, NULL, &timeout);
#ifdef DEBUG
    fprintf(stderr, "DEBUG: select() returns %d\n", sreturn);
#endif
    if (sreturn > 0) {

      // find which sockets have data and process them
      sta_iter = cfg.sta_list.begin();
      while (sta_iter != cfg.sta_list.end()) {
       SocketMcast *sm;

#ifdef DEBUG
        fprintf(stderr, "DEBUG: checking socket %d for %s %s %s %hd\n", 
         ((*sta_iter).second).sockfd, ((*sta_iter).second).net, ((*sta_iter).second).site,
	 ((*sta_iter).second).mcastif, ((*sta_iter).second).port);
#endif
        if (!FD_ISSET(((*sta_iter).second).sockfd, &rset)) {
		sta_iter++;
		continue; // loop again because this mc's socket has no data
        } 
#ifdef DEBUG
        fprintf(stderr, "DEBUG: got data for socket %d for %s %s \n", 
	 ((*sta_iter).second).sockfd, ((*sta_iter).second).net, ((*sta_iter).second).site);
#endif

        // see if we data is convertable?
        sm = ((*sta_iter).second).sm;
        if ((data = sm->getDataPacket()) != NULL) {

	  // Uncompress the onesecond packet
	  retval = onesec_pktToTrace(cfg, (void *) data, 512, ew_trace_buf, &ew_trace_len);
	  if (retval != TN_SUCCESS) {

	    if (cfg.Verbose)
	        logit((char*) "et", (char*) "%s: Unable to convert packet for station %s.%s\n",
		  cfg.Progname, ((*sta_iter).second).net, ((*sta_iter).second).site);

	  } else {
	    if (cfg.Verbose) {
	      // Check that the packet time is within 2 minutes of the
	      // current system time
	      curtime = TimeStamp::current_time();
	      packtime = TimeStamp(EARTHWORM_TIME, ew_trace_buf.trh2.starttime);
	      if (packtime < curtime) {
		lat = curtime - packtime;
		if ((double)lat > 120.0) {
		  logit((char*) "et", 
			(char*) "%s: Received old packet (%lf) for %s %s %s %s\n",
			cfg.Progname, (double)lat, ew_trace_buf.trh2.net,
			ew_trace_buf.trh2.sta, ew_trace_buf.trh2.chan, ew_trace_buf.trh2.loc);
		}
	      }
	    }

	    string sncl = string(ew_trace_buf.trh2.net)+"."+string(ew_trace_buf.trh2.sta)+"."+string(ew_trace_buf.trh2.chan)+"."+string(mapLC(ew_trace_buf.trh2.net,ew_trace_buf.trh2.loc,ASCII));
	    mapLC(ew_trace_buf.trh2.net,ew_trace_buf.trh2.loc,MEMORY);
	    
	    ChanBookmark* cbm = ChanBookmarks[sncl];
	    if(cbm==0){
	      cbm = new ChanBookmark();
	      cbm->prev_pkt_time = 0;
	      cbm->prev_lat_check_time = 0;

	      // Create latency logfile only if latdir is defined.
              if (cfg.latdir[0] != 0) {
		    string filename = string(cfg.latdir) + "/" + sncl + ".delay";
		    cbm->latfile = Logfile(filename.c_str(),LOG_DAILY,TN_TRUE);
	      }
	      ChanBookmarks[sncl] = cbm;
	    }
	    
	    //log latency
	    lat_log(cfg,cbm,ew_trace_buf);
	    //Gap Detection and logging
	    gap_check(cfg,cbm,ew_trace_buf);

	    // Transport trace buffer message to EW ring

	    // new logic as of 0.0.3 to put into many regions if requested
	    ((*sta_iter).second).StartRingLoop();
	    ringname  = ((*sta_iter).second).getNextRingName();
#ifdef DEBUG
            if (ringname != NULL) fprintf(stderr, "DEBUG: ring for injecting is %s\n", ringname);
#endif
	    if (ringname == NULL)
            {
#ifdef DEBUG
      fprintf(stderr, "DEBUG: rings, no specific ring set for %s %s %s %s\n", ew_trace_buf.trh2.net,
                        ew_trace_buf.trh2.sta, ew_trace_buf.trh2.chan, ew_trace_buf.trh2.loc);
#endif
		// there are no specified regions, just use the default one
	        if ( tport_putmsg(&(cfg.Region), &(cfg.DataLogo), 
			      (long) ew_trace_len, (char *)&ew_trace_buf) != PUT_OK) 
  		{
	           logit((char*) "et", (char*) "%s: Unable to write trace to transport ring\n",
		    cfg.Progname);
	           ShutMeDown = TN_TRUE;
	           break;
	        }
             }
	     else
             {
		// there are one or more specified regions
		while (ringname != NULL)
                {
		  SHM_INFO * smi;

		  if ( ((*sta_iter).second).isChanInRing(ringname, ew_trace_buf.trh2.loc, ew_trace_buf.trh2.chan) ) 
		  {
		    reg_iter = cfg.RingName2Region.find( ringname );
#ifdef DEBUG
      		    fprintf(stderr, "DEBUG: ring %s set for %s %s %s %s, sending packet to ring\n", ringname, ew_trace_buf.trh2.net,
                        ew_trace_buf.trh2.sta, ew_trace_buf.trh2.chan, ew_trace_buf.trh2.loc);
#endif
		    smi = (SHM_INFO *) (*reg_iter).second;
	            if ( tport_putmsg( smi, &(cfg.DataLogo), 
			      (long) ew_trace_len, (char *)&ew_trace_buf) != PUT_OK) 
  		    {
	               logit((char*) "et", (char*) "%s: Unable to write trace to transport ring %s\n",
		        cfg.Progname, ringname);
	               ShutMeDown = TN_TRUE;
	               break;
	            }
                  }
#ifdef DEBUG
		  else 
		  {
    			fprintf(stderr, "DEBUG: ring %s REJECTS tuple %s %s %s %s\n", ringname, ew_trace_buf.trh2.net,
                        	ew_trace_buf.trh2.sta, ew_trace_buf.trh2.chan, ew_trace_buf.trh2.loc);
                  }
#endif
	          ringname = ((*sta_iter).second).getNextRingName();
                } // end of while for multiple regions
             } // end of else on sending to multiple regions
	  } // end of else on Trace_buf conversion
      } // end of if for valid data buffers for this station

      if ((hbtime % HEARTBEAT_CHECK_INTERVAL) == 0) {
	// Send heartbeat if needed
	time(&now);
	if (difftime(now, TSLastBeat) > (double)(cfg.HeartbeatInt)) {
	  TSLastBeat = now;
	  sendHeartbeat(cfg, (char *)"");
	}
	hbtime = 1;
      } else {
	hbtime++;
      }
      sta_iter++;
     } // end of while loop on select() check
    } // end of check on select() return 

    // Send heartbeat if needed
    time(&now);
    if (difftime(now, TSLastBeat) > (double)(cfg.HeartbeatInt)) {
       TSLastBeat = now;
       sendHeartbeat(cfg, (char *)"");
       hbtime = 1;
    }

    // See if we had a problem anywhere in processing the last data
    if (ShutMeDown == TN_TRUE) {
      char msg[1024];
      sprintf(msg, "%s: kill request or fatal EW error", PROGNAME);
      shutdown(cfg, msg);
    }

    // EARTHWORM see if we are being told to stop
    if ( tport_getflag( &(cfg.Region) ) == TERMINATE ) {
      shutdown(cfg, (char*)"Earthworm TERMINATE request");
    }

  } // end of infinite while loop

  // should never reach here!
  shutdown(cfg, (char*)"Impossible statement reached");
  exit(0);
}


// Performs all necessary initialization
int init(int argc, char ** argv, ewConfig &cfg) 
{
  int ret_val;
  ChannelList::iterator clp, clpold;
  char client_name[5];
  char server_name[5];
  char curchan[MAX_CHARS_IN_CHANNEL_STRING];
  int i;
  char * cs_server_id, * my_server_id;
  SHM_INFO *region_ptr;
  Name2LongMap::iterator iter;

  // Handle some options, this exits if there are problems
  handleOptions(argc, argv, cfg);

  // Start EARTHWORM logging
  logit_init(cfg.Config, (short) cfg.QModuleId, 256, cfg.LogFile);
  logit((char*) "et", (char*) "%s: Version %s\n%s: Read in config file %s\n", 
		cfg.Progname, VERSION_QMCAST2EW, cfg.Progname, cfg.Config);

  // Load the configured channel list
/* TURNING THIS OFF FOR NOW....maybe used in future for Channel SELECTION
  if (loadChannels(cfg) != TN_SUCCESS) {
    logit((char*) "e", (char*) "%s: Unable to load channel list\n", cfg.Progname);
    return(TN_FAILURE);
  }
  logit((char *)"e", (char *)"%s: Read in channel file %s\n", cfg.Progname, cfg.ChannelFile);
*/

  // set up the mcst socket s here

  
  // Register signal handler for common signals
  signal(SIGHUP, sigHandler);
  signal(SIGINT, sigHandler);
  signal(SIGTERM, sigHandler);
  signal(SIGKILL, sigHandler);

  
  logit((char *)"e", (char *)"%s: Total number of stations : %d\n", cfg.Progname,
	cfg.maxstation);
  
  // EARTHWORM init earthworm connection at this point, 
  // this func() exits if there is a problem 
  tport_attach( &(cfg.Region), cfg.RingKey );
  logit((char *)"e", (char *)"%s: Attached to Ring Key=%d\n", cfg.Progname, cfg.RingKey);

  // now attach to all the other rings as well
  for (iter=cfg.RingName2Key.begin();  iter != cfg.RingName2Key.end(); iter++)
  {
  	if( (region_ptr = (SHM_INFO *) calloc(1, sizeof(SHM_INFO))) == NULL ) 
	{
		fprintf(stderr, "Fatal error, could not allocate a Ring region's memory %s\n", (*iter).first);
		exit(-2);
	}
	tport_attach(region_ptr, (*iter).second);
	cfg.RingName2Region[(*iter).first] = region_ptr;
  }

  
  // sleep for 2 seconds to allow heart to beat so statmgr gets it 
  // this helps statmgr see cs2ew in case COMSERV is really DOA
  // at startup.
  // sleep_ew(2000);

  return(TN_SUCCESS);
}





int loadChannels(ewConfig &cfg)
{
  int retval;
  int curline;
  Configuration c(cfg.ChannelFile);
  char line[CONFIG_MAX_STRING_LEN];
  char origline[CONFIG_MAX_STRING_LEN];
  Channel chan;
  std::pair<ChannelList::iterator, bool> inspair;

  // Parameters in channel definition file
  char network[MAX_CHARS_IN_NETWORK_STRING];
  char station[MAX_CHARS_IN_STATION_STRING];
  char channel[MAX_CHARS_IN_CHANNEL_STRING];
  char location[MAX_CHARS_IN_LOCATION_STRING];

  if (!c) {
    logit((char *)"e", (char *)"%s: Unable to open channel file %s\n", cfg.Progname, 
	  cfg.ChannelFile);
    return(TN_FAILURE);
  };

  while (1) {
    retval = c.next(line);
    c.getLineNum(curline);
    if(retval != TN_SUCCESS) {
      break;
    } else {
      retval = sscanf(line, "%s %s %s %s", network, station, channel, location);
      if (retval != 4) {
	logit((char *)"e", (char *)"%s: Invalid channel entry on line %d\n", cfg.Progname, 
	      curline);
	c.getFullLine(origline);
	logit((char *)"e", (char *)"%s: Line: %s\n", cfg.Progname, origline);
	return(TN_FAILURE);
      }
      mapLC(chan.network, location,MEMORY);

      strcpy(chan.network, network);
      strcpy(chan.station, station);
      strcpy(chan.channel, channel);
      strcpy(chan.location, location);

      inspair = cfg.cl.insert(chan);
      if (inspair.second == TN_FALSE) {
	logit((char *)"e", (char *)"%s: Duplicate channel %s specified in file\n", 
	      cfg.Progname, chan.getPrintableName());
	return(TN_FAILURE);
      }
    }
  }
  if (retval != TN_EOF) {
    logit((char *)"e", (char *)"%s: Error occurred reading config file %s, line %d\n", 
	  cfg.Progname, cfg.ChannelFile, line);
    return(TN_FAILURE);
  }

  return(TN_SUCCESS);
}



int handleOptions(int argc, char ** argv, ewConfig &cfg)  
{
  char opt_char;
  char *pn;
  
  cfg.InstId = 255;
  strcpy(cfg.ChannelFile, "");
  cfg.Verbose = TN_FALSE;    /* not well used */
  cfg.Region.mid = -1;       /* init so we know if an attach happened */
  cfg.SCNL_head = NULL;

  if ((pn = (char *) strrchr(argv[0], '/')) == NULL) {
    strcpy(cfg.Progname, argv[0]);
  } else {
    strcpy(cfg.Progname, ++pn);
  }
  
  while ((opt_char = getopt(argc, argv, "Hhv")) != -1) {
    switch (opt_char) {
    case 'v':
      cfg.Verbose = TN_TRUE;
      break;
    case 'h':
    case 'H':
      usage(cfg);
      exit(0);
      break;
    default:
      return(TN_FAILURE);
    }    
  } 
     
  // NEED to read in the EarthWorm Config file here
  if (optind < argc) {
    strcpy(cfg.Config, argv[optind]);
    if (GetConfig(argv[optind], cfg) == -1)      
      exit(0);
  } else {
    usage(cfg);
    exit(0);
  }

  return(TN_SUCCESS);
}


/* usage() - explains the options available to the user if the
		-h or -H (HELP!!!!! options are given).

*/
void usage(ewConfig &cfg) {
  fprintf(stderr, "%s: usage - Version %s\n", cfg.Progname, VERSION_QMCAST2EW);
  fprintf(stderr, "%s [-v][-hH] ew_config_file\n", cfg.Progname);
}



/***************************************************************************
 messageSend() builds a heartbeat or error message & puts it into
                  shared memory.  Writes errors to log file.
 
*/
void messageSend(ewConfig &cfg, unsigned char type, short ierr, 
		   char *note)
{
  MSG_LOGO logo;
  time_t t;
  char message[MAXSTR];
  long len;
  int my_pid;
  
  // Build the message 
  logo.instid = cfg.InstId;
  logo.mod = cfg.QModuleId;
  logo.type = type;

  time(&t);
  my_pid = getpid();
  
  // Put the message together
  if( type == cfg.TypeHB ) {
    sprintf( message, "%ld %d\n\0", (long int)t, my_pid);
  } else if( type == cfg.TypeErr ) {
    sprintf( message, "%ld %hd %s\n\0", (long int)t, ierr, note);
    logit( (char *)"et", (char *)"%s: %s\n", cfg.Progname, note );
  }
  len = strlen( message );   /* don't include the null byte in the message */
  
  // Write the message to shared memory
  if( tport_putmsg( &(cfg.Region), &logo, len, message ) != PUT_OK ) {
    if( type == cfg.TypeHB ) {
      logit((char *)"et",(char *)"%s:  Error sending heartbeat.\n", cfg.Progname );
    }
    else if( type == cfg.TypeErr ) {
      logit((char *)"et",(char *)"%s:  Error sending error: %d.\n", cfg.Progname, ierr );
    }
  }
  
  return;
}



void sendHeartbeat(ewConfig &cfg, char *str)
{
  messageSend(cfg, cfg.TypeHB, 0, str);
}






void shutdown(ewConfig &cfg, char * str)
{  
  // Detach from the EARTHWORM ring
  logit((char *)"e", (char *)"%s: exiting because %s\n", cfg.Progname, str);
  tport_detach( &(cfg.Region) );
  
  exit(0);
}

void lat_log(ewConfig &cfg, ChanBookmark* cbm, TracePacket& buf){ 
  if (! cbm->latfile) return;	    // bail if latency logfile is not defined.
//  if (cfg.latdir[0] == 0) return;

  double lasttime = cbm->prev_lat_check_time;
  double curtime = TimeStamp::current_time().ts_as_double(EARTHWORM_TIME);
  
  if(curtime - lasttime > cfg.latlog_period){
    float delay =  TimeStamp::current_time() - TimeStamp(EARTHWORM_TIME, buf.trh2.starttime);

    char msg[64];
    sprintf(msg,"%10.2f sec",delay);
    cbm->latfile.write(msg);
    cbm->latfile.flush();
    cbm->prev_lat_check_time = curtime;
  }
}

void gap_check(ewConfig &cfg, ChanBookmark* cbm, TracePacket& buf){
  if (cfg.gapdir[0] == 0) return;   // bail if gapdir is not defined.

  string sncl = string(buf.trh2.net)+"."+string(buf.trh2.sta)+"."+string(buf.trh2.chan)+"."+string(mapLC(buf.trh2.net,buf.trh2.loc,ASCII));
  mapLC(buf.trh2.net,buf.trh2.loc,MEMORY);

  string gapfile = string(cfg.gapdir)+"/"+sncl+".gap";

  int nsamps = buf.trh2.nsamp;
  float dt = 1.0/buf.trh2.samprate;
  
  TimeStamp temptime;
  
  TimeStamp firstsamptime = TimeStamp(EARTHWORM_TIME, buf.trh2.starttime);
  temptime = firstsamptime + Duration(dt*nsamps); 

  double lasttime = cbm->prev_pkt_time;
  double firsttime = firstsamptime.ts_as_double(EARTHWORM_TIME);
  double endtime =  temptime.ts_as_double(EARTHWORM_TIME);

  // cout <<sncl<<" : last pkt's endtime: "<<TimeStamp(EARTHWORM_TIME,lasttime)<<" pkt's start time:"<<TimeStamp(EARTHWORM_TIME,firsttime)<<" pkt's endtime:"<<TimeStamp(EARTHWORM_TIME,endtime)<<" diff:"<<(firsttime - lasttime)<<endl;

  cbm->prev_pkt_time = endtime;

  if(lasttime==0){
    return;
  }
  else{
    double diff = (firsttime - lasttime);
    if( diff > dt*2.0){
      //GAP
      char gapline[1024];
      TimeStamp ts = TimeStamp(EARTHWORM_TIME,lasttime);
      sprintf(gapline,"%4d/%02d/%02d %02d:%02d:%02d|%fs\n",
	      ts.year(),ts.month_of_year(), ts.day_of_month(),
	      ts.hour_of_day(), ts.minute_of_hour(),ts.second_of_minute(),
	      diff);
      ofstream myfile;
      myfile.open((char*)gapfile.c_str(),ios::out | ios::app | ios::binary);
      myfile << gapline;
      if(myfile.fail()){

	cerr <<"Error: Unable to write gap information to the file "<<gapfile<<endl;
      }
      myfile.close();
    }
  }
}
