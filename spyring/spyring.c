#ifndef lint
static char id[] = "@(#) $Id: spyring.c,v 1.7 2016/06/30 02:15:25 doug Exp $";
#endif
/************************************************************************/
/*			spy_ring.c					*/
/*									*/
/*  Program to read everything from a ring and dump its contents.	*/
/*									*/
/*  Usage: spy_ring <ring_name>						*/
/************************************************************************/

#include <stdio.h>

#define	VERSION	"1.3.6 (2017.174)"
char *syntax[] = {
"%s - Version " VERSION,
"%s  [-f] [-d n] [-h] ring_name",
"    where:",
"	-f		Flush ring before starting to spy.",
"	-d n		Set debug value to n.",
"			1 = contents of ew message",
"			2 = datavalues from waveforms",
"	-h		Help - prints syntax message.",
"	ring_name	Name of ring to spy on.",
NULL };

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <transport.h>
#include <earthworm.h>
#include <trace_buf.h>
#include <time_ew.h>

#include <qlib2.h>

#define BUFSIZE 10000
#define	DEBUG
#define	DEBUG_MSG	1
#define	DEBUG_TRACEBUF	2
#define	DEBUG_ANY	\
    (DEBUG_MSG | DEBUG_TRACEBUF)
#define	    debug(val)  (debug_option & (val))

unsigned char mod_wildcard;
unsigned char inst_wildcard;
unsigned char type_wildcard;
unsigned char type_heartbeat;
unsigned char type_error;
unsigned char type_pick2k;
unsigned char type_pickSCNL;
unsigned char type_coda2k;
unsigned char type_codaSCNL;
unsigned char type_mhh;
unsigned char type_mhhSCNL;
unsigned char type_thresh;
unsigned char type_threshSCNL;
unsigned char type_tracebuf;
unsigned char type_tracebuf2;
unsigned char type_mstracebuf;
unsigned char type_link;
unsigned char type_quake2k;
unsigned char type_faststatrig;
unsigned char type_carlstatrigSCNL;
unsigned char type_fastsubtrig;
unsigned char type_triglistSCNL;
unsigned char type_elarms_pgm;
double tnow;

/* Things to read or derive from configuration file
 **************************************************/
#define ALV_LEN 256
static char    *RingName;           /* name of transport ring for i/o    */
static long    MaxMsgSize;          /* max size for input/output msgs    */
static long int	ShmKey = 0;	    /* Key of the transport ring to read from */

char *cmdname;
FILE *info;
static int terminate_proc = 0;
static int debug_option = 0;

char *dump_mstracebuf_hdr (char *buf, int buflen);
char *dump_ewtracebuf_hdr (char *buf, int buflen);
char *dump_ewtracebuf2_hdr (char *buf, int buflen);
void dump_mstracebuf_data (char *buf, int buflen, FILE *fp);
void dump_ewtracebuf_data (char *buf, int buflen, FILE *fp);

/************************************************************************/
/*  finish_handler:							*/
/*	Signal handler -  sets terminate flag so we can exit cleanly.	*/
/************************************************************************/
void finish_handler(int sig)
{
    signal (sig,finish_handler);    /* Re-install handler (for SVR4)	*/
    terminate_proc = 1;
    return;
}

#define	PRINT_CONTENTS(str,con,len) \
    printf ("%-11.11s ", str); \
    if (con[0]) printf ("msg=%-*.*s%s",len, len, con, ((len>0 && con[len-1]!='\n') ? "\n" : "")); \
    else printf ("\n"); \
    fflush (stdout)

