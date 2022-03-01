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
#ifndef _E2Associator_h_
#define _E2Associator_h_
/**
 * \file   E2Associator.h
 *
 * \author Holly, <hollybrown@berkeley.edu>
 *
 * \date   2010/06/18 Created by Holly, <hollybrown@berkeley.edu>
 *         2010/06/21 Updated by Lim, <limlhj@kigam.re.kr>
 *
 * \brief  Associates triggers to form events
 *
 */
#include <pthread.h>
#include <deque>
#include <map>
#include "E2Event.h"
#include "E2EventManager.h"

class E2Trigger;
class E2Event;
class E2EventManager;
class E2TriggerManager;
class E2Location;
class E2Associator;

class CountThread {
  public:
    pthread_t id;
    E2Associator *parent;
    int eventid, version;
    double evkm, evlat, evlon, evtime;
    int ibeg, iend;
    std::map<int, CountInfo> count_info;
    CountThread() : parent(NULL), eventid(0), version(0), evkm(0.), evlat(0.), evlon(0.), evtime(0.),
		ibeg(0), iend(0) {}
};

class TrialEvent
{
  public:
    std::map<std::string, double> triggers;
    double evlat;
    double evlon;
    double evtime;
    int eventid;
    TrialEvent() : evlat(0.), evlon(0.), evtime(0.), eventid(0) {}
};

class E2Associator
{
 private:
    int multiStaMinTrigs;   // minimum number of triggers for multi-station event creation
    double multiStaMaxDist; // maximum trigger to source distance for multi-station event creation
    double pVelocity;       // P velocity (km/sec) used in multi-station and two-station event creation
    double staTTAllowance;  // Trigger time differences between stations must be <= distance/pVelocity + staTTAllowance

    bool useSingleStation; // If true, create a single-trigger event using singleStaMinTP and singleStaMinPD
    double singleStaMinTP; // minimum log10(TauPmax) value to declare a single-station event 
    double singleStaMinPD; // minimum log10(Pd) value to declare a single-station event
    bool useTwoStations;   // If true, create a two-station event using twoStaMinTP and twoStaMinPD
    double twoStaMinTP;	   // minimum log10(TauPmax) value to declare a two-station event 
    double twoStaMinPD;	   // minimum log10(Pd) value to declare a two-station event

    double maxTwoStaDist;
    double maxAssocToEventDist;
    double trigStaPercent; // Percentage of near-source stations which must trigger

    double blackoutTime; // To prevent split/duplicate events, no second event within this space/time
    double blackoutDist; // To prevent split/duplicate events, no second event within this space/time

    double nearSrcKm;    // near source association criteria
    double nearTTMinDif; // near source association criteria
    double nearTTMaxDif; // near source association criteria
    int maxStaRelocate; // when associating additional triggers, compute a relocation if stations <= maxStaRelocate
    double pSlow;    // timemin = slightly less than P-wave arrival time for this distance = pSlow*evkm - pOffset
    double pOffset;
    double sSlow;    // timemax = slightly more than S-wave arrival time for this distance = sSlow*evkm + sOffset
    double sOffset;
    double maxStationDelay; // A station is considered inactive for the current event, if there has been no data within
			    // this many seconds after the expected P arrival from the current event
    bool printCount;	// print stations that are counted and skipper for the trigStaPercent computation
    bool useGMPeak;	// if true, use the gmpeak topic to determine if a station is active for counting purposes.
    double maxCreateMisfit; // maximum location misfit for initial event creation
    double maxAssocMisfit; // maximum location misfit for a trigger association to an existing event
    double maxAssocRMSFactor; // maximum allowed increase in the event location rms when associating a trigger,
                              // expressed as a multiplier. new_event_rms must be <= maxAssocRMSFactor*current_event_rms
    double logPercentMin;  // minimum event percnt value for logging build and count info
    double teleseismLevel; // 1: use region window_start and window_end; 2: use station arrival time window.
    double grid_km; // see E2Locate.h

    int verbose;
    E2TriggerManager *tm;
    E2EventManager *em;
    E2Location *loc;
    E2Trigger **trigs;
    int size_trigs;
    std::set<std::string> triggered_stas;
    std::deque<TrialEvent> trial_events;

    CountThread count_thread[4];

    void getParams();
    bool associateToEvent(EventList *evlist, int num_triggers, E2Trigger **triggers, int *eventid);
    bool multiStationEvent(int num_triggers, E2Trigger **triggers, int *eventid, bool loopforward);
    bool twoStationEvent(int num_triggers, E2Trigger **triggers, int *eventid);
    bool singleStationEvent(int num_triggers, E2Trigger **triggers, int *eventid);
    bool countNeighbors(E2Event *event, CountNeighbors *cn);
    bool splitEvent(double evlat, double evlon, double evtime);
    bool splitEvent(int eventid, double evlat, double evlon, double evtime);
    void getDkm(E2Event *e);

    void printTrialEvent(E2Event &event, int num_trigs);

 public:
    E2Associator(E2TriggerManager *tm, E2EventManager *em, E2Location *e2loc) throw(Error);

    ~E2Associator() {
	std::cout << "Shut down E2Associator" << std::endl;
    } 

    int associateTriggers();
    void countStations(CountThread *t);
};
#endif
