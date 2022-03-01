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
#include "GPSlipMessage.h"

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
GPSlipMessage::GPSlipMessage(enum FaultSegment::FaultSegmentShape shape,
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
			     string mag_units,
			     string mag_uncer_units,
			     string lat_units,
			     string lat_uncer_units,
			     string lon_units,
			     string lon_uncer_units,
			     string dep_units,
			     string dep_uncer_units,
			     string o_time_units,
			     string o_time_uncer_units) : FiniteFaultMessage(GPSLIP,
                                    shape,
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
									mag_units,
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

GPSlipMessage::GPSlipMessage(enum FaultSegment::FaultSegmentShape shape,
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
			     string mag_units,
			     string mag_uncer_units,
			     string lat_units,
			     string lat_uncer_units,
			     string lon_units,
			     string lon_uncer_units,
			     string dep_units,
			     string dep_uncer_units,
			     string o_time_units,
			     string o_time_uncer_units) : FiniteFaultMessage(GPSLIP,
                                    shape,
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
									mag_units,
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

GPSlipMessage::~GPSlipMessage() 
{
}
