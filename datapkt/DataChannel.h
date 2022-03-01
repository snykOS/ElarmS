#ifndef __datachannel_h
#define __datachannel_h

#include "Channel.h"
#include "Exceptions.h"
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <stdio.h>
#include <pthread.h>
#include <regex.h>              // regular expression for channel filter
#include "globals.h"
#include "ChannelConfig.h"
#include "wp.h"
#include "RetCodes.h" // TN_ notifications

#define RCSID_DataChannel_h "$Id$"
extern const string RCSID_DataChannel_cc;

const int MAX_SAMPLES_IN_PACKET = 1024;
const float FLOAT_NOTSET = -9999;
const float NOT_CLIPPED = -1;

typedef map<string,WP*> WPMap;
typedef list<Channel> ChannelList;
typedef map<string,ChannelFlag>  ChannelMap;
typedef map< string,TimeStamp > ChannelTimedMap;
typedef vector<WP*> WPVector;
typedef std::map<Channel, ChannelConfig, std::less<Channel>, std::allocator<std::pair<const Channel, ChannelConfig> > > ChannelConfigList;

class DataChannel : public Channel{

 private:
   static ChannelMap chmap;
   static ChannelTimedMap chtmap;
   static ChannelTimedMap chbookmarks;

   static WPMap eewmap;
   static ChannelConfigList chanlist;
   static map<string,float> chanthresmap;
   static map<string,float> chanazimuthmap;
   static map<string,float> chandipmap;
   static TimeStamp whitelist_checktime;
   static TimeStamp blacklist_checktime;
   static TimeStamp greylist_checktime;
   static bool whitelist_firstreadflag;
   static bool blacklist_firstreadflag;
   static bool greylist_firstreadflag;
   static string channel_file_signature;    // file signature for channel file.
   static string greylist_file_signature;    // file signature for grey list file.
   static string blacklist_file_signature;    // file signature for black list file.
   

   static pthread_mutex_t lock;
   static regex_t* pChannel_regex;

    static void getAllZ(ChannelConfigList &cl,ChannelList& clres);
    //static Channel findChannel(ChannelConfigList &cl,Channel chan);
    static bool findChannel(ChannelConfigList &cl,Channel chansearch, Channel &chanmatch);
    static void GetChannels(string channel_file, ChannelConfigList &cl) throw(Error);
    static bool isStationOff(string net,string sta);
    static bool getColumns(char *line, vector<string> &columns) throw(Error);
    static bool readLine(char *line, vector<string> &columns, map<string, string> &values);     
    static int addChannelsToWPMap(ChannelConfigList channelList, Channel chanZ, WPFactory* pFactory);
    static int removeChannelsFromWPMap(Channel chanZ);

 public:
    static int origin_time;
    static WPVector eewRec;

    static void setChannelFlags(string filename,ChannelFlag type) throw(int);
    static void SetChannelFilter(const string pattern);
    static bool readChannelData(WPFactory* pFactory);
    static int readChannels(WPFactory* pFactory, string channelfile = "");
    static void writePacket(const char*) throw(Error);
    static void writePacket_raw(const char* buf) throw(Error);
    static WP* getWPHandle(Channel ch);
    static WP* getWPHandle(string sncl);
    static ChannelFlag getChannelFlag(Channel ch);
    static float getClipping(Channel ch);
    static bool getClipping(Channel ch, float& clip);
    static bool getAzimuth(Channel ch, float& azimuth);
    static bool getDip(Channel ch, float& dip);
    static void getWPList(vector<WP*>& eewlist);
    static void turnStationOff(string net,string sta);
    static void turnStationOn(string net,string sta);

    static bool isDataPacket(DATA_HDR *hdr);
    //friend std::ostream& operator<<(std::ostream& os,WDAChannel& c);
    static string GetFileSignature(string filename);
    static string GetFileSignature(const ChannelFlag flag);
    
    DataChannel();
    ~DataChannel();

#ifdef UTDC
    static bool readChannelsUT(const string filename);
#endif // UTDC
};


#endif
