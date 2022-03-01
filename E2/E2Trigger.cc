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
#include <errno.h>
#include <math.h>
#include <vector>
#include <sstream>
#include "E2Trigger.h"
#include "TriggerParams.h"
#include "TimeStamp.h"
#include "TimeString.h"
#include "E2TriggerManager.h"
#include "E2Alert.h"

using namespace std;

E2Trigger::E2Trigger(TriggerParams &tp, int sta_id, int stachan_id) :
	eventid(0), update(0), msg_version(0), _sta_id(sta_id),
	_stachan_id(stachan_id), version(0)
{
    station = tp.sta;
    network = tp.net;
    channel = tp.chan;
    location = tp.loc;
    stachan = getSta() + "." + twoCharChan();

    first_sample_time = 0.;
    taup_output_delay = 0.5; // wait 0.5 secs to compute taup
    clip_lev = 6000000.0*1000000;
    clip_dur = 0.5;
    taup_output_delay = 0.5; // wait 0.5 secs to compute taup

    az_computed = false;
    az_nsamps = 0;

    lat = tp.lat;
    lon = tp.lon;
    TimeStamp trigger_ts(UNIX_TIME, tp.trigger_time);
    trigger_time = trigger_ts.ts_as_double(UNIX_TIME);

    plength = tp.packet_length;
    z_recent_sample = tp.z_recent_sample;
    n_recent_sample = tp.n_recent_sample;
    e_recent_sample = tp.e_recent_sample;
    samprate = tp.samplerate;
    toffset = tp.toffset;
    outof_ring = tp.outof_ring;
    outof_feeder_queue = tp.outof_feeder_queue;
    trigger_found = tp.trigger_found;
    into_send_queue = tp.into_send_queue;
    outof_send_queue = tp.outof_send_queue;
    // time relative to the trigger-packet arrival time at the waveform processor
    e2_latency = TimeStamp::current_time().ts_as_double(UNIX_TIME) - outof_ring;
    buf_latency = 0.;

    nsamps = 0;

    order = 0;
    distkm = 0.;
    tterr = -999.;

    magchan_ok = false;
    magdistok = false;
    pdmag_ok = false;
    pdSNRok = false;
    tpmag_ok = false;
    tpSNRok = false;
    tele_fbands = 2;
    tele_window = 0;

    tpmag = 0.;
    pdmag = 0.;
    double rad = M_PI/180.;
    coslat = cos(lat*rad);
    sinlat = sin(lat*rad);
    coslon = cos(lon*rad);
    sinlon = sin(lon*rad);

    tpmag_used = false;
    pdmag_used = false;
    used = false;
    rms_large = false;
    teleseismic = 0;
    minus_tele = 0.;
    assoc_code = UNASSOCIATED;
    free_with_unassoc = false;
    fb_window_printed = -1;
    quality_code = WAITING;
    azimuth = 0.;
    incidence = 0.;
    azimuth_error = -1.;

    updateMeasurements(tp);
}

string E2Trigger::toString()
{
    char s[1000];
    snprintf(s, sizeof(s), " %6.3f   %d  %6.3f    %d %3d %3d %3d %3d %3d %3d %3d %4.1f", tpmag, tpmag_used,
	pdmag, pdmag_used, magchan_ok, magdistok, pdmag_ok, pdSNRok, tpmag_ok, tpSNRok, teleseismic, minus_tele);

    return toString(eventid, version, update, order) + " " + string(assocCodeStr()) + s;
}

string E2Trigger::timeString()
{
    char c[1000];

    string s = TimeString::toString(getTime(), 3);
    snprintf(c, sizeof(c), "%5s %4s %3s %3s %8.4f %9.4f %s", station.c_str(),
		channel.c_str(), network.c_str(), location.c_str(), lat, lon, s.c_str());
    return string(c);
}

string E2Trigger::toEmailString(char group)
{
    char c[100];
    string s = TimeString::toString(getTime(), 3);
    snprintf(c, sizeof(c), "%c %5s %4s %3s %s  %6.3f  %6.3f", group, station.c_str(),
		network.c_str(), channel.c_str(), s.c_str()+11, tpmag, pdmag);
    return string(c);
}


