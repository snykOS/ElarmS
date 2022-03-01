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
#include <map>
#include <time.h>
#include <unistd.h>
#include <plog/Log.h>
#include "E2ModuleManager.h"
#include "E2TriggerManager.h"
#include "E2EventManager.h"
#include "E2Event.h"
#include "E2Associator.h"
#include "E2Alert.h"
#include "E2Location.h"
#include "E2Magnitude.h"
#include "E2Prop.h"

DMMessageSender * E2ModuleManager::log_sender = NULL;
std::stringstream E2ModuleManager::param_str;
bool E2ModuleManager::send_log = false;

extern int stop_signal;

E2ModuleManager* E2ModuleManager::handle = NULL;

E2ModuleManager* E2ModuleManager::getInstance() throw(Error)
{
    if(!handle){
	handle = new E2ModuleManager();
    }
    return handle;
}

E2ModuleManager::E2ModuleManager() : initialized(false), tm(NULL), em(NULL),
		assoc(NULL), loc(NULL), mag(NULL), alert(NULL)
{
    sender = NULL;
    hb = NULL;
}

E2ModuleManager::~E2ModuleManager()
{
}

void E2ModuleManager::init(bool replay_mode) throw(Error)
{
    if(initialized == true) return;

    // create E2EventManager
    string id_file, ttfile, amq_user, amq_password;
    bool send_message;
    string host = "-";
    extern string program_version;
    char name[1000];

    string send_log_key = E2Prop::getString("SendLogKey");
    if(!strcasecmp(send_log_key.c_str(), "true")) {
        send_log = true;
    }
    else if(!strcasecmp(send_log_key.c_str(), "false")) {
        send_log = false;
    }
    else if(send_log_key.length() > 0 && gethostname(name, (int)sizeof(name)-1) == 0) {
        // if eew-bk-prod1.geo.berkeley.edu, shorten to eew-bk-prod1
        char *c = strstr(name, ".");
        if(c != NULL) *c = '\0';
        host = string(name);
        if(send_log_key.length() > 0) {
            int n = 0;
            for(int i = 0; i < (int)send_log_key.length(); i++) {
                if(host.find(send_log_key.at(i)) != string::npos) n++;
            }
            send_log = (n == (int)send_log_key.length());
        }
    }

    E2ModuleManager::param_str << "Start: " << program_version << " E2 " << host << " " << E2Event::nowToString(3) << endl;

    id_file = E2Prop::getString("EventIDFile");
    ttfile = E2Prop::getString("TTFile");
    min_mag_change = E2Prop::getDouble("MinMagChangeAlert");
    amq_user = E2Prop::getString("AmqUser");
    amq_password = E2Prop::getString("AmqPassword");
    send_message = E2Prop::getBool("SendMessage");

    E2ModuleManager::param_str << "P: E2ModuleManager.EventIDFile: " << id_file << endl;
    E2ModuleManager::param_str << "P: E2ModuleManager.TTFile: " << ttfile << endl;
    E2ModuleManager::param_str << "P: E2ModuleManager.MinMagChangeAlert: " << min_mag_change << endl;
    E2ModuleManager::param_str << "P: E2ModuleManager.AmqUser: " << amq_user << endl;
    E2ModuleManager::param_str << "P: E2ModuleManager.SendMessage: " << send_message << endl;

    if(send_message) {
	event_amq_uri = E2Prop::getString("EventURI");
	event_amq_topic = E2Prop::getString("EventTopic");
	heartbeat_topic = E2Prop::getString("HeartbeatTopic");
        heartbeat_sender = E2Prop::getString("HeartbeatSender");
	E2ModuleManager::param_str << "P: E2ModuleManager.EventURI: " << event_amq_uri << endl;
	E2ModuleManager::param_str << "P: E2ModuleManager.EventTopic: " << event_amq_topic << endl;
	E2ModuleManager::param_str << "P: E2ModuleManager.HeartbeatTopic: " << heartbeat_topic << endl;
	E2ModuleManager::param_str << "P: E2ModuleManager.HeartbeatSender: " << heartbeat_sender << endl;
    }

    E2ModuleManager::param_str << "P: E2ModuleManager.SendLog: " << send_log_key << "(" << send_log << ")" << endl;
    if(send_log && log_sender == NULL) {
	log_uri = E2Prop::getString("LogURI");
	log_topic = E2Prop::getString("LogTopic");
	E2ModuleManager::param_str << "P: E2ModuleManager.logURI: " << log_uri << endl;
	E2ModuleManager::param_str << "P: E2ModuleManager.LogTopic: " << log_topic << endl;
    }

    em = new E2EventManager(id_file);

    tm = new E2TriggerManager(replay_mode);

    alert = new E2Alert(tm, em);

    loc = new E2Location(tm, em, ttfile);

    assoc = new E2Associator(tm, em, loc);

    mag = new E2Magnitude(); 

    if(send_message) {
	if(sender == NULL) {
	    if(event_amq_uri == tm->getAmqUri()) {
		sender = new DMMessageSender(tm->getConnection(), event_amq_topic);
	    }
	    else {
		sender = new DMMessageSender(event_amq_uri, amq_user, amq_password, event_amq_topic);
	    }
	    sender->run();
	    hb = new HBProducer(sender->getConnection(), heartbeat_sender, heartbeat_topic);
	}
    }

    if(send_log && log_sender == NULL) {
	if(sender != NULL && log_uri == event_amq_uri) {
	    log_sender = new DMMessageSender(sender->getConnection(), log_topic);
	}
	else {
	    log_sender = new DMMessageSender(log_uri, amq_user, amq_password, log_topic);
	}
	log_sender->setSource(heartbeat_sender);
	log_sender->run();
    }

    LOG_INFO << E2ModuleManager::param_str.str();

    sendLogMsg(E2Event::logTime());
    sendLogMsg(E2ModuleManager::param_str.str());

    tm->run();

    initialized = true;
}

