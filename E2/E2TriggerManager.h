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
#ifndef _E2TriggerManager_H__
#define	_E2TriggerManager_H__

#include <time.h>
#include <pthread.h>
#include <set>
#include <vector>
#include <math.h>
#include <sstream>
#include <activemq/core/ActiveMQConnection.h>
#include "Exceptions.h"

#define USE_STATION_FILE
#ifndef USE_STATION_FILE
#include "Database.h"
#endif
#include "TriggerParams.h"
#include "E2Trigger.h"
#include "E2EventManager.h"
#include "E2Region.h"
#include "GetProp.h"

class E2Time;
class E2Reader;
class Cluster;

/**
 *  Trigger map
 *  @see E2Trigger
 */
typedef std::map<std::string, E2Trigger *> TriggerMap;

class Station
{
  public:
    std::string net;
    std::string sta;
    int sta_id;
    double lat;
    double lon;
    double last_data_time;
    bool replay_count;
    bool use;

    Station() : sta_id(0), lat(0.), lon(0.), last_data_time(0.), replay_count(false), use(true)  {}
    Station(std::string network, std::string station, double latitude, double longitude) :
	net(network), sta(station), sta_id(0), lat(latitude), lon(longitude), last_data_time(0.),
	replay_count(false), use(true)  {}
};

class E2TriggerManager
{
  private:
    TriggerMap trigmap;
    E2Reader *reader;
    bool new_input;
    bool useGMPeak;
    bool useTF;
    std::string tfFile;
    std::string amq_user;
    std::string amq_password;
    std::string trigger_amq_uri;
    std::string trigger_amq_topic;
    std::string station_topic;
    std::map<int, Station> stations;
    static std::map<std::string, int> netsta_map;
    static std::map<int, std::string> sta_id_map;
    static std::map<std::string, int> stachan_map;
    cms::Connection *trigger_connection;
    double pVelocity;
    double maxStationDelay;
    double teleseismic_secs;

    bool getColumns(char *line, std::vector<std::string> &columns) throw(Error);
    bool readLine(char *line, std::vector<std::string> &columns, std::string &net, std::string &sta, double *lat, double *lon);
    void updateStationArray();
    int getClusters();
    void testTeleseismicPgv(E2Trigger *t);
    void getTeleFBLimits(E2Trigger *t, double *min, double *max);
    void printTeleseismicPgv(std::stringstream &msg_str, E2Trigger *t);
    void printUpdateString(std::stringstream &msg_str, E2Trigger *t, char *chan);

  public:
    std::vector<E2Trigger *> updated_trigs;
    int num_stations;
    Station **station_array;
    std::map<std::string, Cluster> clusters;
    pthread_mutex_t stations_lock;
    pthread_mutex_t teleseisms_lock;
    int unassoc_trigger_timeout;
    bool stop;
    static bool replay_mode;
    static double tfMaxStaDist;
    static int tfMinWindowIndex;
    // test if above line: Pgv[tfLow] > tfM2*Pgv[tfHigh] + tfB2
    static int tfLow;
    static int tfHigh;
    static double tfM2;
    static double tfB2;
    static bool tfChannelReverse; // true: allow a channel to change from teleseismic to non-teleseismic
    				  // false: do not allow a channel to change from teleseismic to non-teleseismic

    E2TriggerManager(bool replay_mode) throw(Error);

    ~E2TriggerManager() {
	trigmap.clear();
    }

    std::map<int, Station>::iterator stationsFind(int sta_id) { return stations.find(sta_id); }
    std::map<int, Station>::iterator stationsBegin() { return stations.begin(); }
    std::map<int, Station>::iterator stationsEnd() { return stations.end(); }
    std::map<int, Station>::iterator addStation(std::string netsta, TriggerParams *tp);
    std::map<int, Station>::iterator addStation(std::string net, std::string sta, double lat, double lon);

    void updateClusters(Station &s);

#ifndef USE_STATION_FILE
    void queryForStations(Database &db, std::string channelset, std::map<int, Station> &stations) throw(Error);
#endif

    int read(int last_evid);

    E2Trigger *insertTrigger(TriggerParams &);

    /**
     * Remove Trigger older than current time minus 
     *          unassoc_trigger_timeout for unassociated Trigger.
     */
    void removeOldTriggers(E2EventManager *em);
    
    TriggerMap * getTrigMap() { return &trigmap; }

    void cleanup();

    bool newTrigger() { return new_input; }

    E2Reader *getReader() { return reader; }

    cms::Connection *getConnection() { return trigger_connection; }
    std::string getAmqUri() { return trigger_amq_uri; }

    bool isActiveStation(E2Event *event, int sta_id);

    void checkTeleseisms(void);

    void run();
    void stopReader();

    static int staId(std::string netsta);
    static int stachanId(std::string stachan);
    static std::string netsta(int sta_id);

    static double minTP;  // minimum log10(TauPmax) value for any event 
    static double maxTP;  // maximum log10(TauPmax) value for any event 
    static double minPD;  // minimum log10(Pd) value for any event 
    static double maxPD;  // maximum log10(Pd) value for any event 
    static double minPV;  // minimum log10(Pv) value for any event 
    static double maxPV;  // maximum log10(Pv) value for any event 
    static double minPA;  // minimum log10(Pa) value for any event 
    static bool zCrossVelAlways; // If true, count using velocity even when the channel is acceleration. If false, count vel or accel
    static int zCrossMin;  // minimum number of post-trig zero-crossings
    static double maxNEtoZ; // maximum N/Z or E/Z amplitude ratio

    // range_post_trig = maximum change in the length of the acceleration or velocity vector in the
    // time period from startRangePostTrig to endRangePostTrig seconds after the trigger
    // if range_post_trig < minRangePostTrig, do not use the trigger
    // if rangeAccelAlways, then use acceleration always. Otherwise use velocity for HZ channels and acceleration for NZ channels
    static bool rangeAccelAlways;
    static double minRangePostTrigAccel;
    static double minRangePostTrigVel;
};

class Cluster
{
  public:
    std::set<int> sta_ids;
    int cluster_id;
    std::string name;
    double center_lat;
    double center_lon;
    double radiuskm;
    double min_distkm;
    Cluster(std::string cluster_name) : name(cluster_name), center_lat(0.), center_lon(0.),
		radiuskm(0.), min_distkm(0.)
    {
	cluster_id = E2TriggerManager::staId(cluster_name);
    }
    Cluster() {}
    bool containsSta(int sta_id) {
	return (sta_ids.find(sta_id) != sta_ids.end()) ? true : false;
    }
};

#endif