string E2Trigger::toString(int evid, int version, int update, int order)
{
    char c[1000];
    string s = TimeString::toString(trigger_time, 3);
    snprintf(c, sizeof(c), "%10d %3d %6d %5d %5s %4s %3s %3s %8.4f %9.4f %s %4d %4d %13.6e %13.6e %4d %13.6e %13.6e",
	evid, version, update, order, station.c_str(), channel.c_str(), network.c_str(), location.c_str(), lat, lon, s.c_str(),
	z_recent_sample, max_amplitudes.tp_sample, getLogTaup(), max_amplitudes.tp_snr, max_amplitudes.pd_sample, getLogPd(),
	max_amplitudes.pd_snr);
    return string(c);
}

double E2Trigger::stringToDouble(const string &s, const string name) throw(Error)
{
    char *endptr, last_char;
    const char *c;
    double d;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
        d = strtod(c, &endptr);
        last_char = *endptr;
        if(last_char == c[n]) {
            return d;
        }
    }
    cerr << "Invalid double(" << name << "): " << s << endl;
    throw Error("Invalid double:"+s);
}

int E2Trigger::stringToInt(const string &s, const string name) throw(Error)
{
    char *endptr, last_char;
    const char *c;
    long l;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
        l = strtol(c, &endptr, 10);
        last_char = *endptr;
        if(last_char == c[n]) {
            return (int)l;
        }
    }
    cerr << "Invalid int(" << name << "): " << s << endl;
    throw Error("Invalid int:"+s);
}

