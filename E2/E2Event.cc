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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <pthread.h>
#include "TimeStamp.h"
#include "TimeString.h"
#include "E2TriggerManager.h"
#include "E2ModuleManager.h"
#include "E2Location.h"
#include "E2Event.h"
#include "E2Prop.h"
#include "deltaz.h"
#include <plog/Log.h>

using namespace std;

static int verbose;
bool E2Event::get_properties = true;
bool E2Event::send_email = true;
string E2Event::mailx = "";
string E2Event::email_to = "";
double E2Event::logPercentMin = 20.0;
extern string program_version;
extern string program_instance;

E2Event::E2Event(int eventid, EvType type, double time, double lat, double lon, double depth)
{
    this->eventid = eventid;
    this->type = type;
    this->time = time,
    this->lat = lat;
    this->lon = lon;
    this->depth = depth;
    tele_fbands = 2;

    if(get_properties) {
	getProperties();
    }
}

E2Event::E2Event(const string &line) throw(Error)
{
    if(get_properties) {
	getProperties();
    }
    load(line);
}

void E2Event::getProperties()
{
    get_properties = false;
    verbose = E2Prop::getInt("Verbose");
    send_email = E2Prop::getBool("SendEmail");
    if(send_email) {
	email_to = E2Prop::getString("Emailto");
	mailx = E2Prop::getString("Mailx");
    }
    logPercentMin = E2Prop::getDouble("LogPercentMin");

    stringstream msg_str;
    msg_str << "P: E2Event.SendEmail: " << send_email << endl;
    if(send_email) {
	msg_str << "P: E2Event.Emailto: " << email_to << endl;
	msg_str << "P: E2Event.Mailx: " << mailx << endl;
    }
    char c[20];
    snprintf(c, sizeof(c), "%.1f", logPercentMin);
    msg_str << "P: E2Event.LogPercentMin: " << c;

    LOG_INFO << msg_str.str();
    msg_str << endl;
    E2ModuleManager::sendLogMsg(msg_str.str());
}

void E2Event::load(const string &line) throw(Error)
{
//                    time timeerr    lat laterr      lon lonerr depth deptherr likelihood
//E: 11-09-22 19:34:25.000    20.0 38.870   0.50 -122.670   0.50  8.00     5.00       0.80
    stringstream ss(line);
    string s;
    vector<string> tokens;

    while(ss >> s) tokens.push_back(s);

    if((int)tokens.size() < 10) {
        throw(Error("E2Event line format error: need 10 tokens after 'E:'"));
    }

    time = E2Trigger::stringToTime(tokens[0] + " " + tokens[1]);
    time_uncer = E2Trigger::stringToDouble(tokens[2], "time_err");
    lat = E2Trigger::stringToDouble(tokens[3], "lat");
    lat_uncer = E2Trigger::stringToDouble(tokens[4], "lat_err");
    lon = E2Trigger::stringToDouble(tokens[5], "lon");
    lon_uncer = E2Trigger::stringToDouble(tokens[6], "lon_err");
    depth = E2Trigger::stringToDouble(tokens[7], "depth");
    depth_uncer = E2Trigger::stringToDouble(tokens[8], "depth_err");
    likelihood = E2Trigger::stringToDouble(tokens[9], "uncertainty");
}

E2Event::~E2Event() 
{
    cleanup();
}

void E2Event::cleanup() 
{
    sendEmail();

    for(set<E2Trigger *>::iterator it = triggers.begin(); it != triggers.end(); it++) {
	(*it)->disassociate();
    }
}

