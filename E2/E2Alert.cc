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
/**
 * \file   E2Alert.cc
 *
 * \author Holly, <hollybrown@berkeley.edu>
 *
 * \date   2010/07/15 Created
 * \date   2010/09/16 Updated to include alert history
 * \date   2012 modified by <henson@seismo.berkeley.edu>
 *
 * \brief  Determines whether an event justifies an alert message, and sends that message to the Decision Module. 
 */
#include <math.h>
#include <sstream>
#include <iomanip>
#include <set>
#include <plog/Log.h>
#include "qlib2.h"
#include "E2Alert.h"
#include "E2Event.h"
#include "E2ModuleManager.h"
#include "E2TriggerManager.h"
#include "TimeStamp.h"
#include "TimeString.h"
#include "EWPacket.h"
#include "deltaz.h"
#include "E2Region.h"
#include "E2Prop.h"

using namespace std;

extern string program_version;
extern string program_instance;

static bool teleseism(E2Event *event, double *dif, double *tpaver, double *pdaver);
static int verbose;

double E2Alert::minmag = 2.0;    // minimum magnitude to consider
double E2Alert::maxmag = 10.0;   // maximum magnitude to consider
int E2Alert::mintrigs = 4;       // minimum number of triggers 
int E2Alert::minstats = 4;       // minimum number of stations 
double E2Alert::min_az_span = 0.0;  // minimum azimuth span (360 - largest station azimuth gap)
bool E2Alert::test_teleseism = true;
// log10(pdaverage) = tele_filter_intercept + tele_filter_slope * log10(tpaverage)
// If below this line, then classify as a teleseism.
double E2Alert::tele_filter_intercept = -3.728;
double E2Alert::tele_filter_slope = 2.817;
bool E2Alert::useTF = true;
bool E2Alert::tf_event_reverse = false;
int E2Alert::tf_disagree_sta = 1;
int E2Alert::tf_num_sta_required = 2;

list<E2Region> E2Alert::minStaRegions;


