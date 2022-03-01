#include <cstring>
#include "StationMcast.h"



static char err_string[1024];


StationMcast::StationMcast()
{
}
StationMcast::StationMcast(char *n, char *s, char *m, char *maddr, short p) throw (StationMcast::setup_exception)
{
	init(n,s,m,maddr,p,NULL,NULL);
}
StationMcast::StationMcast(char *n, char *s, char *m, char *maddr, short p, char * ring) throw (StationMcast::setup_exception)
{
	init(n,s,m,maddr,p,ring,NULL);
}
StationMcast::StationMcast(char *n, char *s, char *m, char *maddr, short p, char * ring, char *chans) throw (StationMcast::setup_exception)
{
	init(n,s,m,maddr,p,ring,chans);
}

#define MCAST_ALLCHANS  "*"

void StationMcast::init(char *n, char *s, char *m, char *maddr, short p, char *ring, char *chans) throw (StationMcast::setup_exception)
{
#ifdef DEBUG
      if (ring != NULL && chans != NULL) 
        fprintf(stderr, "DEBUG: StationMcast::init(), setting up mcast for entry: %s %s %s %d %s %s\n", n, s, m, p, ring, chans) ;
      else if (ring != NULL) 
        fprintf(stderr, "DEBUG: StationMcast::init(), setting up mcast for entry: %s %s %s %d %s\n", n, s, m, p, ring);
#endif
  strcpy(net, n);
  strcpy(site, s);
  strcpy(mcastif, m);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: all strings copied\n");
#endif
  if (p >0 && p < 65536) 
  {
     port = p;
  } 
  else
  {
     sprintf(err_string, "Port %d for Mcast line: %s %s %s is out of range, must be positive & less than 65536!\n", p, n, s, m);
     throw(setup_exception(err_string));
  }

  if (ring != NULL) 
  {
#ifdef DEBUG
      fprintf(stderr, "DEBUG: adding ring to RingNames entry: %s\n", ring);
#endif
     RingNames[ring] = 1;
  }
  if (ring != NULL && chans != NULL) {
     // add the channels to the list for this ring
    char *ring_copy;
    ring_copy = strdup(ring);
#ifdef DEBUG
    fprintf(stderr, "DEBUG: Adding in chans: %s for ring %s\n", chans, ring);
#endif
    RingName2ChanList[ring_copy] = chans;
#ifdef DEBUG
    Name2NameMap::iterator it;
	fprintf (stderr, "DEBUG: Printing list of chans in RingName2ChanList map:\n");
	for (it = RingName2ChanList.begin(); it != RingName2ChanList.end(); it++)
		fprintf (stderr, "DEBUG: chans in list for %s %s rings are: %s %s\n", 
			net, site, (*it).first, (*it).second);
#endif
  }

  if (ring != NULL && chans == NULL) {
    char *ring_copy, *cptr;
    ring_copy = strdup(ring);
    cptr = new char[strlen(MCAST_ALLCHANS)+2];
    strcpy(cptr, MCAST_ALLCHANS);
#ifdef DEBUG
    fprintf(stderr, "DEBUG: No Chans specified, Adding ALL in chans for ring %s\n",  ring);
#endif
    RingName2ChanList[ring_copy] = cptr;
  }

  // create the SocketMcast
  try {
#ifdef DEBUG
      fprintf(stderr, "DEBUG: Connecting to mcast for entry: %s %s %s %d\n", n, s, m, p);
#endif
      sm = new SocketMcast(mcastif, maddr, (short) port);
#ifdef DEBUG
      fprintf(stderr, "DEBUG: Connected\n");
#endif
  } catch (SocketMcast::connection_exception ce) {
     fprintf(stderr, "SocketMcast:: exception :");
     ce.display();
     sprintf(err_string, "Problem connecting to mcast if for entry: %s %s %s %d\n", n, s, m, p);
     throw(setup_exception(err_string));
  }
  sockfd = sm->getSocketFD();
#ifdef DEBUG
      fprintf(stderr, "DEBUG: Connected to socket %d\n", sockfd);
#endif
}

