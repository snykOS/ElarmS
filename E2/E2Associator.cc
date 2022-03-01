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
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <sys/time.h>
#include <deque>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <pthread.h>
#include <set>

#include <plog/Log.h>
#include "E2Associator.h"
#include "E2Magnitude.h"
#include "E2ModuleManager.h"
#include "E2TriggerManager.h"
#include "E2Location.h"
#include "E2Alert.h"
#include "TimeStamp.h"
#include "E2Prop.h"
#include "deltaz.h"

extern "C" {
  static int sortTrigs(const void *A, const void *B)
  {
    E2Trigger **a = (E2Trigger **)A;
    E2Trigger **b = (E2Trigger **)B;
    return (*a)->trigger_time < (*b)->trigger_time ? -1 : 1;
  }
  static void *CountStations(void *client_data);
}

/**
 * \file   E2Associator.cc
 * \author Holly, <hollybrown@berkeley.edu>
 * \date   2012 modified by <henson@seismo.berkeley.edu>
 * \brief  Associates triggers with existing events, or associates triggers into new events.
 */

E2Associator::E2Associator(E2TriggerManager *trigm, E2EventManager *evtm, E2Location *e2loc) throw(Error) :
    multiStaMinTrigs(3),   // minimum number of triggers for multi-station event creation
    multiStaMaxDist(100.0),// maximum trigger to source distance for multi-station event creation
    pVelocity(6.0),	   // P velocity (km/sec) used in multi-station and two-station event creation
    staTTAllowance(3.0),   // Trigger time differences between stations must be <= distance/pVelocity + staTTAllowance

    useSingleStation(false),// If true, create a single-trigger event using singleStaMinTP and singleStaMinPD
    singleStaMinTP(0.45),   // minimum log10(TauPmax) value to declare a single-station event
    singleStaMinPD(1.25),   // minimum log10(Pd) value to declare a single-station event
    useTwoStations(false),  // If true, create a two-station event using twoStaMinTP and twoStaMinPD
    twoStaMinTP(0.3),	    // minimum log10(TauPmax) value to declare a two-station event
    twoStaMinPD(0.5),	    // minimum log10(Pd) value to declare a two-station event

    maxTwoStaDist(30.0),
    maxAssocToEventDist(2500.0),
    trigStaPercent(0.4),   // Percentage of near-source stations which must trigger

    blackoutTime(15.0),    // To prevent split/duplicate events, no second event within this space/time
    blackoutDist(90.0),    // To prevent split/duplicate events, no second event within this space/time

    nearSrcKm(20.0),	   // near source association criteria
    nearTTMinDif(-2.0),    // near source association criteria
    nearTTMaxDif(4.0),     // near source association criteria
    maxStaRelocate(10),    // when associating additional triggers, compute a relocation if stations <= maxStaRelocate
    pSlow(0.15),	   // timemin = slightly less than P-wave arrival time for this distance = pSlow*evkm - pOffset
    pOffset(10.0),
    sSlow(0.26),	   // timemax = slightly more than S-wave arrival time for this distance = sSlow*evkm + sOffset
    sOffset(0.0),

    maxStationDelay(20.),  // A station is considered inactive for the current event, if there has been no data since
			   // this many seconds before the expected P arrival from the current event

    maxCreateMisfit(1.0),   // maximum location misfit for initial event creation
    maxAssocMisfit(5.0),    // maximum location misfit for a trigger association to an existing event
    maxAssocRMSFactor(5.0), // maximum allowed increase in the event location rms when associating a trigger,
			    // expressed as a multiplier. new_event_rms must be <= maxAssocRMSFactor*current_event_rms

    verbose(0),

    tm(trigm), em(evtm), loc(e2loc)
{
    getParams();

    size_trigs = 200;
    trigs = (E2Trigger **)malloc(size_trigs*sizeof(E2Trigger *));

    for(int i = 0; i < 4; i++) {
	count_thread[i].parent = this;
    }
}