void E2Event::sendEmail()
{
    ostringstream os;

    // only email the event history if messages are being sent to activemq and
    // the data source is activemq and not a file and an alert message has been
    // sent for this event

    if(send_email && alert_sent) {
	os << "E2 Alert history for Event ID: " << eventid << endl;
	os << "No.      Alert UTC Time      Origin UTC Time  Mag TpMag PdMag     Lat      Lon  nT  nS %stas 35km 50km 100km Alert" << endl;

	set<string> sta35, sta50, sta100;
	// Count stations within distances
	for(set<E2Trigger *>::iterator it = triggers.begin(); it != triggers.end(); it++) {
	    E2Trigger *t = (*it);
	    double dist = distancekm(lat, lon, t->getLat(), t->getLon());
	    if(dist <= 35) sta35.insert(t->getSta());
	    if(dist <= 50) sta50.insert(t->getSta());
	    if(dist <= 100) sta100.insert(t->getSta());
	}

	for( list<EventCore>::iterator it=history.begin(); it != history.end(); it++) {
	    os << (*it).toEmailString(sta35.size(), sta50.size(), sta100.size()) << endl;
	}
	string body(os.str());
 
	// subject
	ostringstream ss;
	ss << "\"E2 Alert History for Event " << eventid << "\""; 
	string subject(ss.str());

	// 4. Recipient email list
	string recipient = email_to;

	// 5. Build command string
	string cmd = string(mailx + " -s") + " " + subject + " " + recipient + string(" <<EOM\n")
			+ body + string("\nEOM\n");

	// 6. Execute (send) via shell
	system(cmd.c_str());
    }
}

string EventCore::toEmailString(int ntrigs_35km, int ntrigs_50km, int ntrigs_100km)
{
    char s[300];
    string ats, ots;

    ats = TimeString::toString(alert_time, 0);
    ots = TimeString::toString(time, 0);

    snprintf(s, sizeof(s), "%3d %s  %s %4.1f  %4.1f  %4.1f  %6.3f %8.3f %3d %3d %5.1f  %3d  %3d   %3d",
	version, ats.c_str(), ots.c_str(), evmag, tpmag, pdmag, lat, lon, nT, nS, percent,
	ntrigs_35km, ntrigs_50km, ntrigs_100km);

    if( alert_sent ) {
	snprintf(s+strlen(s), sizeof(s)-strlen(s), "   yes");
    }
    else {
	snprintf(s+strlen(s), sizeof(s)-strlen(s), "    no");
    }
    return string(s);
}

static bool sortAz(double a, double b) { return (a < b); }

void E2Event::addTrigger(E2Trigger *trigger, AssocCode assocCode)
{
    if(trigger->isAssociated()) {
	LOG_INFO << "W: E2Event::addTrigger: trigger already associated: " << trigger->toString();
	return;
    }
    if(assocCode != UNASSOCIATED) {
	trigger->associate(eventid, version, getAlertMessageVersion(), assocCode);
	if(trigger->getQualityCode() == OKAY) trigger->order = triggers.size() + 1;
    }
    triggers.insert(trigger);

    vector<double> az;
    // recompute az_span
    for(set<E2Trigger *, trigger_compare>::iterator it = triggers.begin(); it != triggers.end(); it++) {
	double delta, azimuth, back_azimuth;
	deltaz(lat, lon, (*it)->getLat(), (*it)->getLon(), &delta, &azimuth, &back_azimuth);
	if(azimuth != azimuth) {
	    // azimuth is NAN
	}
	else {
	    az.push_back(azimuth);
	}
    }
    az_span = 0.0;
    if(az.size() > 1) {
	// find largest azimuth gap
	std::sort(az.begin(), az.end(), sortAz);
	double az_gap = az[0] + 360. - az[az.size()-1];
	for(int i = 1; i < (int)az.size(); i++) {
	    double dif = az[i] - az[i-1];
	    if(dif > az_gap) az_gap = dif;
	}
	az_span = 360.0 - az_gap;
    }
}

// remove E2Trigger pointer from list
// do not free trigger memory
void E2Event::removeTrigger(E2Trigger *trigger, bool disassoc)
{
    set<E2Trigger *>::iterator it, it_tmp;

    for(it = triggers.begin(); it != triggers.end(); ) {
	if((*it) == trigger) {
	    if(disassoc) (*it)->disassociate();
	    it_tmp = it;
	    it++;
	    triggers.erase(it_tmp);
	}
	else {
	    it++;
	}
    }
}