// ----------------------
// empty for now...should free all chars, but who cares for now
StationMcast::~StationMcast()
{
}

// ----------------------
// add just the ring and wild card the channels.
void StationMcast::insertRing(char * ring)
{
    char *cptr;

    RingNames[ring] = 1;

    // this now means chans are wildcarded
    cptr = new char[strlen(MCAST_ALLCHANS)+2];
    strcpy(cptr, MCAST_ALLCHANS);
#ifdef DEBUG
    fprintf(stderr, "DEBUG: No Chans specified, Adding ALL in chans for ring %s\n",  ring);
#endif
    RingName2ChanList[ring] = cptr;
}

// ----------------------
// add both the ring and the channels in.
void StationMcast::insertRing(char * ring, char * chans)
{
     RingNames[ring] = 1;
     RingName2ChanList[ring] = chans;
}

// ----------------------
//  returns true if loc, chan is in the list or false if list is null 
// there is some gross assumptions here and this does not deal with wild cards
// it assumes that the chan list string is in the format CCC.LL,CCC.LL
//   this can be made smarter if wildcards are needed...but for now, this is a test
//   implementation to save some time
//
bool StationMcast::isChanInRing(char * ringname, char * loc, char * chan)
{

 char tmp_search_str[8];
 Name2NameMap::iterator tmp_iter;
	if (ringname == NULL || loc == NULL || chan == NULL) 
 	{	
		return false;
	}
	if (strlen(chan) != 3 || strlen(loc) != 2)
 	{	
		// this should REALLY throw an exception, 
		// but I expect this all to change once wild cards are desired
		return false;
	}
	tmp_search_str[0] =0;
	strcpy(tmp_search_str, chan);
	strcat(tmp_search_str, ".");
	strcat(tmp_search_str, loc);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: StationMcast::isChanInRing: in %s %s searching for chan.loc %s \n", 
		net, site, tmp_search_str);
#endif
	tmp_iter = RingName2ChanList.find(ringname);
	if (tmp_iter == RingName2ChanList.end())
 	{	
#ifdef DEBUG
	fprintf(stderr, "DEBUG: StationMcast::isChanInRing: No ChanList for ring %s found\n", ringname);
#endif
		// This means that no channel selectors were set, all channels pass through
		return true;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: StationMcast::isChanInRing: searching for %s in %s\n", 
		tmp_search_str, (char *) (*tmp_iter).second);
#endif
	// to support just CCC as the chan list (no . chars for locs), this line is used for the str compare
	if (strrchr((char *) (*tmp_iter).second, '.') == NULL)
 	{	
		strcpy(tmp_search_str, chan);
	}
	
	if (strstr( (char *) (*tmp_iter).second, tmp_search_str) != NULL) 
 	{	
		return true;
	}
        if (strcmp((char *) (*tmp_iter).second, MCAST_ALLCHANS) == 0) 
 	{	
		// wildcard for all chans used
		return true;
	}
        return false;
}


// ----------------------
// start a loop on the list of rings in the RingNames map
void StationMcast::StartRingLoop()
{
	RingIterator = RingNames.begin();
}



// ----------------------
// return the next ring name in the list, or NULL when at end of list
char * StationMcast::getNextRingName()
{
 Name2IntMap::iterator tmp_iter;
    if (RingIterator == RingNames.end()) 
	return NULL;
    tmp_iter = RingIterator;
    RingIterator++;
    return (char *) (*tmp_iter).first;
}

StationMcast& StationMcast::operator=(const StationMcast &s)
{
	strcpy(net, s.net);
	strcpy(site, s.site);
	strcpy(mcastif, s.mcastif);
	port = s.port;
	sm = s.sm;
	sockfd = s.sockfd;
	RingNames = s.RingNames;
	RingName2ChanList = s.RingName2ChanList;
	return (*this);
}