void E2Associator::getParams()
{
    multiStaMinTrigs = E2Prop::getInt("MultiStaMinTrigs");
    multiStaMaxDist = E2Prop::getDouble("MultiStaMaxDist");
    pVelocity = E2Prop::getDouble("PVelocity");
    staTTAllowance = E2Prop::getDouble("StaTTAllowance");
    useSingleStation = E2Prop::getBool("UseSingleStation");
    singleStaMinTP = E2Prop::getDouble("SingleStaMinTP");
    singleStaMinPD = E2Prop::getDouble("SingleStaMinPD");
    useTwoStations = E2Prop::getBool("UseTwoStations");
    twoStaMinTP = E2Prop::getDouble("TwoStaMinTP");
    twoStaMinPD = E2Prop::getDouble("TwoStaMinPD");
    maxTwoStaDist = E2Prop::getDouble("MaxTwoStaDist");
    maxAssocToEventDist = E2Prop::getDouble("MaxAssocToEventDist");
    trigStaPercent = E2Prop::getDouble("TrigStaPercent");
    blackoutTime = E2Prop::getDouble("BlackoutTime");
    blackoutDist = E2Prop::getDouble("BlackoutDist");
    nearSrcKm = E2Prop::getDouble("NearSrcKm");
    nearTTMinDif = E2Prop::getDouble("NearTTMinDif");
    nearTTMaxDif = E2Prop::getDouble("NearTTMaxDif");
    maxStaRelocate = E2Prop::getInt("MaxStaRelocate");
    pSlow = E2Prop::getDouble("PSlow");
    pOffset = E2Prop::getDouble("POffset");
    sSlow = E2Prop::getDouble("SSlow");
    sOffset = E2Prop::getDouble("SOffset");
    maxStationDelay = E2Prop::getDouble("MaxStationDelay");
    verbose = E2Prop::getInt("Verbose");
    try {
	logPercentMin = E2Prop::getDouble("LogPercentMin");
    }
    catch(Error e) { // optional
	logPercentMin = 20.0;
    }
    useGMPeak = E2Prop::getBool("UseGMPeak");
    maxCreateMisfit = E2Prop::getDouble("MaxMisfit");
    maxAssocMisfit = E2Prop::getDouble("MaxAssocMisfit");
    maxAssocRMSFactor = E2Prop::getDouble("MaxAssocRMSFactor");
    teleseismLevel = E2Prop::getDouble("TeleseismLevel");
    grid_km = E2Prop::getDouble("GridKm");

    E2ModuleManager::param_str << fixed << setprecision(2);
    E2ModuleManager::param_str << "P: E2Associator.MultiStaMinTrigs: " << multiStaMinTrigs << endl;
    E2ModuleManager::param_str << "P: E2Associator.MultiStaMaxDist: " << multiStaMaxDist << endl;
    E2ModuleManager::param_str << "P: E2Associator.PVelocity: " << pVelocity << endl;
    E2ModuleManager::param_str << "P: E2Associator.StaTTAllowance: " << staTTAllowance << endl;
    E2ModuleManager::param_str << "P: E2Associator.UseSingleStation: " << useSingleStation << endl;
    E2ModuleManager::param_str << "P: E2Associator.SingleStaMinTP: " << singleStaMinTP << endl;
    E2ModuleManager::param_str << "P: E2Associator.SingleStaMinPD: " << singleStaMinPD << endl;
    E2ModuleManager::param_str << "P: E2Associator.UseTwoStations: " << useTwoStations << endl;
    E2ModuleManager::param_str << "P: E2Associator.TwoStaMinTP: " << twoStaMinTP << endl;
    E2ModuleManager::param_str << "P: E2Associator.TwoStaMinPD: " << twoStaMinPD << endl;
    E2ModuleManager::param_str << "P: E2Associator.MaxTwoStaDist: " << maxTwoStaDist << endl;
    E2ModuleManager::param_str << "P: E2Associator.MaxAssocToEventDist: " << maxAssocToEventDist << endl;
    E2ModuleManager::param_str << "P: E2Associator.TrigStaPercent: " << trigStaPercent << endl;
    E2ModuleManager::param_str << "P: E2Associator.BlackoutTime: " << blackoutTime << endl;
    E2ModuleManager::param_str << "P: E2Associator.BlackoutDist: " << blackoutDist << endl;
    E2ModuleManager::param_str << "P: E2Associator.NearSrcKm: " << nearSrcKm << endl;
    E2ModuleManager::param_str << "P: E2Associator.NearTTMinDif: " << nearTTMinDif << endl;
    E2ModuleManager::param_str << "P: E2Associator.NearTTMaxDif: " << nearTTMaxDif << endl;
    E2ModuleManager::param_str << "P: E2Associator.MaxStaRelocate: " << maxStaRelocate << endl;
    E2ModuleManager::param_str << "P: E2Associator.PSlow: " << pSlow << endl;
    E2ModuleManager::param_str << "P: E2Associator.POffset: " << pOffset << endl;
    E2ModuleManager::param_str << "P: E2Associator.SSlow: " << sSlow << endl;
    E2ModuleManager::param_str << "P: E2Associator.SOffset: " << sOffset << endl;
    E2ModuleManager::param_str << "P: E2Associator.MaxStationDelay: " << maxStationDelay << endl;
    E2ModuleManager::param_str << "P: E2Associator.Verbose: " << verbose << endl;
    E2ModuleManager::param_str << "P: E2Associator.LogPercentMin: " << logPercentMin << endl;
    E2ModuleManager::param_str << "P: E2Associator.UseGMPeak: " << useGMPeak << endl;
    E2ModuleManager::param_str << "P: E2Associator.MaxMisfit: " << maxCreateMisfit << endl;
    E2ModuleManager::param_str << "P: E2Associator.MaxAssocMisfit: " << maxAssocMisfit << endl;
    E2ModuleManager::param_str << "P: E2Associator.MaxAssocRMSFactor: " << maxAssocRMSFactor << endl;
    E2ModuleManager::param_str << "P: E2Associator.TeleseismLevel: " << teleseismLevel << endl;
}

int E2Associator::associateTriggers()
{
    EventList *evlist = em->getEventList();
    TriggerMap *tmap = tm->getTrigMap();

    if(size_trigs < (int)tmap->size()) {
	size_trigs = tmap->size() + 50;
	trigs = (E2Trigger **)realloc(trigs, size_trigs*sizeof(E2Trigger *));
    }

    // Collect unassociated triggers that pass minimum criteria
    int num_trigs = 0;
    for(TriggerMap::iterator it=tmap->begin(); it != tmap->end(); it++)
    {
	E2Trigger *t = (*it).second;
	if(t->getAssocCode() == UNASSOCIATED && t->teleseismic < teleseismLevel && t->getQualityCode() == OKAY) {
	    trigs[num_trigs++] = t;
	}
    }
    if(num_trigs == 0) return 0;

    int eventid=0;

    qsort(trigs, num_trigs, sizeof(E2Trigger *), sortTrigs);

    if(	associateToEvent(evlist, num_trigs, trigs, &eventid) ||
	multiStationEvent(num_trigs, trigs, &eventid,  true) ||
	multiStationEvent(num_trigs, trigs, &eventid, false) ||
	twoStationEvent(num_trigs, trigs, &eventid) ||
	singleStationEvent(num_trigs, trigs, &eventid) )
    {
	return eventid;
    }
    return 0;
}

