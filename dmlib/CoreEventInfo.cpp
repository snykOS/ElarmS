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
#include "DMLib.h"              // declaration of DMLib::getVersion
#include "CoreEventInfo.h"
#include "qlib2.h"
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

static const char *version="libdm_version 2.1.6 2019-04-30";

std::string DMLib::getVersion()
{
    return version;
}

enum eewSystemName CoreEventInfo::eewSystemNameArray[] = {ELARMS, ONSITE, VS, FINDER, GPSLIP, FINITE_FAULT, DM, DMREVIEW, EQINFO2GM, EPIC, BEFORES, SA};
enum nudMessageType CoreEventInfo::nudMessageTypeArray[] = {NEW, UPDATE, DELETE};
enum MessageCategory CoreEventInfo::MessageCategoryArray[] = {LIVE, TEST};

    
/** Constructor.
 *  @param[in] sys_name event system name 
 *  @param[in] id event id
 *  @param[in] mag event magnitude
 *  @param[in] mag_uncer event magnitude uncertainty
 *  @param[in] lat event latitude
 *  @param[in] lat_uncer event latitude uncertainty
 *  @param[in] lon event longitude
 *  @param[in] lon_uncer event longitude uncertainty
 *  @param[in] dep event depth
 *  @param[in] dep_uncer event depth uncertainty
 *  @param[in] o_time event origin time
 *  @param[in] o_time_uncer event origin time uncertainty
 *  @param[in] lklihd event likelihood
 *  @param[in] type event message type (NEW, UPDATE, or DELETE)
 *  @param[in] ver event message version number
 *  @param[in] category event message category (LIVE or TEST)
 *  @param[in] time_stamp message send time "YYYY-MM-DDThh:mm.ssZ"
 *  @param[in] alg_ver algorithm version
 *  @param[in] instance program instance
 *  @param[in] num_stations number of contributing stations
 *  @param[in] ref_id an associated DM message id
 *  @param[in] ref_src an associated DM message source
 *  @param[in] mag_units event magnitude units
 *  @param[in] mag_uncer_units event magnitude uncertainty units
 *  @param[in] lat_units event latitude units
 *  @param[in] lat_uncer_units event latitude uncertainty units
 *  @param[in] lon_units event longitude units
 *  @param[in] lon_uncer_units event longitude uncertainty units
 *  @param[in] dep_units event depth units
 *  @param[in] dep_uncer_units event depth uncertainty units
 *  @param[in] o_time_units event origin time units
 *  @param[in] o_time_uncer_units event origin time uncertainty units
 */
