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
#ifndef __GM_Prediction_h
#define __GM_Prediction_h

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "StaNetChanLoc.h"

using namespace std;

enum PredictionType {DISPLACEMENT_PRED, VELOCITY_PRED, ACCELERATION_PRED};
const string PredictionTypeString[] = { "pgd_pred", "pgv_pred", "pga_pred" };

/** A storage class for station ground motion predictions.
 *  @ingroup dm_lib
 */
class GMPrediction: public StaNetChanLoc
{
 private:
    enum PredictionType prediction_type;
    double prediction;
    double prediction_uncertainty;
    double prediction_time;
    double prediction_time_uncertainty;
    double applied_radius;
    double latitude;
    double longitude;

    string prediction_units;
    string prediction_uncertainty_units;
    string prediction_time_units;
    string prediction_time_uncertainty_units;
    string applied_radius_units;
    string latitude_units;
    string longitude_units;
    string orig_system;
 public:
    GMPrediction(enum PredictionType pred_type,
		string sta="", 
		string net="", 
		string chan="", 
		string loc="",
		double pred = -9999.9999,
		double pred_uncer = -99.9999,
		double time = -99999.99,
		double time_uncer = -9.9999,
		double app_rad = -99.9999,
		double lat = -999.9999,
		double lon = -999.9999,
		string pred_units = "cm/s",
		string pred_uncer_units = "cm/s",
		string time_units = "UTC",
		string time_uncer_units = "sec",
		string app_rad_units = "km",
		string lat_units = "deg",
		string lon_units = "deg",
		string orig_sys="");
    virtual ~GMPrediction();
    double setPrediction(double pred);
    double setPredictionUncertainty(double pred_uncer);
    double setPredictionTime(double time);
    double setPredictionTimeUncertainty(double time_uncer);
    double setAppliedRadius(double app_rad);
    double setLatitude(double lat);
    double setLongitude(double lon);

    string setPredictionUnits(string pred_units);
    string setPredictionUncertaintyUnits(string pred_uncer_units);
    string setPredictionTimeUnits(string time_units);
    string setPredictionTimeUncertaintyUnits(string time_uncer_units);
    string setAppliedRadiusUnits(string app_rad_units);
    string setLatitudeUnits(string lat_units);
    string setLongitudeUnits(string lon_units);
    string setOrigSys(string orig_sys);

    enum PredictionType getPredictionType() const;
    string getPredictionTypeString() const;
    double getPrediction() const;
    double getPredictionUncertainty() const;
    double getPredictionTime() const;
    string getPredictionTimeString() const;
    double getPredictionTimeUncertainty() const;
    double getAppliedRadius() const;
    double getLatitude() const;
    double getLongitude() const;

    string getPredictionUnits() const;
    string getPredictionUncertaintyUnits() const;
    string getPredictionTimeUnits() const;
    string getPredictionTimeUncertaintyUnits() const;
    string getAppliedRadiusUnits() const;
    string getLatitudeUnits() const;
    string getLongitudeUnits() const;
    string getOrigSys() const;

    virtual void updateFrom(const GMPrediction &p);
    void coutPrint();
    string toString();

    using StaNetChanLoc::operator==;
    using StaNetChanLoc::operator!=;
    using StaNetChanLoc::operator<;
    using StaNetChanLoc::operator>;
    virtual bool operator==(const GMPrediction &A) const;
    virtual bool operator!=(const GMPrediction &A) const;
    virtual bool operator<(const GMPrediction &A) const;
    virtual bool operator>(const GMPrediction &A) const;
};

#endif
