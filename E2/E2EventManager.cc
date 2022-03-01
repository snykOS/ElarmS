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
#include <pthread.h>
#include <errno.h>
#include <stdio.h>

#include "E2EventManager.h"
#include "E2TriggerManager.h"
#include "E2ModuleManager.h"
#include "E2Reader.h"
#include "E2Event.h"
#include "E2Prop.h"
#include <plog/Log.h>

using namespace std;

#define DEFAULT_EVENT_TIMEOUT 3600

bool E2EventManager::initialized = false;
bool E2EventManager::reset_eventid = false;
int E2EventManager::next_eventid = 1;
string E2EventManager::eventid_file = "";
int E2EventManager::event_timeout = DEFAULT_EVENT_TIMEOUT;
int verbose;

E2EventManager::E2EventManager(const string eventid_filename) throw(Error)
{
    if( !initialized )
    {
	initialized = true;

	try {
	    event_timeout = E2Prop::getInt("EventTimeout");
	}
	catch (Error e) { event_timeout = DEFAULT_EVENT_TIMEOUT; }
	verbose = E2Prop::getInt("Verbose");
	try {
	    reset_eventid = E2Prop::getBool("ResetEventID");
	}
	catch (Error e) { reset_eventid = false; }
	E2ModuleManager::param_str << "P: E2EventManager.EventTimeout: " << event_timeout << endl;
	E2ModuleManager::param_str << "P: E2EventManager.ResetEventID: " << reset_eventid << endl;

	if(reset_eventid) {
	    next_eventid = 1;
	}
	else {
	    FILE *fp;
	    char str[32];

	    eventid_file = eventid_filename;

	    if((fp = fopen(eventid_file.c_str(), "r")) != NULL) {
		fgets(str, 32, fp);
		sscanf(str, "%d", &next_eventid);
		fclose(fp);
	    }
	    if(next_eventid <= 0) next_eventid = 1;

	    if((fp = fopen(eventid_file.c_str(), "w")) != NULL) {
		fprintf(fp, "%d\n", next_eventid);
		fclose(fp);
	    }
	    else {
		throw Error("Cannot open EventID file: " + eventid_file
				+ "\n" + strerror(errno));
	    }
	}
	E2Event::getProperties(); // just to print properties in log file.
    }
}

E2EventManager::~E2EventManager() 
{
    // delete remaining events so their history is printed
    for(EventList::iterator it = eventlist.begin(); it != eventlist.end(); it++) {
	delete (*it);
    }
    eventlist.clear();
}

E2Event *E2EventManager::insertEvent(EvType evtype, double t, double la, double lo, double de)
{
    E2Event *event = new E2Event(getNewEventid(), evtype, t, la, lo, de);
    eventlist.push_back(event);
    return event;
}

E2Event *E2EventManager::insertEvent(E2Event *event)
{
    event->eventid = getNewEventid();
    eventlist.push_back(event);
    return event;
}

void E2EventManager::removeOldEvents(E2Time *hb_time, E2TriggerManager *tm)
{
    EventList::iterator it, it_tmp;
    TriggerList::iterator i;
    time_t del_time = hb_time->currentTime() - event_timeout;
      
    for(it = eventlist.begin(); it != eventlist.end(); )
    {
	E2Event *ev = *it;
	if(ev->time < del_time) {
	    vector<E2Trigger *> trigs;
	    for(set<E2Trigger *>::iterator jt = ev->triggers.begin(); jt != ev->triggers.end(); jt++) {
		trigs.push_back(*jt);
	    }

	    it_tmp = it;
	    it++;
	    if(verbose >= 2) {
		stringstream msg_str;
		msg_str << "M: removing event " << (*it_tmp)->toShortString();
		LOG_INFO << msg_str.str();
		msg_str << endl;
		E2ModuleManager::sendLogMsg(msg_str.str());
	    }
	    if(verbose >= 5) {
		ev->logMessages();
	    }
	    delete (*it_tmp);
	    eventlist.erase(it_tmp);

	    deleteEventTriggers(trigs, tm);
	}
	else {
	    it++;
	}
    }
}

void E2EventManager::deleteEventTriggers(vector<E2Trigger *> &trigs, E2TriggerManager *tm)
{
    TriggerMap *trigmap = tm->getTrigMap();
    TriggerMap::iterator im;

    for(vector<E2Trigger *>::iterator it=trigs.begin(); it!=trigs.end(); it++) {
	E2Trigger *t = *it;
	for(im = trigmap->begin(); im != trigmap->end(); im++) {
	    if((*im).second == t) {
		trigmap->erase(im);
		break;
	    }
        }
	if(im == trigmap->end()) {
	    stringstream msg_str;
	    msg_str << "W: Cannot find trigger in trigmap:\n" << t->toShortString("W") << endl;
	    LOG_INFO << msg_str.str();
	    E2ModuleManager::sendLogMsg(msg_str.str());
	}
 	delete t;
    }
}

void E2EventManager::removeTrigger(E2Trigger *trigger)
{
    for(EventList::iterator it = eventlist.begin(); it != eventlist.end(); it++)
    {
	(*it)->removeTrigger(trigger);
    }
}

E2Event * E2EventManager::getEvent(const int eid)
{
    EventList::iterator it;
    for(it = eventlist.begin(); it != eventlist.end(); it++)
    {
	if((*it)->eventid == eid) return (*it);
    }
    return NULL;
}

E2Event *E2EventManager::getEvent(const double t)
{
    EventList::iterator it;
    for(it = eventlist.begin(); it != eventlist.end(); it++)
    {
	if(fabs((*it)->time - t) < TIME_COMPARE_EPSILON) {
	    return (*it);
	}
    }
    return NULL;
}

int E2EventManager::getNewEventid()
{
    int event_id = next_eventid++;

    if(!reset_eventid) {
	FILE *fp;
	if( (fp = fopen(eventid_file.c_str(), "w")) ) {
	    fprintf(fp, "%d\n", next_eventid);
	    fclose(fp);
	}
	else {
	    cerr << "Cannot write to " << eventid_file << endl;
	    cerr << "       " << strerror(errno) << endl;
	}
    }
    return event_id;
}