E2Alert::E2Alert(E2TriggerManager *trigm, E2EventManager *evtm) throw(Error) : tm(trigm), em(evtm)
{
    send_message = E2Prop::getBool("SendMessage");
    send_email = E2Prop::getBool("SendEmail");
    if(send_email) {
	email_to = E2Prop::getString("Emailto");
	mailx = E2Prop::getString("Mailx");
	e2email = E2Prop::getString("E2Email");
	tmpdir = E2Prop::getString("TmpDir");
    }
    send_cancel_message = E2Prop::getBool("SendCancelMessage");
    test_teleseism = E2Prop::getBool("TestTeleseism");
    tele_filter_intercept = E2Prop::getDouble("TeleFilterIntercept");
    tele_filter_slope = E2Prop::getDouble("TeleFilterSlope");
    minmag = E2Prop::getDouble("AlertMinMag");
    maxmag = E2Prop::getDouble("AlertMaxMag");
    mintrigs = E2Prop::getInt("AlertMinTrigs");
    minstats = E2Prop::getInt("AlertMinStats");
    alert_duration = E2Prop::getDouble("AlertDuration");
    min_az_span = E2Prop::getDouble("AlertMinAzSpan");
    useTF = E2Prop::getBool("UseTF");
    if(useTF) {
	tf_event_reverse = E2Prop::getBool("TFEventReverse");
	tf_disagree_sta = E2Prop::getInt("TFDisagreeSta");
	tf_num_sta_required = E2Prop::getInt("TFNumStaRequired");
    }
    verbose = E2Prop::getInt("Verbose");

    E2ModuleManager::param_str << fixed << setprecision(2);
    E2ModuleManager::param_str << "P: E2Alert.SendMessage: " << send_message << endl;
    E2ModuleManager::param_str << "P: E2Alert.SendEmail: " << send_email << endl;
    if(send_email) {
	E2ModuleManager::param_str << "P: E2Alert.Mailx: " << mailx << endl;
	E2ModuleManager::param_str << "P: E2Alert.Emailto: " << email_to << endl;
	E2ModuleManager::param_str << "P: E2Alert.E2Email: " << e2email << endl;
	E2ModuleManager::param_str << "P: E2Alert.TmpDir: " << tmpdir << endl;
    }
    E2ModuleManager::param_str << "P: E2Alert.SendCancelMessage: " << send_cancel_message << endl;
    E2ModuleManager::param_str << "P: E2Alert.TestTeleseism: " << test_teleseism << endl;
    E2ModuleManager::param_str << "P: E2Alert.TeleFilterIntercept: " << tele_filter_intercept << endl;
    E2ModuleManager::param_str << "P: E2Alert.TeleFilterSlope: " << tele_filter_slope << endl;
    E2ModuleManager::param_str << "P: E2Alert.AlertMinMag: " << minmag << endl;
    E2ModuleManager::param_str << "P: E2Alert.AlertMaxMag: " << maxmag << endl;
    E2ModuleManager::param_str << "P: E2Alert.AlertMinTrigs: " << mintrigs << endl;
    E2ModuleManager::param_str << "P: E2Alert.AlertMinStats: " << minstats << endl;
    E2ModuleManager::param_str << "P: E2Alert.AlertMinAzSpan: " << min_az_span << endl;
    E2ModuleManager::param_str << "P: E2Alert.AlertDuration: " << alert_duration << endl;
    E2ModuleManager::param_str << "P: E2Alert.UseTF: " << useTF << endl;
    if(useTF) {
	E2ModuleManager::param_str << "P: E2Alert.TFEventReverse: " << tf_event_reverse << endl;
	E2ModuleManager::param_str << "P: E2Alert.TFDisagreeSta: " << tf_disagree_sta << endl;
	E2ModuleManager::param_str << "P: E2Alert.TFNumStaRequired: " << tf_num_sta_required << endl;
    }
    if(send_message && alert_duration < 10.) {
	E2ModuleManager::param_str << "W: E2Alert: invalid AlertDuration. must be >= 10.0 ";
	LOG_INFO << E2ModuleManager::param_str.str();
	throw(Error("E2Alert: invalid AlertDuration. must be >= 10.0"));
    }

    E2Region::getRegions(string("AlertMinStaRegion"), 1, minStaRegions);
}

void E2Alert::alert(int evid)
{
    E2Event *event = em->getEvent(evid);

    if(event == NULL) return;

    checkAlertCriteria(event, tm);

    event->alert_time = TimeStamp::current_time().ts_as_double(UNIX_TIME);

    if(event->alert_ok) {
	sendAlert(event);
	sendEmail(event);
    }
    else {
	// check if we need to send a cancel message
	checkSendCancel(event);
    }

    event->logMessages();

    event->incrementVersion();
}

