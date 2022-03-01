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
#include <time.h>
#include <math.h>
#include <string.h>
#include <sstream>
#include <vector>
#include "DMEvent.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384;
#endif

class SystemAverage
{
  public:
    int num; // number of program instances with this system name
    double mag;
    double lat;
    double lon;
    double depth;
    double time;
    double lk;
    int nsta;

    double mag_uncer;
    double lat_uncer;
    double lon_uncer;
    double depth_uncer;
    double time_uncer;
};

static double distkm(double slat, double slon, double rlat, double rlon);

DMEvent::DMEvent(enum MessageCategory category, enum eewSystemName system_name, string id,
		AlgoMap *all_messages, DMConstants &constants, stringstream &ss, int dbg) :
		GMMessage(id), algo_map(all_messages), misfit(NULL_MISFIT), max_delta_time(0.),
                max_dist_km(0.), debug(dbg), publish_flag(false), time_published(-999999.),
		published_one(false), os(ss), previous_num_contributors(0),
                contour_ver_published(-1), map_ver_published(-1),
                publish_next_contour(false), publish_next_map(false)
{
    setID(id);
    setVersion(-1); // it is incremented before each send
    setMessageCategory(category);
    setSystemName(system_name);
    change_threshold.mag	  = constants.change_threshold_mag;
    change_threshold.mag_uncer	  = constants.change_threshold_mag_uncer;
    change_threshold.lat	  = constants.change_threshold_lat;
    change_threshold.lat_uncer	  = constants.change_threshold_lat_uncer;
    change_threshold.lon	  = constants.change_threshold_lon;
    change_threshold.lon_uncer	  = constants.change_threshold_lon_uncer;
    change_threshold.depth	  = constants.change_threshold_depth;
    change_threshold.depth_uncer  = constants.change_threshold_depth_uncer;
    change_threshold.otime	  = constants.change_threshold_otime;
    change_threshold.otime_uncer  = constants.change_threshold_otime_uncer;
    change_threshold.lklhd	  = constants.change_threshold_lklhd;
    change_threshold.num_stations = constants.change_threshold_num_stations;

    change_threshold.misfit = 0.;
    change_threshold.max_dist_km = 0.;
    change_threshold.max_delta_time = 0.;

    max_distkm_to_event = constants.max_distkm_to_event;
    max_time_to_event = constants.max_time_to_event;
}

DMEvent::~DMEvent() 
{
}

/** Return the current misfit value.
 */
double DMEvent::getMisfit() 
{
    return misfit;
}

/** Remove all of the associated algorithm events.
 */
void DMEvent::removeAllAlgoEvents()
{
    algo_events.clear();
    misfit = NULL_MISFIT;
}

/** Return true if the AlgorithmEvent argument is associated with this DMEvent.
 *  @param[in] algo an AlgorithmEvent key
 */
bool DMEvent::contains(AlgorithmEvent algo)
{
    size_t i;
    for(i = 0; i < algo_events.size() && algo_events[i] != algo; i++);
    return (i < algo_events.size());
}

/** Return true if the AlgorithmEvent argument was previously associated with this DMEvent.
 *  @param[in] algo an AlgorithmEvent key
 */
bool DMEvent::previouslyContained(AlgorithmEvent algo)
{
    size_t i;
    for(i = 0; i < previous_algo_events.size() && previous_algo_events[i] != algo; i++);
    return (i < previous_algo_events.size());
}

/** Add an AlgorithmEvent to (associate it with) this DMEvent. The misfit is recalculated.
 *  @param[in] algo an AlgorithmEvent key
 *  @param[in] temporary if true, do not update the previous_algo_events list.
 */
void DMEvent::addAlgoEvent(AlgorithmEvent algo, bool temporary)
{
    size_t i;
    // enforce same category
    if(algo.message_category != getMessageCategory()) {
	os << " DMEvent::addAlgoEvent called with category mismatch." << endl;
	return;
    }
    for(i = 0; i < algo_events.size() && algo_events[i] != algo; i++);
    if(i == algo_events.size()) {
	algo_events.push_back(algo);
	// update previous_algo_events after an event is added
	if( !temporary ) {
	    previous_algo_events.clear();
	    for(i = 0; i < algo_events.size(); i++) {
		previous_algo_events.push_back(algo_events[i]);
	    }
	}
    }
    recalculateDMEvent(temporary & debug);
}

