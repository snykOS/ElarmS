/*****************************************************************************

    Copyright ©2017. 
    The Regents of the University of California (Regents). 
    Authors: Berkeley Seismological Laboratory. All Rights Reserved. 
    Permission to use, copy, modify, and distribute this software
    and its documentation for educational, research, and not-for-profit
    purposes, without fee and without a signed licensing agreement, is hereby
    granted, provided that the above copyright notice, this paragraph and the
    following four paragraphs appear in all copies, modifications, and
    distributions. Contact The Office of Technology Licensing, UC Berkeley,
    2150 Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201,
    otl@berkeley.edu, http://ipira.berkeley.edu/industry-info for commercial
    licensing opportunities.

    Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

    Attribution Rights. You must retain, in the Source Code of any Derivative
    Works that You create, all copyright, patent, or trademark notices from
    the Source Code of the Original Work, as well as any notices of licensing
    and any descriptive text identified therein as an "Attribution Notice."
    You must cause the Source Code for any Derivative Works that You create to
    carry a prominent Attribution Notice reasonably calculated to inform
    recipients that You have modified the Original Work.

    IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
    SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
    ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
    REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
    HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
    MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*****************************************************************************/
#include <signal.h>
#include <plog/Log.h>
#include <ExternalMutexConsoleAppender.h>
#include <SeverityFormatter.h>
#include <pthread.h>
#include "ElarmSWP2.h"
#include "DMLib.h"
#include "WaveformFeeder.h"
#include "WPProperties.h"
#include "GetProp.h"

using namespace eew;

string program_version = "3.1.3b-2019-04-04";
string program_instance = "";

std::stringstream ElarmSWP2::param_str;

static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
static ExternalMutexConsoleAppender<SeverityFormatter> mtx_console_appender(&print_lock);

static int stop_signal = 0;

extern "C" {
    void sig_handler_wrapper (int i) {
	(void)i;
	stop_signal = 1;
    }
}

void usage() {
    LOG_INFO << "epicWP:  Version " << program_version;
    LOG_INFO << "usage: epicWP config_file [Replay=FILE] [ChannelFile=CHANNEL_FILE] [params=CONFIG_FILE]";
    LOG_INFO << "       FILE is the name of TRACEBUF2 file.";
}