CoreEventInfo::CoreEventInfo(enum eewSystemName sys_name,
			     string id,
			     double mag,
			     double mag_uncer,
			     double lat,
			     double lat_uncer,
			     double lon,
			     double lon_uncer,
			     double dep,
			     double dep_uncer,
			     double o_time,
			     double o_time_uncer,
			     double lklihd,
			     enum nudMessageType type,
			     int ver,
			     enum MessageCategory category,
			     string time_stamp,
			     string alg_ver,
			     string instance,
			     int num_stations,
			     string ref_id,
			     string ref_src,
			     string orig_sys,
			     string mag_units,
			     string mag_uncer_units,
			     string lat_units,
			     string lat_uncer_units,
			     string lon_units,
			     string lon_uncer_units,
			     string dep_units,
			     string dep_uncer_units,
			     string o_time_units,
			     string o_time_uncer_units
 			     ) : event_id(id),
				 reference_id(ref_id),
				 reference_src(ref_src),
				 originating_system(orig_sys),
				 system_name(sys_name),
				 message_type(type),
				 message_category(category),
				 version(ver),
				 magnitude(mag),
				 magnitude_uncertainty(mag_uncer),
				 latitude(lat),
				 latitude_uncertainty(lat_uncer),
				 longitude(lon),
				 longitude_uncertainty(lon_uncer),
				 depth(dep),
				 depth_uncertainty(dep_uncer),
				 origin_time(o_time),
				 origin_time_uncertainty(o_time_uncer),
				 origin_time_string("2999-12-31T24:60:60.0000Z"),
				 likelyhood(lklihd),
				 number_stations(num_stations),
				 sent_flag(false),
				 timestamp(time_stamp),
			         algorithm_version(alg_ver),
			         program_instance(instance),
				 magnitude_units(mag_units),
				 magnitude_uncertainty_units(mag_uncer_units),
				 latitude_units(lat_units),
				 latitude_uncertainty_units(lat_uncer_units),
				 longitude_units(lon_units),
				 longitude_uncertainty_units(lon_uncer_units),
				 depth_units(dep_units),
				 depth_uncertainty_units(dep_uncer_units),
				 origin_time_units(o_time_units),
				 origin_time_uncertainty_units(o_time_uncer_units),
				 log_time("")
{
}
CoreEventInfo::CoreEventInfo(enum eewSystemName sys_name,
			     int id,
			     double mag,
			     double mag_uncer,
			     double lat,
			     double lat_uncer,
			     double lon,
			     double lon_uncer,
			     double dep,
			     double dep_uncer,
			     double o_time,
			     double o_time_uncer,
			     double lklihd,
			     enum nudMessageType type,
			     int ver,
			     enum MessageCategory category,
			     string time_stamp,
			     string alg_ver,
			     string instance,
			     int num_stations,
			     int ref_id,
			     string ref_src,
			     string orig_sys,
			     string mag_units,
			     string mag_uncer_units,
			     string lat_units,
			     string lat_uncer_units,
			     string lon_units,
			     string lon_uncer_units,
			     string dep_units,
			     string dep_uncer_units,
			     string o_time_units,
			     string o_time_uncer_units
 			     ) : 
				 reference_src(ref_src),
				 originating_system(orig_sys),
				 system_name(sys_name),
				 message_type(type),
				 message_category(category),
				 version(ver),
				 magnitude(mag),
				 magnitude_uncertainty(mag_uncer),
				 latitude(lat),
				 latitude_uncertainty(lat_uncer),
				 longitude(lon),
				 longitude_uncertainty(lon_uncer),
				 depth(dep),
				 depth_uncertainty(dep_uncer),
				 origin_time(o_time),
				 origin_time_uncertainty(o_time_uncer),
				 origin_time_string("2999-12-31T24:60:60.0000Z"),
				 likelyhood(lklihd),
				 number_stations(num_stations),
				 sent_flag(false),
				 timestamp(time_stamp),
			         algorithm_version(alg_ver),
			         program_instance(instance),
				 magnitude_units(mag_units),
				 magnitude_uncertainty_units(mag_uncer_units),
				 latitude_units(lat_units),
				 latitude_uncertainty_units(lat_uncer_units),
				 longitude_units(lon_units),
				 longitude_uncertainty_units(lon_uncer_units),
				 depth_units(dep_units),
				 depth_uncertainty_units(dep_uncer_units),
				 origin_time_units(o_time_units),
				 origin_time_uncertainty_units(o_time_uncer_units),
				 log_time("")
{
    char s[100];
    snprintf(s, sizeof(s), "%d", id);
    event_id.assign(s);
    snprintf(s, sizeof(s), "%d", ref_id);
    reference_id.assign(s);
}

CoreEventInfo::~CoreEventInfo() 
{}

/** This method can be called after a message is sent to activate automatic update of
 *  the version number. The sent_flag is initially false. The updateSent method sets
 *  the sent_flag to true and sets the message_type to UPDATE. Any subsequent call to
 *  updateVersion, or to any of the Set Value Methods, or any of the Set Units Methods
 *  causes the version number to be incremented, but only once until updateSent is
 *  called again.
 *
 *  For example:
 *
 * @code
 *    DMMessageSender sender(...);
 *    CoreEventInfo cei;
 *
 *    cei.setId(3428);
 *    cei.setMagnitude(3.4);
 *    cei.setLatitude(34.0);
 *    ...
 *    // The version number is initially 0
 *    sender.sendMessage(cei);
 *    cei.updateSent(); // message_type set to UPDATE
 *    cei.setMagnitude(3.5); // version number incremented to 1
 *    cei.setLatitude(34.1); // version number still 1
 *    ...
 *    sender.sendMessage(cei); // send update message with version=1
 * @endcode
 */
void CoreEventInfo::updateSent() 
{
    sent_flag=true;
    message_type = UPDATE;
}

/* This method increments the version number, if the sent_flag is true. It also
 * sets the sent_flag to false. This method is called by all of the Set Value and
 * Set Units methods.
 */
void CoreEventInfo::updateVersion() 
{
    if (sent_flag){
	version++;
	sent_flag=false; 
    }
}

/** Set the Event ID number.
 */
string CoreEventInfo::setID(string id) {
    if (event_id != id) {
	event_id = id;
	updateVersion();
    }
    return event_id;
}

/** Manually set the Event version number.
 */
int CoreEventInfo::setVersion(int v) 
{
    if (version != v) {
	version = v;
    }
    return version;
}

