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
#include <iostream>
#include <iomanip>
#include "GMObservation.h"
#include "CoreEventInfo.h"
using namespace std;


GMObservation::GMObservation(
		enum ObservationType obs_type,
		string sta,
		string net,
		string chan,
		string loc,
		double obs, 
		double lat, 
		double lon, 
		double time,
		string obs_units,
		string lat_units,
		string lon_units,
		string time_units,
		string orig_sys) : StaNetChanLoc(sta, net, chan, loc),
				observation_type(obs_type),
				observation(obs),
				latitude(lat),
				longitude(lon),
				observation_time(time),
				observation_units(obs_units),
				latitude_units(lat_units),
				longitude_units(lon_units),
				observation_time_units(time_units),
				orig_system(orig_sys) {}

GMObservation::~GMObservation() {}

double GMObservation::setObservation(double obs) { observation = obs; return observation; }
double GMObservation::setLatitude(double lat) { latitude = lat; return latitude; }
double GMObservation::setLongitude(double lon) { longitude = lon; return longitude; }
double GMObservation::setObservationTime(double time) {
    observation_time = time;
    return observation_time;
}
string GMObservation::setObservationUnits(string obs_units) {
    observation_units = obs_units;
    return observation_units;
}
string GMObservation::setLatitudeUnits(string lat_units) {
    latitude_units = lat_units;
    return latitude_units;
}
string GMObservation::setLongitudeUnits(string lon_units) {
    longitude_units = lon_units;
    return longitude_units;
}
string GMObservation::setObservationTimeUnits(string time_units) {
    observation_time_units = time_units;
    return observation_time_units;
}
string GMObservation::setOrigSys(string orig_sys) { orig_system = orig_sys; return orig_system; }

enum ObservationType GMObservation::getObservationType() const { return observation_type; }
string GMObservation::getObservationTypeString() const {
	return ObservationTypeString[observation_type]; }
double GMObservation::getObservation() const { return observation; }
double GMObservation::getLatitude() const { return latitude; }
double GMObservation::getLongitude() const { return longitude; }
double GMObservation::getObservationTime() const { return observation_time; }
string GMObservation::getObservationTimeString() const {
    return CoreEventInfo::trueEpochToString(observation_time);
}
string GMObservation::getObservationUnits() const { return observation_units; }
string GMObservation::getLatitudeUnits() const { return latitude_units; }
string GMObservation::getLongitudeUnits() const { return longitude_units; }
string GMObservation::getObservationTimeUnits() const { return observation_time_units; }
string GMObservation::getOrigSys() const { return orig_system; }

void GMObservation::updateFrom(const GMObservation &from)
{
    observation = from.observation;
    latitude = from.latitude;
    longitude = from.longitude;
    observation_time = from.observation_time;
    observation_units = from.observation_units;
    latitude_units = from.latitude_units;
    longitude_units = from.longitude_units;
    observation_time_units = from.observation_time_units;
    orig_system = from.orig_system;
}

bool GMObservation::operator==(const GMObservation &A) const
{
    return ( A.observation_type == observation_type &&
	A.observation == observation &&
	A.latitude == latitude &&
	A.longitude == longitude &&
	A.observation_time == observation_time &&
	A.observation_units == observation_units &&
	A.latitude_units == latitude_units &&
	A.longitude_units == longitude_units &&
	A.observation_time_units == observation_time_units &&
	A.orig_system == orig_system &&
	StaNetChanLoc::operator==((StaNetChanLoc)A));
}
bool GMObservation::operator!=(const GMObservation &A) const { return !(*this==A); }
bool GMObservation::operator<(const GMObservation &A) const {
    return (StaNetChanLoc::operator<((StaNetChanLoc)A));
}
bool GMObservation::operator>(const GMObservation &A) const {
    return (StaNetChanLoc::operator>((StaNetChanLoc)A));
}

string GMObservation::toString()
{
    stringstream s;
    s << setiosflags(ios::fixed) << setprecision(4);

    s << "  " << getSNCL();
    s << " obs: " << getObservation() << " " << getObservationUnits();
    s << "  time: " << getObservationTime() << " " << getObservationTimeUnits();
    s << "  lat: " << getLatitude() << " " << getLatitudeUnits();
    s << "  lon: " << getLongitude() << " " << getLongitudeUnits();
    s << " orig_sys: " << getOrigSys();
    return s.str();
}

void GMObservation::coutPrint()
{
    cout << toString() << endl;
}