bool E2Associator::associateToEvent(EventList *evlist, int num_triggers, E2Trigger **triggers, int *eventid)
{
    bool associated = false;
    double evkm, evtimediff, timemin, timemax, dist;
    int ev_index, nsta;
    set<E2Trigger *>::iterator jt;
    AssocCode assoc_code;

    ev_index = 0;
    for(EventList::iterator ev = evlist->begin(); ev != evlist->end(); ev++, ev_index++) if((*ev)->split_ok)
    {
	// Consider only triggers that are not already associated to the event (*ev)
	// and are not from the same net.sta as an existing trigger in the event (*ev)
	// This loop can associate multiple triggers to the event (*ev)
	for(int i = 0; i < num_triggers; i++)
	    if( !triggers[i]->isAssociated() && !(*ev)->containsChannel(triggers[i]))
	{
	    E2Trigger *t = triggers[i]; // the candidate trigger
	    // Calculate trigger time - origin time and station-to-event distance
	    evtimediff = t->trigger_time - (*ev)->time;
	    evkm = distancekm((*ev)->lat, (*ev)->lon, t->getLat(), t->getLon());
	    AssocInfo assoc_info(*ev, t, evtimediff, evkm);
	    LocateInfo loc_info(3);

	    // Check distance/time criteria
	    // Use maximum distance and P/S travel times
	    // timemin = slightly less than P-wave arrival time for this distance
	    // timemax = slightly more than S-wave arrival time for this distance
	    // timemin = 0.15*evkm - 10;
	    // timemax = 0.26*evkm;

	    if(evkm < 700.) {
		timemin = pSlow*evkm - pOffset;
		assoc_info.ttmin = timemin;
	    }
	    else {
		double pSlowFar = 0.70*pSlow;
		timemin = (pSlow*700 - pOffset) + pSlowFar*(evkm-700.);
		assoc_info.ttmin = timemin;
	    }
	    timemax = sSlow*evkm + sOffset;
	    assoc_info.ttmax = timemax;
	    assoc_info.tt_ok = (evtimediff >= timemin && evtimediff <= timemax);
	    assoc_info.dist_ok = (evkm < maxAssocToEventDist);
	    assoc_info.near_tt_ok = (evtimediff >= nearTTMinDif && evtimediff <= nearTTMaxDif);
	    assoc_info.near_dist_ok = (evkm <= nearSrcKm);

	    // if the trigger station distance and trigger time are within acceptable bounds
	    if(((assoc_info.near_dist_ok && assoc_info.near_tt_ok) || (assoc_info.dist_ok && assoc_info.tt_ok)))
	    {
		assoc_code = (evkm <= nearSrcKm && evtimediff >= nearTTMinDif
						&& evtimediff <= nearTTMaxDif) ? NEAR_SRCE : TT_WINDOW;
		E2Event event;
		// setup a temporary event that includes the candidate trigger t
		event.eventid = (*ev)->eventid;
		event.version = (*ev)->version;
		event.lat = (*ev)->lat;
		event.lon = (*ev)->lon;
		event.depth = (*ev)->depth;
		event.time = (*ev)->time;
		event.misfit_rms = (*ev)->misfit_rms;
		event.misfit_ave = (*ev)->misfit_ave;
		// copy existing triggers to the temporary event for counting and location
		for(jt = (*ev)->triggers.begin(); jt != (*ev)->triggers.end(); jt++) {
		    event.triggers.insert(*jt);
		}

		// add the new trigger to the temporary event
		event.triggers.insert(t);
	  
	        // relocate for each new station up to maxStaRelocate
	        // do not relocate if the station is off the grid
		bool relocated = false;
		nsta = E2Alert::countStations(&event, tm);

		if(nsta <= maxStaRelocate && evkm < grid_km) {
		    loc_info.init(event);
		    loc->locate(&event, loc_info); // trial location
		    // check for split event conflict
		    if(splitEvent((*ev)->eventid, event.lat, event.lon, event.time)) {
			// this event has relocated and is now a split event
			(*ev)->split_ok = false;
                        event.triggers.clear();
			continue;
		    }
		    // find the minimum station distance to this new event location
		    double min_dist = 0.;
		    for(jt = event.triggers.begin(); jt != event.triggers.end(); jt++) {
			dist = distancekm((*jt)->getLat(), (*jt)->getLon(), event.lat, event.lon);
			if(jt == event.triggers.begin() || dist < min_dist) min_dist = dist;
		    }
		    assoc_info.relocate = true;
		    assoc_info.newLocation(&event);
		    assoc_info.min_dist = min_dist;
		    assoc_info.mag_dist_ok = (min_dist < E2Magnitude::maxMagKm(&event));
		    relocated = true;
		}

		// if the number of stations counting the new trigger is > 4,
		// check that the location average misfit is < maxAssocMisfit
		if(nsta > 4) {
		    assoc_info.misfit_ok = (event.misfit_ave < maxAssocMisfit);

		    // also, if an alert has been sent for this event and the new event misfit_rms is > 1.0
		    // compare the new event misfit_rms with the current event misfit_rms without the 
		    // candidate trigger t.
		    // if the new event misfit_rms is > the current event misfit_rms * maxAssocRMSFactor
		    // then do not associate the candidate trigger.
		    if((*ev)->alert_sent && event.misfit_rms > 1.0
			&& event.misfit_rms > maxAssocRMSFactor*(*ev)->misfit_rms)
		    {
			assoc_info.misfit_ok = false;
		    }
		}
		else {
		    assoc_info.misfit_ok = (event.misfit_ave < maxCreateMisfit);
		}

		assoc_info.percent_ok = countNeighbors(&event, &assoc_info.cn);

		if(assoc_info.misfit_ok && assoc_info.percent_ok)
		{
		    (*ev)->percent = assoc_info.cn.percent;
		    (*ev)->addTrigger(t, assoc_code); 

		    if(relocated) {
			(*ev)->lat = event.lat;
			(*ev)->lon = event.lon;
			(*ev)->depth = event.depth;
			(*ev)->time = event.time;
			(*ev)->misfit_rms = event.misfit_rms;
			(*ev)->misfit_ave = event.misfit_ave;
		    }
		    associated = true;
		    (*ev)->computeAzimuthError();
		    (*ev)->assoc_info.push_back(assoc_info);
		    if(relocated) (*ev)->loc_info.push_back(loc_info);
		}
		else if(assoc_info.misfit_ok && (nsta > maxStaRelocate && evtimediff < timemin + 40))
		{
		    // except for the percent-triggered requirement, this trigger could belong to this
		    // event. Mark it as associated so that it cannot be associated with another event,
		    // but don't actually use it in this event
		    t->assoc_code = assoc_code;
                    t->free_with_unassoc = true; // flag it to be freed with unassociated triggers
		    if(verbose >= 1) {
			LOG_INFO << "R: " << t->toString();
		    }
		}
		event.triggers.clear();
	    }
	    if(!associated && verbose >= 1) {
		// save one failed attempt per trigger
		long l = (long)(1000*t->trigger_time + 0.5); // msecs
		if((*ev)->failed_assoc_info.find(l) == (*ev)->failed_assoc_info.end()) {
		    (*ev)->failed_assoc_info[l] = assoc_info;
		}
	    }
	}
	// If successfully associated any triggers with this event
	if( associated ) 
	{
	    *eventid = (*ev)->eventid;
	    return true;
	}
    }
    return false;
}