/** Remove an AlgorithmEvent from (disassociate it from) this DMEvent.
 *  The misfit is recalculated.
 *  @param[in] algo an AlgorithmEvent key
 *  @returns true if algo is contained in this DMEvent or false if it is not.
 */
bool DMEvent::removeAlgoEvent(AlgorithmEvent algo)
{
    size_t i;
    for(i = 0; i < algo_events.size() && algo_events[i] != algo; i++);
    if(i < algo_events.size()) {
	algo_events.erase(algo_events.begin()+i);
	recalculateDMEvent();
	return true;
    }
    return false;
}

/** Recalculate the misfit and set the publish_flag to true.
 *  @returns the recomputed misfit value.
 */
double DMEvent::update()
{
    publish_flag = true;
    recalculateDMEvent();
    return misfit;
}

double DMEvent::individualAlgoMisfit(AlgorithmEvent algo, double *delta_time,
			double *dist_km, bool verbose)
{
    AlgoMapIter it = algo_map->find(algo);
    CoreEventInfo *cei = (*it).second;

    double alat = cei->getLatitude();
    double alon = cei->getLongitude();
    double dlat = getLatitude();
    double dlon = getLongitude(); 
    double misfit;

    *delta_time = fabs( cei->getOriginTime() - getOriginTime() );

    *dist_km = distkm(alat, alon, dlat, dlon);

    misfit = (*delta_time/max_time_to_event) + (*dist_km/max_distkm_to_event);

    if(verbose) {
	os << " " << algoString(algo) << ": "
		<< setprecision(2) << "sec:" << *delta_time << " km:" << *dist_km;
    }
    return misfit;
}

double DMEvent::totalDMEventMisfit(bool verbose)
{
    int num;
    double delta_time, dist_km;
    double total_misfit = 0.;

    max_delta_time = 0.;
    max_dist_km = 0.;

    num = 0;
    for(size_t i = 0; i < algo_events.size(); i++)
	if(algo_events[i].useCoreEventInfo() || algo_events.size() == 1) 
    {
	num++;
	total_misfit += individualAlgoMisfit(algo_events[i], &delta_time, &dist_km, verbose);
	if(delta_time > max_delta_time) max_delta_time = delta_time;
	if(dist_km > max_dist_km) max_dist_km = dist_km;
    }
    if(num > 0) {
	total_misfit = total_misfit/num;
    }
    current.misfit = total_misfit;
    current.max_delta_time = max_delta_time;
    current.max_dist_km = max_dist_km;
    return total_misfit;
}