bool E2Alert::checkAlertCriteria(E2Event *event, E2TriggerManager *tm)
{
    event->alert_ok = true;
    if(!event->split_ok) {
	event->alert_ok = false;
    }
    event->nS = countStations(event, tm);

    // check if magnitude is outside of acceptable range
    event->alert_mag_ok = true;
    if(event->evmag < minmag || event->evmag > maxmag)
    {
	event->alert_ok = false;
	event->alert_mag_ok = false;
    }
    event->mag_checked = event->evmag;

    // check for required number of triggers and stations
    event->alert_nsta_ok = true;
    if((int)event->triggers.size() < mintrigs || event->nS < minstats
		|| !checkAlertRegions(event, event->nS))
    {
	event->alert_ok = false;
	event->alert_nsta_ok = false;
    }

    // check that the location was not on the edge of the grid
    event->alert_loc_ok = true;
    if(event->nS >= 3 && event->misfit_rms < 0.)
    {
	event->alert_ok = false;
	event->alert_loc_ok = false;
    }

    // check that the event azimuth span is acceptible
    event->alert_az_ok = true;
    if(event->az_span < min_az_span) {
	event->alert_ok = false;
	event->alert_az_ok = false;
    }

    event->alert_teleseism_ok = true;
    if( test_teleseism && teleseism(event, &event->teleseism_dif, &event->tpaver, &event->pdaver) )
    {
	event->alert_ok = false;
	event->alert_teleseism_ok = false;
    }

    if(useTF && event->tele_fbands != 0) { // if the event has not been declared non-teleseismic
	// if the event has been declared teleseismic and cannot be reversed
	if(event->tele_fbands == 1 && !tf_event_reverse) {
	    event->alert_ok = false;
	}
	else if(event->tele_fbands >= 1) {
	    checkTeleFBands(event);
	}
	if(event->tele_fbands == 1) {
	    event->alert_ok = false;
	}
    }

    event->nT = event->triggers.size();

    set<string> stations;
    for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++) {
	stations.insert((*it)->getSta());
    }
    if((int)stations.size() < minstats) {
	event->alert_ok = false;
    }

    setUncertainties(event);

    return event->alert_ok;
}

static bool sortByDist(E2Trigger *a, E2Trigger *b) { return (a->distkm < b->distkm); }

void E2Alert::checkTeleFBands(E2Event *event)
{
    // event->tele_fbands:
	// 0: have >= 2 stations that are NON teleseismic
	// 1: have >= 2 stations that are teleseismic
	// 2: waiting for >= 2 stations measured at TFMinWindowIndex
	// 3: have < 2 stations with lead >= 30 secs and distance < tfMaxStaDist
    set<string> sta_ready; // station with trigger tf <= 1
    set<string> sta_tele;
    set<string> sta_non_tele;
    vector<E2Trigger *> trigs;
    bool bad_distance = false;

    // count the number of stations that have a trigger with tele_fbands=0 or 1 (not 2), and
    // the station distance is < tfMaxStaDist
    // (t->tele_fbands= 0: trigger is not teleseismic; 1: it is teleseismic; 2: waiting for more data)
    for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++) {
	E2Trigger *t = (*it);
	t->distkm = distancekm(event->lat, event->lon, t->getLat(), t->getLon());
	trigs.push_back(t);

	if(t->tele_fbands <= 1 && t->distkm <= E2TriggerManager::tfMaxStaDist) {
	    sta_ready.insert(t->getSta());
	}
//	if(t->distkm > E2TriggerManager::tfMaxStaDist) bad_distance = true;
    }
    // if there are less that tf_num_sta_required triggers with a designated teleseismic status
    if((int)sta_ready.size() < tf_num_sta_required) {
	if(!bad_distance) { // if none of the triggers had a bad distance (too far away)
	    // then can wait for more trigger updates as more waveform data is processed
	    event->tele_fbands = 2; // waiting, block the alert for this iteration
	    event->alert_ok = false;
	}
	else {
	    // at least one station is already too distant. Might not get more waveform data that will
	    // change the status. Flag the event as undecided and do not stop a possible alert
	    event->tele_fbands = 3; // cannot decide
	}
	return;
    }

    std::sort(trigs.begin(), trigs.end(), sortByDist);

    for(vector<E2Trigger *>::iterator it = trigs.begin(); it != trigs.end(); it++) {
	// trigger->tele_fbands:
	// 0: measured at TFMinWindowIndex secs and is NON teleseismic
	// 1: measured at TFMinWindowIndex secs and is teleseismic
	// 2: Waiting to measure at TFMinWindowIndex
	// 3: lag >= TFMinWindowIndex secs but lead < 30 secs. Cannot measure.
	E2Trigger *t = (*it);

	if(t->tele_fbands <= 1 && t->distkm <= E2TriggerManager::tfMaxStaDist) {
	    if(t->tele_fbands == 0) sta_non_tele.insert(t->getSta());
	    else if(t->tele_fbands == 1) sta_tele.insert(t->getSta());
	}
	int n = 0;
	// count the number of stations in sta_tele with either tf_disagree_sta=1 or no channel in sta_non_tele
	for(set<string>::iterator it = sta_tele.begin(); it != sta_tele.end(); it++) {
	    if(tf_disagree_sta==1 || sta_non_tele.find(*it) == sta_non_tele.end()) n++;
	}
	if(n >= tf_num_sta_required) {
	    event->tele_fbands = 1; // two stations with TFMinWindowIndex seconds are teleseismic
	    event->alert_ok = false;
	    return;
	}
	n = 0;
	// count the number of stations in sta_non_tele and either tf_disagree_sta=0 or no channel in sta_tele
	for(set<string>::iterator it = sta_non_tele.begin(); it != sta_non_tele.end(); it++) {
	    if(tf_disagree_sta==0 || sta_tele.find(*it) == sta_tele.end()) n++;
	}
	if(n >= tf_num_sta_required) {
	    event->tele_fbands = 0; // two stations with TFMinWindowIndex seconds are non-teleseismic
	    return;
	}
    }
    // cannot make a decision yet. block the alert for this iteration
    event->tele_fbands = 2;
    event->alert_ok = false;
}