string E2Event::toShortString()
{
    char s[200];
    string t = TimeString::toString(time, 3);
    snprintf(s, sizeof(s), "%10d %6.3f %8.3f %s", eventid, lat, lon, t.c_str());
    return string(s);
}

string E2Event::nowToString(int decimals)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now = (double)tv.tv_sec + (double)tv.tv_usec*1.e-06;
    return TimeString::toString(now, decimals);
}

string E2Event::logTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now = (double)tv.tv_sec + (double)tv.tv_usec*1.e-06;
    string s = TimeString::toString(now, 2);
    return string("GMT: ") + s + " " + program_version + " " + program_instance;
}

int E2Event::incrementVersion()
{
    version++;
    for(set<E2Trigger *>::iterator it = triggers.begin(); it != triggers.end(); it++) {
	(*it)->setEventVersion(version);
    }

    history.push_back(*(EventCore *)this);

    alert_mag_ok = false;
    alert_nsta_ok = false;
    alert_loc_ok = false;
    teleseism_dif = 0.;
    tpaver = 0.;
    pdaver = 0.;
    alert_teleseism_ok = false;
    alert_az_ok = false;
    alert_ok = false;
    alert_sent = false;
    alert_time = 0.;

    mag_checked = 0.;
    located = false;
    cancel_sent = false;
    email_sent = false;
    alert_msg = "";

    build_info.clear();
    loc_info.clear();
    assoc_info.clear();
    alert_msg.clear();
    cn.stas.clear();

    return version;
}

int E2Event::tmp_id = 0;

int E2Event::getTmpId()
{
    tmp_id--;
    if(tmp_id < -999999999) tmp_id = 0;
    return tmp_id;
}

list<string> E2Event::printBuildInfo()
{
    char buf[1000];
    list<string> l;

    if(build_info.size() > 0) {
	snprintf(buf, sizeof(buf), "B:ID:%s %ld", toShortString().c_str(), (long)triggers.size());
	l.push_back(string(buf));
	list<BuildInfo>::iterator it = build_info.begin();
	string s = (*it).startString();
	snprintf(buf, sizeof(buf), "B:N: %s %ld", s.c_str(), (long)build_info.size());
	l.push_back(string(buf));

	snprintf(buf, sizeof(buf), "B:H:  eventid loop ord net   sta ch loc    evlat     evlon  dep                 evtime\
 ttok     tt dok    dist  msta    maxd  mttres mttok trig ns nns nt lo  avefit  rmsfit");
	l.push_back(string(buf));

	for(list<BuildInfo>::iterator it = build_info.begin(); it != build_info.end(); it++) {
	    list<string> s = (*it).toString();
	    for(list<string>::iterator jt = s.begin(); jt != s.end(); jt++) {
		string a = string("B:") + (*jt).substr(2);
		l.push_back(a);
	    }
	}
    }
    return l;
}

BuildInfo::BuildInfo(E2Event &event, int bloop, int ndx, E2Trigger *t, E2Trigger *r) :
	eventid(event.eventid), loop(bloop), index(ndx), t_lat(0.), t_lon(0.), t_time(0.), r_lat(0.), r_lon(0.), r_time(0.),
	evtime(0.), evlat(0.), evlon(0.), evdepth(0.), tt(0.), dist(0.), tt_ok(false), dist_ok(false),
	sta_max_dist(0.), sta_max_tterr(0.), sta_max_tterr_ok(false), include_trigger(false),
	nsta(0), new_nsta(0), ntrig(0), located(false), misfit_ave(0.), misfit_rms(0.)
{
    eventid = event.eventid;
    index = ndx;
    loop = bloop;
    t_net = t->getNet();
    t_sta = t->getSta();
    t_chan2 = t->twoCharChan();
    t_loc = t->getLoc();
    t_lat = t->getLat();
    t_lon = t->getLon();
    t_time = t->getTime();
    r_net = r->getNet();
    r_sta = r->getSta();
    r_chan2 = r->twoCharChan();
    r_loc = r->getLoc();
    r_lat = r->getLat();
    r_lon = r->getLon();
    r_time = r->getTime();
}