void E2ModuleManager::cleanup()
{
    if(assoc) delete assoc;
    if(loc) delete loc;
    if(mag) delete mag;
    if(alert) delete alert;
    if(tm) {
	tm->cleanup();
	delete tm;	
    }
    if(em) delete em;    

    if(hb) hb->close();
//    if(sender)
//	delete sender; dumps core
    if(sender) sender->close();
//hb->close();

//    if(hb)
//	delete hb; dumps core

//      delete handle;
}

void E2ModuleManager::run()
{
    int evid=0;

    // As long as evid > 0, there could be more work to do in associateTriggers.
    // If evid == 0, all work with existing triggers has been done and
    // tm->read() can block until new triggers arrive.

    while( !stop_signal && tm->read(evid) >= 0 )
    {
	tm->checkTeleseisms();

	if(tm->newTrigger() || evid > 0) {
	    if((evid = assoc->associateTriggers()) > 0) {
		E2Event *event = em->getEvent(evid);
		if(event != NULL) {
		    E2Magnitude::calcMag(event);
		    alert->alert(evid);
		}
	    }
	}

	if(tm->updated_trigs.size() > 0) {
	    updateEvents(evid);
	}
	alert->sendDelayedAlerts();

	em->removeOldEvents(tm->getReader(), tm);
	tm->removeOldTriggers(em);
//	alert->sendDelayedUpdates();
    }
    tm->stopReader();

    delete em; // forces remaining event summaries to be printed
}

void E2ModuleManager::updateEvents(int evid)
{
    set<int> ids;  // ids of events to update

    // Collect unique event ids from the updated triggers.
    // Don't include evid, since that event has just been processed
    for(int i = 0; i < (int)tm->updated_trigs.size() && (int)ids.size() < 10; i++)
	if(tm->updated_trigs[i]->isAssociated() && tm->updated_trigs[i]->getEventid() > 0
		&& tm->updated_trigs[i]->getEventid() != evid)
    {
	ids.insert(tm->updated_trigs[i]->getEventid());
    }

    for(set<int>::iterator it = ids.begin(); it != ids.end(); it++) {
	E2Event *event = em->getEvent(*it);
	if(event != NULL) {
	    E2Magnitude::calcMag(event);
	    double mag_change = event->evmag - event->mag_checked;
	    if(fabs(mag_change) > min_mag_change) {
		alert->alert(*it);
	    }
	}
    }
    tm->updated_trigs.clear();
}

bool E2ModuleManager::sendLogMsg(string msg)
{
    if(!send_log || !log_sender) return false;
    log_sender->sendString(msg);
    return true;
}

bool E2ModuleManager::sendLogMsg(list<string> &msg)
{
    if(!send_log || !log_sender) return false;
    stringstream s;
    for(list<string>::iterator it = msg.begin(); it != msg.end(); it++) {
        if(it != msg.begin()) s << endl;
        s << (*it);
    }
    log_sender->sendString(s.str());
    return true;
}

bool E2ModuleManager::sendLogMsg(string line_label, list<string> &msg)
{
    if(!send_log || !log_sender) return false;
    stringstream s;
    for(list<string>::iterator it = msg.begin(); it != msg.end(); it++) {
        if(it != msg.begin()) s << endl;
        s << line_label << (*it);
    }
    log_sender->sendString(s.str());
    return true;
}