bool E2Associator::multiStationEvent(int num_triggers, E2Trigger **triggers, int *eventid, bool loopforward)
{
    int num, num_event_trigs, sci, nsta;
    double evlat, evlon, evdepth, evtime, misfit, dist, tt, tdif, max_ttdif1, max_ttdif2;
    double max_ttime = multiStaMaxDist/pVelocity;
    double maxlatdif = multiStaMaxDist/111.19492664;
    map<int, E2Trigger *> sta_trigs;
    map<int, E2Trigger *>::iterator it;
    bool percent_ok, include_trigger, replaced, located;
    E2Event event;

    for(int i = 0; i < num_triggers; i++) if( !triggers[i]->isAssociated() )
    {
	E2Trigger *t = triggers[i];

	// initialize trigger->used field
	for(int j = 0; j < num_triggers; j++) {
	    if(!triggers[j]->isAssociated()) triggers[j]->used = false;
	}

	trigs[0] = t;
        num_event_trigs = 1;
	sta_trigs.clear();
	sta_trigs[t->staId()] = t;
	t->used = true;

	// initialize event location to first trigger
	evlat = t->getLat();
	evlon = t->getLon();
	evdepth = 8.0;
	evtime = t->trigger_time;
	misfit = 0.;

	event.eventid = -1;
	event.lat = evlat;
	event.lon = evlon;
	event.depth = evdepth;
	event.time = evtime;
	event.triggers.clear();
	event.build_info.clear();
	event.loc_info.clear();
        event.addTrigger(t, UNASSOCIATED);
	located = false;

	int build_loop = 0;
	do {
	    build_loop++;
	    num = num_event_trigs;
	    int build_order = 1;
	    int jstart = loopforward ? i+1 : 0;
	    for(int j = jstart; j < num_triggers; j++) {
		E2Trigger *r = triggers[j];
		if(r->isAssociated() || r->used) continue;
		BuildInfo build_info(event, build_loop, build_order, t, r);
		build_order++;

		double statt_err, sta_max_tterr=-1.e+60;
		string sta_max;
		tt = fabs(r->trigger_time - evtime);
		dist = distancekm(r->getLat(), r->getLon(), evlat, evlon);
		build_info.evtime = evtime;
		build_info.evlat = evlat;
		build_info.evlon = evlon;
		build_info.tt = tt;
		build_info.tt_ok = (tt <= max_ttime) ? true : false;
		build_info.dist = dist;
		build_info.dist_ok = (dist <= multiStaMaxDist) ? true : false;

		for(it = sta_trigs.begin(); it != sta_trigs.end(); it++) {
		    dist = distancekm(r->getLat(), r->getLon(), (*it).second->getLat(), (*it).second->getLon());
		    tt = dist/pVelocity;
		    statt_err = fabs(r->trigger_time - (*it).second->trigger_time) - tt;
		    if(statt_err > sta_max_tterr) {
			sta_max_tterr = statt_err;
			sta_max = (*it).second->getSta();
			build_info.sta_max_dist = dist;
		    }
		}
		build_info.sta_max = sta_max;
		build_info.sta_max_tterr = sta_max_tterr;
		build_info.sta_max_tterr_ok = (sta_max_tterr <= staTTAllowance) ? true : false;

		include_trigger = false;

		if(fabs(r->trigger_time - evtime) <= max_ttime && fabs(r->getLat()-evlat) <= maxlatdif &&
		    distancekm(r->getLat(), r->getLon(), evlat, evlon) <= multiStaMaxDist)
		{
		    for(it = sta_trigs.begin(); it != sta_trigs.end(); it++) {
			dist = distancekm(r->getLat(), r->getLon(), (*it).second->getLat(), (*it).second->getLon());
			tt = dist/pVelocity;
			if(fabs(r->trigger_time - (*it).second->trigger_time) > tt + staTTAllowance) break;
		    }
		    if(it == sta_trigs.end()) include_trigger = true;
		}
		build_info.include_trigger = include_trigger;
		if(include_trigger) 
		{
		    replaced = false;
		    sci = r->staChanId();
		    int k = 0;
		    for(k = 0; k < num_event_trigs; k++) {
		        if(trigs[k]->staChanId() == sci) break;
		    }
		    if(k < num_event_trigs) { // duplicate sta/chan. Check which trigger fits better
			max_ttdif1 = 0.;
			for(it = sta_trigs.begin(); it != sta_trigs.end(); it++)
			    if((*it).second->staChanId() != sci)
			{
			    dist = distancekm(trigs[k]->getLat(), trigs[k]->getLon(),
						(*it).second->getLat(), (*it).second->getLon());
			    tt = dist/pVelocity;
			    if((tdif = fabs(trigs[k]->trigger_time - (*it).second->trigger_time)) > tt) {
				double dif = tdif - tt;
				if(dif > max_ttdif1) max_ttdif1 = dif;
			    }
			}
			max_ttdif2 = 0.;
			for(it = sta_trigs.begin(); it != sta_trigs.end(); it++)
			    if((*it).second->staChanId() != sci)
			{
			    dist = distancekm(r->getLat(), r->getLon(), (*it).second->getLat(), (*it).second->getLon());
			    tt = dist/pVelocity;
			    if((tdif = fabs(r->trigger_time - (*it).second->trigger_time)) > tt) {
				double dif = tdif - tt;
				if(dif > max_ttdif2) max_ttdif2 = dif;
			    }
			}
			if(max_ttdif2 < max_ttdif1) {
			    replaced = true; // new trigger fits better. replace the older one with this one
			    event.removeTrigger(trigs[k], false);
			    trigs[k] = r;
			    event.addTrigger(r, UNASSOCIATED);
			}
			else {
			    continue;
			}
		    }

		    nsta = sta_trigs.size();
		    build_info.nsta = nsta;
		    if(nsta <= 3) {
			// Calculate new temporary event/centroid location
			if(!replaced) {
			    event.addTrigger(r, UNASSOCIATED);
			    evlat = (evlat*num_event_trigs + r->getLat())/(num_event_trigs+1);
			    evlon = (evlon*num_event_trigs + r->getLon())/(num_event_trigs+1);
			    if(r->trigger_time < evtime) evtime = r->trigger_time;
			}
			else {
			    evlat = r->getLat();
			    evlon = r->getLon();
			    evtime = r->trigger_time;
			    for(k = 0; k < num_event_trigs; k++) {
				evlat += trigs[k]->getLat();
				evlon += trigs[k]->getLon();
				if(trigs[k]->trigger_time < evtime) trigs[k]->trigger_time = evtime;
			    }
			    evlat /= (num_event_trigs+1);
			    evlon /= (num_event_trigs+1);
			}
			percent_ok = true;
			if(nsta == 3) {
			    event.lat = evlat;
			    event.lon = evlon;
			    event.time = evtime;
			    LocateInfo loc_info(1);
			    loc_info.init(event);
			    if(loc->locate(&event, loc_info) != 0) located = true;
			    event.loc_info.push_back(loc_info);
			    evlat = event.lat;
			    evlon = event.lon;
			    evdepth = event.depth;
			    evtime = event.time;
			    misfit = event.misfit_ave;
                        }
		    }
		    else {
			if(!replaced) event.addTrigger(r, UNASSOCIATED);
			LocateInfo loc_info(1);
			loc_info.init(event);
			if(loc->locate(&event, loc_info) != 0) located = true;
			event.loc_info.push_back(loc_info);

			percent_ok = true;
			// countNeighbors is too slow here. Do count below
//			percent_ok = countNeighbors(&event, &mindist, &maxdist, &percent);
			if(percent_ok) {
			    evlat = event.lat;
			    evlon = event.lon;
			    evdepth = event.depth;
			    evtime = event.time;
			    misfit = event.misfit_ave;
			}
			else {
			    if(!replaced) event.triggers.erase(r);
			}
		    }
		    if(percent_ok) {
			if(!replaced) trigs[num_event_trigs++] = r;
			sta_trigs[r->staId()] = r;
			r->used = true;
		    }
		    build_info.evlat = evlat;
		    build_info.evlon = evlon;
		    build_info.evdepth = evdepth;
		    build_info.evtime = evtime;
		    build_info.located = located;
		    if(located) {
			build_info.misfit_ave = event.misfit_ave;
			build_info.misfit_rms = event.misfit_rms;
		    }
		    nsta = sta_trigs.size();
		    build_info.ntrig = num_event_trigs;
		    build_info.new_nsta = nsta;
		    if(nsta >= 6) {
			event.build_info.push_back(build_info);
			goto DONE;
		    }
		}
		event.build_info.push_back(build_info);
	    }
	  // repeat as long as a new trigger is added and have < 10 stations
	} while(num_event_trigs > num && sta_trigs.size() < 10);

    DONE:

	if((int)sta_trigs.size() >= multiStaMinTrigs) { // number of stations triggered

	    if(!located) {
		LocateInfo loc_info(2);
		loc_info.init(event);
		if(loc->locate(&event, loc_info) != 0) {
		    event.loc_info.push_back(loc_info);
		    located = true;
		    evlat = event.lat;
		    evlon = event.lon;
		    evdepth = event.depth;
		    evtime = event.time;
		    misfit = event.misfit_ave;
		}
		else { // if location fails, continue with centroid location
		    event.lat = evlat;
		    event.lon = evlon;
		    event.depth = evdepth;
		    event.time = evtime;
		    event.misfit_ave = 0.; // compute actual misfit from location
		}
	    }
	    event.nsta_build = sta_trigs.size();
	    event.ntrig_build = event.triggers.size();
	    event.nS = event.nsta_build;
	    event.nT = event.ntrig_build;
	    event.located = located;
	    event.lat = evlat;
	    event.lon = evlon;
	    event.depth = evdepth;
	    event.time = evtime;

	    event.misfit_ok = (event.misfit_ave < maxCreateMisfit);
	    event.split_ok = !splitEvent(evlat, evlon, evtime);
	    event.percent_ok = countNeighbors(&event, &event.cn);
	    event.percent = event.cn.percent;
	    event.computeAzimuthError();
	    double min_dist = 0., max_dist = 0.;
	    for(int j = 0; j < num_event_trigs; j++) {
		dist = distancekm(trigs[j]->getLat(), trigs[j]->getLon(), evlat, evlon);
		if(j == 0 || dist < min_dist) min_dist = dist;
		if(dist > max_dist) max_dist = dist;
	    }
	    event.dist_ok = (min_dist < E2Magnitude::maxMagKm(&event));

//	    if(event.misfit_ok && event.split_ok && event.percent_ok && event.dist_ok)
	    if(event.misfit_ok && event.split_ok && event.percent_ok)
	    {
		event.triggers.clear();

		// new event is not in blackout zone and trigStaPercent neighbors triggering is satisfied
		E2Event *ev = em->insertEvent(MULTI_STATION_EVENT, evtime, evlat, evlon, evdepth);
		event.eventid = ev->eventid;
	 	*(EventCore *)ev = (EventCore)event;
		ev->eventid = event.eventid;

		for(int j = 0; j < num_event_trigs; j++) {
		    ev->addTrigger(trigs[j], MULTI_STA);
		}

		ev->cn = event.cn;

		ev->build_info.insert(ev->build_info.begin(), event.build_info.begin(), event.build_info.end());
		for(list<BuildInfo>::iterator it = ev->build_info.begin(); it != ev->build_info.end(); it++) {
		    (*it).eventid = ev->eventid;
		}
		ev->loc_info.insert(ev->loc_info.begin(), event.loc_info.begin(), event.loc_info.end());
		for(list<LocateInfo>::iterator it = ev->loc_info.begin(); it != ev->loc_info.end(); it++) {
		    (*it).eventid = ev->eventid;
		}
		*eventid = ev->eventid;

		ev->misfit_ave = misfit;
		ev->algorithm = (loopforward) ? 1 : 2;
		return true;
	    }
	    else {
		printTrialEvent(event, num_event_trigs);
	    }
	}
    }
    event.triggers.clear();

    return false;
}

