
/***********************************************************

File Name :
	flfeeder.cc

Programmer:
	Jen Andrews. Caltech

Description:
	This reads files from a directory 
	and passes on for further EEW calculations

Creation Date:
    Nov, 2014

Modification History:
    1.0 Base code on ewfeeder.cc

Usage Notes:

**********************************************************/

#include "WPProperties.h"
#include "plog/Log.h"       // plog logging library
#include "flfeeder.h"

// vars needed if want to assess latency in offline mode wrt computer time
unsigned long flfeeder::tsoffset;

const string RCSID_flfeeder_cc = "$Id: flfeeder.cc $";

extern "C" {
static void* stnloop_wrapper(void* data)
{
    return flfeeder::stnloop(data);
} // stnloop_wrapper
} // extern "C"


void flfeeder::start(string config, int orgtime __attribute__((unused)), WPFactory* pFactory) throw(Error)
{
    orgtime = 0;
    // read in config parameters
    WPProperties* prop;
    try {
        prop = WPProperties::getInstance();
        prop->init(config);
    }
    catch(Error& e) {
        e.print();
        throw e;
    }

    bool debug = prop->getFLdebug();

    // initialization of data channels
    /**
    try {
        DataChannel::setChannelFlags(prop->getChannelGreyList(),GREY);
        DataChannel::setChannelFlags(prop->getChannelBlackList(),BLACK);
    }
    catch(int& e) {
        LOGE <<"FLFeeder ERROR: Unable to read channel list files"<<endl;
        throw e;
    }
    **/
  
    // check for optional channel filters
    string pattern = prop->getChannelFilter();
    if (pattern.length() > 0)
        DataChannel::SetChannelFilter(pattern);

    // populate channels
    if (!DataChannel::readChannelData(pFactory)) {
        LOGF << "FLFeeder ERROR: Error reading channel metadata.  Aborting!" << endl;
        exit(2);
    }
    
    // read in list of data files to work on
    string flfile = prop->getFLfile();
    if (flfile.size() == 0) {
        LOGF << "FLFeeder ERROR: Invalid file list" << endl;
        exit(2);
    }
    ifstream fp (flfile.c_str());

    StrList flist;
    string tmp;
    while(getline(fp,tmp))
        flist.push_back(tmp);
    fp.close();

    // declare some stuff
    hptime_t mintime = 0;
    hptime_t maxtime = 0;
    bool bFirst = true;

    StrList stnlist;
    FlMap stnmap;
    FPMap fpmap;
    bMap recmap;
    FPMap::iterator it;
    string tag, longtag;
    MSTraceGroup* mstg = NULL;

    // loop over all files to find out: min/max times available; unique stn/chan
    for (size_t i = 0; i < flist.size(); i++) {
       mstg = mst_initgroup(mstg);
       if (ms_readtraces (&mstg, (char*)flist[i].c_str(), 0, -1.0, -1.0, 0, 1, 0, 0) != MS_NOERROR) {
           LOGE << "FLFeeder : Error reading file " << flist[i].c_str() << endl;
           continue;
        }

        if (bFirst) {
            mintime = mstg->traces->starttime;
            maxtime = mstg->traces->endtime;
            bFirst = false;
        }
        else {
            if (mstg->traces->starttime < mintime)
                mintime = mstg->traces->starttime;
            if (mstg->traces->endtime > maxtime)
                maxtime = mstg->traces->endtime;
        }

        tag = (string)mstg->traces->network + "." + (string)mstg->traces->station + "." + (string)mstg->traces->channel;
        //LOGD << "FLFeeder : Location code " << (string)mstg->traces->location << " " << flist[i] << endl;
        if (std::find(stnlist.begin(), stnlist.end(), tag.substr(0,tag.size()-1)) == stnlist.end()) {
            stnmap[tag.substr(0,tag.size()-1)] = StrList();
            stnlist.push_back(tag.substr(0,tag.size()-1));
        }
        stnmap[tag.substr(0,tag.size()-1)].push_back(flist[i]);
        longtag = tag + "." + (string)mstg->traces->location + ".";
        it = fpmap.lower_bound(longtag);
        if (!(it != fpmap.end() && !(fpmap.key_comp()(longtag,it->first)))) {
            fpmap.insert(std::pair<string, off_t>(longtag,0));
            recmap.insert(std::pair<string, bool>(longtag,false));
        }

        mst_freegroup (&mstg);
    }

    LOGI << "FLFeeder: " << flist.size() << " data files and " << stnlist.size() << " unique stations/channels" << endl;
    LOGI << "unique number in fpmap " << fpmap.size() << endl;
    
    // declare more variables 
    int dt = prop->getFLpktlen(); // set appropriately for fixed buffer lengths!
    if (dt == 0) {
        LOGF << "FLFeeder ERROR: packet length set incorrectly!" << endl;
        exit(2);
    }

    // set mintime to nearest second, as only correct subsample offset
    // additionally
    mintime = (long)floor((double)mintime/HPTMODULUS);
    mintime *= HPTMODULUS;
    maxtime = (long)floor((double)maxtime/HPTMODULUS) + 1;
    maxtime *= HPTMODULUS;

    struct timeval curtime;
    gettimeofday(&curtime,NULL);	  
	tsoffset = curtime.tv_sec - (mintime/HPTMODULUS);

    size_t nstep = (maxtime-mintime)/HPTMODULUS/dt;
    long long int tstep = dt * HPTMODULUS;
    pthread_t* thread;
    pthread_attr_t attr;
    void* thrstatus;
    int result;
    thread_data* tmpa;

    LOGD << "FLFeeder: " << mintime << " " << maxtime << " " << nstep  << endl;
 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    size_t max_threads = 100;
    max_threads = (stnlist.size() < max_threads) ? stnlist.size() : max_threads;

    try {
        thread = new pthread_t [max_threads];
    }
    catch (exception& e) {
        LOGF << "FLFeeder ERROR: Cannot allocate thread memory for stations!" << endl;
        exit(2);
    }

    try {
        tmpa = new thread_data [max_threads];
    }
    catch (exception& e) {
        LOGF << "FLFeeder ERROR: Cannot allocate thread memory for stations!" << endl;
        exit(2);
    }

   
    // MAIN PROCESSING LOOP over all time available in data files
    // not considered multiple days.. 

    // create threads
    size_t niter = (stnlist.size()/max_threads);
    if (stnlist.size()%max_threads > 0) {
        niter += 1;
    }
    LOGI << "FLFeeder: for " << stnlist.size() << " stations need " << niter << " iterations with " << max_threads << " threads" << endl;

    TimeStamp lastproct = TimeStamp::current_time();
	for (size_t ii=0; ii<=nstep; ii++ ) {
        maxtime = mintime + tstep;
        
        for (size_t j=0; j<niter; j++) {
            for (size_t i=0; i<max_threads; i++) {
                if (max_threads*j + i == stnlist.size()) {
                    break;
                }

                tmpa[i].stn = stnlist[max_threads*j + i];
                tmpa[i].flist.clear();
                for (size_t k=0; k<stnmap[tmpa[i].stn].size(); k++) {
                    tmpa[i].flist.push_back(stnmap[tmpa[i].stn][k]); // iteration over map list and addition of file names
                }
                tmpa[i].fpmap = &fpmap; // only safe because no time-step should overlap (i.e. station unique) ! Otherwise consider mutex...
                tmpa[i].mintime = mintime;
                tmpa[i].maxtime = maxtime;
                tmpa[i].lastproct = lastproct;
                tmpa[i].rtfeed = prop->getFLRTfeed();
                tmpa[i].msfp = NULL;
                tmpa[i].debug = debug;
                if (ii%(3600/dt) == 0) {
                    tmpa[i].tdebug = true;
                }
                else {
                    tmpa[i].tdebug = false;
                }
                tmpa[i].recmap = &recmap; // as above

                result = pthread_create(&thread[i], &attr, stnloop_wrapper, static_cast<thread_data*>(&tmpa[i]));
            }

            // wait for threads and destroy
            for (size_t i=0; i<max_threads; i++) {
                if (max_threads*j + i == stnlist.size()) {
                    break;
                }
                result = pthread_join(thread[i], &thrstatus);
                if (result) {
                    LOGE << "FLFeeder ERROR: return code from thread in flfeeder!" << endl;
                }
            }
        }

        if (prop->getFLRTfeed()) {
            //LOGD << "FLFeeder: running as if RT " << TimeStamp::current_time() << endl;
            if (double(TimeStamp::current_time() - lastproct) > dt) {
                LOGD << "FLFeeder: flfeeder is running slower than real-time!" << endl;
                LOGD << "FLFeeder: " << TimeStamp::current_time() << " " << lastproct << " " << double(TimeStamp::current_time()-lastproct) << " " << tstep << endl;
            }

            while (double(TimeStamp::current_time() - lastproct) < dt) {
                //LOGD << "FLFeeder: sleeping " << TimeStamp::current_time() << endl;
                //sleep_ew(10); // 0.01s 
                usleep(10000); // 0.01s
            }
        }
        mintime += tstep;
        lastproct += dt;
	}

    LOGI << "FLFeeder: Completed file feed run!" << endl;

    // Carry out trial waveform writing //
    if (debug) {
        LOGD << "FLFeeder ending data recording!" << endl;
        for (it = fpmap.begin(); it != fpmap.end(); it++) {
            char split_char = '.';
            std::istringstream split(it->first);
            std::vector<std::string> tokens;
            for (std::string each; std::getline(split, each, split_char); tokens.push_back(each));
            if (tokens[2].find("Z") == std::string::npos) continue;
            Channel chn = Channel(tokens[0].c_str(), tokens[1].c_str(), tokens[3].c_str(), tokens[2].c_str());
            try {
                WP* eew = DataChannel::getWPHandle(chn);
                if (eew) {
                    eew->stoprecord();
                }
            }
            catch (Error& e) {
                e.print();
                continue;
            }
        }
    }

    pthread_attr_destroy(&attr);
    delete [] thread;
    delete [] tmpa;

    stopLoop();    
} // flfeeder::start