string BuildInfo::startString()
{
    char buf[1000];
    string ts = TimeString::toString(t_time, 2); // round to nearest .01
    snprintf(buf, sizeof(buf), "%10d %3s %5s %3s %.4f %.4f %s", eventid, t_net.c_str(), t_sta.c_str(),
			t_loc.c_str(), t_lat, t_lon, ts.c_str());
    return string(buf);
}

list<string> BuildInfo::toString()
{
    list<string> s;
    char buf[1000];
    string rs = TimeString::toString(r_time, 2);
    string es = TimeString::toString(evtime, 2);

    snprintf(buf, sizeof(buf), "   %10d %4d %3d %3s %5s %2s %3s %8.4f %9.4f %4.1f %s    %d %6.2f   %d %7.2f %5s %7.2f %7.4f     %d    %d %2d  %2d %2d  %d %7.4f %7.4f",
	eventid, loop, index, r_net.c_str(), r_sta.c_str(), r_chan2.c_str(), r_loc.c_str(), evlat, evlon,
	evdepth, es.c_str(), tt_ok, tt, dist_ok, dist, sta_max.c_str(), sta_max_dist, sta_max_tterr,
	sta_max_tterr_ok, include_trigger, nsta, new_nsta, ntrig, located, misfit_ave, misfit_rms);
    s.push_back(string(buf));

    if(cn.counted) {
	list<string> l = cn.toString();
	for(list<string>::iterator it = l.begin(); it != l.end(); it++) s.push_back(*it);
    }
    return s;
}

AssocInfo::AssocInfo(E2Event *ev, E2Trigger *t, double evtimediff, double evkm) :
		eventid(0), version(0), lat(0.), lon(0.), depth(0.), trig_lat(0.), trig_lon(0.),
		trig_time(0.), tt(0.), dist(0.), tt_ok(false), dist_ok(false), near_tt_ok(false),
		near_dist_ok(false), ttmin(0.), ttmax(0.),
		relocate(false), new_lat(0.), new_lon(0.), new_depth(0.), new_time(0.),
		new_misfit_rms(0.), new_misfit_ave(0.), min_dist(0.), mag_dist_ok(false),
		misfit_ok(false), percent_ok(false), logged(false)
{
    eventid = ev->eventid;
    version = ev->version;
    lat = ev->lat;
    lon = ev->lon;
    depth = ev->depth;
    trig_sta = t->getSta();
    trig_chan = t->twoCharChan() + "Z";
    trig_net = t->getNet();
    trig_loc = t->getLoc();
    trig_lat = t->getLat();
    trig_lon = t->getLon();
    trig_time = t->getTime();
    tt = evtimediff;
    dist = evkm;
}

AssocInfo::AssocInfo() : eventid(0), version(0), lat(0.), lon(0.), depth(0.), trig_lat(0.), trig_lon(0.),
		trig_time(0.), tt(0.), dist(0.), tt_ok(false), dist_ok(false), near_tt_ok(false),
		near_dist_ok(false), ttmin(0.), ttmax(0.),
		relocate(false), new_lat(0.), new_lon(0.), new_depth(0.), new_time(0.),
		new_misfit_rms(0.), new_misfit_ave(0.), min_dist(0.), mag_dist_ok(false),
		misfit_ok(false), percent_ok(false), logged(false)
{
}

void AssocInfo::newLocation(E2Event *ev)
{
    new_lat = ev->lat;
    new_lon = ev->lon;
    new_depth = ev->depth;
    new_time = ev->time;
    new_misfit_rms = ev->misfit_rms;
    new_misfit_ave = ev->misfit_ave;
}