void E2Alert::checkSendCancel(E2Event *ev)
{
    if(send_cancel_message) {
	// check if already cancelled
	for(list<EventCore>::iterator it=ev->history.begin(); it != ev->history.end(); it++) {
	    if((*it).cancel_sent) return;
	}
	// check if alert sent
	for(list<EventCore>::iterator it=ev->history.begin(); it != ev->history.end(); it++) {
	    if((*it).alert_sent) {
		sendCancelMsg(ev);
		return;
	    }
	}
    }
}

void E2Alert::sendAlert(E2Event *ev)
{
    if(!ev->alert_ok || ev->alert_sent) return;

    int msg_version = 0;
    double first_alert_time = 0., last_alert_time = 0.;
    for(list<EventCore>::iterator it=ev->history.begin(); it != ev->history.end(); it++) {
	if( (*it).alert_sent ) {
	    if(msg_version == 0) { // get first alert time
		first_alert_time = (*it).alert_time;
	    }
	    msg_version++;
	    last_alert_time = (*it).alert_time;
	}
    }

    double alert_time = TimeStamp::current_time().ts_as_double(UNIX_TIME);

    // send alerts for the first AlertDuration seconds only.
    if(first_alert_time > 0. && (alert_time - first_alert_time) > alert_duration) return;

    // do not update more than once a second
    if(msg_version == 0 || (alert_time - last_alert_time >= 1.0)) {
	ev->alert_sent = true;
	ev->alert_time = alert_time;

	stringstream msg_str;

	EpicMessage em(
		ev->eventid,	// ID
		ev->evmag,	// magnitude
		ev->mag_uncer,	// magnitude uncertainty
		ev->lat,  	// latitude
		ev->lat_uncer,	// latitude uncertainty
		ev->lon,	// longitude
		ev->lon_uncer,	// longitude uncertainty
		ev->depth,	// depth
		ev->depth_uncer,// depth uncertainty
		nepoch_to_tepoch(ev->time),	// leap second time for dm_lib
		ev->time_uncer,	// time uncertainty
		ev->likelihood,	// likelihood
		msg_version ? UPDATE:NEW, // message type
		msg_version,		  // message version
		LIVE);			  // message category (LIVE or TEST)
	em.setAlgorithmVersion(program_version);
	em.setProgramInstance(program_instance);
	em.setNumberStations(ev->stationCount());

	msg_str << endl << "Alert Message Sent for Event " << ev->eventid << endl;
	if(send_message) {
	    msg_str <<  E2ModuleManager::getInstance()->getMessageSender()->sendMessage(em) << endl;
	}
	else {
	    DMMessageEncoder encoder;
	    msg_str << encoder.encodeMessage(&em) << endl;
	}

	ev->alert_msg = msg_str.str();
    }
}

