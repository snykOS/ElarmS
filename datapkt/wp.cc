#include <string.h>		// memset
#include <string>
#include "plog/Log.h"   // plog logging library

#include "TimeStamp.h"          // AQMS time stamp class
#include "wp.h"
#include "WPProperties.h"     // getIgnoreFirstPackets

using namespace std;

const string RCSID_wp_cc = "$Id: wp.cc $";

WPProperties* prop = NULL;

WP::WP(Channel Z,Channel E, Channel N){
  chanZ = Z;
  chanE = E;
  chanN = N;

  samplerate = 0;
  firsttimeE = true;
  firsttimeN = true;
  firsttimeZ = true;
  modules_created = false;
  ignore_packets = 0;
  use_raw_packet = false;

  if (prop == NULL)
      prop = WPProperties::getInstance();
  ignore_packets = prop->getIgnoreFirstPackets();
}

WP::~WP(){
    LOGV << "destructor called";
}

string wp_version = WP_VERSION;

string WP::getVersion() {
    return wp_version;
}

void WP::setSampleRate(int samprate){
  samplerate = samprate;
}

int WP::getSampleRate(){
  return samplerate;
}

bool WP::process(Channel ch, TimeStamp starttime, int* samples __attribute__((unused)), int size){

  if(modules_created==false){
    if (createModules()) {
        modules_created = true;
    }
    else {
        return false;
    }
  }

  // skip gap check if ignore packets is negative
  if (ignore_packets < 0) {
      return true;
  }

  if(string(ch.channel) == string(chanZ.channel)){
    checkgap(chanZ,starttime,lasttimeZ,size,firsttimeZ);    
  }
  else if(string(ch.channel) == string(chanN.channel)){
    checkgap(chanN,starttime,lasttimeN,size,firsttimeN);
  }
  else if(string(ch.channel) == string(chanE.channel)){
    checkgap(chanE,starttime,lasttimeE,size,firsttimeE);
  }

  if(ignore_packets > 0) {
      LOGI << ch << " gap detected, ignoring " << ignore_packets << " packets";
      ignore_packets--;
      return false;
  }

  return true;
}

bool WP::process(RawPacket &p) {

  if(modules_created==false){
    if (createModules()) {
        modules_created = true;
    }
    else {
        return false;
    }
  }

  // skip gap check if ignore packets is negative
  if (ignore_packets < 0) {
      return true;
  }

  if(string(p.ch.channel) == string(chanZ.channel)){
    checkgap(chanZ, TimeStamp(UNIX_TIME, p.start_time), lasttimeZ, p.nsamps, firsttimeZ);
  }
  else if(string(p.ch.channel) == string(chanN.channel)){
    checkgap(chanN, TimeStamp(UNIX_TIME, p.start_time), lasttimeN, p.nsamps, firsttimeN);
  }
  else if(string(p.ch.channel) == string(chanE.channel)){
    checkgap(chanE, TimeStamp(UNIX_TIME, p.start_time), lasttimeE, p.nsamps, firsttimeE);
  }
  return true;
}

void WP::reset() {

    if (prop == NULL)
        prop = WPProperties::getInstance();

    ignore_packets = prop->getIgnoreFirstPackets();
}

void WP::checkgap(Channel chan,TimeStamp firstsamptime,TimeStamp& lastsamptime,int nsamps,bool& firsttime){
  float dt = 1.0/samplerate;

  TimeStamp temptime;
  float offset = dt*nsamps;
  struct timeval time_offset = {0,0};
  time_offset.tv_sec = 0;
  time_offset.tv_usec = offset*1000000;
  temptime = firstsamptime + Duration(time_offset); 


  // LOGD <<"Processing "<<chan<<" Start Time:"<<firstsamptime<<" End Time:"<<temptime;


  if(firsttime){
    lastsamptime = temptime;
    firsttime = false;
  }
  else{
    double timediff = (double)(firstsamptime - lastsamptime);
    // LOGD <<"TimeDiff: "<<timediff<<" dt:"<<dt;
    if(timediff > dt){
      //gap
      LOGD <<"GAP "<<chan<<" ThisPacketFirstSampTime: "<<firstsamptime<<" ThisPacketLastSampTime: "<<temptime<<" PrevPacketLastTime: "<<lastsamptime<<" TimeDiff: "<<timediff<<" dt:"<<dt;
      lastsamptime = temptime;
      reset();
      firsttime = true;
    }
    else{
      lastsamptime = temptime;
    }
  }
}