list<string> AssocInfo::toString(bool print_header)
{
    list<string> s;
    char buf[1000];
    string trig_t = TimeString::toString(trig_time, 3);
    string new_t = TimeString::toString(new_time, 2);

    if(print_header) {
	snprintf(buf, sizeof(buf),
"H:  eventid ver    evlat     evlon  dep   sta chan net loc      lat       lon       date         time     tt    dist ttok dok nttok ndok  ttmin  ttmax relo     nlat      nlon ndep                  ntime    nrms    nave fitok    dmin mdok prcnt pok");
	s.push_back(string(buf));
    }

    snprintf(buf, sizeof(buf),
" %10d %3d %8.4f %9.4f %4.1f %5s %4s %3s %3s %8.4f %9.4f %s %6.2f %7.2f    %d   %d     %d    %d %6.2f %6.2f    %d %8.4f %9.4f %4.1f %s %7.4f %7.4f     %d %7.2f    %d %5.1f   %d",

	eventid, version, lat, lon, depth, trig_sta.c_str(), trig_chan.c_str(), trig_net.c_str(), trig_loc.c_str(),
	trig_lat, trig_lon, trig_t.c_str(), tt, dist, tt_ok, dist_ok, near_tt_ok, near_dist_ok, ttmin, ttmax,
	relocate, new_lat, new_lon, new_depth, new_t.c_str(), new_misfit_rms, new_misfit_ave,
	misfit_ok, min_dist, mag_dist_ok, cn.percent, percent_ok);
    s.push_back(string(buf));

    if(cn.counted) {
	list<string> l = cn.toString();
	for(list<string>::iterator it = l.begin(); it != l.end(); it++) s.push_back(*it);
    }
    return s;
}

