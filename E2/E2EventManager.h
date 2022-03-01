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
#ifndef _E2EventManager_H__
#define	_E2EventManager_H__

#define TIME_COMPARE_EPSILON 0.2

#include <vector>
#include <list>
#include "EventCore.h"
#include "Exceptions.h" // for Error

class E2Event;
class E2Time;
class E2ModuleManager;
class E2Trigger;
class E2TriggerManager;

typedef std::list<E2Event *> EventList;

class E2EventManager 
{
 private:
    static bool initialized;
    static bool reset_eventid;
    static int next_eventid;
    static std::string eventid_file;
    static int event_timeout;

    EventList eventlist;

    int getNewEventid();
    void deleteEventTriggers(std::vector<E2Trigger *> &trigs, E2TriggerManager *em);
    
 public:

    /**
     * @param eventid_filename file name of event id.
     */
    E2EventManager(const std::string eventid_filename="elarms_event.id") throw(Error);

    ~E2EventManager();

    /**
     * Insert a Event into list.
     * @param time Event time.
     * @param latitude Event location
     * @param logitude Event location
     * @param depth Event depth
     *
     * @return Pointer of E2Event.
     */
    E2Event *insertEvent(EvType evtype, double time, double latitude,
		double longitude, double depth);
    
    E2Event *insertEvent(E2Event *event);

    /**
     * Remove Event older than current time minus twin_del_event.
     * @return integer(TN_TRUE, TN_FALSE)
     */
    void removeOldEvents(E2Time *hb_time, E2TriggerManager *tm);

    void removeTrigger(E2Trigger *trigger);

    /**
     * Get specific Event using event id.
     * @param eid event id.
     * @return pointer of E2Event.
     */
    E2Event *getEvent(const int eid);

    /**
     * Get specific Event using event time
     * @param d double type event time.
     * @return pointer of E2Event.
     */
    E2Event *getEvent(const double time);

    EventList *getEventList() { return &eventlist; }
};

#endif
