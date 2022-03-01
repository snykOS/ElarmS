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
#include "GMPrediction.h"
#include "CoreEventInfo.h"

using namespace std;

GMPrediction::GMPrediction(
		enum PredictionType pred_type,
		string sta, 
		string net, 
		string chan,
		string loc,
		double pred,
		double pred_uncer,
		double time,
		double time_uncer,
		double app_rad,
		double lat,
		double lon,
		string pred_units,
		string pred_uncer_units,
		string time_units,
		string time_uncer_units,
		string app_rad_units,
		string lat_units,
		string lon_units,
		string orig_sys) : StaNetChanLoc(sta, net, chan, loc),
			prediction_type(pred_type),
			prediction(pred),
			prediction_uncertainty(pred_uncer),
			prediction_time(time),
			prediction_time_uncertainty(time_uncer),
			applied_radius(app_rad),
			latitude(lat),
			longitude(lon),
			prediction_units(pred_units),
			prediction_uncertainty_units(pred_uncer_units),
			prediction_time_units(time_units),
			prediction_time_uncertainty_units(time_uncer_units),
			applied_radius_units(app_rad_units),
			latitude_units(lat_units),
			longitude_units(lon_units),
			orig_system(orig_sys) {}

GMPrediction::~GMPrediction() {}

double GMPrediction::setPrediction(double pred) { prediction = pred; return prediction; }
double GMPrediction::setPredictionUncertainty(double pred_uncer) {
    prediction_uncertainty = pred_uncer;
    return prediction_uncertainty;
}
double GMPrediction::setPredictionTime(double o_time) {
    prediction_time = o_time;
    return prediction_time;
}
double GMPrediction::setPredictionTimeUncertainty(double o_time_uncer) {
    prediction_time_uncertainty = o_time_uncer;
    return prediction_time_uncertainty;
}
double GMPrediction::setAppliedRadius(double app_rad) {
    applied_radius = app_rad;
    return applied_radius;
}
double GMPrediction::setLatitude(double lat) { latitude = lat; return latitude; }
double GMPrediction::setLongitude(double lon) { longitude = lon; return longitude; }
string GMPrediction::setPredictionUnits(string pred_units) {
    prediction_units = pred_units;
    return prediction_units;
}
string GMPrediction::setPredictionUncertaintyUnits(string pred_uncer_units) {
    prediction_uncertainty_units = pred_uncer_units;
    return prediction_uncertainty_units;
}
string GMPrediction::setPredictionTimeUnits(string o_time_units) {
    prediction_time_units = o_time_units;
    return prediction_time_units;
}
string GMPrediction::setPredictionTimeUncertaintyUnits(string o_time_uncer_units) {
    prediction_time_uncertainty_units = o_time_uncer_units;
    return prediction_time_uncertainty_units;
}
string GMPrediction::setAppliedRadiusUnits(string app_rad_units) {
    applied_radius_units = app_rad_units;
    return applied_radius_units;
}
string GMPrediction::setLatitudeUnits(string lat_units) {
    latitude_units = lat_units;
    return latitude_units;
}
string GMPrediction::setLongitudeUnits(string lon_units) {
    longitude_units = lon_units;
    return longitude_units;
}
string GMPrediction::setOrigSys(string orig_sys) { orig_system = orig_sys; return orig_system; }

enum PredictionType GMPrediction::getPredictionType() const { return prediction_type; }
string GMPrediction::getPredictionTypeString() const { return PredictionTypeString[prediction_type]; }
double GMPrediction::getPrediction() const { return prediction; }
double GMPrediction::getPredictionUncertainty() const { return prediction_uncertainty; }
double GMPrediction::getPredictionTime() const { return prediction_time; }
string GMPrediction::getPredictionTimeString() const {
    return CoreEventInfo::trueEpochToString(prediction_time);
}

