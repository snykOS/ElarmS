#ifndef __ewfeeder_h
#define __ewfeeder_h

#include <iostream>
#include <map>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <strings.h>
#include "zlib.h"

#include "RetCodes.h"
#include "TimeStamp.h"
#include "Channel.h"
#include "DataChannel.h"
#include "PacketBuffer.h"
#include "LockedQueue.h"
#include "globals.h"
#include "version.h"
#include "wp.h"

#include "WaveformFeeder.h"

#define ERR_MISSMSG      0     // Message missed in transport ring
#define ERR_TOOBIG       1     // Retrieved msg too large for buffer
#define ERR_NOTRACK      2     // Msg retrieved; tracking limit exceeded

#define MAXLOGO 10
#define BUF_SIZE 60000         // Maximum size of a tracebuf message

// Earthworm include files
namespace earthworm {

extern "C" {
#include <earthworm.h>
#include <kom.h>
#include <transport.h>
#include <trace_buf.h>
#include <swap.h>
};

}

using namespace earthworm;

class PacketInfo{
 public:
  Channel ch;
  TRACE2_HEADER trh;
  PacketBuffer pbuf;
};
typedef LockedQueue< PacketInfo > PacketQueue;

class ewfeeder : public WaveformFeeder
{
private:

  void r2w_config( char *ewConfig );
  void r2w_lookup( void );
  void r2w_status( unsigned char msgType, short errNum, char *errMsg );
  unsigned long hash(unsigned char *str);
  bool okDataType(TRACE2_HEADER *trh);
  bool readPacketFromFile(char *Buffer, TRACE2_HEADER *trh, string tracebuf_file, FILE *fp, gzFile zfd);
    
  PacketQueue* pktqmap[MAX_THREADS];


  typedef long waveform_data_type;
  pthread_t thread_array[512];
  int total_thread_count;

  static bool is_offline;
  static unsigned long offline_timestamp_offset;
  static bool offline_first_packet;
  static bool record;
  
  // Global variables
  SHM_INFO Region;
  pid_t    myPid;
  MSG_LOGO GetLogo[MAXLOGO];
  short    nLogo;
  
  // Parameters to read from config file
  char     RingName[MAX_RING_STR];
  char     MyModName[MAX_MOD_STR];
  char     ConfigFile[MAX_FILENAME];
  int      LogSwitch;
  time_t   HeartBeatInterval;
  int      debug;
  
  // Parameters to look up with getutil functions
  long RingKey;
  unsigned char InstId;
  unsigned char MyModId;
  unsigned char TypeHeartBeat;
  unsigned char TypeError;
  unsigned char TypeTracebuf2;
  int datatype_error_cnt;

public:
  void start(string config, int orgtime, WPFactory* pFactory) throw(Error);
  void stop();
  static void* process_packet(void* _pq);

}; // class ewfeeder

#endif
