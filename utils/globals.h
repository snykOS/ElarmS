#ifndef __globals_h
#define __globals_h

#include <map>
#include <list>
#include <stdio.h>
#include <string>
#include <string.h>

#define RCSID_globals_h "$Id: globals.h $"

const std::string UTILS_VERSION = "2.5.7 2017-01-24";


#define PSTEP() {int l = __LINE__;printf("At line:%d\n",l);}

const int UNCOMP_PACKET_SIZE  = 1024;
const int COMP_PACKET_SIZE = 512;
const int PACKET_SIZE = 512;
const int MAX_THREADS = 1000;
const int  MAX_FILENAME = 1024;

extern int errno;

/* Queue ring indexes and bit positions */
#define DATAQ 0
#define DETQ 1
#define CALQ 2
#define TIMQ 3
#define MSGQ 4
#define BLKQ 5

#define DAT_INDEX	DATAQ
#define DET_INDEX	DETQ
#define	CAL_INDEX	CALQ
#define	CLK_INDEX	TIMQ
#define	LOG_INDEX	MSGQ
#define	BLK_INDEX	BLKQ


enum ChannelFlag {UNDEFINED=0, WHITE=1,GREY,BLACK}; 

std::string ChannelFlagName(ChannelFlag val);

/* Testing ... need initialisation of print_lock - temporarily in mathfn.cc */
class logging {
private:
    static pthread_mutex_t* print_lock;

public:
    static void setPrintLock(pthread_mutex_t *printLock) { print_lock = printLock; }
    static void printLock() { if(print_lock != NULL) pthread_mutex_lock(print_lock); }
    static void printUnlock() { if(print_lock != NULL) pthread_mutex_unlock(print_lock); }
};

#endif
