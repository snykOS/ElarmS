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
#ifndef __Alg_Message_h_
#define __Alg_Message_h_

/** \page elarms_page Elarms Example XML Message
 *  \section elarms_sec1 An Example Elarms XML Message
 *  The <b>pgv_obs</b> node is only present when there are velocity observations.
 *  Similarly, the <b>pga_obs</b> node is only present when there are acceleration
 *  observations. The <b>elarms_info</b> node will not be present when there are no observations of
 *  either type.
 *  \include elarms.xml
 */
#include "CoreEventInfo.h"
#include "GMObservation.h"
#include "GMPrediction.h"

typedef list<GMObservation>::iterator GMObservationIter;
typedef list<GMPrediction>::iterator GMPredictionIter;


/** A CoreEventInfo subclass that can contain velocity and acceleration observation values and
 *  velocity predictions.
 *  @ingroup dm_lib
 */
class AlgMessage : public CoreEventInfo
{
 protected:
    list<GMObservation> gm_observations;
    list<GMPrediction> gm_predictions;

 public:
    AlgMessage(enum eewSystemName sys_name = ELARMS,
		string id="-9",
		double mag=-9.9,
		double mag_uncer=-9.9,
		double lat=-999.9,
		double lat_uncer=-999.9,
		double lon=-999.9,
		double lon_uncer=-999.9,
		double dep=-9.9,
		double dep_uncer=-9.9,
		double o_time = -99999.99,
		double o_time_uncer=-9.9,
		double likelihood=-9.9,
		enum nudMessageType type = NEW,
		int ver = 0,
		enum MessageCategory category = LIVE,
		string time_stamp = "",
		string alg_ver = "-",
		string instance = "",
		int num_stations = 0,
		string ref_id = "0",
		string ref_src = "dm",
		string orig_sys = "",
		string mag_units = "Mw",
		string mag_uncer_units = "Mw",
		string lat_units = "deg",
		string lat_uncer_units = "deg",
		string lon_units = "deg",
		string lon_uncer_units = "deg",
		string dep_units = "km",
		string dep_uncer_units = "km",
		string o_time_units = "UTC",
		string o_time_uncer_units = "sec"
		);
    AlgMessage(enum eewSystemName sys_name = ELARMS,
		int id=-9,
		double mag=-9.9,
		double mag_uncer=-9.9,
		double lat=-999.9,
		double lat_uncer=-999.9,
		double lon=-999.9,
		double lon_uncer=-999.9,
		double dep=-9.9,
		double dep_uncer=-9.9,
		double o_time = -99999.99,
		double o_time_uncer=-9.9,
		double likelihood=-9.9,
		enum nudMessageType type = NEW,
		int ver = 0,
		enum MessageCategory category = LIVE,
		string time_stamp = "",
		string alg_ver = "-",
		string instance = "",
		int num_stations = 0,
		int ref_id = 0,
		string ref_src = "dm",
		string orig_sys = "",
		string mag_units = "Mw",
		string mag_uncer_units = "Mw",
		string lat_units = "deg",
		string lat_uncer_units = "deg",
		string lon_units = "deg",
		string lon_uncer_units = "deg",
		string dep_units = "km",
		string dep_uncer_units = "km",
		string o_time_units = "UTC",
		string o_time_uncer_units = "sec"
		);

    virtual ~AlgMessage();

    int getNumberObservations();
    int getNumberPredictions();

    void clearObservations();
    void clearPredictions();

    GMObservationIter getGMObservationIteratorBegin();
    GMObservationIter getGMObservationIteratorEnd();

    GMPredictionIter getGMPredictionIteratorBegin();
    GMPredictionIter getGMPredictionIteratorEnd();

    int addGMObservation(GMObservation &A);
    int addGMObservation(enum ObservationType obs_type,
			string sta,
			string net, 
			string chan, 
			string loc, 
			double obs, 
			double lat, 
			double lon,
			double time,
			string obs_units,
			string lat_units = "deg",
			string lon_units = "deg",
			string time_units= "UTC",
			string orig_sys="");

    /// \deprecated interface
    int addPGVObservation(string sta, string net, string chan, string loc, double obs, 
			double lat, double lon, double time, string obs_units="cm/s", string lat_units = "deg",
			string lon_units = "deg", string time_units= "UTC")
    {
	return addGMObservation(VELOCITY_OBS, sta, net, chan, loc, obs, 
			lat, lon, time, obs_units, lat_units, lon_units, time_units);
    }
    /// \deprecated interface
    int addPGAObservation(string sta, string net, string chan, string loc, double obs, 
			double lat, double lon, double time, string obs_units="g", string lat_units = "deg",
			string lon_units = "deg", string time_units= "UTC")
    {
	return addGMObservation(ACCELERATION_OBS, sta, net, chan, loc, obs, 
			lat, lon, time, obs_units, lat_units, lon_units, time_units);
    }

    void deleteGMObservation(GMObservation &A);
    void deleteGMObservation(enum ObservationType obs_type,
			string sta,
			string net, 
			string chan, 
			string loc, 
			double obs, 
			double lat, 
			double lon,
			double time,
			string obs_units,
			string lat_units = "deg",
			string lon_units = "deg",
			string time_units= "UTC");
    void deleteGMObservation(enum ObservationType obs_type,
			string sta,
			string net, 
			string chan, 
			string loc);

    int addGMPrediction(GMPrediction &A);
    int addGMPrediction(enum PredictionType pred_type,
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
			string time_units = "UTC",
			string time_uncer_units = "sec",
			string app_rad_units = "km",
			string lat_units = "deg",
			string lon_units = "deg");
    /// \deprecated interface
    int addPGVPrediction(string sta, string net, string chan, string loc, double pred,
			double pred_uncer, double time, double time_uncer, double app_rad,
			double lat, double lon, string pred_units="cm/s",
			string pred_uncer_units="cm/s", string time_units = "UTC",
			string time_uncer_units = "sec", string app_rad_units = "km",
			string lat_units = "deg", string lon_units = "deg")
    {
	return addGMPrediction(VELOCITY_PRED, sta, net, chan, loc, pred, pred_uncer, time,
			time_uncer, app_rad, lat, lon, pred_units, pred_uncer_units, time_units,
			time_uncer_units, app_rad_units, lat_units, lon_units);
    }

    void deleteGMPrediction(GMPrediction &A);
    void deleteGMPrediction(enum PredictionType pred_type,
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
			string time_units = "UTC",
			string time_uncer_units = "sec",
			string app_rad_units = "km",
			string lat_units = "deg",
			string lon_units = "deg");
    void deleteGMPrediction(enum PredictionType pred_type,
			string sta,
			string net,
			string chan,
			string loc);


    virtual string updateFrom(const CoreEventInfo &);

    void coutPrint(bool long_form);
    string toString(bool long_form=false);
    string obsToString();
    string predToString();
};


#endif
