/***********************************************************

File Name :
	EWConfig.C

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
	23 May 2000

Modification History:
	Modified for mcast2ew by Paul Friberg 7/12/2005


Usage Notes:

**********************************************************/

// Various include files
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "EWConfig.h"


/* Number of commands in the config file */
const int NCOMMAND = 7;


// Function prototypes
int IsComment( char string[] );
int insertSCNL(char * S, char *C, char *N, char *L, int pin, ewConfig &cfg);
int validateSCNL(char *S,char *C,char *N, char *L, int pinno);
int equivSCNL(char *S,char *C,char *N, char *L, SCNL *scn);
void setuplogo(MSG_LOGO *, ewConfig &cfg);



/*********************************************************************
 *                             IsComment()                           *
 *                                                                   *
 *  Accepts: String containing one line from a config file.          *
 *  Returns: 1 if it's a comment line                                *
 *           0 if it's not a comment line                            *
 *********************************************************************/
int IsComment( char string[] )
{
   int i;

   for ( i = 0; i < (int)strlen( string ); i++ )
   {
      char test = string[i];

      if ( test!=' ' && test!='\t' )
      {
         if ( test == '#'  )
            return 1;          /* It's a comment line */
         else
            return 0;          /* It's not a comment line */
      }
   }
   return 1;                   /* It contains only whitespace */
}




/*****************************************************************/
/* validate the SCNL lengths and non-null states 
	SUCCESS returns 0
	FAILURE returns -1 if any of S,C,N or pinno are invalid 
*/
int validateSCNL(char *S,char *C,char *N, char *L, int pinno) {
	if (S == NULL || strlen(S) > SEED_SITE_MAX_CHAR) { return (-1);}
	if (C == NULL || strlen(C) > SEED_CHAN_MAX_CHAR) { return (-1);}
	if (N == NULL || strlen(N) > SEED_NET_MAX_CHAR) { return (-1);}
	if (L == NULL || strlen(L) > SEED_LOC_MAX_CHAR) { return (-1);}
	if (pinno < 0) {return (-1);}
	return(0);
}


/*****************************************************************/
/* compares the S,C,N,L to an SCNL struct
	SUCCESS returns 0 (match!)
	FAILURE returns -1 (no match)
*/
int equivSCNL(char *S,char *C,char *N, char *L, SCNL *scn) {
  char tmp[SEED_SITE_MAX_CHAR+SEED_CHAN_MAX_CHAR+SEED_NET_MAX_CHAR+SEED_LOC_MAX_CHAR+1]; 

  sprintf(tmp, "%s%s%s%s", S, C, N, L);
  return (strcmp(tmp, scn->comb));
}


/*****************************************************************/
/* mallocs and inserts an SCN into the linked list
	SUCCESS returns 0 (inserted!)
	FAILURE returns -1 (bad SCN or memory alloc() problem)
*/
int insertSCNL(char * S, char *C, char *N, char *L, int pin, ewConfig &cfg) {
  SCNL *scn;
  if (validateSCNL(S,C,N,L,pin) == -1) {return (-1);}
  if ((scn = (SCNL *) calloc(sizeof(SCNL),1)) == NULL) {return (-1);}

  mapLC(N, L,MEMORY);
  strcpy(scn->site, S);
  strcpy(scn->chan, C);
  strcpy(scn->net, N);
  strcpy(scn->loc, L);
  sprintf(scn->comb, "%s%s%s%s", S, C, N, L);
  
  if (cfg.Verbose == TRUE)
    fprintf(stderr, "%s: read in SCN combo = %s pin = %d\n", 
	    cfg.Progname, scn->comb, pin);
  scn->pinno=pin;
  if (cfg.SCNL_head == NULL) {
    /* init the linked list */
    cfg.SCNL_head = scn;
    scn->next = NULL;
  } else {
    /* push the scn onto the list */
    scn->next = cfg.SCNL_head;
    cfg.SCNL_head = scn;
  }
  return(0);
}