void DMEvent::calculateComboValues()
{
    double mag_combo = 0;
    double mag_uncer_combo = 0;
    double lat_combo = 0;
    double lat_uncer_combo = 0;
    double lon_combo = 0;
    double lon_uncer_combo = 0;
    double dep_combo = 0;
    double dep_uncer_combo = 0;
    double o_time_e_combo = 0;
    double o_time_e_uncer_combo = 0;
    double lklhd_combo = 0;
    int num_stations_combo = 0;
    double temp_value, temp_uncer;
    map<enum eewSystemName, SystemAverage> system_average;
    map<enum eewSystemName, SystemAverage>::iterator is;
    SystemAverage ave;

    setMessageType(NEW);
    if(getVersion() > 0) {
	setMessageType(UPDATE);
    }
    
    // check for algo_events that have been removed
    for(int i = (int)algo_events.size()-1; i >= 0; i--) {
	if( algo_map->find(algo_events[i]) == algo_map->end() ) {
	    algo_events.erase(algo_events.begin()+i);
	}
    }

    if( algo_events.empty() ) {
	return;
    }
// uncertainties from the same system could be different, if they are based on a different number of stations.
// Weight within a system using uncertainties? Take best? Take one with most stations?
    // average events from the same system, unweighted 
    for(size_t i = 0; i < algo_events.size(); i++)
	if(algo_events[i].useCoreEventInfo() || algo_events.size() == 1) 
    {
	AlgoMapIter it = algo_map->find(algo_events[i]);
	CoreEventInfo *cei = (*it).second;

	if((is = system_average.find(algo_events[i].system_name)) != system_average.end()) {
	    // update the averages
	    int num = (*is).second.num;
	    (*is).second.mag = ((*is).second.mag*num + cei->getMagnitude())/(num+1);
	    (*is).second.lat = ((*is).second.lat*num + cei->getLatitude())/(num+1);
	    (*is).second.lon = ((*is).second.lon*num + cei->getLongitude())/(num+1);
	    (*is).second.depth = ((*is).second.depth*num + cei->getDepth())/(num+1);
	    if(cei->getLikelyhood() > (*is).second.lk) (*is).second.lk = cei->getLikelyhood();
	    if(cei->getNumberStations() > (*is).second.nsta) (*is).second.nsta = cei->getNumberStations();
	    (*is).second.num++;
	}
	else {
	    ave.num = 1; // number of program instances with this system name
	    ave.mag = cei->getMagnitude();
	    ave.lat = cei->getLatitude();
	    ave.lon = cei->getLongitude();
	    ave.depth = cei->getDepth();
	    ave.time = cei->getOriginTime();
	    ave.lk = cei->getLikelyhood();
	    ave.nsta = cei->getNumberStations();

	    ave.mag_uncer = cei->getMagnitudeUncertainty();
	    ave.lat_uncer = cei->getLatitudeUncertainty();
	    ave.lon_uncer = cei->getLongitudeUncertainty();
	    ave.depth_uncer = cei->getDepthUncertainty();
	    ave.time_uncer = cei->getOriginTimeUncertainty();

	    system_average[algo_events[i].system_name] = ave;
	}
    }

    // average events from different systems, weighted by 1/uncertainties
    int num = 0;
    for(is = system_average.begin(); is != system_average.end(); is++)
    {
	SystemAverage ave = (*is).second;

	num++;
	// Magnitude
	temp_value = ave.mag;
	temp_uncer = ave.mag_uncer;
	mag_combo += (temp_value/(temp_uncer*temp_uncer));
	mag_uncer_combo += (1/(temp_uncer*temp_uncer));
	// Latitude
	temp_value = ave.lat;
	temp_uncer = ave.lat_uncer;
	lat_combo += (temp_value/(temp_uncer*temp_uncer));
	lat_uncer_combo += (1/(temp_uncer*temp_uncer));
	// Longitude
	temp_value = ave.lon;
	temp_uncer = ave.lon_uncer;
	lon_combo += (temp_value/(temp_uncer*temp_uncer));
	lon_uncer_combo += (1/(temp_uncer*temp_uncer));
	// Depth
	temp_value = ave.depth;
	temp_uncer = ave.depth_uncer;
	dep_combo += (temp_value/(temp_uncer*temp_uncer));
	dep_uncer_combo += (1/(temp_uncer*temp_uncer));
	// Origin Time
	temp_value = ave.time;
	temp_uncer = ave.time_uncer;
	o_time_e_combo += (temp_value/(temp_uncer*temp_uncer));
	o_time_e_uncer_combo += (1/(temp_uncer*temp_uncer));
	// Likelihood
	if(ave.lk > lklhd_combo) lklhd_combo = ave.lk;
	// NumberStations
	if(ave.nsta > num_stations_combo) num_stations_combo = ave.nsta;
    }

    if(num == 0) return;
    
    //after accumulation is done:
    //1st: divide VALUE_combo by UNCER_combo
    //2nd: pow(-1/2) UNCER_combo
    //Magnitude
    current.mag = mag_combo/mag_uncer_combo;
    setMagnitude(current.mag);
    current.mag_uncer = pow(mag_uncer_combo, -0.5);
    setMagnitudeUncertainty(current.mag_uncer);
    //Latitude
    current.lat = lat_combo/lat_uncer_combo;
    setLatitude(current.lat);
    current.lat_uncer = pow(lat_uncer_combo, -0.5);
    setLatitudeUncertainty(current.lat_uncer);
    //Longitude
    current.lon = lon_combo/lon_uncer_combo;
    setLongitude(current.lon);
    current.lon_uncer = pow(lon_uncer_combo, -0.5);
    setLongitudeUncertainty(current.lon_uncer);
    //Depth
    current.depth = dep_combo/dep_uncer_combo;
    setDepth(current.depth);
    current.depth_uncer = pow(dep_uncer_combo, -0.5);
    setDepthUncertainty(current.depth_uncer);
    //Origin Time 
    current.otime = o_time_e_combo/o_time_e_uncer_combo;
    setOriginTime(current.otime);
    current.otime_uncer = pow(o_time_e_uncer_combo, -0.5);
    setOriginTimeUncertainty(current.otime_uncer);
    // Likelihood
    current.lklhd = lklhd_combo;
    setLikelyhood(current.lklhd);
    // Number of Stations
    current.num_stations = num_stations_combo;
    setNumberStations(num_stations_combo);
}

