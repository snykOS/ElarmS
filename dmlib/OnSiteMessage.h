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
#ifndef __OnSiteMessage_h
#define __OnSiteMessage_h

/** \deprecated
 *  \page onsite_page OnSite Example XML Message
 *  \section onsite_sec An Example OnSite Message
 *  The <b>onsite_info</b> node will only be present when there are predictions.
 *  \include onsite.xml
 */

#include "AlgMessage.h"

/** A CoreEventInfo subclass for the OnSite event messages.
 *  @ingroup deprecated
 */
class OnSiteMessage : public AlgMessage
{
 public:
    OnSiteMessage(string id,
		  double mag=-9.9,
		  double mag_uncer=-9.9,
		  double lat=-99.9,
		  double lat_uncer=-99.9,
		  double lon=-999.9,
		  double lon_uncer=-999.9,
		  double dep=-9.9,
		  double dep_uncer=-9.9,
		  double o_time=-99999.99,
		  double o_time_uncer=-9.9,
		  double likelihood=-9.9,
		  enum nudMessageType type = NEW,
		  int ver = 0,
		  enum MessageCategory category = LIVE,
		  string time_stamp = "",
		  string alg_ver = "-",
		  string instance = "",
		  int num_stations = 0,
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
    OnSiteMessage(int id,
		  double mag=-9.9,
		  double mag_uncer=-9.9,
		  double lat=-99.9,
		  double lat_uncer=-99.9,
		  double lon=-999.9,
		  double lon_uncer=-999.9,
		  double dep=-9.9,
		  double dep_uncer=-9.9,
		  double o_time=-99999.99,
		  double o_time_uncer=-9.9,
		  double likelihood=-9.9,
		  enum nudMessageType type = NEW,
		  int ver = 0,
		  enum MessageCategory category = LIVE,
		  string time_stamp = "",
		  string alg_ver = "-",
		  string instance = "",
		  int num_stations = 0,
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

    virtual ~OnSiteMessage();
};

#endif
