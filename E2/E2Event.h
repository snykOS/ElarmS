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
#ifndef _E2Event_H__
#define	_E2Event_H__

#include <set>
#include <list>
#include <map>
#include "EventCore.h"
#include "E2Trigger.h"
#include "Exceptions.h"

class E2Event;

typedef std::list<E2Trigger *> TriggerList;

class CountInfo
{
  public:
    int eventid;
    int version;
    std::string sta;
    std::string net;
    double lat;
    double lon;
    int cluster_id;
    int sta_id;
    double dist;
    double tt;
    double last_sample_time;
    double time_check;
    bool triggered;
    bool in_cluster;
    bool cluster_triggered;
    bool active;

    CountInfo() : eventid(0), version(0), lat(0.), lon(0.), cluster_id(0), sta_id(0),
        dist(0.), tt(0.), last_sample_time(0.), time_check(0.), triggered(false),
        in_cluster(false), cluster_triggered(false), active(false) {}

    std::string toString();
};

class CountNeighbors
{
  public:
    int eventid;
    int version;
    bool counted;
    double evlat;
    double evlon;
    double evtime;
    double mindist;
    double maxdist;
    double percent;
    int num_near;
    int num_sta_trig;
    int num_active;
    int num_inactive;
    std::map<int, CountInfo> stas;

    CountNeighbors() : eventid(0), version(0), counted(false), evlat(0.), evlon(0.), evtime(0.), mindist(0.),
        maxdist(0.), percent(0.), num_near(0), num_sta_trig(0), num_active(0), num_inactive(0) {}
    std::list<std::string> toString();
};

class LocateTrigger
{
  public:
    int eventid;
    int version;
    std::string net;
    std::string sta;
    std::string chan;
    std::string loc;
    int stachan_id;
    double lat;
    double lon;
    double time;
    bool used;
    double dist;
    double tt;
    double tterr;
    bool tele_fbands;
    LocateTrigger(E2Trigger *t, int version);
};

class LocateInfo
{
  public:
    int eventid;
    int version;
    int src;
    int depth_kmin;
    double initial_lat;
    double initial_lon;
    double initial_depth;
    double initial_time;
    double bestex;
    double bestey;
    double lat;
    double lon;
    double depth;
    double time;
    double misfit_ave;
    double misfit_rms;
    bool printed;
    std::list<LocateTrigger> trigs;
    LocateInfo() : eventid(0), version(0), src(0), depth_kmin(0), initial_lat(0.), initial_lon(0.), initial_depth(0.),
	    initial_time(0.), bestex(0.), bestey(0.), lat(0.), lon(0.), depth(0.), time(0.),
	    misfit_ave(0.), misfit_rms(0.), printed(false) {}
    LocateInfo(int i) : eventid(0), version(0), src(i), depth_kmin(0), initial_lat(0.), initial_lon(0.), initial_depth(0.),
	    initial_time(0.), bestex(0.), bestey(0.), lat(0.), lon(0.), depth(0.), time(0.),
	    misfit_ave(0.), misfit_rms(0.), printed(false) {}
    void init(E2Event &event);
    std::list<std::string> toString();
};

class BuildInfo
{
  public:
    int eventid;
    int loop;
    int index;
    std::string t_net;
    std::string t_sta;
    std::string t_chan2;
    std::string t_loc;
    double t_lat;
    double t_lon;
    double t_time;
    std::string r_net;
    std::string r_sta;
    std::string r_chan2;
    std::string r_loc;
    double r_lat;
    double r_lon;
    double r_time;

    double evtime;
    double evlat;
    double evlon;
    double evdepth;
    double tt;
    double dist;
    bool tt_ok;
    bool dist_ok;
    std::string sta_max;
    double sta_max_dist;
    double sta_max_tterr;
    bool sta_max_tterr_ok;
    bool include_trigger;
    int nsta, new_nsta;
    int ntrig;
    bool located;
    double misfit_ave;
    double misfit_rms;
    CountNeighbors cn;

