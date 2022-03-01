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
#include "AlgMessage.h"

/** Constructor.
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
AlgMessage::AlgMessage(enum eewSystemName sys_name,
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
			string o_time_uncer_units) : CoreEventInfo(sys_name,
								id, 
								mag, 
								mag_uncer, 
								lat, 
								lat_uncer, 
								lon, 
								lon_uncer, 
								dep, 
								dep_uncer, 
								o_time, 
								o_time_uncer, 
								lklihd,
								type,
								ver,
								category,
								time_stamp,
								alg_ver,
								instance,
								num_stations,
								ref_id,
								ref_src,
								orig_sys,
								mag_units ,
								mag_uncer_units,
								lat_units,
								lat_uncer_units,
								lon_units,
								lon_uncer_units,
								dep_units,
								dep_uncer_units,
								o_time_units,
								o_time_uncer_units
								) {}
AlgMessage::AlgMessage(enum eewSystemName sys_name,
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
			string o_time_uncer_units) : CoreEventInfo(sys_name,
								id, 
								mag, 
								mag_uncer, 
								lat, 
								lat_uncer, 
								lon, 
								lon_uncer, 
								dep, 
								dep_uncer, 
								o_time, 
								o_time_uncer, 
								lklihd,
								type,
								ver,
								category,
								time_stamp,
								alg_ver,
								instance,
								num_stations,
								ref_id,
								ref_src,
								orig_sys,
								mag_units ,
								mag_uncer_units,
								lat_units,
								lat_uncer_units,
								lon_units,
								lon_uncer_units,
								dep_units,
								dep_uncer_units,
								o_time_units,
								o_time_uncer_units
								) {}

AlgMessage::~AlgMessage() 
{
    gm_observations.clear();
    gm_predictions.clear();
}

int AlgMessage::getNumberObservations() { return (int)gm_observations.size(); }
int AlgMessage::getNumberPredictions() { return (int)gm_predictions.size(); }

void AlgMessage::clearObservations()
{
    gm_observations.clear();
}
void AlgMessage::clearPredictions()
{
    gm_predictions.clear();
}

GMObservationIter AlgMessage::getGMObservationIteratorBegin() {
    return gm_observations.begin();
}
GMObservationIter AlgMessage::getGMObservationIteratorEnd() {
    return gm_observations.end();
}
GMPredictionIter AlgMessage::getGMPredictionIteratorBegin() {
    return gm_predictions.begin();
}
GMPredictionIter AlgMessage::getGMPredictionIteratorEnd() {
    return gm_predictions.end();
}


int AlgMessage::addGMObservation(GMObservation &A) 
{
    GMObservationIter i = gm_observations.begin();
    GMObservationIter e = gm_observations.end();
    for (; i != e; i++) { 
	if (A == *i) {
	    return 0;
	}
    }
    gm_observations.push_back(A);
    this->updateVersion();
    return 1;
}
int AlgMessage::addGMObservation(enum ObservationType obs_type,
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
			string orig_sys)
{
    GMObservation A(obs_type, sta, net, chan, loc, obs, lat, lon, time,
			obs_units, lat_units, lon_units, time_units, orig_sys);
    GMObservationIter i = gm_observations.begin();
    GMObservationIter e = gm_observations.end();
    for (; i != e; i++) { 
	if (A == *i) {
	    return 0;
	}
    }
    gm_observations.push_back(A);
    this->updateVersion();
    return 1;
}
void AlgMessage::deleteGMObservation(GMObservation &A) 
{
    gm_observations.remove(A);
    this->updateVersion();
}
void AlgMessage::deleteGMObservation(enum ObservationType obs_type,
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
			string time_units)
{
    GMObservation A(obs_type, sta, net, chan, loc, obs, lat, lon, time,
			obs_units, lat_units, lon_units, time_units);
    gm_observations.remove(A);
    this->updateVersion();
}

void AlgMessage::deleteGMObservation(enum ObservationType obs_type,
			string sta,
			string net, 
			string chan, 
			string loc)
{
    StaNetChanLoc snclcomp(sta, net, chan, loc);
    GMObservationIter i = gm_observations.begin();
    GMObservationIter e = gm_observations.end();
    bool found_one = false;
    for (; i != e; i++) { 
	if(i->getObservationType() == obs_type && i->getSNCL() == snclcomp.getSNCL()) {
	    gm_observations.erase(i);
	    found_one = true;
	}
    }
    if(found_one) this->updateVersion();
}

int AlgMessage::addGMPrediction(GMPrediction &A) 
{
    GMPredictionIter i = gm_predictions.begin();
    GMPredictionIter e = gm_predictions.end();
    for (; i != e; i++) { 
	if (A == *i) {
	    return 0;
	}
    }
    gm_predictions.push_back(A);
    this->updateVersion();
    return 1;
}
int AlgMessage::addGMPrediction(enum PredictionType pred_type,
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
			string lon_units) {
    GMPrediction A(pred_type, sta, net, chan, loc, pred, pred_uncer, time, time_uncer, app_rad, 
		    lat, lon, pred_units, pred_uncer_units, time_units, time_uncer_units,
		    app_rad_units, lat_units, lon_units);
    GMPredictionIter i = gm_predictions.begin();
    GMPredictionIter e = gm_predictions.end();
    for (; i != e; i++) { 
	if (A == *i) {
	    return 0;
	}
    }
    gm_predictions.push_back(A);
    this->updateVersion();
    return 1;
}
void AlgMessage::deleteGMPrediction(GMPrediction &A) 
{
    gm_predictions.remove(A);
    this->updateVersion();
}
void AlgMessage::deleteGMPrediction(enum PredictionType pred_type,
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
			string lon_units)

{
    GMPrediction A(pred_type, sta, net, chan, loc, pred, pred_uncer, time, time_uncer, app_rad, 
		    lat, lon, pred_units, pred_uncer_units, time_units, time_uncer_units,
		    app_rad_units, lat_units, lon_units);
    gm_predictions.remove(A);
    this->updateVersion();
}

void AlgMessage::deleteGMPrediction(enum PredictionType pred_type,
			string sta,
			string net, 
			string chan, 
			string loc)
{
    StaNetChanLoc snclcomp(sta, net, chan, loc);
    GMPredictionIter i = gm_predictions.begin();
    GMPredictionIter e = gm_predictions.end();
    bool found_one = false;
    for (; i != e; i++) { 
	if(i->getPredictionType() == pred_type && i->getSNCL() == snclcomp.getSNCL()) {
	    gm_predictions.erase(i);
	    found_one = true;
	}
    }
    if(found_one) this->updateVersion();
}

string AlgMessage::updateFrom(const CoreEventInfo &cei)
{
    string s;

    // change core values
    s = CoreEventInfo::updateFrom(cei);

    AlgMessage *alg = (AlgMessage *)&cei;

    // replace GMObservations
    gm_observations.clear();
    for(GMObservationIter it = alg->getGMObservationIteratorBegin();
		it != alg->getGMObservationIteratorEnd(); it++)
    {
	gm_observations.push_back(*it);
    }

    // replace GMPredictions
    gm_predictions.clear();
    for(GMPredictionIter it = alg->getGMPredictionIteratorBegin();
		it != alg->getGMPredictionIteratorEnd(); it++)
    {
	gm_predictions.push_back(*it);
    }

    return s;
}

void AlgMessage::coutPrint(bool long_form)
{
    (void)long_form;
    cout << toString() << endl;
}

string AlgMessage::toString(bool long_form)
{
    string s = CoreEventInfo::toString(long_form);
    string obs = obsToString();
    string pred = predToString();
    if(obs.length() > 0) s = s + "\n" + obs;
    if(pred.length() > 0) s = s + "\n" + pred;
    return s;
}

string AlgMessage::obsToString()
{
    stringstream s;
    GMObservationIter iv = gm_observations.begin();
    GMObservationIter ev = gm_observations.end();

    if((int)gm_observations.size() > 0) {
	s << getSystemNameString() << " Observations Start:" << endl;
	for (; iv != ev; iv++) { 
	    s << iv->toString();
	}
	s << getSystemNameString() << " Observations End";
    }
    return s.str();
}

string AlgMessage::predToString()
{
    stringstream s;
    GMPredictionIter iv = gm_predictions.begin();
    GMPredictionIter ev = gm_predictions.end();
    if(gm_predictions.size() > 0) {
        s << getSystemNameString() << " Predictions Start:" << endl;
        for (; iv != ev; iv++) {
            s << iv->toString();
        }
        s << getSystemNameString() << " Predictions End";
    }
    return s.str();
}