void E2Alert::sendCancelMsg(E2Event *ev)
{
    stringstream msg_str;
    char s[20];
    // check if already cancelled
    for(list<EventCore>::iterator it=ev->history.begin(); it != ev->history.end(); it++) {
	if((*it).cancel_sent) return;
    }

    // set status for current history
    ev->cancel_sent = true;

    int msg_version = 0;

    for(list<EventCore>::iterator it=ev->history.begin(); it != ev->history.end(); it++) {
	if( (*it).alert_sent ) msg_version++;
    }

    ev->alert_sent = true;
    ev->alert_time = TimeStamp::current_time().ts_as_double(UNIX_TIME);

    EpicMessage em(
		ev->eventid,	// ID
		ev->evmag,	// magnitude
		ev->mag_uncer,	// magnitude uncertainty
		ev->lat,  	// latitude
		ev->lat_uncer,	// latitude uncertainty
		ev->lon,	// longitude
		ev->lon_uncer,	// longitude uncertainty
		ev->depth,	// depth
		ev->depth_uncer,// depth uncertainty
		nepoch_to_tepoch(ev->time),	// leap second time for dmlib
		ev->time_uncer,	// time uncertainty
		ev->likelihood,	// likelihood
		DELETE,		// message type
		msg_version,	// message version
		LIVE);		// message category (LIVE or TEST)
    em.setAlgorithmVersion(program_version);
    em.setProgramInstance(program_instance);
    em.setNumberStations(ev->stationCount());

    msg_str << endl << "M: Alert Message Sent for Event " << ev->eventid << endl;
    if(send_message) {
	msg_str <<  E2ModuleManager::getInstance()->getMessageSender()->sendMessage(em) << endl;
    }
    else {
	DMMessageEncoder encoder;
	msg_str << encoder.encodeMessage(&em) << endl;
    }

    msg_str << em.labelString() << "  NSTA" << endl;
    snprintf(s, sizeof(s), "E%02d ", msg_version);
    msg_str << s << em.toString();
    snprintf(s, sizeof(s), " %5d", ev->stationCount());
    msg_str << s;

    LOG_INFO << msg_str.str();
    msg_str << endl;
    E2ModuleManager::sendLogMsg(E2Event::logTime());
    E2ModuleManager::sendLogMsg(msg_str.str());
}

static bool sortTrigs(E2Trigger *a, E2Trigger *b)
{
    return (a->getTime() < b->getTime());
}