list<string> E2Event::toString()
{
    list<string> l;
    list<double> trigger_azimuth;
    vector<E2Trigger *> trigs;
    double delta, azimuth, back_azimuth, min_dist=1.e+60, max_dist=0.;
    char buf[1000];

    snprintf(buf, sizeof(buf),
"I:H:    eventid ver    evlat     evlon   dep   mag                   time   latu   lonu depu magu timeu   lk nTb nSb  nT  nS     ave     rms \
fitok splitok near statrig active inact nsta percnt prcntok mindist maxdist distok azspan Mok nSok Lok  Tdif tpave pdave TF Tok Azok Aok Ast             alert_time");
    l.push_back(string(buf));

    for(set<E2Trigger *, trigger_compare>::iterator it = triggers.begin(); it != triggers.end(); it++) {
	trigs.push_back(*it);
	deltaz(lat, lon, (*it)->getLat(), (*it)->getLon(), &delta, &azimuth, &back_azimuth);
	if(azimuth != azimuth) {
	    // azimuth is NAN
	    trigger_azimuth.push_back(0.);
	}
	else {
	    trigger_azimuth.push_back(azimuth);
	}
	double distkm = delta*111.19492664;
	if(min_dist > distkm) min_dist = distkm;
	if(max_dist < distkm) max_dist = distkm;
    }

    const char *first_alert_c = "  ";
    if(alert_sent) {
	first_alert_c = "F:";
	for( list<EventCore>::iterator it=history.begin(); it != history.end(); it++) {
	    if((*it).alert_sent) { first_alert_c = "  "; break; }
	}
    }
    char latu[7],lonu[7],magu[5],lku[5];
    snprintf(latu, sizeof(latu), "%6.4f", lat_uncer); // insures format is 6 characters when latu = -1
    snprintf(lonu, sizeof(lonu), "%6.4f", lon_uncer); // insures format is 6 characters when lonu = -1
    snprintf(magu, sizeof(magu), "%4.1f", mag_uncer); // insures format is 4 characters when magu = -1
    snprintf(lku, sizeof(lku), "%4.2f", likelihood);  // insures format is 4 characters when lku = -1

    snprintf(buf, sizeof(buf), "I:%s %10d %3d %8.4f %9.4f %5.1f %5.2f %s %s %s %4.1f %s %5.2f %s %3d %3d %3d %3d %7.4f %7.4f %5d %7d %4d %7d %6d %5d %4d %6.2f %7d %7.2f %7.2f %6d %6.2f %3d %4d %3d %5.2f %5.2f %5.2f %2d %3d %4d %3d %3d %s",
	first_alert_c, eventid, version, lat, lon, depth, evmag, TimeString::toString(time, 2).c_str(), latu, lonu, depth_uncer,
	magu, time_uncer, lku, ntrig_build, nsta_build, nT, nS, misfit_ave, misfit_rms, misfit_ok, split_ok, cn.num_near,
	cn.num_sta_trig, cn.num_active, cn.num_inactive, (int)cn.stas.size(), percent, percent_ok, min_dist, max_dist, dist_ok, az_span,
	alert_mag_ok, alert_nsta_ok, alert_loc_ok, teleseism_dif, tpaver, pdaver, tele_fbands, alert_teleseism_ok, alert_az_ok, alert_ok,
	alert_sent, TimeString::toString(alert_time, 2).c_str());

    l.push_back(string(buf));

    l.push_back(string("I:T:H: ") + E2Trigger::labels().substr(2) + string(" TF   tterr azerror  incid plen    sps toffset arrtime protime fndtime quetime sndtime  e2time buftime   alert zc ne_to_z  acc_range"));

    if(loc_info.size() > 0) {
	// get the tterr's of the last location
	E2ModuleManager::getLocator()->stationTTError(loc_info.back(), trigs);
    }

    list<double>::iterator jt = trigger_azimuth.begin();
    for(set<E2Trigger *, trigger_compare>::iterator it = triggers.begin(); it != triggers.end(); it++) {
	(*it)->setEventVersion(version);
	string s = (*it)->toString();
	double distkm = distancekm(lat, lon, (*it)->getLat(), (*it)->getLon());
	double alert_minus_outof_ring = alert_time - (*it)->outof_ring;
	double outof_ring_minus_trigger = (!E2TriggerManager::replay_mode) ?
					(*it)->outof_ring - (*it)->trigger_time : 0.;
	double min_range;
	double range = (*it)->rangePostTrig(&min_range);

	snprintf(buf, sizeof(buf),
		" %6.1f  %6.2f %2d %7.3f %7.2f %6.2f %4d %6.1f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %2d %7.2f %10.3e",
		distkm, (*jt), (*it)->tele_fbands, (*it)->tterr, (*it)->azimuth_error,
		(*it)->incidence, (*it)->getPlength(), (*it)->getSamprate(),
		(*it)->getToffset(), outof_ring_minus_trigger,
		(*it)->outof_feeder_queue - (*it)->outof_ring,
		(*it)->trigger_found - (*it)->outof_ring,
		(*it)->into_send_queue - (*it)->outof_ring,
		(*it)->outof_send_queue - (*it)->outof_ring,
		(*it)->getE2Latency(), (*it)->getBufLatency(), alert_minus_outof_ring,
		(*it)->numZeroCross(), (*it)->ne_to_z.ne_to_z(), range);

	jt++;
	l.push_back(string("I:T: ") + s + buf);
    }

    if(cn.counted) {
	cn.eventid = eventid;
	cn.version = version;
	if(eventid > 0 || percent > logPercentMin) {
	    list<string> cnlist = cn.toString();
	    for(list<string>::iterator it = cnlist.begin(); it != cnlist.end(); it++) l.push_back(*it);
	}
	cn.counted = false;
    }
    return l;
}