/** Set the Event Magnitude value. */
double CoreEventInfo::setMagnitude(double mag) { 
    if (magnitude != mag) {
	magnitude=mag;
	updateVersion();
    }
    return magnitude;
}
/** Set the Event Magnitude uncertainty. */
double CoreEventInfo::setMagnitudeUncertainty(double mag_uncer) { 
    if (magnitude_uncertainty != mag_uncer) {
	magnitude_uncertainty=mag_uncer;
	updateVersion();
    }
    return magnitude_uncertainty; 
}
/** Set the Event Latitude value. */
double CoreEventInfo::setLatitude(double lat) {
    if (latitude != lat) {
	updateVersion();
	latitude=lat;
    }
    return latitude; 
}
/** Set the Event Latitude uncertainty. */
double CoreEventInfo::setLatitudeUncertainty(double lat_uncer) { 
    if (latitude_uncertainty != lat_uncer) {
	latitude_uncertainty = lat_uncer;
	updateVersion();
    }
    return latitude_uncertainty; 
}
/** Set the Event Longitude value. */
double CoreEventInfo::setLongitude(double lon) { 
    if (longitude != lon) {
	longitude = lon;
	updateVersion();
    }
    return longitude; 
}
/** Set the Event Longitude uncertainty. */
double CoreEventInfo::setLongitudeUncertainty(double lon_uncer) { 
    if (longitude_uncertainty != lon_uncer) {
	longitude_uncertainty = lon_uncer;
	updateVersion();
    }
    return longitude_uncertainty; 
}
/** Set the Event Depth value. */
double CoreEventInfo::setDepth(double dep) {
    if (depth != dep) {
	depth = dep;
	updateVersion();
    }
    return depth;
}
/** Set the Event Depth uncertainty. */
double CoreEventInfo::setDepthUncertainty(double dep_uncer) {
    if (depth_uncertainty != dep_uncer) {
	depth_uncertainty = dep_uncer;
	updateVersion();
    }
    return depth_uncertainty;
}
/** Set the Event Origin time. */
double CoreEventInfo::setOriginTime(double o_time) 
{
    if (origin_time != o_time) {
	origin_time = o_time;
	updateVersion();
    }
    return origin_time;
}
/** Set the Event Origin time with a time string argument. */
bool CoreEventInfo::setOriginTime(string o_time_str)
{
    double ot;
    if(stringToTepoch(o_time_str, &ot)) {
	if (origin_time != ot) {
	    origin_time = ot;
	    updateVersion();
	}
    }
    else {
	cerr << "CoreEventInfo::setOriginTime: invalid input." << endl;
	return false;
    }
    return true;
}

// static
/** Convert a string to a double value. */
bool CoreEventInfo::stringToDouble(string &s, double *value)
{
    char *endptr, last_char;
    const char *c = s.c_str();
    double d;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
	d = strtod(c, &endptr);
	last_char = *endptr;
	if(last_char == c[n]) {
	    *value = d;
	    return true;
	}
    }
    return false;
}
// static
/** Convert a string to an int value. */
bool CoreEventInfo::stringToInt(string &s, int *value)
{
    char *endptr, last_char;
    const char *c = s.c_str();
    long l;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
	l = strtol(c, &endptr, 10);
	last_char = *endptr;
	if(last_char == c[n]) {
	    *value = (int)l;
	    return true;
	}
    }
    return false;
}

// static
/** Convert a time string to a double epochal time value.
 *  Uses qlib2 parse_date() and int_to_tepoch().
 */
bool CoreEventInfo::stringToTepoch(string otime_str, double *otime)
{
//  otime_str must be either a double
//  or in this format: "2011-05-06T18:12:37.038Z"
//  convert to "2011/05/06/18:12:37.038" for qlib2::parse_date()

    char c[25];
    int n = (int)otime_str.length();
    if(n < 20) return false;
    const char *s = otime_str.c_str();
    if(s[4] != '-' || s[7] != '-' || s[10] != 'T') return false;
    strcpy(c, s);
    c[4] = '/';    // replace '-'
    c[7] = '/';    // replace '-';
    c[10] = '/';   // replace 'T'
    c[n-1] = '\0'; // replace 'Z'

    INT_TIME *nt = parse_date(c);
    if(nt == NULL) return false;
    *otime = int_to_tepoch(*nt);
    return true;
}

/** Set Event origin time uncertainty. */
double CoreEventInfo::setOriginTimeUncertainty(double o_time_uncer) { 
    if (origin_time_uncertainty != o_time_uncer) {
	origin_time_uncertainty = o_time_uncer;
	updateVersion();
    }
    return origin_time_uncertainty;
}