void E2Associator::printTrialEvent(E2Event &event, int num_event_trigs)
{
    bool new_event = true;
    // check for repeated events
    for(deque<TrialEvent>::iterator it = trial_events.begin(); it != trial_events.end(); it++) {
	double distkm = distancekm(event.lat, event.lon, (*it).evlat, (*it).evlon);
	double tdif = fabs(event.time - (*it).evtime);
	if(distkm < 50.0 && tdif < 10.) {
	    new_event = false;
	    break;
	}
    }

    if(new_event) {
	event.eventid = E2Event::getTmpId();
	for(list<BuildInfo>::iterator it = event.build_info.begin(); it != event.build_info.end(); it++) {
	    (*it).eventid = event.eventid;
	}
	event.cn.eventid = event.eventid;
	for(map<int, CountInfo>::iterator it = event.cn.stas.begin(); it != event.cn.stas.end(); it++) {
	    (*it).second.eventid = event.eventid;
	}

	for(list<LocateInfo>::iterator it = event.loc_info.begin(); it != event.loc_info.end(); it++) {
	    (*it).eventid = event.eventid;
	}
	event.triggers.clear();
	// add triggers just for logging
	for(int i = 0; i < num_event_trigs; i++) {
	    event.addTrigger(trigs[i], MULTI_STA);
	}

	E2Magnitude::calcMag(&event);
	
	event.tele_fbands = 2;
	E2Alert::checkAlertCriteria(&event, tm);
	event.alert_ok = false;

	event.alert_time = TimeStamp::current_time().ts_as_double(UNIX_TIME);

	TrialEvent e;
	e.evlat = event.lat;
	e.evlon = event.lon;
	e.evtime = event.time;
	e.eventid = event.eventid;

	trial_events.push_back(e);

	string log_time = E2Event::logTime();
	stringstream msg_str;

	msg_str << E2Event::logTime() << endl;

	list<string> l = event.toString();
	for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << "E:" << (*it) << endl;

	if(event.percent > logPercentMin) {
	    l = event.printBuildInfo();
	    for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << (*it) << endl;

	    for(list<LocateInfo>::iterator jt = event.loc_info.begin(); jt != event.loc_info.end(); jt++) {
		l = (*jt).toString();
		for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << "L:" << (*it) << endl;
	    }
	}
	E2ModuleManager::sendLogMsg(msg_str.str());
	if(verbose >= 1) {
	    LOG_INFO << msg_str.str();
	}

	for(int i = 0; i < num_event_trigs; i++) event.removeTrigger(trigs[i], true);
    }
    // discard trial_events that are ten minutes earlier than input event
    while(trial_events.size() > 0 && event.time - trial_events.front().evtime > 600) {
	trial_events.pop_front();
    }
}