/*****************************************************************/
/* finds a pinno for a given SCN
   SUCCESS returns pinno matching SCN
   FAILURE returns 0  (default pin)
*/
int getPinFromSCNL( char * S, char *C, char *N, char *L, ewConfig &cfg)  {
  SCNL *ptr;
  ptr = cfg.SCNL_head;
  while (ptr != NULL) {
    if (equivSCNL(S,C,N,L,ptr) == 0) {
      return(ptr->pinno);
    } else {
      ptr=ptr->next;
    }
  }
  return (-1);
}


Name2StationMcast::iterator getStation(char *n, char *s, char *m, int p, ewConfig &cfg)
{
   Name2StationMcast::iterator sta_iter;
	sta_iter = cfg.sta_list.begin();
	while (sta_iter != cfg.sta_list.end()) {
#ifdef DEBUG_GS
		fprintf(stderr, "DEBUG: getStation() checking against station %s %s %s %hd\n", 
			(*sta_iter).second.net, (*sta_iter).second.site, (*sta_iter).second.mcastif, (*sta_iter).second.port);
#endif
		if (strcmp(n, (*sta_iter).second.net) == 0 && 
		    strcmp(s, (*sta_iter).second.site) == 0 &&
		    strcmp(m, (*sta_iter).second.mcastif) == 0 &&
		    p == (*sta_iter).second.port) {
			return sta_iter;
		}
		sta_iter++;
	}
	return sta_iter;
}