/** Set Event likelihood value. */
double CoreEventInfo::setLikelyhood(double lklyhd) {
    if ((lklyhd >= 0.0) && (lklyhd <= 1.0) && (likelyhood != lklyhd)) {
	likelyhood = lklyhd;
	updateVersion();
    }
    return likelyhood;
}
/** Set the system name. */
enum eewSystemName CoreEventInfo::setSystemName(enum eewSystemName name) {
    system_name = name;
    return system_name;
}

/** Set the message type. NEW, UPDATE, or DELETE */
enum nudMessageType CoreEventInfo::setMessageType(enum nudMessageType type) 
{
    message_type = type;
    return message_type;
}

/** Set the message category. LIVE or TEST */
enum MessageCategory CoreEventInfo::setMessageCategory(enum MessageCategory category) 
{
    message_category = category;
    return message_category;
}

/** Set the message timestamp. Defaults to "-" */
string CoreEventInfo::setTimestamp(string time_stamp) 
{
    timestamp = time_stamp;
    return timestamp;
}

/** Set the message timestamp. Defaults to "-" */
string CoreEventInfo::setTimestamp(double epoch_time) 
{
    timestamp = nominalEpochToString(epoch_time);
    return timestamp;
}

/** Set the algorithm version. Defaults to "-" */
string CoreEventInfo::setAlgorithmVersion(string alg_ver) 
{
    algorithm_version = alg_ver;
    return algorithm_version;
}

/** Set the program instance. Defaults to "-" */
string CoreEventInfo::setProgramInstance(string instance) 
{
    program_instance = instance;
    return program_instance;
}

/** Set the number of stations. Defaults to 1 */
int CoreEventInfo::setNumberStations(int num_stations) 
{
    number_stations = num_stations;
    return number_stations;
}

/** Set the reference id. Defaults to 0 */
string CoreEventInfo::setReferenceId(string id) 
{
    reference_id = id;
    return reference_id;
}

/** Set the reference src. Defaults to "" */
string CoreEventInfo::setReferenceSrc(string src) 
{
    reference_src = src;
    return reference_src;
}

/** Set the originating system. Defaults to "" */
string CoreEventInfo::setOrigSys(string orig_sys) 
{
    originating_system = orig_sys;
    return originating_system;
}

/** Set the Event magnitude units. Defaults to Mw. */
string CoreEventInfo::setMagnitudeUnits(string mag_units) {
    if (magnitude_units != mag_units) {
	magnitude_units = mag_units;
	updateVersion();
    }
    return magnitude_units;
}
/** Set the Event magnitude units. Defaults to Mw. */
string CoreEventInfo::setMagnitudeUncertaintyUnits(string mag_uncer_units) {
    if (magnitude_uncertainty_units != mag_uncer_units) {
	magnitude_uncertainty_units = mag_uncer_units;
	updateVersion();
    }
    return magnitude_uncertainty_units;
}
/** Set the Event latitude units. Defaults to deg. */
string CoreEventInfo::setLatitudeUnits(string lat_units) {
    if (latitude_units != lat_units) { 
	latitude_units = lat_units;
	updateVersion();
    }
    return latitude_units; 
}
/** Set the Event latitude uncertainty units. Defaults to deg. */
string CoreEventInfo::setLatitudeUncertaintyUnits(string lat_uncer_units) { 
    if (latitude_uncertainty_units != lat_uncer_units) { 
	latitude_uncertainty_units = lat_uncer_units;
	updateVersion();
    }
    return latitude_uncertainty_units; 
}
/** Set the Event longitude units. Defaults to deg. */
string CoreEventInfo::setLongitudeUnits(string lon_units) { 
    if (longitude_units != lon_units) { 
	longitude_units = lon_units;
	updateVersion();
    }
    return longitude_units; 
}
/** Set the Event longitude uncertainty units. Defaults to deg. */
string CoreEventInfo::setLongitudeUncertaintyUnits(string lon_uncer_units) { 
    if (longitude_uncertainty_units != lon_uncer_units) {
	longitude_uncertainty_units = lon_uncer_units;
	updateVersion();
    }
    return longitude_uncertainty_units; 
}
/** Set the Event depth units. Defaults to km. */
string CoreEventInfo::setDepthUnits(string dep_units) {
    if (depth_units != dep_units) {
	depth_units = dep_units;
	updateVersion();
    }
    return depth_units;
}
/** Set the Event depth uncertainty units. Defaults to km. */
string CoreEventInfo::setDepthUncertaintyUnits(string dep_uncer_units) {
    if (depth_uncertainty_units != dep_uncer_units) {
	depth_uncertainty_units = dep_uncer_units;
	updateVersion();
    }
    return depth_uncertainty_units;
}
/** Set the Event time units. Defaults to UTC. */
string CoreEventInfo::setOriginTimeUnits(string o_time_units) { 
    if (origin_time_units != o_time_units) {
	origin_time_units = o_time_units;
	updateVersion();
    }
    return origin_time_units;
}
/** Set the Event time uncertainty units. Defaults to sec. */
string CoreEventInfo::setOriginTimeUncertaintyUnits(string o_time_uncer_units) { 
    if (origin_time_uncertainty_units != o_time_uncer_units) {
	origin_time_uncertainty_units = o_time_uncer_units;
	updateVersion();
    }
    return origin_time_uncertainty_units;
}