// 2011-09-23T18:24:54.028
double E2Trigger::stringToTime(const string &s) throw(Error)
{
    const char *c;
    int i, n, year, month, day, hour, min, sec, usec, usec_len;

    for(c = s.c_str(), i = 0; *c != '\0' && isspace((int)(*c)); c++, i++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;
    if(n == 23) {
	usec_len = 3;
    }
    else if(n == 24) {
	usec_len = 4;
    }
    else {
	throw Error("Invalid time format. Expecting yyyy-mm-dd hh:mm:ss.sss[s]");
    }

    try {
	year  = stringToInt(s.substr(i, 4), "year");   i += 5;
	month = stringToInt(s.substr(i, 2), "month");  i += 3;
	day   = stringToInt(s.substr(i, 2), "day");    i += 3;
	hour  = stringToInt(s.substr(i, 2), "hour");   i += 3;
	min   = stringToInt(s.substr(i, 2), "minute"); i += 3;
	sec   = stringToInt(s.substr(i, 2), "second"); i += 3;
	n  = stringToInt(s.substr(i, usec_len), "tmsec");
	usec = (usec_len == 3) ? n*1000 : n*100;
    }
    catch (Error e) {
	throw Error(" Expecting yyyy-mm-dd hh:mm:ss.sss[s]");
    }
    TimeStamp ts(year, month, day, hour, min, sec, usec);
    return ts.ts_as_double(UNIX_TIME);
}

int E2Trigger::numZeroCross()
{
    bool use_acc = false;
    if(!E2TriggerManager::zCrossVelAlways) {
	const char *c = channel.c_str();
	int n = strlen(c);
	if(n >= 2 && (c[n-2] == 'N' || c[n-2] == 'L')) use_acc = true;
    }
    return use_acc ? zero_crossings.zero_cross_acc : zero_crossings.zero_cross_vel;
}

double E2Trigger::rangePostTrig(double *min_range)
{
    bool use_velocity = false;
    if(!E2TriggerManager::rangeAccelAlways) {
	const char *c = channel.c_str();
	int n = strlen(c);
	if(n >= 2 && (c[n-2] != 'N' && c[n-2] != 'L')) use_velocity = true;
    }
    double range;
    if(use_velocity) {
	range = range_post_trig.vel_max - range_post_trig.vel_min;
	*min_range = E2TriggerManager::minRangePostTrigVel;
    }
    else {
	range = range_post_trig.acc_max - range_post_trig.acc_min;
	*min_range = E2TriggerManager::minRangePostTrigAccel;
    }
    return range;
}

QualityCode E2Trigger::getQualityCode()
{
    if(getEventid() == 0) {
	if(ne_to_z.getStatus() != MEASUREMENT_COMPLETE
	    || range_post_trig.getStatus() != MEASUREMENT_COMPLETE
	    || zero_crossings.getStatus() != MEASUREMENT_COMPLETE)
 	{
	    if(samprate > 0. && z_recent_sample/samprate < 1.5) {
		quality_code = WAITING;
		return quality_code;
	    }
	}
	double min_range;
	double range = rangePostTrig(&min_range);

        quality_code = OKAY;
        if(getLogPd() < E2TriggerManager::minPD) {
            quality_code = PD_SMALL;
        }
        else if(getLogPd() > E2TriggerManager::maxPD ) {
            quality_code = PD_LARGE;
        }
        else if(getLogPv() < E2TriggerManager::minPV) {
            quality_code = PV_SMALL;
        }
        else if(getLogPv() > E2TriggerManager::maxPV ) {
            quality_code = PV_LARGE;
        }
        else if(getLogPa() < E2TriggerManager::minPA) {
            quality_code = PA_SMALL;
        }
        else if(fabs(getTaup()) > 0. && (getLogTaup() < E2TriggerManager::minTP) ) {
            quality_code = TP_SMALL;
        }
        else if(fabs(getTaup()) > 0. && (getLogTaup() > E2TriggerManager::maxTP) ) {
            quality_code = TP_LARGE;
        }
	else if(ne_to_z.getStatus() == MEASUREMENT_COMPLETE
		&& ne_to_z.ne_to_z() > E2TriggerManager::maxNEtoZ)
	{
	    quality_code = NE_LARGE;
	}
	else if(zero_crossings.getStatus() == MEASUREMENT_COMPLETE
		&& numZeroCross() < E2TriggerManager::zCrossMin)
	{
            quality_code = Z_CROSS;
	}
	else if(range_post_trig.getStatus() == MEASUREMENT_COMPLETE
		&& range < min_range)
	{
	    quality_code = ACCRANGE;
	}
	else if(rms_large) {
	    quality_code = RMSLARGE;
	}
    }
    return quality_code;
}

const char *E2Trigger::assocCodeStr()
{
    static const char *quality_str[] = {
	"    OKAY", "NO_ZCOMP", "TP_SMALL", "TP_LARGE", "PD_SMALL", "PD_LARGE", "PV_SMALL", "PV_LARGE",
	"PA_SMALL", "ZEROCROS", "NE_LARGE", "ACCRANGE", " WAITING", "RMSLARGE", "" };
    static const char *assoc_str[] = {" UNASSOC", "NEAR_SRC", "TTWINDOW", "MULTISTA", "TWOSTATN", "ONESTATN"};
    int n;

    quality_code = getQualityCode();
    if(quality_code == OKAY) {
	n = (int)(sizeof(assoc_str)/sizeof(char *));
	return (assoc_code >= 0 && assoc_code < n) ? assoc_str[assoc_code] : "";
    }
    else {
	n = (int)(sizeof(quality_str)/sizeof(char *));
	return (quality_code >= 0 && quality_code < n) ? quality_str[quality_code] : "";
    }
}

string E2Trigger::toShortString()
{
    return toShortString("Z");
}

string E2Trigger::toShortString(const char *comp)
{
    char c[1000];
    string s = TimeString::toString(trigger_time, 3);
    string chan = twoCharChan() + string(comp);
    snprintf(c, sizeof(c), "%5s %4s %3s %3s %8.4f %9.4f %s", station.c_str(), chan.c_str(),
		network.c_str(), location.c_str(), lat, lon, s.c_str());
    return string(c);
}

bool E2Trigger::updateMeasurements(TriggerParams &tp)
{
    bool changed = false;
    if(tp.filter_bank != filter_bank
	|| tp.max_amplitudes != max_amplitudes
	|| tp.ne_to_z != ne_to_z
	|| tp.range_post_trig != range_post_trig
	|| tp.zero_crossings != zero_crossings) changed = true;

    filter_bank = tp.filter_bank;
    max_amplitudes = tp.max_amplitudes;
    ne_to_z = tp.ne_to_z;
    range_post_trig = tp.range_post_trig;
    zero_crossings = tp.zero_crossings;

    z_recent_sample = tp.z_recent_sample;
    n_recent_sample = tp.n_recent_sample;
    e_recent_sample = tp.e_recent_sample;

    return changed;
}