bool E2Associator::twoStationEvent(int num_triggers, E2Trigger **triggers, int *eventid)
{
    double evlat, evlon, evtime;
    double maxlatdif = maxTwoStaDist/111.19492664;

    if( !useTwoStations ) return false;

    // flag unassociated triggers that fail two-station taup and Pd limits
    for(int i = 0; i < num_triggers; i++) {
	E2Trigger *t = triggers[i];
	// assumes twoStaMinTP >= minTP && twoStaMinPD >= minPD
	t->used = (t->isAssociated() || (t->getTaup() > 0. && t->getLogTaup() < twoStaMinTP)
			|| t->getLogPd() < twoStaMinPD);
    }

    for(int i = 0; i < num_triggers; i++) if( !triggers[i]->used )
    {
	E2Trigger *t = triggers[i];

	for(int j = 0; j < num_triggers; j++)
	    if( !triggers[j]->used && triggers[j] != triggers[i] && triggers[j]->getSta() != triggers[i]->getSta() )
	{
	    E2Trigger *r = triggers[j];

	    if(fabs(r->trigger_time - t->trigger_time) <= maxTwoStaDist/pVelocity &&
		    fabs(r->getLat() - t->getLat()) < maxlatdif &&
		    distancekm(r->getLat(), r->getLon(), t->getLat(), t->getLon()) <= maxTwoStaDist)
	    {
		if(t->trigger_time < r->trigger_time) {
		    evlat = (t->getLat()*2 + r->getLat()) / 3;
		    evlon = (t->getLon()*2 + r->getLon()) / 3;
		    evtime = t->trigger_time;
		}
		else {
		    evlat = (r->getLat()*2 + t->getLat()) / 3;
		    evlon = (r->getLon()*2 + t->getLon()) / 3;
		    evtime = r->trigger_time;
		}

		E2Event event;
		event.eventid = 0;
		event.lat = evlat;
		event.lon = evlon;
		event.depth = 8.0;
		event.time = evtime;
		event.triggers.clear();
		event.addTrigger(r, UNASSOCIATED);
		event.addTrigger(t, UNASSOCIATED);
		event.nsta_build = 2;
		event.ntrig_build = 2;
		event.nS = event.nsta_build;
		event.nT = event.ntrig_build;
		event.located = false;
		LocateInfo loc_info(2);
		loc_info.init(event);
		loc->locate(&event, loc_info);
		BuildInfo build_info(event, 1, 1, t, r);
		build_info.evtime = evtime;
		build_info.evlat = evlat;
		build_info.evlon = evlon;
		build_info.tt = fabs(r->trigger_time - t->trigger_time);
		build_info.tt_ok = true;
		build_info.dist = distancekm(r->getLat(), r->getLon(), t->getLat(), t->getLon());
		build_info.dist_ok = true;
		event.build_info.push_back(build_info);

		event.split_ok = !splitEvent(evlat, evlon, evtime);
		event.percent_ok = countNeighbors(&event, &event.cn);
		event.triggers.clear();

		if( event.split_ok && event.percent_ok) {
		    E2Event *ev = em->insertEvent(TWO_STATION_EVENT, evtime, evlat, evlon, 0.);
		    event.eventid = ev->eventid;
		    *(EventCore *)ev = (EventCore)event;
		    ev->eventid = event.eventid;

		    ev->cn = event.cn;
		    ev->build_info.insert(ev->build_info.begin(), event.build_info.begin(), event.build_info.end());
		    ev->addTrigger(t, TWO_STATN);
		    ev->addTrigger(r, TWO_STATN);
		    t->used = true;
		    r->used = true;
		    ev->percent = ev->cn.percent;
		    *eventid = ev->eventid;
		    ev->algorithm = 3;
		    return true;
		}
	    }
	}
    }
    return false;
}

