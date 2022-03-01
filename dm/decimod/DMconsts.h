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
#ifndef __DMconsts_h
#define __DMconsts_h

/** @addtogroup decimod 
 * @{
 */

typedef struct {
    int dmevent_timeout_secs;
    int algevent_timeout_secs;

    double max_misfit;
    double max_distkm_to_event;
    double max_time_to_event;

    double dm_min_lh;

    double min_mag;
    double max_mag;
    double max_mag_uncer;

    double max_lat_uncer;
    double max_lon_uncer;

    double max_otime_uncer;
    double default_otime_uncer;

    double min_depth_km;
    double max_depth_km;
    double max_depth_uncer;

    double min_likelihood;

    double two_alg_mag;
    double fault_info_min_mag;

    double change_threshold_mag;
    double change_threshold_mag_uncer;
    double change_threshold_lat;
    double change_threshold_lat_uncer;
    double change_threshold_lon;
    double change_threshold_lon_uncer;
    double change_threshold_depth;
    double change_threshold_depth_uncer;
    double change_threshold_otime;
    double change_threshold_otime_uncer;
    double change_threshold_lklhd;
    double change_threshold_num_stations;
} DMConstants;

const double NULL_MISFIT = 99999.9;

/** @name DMEvent republish threshold values.
 *  A DMEvent will be republished, if there is a change in a core value
 *  that is greater than the threshold value.
 */
//@{
const double CHANGE_THRESHOLD_MAG = 0.1;
const double CHANGE_THRESHOLD_MAG_UNCER = 0.01;
const double CHANGE_THRESHOLD_LAT = 0.01;
const double CHANGE_THRESHOLD_LAT_UNCER = 0.01;
const double CHANGE_THRESHOLD_LON = 0.01;
const double CHANGE_THRESHOLD_LON_UNCER = 0.01;
const double CHANGE_THRESHOLD_DEPTH = 0.5;
const double CHANGE_THRESHOLD_DEPTH_UNCER = 1.0;
const double CHANGE_THRESHOLD_OTIME = 0.2;
const double CHANGE_THRESHOLD_OTIME_UNCER = 0.1;
const double CHANGE_THRESHOLD_LKLHD = 0.01;
const double CHANGE_THRESHOLD_NUM_STATIONS = 0.5;
//@}

#endif