/** Get Event id. */
string CoreEventInfo::getID() const{
    return event_id;
}
/** Get Event version number. */
int CoreEventInfo::getVersion() const{
    return version;
}
/** Get Event magnitude. */
double CoreEventInfo::getMagnitude() const{
    return magnitude; 
}
/** Get Event magnitude uncertainty. */
double CoreEventInfo::getMagnitudeUncertainty() const{
    return magnitude_uncertainty; 
}
/** Get Event latitude. */
double CoreEventInfo::getLatitude() const{
    return latitude; 
}
/** Get Event latitude uncertainty. */
double CoreEventInfo::getLatitudeUncertainty() const{
    return latitude_uncertainty; 
}
/** Get Event longitude. */
double CoreEventInfo::getLongitude() const{
    return longitude; 
}
/** Get Event longitude uncertainty. */
double CoreEventInfo::getLongitudeUncertainty() const{ 
    return longitude_uncertainty;
}
/** Get Event depth. */
double CoreEventInfo::getDepth() const {
    return depth;
}
/** Get Event depth uncertainty. */
double CoreEventInfo::getDepthUncertainty() const {
    return depth_uncertainty;
}
/** Get origin time as a string. */
string CoreEventInfo::getOriginTimeString() const
{
    return trueEpochToString(origin_time);
}

// static
/** Convert true epoch time to a ISO time string */
string CoreEventInfo::trueEpochToString(double tepoch)
{
    INT_TIME it = tepoch_to_int(tepoch);
    EXT_TIME et = int_to_ext(it);
    char c_iso[80];

    snprintf(c_iso, sizeof(c_iso), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
	et.year, et.month, et.day, et.hour, et.minute, et.second, et.usec/USECS_PER_MSEC);
    return string(c_iso);
}
/** Convert nominal epoch time to a ISO time string */
string CoreEventInfo::nominalEpochToString(double nepoch)
{
    INT_TIME it = nepoch_to_int(nepoch);
    EXT_TIME et = int_to_ext(it);
    char c_iso[80];

    snprintf(c_iso, sizeof(c_iso), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
	et.year, et.month, et.day, et.hour, et.minute, et.second, et.usec/USECS_PER_MSEC);
    return string(c_iso);
}
/** Get Event origin time. */
double CoreEventInfo::getOriginTime() const 
{
    return origin_time;
}
/** Get Event origin time uncertainty. */
double CoreEventInfo::getOriginTimeUncertainty() const {  
    return origin_time_uncertainty;
}
/** Get Event likelihood. */
double CoreEventInfo::getLikelyhood() const {
    return likelyhood;
}
/** Get Event system name. */
enum eewSystemName CoreEventInfo::getSystemName() const {
    return system_name;
}
/** Get Event message type. NEW, UPDATE, or DELETE */
enum nudMessageType CoreEventInfo::getMessageType() const {
    return message_type;
}
/** Get Event message category. LIVE or TEST */
enum MessageCategory CoreEventInfo::getMessageCategory() const {
    return message_category;
}
/** Get Event system name as a string. "elarms", "onsite", "vs", "finder", "gpslip", "finite_fault", or "dm" */
string CoreEventInfo::getSystemNameString() const {
    return eewSystemNameString[(enum eewSystemName)system_name];
}
/** Get Event message type as a string. "new", "update", or "delete" */
string CoreEventInfo::getMessageTypeString() const {
    return MessageTypeString[(enum nudMessageType)message_type];
}
/** Get Event message category as a string. "live" or "test" */
string CoreEventInfo::getMessageCategoryString() const {
    return MessageCategoryString[(enum MessageCategory)message_category];
}
/** Get the message timestamp. */
string CoreEventInfo::getTimestamp() const {
    return timestamp;
}
/** Get the algorithm version. */
string CoreEventInfo::getAlgorithmVersion() const {
    return algorithm_version;
}
/** Get the program instance. */
string CoreEventInfo::getProgramInstance() {
    if(program_instance.empty()) {
	char name[1000];
	if(gethostname(name, (int)sizeof(name)-1) == 0) {
	    setProgramInstance(getSystemNameString() + "@" + string(name));
	}
    }
    return program_instance;
}
/** Get the number of stations. */
int CoreEventInfo::getNumberStations() const {
    return number_stations;
}
/** Get the reference id. */
string CoreEventInfo::getReferenceId() const {
    return reference_id;
}
/** Get the reference src. */
string CoreEventInfo::getReferenceSrc() const {
    return reference_src;
}
/** Get the originating system. */
string CoreEventInfo::getOrigSys() const {
    return originating_system;
}
/** Get Event magnitude units string. */
string CoreEventInfo::getMagnitudeUnits() const{
    return magnitude_units;
}
/** Get Event magnitude uncertainty units string. */
string CoreEventInfo::getMagnitudeUncertaintyUnits() const{
    return magnitude_uncertainty_units;
}
/** Get Event latitude units string. */
string CoreEventInfo::getLatitudeUnits() const{
    return latitude_units; 
}
/** Get Event latitude uncertainty units string. */
string CoreEventInfo::getLatitudeUncertaintyUnits() const{
    return latitude_uncertainty_units; 
}
/** Get Event longitude units string. */
string CoreEventInfo::getLongitudeUnits() const{
    return longitude_units; 
}
/** Get Event longitude uncertainty units string. */
string CoreEventInfo::getLongitudeUncertaintyUnits() const{ 
    return longitude_uncertainty_units; 
}
/** Get Event depth units string. */
string CoreEventInfo::getDepthUnits() const{
    return depth_units; 
}
/** Get Event depth uncertainty units string. */
string CoreEventInfo::getDepthUncertaintyUnits() const{ 
    return depth_uncertainty_units; 
}
/** Get Event origin time units string. */
string CoreEventInfo::getOriginTimeUnits() const{
    return origin_time_units;
}
/** Get Event origin time uncertainty units string. */
string CoreEventInfo::getOriginTimeUncertaintyUnits() const{ 
    return origin_time_uncertainty_units;
}

