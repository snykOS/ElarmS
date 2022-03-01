#ifndef STATION_MCAST_H
#define STATION_MCAST_H
#include <map>
#include <cstring>
#include "mcastutil.h"
#include "qlib2.h"
using namespace std;

struct LessStr
{
    bool operator() (const char* s1, const char *s2) const
    {
	return strcmp(s1, s2) < 0;
    }
}; 


#define MAX_LOC_CHARS 3
#define MAX_CHAN_CHARS 4
class SimpleChan {
 private: 
    char location[MAX_LOC_CHARS];
    char channel[MAX_CHAN_CHARS];
 public:
    SimpleChan() {
	location[0]=0;
	channel[0]=0;
    }
    SimpleChan(char *l, char *c) {
	strcpy(location, l);
	strcpy(channel, c);
    }
    ~SimpleChan() {}
    void setLocation(char *l) {
	strcpy(location, l);
    }
    void setChannel(char *c) {
	strcpy(channel, c);
    }
    bool isEqual(char *l, char *c) {
	if(strcmp(l, location)==0 && strcmp(c,channel)==0) {
	    return true;
	} else {
	    return false;
	}
    }
};


typedef map<const char*, int, LessStr, allocator<pair<const char *, int> > >  Name2IntMap;
typedef map<const char*, char*, LessStr, allocator<pair<const char*, char* > > >  Name2NameMap;

class StationMcast {

 public:

    class setup_exception {
    private:
	const char * err_string;
    public:
	setup_exception(const char  *str) {
	    err_string = str;
	};
	~setup_exception() {};
	void display() {
	    cerr << err_string << endl;
	};
    };

    char  net[3];          /* network code SEED */
    char  site[6];
    char  mcastif[1024];    // multicast address
    short port;
    int   sockfd;
    SocketMcast *sm;
    Name2IntMap RingNames;
    Name2NameMap RingName2ChanList;
    Name2IntMap::iterator RingIterator;

    StationMcast();
    StationMcast(char *n, char *s, char *m, char *maddr, short p) throw (setup_exception);
    StationMcast(char *n, char *s, char *m,  char *maddr,short p, char * ring) throw (setup_exception);
    StationMcast(char *n, char *s, char *m,  char *maddr,short p, char * ring, char *chans) throw (setup_exception);
    ~StationMcast();

    void init(char *n, char *s, char *m,  char *maddr,short p, char * ring, char * chans) throw (setup_exception);
    void insertRing(char * ring);
    void insertRing(char * ring, char *chans);
    void StartRingLoop();
    char *getNextRingName();
    bool isChanInRing(char * ringname, char * loc, char * chan);
    StationMcast& operator=(const StationMcast &s);

};
#endif
