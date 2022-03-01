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
#ifndef _EventCore_H__
#define	_EventCore_H__
#include <iostream>

typedef enum {
    MULTI_STATION_EVENT  = 1,
    TWO_STATION_EVENT    = 2,
    SINGLE_STATION_EVENT = 3
} EvType;

class EventCore
{
  public:
    int eventid;
    int version;
    double lat;
    double lon;
    double depth;
    double time;
    double tpmag;
    double pdmag;
    double evmag;
    double lat_uncer;
    double lon_uncer;
    double depth_uncer;
    double mag_uncer;
    double time_uncer;
    double likelihood;

    int ntrig_build;
    int nsta_build;
    int nT;
    int nS;

    double misfit_ave;
    double misfit_rms;
    bool misfit_ok;
    bool split_ok;

    int num_near;
    int num_sta_trig;
    int num_active;
    int num_inactive;
    int nsta;

    double percent;
    bool percent_ok;
    bool dist_ok;
    double az_span;

    bool alert_mag_ok;
    bool alert_nsta_ok;
    bool alert_loc_ok;
    bool alert_az_ok;
    double teleseism_dif;
    double tpaver;
    double pdaver;
    bool alert_teleseism_ok;
    int tele_fbands;
    bool alert_ok;
    bool alert_sent;
    double alert_time;

    EvType type;
    int algorithm;
    double mag_checked;
    bool located;
    bool cancel_sent;
    bool email_sent;
    std::string alert_msg;

    EventCore() :
	eventid(0), version(0), lat(0.), lon(0.), depth(0.), time(0.), tpmag(0), pdmag(0.), evmag(0.),
	lat_uncer(-1.0), lon_uncer(-1.0), depth_uncer(-1.0), mag_uncer(-1.0), time_uncer(-1.0),
	likelihood(-1.0), ntrig_build(0), nsta_build(0), nT(0), nS(0), misfit_ave(0.), misfit_rms(0.),
	misfit_ok(false), split_ok(false), num_near(0), num_sta_trig(0), num_active(0), num_inactive(0), nsta(0),
	percent(0.), percent_ok(false), dist_ok(false), az_span(0.), alert_mag_ok(false),
	alert_nsta_ok(false), alert_loc_ok(false), alert_az_ok(false), teleseism_dif(0.), tpaver(0.), pdaver(0.),
	alert_teleseism_ok(false), tele_fbands(2), alert_ok(false), alert_sent(false), alert_time(0.),
	type(MULTI_STATION_EVENT), algorithm(0), mag_checked(0.), located(false), cancel_sent(false),
	email_sent(false) {}

	std::string toEmailString(int ntrigs_35km, int ntrigs_50km, int ntrigs_100km);
};

#endif