void E2Alert::sendEmail(E2Event *ev)
{
    FILE *fp;
    char s[500];
    int num_sent = 0;

    // don't send emails if not sending out event messages or if the data source is a file
    if(!send_email || !send_message) return;

    for(list<EventCore>::iterator i=ev->history.begin(); i != ev->history.end(); i++) {
	if( (*i).email_sent ) num_sent++;
    }
    // only send email once
    if(num_sent >= 1) return;

    ev->email_sent = true;

    string alert_time = TimeString::toString(ev->alert_time);
    string otime = TimeString::toString(ev->time);

    // 2. Message body
    stringstream body;
    if(num_sent > 1) {
	body << "UPDATED ALERT: \n";
    }
    body <<  "        ID      Alert UTC Time      Origin UTC Time  Mag TpMag PdMag     Lat      Lon\n";

    snprintf(s, sizeof(s), "%10d %s  %s %4.1f  %4.1f  %4.1f %7.3f %8.3f\n", ev->eventid, alert_time.c_str(),
		otime.c_str(), ev->evmag, ev->tpmag, ev->pdmag, ev->lat, ev->lon);
    body << s;

    set<string> sta35, sta50, sta100;
    // Count stations within distances
    for(set<E2Trigger *>::iterator it = ev->triggers.begin(); it != ev->triggers.end(); it++) {
	E2Trigger *t = (*it);
	double dist = distancekm(ev->lat, ev->lon, t->getLat(), t->getLon());
	if(dist <= 35) sta35.insert(t->getSta());
	if(dist <= 50) sta50.insert(t->getSta());
	if(dist <= 100) sta100.insert(t->getSta());
    }
    int ntrigs_35km = (int)sta35.size();
    int ntrigs_50km = (int)sta50.size();
    int ntrigs_100km = (int)sta100.size();

    if(ntrigs_100km > ntrigs_50km) {
	body << "\n     Trigs Stas %assoc Stas<35km Stas<50km Stas<100km\n";
	snprintf(s, sizeof(s), "     %5d %4d  %5.1f %9d %9d  %9d\n", ev->nT, ev->nS,
		ev->percent, ntrigs_35km, ntrigs_50km, ntrigs_100km);
    }
    else if(ntrigs_50km > ntrigs_35km) {
	body << "\n     Trigs Stas %assoc Stas<35km Stas<50km\n";
	snprintf(s, sizeof(s), "     %5d %4d  %5.1f %9d %9d\n", ev->nT, ev->nS,
		ev->percent, ntrigs_35km, ntrigs_50km);
    }
    else {
	body << "\n     Trigs Stas %assoc Stas<35km\n";
	snprintf(s, sizeof(s), "     %5d %4d  %5.1f %9d\n", ev->nT, ev->nS,
		ev->percent, ntrigs_35km);
    }

    body << s;

    body << "\n http://maps.google.com?q=@" << ev->lat << "," << ev->lon;

    // 3. Subject
    if(num_sent > 1) {
	snprintf(s, sizeof(s), "\"E2 Update: M %.2f, nT %d, %.3f, %.3f, %s\"",
		ev->evmag, ev->nT, ev->lat, ev->lon, otime.c_str());
    }
    else { 
	snprintf(s, sizeof(s), "\"E2 ALERT: M %.2f, nT %d, %.3f, %.3f, %s\"",
		ev->evmag, ev->nT, ev->lat, ev->lon, otime.c_str());
    }
    string subject = s;

    string recipient = email_to;
    string tmpfile = tmpdir + "/" + "E2tmp";

    if((fp = fopen(tmpfile.c_str(), "w")) == NULL) {
	stringstream msg_str;
        msg_str << "W: Cannot open tmp file " << tmpfile << endl << strerror(errno);
	LOG_INFO << msg_str.str();
        msg_str << endl;
	E2ModuleManager::sendLogMsg(E2Event::logTime());
	E2ModuleManager::sendLogMsg(msg_str.str());
        string cmd = string(mailx + " -s") + " " + subject + " " + recipient + string(" <<EOM\n")
                        + body.str() + string("\nEOM\n");
	system(cmd.c_str());
        return;
    }

    char labels[] = "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    vector<E2Trigger *> trigs;
    for(set<E2Trigger *>::iterator jt = ev->triggers.begin(); jt != ev->triggers.end(); jt++) {
	int i;
	for(i = 0; i < (int)trigs.size() && trigs[i]->staId() != (*jt)->staId(); i++);
	if(i == (int)trigs.size()) {
	    trigs.push_back(*jt);
	}
    }
    // sort by time
    sort(trigs.begin(), trigs.end(), sortTrigs);

    // make url for google static map
    fprintf(fp,
"http://maps.googleapis.com/maps/api/staticmap?size=300x300&maptype=hybrid&markers=size:mid%%7Ccolor:red%%7C%6.3f,%8.3f", ev->lat, ev->lon);
    for(int i = 0; i < (int)trigs.size() && i < 35; i++) {
	// 32 (is %7C = '|' one char or 2 or 3?) characters per marker
	fprintf(fp, "&markers=size:mid%%7Clabel:%c%%7C%6.3f,%8.3f", labels[i], trigs[i]->getLat(), trigs[i]->getLon());
    }
    fprintf(fp, "&sensor=false\n");
    fprintf(fp, "%s\n\n", body.str().c_str());

    fprintf(fp, "    sta  net chn         time   tpmag   pdmag\n");

    for(int i = 0; i < (int)trigs.size(); i++) {
        char c = (i < 35) ? labels[i] : ' ';
	for(set<E2Trigger *>::iterator jt = ev->triggers.begin(); jt != ev->triggers.end(); jt++) {
	    if(trigs[i]->staId() == (*jt)->staId()) {
		fprintf(fp, "%s\n", (*jt)->toEmailString(c).c_str());
	    }
	}
    }
    fclose(fp);

    string cmd = e2email + " " + tmpdir + " " + recipient + " " + subject;

    system(cmd.c_str());
}