/** Copy the Event values from the input argument to this object. */
string CoreEventInfo::updateFrom(const CoreEventInfo &cei)
{
    char buf[500];

    snprintf(buf, sizeof(buf), "%6.6s %s %3d %4.4s %6.6s", cei.getSystemNameString().c_str(),
        cei.getID().c_str(), cei.getVersion(), cei.getMessageCategoryString().c_str(),
	cei.getMessageTypeString().c_str());

    if(magnitude != cei.magnitude) {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.3f", cei.magnitude);
    }
    else {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(magnitude_uncertainty != cei.magnitude_uncertainty ) {
	if(cei.magnitude_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -9999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", cei.magnitude_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(latitude != cei.latitude) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %8.4f", cei.latitude);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "         ");
    }
    if(latitude_uncertainty != cei.latitude_uncertainty) {
	if(cei.latitude_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -9999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", cei.latitude_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(longitude != cei.longitude) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %9.4f", cei.longitude);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "          ");
    }
    if(longitude_uncertainty != cei.longitude_uncertainty) {
	if(cei.longitude_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -9999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", cei.longitude_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }

    if(depth != cei.depth) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.2f", cei.depth);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(depth_uncertainty != cei.depth_uncertainty) {
	if(cei.depth_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -99999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.2f", cei.depth_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(origin_time != cei.origin_time) {
	string os = cei.getOriginTimeString().substr(2);
	os.replace(8, 1, " ");
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %21.21s", os.c_str());
    }
    else {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "                     ");
    }
    if(origin_time_uncertainty != cei.origin_time_uncertainty) {
	if(cei.origin_time_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -99999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.2f", cei.origin_time_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(likelyhood != cei.likelyhood) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", getLikelyhood());
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(number_stations != cei.number_stations) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5d", getNumberStations());
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    version 			= cei.version;
    magnitude 			= cei.magnitude;
    magnitude_uncertainty	= cei.magnitude_uncertainty;
    latitude 			= cei.latitude;
    latitude_uncertainty 	= cei.latitude_uncertainty;
    longitude 			= cei.longitude;
    longitude_uncertainty 	= cei.longitude_uncertainty;
    depth 			= cei.depth;
    depth_uncertainty 		= cei.depth_uncertainty;
    origin_time 		= cei.origin_time;
    origin_time_uncertainty 	= cei.origin_time_uncertainty;
    origin_time_string 		= cei.origin_time_string;
    likelyhood 			= cei.likelyhood;
    number_stations 		= cei.number_stations;
    sent_flag 			= cei.sent_flag;

    magnitude_units 		= cei.magnitude_units;
    magnitude_uncertainty_units	= cei.magnitude_uncertainty_units;
    latitude_units 		= cei.latitude_units;
    latitude_uncertainty_units 	= cei.latitude_uncertainty_units;
    longitude_units 		= cei.longitude_units;
    longitude_uncertainty_units	= cei.longitude_uncertainty_units;
    depth_units 		= cei.depth_units;
    depth_uncertainty_units 	= cei.depth_uncertainty_units;
    origin_time_units 		= cei.origin_time_units;
    origin_time_uncertainty_units = cei.origin_time_uncertainty_units;

    return string(buf);
}

string CoreEventInfo::updateString(const CoreEventInfo &cei)
{
    char buf[500];

    snprintf(buf, sizeof(buf), "%6.6s %s %3d %4.4s %6.6s", cei.getSystemNameString().c_str(),
        cei.getID().c_str(), cei.getVersion(), cei.getMessageCategoryString().c_str(),
	cei.getMessageTypeString().c_str());

    if(magnitude != cei.magnitude) {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.3f", cei.magnitude);
    }
    else {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(magnitude_uncertainty != cei.magnitude_uncertainty ) {
	if(cei.magnitude_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -9999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", cei.magnitude_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(latitude != cei.latitude) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %8.4f", cei.latitude);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "         ");
    }
    if(latitude_uncertainty != cei.latitude_uncertainty) {
	if(cei.latitude_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -9999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", cei.latitude_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(longitude != cei.longitude) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %9.4f", cei.longitude);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "          ");
    }
    if(longitude_uncertainty != cei.longitude_uncertainty) {
	if(cei.longitude_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -9999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", cei.longitude_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }

    if(depth != cei.depth) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.2f", cei.depth);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(depth_uncertainty != cei.depth_uncertainty) {
	if(cei.depth_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -99999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.2f", cei.depth_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(origin_time != cei.origin_time) {
	string os = cei.getOriginTimeString().substr(2);
	os.replace(8, 1, " ");
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %21.21s", os.c_str());
    }
    else {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "                     ");
    }
    if(origin_time_uncertainty != cei.origin_time_uncertainty) {
	if(cei.origin_time_uncertainty < 0.) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " -99999");
	else snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.2f", cei.origin_time_uncertainty);
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "       ");
    }
    if(likelyhood != cei.likelyhood) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5.2f", getLikelyhood());
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    if(number_stations != cei.number_stations) {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %5d", getNumberStations());
    }
    else {
	snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), "      ");
    }
    return string(buf);
}