bool E2Associator::singleStationEvent(int num_triggers, E2Trigger **triggers, int *eventid)
{
    E2Event *event;

    if( !useSingleStation ) return false;

    // assumes singleStaMinTP >= minTP && singleStaMinPD >= minPD
    for(int i = 0; i < num_triggers; i++)
    {
	E2Trigger *t = triggers[i];

	if(!t->isAssociated() && t->getTaup() > 0. && t->getLogTaup() >= singleStaMinTP
		&& t->getLogPd() >= singleStaMinPD
		&& !splitEvent(t->getLat(), t->getLon(), t->trigger_time) )
	{
	    event = em->insertEvent(SINGLE_STATION_EVENT, t->trigger_time, t->getLat(), t->getLon(), 0.);
	    event->addTrigger(t, ONE_STATN);
	    event->depth = 8.0;
	    *eventid = event->eventid;
	    LocateInfo loc_info(2);
	    loc_info.init(*event);
	    loc->locate(event, loc_info);
	    event->split_ok = true;
	    event->algorithm = 4;
	    event->nsta_build = 1;
	    event->ntrig_build = 1;
	    event->nS = event->nsta_build;
	    event->nT = event->ntrig_build;
	    event->located = false;
	    return true;
	}
    }
    return false;
}

bool E2Associator::countNeighbors(E2Event *event, CountNeighbors *cn)
{
    // Check the % of stations triggering within maxdist of the source
    set<int> triggered_stas;

    cn->eventid = event->eventid;
    cn->version = event->version;
    cn->counted = true;
    cn->evlat = event->lat;
    cn->evlon = event->lon;
    cn->evtime = event->time;
    cn->mindist = 1.e+60;
    cn->maxdist = 0.;

    cn->stas.clear();
    for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++)
    {
	triggered_stas.insert((*it)->staId());
	(*it)->distkm = distancekm(event->lat, event->lon, (*it)->getLat(), (*it)->getLon());
	if((*it)->distkm < cn->mindist)  cn->mindist = (*it)->distkm;
	if((*it)->distkm > cn->maxdist)  cn->maxdist = (*it)->distkm;
    }

    // block E2AMQReader from changing station_array or changing tm->clusters during the count
    pthread_mutex_lock(&tm->stations_lock);

    // Collect active clusters. If the cluster center is greater than cluster.min_distkm from the event location,
    // count the cluster stations as one station.
    vector<Cluster> clusters;
    for(map<string, Cluster>::iterator jt = tm->clusters.begin(); jt != tm->clusters.end(); jt++) {
	double distkm = distancekm(event->lat, event->lon, (*jt).second.center_lat, (*jt).second.center_lon);
	if(distkm > (*jt).second.min_distkm) { // the cluster center is far enough from the event to act as a cluster
	    // if any trigger is a member of this cluster, save the cluster.
	    for(set<int>::iterator it = triggered_stas.begin(); it != triggered_stas.end(); it++) {
		if((*jt).second.sta_ids.find(*it) != (*jt).second.sta_ids.end()) {
		    clusters.push_back((*jt).second);
		    break;
		}
	    }
	}
    }

    // Get all active stations within maxdist of the event location.
    // Split station list into four groups and count in four threads.
    count_thread[0].ibeg = 0;
    count_thread[0].iend = tm->num_stations/4.;
    count_thread[1].ibeg = count_thread[0].iend + 1;
    count_thread[1].iend = tm->num_stations/2.;
    count_thread[2].ibeg = count_thread[1].iend + 1;
    count_thread[2].iend = 3*tm->num_stations/4.;
    count_thread[3].ibeg = count_thread[2].iend + 1;
    count_thread[3].iend = tm->num_stations-1;

    for(int i = 0; i < 4; i++) {
	count_thread[i].eventid = event->eventid;
	count_thread[i].version = event->version;
	count_thread[i].evkm = cn->maxdist;
	count_thread[i].evlat = event->lat;
	count_thread[i].evlon = event->lon;
	count_thread[i].evtime = event->time;
	count_thread[i].count_info.clear();
	pthread_create(&count_thread[i].id, NULL, CountStations, (void *)&count_thread[i]);
    }

    // wait for threads to finish
    for(int i = 0; i < 4; i++) {
	pthread_join(count_thread[i].id, NULL);
	for(map<int, CountInfo>::iterator it = count_thread[i].count_info.begin(); it != count_thread[i].count_info.end(); it++) {
	    // add the CountInfo objects from this thread to the CountNeighbors map<string, CountInfo>
	    if(triggered_stas.find((*it).first) != triggered_stas.end()) (*it).second.triggered = true;
	    cn->stas[(*it).first] = (*it).second;
	}
	count_thread[i].count_info.clear();
    }

    pthread_mutex_unlock(&tm->stations_lock);

    // [ find triggered clusters. A cluster counts as one trigger if 40% of near-cluster stations have triggered ]
    // find triggered clusters. A cluster counts as one trigger if any of near-cluster stations have triggered
    for(vector<Cluster>::iterator jt = clusters.begin(); jt != clusters.end(); jt++)
    {
	// count the number of near stations that are in the cluster and the number that triggered
	int num_cluster_near = 0;
	int num_cluster_triggers = 0;
	for(map<int, CountInfo>::iterator it = cn->stas.begin(); it != cn->stas.end(); it++) {
	   // if this near station is active and a member of the jt cluster
	   if((*it).second.active && (*jt).sta_ids.find((*it).first) != (*jt).sta_ids.end()) {
		num_cluster_near++;
		if((*it).second.triggered) num_cluster_triggers++;
	    }
	}

	if(num_cluster_near > 0) {
//	    bool cluster_triggered = ((double)num_cluster_triggers/(double)num_cluster_near >= 0.40);
	    for(map<int, CountInfo>::iterator it = cn->stas.begin(); it != cn->stas.end(); it++) {
		if((*jt).sta_ids.find((*it).first) != (*jt).sta_ids.end()) {
		    (*it).second.in_cluster = true; // station is in this cluster
		    (*it).second.cluster_id = (*jt).cluster_id;
//		    (*it).second.cluster_triggered = cluster_triggered;
		    (*it).second.cluster_triggered = true;
		}
	    }
	}
    }

    cn->num_active = 0;
    cn->num_inactive = 0;
    triggered_stas.clear();
    set<int> near_stas;
    for(map<int, CountInfo>::iterator it = cn->stas.begin(); it != cn->stas.end(); it++) {
	if((*it).second.active) {
	    cn->num_active++;
	    if((*it).second.in_cluster) {
		near_stas.insert((*it).second.cluster_id);
		if((*it).second.cluster_triggered) triggered_stas.insert((*it).second.cluster_id);
	    }
	    else {
		near_stas.insert((*it).second.sta_id);
		if((*it).second.triggered) triggered_stas.insert((*it).second.sta_id);
	    }
	}
	else {
	    cn->num_inactive++;
	}
    }

    cn->num_near = near_stas.size();
    cn->num_sta_trig = triggered_stas.size();

    if(cn->num_near > 0 && cn->num_sta_trig > 0) {
        cn->percent = 100.*(double)cn->num_sta_trig/(double)cn->num_near;
        return (cn->num_sta_trig >= cn->num_near*trigStaPercent);
    }
    else {
        cn->percent = 0;
        return false;
    }
}

