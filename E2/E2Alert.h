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
#ifndef __E2Alert_h__
#define __E2Alert_h__

/**
 * \file   E2Alert.h
 *
 * \author Holly, <hollybrown@berkeley.edu>
 *
 * \date   2010/07/15 created 
 * \date   2010/09/16 updated to include AlertHistory 
 * \date   2012 modified by <henson@seismo.berkeley.edu>
 *
 * \brief  Determines whether to send an alert message for an event, and sends the alert. 
 *
 */
#include <vector>
#include <list>
#include <sstream>
#include "E2Region.h"
#include "E2Reader.h"
#include "Exceptions.h"

class E2Event;
class E2TriggerManager;
class E2EventManager;

#define FAILED_EVMAG            0x1
#define FAILED_MIN_STA          0x2
#define FAILED_MIN_PDMAG        0x4
#define FAILED_MIN_MAGDIF       0x8
#define FAILED_LOCATION         0x10
#define FAILED_TELESEISM        0x20
#define FAILED_CANCELMSG        0x40

class E2Alert
{
 private:
    double updatemag;	// change in magnitude that sends an alert update
    double updateloc;	// change in location that sends an alert update
    double alert_duration; // stop sending alerts when alert_time - first_alert_time > alert_duration
    bool send_message;
    bool send_email;
    bool send_cancel_message;
    std::string email_to;
    std::string mailx;
    std::string e2email;
    std::string tmpdir;

    E2TriggerManager *tm;
    E2EventManager *em;

    void sendAlert(E2Event *ev);
    void sendEmail(E2Event *ev);
    static void setUncertainties(E2Event *ev);
    void checkSendCancel(E2Event *ev);
    void sendCancelMsg(E2Event *ev);
    void getAlertRegions();
    static void checkTeleFBands(E2Event *event);
    static bool checkAlertRegions(E2Event *event, int nsta);

    static std::list<E2Region> minStaRegions;

 public:
    static double minmag;	// minimum magnitude to consider
    static double maxmag;	// maximum magnitude to consider
    static int mintrigs;	// minimum number of triggers 
    static int minstats;	// minimum number of stations 
    static double min_az_span;	// minumum station azimuth span
    static bool test_teleseism;
    static double tele_filter_intercept;// defines teleseismic filter cutoff-line
    static double tele_filter_slope;    // defines teleseismic filter cutoff-line
    static bool useTF;      		// Use FilterWindow information in TriggerParams class
    static bool tf_event_reverse;	// true: allow an event to change from teleseismic to non-teleseismic
					// false: do not allow an event to change from teleseismic to non-teleseismic
    static int tf_disagree_sta;		// 0: if channels at one station disagree, count the station non-teleseismic
    					// 1: if channels at one station disagree, count the station teleseismic
    static int tf_num_sta_required;	// number of stations required to be teleseismic or non-teleseismic before
					// the event is designated as teleseismic or non-teleseismic

    E2Alert() : updatemag(0.5), updateloc(35.0), send_message(false) {};

    E2Alert(E2TriggerManager *tm, E2EventManager *em) throw(Error);

    ~E2Alert() { }

    void alert( int evid );
    void sendDelayedAlerts();
    void sendDelayedUpdates();

    static int countStations(E2Event *ev, E2TriggerManager *tm);
    static bool checkAlertCriteria(E2Event *event, E2TriggerManager *tm);
    static bool sendLogMsg(std::string msg);
    static bool sendLogMsg(std::list<std::string> &msg);
    static bool sendLogMsg(std::string line_label, std::list<std::string> &msg);
};

#endif