bool DMEvent::containsCoreEventInfo()
{
    for(size_t i = 0; i < algo_events.size(); i++) {
	if(algo_events[i].useCoreEventInfo()) return true;
    }
    return false;
}

double DMEvent::recalculateDMEvent(bool verbose) 
{
    calculateComboValues();
    misfit = totalDMEventMisfit(verbose);
    return misfit;
}

/** Compute the misfit, maxDeltaTime, and maxDistKm when the input algo contributes
 *  to the DMEvent.
 *  @param[in] algo an AlgorithmEvent key
 *  @param[out] check_misfit the misfit value with algo contributing to the DMEvent
 *  @param[out] check_delta_time the maximum delta_time with algo contributing to the DMEvent
 *  @param[out] check_dist_km the maximum dist_km with algo contributing to the DMEvent
 *  @returns false is the DMEvent already contains an algorithm event of the same type
 */
bool DMEvent::checkNewAlgoMisfit(AlgorithmEvent algo, double *check_misfit,
			double *check_delta_time, double *check_dist_km)
{
    size_t i;

    *check_misfit = 0.;
    *check_delta_time = 0.;
    *check_dist_km = 0.;

    // should not call with the wrong category
    if(algo.message_category != getMessageCategory()) {
	os << " DMEvent::checkNewAlgoMisfit called with category mismatch." << endl;
	return false;
    }

    // ensure that there is not already an algorithm event from the same system and program_instance
    for(i = 0; i < algo_events.size() && !(algo.system_name == algo_events[i].system_name
		&& algo.program_instance == algo_events[i].program_instance); i++);

    if( i < algo_events.size() ) {
	if(debug >= 1) {
	    os << " conflicts with " << algoString(algo_events[i]);
	}
	return false;
    }

    if(algo.useCoreEventInfo()) {
	// temporarily add this algo
	addAlgoEvent(algo, true);

	*check_misfit = getMisfit();
	*check_delta_time = getMaxDeltaTime();
	*check_dist_km = getMaxDistKm();

	// remove algo
	removeAlgoEvent(algo);
    }
    else { // algo will not contribute to DM attributes. Compute misfit from current origin
	AlgoMapIter it = algo_map->find(algo);
	CoreEventInfo *cei = (*it).second;

	double alat = cei->getLatitude();
	double alon = cei->getLongitude();
	double dlat = getLatitude();
	double dlon = getLongitude(); 

	*check_delta_time = fabs( cei->getOriginTime() - getOriginTime() );
	*check_dist_km = distkm(alat, alon, dlat, dlon);
	*check_misfit = *check_delta_time/max_time_to_event + *check_dist_km/max_distkm_to_event;
    }
    
    return true;
}

string DMEvent::contribString()
{
    stringstream comment;
    int version;

    for(size_t i = 0; i < algo_events.size(); i++) {
	AlgoMapIter it = algo_map->find(algo_events[i]);
	if( it == algo_map->end() ) { // should never happen
	    cerr << "DMEvent::contribString: cannot find AlgorithmEvent in map." << endl;
	    version = -1;
	}
	else {
	    version = (*it).second->getVersion();
	}
	// make shorter contrib string, since conlog truncates lines at 249 characters
	string alg = eewSystemNameString[algo_events[i].system_name].substr(0, 1);
	string cat = MessageCategoryString[algo_events[i].message_category].substr(0, 1);
	if(i > 0) comment << "-";
	comment << alg << ","
		<< algo_events[i].id << ","
		<< version << ","
		<< cat;
    }

    return comment.str();
}

