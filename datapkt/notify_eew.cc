/*
* Copyright (c) 2014 California Institute of Technology.
* All rights reserved.
* This program is distributed WITHOUT ANY WARRANTY whatsoever.
* Do not redistribute this program without written permission.
*/

// Version history
// 01 KS  11/12/2009 -- Original version
// 02 CAF 01/14/2013 -- Minor cleanup and refactor
// 03 CAF 04/02/2014 -- Don't delete results from message.next() per notes in aqms/lib/amq/Message.h
//                      otherwise coredump on linux.

#include <iostream>         // std::cout
#include <string>           // std::string

#include "plog/Log.h"       // plog logging library
#ifdef UTNE
#include "ExternalMutexConsoleAppender.h"
#include "SeverityFormatter.h"
#endif // UTNE
#ifndef FILE_FUNC
#define FILE_FUNC __FILE__ << ":" << __FUNCTION__
#endif

#include "notify_eew.h"
#include "DataChannel.h"
#include "Connection.h"
#include "WPProperties.h"

const string RCSID_notify_eew_cc = "$Id: notify_eew.cc $";

// forward declare
pthread_t notify_eew_thread;
extern "C" {
void* notify_eew_listener(void*);
} // extern "C"
void handleNotification(Message &m,void *arg);

void start_notify_eew_listener_thread()
{
    if(pthread_create(&notify_eew_thread,NULL,notify_eew_listener,NULL)<0){
        LOGF << "ERROR Creating notify_eew_thread" << std::endl;
        throw Error();
    }
} // start_notify_eew_listener_thread


// timeout and retry delay
int timeout = 60;

void* notify_eew_listener(void* arg __attribute__((unused)) ) {
    char progname[128];
    int pid = getpid();
    sprintf(progname,"EEW_WP_%d",pid);

    LOGD << FILE_FUNC << ": started" << std::endl;

    Connection* pConn = NULL;

    while (true) {

        LOGD << FILE_FUNC << ": starting up CMS connection using "
            << WPProperties::getInstance()->getCMSConfigFile().c_str();

        if (pConn != NULL) delete pConn;
        pConn = new Connection(WPProperties::getInstance()->getCMSConfigFile().c_str(),progname);  
        if (!pConn) { // || !(*pConn)) {
            LOGF << "ERROR Not able to create notify_eew connection.  Aborting!";
            exit(1);
        }

        string notify_eewchan = WPProperties::getInstance()->getEEWChannel();
        int status = TN_SUCCESS;

        while (status == TN_SUCCESS && pConn != NULL) {

            // LOGD << FILE_FUNC << ": Trying to subscribe to notify_eew topic eew." << notify_eewchan;
            if ((status = pConn->subscribe((char*)notify_eewchan.c_str())) != TN_SUCCESS) {
                LOGE << FILE_FUNC << ": Failed to subscribe to notify_eew topic eew." << notify_eewchan
                    << ", status=" << status << " errno=" << errno;
                break;
            } else {
                LOGD << FILE_FUNC << ": Subscribed to notify_eew topic eew." << notify_eewchan;
            }

            if ((status = pConn->registerHandler(TN_TMT_EVENT_SIG, handleNotification, NULL)) != TN_SUCCESS) {
                LOGE << FILE_FUNC << ": Error registering notify_eew message handler"
                    << ", status=" << status << " errno=" << errno;
                throw Error();
            }

            while (status == TN_SUCCESS && pConn != NULL) {
                LOGD << FILE_FUNC << ": calling conn.listen with " << timeout;
                try {
                    status = pConn->listen(timeout);

                    LOGD << FILE_FUNC << ": pConn listen returned status=" << status << " errno=" << errno;

                    if (!pConn->isAlive()) break;

                } catch (RTException rex) {
                    LOGW << FILE_FUNC << ": caught error from conn.listen. status=" << status << " errno=" << errno;
                    status = TN_FAILURE;
                    break;
                }
            }

        } // inner while loop

        if (pConn != NULL) {
            if (pConn->isAlive()) {
                LOGV << ": Trying to unsubscribe to notify_eew topic eew." << notify_eewchan;
                if ((status = pConn->unsubscribe((char*)notify_eewchan.c_str())) != TN_SUCCESS) {
                    LOGE << ": Failed to unsubscribe to notify_eew topic eew." << notify_eewchan
                        << ", status=" << status << " errno=" << errno;
                }
            }
            // LOGD << FILE_FUNC << ": Deleting pConn";
            delete pConn;
            pConn = NULL;
        }

        LOGD << FILE_FUNC << ": internal loop exiting, sleeping for " << timeout << " seconds before restarting inner loop";
        sleep(timeout);

    } // main while loop

    return NULL;
} // notify_eew_listener

void handleNotification(Message &m,void *arg __attribute__((unused)) ) {

// 03 -- Note, do not delete the following pointers which are maintained by m.next()
//       Coredumps on Linux -- apparently not a problem on Solaris.
  char *command= new char[64];
  char *net = new char[4];
  char *sta = new char[16];

  m.next(&command);

  // LOGD << FILE_FUNC << ": Received notify_eew Message, Command: " << command;

  if(string(command) == "OFF"){

    m.next(&net);
    m.next(&sta);
    
    LOGI << __FILE__ << "|SOH: Turning station " << net << "." << sta << " OFF" << std::endl;
    //add into the list//
    DataChannel::turnStationOff(net,sta);
  }
  else if(string(command) == "ON"){

    m.next(&net);
    m.next(&sta);
    
    LOGI << __FILE__ << "|SOH: Turning station " << net << "." << sta << " ON" << std::endl;
    //add into the list//
    DataChannel::turnStationOn(net,sta);
  }
  else if(string(command) == "RECORD"){
    char *loc = new char[16];
    char *comp = new char[16];
    int dur;

    m.next(&net);
    m.next(&sta);
    m.next(&loc);
    m.next(&comp);
    m.next(dur);
 
    string dot = ".";
    string sncl = string(net)+dot+string(sta)+dot+string(loc)+dot+string(comp)+"Z";

    LOGI << __FILE__ << ": Received COMMAND: RECORD to record " << sncl << std::endl;

    WP* eew = DataChannel::getWPHandle(sncl);
    if(eew){
        LOGV << "Found WP object for " << sncl << " for recording..";
        eew->record(dur);
    }

  }
  else {
    ostringstream ostrm;
    ostrm << m;
    LOGW << FILE_FUNC << ": Unexpected message '" << command << "' : " << ostrm.str() << std::endl;
  }


} // handleNotification

