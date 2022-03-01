#ifndef __mcastfeeder_h
#define __mcastfeeder_h

#include <pthread.h>
#include <pcap.h>
#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include "inetheader.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ios>
#include <iomanip>
#include <map>
#include <list>
#include "qlib2.h"
#include "TimeStamp.h"
#include "Duration.h"
#include "readraw.h"
#include "PacketBuffer.h"
#include "LockedQueue.h"
#include "DataChannel.h"
#include "globals.h"
#include "version.h"
#include "Compat.h"

#include "WaveformFeeder.h"

typedef LockedQueue< PacketBuffer > PacketBufferQueue;
typedef map< string , PacketBufferQueue* > PacketQueueMap;

Channel getChannelFromMSEED(const char* buf) throw(Error);  
Channel getChannelFromRaw(const char* buf) throw(Error);

class mcastfeeder : public WaveformFeeder {
public:
  void start(string config, int orgtime, WPFactory* pFactory) throw(Error);
  void stop();
  static void* process_packet(void* _pq);

}; // class mcastfeeder

#endif