int main(int argc, char* argv[])
{
    string params, channel_file;
    string amq_uri, amq_user, amq_passwd;
    ElarmSWP2 *ewp2;
    EWP2ReaderFactory *readerfactoryP;
    WPProperties *wp_prop = WPProperties::getInstance();
    GetProp *prop = GetProp::getInstance();

    WPLib::setPrintLock(&print_lock); // initialize WPLib printlock with the mutex instance
    DMLib::setPrintLock(&print_lock); // initialize DMLib printlock with the mutex instance
    plog::init(plog::verbose, &mtx_console_appender);

    for(int i = 1; i < argc; i++) {
	if(!strstr(argv[i], "=")) {
	    params.assign(argv[i]);
	    break;
	}
	else if(!strncmp(argv[i], "params=", 7)) {
	    params.assign(argv[i]+7);
	}
    }
    if(params.length() == 0) {
	usage();
	return 0;
    }
    prop->init(params, argc, argv);
    try {
	string wpconfig = prop->getString("WPConfig");
	wp_prop->init(wpconfig, argc, argv);
    }
    catch (Error e) {
	LOG_INFO << "Parameter WPConfig not found in " << params;
	return 0;
    }

    char name[1000];
    string host = "-";
    if(gethostname(name, (int)sizeof(name)-1) == 0) {
	// if eew-bk-prod1.geo.berkeley.edu, shorten to eew-bk-prod1
	char *c = strstr(name, ".");
	if(c != NULL) *c = '\0';
	host = string(name);
	program_instance = string(argv[0]) + "@" + host;
    }

    // initialize the signal handler

    signal(SIGHUP,sig_handler_wrapper);
    signal(SIGTERM,sig_handler_wrapper);
    signal(SIGINT,sig_handler_wrapper);
    signal(SIGQUIT,sig_handler_wrapper);

    try {
	amq_uri = prop->getString("TriggerURI");
	amq_user = prop->getString("AmqUser");
	amq_passwd = prop->getString("AmqPassword");
	ElarmSWP2::param_str << "Start: " << program_version << " EWP2 " << host << " " << EWP2Sender::timeString() << endl;
	ElarmSWP2::param_str << "P: ElarmSWP2.TriggerURI: " << amq_uri << endl;
	ElarmSWP2::param_str << "P: ElarmSWP2.AmqUser: " << amq_user;
	LOG_INFO << ElarmSWP2::param_str.str();
    }
    catch(Error e) {
	e.print();
	return -1;
    }
    LOG_INFO << "ElarmSWP2 Version " << program_version;
    LOG_INFO << "ElarmSWP2: TriggerURI: " << amq_uri;

//    DMLib::setPrintLock(PrintLock::getPrintLock());
//    WPLib::setPrintLock(PrintLock::getPrintLock());

    activemq::library::ActiveMQCPP::initializeLibrary();
    LOG_INFO << "Connecting to ActiveMQ at " << amq_uri;
    ewp2 = new ElarmSWP2(amq_uri, amq_user, amq_passwd);
    try {
	ewp2->run();
    }
    catch ( Error e ) {
        e.print();
	return -1;
    }
    catch ( CMSException &e ) {
        LOG_FATAL << e.getStackTraceString();
	return -1;
    }

    readerfactoryP = new EWP2ReaderFactory(ewp2->getSender());

    WaveformFeeder *waveformfeederP =
		WaveformFeederFactory::createInstance(WaveformFeederFactory::EW_FEEDER);
  
    try {
	waveformfeederP->startLoop(params, 0, readerfactoryP);
    }
    catch(string &e) {
	LOG_INFO << "WaveformFeeder.startLoop failed.";
	LOG_INFO << e;
	return -1;
    }

    TimeStamp current_time = TimeStamp::current_time();
    int initial_delay = 60 - current_time.second_of_minute();

    //const Duration info_interval(60 * 10);  // report program info every 10 minutes
    const Duration stats_interval(60);      // report stats every minute

    LOGI << std::endl << std::endl 
        << "Starting reporting loop in " << initial_delay << " seconds"
        //<< ", program info every " << (long)info_interval
        << ", stats every " << (long)stats_interval
        << std::endl << std::endl;  
    sleep(initial_delay);

    //TimeStamp last_info_time  = TimeStamp::current_time() - info_interval - 1;
    TimeStamp last_stats_time = TimeStamp::current_time() - stats_interval - 1;

    WaveformFeeder::Packet_Statistics old_packet_statistics = {0, 0, 0, 0, 0};

    while( !stop_signal && waveformfeederP->isRunning() ) {
        current_time = TimeStamp::current_time();
        //if (current_time - last_info_time >= info_interval) {
        //    LOGI << getProgramInfo() << std::endl;
        //    last_info_time = current_time;
        //}
        if (current_time - last_stats_time >= stats_interval) {
            LOGI << std::endl << __FILE__ << "|SOH: "
                 << WaveformFeeder::Get_packet_statistics_as_string(old_packet_statistics) 
                 << std::endl;
            old_packet_statistics = WaveformFeeder::Get_packet_statistics();
            last_stats_time = current_time;
        }
	    sleep(1);
    }

    if(stop_signal) LOG_INFO << "Received signal. Shutting Down ElarmSWP2 ...";
    else LOG_INFO << "WaveformFeeder has stopped. Shutting Down ElarmSWP2 ...";

    if(ewp2->getSender()->connectedToAmq()) {
	ewp2->stop();
	delete ewp2;
    }

    activemq::library::ActiveMQCPP::shutdownLibrary();

    return 0;
}

void ElarmSWP2::run()
{
    sender = new EWP2Sender(uri, user, passwd);
    sender->start();
}