/** Print the Event values to standard output. */
void CoreEventInfo::coutCore(bool long_form, bool new_line)
{
    cout << toString(long_form);
    if(new_line) cout << endl;
}

string CoreEventInfo::toString(bool long_form)
{
    if( long_form ) {
	stringstream msg;
	msg << setiosflags(ios::fixed) << setprecision(4);
	msg << "CoreEventInfo Start:" << endl;
	msg << "  systemName: " << getSystemNameString() << endl;
	msg << "  id: " << getID() << endl;
	msg << "  version: " << getVersion() << endl;
	msg << "  messageCategory: " << getMessageCategoryString() << endl;
	msg << "  messageType: " << getMessageTypeString() << endl;
	msg << "  timestamp: " << getTimestamp() << endl;
	msg << "  algorithmVersion: " << getAlgorithmVersion() << endl;
	msg << "  programInstance: " << getProgramInstance() << endl;
	msg << "  numberStations: " << getNumberStations() << endl;

	msg << "  magnitude: " << getMagnitude() << " "
		<< getMagnitudeUnits() << endl;
	msg << "  magnitudeUncertainty: " << getMagnitudeUncertainty() << " "
		<< getMagnitudeUncertaintyUnits() << endl;
	msg << "  latitude: " << getLatitude() << " "
		<< getLatitudeUnits() << endl;
	msg << "  latitudeUncertainty: " << getLatitudeUncertainty() << " "
		<< getLatitudeUncertaintyUnits() << endl;
	msg << "  longitude: " << getLongitude() << " "
		<< getLongitudeUnits() << endl;
	msg << "  longitudeUncertainty: " << getLongitudeUncertainty() << " "
		<< getLongitudeUncertaintyUnits() << endl;
	msg << "  depth: " << getDepth() << " " << getDepthUnits() << endl;
	msg << "  depthUncertainty: " << getDepthUncertainty() << " "
		<< getDepthUncertaintyUnits() << endl;
	msg << "  originTime: " << getOriginTime() << " "
		<< getOriginTimeUnits() << endl;
	msg << "  originTimeUncertainty: " << getOriginTimeUncertainty() << " "
		<< getOriginTimeUncertaintyUnits() << endl;
	msg << "  likelihood: " << getLikelyhood() << endl;
	msg << "CoreEventInfo End:";
	return msg.str();
    }
    else {
	return coreToString();
    }
}