pthread_mutex_t * WPLib::print_lock = NULL;


////////////////////////////////////////////////////////////////////

// ParseTimeStamp: YYYY/MM/DD,HH:MM:SS.ssss or YYYY-MM-DDTHH:MM:SS.ssss

TimeStamp WPLib::ParseTimeStamp(string str) {

    int year, mon, day, hour, min, sec, usec; char dummy;

    if (sscanf(str.c_str(), "%d/%d/%d%c%d:%d:%d.%d", &year, &mon, &day, &dummy, &hour, &min, &sec, &usec) == 8) {
        TimeStamp time(year, mon, day, hour, min, sec, usec * 100);
        return time;
    }
    if (sscanf(str.c_str(), "%d-%d-%d%c%d:%d:%d.%d", &year, &mon, &day, &dummy, &hour, &min, &sec, &usec) == 8) {
        TimeStamp time(year, mon, day, hour, min, sec, usec * 100);
        return time;
    }

#if 1
    TimeStamp badTime;
    return badTime;
#else
    TimeStamp zeroTime(UNIX_TIME, 0.0);
    return zeroTime;
#endif
} // WPLib::ParseTimeStamp


// format time stamp:  see strtime man page
// note: %F means %Y-%m-%d
// and %T means %H:%M:%S
// special formating:
//  %ES which means %S plus fraction of seconds
//  %ET which means %H:%M%S plus fraction of seconds

char* _append(char* cp, const char* str)
{
    while ( (*cp = *str++) ) ++cp;
    return cp;
} // _append 

char* _conv(char* cp, const char* fp, const int val)
{
    char buffer[32];
    sprintf(buffer, fp, val);
    return _append(cp, buffer);
} // _conv

char* _format(char* cp, const char* fp, const EXT_TIME& et)
{
    bool extended = false;
    for ( ; *fp; ++fp) {
        if (*fp == '%') {
            if (*(fp+1) == 'E') {
                fp++; // alternate flag
                extended = true;
            }
            switch (*++fp) {
                case '\0': --fp;
                           break;

                case 'F': cp = _format(cp, "%Y-%m-%d", et);
                          continue;

                case 'Y': cp = _conv(cp, "%04d", et.year);
                          continue;
                case 'm': cp = _conv(cp, "%02d", et.month);
                          continue;
                case 'd': cp = _conv(cp, "%02d", et.day);
                          continue;

                case 'T': cp = _format(cp, extended ? "%H:%M:%ES" : "%H:%M:%S", et);
                          continue;

                case 'H': cp = _conv(cp, "%02d", et.hour);
                          continue;
                case 'M': cp = _conv(cp, "%02d", et.minute);
                          continue;
                case 'S': cp = _conv(cp, "%02d", et.second);
                          if (extended)
                              cp = _conv(cp, ".%04d", int(et.usec / 100));
                          continue;

                case '%':
                default:
                          break;
            } // switch
        }
        *cp++ = *fp;
    } // while parsing format string
    return cp;
} // _format


string WPLib::FormatTimeStamp(const TimeStamp& time, const string format)
{
    // TimeStamp::ts_as_double(UNIX_TIME) returns nepoch not tepoch!
    EXT_TIME et = int_to_ext(nepoch_to_int(time.ts_as_double(UNIX_TIME)));

    char buffer[100]; (void)memset((char*)buffer, '\0', sizeof(buffer));
    char* cp = buffer;

    _format(cp, (char*)format.c_str(), et);
    
    return string(buffer);
} // WPLib.FormatTimeStamp


string WPLib::FormatChannel(const Channel& c)
{
    char location[MAX_CHARS_IN_LOCATION_STRING+1];
    strcpy(location, c.location);
    std::ostringstream os;
    os << c.network << "." << c.station << "." << c.channel << "." << location;
    return os.str();
} // WPLib.FormatChannel


// end of file: wp.cc