void* flfeeder::stnloop(void* threadarg)
{
    // unpack passed arguments
    struct thread_data *targs;
    targs = (thread_data *) threadarg;
    string stntag = targs->stn;
    StrList flist = targs->flist;
    FPMap* fpmap = targs->fpmap;
    hptime_t mintime = targs->mintime;
    hptime_t maxtime = targs->maxtime;
    TimeStamp lastproct = targs->lastproct;
    bool rtfeed = targs->rtfeed;
    MSFileParam *msfp = targs->msfp;
    bool debug = targs->debug;
    bool tdebug = targs->tdebug;
    bMap* recmap = targs->recmap;
 
    // make the stn tag finding in filename more general
    char split_char = '.';
    std::istringstream split(stntag);
    std::vector<std::string> tokens;
    for (std::string each; std::getline(split, each, split_char); tokens.push_back(each));
    
    // create additional temporary variables
    MSRecord *msr = 0;
    string tag = "";
    hptime_t endtime = 0, tmptime = 0;
    int spos=0; //, epos=0;    
    int valid = 1, offset = 0, len = 0, valdata = 0, nsamp = 0;
    int* buf = NULL;
    string stn = "", net = "", chan = "", loc = "";
    double samprate = 0.0;
    off_t fpos = 0;
    double uscorr = 0.0;
    timeval tfs = {0,0};

    // loop over files and find station/channel matches (expect 3)
    for (size_t j=0; j<flist.size(); j++) {
        //if (flist[j].find(tokens[0]) != std::string::npos && flist[j].find(tokens[1]) != std::string::npos && flist[j].find(tokens[2]) != std::string::npos) {

        msfp = NULL;
        spos=0;
        // epos=0;
        endtime = 0;
        valid = 1;
        offset = 0;
        len = 0;
        valdata = 0;
        nsamp = 0;
        buf = NULL;
        stn = "";
        net = "";
        chan = "";
        loc = "";
        samprate = 0.0;
        fpos = 0;
        uscorr = 0.0;

        // read single packet to discover tag information
        ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, NULL, NULL, 1, 0, 0);
        tag = (string)msr->network + "." + (string)msr->station + "." + (string)msr->channel + "." + (string)msr->location + ".";
        ms_readmsr_r (&msfp, &msr, NULL, 0, NULL, NULL, 0, 0, 0);
        int ofpos = fpmap->operator[](tag);
        bool rec = recmap->operator[](tag);

        // dataless read and discard of packets earlier than needed
        while (fpos < ofpos) {
            if (ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 0, 0) != MS_NOERROR) {
                LOGE << "FLFeeder ERROR: Error reading mseed! " << flist[j] << " with error " << ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 0, 0) << endl;
                break;
            }
        }

        // read with data
        while (ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 1, 0) == MS_NOERROR) {
            if (valid) {
                nsamp = int(msr->samprate*(maxtime-mintime)/HPTMODULUS);
                if (nsamp > 2000) 
                {
                    LOGF << "FLFeeder ERROR: buffer length exceeds fixed vars in Onsite!" << endl;
                    return NULL;
                }
                stn = msr->station;
                net = msr->network;
                chan = msr->channel;
                loc = msr->location;
                samprate = msr->samprate;
                valid = 0;

                try {
                    buf = new int [nsamp];
                }
                catch (exception& e)
                {
                    LOGF << "FLFeeder ERROR: unable to allocate memory for record reading!" << endl;
                    return NULL;
                }
                int* idata = (int*)msr->datasamples;
                std::fill_n(buf, nsamp, idata[0]);

                // correct sub-sample time offset
                uscorr = (fmod(msr->starttime,HPTMODULUS/msr->samprate) - fmod(mintime,HPTMODULUS/msr->samprate))*msr->samprate/HPTMODULUS;
                uscorr = (fabs(uscorr) > 0.5) ? fabs(uscorr)-1.0: fabs(uscorr);
                uscorr /= msr->samprate;
            }
            // check for integer data - needs to be generalised
            if (msr->sampletype != 'i') {
                LOGE << "FLFeeder ERROR: Only works with integer data encoding from mseed file!" << endl;
                if (ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 1, 0) != MS_NOERROR) {
                    LOGE << "FLFeeder ERROR: Error reading mseed! " << flist[j] << " with error " << ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 0, 0) << endl;
                    break;
                }
                continue; //Perhaps should be a hard break here, but have to deal with memory cleanup...
            }

            endtime = msr->starttime + (msr->samplecnt * (HPTMODULUS/msr->samprate));
            tmptime = maxtime + (0 * HPTMODULUS);
            
            if (msr->starttime > maxtime) {
            // trying a buffer around break statement in case slight time
            // mis-ordering of packets
                if (msr->starttime > tmptime) {
                    break;
                }
                else {
                    if (ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 1, 0) != MS_NOERROR) {
                        LOGE << "FLFeeder ERROR: Error reading mseed! " << flist[j] << " with error " << ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 0, 0) << endl;
                        break;
                    }
                    continue;
                }
            }
            else if (endtime < mintime) {
                fpmap->operator[](tag) = fpos; // update file position for next read
                //if (ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 1, 0) != MS_NOERROR) {
                //    LOGE << "FLFeeder ERROR: Error reading mseed! " << flist[j] << " with error " << ms_readmsr_r (&msfp, &msr, (char*)flist[j].c_str(), -1, &fpos, NULL, 1, 0, 0) << endl;
                //    break;
                //}
                continue;
            }

            // select data to be passed to EEW 
            valdata = 1;
            //spos = (int) round((msr->starttime - mintime) * (msr->samprate / (double)HPTMODULUS));
            spos = (int) floor((msr->starttime - mintime) * (msr->samprate / (double)HPTMODULUS));
            offset = 0;
            len = msr->samplecnt;

            if (msr->starttime < mintime) {
                spos = 0;
                offset = (int) round((mintime - msr->starttime) * (msr->samprate / (double)HPTMODULUS));
                len -= offset;
            }

            if (endtime > maxtime) {
                len -= (int) round((endtime - maxtime) * (msr->samprate / (double)HPTMODULUS));
            }

            if ((len == ((maxtime-mintime)*(msr->samprate/(double)HPTMODULUS))-1 || (spos + len) == ((maxtime-mintime)*(msr->samprate/(double)HPTMODULUS))-1) && offset+len < msr->samplecnt) {
                len += 1;
            }

            int* idata = (int*)msr->datasamples;
            memcpy(&buf[spos],&idata[offset],sizeof(int)*(len));

        }
        // clean up file reading memory and invalid loops 

        ms_readmsr_r (&msfp, &msr, NULL, 0, NULL, NULL, 0, 0, 0);
        if (! valdata) {
            if (! valid) {
                delete [] buf;
            }

            continue;
        }

        // pass data to EEW for processing
        Channel chn = Channel(net.c_str(),stn.c_str(),loc.c_str(),chan.c_str());
        try {
            WP* eew = DataChannel::getWPHandle(chn);
            if (eew) {
                if (false)
                {
                    // variables/method to calculate latency wrt computer time
                    tfs.tv_sec = (long) (mintime/HPTMODULUS) + tsoffset;
                    tfs.tv_usec = (long) 1000000*(mintime%HPTMODULUS)/HPTMODULUS;
                }
                else
                {
                    tfs.tv_sec = (long) (mintime/HPTMODULUS);
                    if (uscorr < 0) {
                        tfs.tv_sec -= 1;
                        tfs.tv_usec = (long) ((1.0 + uscorr) * 1000000);
                    }
                    else {
                        tfs.tv_usec = (long) (uscorr * 1000000);
                    }
                }

                eew->setSampleRate(samprate); 
                if (rtfeed) {
                    eew->process(chn,lastproct,buf,nsamp);
                }
                else {
                    eew->process(chn,TimeStamp(UNIX_TIME,tfs),buf,nsamp);
                }
                
                if (chan.find("Z") == std::string::npos) continue;
                if (debug && (tdebug || !rec)) {
                    try {
                        eew->stoprecord();
                        eew->record(3600);
                        recmap->operator[](tag) = true;
                    }
                    catch (Error& e) {
                        e.print();
                    }
                }

            }
        }
        catch (Error& e) {
            e.print();
        }

        if (! valid) {
            delete [] buf;
        }
        //}
    }

    //pthread_exit(NULL);
    return NULL;
}