double GMPrediction::getPredictionTimeUncertainty() const { return prediction_time_uncertainty; }
double GMPrediction::getAppliedRadius() const { return applied_radius; }
double GMPrediction::getLatitude() const { return latitude ; }
double GMPrediction::getLongitude() const { return longitude; }
string GMPrediction::getPredictionUnits() const { return prediction_units; }
string GMPrediction::getPredictionUncertaintyUnits() const {
    return prediction_uncertainty_units;
}
string GMPrediction::getPredictionTimeUnits() const {
    return prediction_time_units;
}
string GMPrediction::getPredictionTimeUncertaintyUnits() const {
    return prediction_time_uncertainty_units;
}
string GMPrediction::getAppliedRadiusUnits() const { return applied_radius_units; }
string GMPrediction::getLatitudeUnits() const { return latitude_units; }
string GMPrediction::getLongitudeUnits() const { return longitude_units; }
string GMPrediction::getOrigSys() const { return orig_system; }

void GMPrediction::updateFrom(const GMPrediction &from)
{
    prediction = from.prediction;
    prediction_uncertainty = from.prediction_uncertainty;
    prediction_time = from.prediction_time;
    prediction_time_uncertainty = from.prediction_time_uncertainty;
    applied_radius = from.applied_radius;
    latitude = from.latitude;
    longitude = from.longitude;
    prediction_units = from.prediction_units;
    prediction_uncertainty_units = from.prediction_uncertainty_units;
    prediction_time_units = from.prediction_time_units;
    prediction_time_uncertainty_units = from.prediction_time_uncertainty_units;
    applied_radius_units = from.applied_radius_units;
    latitude_units = from.latitude_units;
    longitude_units = from.longitude_units;
    orig_system = from.orig_system;
}
bool GMPrediction::operator==(const GMPrediction &A) const
{
    return ( A.prediction_type == prediction_type &&
	A.prediction == prediction &&
	A.prediction_uncertainty == prediction_uncertainty &&
	A.prediction_time == prediction_time &&
	A.prediction_time_uncertainty == prediction_time_uncertainty &&
	A.applied_radius == applied_radius &&
	A.latitude == latitude &&
	A.longitude == longitude &&
	A.prediction_units == prediction_units &&
	A.prediction_uncertainty_units == prediction_uncertainty_units &&
	A.prediction_time_units == prediction_time_units &&
	A.prediction_time_uncertainty_units == prediction_time_uncertainty_units &&
	A.applied_radius_units == applied_radius_units &&
	A.latitude_units == latitude_units &&
	A.longitude_units == longitude_units &&
	A.orig_system == orig_system &&
	StaNetChanLoc::operator==((StaNetChanLoc)A) );
}

bool GMPrediction::operator!=(const GMPrediction &A) const { return !(*this==A); }
bool GMPrediction::operator<(const GMPrediction &A) const {
    return (StaNetChanLoc::operator<((StaNetChanLoc)A));
}
bool GMPrediction::operator>(const GMPrediction &A) const {
    return (StaNetChanLoc::operator>((StaNetChanLoc)A));
}

string GMPrediction::toString()
{
    stringstream s;

    s << setiosflags(ios::fixed) << setprecision(4);

    s << "  " << getSNCL();
    s << " pred: " << getPrediction() << " " << getPredictionUnits();
    s << "  pred_ucer: " << getPredictionUncertainty() << " " << getPredictionUncertaintyUnits();
    s << "  time: " << getPredictionTime() << " " << getPredictionTimeUnits();
    s << "  time_uncer: " << getPredictionTimeUncertainty() << " " << getPredictionTimeUncertaintyUnits();
    s << "  apprad: " << getAppliedRadius() << " " << getAppliedRadiusUnits();
    s << "  lat: " << getLatitude() << " " << getLatitudeUnits();
    s << "  lon: " << getLongitude() << " " << getLongitudeUnits();
    s << " orig_sys: " << getOrigSys();

    return s.str();
}

void GMPrediction::coutPrint()
{
    cout << toString() << endl;
}
