#ifndef __flfeeder_h
#define __flfeeder_h

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <strings.h>

#include "RetCodes.h"
#include "TimeStamp.h"
#include "Channel.h"
#include "DataChannel.h"
#include "globals.h"
#include "version.h"
#include "wp.h"

#include <libmseed.h>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <pthread.h>
//#include <earthworm.h> // needed for sleep_ew

#include "WaveformFeeder.h"

class flfeeder : public WaveformFeeder
{
private:

    typedef std::map <std::string, off_t> FPMap;
    typedef std::vector<std::string> StrList;
    typedef std::map <std::string, StrList> FlMap;
    typedef std::map <std::string, bool> bMap;
    typedef long waveform_data_type;
    //pid_t    myPid;
  
    // Parameters to read from config file
    char     ConfigFile[MAX_FILENAME];
    int      LogSwitch;

    static unsigned long tsoffset;

    struct thread_data {
        StrList flist;
        string stn;
        FPMap* fpmap;
        bMap* recmap;
        hptime_t mintime;
        hptime_t maxtime;
        TimeStamp lastproct;
        bool rtfeed;
        MSFileParam* msfp;
        bool debug;
        bool tdebug;
    };

public:
    static void* stnloop(void* targs);
    void start(string config, int orgtime, WPFactory* pFactory) throw(Error);
    void stop();

}; // class flfeeder

#endif