list<string> CountNeighbors::toString()
{
    list<string> l;

    if(stas.size() > 0) {
	char buf[1000];
	string s = TimeString::toString(evtime, 3);

	snprintf(buf, sizeof(buf), "S:H:   eventid ver    evlat     evlon                    time mindist maxdist percnt near_sta_cnt sta_trig_cnt active inactive nsta");
	l.push_back(string(buf));

	snprintf(buf, sizeof(buf), "S:  %10d %3d %8.4f %9.4f %s  %6.2f  %6.2f %6.2f          %3d          %3d    %3d      %3d  %3d",
		eventid, version, evlat, evlon, s.c_str(), mindist, maxdist, percent, num_near, num_sta_trig, num_active, 
		num_inactive, (int)stas.size());
	l.push_back(string(buf));

	snprintf(buf, sizeof(buf), 
	    "C:H:    eventid ver   sta  net      lat       lon    cluster   dist      tt                time  time_check active trig clu ctrig");
	l.push_back(string(buf));

	for(map<int, CountInfo>::iterator it = stas.begin(); it != stas.end(); it++) {
	    (*it).second.eventid = eventid;
	    (*it).second.version = version;
	    s = string("C: ") + (*it).second.toString();
	    l.push_back(s);
	}
    }

    return l;
}

string CountInfo::toString()
{
    char buf[1000];
    string cluster = E2TriggerManager::netsta(cluster_id);

    string s = TimeString::toString(last_sample_time, 0);

    snprintf(buf, sizeof(buf), "  %10d %3d %5s %4s %8.4f %9.4f %10s %6.2f %7.1f %s %11.0f      %1d    %1d   %1d     %1d",
	eventid, version, sta.c_str(), net.c_str(), lat, lon, cluster.c_str(), dist, tt, s.c_str(),
	time_check, active, triggered, in_cluster, cluster_triggered);
    return string(buf);
}

LocateTrigger::LocateTrigger(E2Trigger *t, int ver)
{
    net = t->getNet();
    sta = t->getSta();
    chan = t->twoCharChan() + string("Z");
    loc = t->getLoc();
    stachan_id = t->staChanId();
    version = ver;
    lat = t->getLat();
    lon = t->getLon();
    time = t->getTime();
    used = false;
    dist = 0.;
    tt = 0.;
    tterr = 0.;
}

void LocateInfo::init(E2Event &event)
{
    eventid = event.eventid;
    version = event.version;
    initial_lat = event.lat;
    initial_lon = event.lon;
    initial_depth = event.depth;
    initial_time = event.time;
    lat = 0.;
    lon = 0.;
    depth = 0.;
    time = 0.;
    misfit_ave = 0.;
    misfit_rms = 0.;
    trigs.clear();
    int o = 1;
    for(set<E2Trigger *>::iterator it = event.triggers.begin(); it != event.triggers.end(); it++) {
	trigs.push_back(LocateTrigger(*it, o++));
    }
}

list<string> LocateInfo::toString()
{
    char buf[1000];
    string t0, t1;
    list<string> l;
    set<string> nsta;

    snprintf(buf, sizeof(buf),
"E:H:    eventid ver s    lat0      lon0 dep0                  time0     lat       lon  dep                   time  ddist  avefit  rmsfit  nT  nS");
    l.push_back(string(buf));

    t0 = TimeString::toString(initial_time, 2);
    t1 = TimeString::toString(time, 2);
    double dist = distancekm(initial_lat, initial_lon, lat, lon);

    for(list<LocateTrigger>::iterator it = trigs.begin(); it != trigs.end(); it++) {
	nsta.insert((*it).sta + "." + (*it).net);
    }
    snprintf(buf, sizeof(buf), "E:   %10d %3d %d %.4f %.4f %4.1f %s %.4f %.4f %4.1f %s %6.1f %7.4f %7.4f %3d %3d",
		eventid, version, src, initial_lat, initial_lon, initial_depth, t0.c_str(), lat, lon, depth,
		t1.c_str(), dist, misfit_ave, misfit_rms, (int)trigs.size(), (int)nsta.size());
    l.push_back(string(buf));

    snprintf(buf, sizeof(buf), "T:H:    eventid ver   nT index   sta chan net loc      lat       lon U   dist     tt   tterr");
    l.push_back(string(buf));

    for(list<LocateTrigger>::iterator it = trigs.begin(); it != trigs.end(); it++) {
	snprintf(buf, sizeof(buf), "T:   %10d %3d %4d %5d %5s %4s %3s %3s %8.4f %9.4f %d %6.1f %6.2f %7.4f", eventid, version,
		(int)trigs.size(), (*it).version, (*it).sta.c_str(), (*it).chan.c_str(), (*it).net.c_str(),
		(*it).loc.c_str(), (*it).lat, (*it).lon, (*it).used, (*it).dist, (*it).tt, (*it).tterr);
	l.push_back(string(buf));
    }

    return l;
}

