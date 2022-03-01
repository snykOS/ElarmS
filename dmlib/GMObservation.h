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
#ifndef __GM_Observation_h
#define __GM_Observation_h

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "StaNetChanLoc.h"

using namespace std;

enum ObservationType {DISPLACEMENT_OBS, VELOCITY_OBS, ACCELERATION_OBS};
const string ObservationTypeString[] = { "pgd_obs", "pgv_obs", "pga_obs" };

/** A base class for station ground motion observations.
 *  @ingroup dm_lib
 */
class GMObservation : public StaNetChanLoc
{
 private:
    enum ObservationType observation_type;
    double observation;
    double latitude;
    double longitude;
    double observation_time;
    string observation_units;
    string latitude_units;
    string longitude_units;
    string observation_time_units;
    string orig_system;
    
 public:
    GMObservation(enum ObservationType obs_type,
		string sta="", 
		string net="", 
		string chan="", 
		string loc="",
		double obs = -9.9999, 
		double lat = -999.9999, 
		double lon = -999.9999,
		double time = -99999.99,
		string obs_units = "cm/s",
		string lat_units = "deg",
		string lon_units = "deg",
		string time_units = "UTC",
		string orig_sys = "");
    virtual ~GMObservation();
    
    double setObservation(double obs);
    double setLatitude(double lat);
    double setLongitude(double lon);
    double setObservationTime(double time);

    string setObservationUnits(string obs_units);
    string setLatitudeUnits(string lat_units);
    string setLongitudeUnits(string lon_units);
    string setObservationTimeUnits(string time_units);
    string setOrigSys(string orig_sys);

    enum ObservationType getObservationType() const;
    string getObservationTypeString() const;
    double getObservation() const;
    double getLatitude() const;
    double getLongitude() const;
    double getObservationTime() const;
    string getObservationTimeString() const;

    string getObservationUnits() const;
    string getLatitudeUnits() const;
    string getLongitudeUnits() const;
    string getObservationTimeUnits() const;
    string getOrigSys() const;

    virtual void updateFrom(const GMObservation &);

    using StaNetChanLoc::operator==;
    using StaNetChanLoc::operator!=;
    virtual bool operator==(const GMObservation &A) const;
    virtual bool operator!=(const GMObservation &A) const;
    virtual bool operator<(const GMObservation &A) const;
    virtual bool operator>(const GMObservation &A) const;

    void coutPrint();
    string toString();
};

#endif