/* Sent updates that were delayed because they were within one second of the previous update.
 */
void E2Alert::sendDelayedUpdates()
{
    EventList *evlist = em->getEventList();

    for(EventList::iterator ev = evlist->begin(); ev != evlist->end(); ev++) {
	// Only send the most recent update for each event.
	// No need to send earlier updates
	list<EventCore>::reverse_iterator rit = (*ev)->history.rbegin();
	if(rit != (*ev)->history.rend() && (*rit).alert_ok && !(*rit).alert_sent) {
	    sendAlert(*ev);
	}
    }
}

/* Sent alerts for events that intially failed the alert criteria
 */
void E2Alert::sendDelayedAlerts()
{
    EventList *evlist = em->getEventList();

    for(EventList::iterator ev = evlist->begin(); ev != evlist->end(); ev++) {
	bool alert_sent = false;
	for(list<EventCore>::iterator it=(*ev)->history.begin(); it != (*ev)->history.end(); it++) {
	    if( (*it).alert_sent ) {
		alert_sent = true;
		break;
	    }
	}
	if(!alert_sent) {
	    checkAlertCriteria(*ev, tm);
	    if((*ev)->alert_ok) {
		alert((*ev)->eventid);
	    }
	}
    }
}

static bool teleseism(E2Event *event, double *dif, double *tpaver, double *pdaver)
{
    // define parameters for log10(Tp), log10(Pd) line
    // intercept = -3.728, slope = 2.817;
    double tpaverage, pdaverage;
    int num_with_taup = 0;

    *dif = *tpaver = *pdaver = 0.;
    tpaverage = 0.;
    pdaverage = 0.;
    for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++)
    {
	// Taup will be zero until 0.5 seconds after the trigger
	if(fabs((*it)->getTaup()) > 0.0 && (*it)->tpmag_ok && (*it)->tpSNRok && (*it)->pdmag_ok
		&& (*it)->pdSNRok && (*it)->magdistok)
	{
	    num_with_taup++;
	    tpaverage += fabs((*it)->getTaup());
	    pdaverage += fabs((*it)->getPd());
        }
    }
    if(num_with_taup == 0 || tpaverage == 0. || pdaverage == 0.) return false;

    tpaverage /= (double)num_with_taup;
    pdaverage /= (double)num_with_taup;

    // cutoff line for (log10(tpaverage), log10(pdaverage))
    // log10(pdaverage) = -3.728 + 2.817 * log10(tpaverage)
    // If below this line, then classify as a teleseism.
    *pdaver = log10(pdaverage);
    *tpaver = log10(tpaverage);
    *dif = *pdaver - (E2Alert::tele_filter_intercept + E2Alert::tele_filter_slope * *tpaver);

    return ( *pdaver < E2Alert::tele_filter_intercept + E2Alert::tele_filter_slope * *tpaver );
}