string DMEvent::algoString(AlgorithmEvent &algo)
{
    stringstream comment;
    int version;

    AlgoMapIter it = algo_map->find(algo);
    if( it == algo_map->end() ) { // should never happen
	cerr << "DMEvent::algoString: cannot find AlgorithmEvent in map." << endl;
	version = -1;
    }
    else {
	version = (*it).second->getVersion();
    }
    comment << "[" << eewSystemNameString[algo.system_name] << ","
		<< algo.program_instance << ","
		<< algo.id << ","
		<< version << ","
		<< MessageCategoryString[algo.message_category] << "]";
    return comment.str();
}

bool DMEvent::aboveThreshold()
{
    if(!published_one) {
        // Nothing in the published structure yet
        return true;
    }
    change.mag 		= current.mag         - published.mag;
    change.mag_uncer 	= current.mag_uncer   - published.mag_uncer;
    change.lat 		= current.lat         - published.lat;
    change.lat_uncer 	= current.lat_uncer   - published.lat_uncer;
    change.lon 		= current.lon         - published.lon;
    change.lon_uncer 	= current.lon_uncer   - published.lon_uncer;
    change.depth 	= current.depth       - published.depth;
    change.depth_uncer  = current.depth_uncer - published.depth_uncer;
    change.otime 	= current.otime       - published.otime;
    change.otime_uncer 	= current.otime_uncer - published.otime_uncer;
    change.lklhd 	= current.lklhd       - published.lklhd;
    change.num_stations = current.num_stations- published.num_stations;

    change.misfit 	= current.misfit      - published.misfit;
    change.max_dist_km  = current.max_dist_km - published.max_dist_km;
    change.max_delta_time = current.max_delta_time - published.max_delta_time;

    return(
	fabs(change.mag)	 > change_threshold.mag ||
	fabs(change.mag_uncer)   > change_threshold.mag_uncer ||
	fabs(change.lat)	 > change_threshold.lat ||
	fabs(change.lat_uncer)   > change_threshold.lat_uncer ||
	fabs(change.lon)	 > change_threshold.lon ||
	fabs(change.lon_uncer)   > change_threshold.lon_uncer ||
	fabs(change.depth)	 > change_threshold.depth ||
	fabs(change.depth_uncer) > change_threshold.depth_uncer ||
	fabs(change.otime)	 > change_threshold.otime ||
	fabs(change.otime_uncer) > change_threshold.otime_uncer ||
	fabs(change.lklhd)       > change_threshold.lklhd ||
	fabs(change.num_stations)> change_threshold.num_stations);
}

bool DMEvent::noChange()
{
    return(
	current.mag         == published.mag &&
	current.mag_uncer   == published.mag_uncer &&
	current.lat         == published.lat &&
	current.lat_uncer   == published.lat_uncer &&
	current.lon         == published.lon &&
	current.lon_uncer   == published.lon_uncer &&
	current.depth       == published.depth &&
	current.depth_uncer == published.depth_uncer &&
	current.otime       == published.otime &&
	current.otime_uncer == published.otime_uncer &&
	current.lklhd       == published.lklhd &&
	current.num_stations== published.num_stations);
}

void DMEvent::publish(double publish_time)
{
    time_published	  = publish_time;
    published.mag         = current.mag;
    published.mag_uncer   = current.mag_uncer;
    published.lat         = current.lat;
    published.lat_uncer   = current.lat_uncer;
    published.lon         = current.lon;
    published.lon_uncer   = current.lon_uncer;
    published.depth       = current.depth;
    published.depth_uncer = current.depth_uncer;
    published.otime       = current.otime;
    published.otime_uncer = current.otime_uncer;
    published.lklhd       = current.lklhd;
    published.num_stations= current.num_stations;

    published.misfit 	  = current.misfit;
    published.max_dist_km = current.max_dist_km;
    published.max_delta_time = current.max_delta_time;
    published_one = true;
}

