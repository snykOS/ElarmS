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
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <plog/Log.h>
#include <pthread.h>
#include <ExternalMutexConsoleAppender.h>
#include <SeverityFormatter.h>
#include "E2ModuleManager.h"
#include "E2Event.h"
#include "GetProp.h"

string program_version="3.2.1-2020-04-17";
string program_instance = "";

using namespace cms;
using namespace eew;

static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
static ExternalMutexConsoleAppender<SeverityFormatter> mtx_console_appender(&print_lock);

static bool running = false;
int stop_signal = 0;

extern "C" {
    void sig_handler_wrapper (int i) {
	printf("Received Signal %d\n",i);
	if(!running) exit(0);
	stop_signal = 1;
    }
}   

void usage (const char *progname) {
    LOG_INFO << progname << ":  Version " << program_version;
    LOG_INFO << "usage: " << progname << " config_file [param1=VALUE param2=VALUE ...]";
}

int main(int argc, char* argv[])
{
    bool replay_mode = false;
    string config;

    std::string longprog = string(argv[0]); 
    std::string prog = longprog;
    if (longprog.rfind('/') != std::string::npos) {
        prog = longprog.substr(longprog.rfind('/')+1, std::string::npos);
    }

    plog::init(plog::verbose, &mtx_console_appender);

    for(int i = 1; i < argc; i++) {
	char *arg = argv[i];
	if(arg[0] == '-') {
	    if(!strcmp(arg, "-replay")) {
		replay_mode = true;
	    }
	    else {
		usage(prog.c_str());
		return 0;
	    }
	}
	else if(!strstr(arg, "=")) {
	    config.assign(arg);
	}
    }
    if(config.empty()) {
	usage(prog.c_str());
	return 0;
    }
    GetProp *prop = GetProp::getInstance();
    prop->init(config, argc, argv);

    // initialize the signal handler
    signal(SIGHUP,sig_handler_wrapper);
    signal(SIGINT,sig_handler_wrapper);
    signal(SIGQUIT,sig_handler_wrapper);
/*
    signal(SIGILL,sig_handler_wrapper);
    signal(SIGTRAP,sig_handler_wrapper);
    signal(SIGFPE,sig_handler_wrapper);
    signal(SIGKILL,sig_handler_wrapper);
    signal(SIGBUS,sig_handler_wrapper);
    signal(SIGSEGV,sig_handler_wrapper);
    signal(SIGSYS,sig_handler_wrapper);
    signal(SIGTERM,sig_handler_wrapper);
*/
    LOG_INFO << "M: E2 Version " << program_version << " " << E2Event::nowToString(3);

    char name[1000];
    if(gethostname(name, (int)sizeof(name)-1) == 0) {
	// if bk1-eew.geo.berkeley.edu, shorten to bk1-eew
	char *c = strstr(name, ".");
	if(c != NULL) *c = '\0';
	program_instance = prog + "@" + string(name);
    }
    activemq::library::ActiveMQCPP::initializeLibrary();

    E2ModuleManager *manager;        

    try {
	manager = E2ModuleManager::getInstance();
	manager->init(replay_mode);
	running = true;
	manager->run();
    }
    catch(Error e) {
	e.print();
	exit(1);
    }
    try {
	activemq::library::ActiveMQCPP::shutdownLibrary();
    }
    catch(...) {}
    LOG_INFO <<"M: Done";
}