int GetConfig(char *configfile, ewConfig &cfg)
{
   const int ncommand = NCOMMAND;

   char     init[NCOMMAND];     /* Flags, one for each command */
   int      nmiss;              /* Number of commands that were missed */
   int      nfiles;
   int      i;

   StationMcast *sta, sta_const;
   Name2StationMcast::iterator sta_iter;
   char search_name[1024];
   char *sname_copy;
   cfg.maxstation=0;

/* Set to zero one init flag for each required command
   ***************************************************/
   for ( i = 0; i < ncommand; i++ )
      init[i] = 0;


/* turn of requirement for ChannelFIle as it is not used */
   init[5] = 1;

/* Open the main configuration file
   ********************************/
   nfiles = k_open( configfile );
   if ( nfiles == 0 )
   {
      fprintf(stderr, "%s: Error opening configuration file <%s>\n", 
	      cfg.Progname, configfile );
      return -1;
   }

/* Process all nested configuration files
   **************************************/
   while ( nfiles > 0 )          /* While there are config files open */
   {
      while ( k_rd() )           /* Read next line from active file  */
      {
         int  success;
         char *com;
         char *str;

         com = k_str();          /* Get the first token from line */

         if ( !com ) continue;             /* Ignore blank lines */
         if ( com[0] == '#' ) continue;    /* Ignore comments */

/* Open another configuration file
   *******************************/
         if ( com[0] == '@' ) {
            success = nfiles + 1;
            nfiles  = k_open( &com[1] );
            if ( nfiles != success ) {
               fprintf(stderr, "%s: Error opening command file <%s>.\n", 
		       cfg.Progname, &com[1] );
               return -1;
            }
            continue;
         }

/* Read configuration parameters
   *****************************/
         if ( k_its( (char*) "ModuleId" ) ) {
            if ( (str = k_str()) ) {
               if ( GetModId(str, &(cfg.QModuleId)) == -1 ) {
                  fprintf( stderr, "%s: Invalid ModuleId <%s>. \n", 
				cfg.Progname, str );
                  fprintf( stderr, "%s: Please Register ModuleId <%s> in earthworm.d!\n", 
				cfg.Progname, str );
                  return -1;
               }
            }
            init[0] = 1;
         } 
	 else if ( k_its((char*) "GapDir") ){
	   if( (str = k_str()) ){
	     strcpy(cfg.gapdir,str);
	   }
	   else{
	     fprintf(stderr,"%s: GapDir not specified in the configuration file\n",cfg.Progname);
	     return -1;
	   }
	 }
	 else if ( k_its((char*) "LatencyDir") ){
	   if( (str = k_str()) ){
	     strcpy(cfg.latdir,str);
	   }
	   else{
	     fprintf(stderr,"%s: LatencyDir not specified in the configuration file\n",cfg.Progname);
	     return -1;
	   }
	 }
	 else if ( k_its((char*) "LatencyLogPeriod") ){
	   if( (str = k_str()) ){
	     cfg.latlog_period = atoi(str);
	   }
	   else{
	     fprintf(stderr,"%s: LatencyLogPeriod not specified in the configuration file\n",cfg.Progname);
	     return -1;
	   }
	 }
	 else if ( k_its( (char*) "RingName" ) ) {
            if ( (str = k_str()) != NULL ) {
               if ( (cfg.RingKey = GetKey(str)) == -1 )
               {
                  fprintf( stderr, "%s: Invalid RingName <%s>. \n", cfg.Progname, str );
                  return -1;
               }
            }
            init[1] = 1;

         } else if ( k_its( (char*) "HeartbeatInt" ) ) {
            cfg.HeartbeatInt = k_int();
            init[2] = 1;

         } else if ( k_its( (char*) "LogFile" ) ) {
            cfg.LogFile = k_int();
            init[3] = 1;
         } else if ( k_its( (char*) "McastAddr" ) ) {
		if ( (str=k_str()) != NULL) {
			strcpy(cfg.mcast_addr, str);
		} else {
			fprintf(stderr, "%s: McastAddr not provided!\n", cfg.Progname);
			return -1;
		}
            init[6] = 1;
         } else if ( k_its( (char*) "Mcast" ) ) {
		/* this parameter should just be "net sta mcastif port"  and a default ring gets used */
	    char *Net, *Site, *Mif;
	    int port;
	    Net = k_str();
	    Site = k_str();
            Mif = k_str();
            port = k_int();
	    sprintf (search_name, "%s:%s:%s:%d", Net, Site, Mif, port);
//::	    strcpy(search_name, Net);
//:: 	    strcat(search_name, Site);
	    sta_iter = getStation(Net, Site, Mif, port, cfg);
	    if (sta_iter == cfg.sta_list.end()) {
	      try {
	    	sta = new StationMcast(Net, Site, Mif, cfg.mcast_addr, port);
              } catch (StationMcast::setup_exception se) {
		fprintf(stderr, "%s: McastAddr exception caught!\n", cfg.Progname);
		se.display();
		return -1;
              }
	        sname_copy = strdup(search_name);
		cfg.sta_list[sname_copy] = (const StationMcast &) sta;
		cfg.maxstation++;
	    } 
            init[4] = 1;
         } else if ( k_its( (char*) "McastRing" ) ) {
	    char *Net, *Site, *Mif, *ring, *ring_copy;
	    int port;
	    Net = k_str();
	    Site = k_str();
            Mif = k_str();
            port = k_int();
 	    ring = k_str();
	
	    if (ring != NULL) {
                /* see if we got the key for it already */
		if (cfg.RingName2Key.find(ring) == cfg.RingName2Key.end()) {
			long  key;
			if ( (key = GetKey(ring)) == -1 ) {
                  		fprintf( stderr, "%s: Invalid RingName <%s>. \n", cfg.Progname, ring );
                  		return -1;
			}
		   /* ring not in the list, add it in */
			ring_copy = strdup(ring);
			cfg.RingName2Key[ring_copy] = key;
#ifdef DEBUG
                  	fprintf( stderr, "DEBUG: %s Got Key %ld for RingName <%s>. \n", cfg.Progname, key, ring );
#endif
                }
                else
                {
#ifdef DEBUG
                  	fprintf( stderr, "DEBUG: %s RingName <%s> in list already. \n", cfg.Progname, ring );
#endif 
                }

            }
	    else
	    {
                  fprintf( stderr, "%s: McastRing line missing ring name %s %s %s %d\n", cfg.Progname,
				Net, Site, Mif, port);
		  return -1;
            }
	    sprintf (search_name, "%s:%s:%s:%d", Net, Site, Mif, port);
//:: 	    strcpy(search_name, Net);
//:: 	    strcat(search_name, Site);
	    sta_iter = getStation(Net, Site, Mif, port, cfg);
	    ring_copy = strdup(ring);
	    if (sta_iter == cfg.sta_list.end()) {
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s creating StationMcast for %s %s. \n", cfg.Progname, Net, Site);
#endif 
 	       try {
	    	  sta = new StationMcast(Net, Site, Mif, cfg.mcast_addr, port, ring_copy);
                } catch (StationMcast::setup_exception se) {
	  	  fprintf(stderr, "%s: McastAddr exception caught!\n", cfg.Progname);
		  se.display();
		  return -1;
                }
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s adding StationMcast %s %s to sta_list. \n", cfg.Progname, Net, Site);
#endif 
	        sname_copy = strdup(search_name);
		cfg.sta_list[sname_copy] = *sta;
		cfg.maxstation++;
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s added \n", cfg.Progname);
#endif 
	    } else {
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s:  %s %s already in sta list. \n", cfg.Progname, Net, Site);
		fprintf (stderr, "DEBUG: searchname is %s, adding ring %s to %s %s\n", search_name, ring, (*sta_iter).second.net, (*sta_iter).second.site);
#endif 
  	 	(*sta_iter).second.insertRing(ring_copy);
   	    }
            init[4] = 1;
         } else if ( k_its( (char*) "McastRingChannels" ) ) {
	    char *Net, *Site, *Mif, *ring, *ring_copy, *chans, *chan_copy;
	    int port;
	    Net = k_str();
	    Site = k_str();
            Mif = k_str();
            port = k_int();
 	    ring = k_str();
	    chans = k_str();
	
	    if (ring != NULL) {
                /* see if we got the key for it already */
		if (cfg.RingName2Key.find(ring) == cfg.RingName2Key.end()) {
			long  key;
			if ( (key = GetKey(ring)) == -1 ) {
                  		fprintf( stderr, "%s: Invalid RingName <%s>. \n", cfg.Progname, ring );
                  		return -1;
			}
		   /* ring not in the list, add it in */
			ring_copy = strdup(ring);
			cfg.RingName2Key[ring_copy] = key;
#ifdef DEBUG
                  	fprintf( stderr, "DEBUG: %s Got Key %ld for RingName <%s>. \n", cfg.Progname, key, ring );
#endif
                }
                else
                {
#ifdef DEBUG
                  	fprintf( stderr, "DEBUG: %s RingName <%s> in list already. \n", cfg.Progname, ring );
#endif 
                }

            }
	    else
	    {
                  fprintf( stderr, "%s: McastRingChannels line missing ring name %s %s %s %d\n", cfg.Progname,
				Net, Site, Mif, port);
		  return -1;
            }
	    if (chans == NULL) 
	    {
                  fprintf( stderr, "%s: McastRingChannels line missing channels list %s %s %s %d %s\n", cfg.Progname,
				Net, Site, Mif, port, ring);
		  return -1;
            }
	    sprintf (search_name, "%s:%s:%s:%d", Net, Site, Mif, port);
//:: 	    strcpy(search_name, Net);
//:: 	    strcat(search_name, Site);
	    sta_iter = getStation(Net, Site, Mif, port, cfg);
	    ring_copy = strdup(ring);
	    chan_copy = strdup(chans);
	    if (sta_iter == cfg.sta_list.end()) {
		// NEW STATION
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s: creating StationMcast for %s %s. \n", cfg.Progname, Net, Site);
#endif 
 	       try {
	    	  sta = new StationMcast(Net, Site, Mif, cfg.mcast_addr, port, ring_copy, chan_copy);
                } catch (StationMcast::setup_exception se) {
	  	  fprintf(stderr, "%s: McastRingChannels exception caught!\n", cfg.Progname);
		  se.display();
		  return -1;
                }
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s adding StationMcast %s %s to sta_list with chans %s for ring %s. \n", 
				cfg.Progname, Net, Site, chan_copy, ring_copy);
#endif 
	        sname_copy = strdup(search_name);
		cfg.sta_list[sname_copy] = *sta;
		cfg.maxstation++;
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s station added \n", cfg.Progname);
#endif 
	    } else {
		// OLD STATION, new ring and channels list
#ifdef DEBUG
               	fprintf( stderr, "DEBUG: %s: %s %s already in sta list. \n", cfg.Progname, Net, Site);
		fprintf (stderr, "DEBUG: searchname is %s, adding ring %s and chans %s to %s %s\n", search_name, ring, chan_copy, (*sta_iter).second.net, (*sta_iter).second.site);
#endif 
  	 	(*sta_iter).second.insertRing(ring_copy, chan_copy);
   	    }
            init[4] = 1;
         } else if ( k_its( (char*) "ChannelFile" ) ) {
            if (( (str = k_str()) != NULL ) && (strlen(str) < MAXSTR)) {
	      strcpy(cfg.ChannelFile, str);
            } else {
	      fprintf(stderr, "%s: Invalid ChannelFile %s\n", 
		      cfg.Progname, str);
	      return(-1);
	    }
            init[5] = 1;

         } else if ( k_its( (char*) "SCNL2pinmap") ) {
	    char *S,*C,*N,*L;
	    int pin;
	    S=k_str();
	    C=k_str();
	    N=k_str();
	    L=k_str();

	    pin=k_int();
	    if (insertSCNL(S,C,N,L,pin, cfg) == -1) {
                  fprintf( stderr, "%s: Invalid SCN2pinmap entry <%s %s %s %s %d>.\n",  
			cfg.Progname, S,C,N,L,pin);
                  fprintf( stderr, "Follow the SEED header definitions!\n");
	    }

	 } else {
	    /* An unknown parameter was encountered */
            fprintf( stderr, "%s: <%s> unknown parameter in <%s>\n", 
		cfg.Progname,com, configfile );
            return -1;
         }

/* See if there were any errors processing the command
   ***************************************************/
         if ( k_err() ) {
            fprintf( stderr, "%s: Bad <%s> command in <%s>.\n", 
		cfg.Progname, com, configfile );
            return -1;
         }
      }
      nfiles = k_close();
   }