string DMEvent::changeString()
{
    char buf[500];

    snprintf(buf, sizeof(buf), "%10.10s %15.15s %3d %4.4s %6.6s", getSystemNameString().c_str(),
        getID().c_str(), getVersion(), getMessageCategoryString().c_str(),
	getMessageTypeString().c_str());

    if(change.mag != 0.) {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+6.3f", change.mag);
    }
    else {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(change.mag_uncer != 0. ) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.2f", change.mag_uncer);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(change.lat != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+8.4f", change.lat);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "         ");
    }
    if(change.lat_uncer != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.2f", change.lat_uncer);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(change.lon != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+9.4f", change.lon);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "          ");
    }
    if(change.lon_uncer != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.2f", change.lon_uncer);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }

    if(change.depth != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+6.2f", change.depth);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(change.depth_uncer != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+6.2f", change.depth_uncer);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(change.otime != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+21.3f", change.otime);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "                      ");
    }
    if(change.otime_uncer != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+6.2f", change.otime_uncer);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(change.lklhd != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.2f", change.lklhd);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(change.num_stations != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.2f", change.num_stations);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(change.misfit != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.2f", change.misfit);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(change.max_dist_km != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.1f", change.max_dist_km);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(change.max_delta_time != 0.) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %+5.1f", change.max_delta_time);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }

    return string(buf);
}

void DMEvent::setChangeThreshold(DMResult tol)
{
    change_threshold = tol;
}

/* Add algorithm finite_fault and gm_info sections to the DMEvent.
 */
bool DMEvent::loadAlgExtra(double fault_info_min_mag)
{
    vector<AlgMessage *> alg;
    for(size_t i = 0; i < algo_events.size(); i++) {
	AlgoMapIter it = algo_map->find(algo_events[i]);
	CoreEventInfo *cei = (*it).second;
        // don't include eqinfo since its gm_info and finite_fault duplicate the algs
	if(cei->getSystemName() != EQINFO2GM) {
	    alg.push_back((AlgMessage *)cei);
	}
    }

    bool load_fault_info = (current.mag >= fault_info_min_mag);

    loadAlgMessages(alg, load_fault_info);

    for(size_t i = 0; i < algo_events.size(); i++) {
	AlgoMapIter it = algo_map->find(algo_events[i]);
	CoreEventInfo *cei = (*it).second;
	if(cei->getSystemName() == EQINFO2GM) {
            addContributor((AlgMessage *)cei);
	}
    }
    return false;
}

static double distkm(double slat, double slon, double rlat, double rlon)
{
    double deg_to_radians = M_PI/180.;
    double deg_to_km = 111.19492664;

    slat *= deg_to_radians;
    rlat *= deg_to_radians;
    slon *= deg_to_radians;
    rlon *= deg_to_radians;

    double arg = sin(slat)*sin(rlat) + cos(slat)*cos(rlat)*cos(slon-rlon);
    if(arg <= -1.0) {
	return 180.*deg_to_km;
    }
    else if(arg >= 1.0) {
	return 0.;
    }
    return deg_to_km*(acos(arg)/deg_to_radians);
}

bool DMEvent::passedTwoAlgMag(double two_alg_mag)
{
    // only applies to the first DM alert, version = 0
    if(getVersion() > 0) return true;

    // if a contributing algorithm_mag >= two_alg_mag, wait for a message
    // from a second contributing algorithm with mag >= algorithm_mag-1.0

    // find the largest contributing magnitude
    CoreEventInfo *cei_largest = NULL;
    for(size_t i = 0; i < algo_events.size(); i++) if(algo_events[i].useCoreEventInfo())
    {
	AlgoMapIter it = algo_map->find(algo_events[i]);
	CoreEventInfo *cei = (*it).second;
	if(cei_largest == NULL || cei->getMagnitude() > cei_largest->getMagnitude()) {
	    cei_largest = cei;
	}
    }

    if(cei_largest != NULL && cei_largest->getMagnitude() >= two_alg_mag)
    {
	// look for a second algorithm magnitude >= algorithm_mag - 1.0
	for(size_t i = 0; i < algo_events.size(); i++) if(algo_events[i].useCoreEventInfo())
	{
	    AlgoMapIter it = algo_map->find(algo_events[i]);
	    CoreEventInfo *cei = (*it).second;
	    if(cei != cei_largest && cei->getMagnitude() >= cei_largest->getMagnitude()-1.0) {
		return true; // passed
	    }
	}
	return false; // failed two_alg_mag criteria
    }
    return true;
}

AlgorithmEvent * DMEvent::getContributor(enum eewSystemName system_name)
{
    for(size_t i = 0; i < algo_events.size(); i++) {
	if(algo_events[i].system_name == system_name) {
	    return &algo_events[i];
	}
    }
    return NULL;
}
