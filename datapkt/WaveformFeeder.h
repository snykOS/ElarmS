#ifndef WaveformFeeder_h__
#define WaveformFeeder_h__

#include <assert.h>
#include <pthread.h>

#include "Exceptions.h"
#include "wp.h"

using namespace std;

#define RCSID_WaveformFeeder_h "$Id: WaveformFeeder.h $"
extern const string RCSID_WaveformFeeder_cc;


class WaveformFeeder
{
 private:
  static bool                 _rocknroll;
  pthread_t                   _pthread;
  pid_t                       _pid;

  string                      _config;
  int                         _orgtime;
  WPFactory *                 _factoryP;
  bool			              _isRunning;

 public:
  virtual ~WaveformFeeder() {}
  static bool isRocknRoll() { return _rocknroll; }

  static void setRocknRoll(bool val) {
_rocknroll = val;
}

  WaveformFeeder() : _isRunning(false) {}

  virtual void start(string config, int orgtime, WPFactory * factoryP) throw(Error) = 0;

  void doit();

  static void * workload(void * context);

  void startLoop(string config, int orgtime, WPFactory * factoryP) throw(Error);

  void stopLoop() throw(Error);

  bool isRunning() { return _isRunning; }
 
  // 2017-01-20 CAF -- may need to use std::atomic once it is available on all machines
  typedef struct packet_statistics {
      long long read;
      long long latent;
      long long dropped;
      long long blacklisted;
      long long processed;
  } Packet_Statistics;

  static Packet_Statistics Get_packet_statistics();
  static string Get_packet_statistics_as_string(const Packet_Statistics old_stats);

  protected:
  static Packet_Statistics _packet_statistics;
};

class WaveformFeederFactory
{
 public:
  static const int EW_FEEDER = 1;
  static const int MC_FEEDER = 2;
  static const int FL_FEEDER = 3;

  WaveformFeederFactory() {
  }

  static WaveformFeeder * createInstance(int type);

  static WaveformFeeder * destroyInstance(WaveformFeeder * p);
};

#endif