int print_contents (MSG_LOGO logo, char *gotmsg, int gotsize)
{
    char buf[1024], *str_type;
    char *contents = buf;
    int contents_len = 0;

    buf[0] = '\0';
    if (logo.type == type_heartbeat) {
	str_type = "HEARTBEAT";
	if (debug(DEBUG_MSG)) {
	    INT_TIME it;
	    struct timeval tv;
	    strncpy(contents,gotmsg,gotsize);
	    contents_len = gotsize;
	    contents[contents_len] = '\0';
	    if (contents[contents_len-1] == '\0') contents_len--;
	    if (contents[contents_len-1] == '\n') {
		contents_len--;
		contents[contents_len] = '\0';
	    }
	    sscanf (contents, "%ld", &tv.tv_sec);
	    tv.tv_usec = 0;
	    it = int_time_from_timeval(&tv);
	    sprintf (contents+contents_len, " (%s)\n",
		     time_to_str(it, MONTHS_FMT_1));
	    contents_len = strlen(contents);
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_error) {
	str_type = "ERROR";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_pick2k) {
	str_type = "PICK2K";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_pickSCNL) {
	str_type = "PICK_SCNL";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_coda2k) {
	str_type = "CODA2K";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_codaSCNL) {
	str_type = "CODA_SCNL";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_mhh)   {
	str_type = "MHH";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_mhhSCNL)   {
	str_type = "MHH_SCNL";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_thresh)   {
	str_type = "THRESH";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_threshSCNL)   {
	str_type = "THRESH_SCNL";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_link)   {
	str_type = "LINK";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_quake2k)   {
	str_type = "QUAKE2K";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_faststatrig)   {
	str_type = "FASTSTATRIG";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_carlstatrigSCNL)   {
	str_type = "CARLSTATRIG_SCNL";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_fastsubtrig)   {
	str_type = "FASTSUBTRIG";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_triglistSCNL)   {
	str_type = "TRIGLIST_SCNL";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else if (logo.type == type_tracebuf) {
	str_type = "TRACEBUF";
	if (debug(DEBUG_MSG)) {
	    contents = dump_ewtracebuf_hdr (gotmsg, gotsize);
	    contents_len = strlen(contents);
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
	if (debug(DEBUG_TRACEBUF)) {
	    dump_ewtracebuf_data (gotmsg, gotsize, stdout);
	}
    }
    else if (logo.type == type_tracebuf2) {
	str_type = "TRACEBUF2";
	if (debug(DEBUG_MSG)) {
	    contents = dump_ewtracebuf2_hdr (gotmsg, gotsize);
	    contents_len = strlen(contents);
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
	if (debug(DEBUG_TRACEBUF)) {
	    dump_ewtracebuf_data (gotmsg, gotsize, stdout);
	}
    }
    else if (logo.type == type_mstracebuf) {
	str_type = "MSTRACEBUF";
	if (debug(DEBUG_MSG)) {
	    contents = dump_mstracebuf_hdr (gotmsg, gotsize);
	    contents_len = strlen(contents);
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
	if (debug(DEBUG_TRACEBUF)) {
	    dump_mstracebuf_data (gotmsg, gotsize, stdout);
	}
    }
    else if (logo.type == type_elarms_pgm) {
	str_type = "ELARMS_PGM";
	if (debug(DEBUG_MSG)) {
	    contents = gotmsg;
	    contents_len = gotsize;
	}
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    else {
	str_type = "UNKNOWN";
	PRINT_CONTENTS(str_type,contents,contents_len);
    }
    return (0);
}

int main( int argc, char *argv[] )
{
    FILE     *fp;
    SHM_INFO region;
    MSG_LOGO getlogo[1], logo;
    char     *gotmsg;
    long     gotsize;
    int      i;
    int      res;
    int	     flush_ring = 0;
    int	     nflushed = 0;

    /* Variables needed for getopt. */
    extern char	*optarg;
    extern int	optind, opterr;
    int		c;

    info = stdout;

/* Catch broken socket signals
******************************/
#ifdef _SOLARIS
   (void)sigignore(SIGPIPE);
#endif
   terminate_proc = 0;
   signal (SIGINT,finish_handler);
   signal (SIGTERM,finish_handler);

/* Check command line arguments
****************************/
    cmdname = tail(argv[0]);
    get_my_wordorder();
    while ( (c = getopt(argc,argv,"hfd:")) != -1)
    switch (c) {
    case '?':
    case 'h':   print_syntax (cmdname,syntax,info); exit(0);
    case 'f':	flush_ring = 1; break;
    case 'd':   debug_option = atoi(optarg); break;
    default:
	fprintf (info, "%s: Unknown option: -%c\n", cmdname, c);
	exit(1);
    }
    argv = &(argv[optind]);
    argc -= optind;
    if ( argc < 1 ) {
	print_syntax (cmdname,syntax,info);
	exit(-1);
    }

/* Attach to ring
**************/
    RingName = argv[0];
    if( ( ShmKey = GetKey(RingName) ) == -1 ) {
	fprintf( stderr,
		 "%s:  Invalid or missing ShmKey for name <%s>; exiting!\n", 
		 cmdname, RingName);
	exit( -1 );
    }
    tport_attach( &region, ShmKey );

/* Allocate message buffer
***********************/
    gotmsg = (char *) malloc( BUFSIZE );
    if ( gotmsg == NULL )
    {
	printf( "Error allocating gotmsg.\n" );
	return -1;
    }

/* Specify logos to get
********************/
    if ( GetInst( "INST_WILDCARD", &inst_wildcard ) != 0 )
    {
	printf( "%s: Error getting INST_WILDCARD\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_ERROR", &type_error ) != 0 )
    {
	printf( "%s: Error getting TYPE_ERROR\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_PICK2K", &type_pick2k ) != 0 )
    {
	printf( "%s: Error getting TYPE_PICK2K\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_PICK_SCNL", &type_pickSCNL ) != 0 )
    {
	printf( "%s: Error getting TYPE_PICK_SCNL\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_HEARTBEAT", &type_heartbeat ) != 0 )
    {
	printf( "%s: Error getting TYPE_HEARTBEAT\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_CODA2K", &type_coda2k ) != 0 )
    {
	printf( "%s: Error getting TYPE_CODA2K\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_CODA_SCNL", &type_codaSCNL ) != 0 )
    {
	printf( "%s: Error getting TYPE_CODA_SCNL\n", cmdname );
	return -1;
    }
#ifdef UCB_SPECIFIC 
    if ( GetType( "TYPE_MHH", &type_mhh ) != 0 )
    {
	printf( "%s: Error getting TYPE_MHH\n", cmdname );
	return -1;
    }

    if ( GetType( "TYPE_MHH_SCNL", &type_mhhSCNL ) != 0 )
    {
	printf( "%s: Error getting TYPE_MHH_SCNL\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_THRESH", &type_thresh ) != 0 )
    {
	printf( "%s: Error getting TYPE_THRESH\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_THRESH_SCNL", &type_threshSCNL ) != 0 )
    {
	printf( "%s: Error getting TYPE_THRESH_SCNL\n", cmdname );
	return -1;
    }

    if ( GetType( "TYPE_LINK", &type_link ) != 0 )
    {
	printf( "%s: Error getting TYPE_LINK\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_QUAKE2K", &type_quake2k ) != 0 )
    {
	printf( "%s: Error getting TYPE_QUAKE2K\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_FASTSTATRIG", &type_faststatrig ) != 0 )
    {
	printf( "%s: Error getting TYPE_FASTSTATRIG\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_CARLSTATRIG_SCNL", &type_carlstatrigSCNL ) != 0 )
    {
	printf( "%s: Error getting TYPE_CARLSTATRIG_SCNL\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_FASTSUBTRIG", &type_fastsubtrig ) != 0 )
    {
	printf( "%s: Error getting TYPE_FASTSUBTRIG\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_TRIGLIST_SCNL", &type_triglistSCNL ) != 0 )
    {
	printf( "%s: Error getting TYPE_TRIGLIST_SCNL\n", cmdname );
	return -1;
    }
#endif 
    if ( GetType( "TYPE_TRACEBUF", &type_tracebuf ) != 0 )
    {
	printf( "%s: Error getting TYPE_TRACEBUF\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_TRACEBUF2", &type_tracebuf2 ) != 0 )
    {
	printf( "%s: Error getting TYPE_TRACEBUF2\n", cmdname );
	return -1;
    }
#ifdef UCB_SPECIFIC 
    if ( GetType( "TYPE_MSTRACEBUF", &type_mstracebuf ) != 0 )
    {
	printf( "%s: Error getting TYPE_MSTRACEBUF\n", cmdname );
	return -1;
    }
 
    if ( GetType( "TYPE_ELARMS_PGM", &type_elarms_pgm ) != 0 )
    {
	printf( "%s: Error getting TYPE_ELARMS_PGM\n", cmdname );
	return -1;
    }
#endif 
    if ( GetType( "TYPE_WILDCARD", &mod_wildcard ) != 0 )
    {
	printf( "getter: Error getting TYPE_WILDCARD\n" );
	return -1;
    }

    if ( GetModId( "MOD_WILDCARD", &mod_wildcard ) != 0 )
    {
	printf( "getter: Error getting MOD_WILDCARD\n" );
	return -1;
    }

    /* Get everything */
    getlogo[0].instid = inst_wildcard;
    getlogo[0].type  = type_wildcard;
    getlogo[0].mod   = mod_wildcard;

    if (flush_ring) {
	while ((res = tport_getmsg( &region, getlogo, (short)4, &logo,&gotsize,
				    (char *)gotmsg, BUFSIZE )) != GET_NONE) ++nflushed;
    }
/*    if (flush_ring) printf ("flushed = %d, res = %d\n", nflushed, res); */

idle:
    sleep_ew( 100 );

    if ( tport_getflag( &region ) == TERMINATE || terminate_proc ) {
	tport_detach( &region );
	return 0;
    }

    while ( 1 ) {
	res = tport_getmsg( &region, getlogo, (short)4,
			    &logo, &gotsize, (char *)gotmsg, BUFSIZE );
	
	switch( res ) {
	case GET_NONE:
	    goto idle;
	    
	case GET_TOOBIG:
	    printf( "%s: Retrieved message too big (%d) for gotmsg\n", cmdname,
		    gotsize );
	    fflush (stdout);
	    goto idle;

	case GET_NOTRACK:
	    printf ( "%s: NTRACK_GET exceeded\n", cmdname );
	    fflush (stdout);
	    break;
	case GET_MISS:
	    printf ( "%s: Missed message(s) for ", cmdname);
	    printf ("instid=%3d mod=%3d type=%3d\n",
		    logo.instid, logo.mod, logo.type);
	    fflush (stdout);
	    break;
	case GET_OK:
	    /* Print the logo */
	    printf ("instid=%3d mod=%3d type=%3d size=%4d ",
		    logo.instid, logo.mod, logo.type, gotsize);
	    /* If it is a type that we know, print the name of the type	*/
	    hrtime_ew(&tnow);
	    print_contents (logo, gotmsg, gotsize);
	    fflush (stdout);
	}
    }
}

char *dump_mstracebuf_hdr (char *buf, int buflen) 
{
    DATA_HDR *hdr = NULL;
    static char msg[5120];

    hdr = decode_hdr_sdr ((SDR_HDR *)buf, buflen);
    if (hdr) {
	dump_hdr (hdr, msg, MONTHS_FMT_1);
    }
    else {
	strcpy (msg, "<bad mstracebuf>\n");
    }
    if (hdr) free_data_hdr (hdr);
    return (msg);
}

void dump_mstracebuf_data (char *buf, int buflen, FILE *fp) 
{
    DATA_HDR *hdr = NULL;
    char msg[5120];
    int *data = NULL;
    int i, n;

    hdr = decode_hdr_sdr ((SDR_HDR *)buf, buflen);
    if (hdr) {
	data = (int *)malloc(hdr->num_samples * sizeof(int));
	if (data) {
	    n = ms_unpack (hdr, hdr->num_samples, buf, data);
	    if (n != hdr->num_samples) {
		fprintf (fp, "Error unpacking data: expected %d, got %d\n",
			 hdr->num_samples, n);
	    }
	    for (i=0; i<n; i++) fprintf (fp, "%d\n", data[i]);
	}
	else {
	    fprintf (fp, "Error mallocing data buffer of size %d\n", hdr->num_samples);
	}
    }
    else {
	fprintf (fp, "<bad mstracebuf>\n");
    }
    if (data) free (data);
    if (hdr) free_data_hdr (hdr);
    return;
}

char *dump_ewtracebuf_hdr (char *buf, int buflen) 
{
    static char msg[5120];
    TRACE_HEADER trace_header;
    INT_TIME it;
    char *starttime = NULL;
    char *endtime = NULL;
    struct timeval tv;
    int wordorder;
    double max_latency, min_latency, avg_latency;

    memcpy ((void *)&trace_header, (void *)buf, sizeof(TRACE_HEADER));
    switch (trace_header.datatype[0]) {
    case 'f':
    case 'i':   wordorder = SEED_LITTLE_ENDIAN; break;
    case 't':
    case 's':	wordorder = SEED_BIG_ENDIAN; break;
    default:
	sprintf (msg, "Unknown byteorder: %s\n", trace_header.datatype);
	return (msg);
    }
    if (my_wordorder != wordorder) {
	swab4 (&trace_header.pinno);
	swab4 (&trace_header.nsamp);
	swab8 (&trace_header.starttime);
	swab8 (&trace_header.endtime);
	swab8 (&trace_header.samprate);
    }

    tv.tv_sec = trace_header.starttime;
    tv.tv_usec = (int)((trace_header.starttime-tv.tv_sec)*USECS_PER_SEC);
    it = int_time_from_timeval(&tv);
    starttime = strdup(time_to_str(it, MONTHS_FMT_1));

    tv.tv_sec = trace_header.endtime;
    tv.tv_usec = (int)((trace_header.endtime-tv.tv_sec)*USECS_PER_SEC);
    it = int_time_from_timeval(&tv);
    endtime = strdup (time_to_str(it, MONTHS_FMT_1));

    max_latency = (double)(tnow - trace_header.starttime);
    min_latency = (double)(tnow - trace_header.endtime);
    avg_latency = (max_latency+min_latency)/2.;
    sprintf (msg, "%s.%s.%s pin=%d ns=%d start=%.4lf(%s) end=%.4lf(%s) "
	     "rate=%.2lf, fmt=%s flags=%-2.2s latency=(%.4lf,%.4lf,%.4lf)\n",
	     trace_header.sta, trace_header.net, trace_header.chan,
	     trace_header.pinno, trace_header.nsamp, 
	     trace_header.starttime, starttime, trace_header.endtime, endtime,
	     trace_header.samprate,
	     trace_header.datatype, trace_header.quality,
	     min_latency, max_latency, avg_latency);
    if (starttime) free (starttime);
    if (endtime) free (endtime);
    return (msg);
}

char *dump_ewtracebuf2_hdr (char *buf, int buflen) 
{
    static char msg[5120];
    TRACE2_HEADER trace_header;
    INT_TIME it;
    char *starttime = NULL;
    char *endtime = NULL;
    struct timeval tv;
    int wordorder;
    double max_latency, min_latency, avg_latency;

    memcpy ((void *)&trace_header, (void *)buf, sizeof(TRACE_HEADER));
    switch (trace_header.datatype[0]) {
    case 'f':
    case 'i':   wordorder = SEED_LITTLE_ENDIAN; break;
    case 't':
    case 's':	wordorder = SEED_BIG_ENDIAN; break;
    default:
	sprintf (msg, "Unknown byteorder: %s\n", trace_header.datatype);
	return (msg);
    }
    if (my_wordorder != wordorder) {
	swab4 (&trace_header.pinno);
	swab4 (&trace_header.nsamp);
	swab8 (&trace_header.starttime);
	swab8 (&trace_header.endtime);
	swab8 (&trace_header.samprate);
    }

    tv.tv_sec = trace_header.starttime;
    tv.tv_usec = (int)((trace_header.starttime-tv.tv_sec)*USECS_PER_SEC);
    it = int_time_from_timeval(&tv);
    starttime = strdup(time_to_str(it, MONTHS_FMT_1));

    tv.tv_sec = trace_header.endtime;
    tv.tv_usec = (int)((trace_header.endtime-tv.tv_sec)*USECS_PER_SEC);
    it = int_time_from_timeval(&tv);
    endtime = strdup (time_to_str(it, MONTHS_FMT_1));

    max_latency = (double)(tnow - trace_header.starttime);
    min_latency = (double)(tnow - trace_header.endtime);
    avg_latency = (max_latency+min_latency)/2.;
    sprintf (msg, "%s.%s.%s.%s ver=%c.%c pin=%d ns=%d start=%.4lf(%s) end=%.4lf(%s) "
	     "rate=%.2lf, fmt=%s flags=%-2.2s latency=(%.4lf,%.4lf,%.4lf)\n",
	     trace_header.sta, trace_header.net, trace_header.chan, 
	     trace_header.loc, trace_header.version[0], trace_header.version[1],
	     trace_header.pinno, trace_header.nsamp, 
	     trace_header.starttime, starttime, trace_header.endtime, endtime,
	     trace_header.samprate,
	     trace_header.datatype, trace_header.quality,
	     min_latency, max_latency, avg_latency);
    if (starttime) free (starttime);
    if (endtime) free (endtime);
    return (msg);
}

void dump_ewtracebuf_data (char *buf, int buflen, FILE *fp) 
{
    char msg[5120];
    TRACE_HEADER trace_header;
    INT_TIME it;
    char *starttime = NULL;
    char *endtime = NULL;
    struct timeval tv;
    int wordorder;
    short int *i2data;
    int *i4data;
    float *f4data;
    double *f8data;
    int i;

    memcpy ((void *)&trace_header, (void *)buf, sizeof(TRACE_HEADER));
    switch (trace_header.datatype[0]) {
    case 'f':
    case 'i':	wordorder = SEED_LITTLE_ENDIAN; break;
    case 't':	
    case 's':	wordorder = SEED_BIG_ENDIAN; break;
    default:
	fprintf (fp, "Unknown byteorder: %s\n", trace_header.datatype);
	return;
    }
    if (my_wordorder != wordorder) {
	swab4 (&trace_header.pinno);
	swab4 (&trace_header.nsamp);
	swab8 (&trace_header.starttime);
	swab8 (&trace_header.endtime);
	swab8 (&trace_header.samprate);
    }

    if (strcmp(trace_header.datatype,"i2")==0 || strcmp(trace_header.datatype,"s2")==0) {
	i2data = (short int *)(buf + sizeof(TRACE_HEADER));
	short int val;
	for (i=0; i<trace_header.nsamp; i++) {
	    memcpy ((void *)&val, (void *)&i2data[i], 2);
	    if (my_wordorder != wordorder) swab2(&val);
	    fprintf (fp, "%hd\n", val);
	}
    }


    else if (strcmp(trace_header.datatype,"i4")==0 || strcmp(trace_header.datatype,"s4")==0) {
	i4data = (int *)(buf + sizeof(TRACE_HEADER));
	int val;
	for (i=0; i<trace_header.nsamp; i++) {
	    memcpy ((void *)&val, (void *)&i4data[i], 4);
	    if (my_wordorder != wordorder) swab4(&val);
	    fprintf (fp, "%d\n", val);
	}
    }

    else if (strcmp(trace_header.datatype,"f4")==0 || strcmp(trace_header.datatype,"t4")==0) {
	f4data = (float *)(buf + sizeof(TRACE_HEADER));
	float val;
	for (i=0; i<trace_header.nsamp; i++) {
	    memcpy ((void *)&val, (void *)&f4data[i], 4);
	    if (my_wordorder != wordorder) swabf(&val);
	    fprintf (fp, "%g\n", val);
	}
    }

    else if (strcmp(trace_header.datatype,"f8")==0 || strcmp(trace_header.datatype,"t4")==0) {
	f8data = (double *)(buf + sizeof(TRACE_HEADER));
	double val;
	for (i=0; i<trace_header.nsamp; i++) {
	    memcpy ((void *)&val, (void *)&f8data[i], 8);
	    if (my_wordorder != wordorder) swab8(&val);
	    fprintf (fp, "%lf\n", val);
	}
    }

    else {
	fprintf (fp, "Unable to decode tracebuf format: %s\n", trace_header.datatype);
    }

    return;
}