#ifdef UTNE
/** Test code: unit test for notify_eew listener.
 * If one argument reads file for connect and topic info and runs for 1 minute.
 * If two arguments then 2nd argument changes default run time to specified time.
 * @param argc number of command line arguments
 * @param argv char buffer pointer to command line arguments
 */
int main (int argc, char* argv[]) {

    int rc = 0;         // return code - assume OK

    // logging stuff
    static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
    WPLib::setPrintLock(&print_lock);
    static eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> Severity_appender(&print_lock);

    plog::init(plog::verbose, &Severity_appender);

    LOGN << "Program: " << argv[0] << " -- unit test for " << RCSID_notify_eew_cc << std::endl
        << "libwp version " << WP::getVersion() << std::endl;

    if(argc < 2) {
        LOGN << "Usage: " << argv[0] << " [flags] <wp.cfg>";
        LOGN << "    or " << argv[0] << " [flags] <wp.cfg> <duration> [<timeout>]";
        LOGN << " where <duration> is in seconds, use 0 for forever";
        LOGN << "       <timeout> is delay between reconnects, default is " << timeout;
        LOGN << "   and [flags] are as follows:";
        LOGN << "    -V    -- show version info";
        LOGN;
        exit(rc);
    }

    // parse options

    int show_version_info = 0;
    char c;
    while ((c = getopt(argc, argv, "V")) != -1) {
        switch (c) {
            case 'V': show_version_info = 1;
                      break;
            default:
                      rc++;
                      break;
        } // switch
    } // while getopt

    if (show_version_info) {
        LOGN << "Modules:";
        LOGN << "  " << RCSID_Exceptions_h;
        LOGN << "  " << RCSID_Exceptions_cc;
        LOGN << "  " << RCSID_GetProp_h;
        LOGN << "  " << RCSID_GetProp_cc;
        LOGN << "  " << RCSID_WPProperties_h;
        LOGN << "  " << RCSID_WPProperties_cc;
        LOGN << "  " << RCSID_wp_h;
        LOGN << "  " << RCSID_wp_cc;
        LOGN << "  " << RCSID_DataChannel_h;
        LOGN << "  " << RCSID_DataChannel_cc;
        LOGN << "  " << RCSID_notify_eew_h;
        LOGN << "  " << RCSID_notify_eew_cc;
        LOGN;
    }

    if (optind == argc || rc != 0)
        exit(rc);

    LOGD << FILE_FUNC <<": Unit test for " << RCSID_notify_eew_cc << std::endl;

    string filename(argv[optind++]);
    long long duration = 10;
    if (optind < argc) {
        duration = atoi(argv[optind++]); 
        LOGN << FILE_FUNC <<": duration changed to " << duration;
    }
    if (optind < argc) {
        timeout = atoi(argv[optind++]); 
        LOGN << FILE_FUNC <<": timeout changed to " << timeout;
    }

    try {
        LOGD << FILE_FUNC <<": Calling activemq::library::initializeLibrary()";
        activemq::library::ActiveMQCPP::initializeLibrary();

        LOGD << FILE_FUNC <<": Setting up waveform config file";
        WPProperties* prop;
        prop = WPProperties::getInstance();
        prop->init(filename);

        LOGD << FILE_FUNC <<": Calling start_notify_eew_listener...";
        try {
            start_notify_eew_listener_thread();
        } catch (Error e) {
            LOGF << "Unable to start notify_eew_listener.  Aborting.";
            rc++;
            exit(rc);
        }

        LOGD << FILE_FUNC <<": Waiting " << duration << " seconds (0 means infinite)" << std::endl;
        for (long long i = duration; duration == 0 || i > 0; --i) {
            if (duration == 0) {
                if ((i % 600) == 0) {
                    LOGD << FILE_FUNC <<": Waiting forever, " << -i << " elapsed";
                }
            } else if ( (duration <= 600 && i % 60 == 0) || i % 600 == 0) {
                LOGD << FILE_FUNC <<": Waiting " << i << " out of " << duration << " seconds" << std::endl;
            }
            
            std::string report = DataChannel::GetStats();
            if (report.size() > 0) {
                LOGI << std::endl << report << std::endl;
            }
            sleep(1);
        }
        LOGD << FILE_FUNC <<": Exiting listener loop after " << duration << " seconds" << std::endl;

    } catch(Error& e) {
        LOGE << FILE_FUNC <<": Error while running unit test: " << e.printStr() << std::endl; 
        rc++;
    }

    if (rc == 0) {
        LOGI << FILE_FUNC <<": Finished unit test successfully.";
    } else {
        LOGE << FILE_FUNC <<": Finished unit test with error code=" << rc;
    }
    sleep(1);
    exit(rc);
} // main

#endif // UTNE

// end of file notify_eew.cc