void E2Event::logMessages(void)
{
    stringstream msg_str;
    list<string> l;

    msg_str << E2Event::logTime() << endl;

    if(assoc_info.size() > 0) {
	list<AssocInfo>::reverse_iterator it=assoc_info.rbegin();
	cn = (*it).cn;
    }
    l = toString();
    for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << "E:" << (*it) << endl;

    if(version == 0) {
	l = printBuildInfo();
	for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << (*it) << endl;
    }
    for(list<LocateInfo>::iterator jt = loc_info.begin(); jt != loc_info.end(); jt++) {
	l = (*jt).toString();
	for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << "L:" << (*it) << endl;
    }

    bool assoc_header_printed = false;
    for(list<AssocInfo>::iterator it = assoc_info.begin(); it != assoc_info.end(); it++) {
	list<string> l = (*it).toString();
	for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << "A:" << (*it) << endl;
	assoc_header_printed = true;
    }
    for(map<long, AssocInfo>::iterator it = failed_assoc_info.begin(); it != failed_assoc_info.end(); it++) {
	if(!(*it).second.logged) {
	    (*it).second.logged = true;
	    list<string> l = (*it).second.toString(!assoc_header_printed);
	    for(list<string>::iterator it = l.begin(); it != l.end(); it++) msg_str << "a:" << (*it) << endl;
	    assoc_header_printed = true;
	}
    }

    msg_str << alert_msg;

    E2ModuleManager::sendLogMsg(msg_str.str());

    if(verbose >= 1) {
	LOG_INFO << msg_str.str();
    }
}

void E2Event::computeAzimuthError()
{
    double deg_to_radians = M_PI/180.;
    double delta, azimuth, back_azimuth;
    float ex, ey, tx, ty;

    for(set<E2Trigger *>::iterator it = triggers.begin(); it != triggers.end(); it++) {
        E2Trigger *t = (*it);
	if(t->az_computed) {
	    deltaz(t->getLat(), t->getLon(), lat, lon, &delta, &azimuth, &back_azimuth);
	    if(azimuth != azimuth) {
		// azimuth is NAN
	    }
	    else {
		ex = cos(azimuth*deg_to_radians);
		ey = sin(azimuth*deg_to_radians);
		tx = cos(t->azimuth*deg_to_radians);
		ty = sin(t->azimuth*deg_to_radians);
		double dotproduct = ex*tx + ey*ty;
		if(dotproduct < -1.0) dotproduct = -1.0;
		else if(dotproduct > 1.0) dotproduct = 1.0;
		double az = acos(dotproduct);
		if(az != az) { // az is NAN
		}
		else {
		    t->azimuth_error = az/deg_to_radians;
		}
//		char buf[200];
//		snprintf(buf, sizeof(buf), "azimuth: %s %d %7.2f to-origin: %7.2f  dif: %7.2f inc: %6.2f",
//		t->toShortString().c_str(), t->az_computed, t->azimuth, azimuth, azdif, t->incidence);
//		LOG_INFO << string(buf);
	    }
	}
    }
}
