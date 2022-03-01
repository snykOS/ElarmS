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
#include "GMMessage.h"

/* Constructor */
// JA note 2017-10-27: This constructor sets the system_name to DM because the DMMessage 
// constructor automatically sets the system_name in the underlying AlgMessage/CoreEventInfo.
// This needs over-writing, but should it be done in constructor? I.e. call constructor with 
// sys_name as argument for example? Would require changes to DM constructor and GM constructor
// so I am not doing this now!
GMMessage::GMMessage(string i,
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
			     double lklyhd,
			     enum nudMessageType type,
			     int ver,
			     enum MessageCategory category,
			     string time_stamp,
			     string dmMsg_ver,
			     string instance,
			     int num_stations,
			     string mag_units,
			     string mag_uncer_units,
			     string lat_units,
			     string lat_uncer_units,
			     string lon_units,
			     string lon_uncer_units,
			     string dep_units,
			     string dep_uncer_units,
			     string o_time_units,
			     string o_time_uncer_units) : DMMessage(
								i, 
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
								lklyhd,
								type,
								ver,
								category,
								time_stamp,
								dmMsg_ver,
								instance,
								num_stations,
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
GMMessage::GMMessage(int i,
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
			     double lklyhd,
			     enum nudMessageType type,
			     int ver,
			     enum MessageCategory category,
			     string time_stamp,
			     string dmMsg_ver,
			     string instance,
			     int num_stations,
			     string mag_units,
			     string mag_uncer_units,
			     string lat_units,
			     string lat_uncer_units,
			     string lon_units,
			     string lon_uncer_units,
			     string dep_units,
			     string dep_uncer_units,
			     string o_time_units,
			     string o_time_uncer_units) : DMMessage(
								i, 
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
								lklyhd,
								type,
								ver,
								category,
								time_stamp,
								dmMsg_ver,
								instance,
								num_stations,
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

/* Destructor */
GMMessage::~GMMessage() 
{
    multiContours.clear();
}

/* Pushes back a new GMContour object into the private member multiContours (list of GMContours)
 * @param gmCont GMContour object to be pushed back into list container class member variable*/
void GMMessage::addGMContourPredictions(GMContour gmCont) {
    multiContours.push_back(gmCont);
} 

/* Add all components of DM message into GMMessage object: gm_observations, gm_predictions,
 * rupture_predictions, slip_predictions, and contributors
 * @param dmMsg pointer to DMMessage argument
 * @return bool indicating no change (false)  */
bool GMMessage::loadDMMessage(DMMessage * dmMsg)
{
    gm_observations.clear();
    gm_predictions.clear();
    contributors.clear();

    for(GMObservationIter it = dmMsg->getGMObservationIteratorBegin();
            it != dmMsg->getGMObservationIteratorEnd(); it++) {
        addGMObservation(*it);
    }

    for(GMPredictionIter it = dmMsg->getGMPredictionIteratorBegin();
            it != dmMsg->getGMPredictionIteratorEnd(); it++) {
        addGMPrediction(*it);
    }

    for (DMContributorIter it = dmMsg->getContributorBegin(); 
            it != dmMsg->getContributorEnd(); it++) {
        addContributor(*it);
    }
    addContributor(dmMsg);

    if(dmMsg->getNumberFiniteFaults() > 0) {
        list<FiniteFaultMessage>::iterator it;
        for(it = dmMsg->getFiniteFaultIteratorBegin(); it != dmMsg->getFiniteFaultIteratorEnd(); ++it) {
            finite_faults.push_back(*it);
        }
    }

    // currently not checking for changes in observations/predictions
    return false; // return no change
}

/* Creates a string with all contents of GMMessage object (including all DMMessage components as
 * well as GMContour and GMMap components) 
 * @param long_form a boolean carried over from argument to parent DMMessage class 
 *          toString() implementation
 * @return the string with all parts of GMMessage concatenated one after the other */
string GMMessage::toString(bool long_form)
{
    (void)long_form;
    stringstream s;
    
    s << std::endl << DMMessage::toString();

    //adding toString for contour list and map objects
    for (MultiContourIter it = multiContours.begin(); it != multiContours.end(); it++) {
        s << std::endl << it->toString(); 
    } 

    if (gmFullMap.getNumGridPts() > 0) {
        s << std::endl << gmFullMap.toString();
    }

    return s.str();
}