    BuildInfo() : eventid(0), loop(0), index(0), t_lat(0.), t_lon(0.), t_time(0.), r_lat(0.), r_lon(0.), r_time(0.),
	evtime(0.), evlat(0.), evlon(0.), evdepth(0.), tt(0.), dist(0.), tt_ok(false), dist_ok(false),
	sta_max_dist(0.), sta_max_tterr(0.), sta_max_tterr_ok(false), include_trigger(false),
	nsta(0), new_nsta(0), ntrig(0), located(false), misfit_ave(0.), misfit_rms(0.) {}
    BuildInfo(E2Event &, int, int, E2Trigger *, E2Trigger *);

    std::string startString();
    std::list<std::string> toString();
};

class AssocInfo
{
  public:
    int eventid;
    int version;
    double lat;
    double lon;
    double depth;
    std::string trig_sta;
    std::string trig_chan;
    std::string trig_net;
    std::string trig_loc;
    double trig_lat;
    double trig_lon;
    double trig_time;
    double tt;
    double dist;
    bool tt_ok;
    bool dist_ok;
    bool near_tt_ok;
    bool near_dist_ok;
    double ttmin;
    double ttmax;
    bool relocate;
    double new_lat;
    double new_lon;
    double new_depth;
    double new_time;
    double new_misfit_rms;
    double new_misfit_ave;
    double min_dist;
    bool mag_dist_ok;
    bool misfit_ok;
    bool percent_ok;
    bool printed;
    CountNeighbors cn;
    bool logged;

    AssocInfo(E2Event *ev, E2Trigger *t, double evtimediff, double evkm);
    AssocInfo();
    void newLocation(E2Event *ev);

    std::list<std::string> toString(bool print_header=true);
};

class E2Event : public EventCore
{
  private:
    static int tmp_id;

  public:
    CountNeighbors cn;

    std::list<EventCore> history;
    std::set<E2Trigger *, trigger_compare> triggers;
    std::list<BuildInfo> build_info;
    std::list<AssocInfo> assoc_info;
    std::map<long, AssocInfo> failed_assoc_info;
    std::list<LocateInfo> loc_info;

    static bool get_properties;
    static bool send_email;
    static std::string email_to;
    static std::string mailx;
    static double logPercentMin;

    /**
     * A general constructor.
     */
    E2Event(int id, EvType evtype, double time, double latitude, double longitude, double depth);

    E2Event(int id, double evtime, double evlat, double evlon) {
	eventid = id; 
	time = evtime;
	lat = evlat;
	lon = evlon;
    }

    E2Event() {}

    E2Event(const std::string &line) throw(Error);

    static void getProperties();
    void load(const std::string &line) throw(Error);
    void sendEmail();
    std::list<std::string> printBuildInfo();

    ~E2Event();

    /**
     * Insert a Trigger into Trigger list
     * @param  trigger Pointer of E2Trigger.
     */
    void addTrigger(E2Trigger *trigger, AssocCode assocCode);

    int getAlertMessageVersion() {
	int msg_version = 0;
	for( std::list<EventCore>::iterator it=history.begin(); it != history.end(); it++) {
	    if( (*it).alert_sent ) msg_version++;
	}
	return msg_version;
    }

    int incrementVersion(void);

    void removeTrigger(E2Trigger *, bool disassoc = true);

    /**
     * Clean up all of resource for destructor.
     * Un-associating Trigger were associated.
     * after then clear all of Trigger and Magnitude list.
     */
    void cleanup();

    int stationCount() {
        std::set<std::string> stations;
        for(std::set<E2Trigger *>::iterator it = triggers.begin(); it != triggers.end(); it++) {
            stations.insert((*it)->getSta());
        }
        return stations.size();
    }

    bool containsChannel(E2Trigger *t) {
	int stachan_id = t->staChanId();
	for(std::set<E2Trigger *>::iterator it = triggers.begin(); it != triggers.end(); it++) {
	    if((*it)->staChanId() == stachan_id) return true;
	}
	return false;
    }

    void setEventid(int eventid) { this->eventid = eventid; }

    void setTime(double time) { this->time = time; }
    
    void setLat(double lat) { this->lat = lat; }

    void setLon(double lon) { this->lon = lon; }

    void setDepth(double depth) { this->depth = depth; }

    void setLikelihood(double likelihood)  { this->likelihood = likelihood; }

    void computeAzimuthError();

    std::string toShortString();
    std::list<std::string> toString();

    void logMessages();

    static std::string toTimeString(double t, int decimals);
    static std::string nowToString(int decimals);
    static std::string logTime();
    static int getTmpId();
};

#endif