static void *
CountStations(void *client_data)
{
    CountThread *t = (CountThread *)client_data;
    t->parent->countStations(t);
    return NULL;
}

void E2Associator::countStations(CountThread *t)
{
    CountInfo ci;
    double lat, lon, distkm, maxlatdif;
    double evkm = t->evkm;

    evkm += .1; // make sure to count the farthest station
    maxlatdif = evkm/111.19492664;

    // count number of active stations within evkm of evlat,evlon
    for(int i = t->ibeg; i <= t->iend; i++)
    {
	lat = tm->station_array[i]->lat;
	lon = tm->station_array[i]->lon;
	if(fabs(lat - t->evlat) <= maxlatdif) {
	    // station's distance to epicenter
	    distkm = distancekm(t->evlat, t->evlon, lat, lon);
	    if(distkm <= evkm) {
		double tt = distkm/pVelocity;
		ci.eventid = t->eventid;
		ci.version = t->version;
		ci.sta = tm->station_array[i]->sta;
		ci.net = tm->station_array[i]->net;
		ci.lat = tm->station_array[i]->lat;
		ci.lon = tm->station_array[i]->lon;
		ci.sta_id = tm->station_array[i]->sta_id;
		ci.dist = distkm;
		ci.tt = tt;
		ci.last_sample_time = tm->station_array[i]->last_data_time;
		// A station is considered inactive for the current event, if there has been no data since
		// maxStationDelay seconds before the expected P arrival from the current event

		double tdif = tm->station_array[i]->last_data_time - (t->evtime + tt - maxStationDelay);
		ci.time_check = tdif;
		ci.active = false;
		if(!useGMPeak || tm->station_array[i]->last_data_time > t->evtime + tt - maxStationDelay ||
			tm->station_array[i]->replay_count)
		{
		    ci.active = true;
		}
		t->count_info[ci.sta_id] = ci;
	    }
	}
    }
}

bool E2Associator::splitEvent(double evlat, double evlon, double evtime)
{
    EventList *evlist = em->getEventList();
    EventList::iterator ev;
    for(ev = evlist->begin(); ev != evlist->end(); ev++) {
	double dist = distancekm(evlat, evlon, (*ev)->lat, (*ev)->lon);
	if(fabs(evtime-(*ev)->time) <= blackoutTime && dist <= blackoutDist) break;
    }
    return (ev != evlist->end());
}

bool E2Associator::splitEvent(int eventid, double evlat, double evlon, double evtime)
{
    EventList *evlist = em->getEventList();
    EventList::iterator ev;
    for(ev = evlist->begin(); ev != evlist->end(); ev++) {
	if((*ev)->eventid != eventid) {
	    double dist = distancekm(evlat, evlon, (*ev)->lat, (*ev)->lon);
	    if(fabs(evtime-(*ev)->time) <= blackoutTime && dist <= blackoutDist) break;
	}
    }
    return (ev != evlist->end());
}
