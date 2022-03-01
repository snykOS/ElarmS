#include <math.h>               // Linux needs this before others
#include <pthread.h>
#include <unistd.h>             // sleep
#include "plog/Log.h"           // plog logging library

#include "WaveformFeeder.h"
#ifdef ENABLE_EARTHWORM
#include "ewfeeder.h"
#endif
#ifdef ENABLE_MCAST
#include "mcastfeeder.h"
#endif
#ifdef ENABLE_FLFEED
#include "flfeeder.h"
#endif

const string RCSID_WaveformFeeder_cc = "$Id: WaveformFeeder.cc $";

extern "C" {
static void *workLoad(void *context);
}

bool WaveformFeeder::_rocknroll = true;

void WaveformFeeder::doit() {
  while (isRocknRoll()) {
    try {
	start(_config,_orgtime,_factoryP);
    }
    catch(Error& e) {
        LOGF << "WaveformFeeder workload thread failed: " << e.str() << ". Aborting...";
        exit(1);
    }
    while (isRocknRoll()) {
      sleep(1);
    }
  }
}

static void *workLoad(void *context)
{
    return WaveformFeeder::workload(context);
}

void * WaveformFeeder::workload(void * context) {
  assert(NULL != context);
  WaveformFeeder *w = (WaveformFeeder *)context;
  w->_isRunning = true;
  w->doit();
  w->_isRunning = false;
  return NULL;
}


void WaveformFeeder::startLoop(string config, int orgtime, WPFactory * factoryP) throw(Error)
{
  _config = config;
  _orgtime = orgtime;
  _factoryP = factoryP;
  _isRunning = true;

  setRocknRoll(true);
  pthread_create(&_pthread, NULL, workLoad, this);
}


void WaveformFeeder::stopLoop() throw(Error)
{
  setRocknRoll(false);
  LOGI << "Waiting for WPFactory thread to join...";
  int rtn = pthread_join(_pthread,NULL);
  LOGI << "Done. Return value: " << rtn << std::endl;
  _isRunning = false;
}

WaveformFeeder * WaveformFeederFactory::createInstance(int type)
{
  WaveformFeeder * p = NULL;

  switch(type) {
  case EW_FEEDER:
#ifdef ENABLE_EARTHWORM
    p = new ewfeeder();
#else
    LOGF << "ewfeeder " << type << " not enabled during compilation " << std::endl;
    exit(1);
#endif
    break;
  case MC_FEEDER:
#ifdef ENABLE_MCAST
    p = new mcastfeeder();
#else
    LOGF << "mcfeeder " << type << " not enabled during compilation " << std::endl;
    exit(1);
#endif
#if !defined(ENABLE_EARTHWORM) && !defined(ENABLE_MCAST) && !defined(ENABLE_FLFEED)
    ERROR_NO_FEED_ENABLED.  Define ENABLE_EARTHWORM and_or ENABLE_MCAST!
#endif
    break;
  case FL_FEEDER:
#ifdef ENABLE_FLFEED
    p = new flfeeder();
#else
    LOGF << "flfeeder " << type << " not enabled during compilation" << std::endl;
    exit(1);
#endif
    break;
  default:
    LOGF << "feeder " << type << " unknown" << std::endl;
    exit(1);
  }

  return p;
}

WaveformFeeder * WaveformFeederFactory::destroyInstance(WaveformFeeder * p)
{
    if(p->isRocknRoll()) {
	p->stopLoop();
    }
    
    if (NULL != p) {
	delete p;
    }
    return NULL;
}

WaveformFeeder::Packet_Statistics WaveformFeeder::_packet_statistics;

WaveformFeeder::Packet_Statistics WaveformFeeder::Get_packet_statistics()
{
    return _packet_statistics;
}

string WaveformFeeder::Get_packet_statistics_as_string(const Packet_Statistics old_stats)
{
    long long delta_read = _packet_statistics.read - old_stats.read;
    long long delta_latent = _packet_statistics.latent - old_stats.latent;
    long long delta_dropped = _packet_statistics.dropped - old_stats.dropped;
    long long delta_processed = _packet_statistics.processed - old_stats.processed;
    long long delta_blacklisted = _packet_statistics.blacklisted - old_stats.blacklisted;

    stringstream ss;
    ss << "Packet counts (delta/total)"
        << " read=" << delta_read << "/" << _packet_statistics.read
        << " latent=" << delta_latent << "/" << _packet_statistics.latent
        << " dropped=" << delta_dropped << "/" << _packet_statistics.dropped
        << " processed=" << delta_processed << "/" << _packet_statistics.processed
        << " black=" << delta_blacklisted << "/" << _packet_statistics.blacklisted;
    return ss.str();
}

// end of file: WaveformFeeder.cc
