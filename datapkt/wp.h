#ifndef __wp_h
#define __wp_h

#include <sstream>      // for ostringstream
#include <pthread.h>
#include "RawPacket.h"
#include "TimeStamp.h"
#include "Duration.h"
#include "Channel.h"
#include "version.h"

using namespace std;

#define RCSID_wp_h "$Id: wp.h $"
extern const string RCSID_wp_cc;

class WP {
  
 protected:
  Channel chanZ,chanE,chanN;
  TimeStamp lasttimeE,lasttimeN,lasttimeZ;
  bool firsttimeE,firsttimeN,firsttimeZ;
  bool modules_created;
  int samplerate;
  int ignore_packets;
  bool use_raw_packet;

  void checkgap(Channel chan,TimeStamp firstsamptime,TimeStamp& lastsamptime,int nsamps,bool& firsttime);

 public:
  WP(Channel Z, Channel E, Channel N);
  virtual ~WP();

  void setSampleRate(int);
  int getSampleRate();

  //create instances of modules in child class
  virtual bool createModules() = 0;
  
  //called on each packet arrival if use_raw_packet = false
  virtual bool process(Channel ch, TimeStamp starttime, int* samples, int size);

  //called on each packet arrival if use_raw_packet = true
  virtual bool process(RawPacket &raw);
  
  //called if gaps found 
  virtual void reset() = 0;    

  virtual void record(int duration) = 0;
  
  virtual void stoprecord() = 0;

  bool useRawPacket() { return use_raw_packet; }

  static string getVersion();

};


class WPFactory {
 public:
  WPFactory() {
  }
  virtual ~WPFactory() {
  }
  virtual WP * createInstance(Channel Z, Channel E, Channel N) = 0;
//  virtual WP * createInstance(Channel Z, Channel E, Channel N) {
//    std::LOGD <<"WPFactory.createInstance(): XXX "<<__FILE__<<" XXX "<<__LINE__<<" XXX"<<std::endl;
//    return NULL;
//  }
};

class WPLib
{
 private:
    static pthread_mutex_t *print_lock;

 public:
    static void setPrintLock(pthread_mutex_t *printLock) { print_lock = printLock; }
    static void printLock() { if(print_lock != NULL) pthread_mutex_lock(print_lock); }
    static void printUnlock() { if(print_lock != NULL) pthread_mutex_unlock(print_lock); }

    // additional utilities

    // parseTimeStamp: YYYY/MM/DD,HH:MM:SS.ssss or YYYY-MM-DDTHH:MM:SS.ssss
    TimeStamp static ParseTimeStamp(string str);

    // format time stamp:  see strtime man page
    // note: %F means %Y-%m-%d
    // and %T means %H:%M:%S
    // special formating:
    //  %ES which means %S plus fraction of seconds
    //  %ET which means %H:%M%S plus fraction of seconds
    string static FormatTimeStamp(const TimeStamp& time, const string format = "%F,%ET");

    // format Channel for output using dotted notation
    string static FormatChannel(const Channel& c);

};

#endif