string CoreEventInfo::coreToString()
{
    stringstream msg;
    msg << setiosflags(ios::fixed) << setprecision(4);

    char line[200];
    snprintf(line, sizeof(line), "%10.10s %15.15s %3d %4.4s %6.6s %6.3f",getSystemNameString().c_str(),
		getID().c_str(), getVersion(), getMessageCategoryString().c_str(),
		getMessageTypeString().c_str(), getMagnitude());
    msg << line;
    // do this for aligned columns
    if(getMagnitudeUncertainty() < 0.) snprintf(line, sizeof(line), " -9999");
    else snprintf(line, sizeof(line), " %5.2f", getMagnitudeUncertainty());
    msg << line;

    snprintf(line, sizeof(line), " %8.4f", getLatitude());
    msg << line;
    if(getLatitudeUncertainty() < 0.) snprintf(line, sizeof(line), " -9999");
    else snprintf(line, sizeof(line), " %5.2f", getLatitudeUncertainty());
    msg << line;

    snprintf(line, sizeof(line), " %9.4f", getLongitude());
    msg << line;
    if(getLongitudeUncertainty() < 0.) snprintf(line, sizeof(line), " -9999");
    else snprintf(line, sizeof(line), " %5.2f", getLongitudeUncertainty());
    msg << line;

    snprintf(line, sizeof(line), " %6.2f", getDepth());
    msg << line;
    if(getDepthUncertainty() < 0.) snprintf(line, sizeof(line), " -99999");
    else snprintf(line, sizeof(line), " %6.2f", getDepthUncertainty());
    msg << line;

    string os = getOriginTimeString().substr(2);
    os.replace(8, 1, " ");
    snprintf(line, sizeof(line), " %21.21s", os.c_str());
    msg << line;
    if(getOriginTimeUncertainty() < 0.) snprintf(line, sizeof(line), " -99999");
    else snprintf(line, sizeof(line), " %6.2f", getOriginTimeUncertainty());
    msg << line;

    snprintf(line, sizeof(line), " %5.2f", getLikelyhood());
    msg << line;

    return msg.str();
}

// static
string CoreEventInfo::labelString()
{
    char labels[200];
    snprintf(labels, sizeof(labels),
        "    SYSTEM              ID VER  CAT   TYPE    MAG  MAGU      LAT  LATU       LON  LONU    DEP   DEPU                  TIME  TIMEU    LH");
    return string(labels);
}

/* return -1, 0, 1 for c1 < c2, c1 == c2 or c1 > c2
 */
int CoreEventInfo::compareTimeString(CoreEventInfo *c1, CoreEventInfo *c2)
{
    // 2018-05-23T06:11:57.341Z
    int c1_year, c1_mon, c1_day, c1_hour, c1_min;
    int c2_year, c2_mon, c2_day, c2_hour, c2_min;
    float c1_sec, c2_sec;

    sscanf(c1->getTimestamp().c_str(), "%4d-%2d-%2dT%2d:%2d:%f",
        &c1_year, &c1_mon, &c1_day, &c1_hour, &c1_min, &c1_sec);

    sscanf(c2->getTimestamp().c_str(), "%4d-%2d-%2dT%2d:%2d:%f",
        &c2_year, &c2_mon, &c2_day, &c2_hour, &c2_min, &c2_sec);

    if(c1_year != c2_year) {
        return c1_year > c2_year ? 1 : -1;
    }
    else if(c1_mon != c2_mon) {
        return c1_mon > c2_mon ? 1 : -1;
    }
    else if(c1_day != c2_day) {
        return c1_day > c2_day ? 1 : -1;
    }
    else if(c1_hour != c2_hour) {
        return c1_hour > c2_hour ? 1 : -1;
    }
    else if(c1_min != c2_min) {
        return c1_min > c2_min ? 1 : -1;
    }
    else if(c1_sec != c2_sec) {
        return c1_sec > c2_sec ? 1 : -1;
    }
    return 0;
}