/* After all files are closed, check flags for missed commands
   ***********************************************************/
   nmiss = 0;
	/* note the last argument is optional LOG2LogFile, hence
	the ncommand-1 in the for loop and not simply ncommand */
   for ( i = 0; i < ncommand-1; i++ )
      if ( !init[i] )
         nmiss++;

   if ( nmiss > 0 ) {
      fprintf( stderr,"%s: ERROR, no ", cfg.Progname );
      if ( !init[0]  ) fprintf(stderr, "<ModuleId> " );
      if ( !init[1]  ) fprintf(stderr, "<RingName> " );
      if ( !init[2] ) fprintf(stderr, "<HeartbeatInt> " );
      if ( !init[3] ) fprintf(stderr, "<LogFile> " );
      if ( !init[4] ) fprintf(stderr, "<Mcast> " );
      /* if ( !init[5] ) fprintf(stderr, "<ChannelFile> " ); */
      if ( !init[6] ) fprintf(stderr, "<McastAddr> " );
	/* note that LOG2LogFile is an optional one, default is 0=NO */
      fprintf(stderr, "command(s) in <%s>.\n", configfile );
      return -1;
   }
	
   if ( GetType( "TYPE_HEARTBEAT", &(cfg.TypeHB) ) != 0 ) {
      fprintf( stderr,
              "%s: Invalid message type <TYPE_HEARTBEAT>\n",cfg.Progname);
      return( -1 );
   }
   if ( GetType( "TYPE_TRACEBUF2", &(cfg.TypeTrace) ) != 0 ) {
      fprintf( stderr,
              "%s: Invalid message type <TYPE_TRACEBUF2>; exiting!\n", 
	       cfg.Progname);
        return(-1);
   }
   if ( GetType( "TYPE_ERROR", &(cfg.TypeErr) ) != 0 ) {
      fprintf( stderr,
              "%s: Invalid message type <TYPE_ERROR>\n", cfg.Progname);
      return( -1 );
   }

   /* build the datalogo */
   setuplogo(&(cfg.DataLogo), cfg);
   cfg.DataLogo.type = cfg.TypeTrace;

   return 0;
}


void setuplogo(MSG_LOGO *logo, ewConfig &cfg) {
   /* only get the InstId once */
   if (cfg.InstId == 255  && GetLocalInst(&(cfg.InstId)) != 0) {
      fprintf( stderr,
              "%s: Invalid Installation code; exiting!\n", cfg.Progname);
      exit(-1);
   }
   logo->mod = cfg.QModuleId;
   logo->instid = cfg.InstId;
}
