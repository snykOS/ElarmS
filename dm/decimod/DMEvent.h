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
#ifndef __DMEvent_h
#define __DMEvent_h

#include <vector>
#include <sstream>
#include "GMMessage.h"
#include "AlgorithmEvent.h"

/** A calculated set of combined magnitude, location, origin time estimates,
 *  and uncertainties for the contributing algorithm events.
 *  @ingroup decimod
 */
class DMResult
{
  public:
    double mag;		//!< combined magnitude estimation
    double mag_uncer;   //!< combined magnitude uncertainty
    double lat;		//!< combined latitude estimation
    double lat_uncer;	//!< combined latitude uncertainty
    double lon;		//!< combined longitude estimation
    double lon_uncer;	//!< combined longitude uncertainty
    double depth;	//!< combined depth estimation
    double depth_uncer;	//!< combined depth uncertainty
    double otime;	//!< combined origin time estimation
    double otime_uncer;	//!< combined origin time uncertainty
    double lklhd;	//!< combined likelihood
    double num_stations;//!< combined number_stations
    double misfit;	//!< total misfit
    double max_dist_km; //!< maximum algorithm to combined location distance
    double max_delta_time; //!< maximum absolute value algorithm.time - otime

    DMResult() :
	mag(-9.9),
	mag_uncer(-9.9),
	lat(-999.9),
	lat_uncer(-999.9),
	lon(-999.9),
	lon_uncer(-999.9),
	depth(-9.9),
	depth_uncer(-9.9),
	otime(-99999.99),
	otime_uncer(-9.9),
	lklhd(-9.9),
	num_stations(-9.9),
	misfit(-9.9),
	max_dist_km(-9.9),
	max_delta_time(-9.9) { }
};

/** A subclass of DMMessage that contains a list contributing AlgorithmEvents and methods for computing
 *  the combined event attributes.
 * @ingroup decimod
 */
class DMEvent : public GMMessage
{
  private:
    vector<AlgorithmEvent> algo_events; //!< Contributing algorithm event keys
    /** The previously contributing keys. This vector is updated as events are
     *  added by addAlgoEvent. It is not changed when events are removed.
     */
    vector<AlgorithmEvent> previous_algo_events;
    /* pointer to the DecisionModule object's AlgoMap algo_messages */
    AlgoMap *algo_map;
    double misfit; //!< The last computed misfit value
    /** The maximum absolute value time difference between the contributing algo_events and the
     *  combined event time.
     */
    double max_delta_time;
    /** The maximum distance(km) between the contributing algo_events and the combined event location.
     */
    double max_dist_km;
    /** If this flag is true, the event will be considered for publishment by
     *  DecisionModule::publishDMEvents. It might not be published, if the DecisionModule
     *  publish_only_when_changed flag is true and there have not been any changes (larger
     *  than the change_threshold) to the CoreEventInfo values and no changes to the
     *  observation or prediction lists since the last publishment.
     */
    int debug;
    bool publish_flag;
    double time_published; //!< The system time that the event was last published
    bool published_one; //!< At least one alert message has been sent.
    DMResult published; //!< The last published values
    DMResult current;   //!< The last computed values
    DMResult change;
    /** The absolute value difference between published and current that will force
      * a republish of the event.
      * <pre>if |current.value - published.value| >= change_threshold for any value, publish again.</pre>
      */
    DMResult change_threshold;

    void calculateComboValues();
    double individualAlgoMisfit(AlgorithmEvent algo, double *delta_time, double *dist_km, bool verbose=false);

    double totalDMEventMisfit(bool verbose=false);
    double recalculateDMEvent(bool verbose=false);
    string algoString(AlgorithmEvent &algo);

    double max_distkm_to_event;
    double max_time_to_event;

    stringstream &os;

  public:
    DMEvent(enum MessageCategory category, enum eewSystemName system_name, string id, AlgoMap *,
                DMConstants &constants, stringstream &os, int dbg);
    virtual ~DMEvent();
    
    void addAlgoEvent(AlgorithmEvent key, bool temporary=false);
    bool removeAlgoEvent(AlgorithmEvent key);
    void removeAllAlgoEvents();
    double update();
    bool contains(AlgorithmEvent algo);
    bool previouslyContained(AlgorithmEvent algo);
    double getMisfit();
    double getMaxDeltaTime() { return max_delta_time; }
    double getMaxDistKm() { return max_dist_km; }
    bool checkNewAlgoMisfit(AlgorithmEvent algo, double *check_misfit,
				double *check_delta_time, double *check_dist_km);
    int numAlgoEvents() { return (int)algo_events.size(); }
    vector<AlgorithmEvent> getAlgoEvents() { return algo_events; }
    map<string, int> forward_version;
    void publish(double publish_time);
    bool aboveThreshold();
    bool noChange();
    void setChangeThreshold(DMResult tol);
    string contribString();
    bool loadAlgExtra(double fault_info_min_mag);
    /** If this flag is true, the event will be considered for publishment by
     *  DecisionModule::publishDMEvents. It might not be published, if the DecisionModule
     *  publish_only_when_changed flag is true and there have not been any changes (larger
     *  than the change_threshold) to the CoreEventInfo values and no changes to the
     *  observation or prediction lists since the last publishment.
     */
    void setPublishFlag(bool set) { publish_flag = set; }
    bool getPublishFlag() { return publish_flag; }
    double getTimePublished()  { return time_published; }
    bool deleted() { return (numAlgoEvents() == 0); }
    bool publishedOne() { return published_one; }
    bool containsCoreEventInfo();
    bool passedTwoAlgMag(double two_alg_mag);
    string changeString();
    AlgorithmEvent *getContributor(enum eewSystemName system_name);
    string previous_message;
    int previous_num_contributors;
    int contour_ver_published;
    int map_ver_published;
    bool publish_next_contour;
    bool publish_next_map;
};

#endif