int E2Alert::countStations(E2Event *ev, E2TriggerManager *tm)
{
    // Collect active clusters. If cluster center is greater than cluster.min_distkm from the event location,
    // count the cluster stations as one station.
    vector<Cluster> clusters;
    for(map<string, Cluster>::iterator jt = tm->clusters.begin(); jt != tm->clusters.end(); jt++) {
        double distkm = distancekm(ev->lat, ev->lon, (*jt).second.center_lat, (*jt).second.center_lon);
        if(distkm > (*jt).second.min_distkm) clusters.push_back((*jt).second);
    }

    map<int, E2Trigger *> trigger_sta;
    set<int> staIds;
    bool have_cluster = false;

    // count the triggered stations. Count active clusters as one triggered station.
    for(set<E2Trigger *>::iterator kt = ev->triggers.begin(); kt != ev->triggers.end(); kt++) {
	int sta_id = (*kt)->staId();
	staIds.insert(sta_id);
        // check for cluster stations
        bool in_cluster = false;
        for(vector<Cluster>::iterator it = clusters.begin(); it != clusters.end(); it++) {
            if((*it).sta_ids.find(sta_id) != (*it).sta_ids.end()) {
                in_cluster = true;
		have_cluster = true;
                trigger_sta[(*it).cluster_id] = *kt; // use cluster_id so stations in cluster are counted as one.
            }
        }
        if(!in_cluster) trigger_sta[sta_id] = *kt; // if station is not in a cluster, add its net.sta
    }

    if(!have_cluster && (int)trigger_sta.size() == 4) {
	// check for station-to-station distance < 5% of station-to-event
	for(map<int, E2Trigger *>::iterator kt = trigger_sta.begin(); kt != trigger_sta.end(); kt++) {
	    map<int, E2Trigger *>::iterator jt = kt;
	    for(jt++; jt != trigger_sta.end(); jt++) {
		E2Trigger *t = kt->second;
		E2Trigger *r = jt->second;
		double distkm = distancekm(t->lat, t->lon, r->lat, r->lon);
		if(distkm < 0.05*t->distkm) {
		    return 3;
		}
	    }
	}
    }

    return (int)trigger_sta.size();
}

void E2Alert::setUncertainties(E2Event *ev)
{
    double km_to_deg = 1./111.19492664;
    int nsta = ev->stationCount();

    if(nsta < 3) {
	ev->mag_uncer = -1.0;	// magnitude uncertainty
	ev->lat_uncer = -1.0;	// latitude uncertainty
	ev->lon_uncer = -1.0;	// longitude uncertainty
	ev->depth_uncer = -1.0; // depth uncertainty
	ev->time_uncer = -1.0;  // time uncertainty
	ev->likelihood = -1.0;  // likelihood
    }
    else if(nsta <= 15) {
	ev->mag_uncer = 2.366/pow(nsta, 1.849) + 0.2159;
	ev->lat_uncer = (100.0/pow(nsta, 1.642) + 4.0) * km_to_deg;
	ev->lon_uncer = (100.0/pow(nsta, 1.642) + 4.0) * km_to_deg;
	ev->depth_uncer = 5.0;
	ev->time_uncer =  50.53/pow(nsta, 2.293) + 0.4138;
	ev->likelihood = (-0.08262*nsta*nsta + 2.396*nsta + 82.65)/100.0;
    }
    else {
	ev->mag_uncer = 0.2317;
	ev->lat_uncer = 5.1718 * km_to_deg;
	ev->lon_uncer = 5.1718 * km_to_deg;
	ev->depth_uncer = 5.0;
	ev->time_uncer = 0.5154;
	ev->likelihood = 1.00;
    }
}

bool E2Alert::checkAlertRegions(E2Event *event, int nsta)
{
    for(list<E2Region>::iterator it = E2Alert::minStaRegions.begin(); it != E2Alert::minStaRegions.end(); it++) {
	if((*it).inRegion(event->lat, event->lon)) {
	    return (nsta >= (*it).getIntParam());
	}
    }
    return true;
}
