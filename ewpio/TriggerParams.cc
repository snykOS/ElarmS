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
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include "TriggerParams.h"
#include "MaxAmplitudes.h"
#include "FilterBank.h"
#include "NEtoZ.h"
#include "RangePostTrig.h"
#include "ZeroCrossings.h"
#include "PrintLock.h"
#include "TimeString.h"

using namespace std;


TriggerParams::TriggerParams()
{
    init();
}

TriggerParams::TriggerParams(char *packet, int length)
{   
    init();
    deserialize(packet, length);
}

void TriggerParams::init()
{
    memset(sta, 0, sizeof(sta));
    memset(chan, 0, sizeof(chan));
    memset(net, 0, sizeof(net));
    memset(loc, 0, sizeof(loc));
    lat = 0;
    lon = 0;
    memset(&trigger_time, 0, sizeof(timeval));
    packet_length = 0;
    z_recent_sample = 0;
    n_recent_sample = 0;
    e_recent_sample = 0;
    samplerate = 0;
    toffset = 0;
    outof_ring = 0;
    outof_feeder_queue = 0;
    trigger_found = 0;
    into_send_queue = 0;
    outof_send_queue = 0;
}

char * TriggerParams::serialize(char *packet, int length) throw(string)
{
    if(length < headerSize()) {
	stringstream s;
	s << "TriggerParams.startSerialize: buffer length (" << length << ") < headerSize ("
		<< headerSize() << ")";
	PrintLock::lock();
	cerr << s.str() << endl;
	PrintLock::unlock();
	throw s.str();
    }
    char *p = packet;
    EWPacketType type = TRIGPARAMS;
    addToPacket(type, p);
    int pv = CURRENT_VERSION;
    addToPacket(pv, p);
    addToPacket(sta, sta_len, p);
    addToPacket(chan, chan_len, p);
    addToPacket(net, net_len, p);
    addToPacket(loc, loc_len, p);
    addToPacket(lat, p);
    addToPacket(lon, p);
    addToPacket(trigger_time, p);
    addToPacket(packet_length, p);
    addToPacket(z_recent_sample, p);
    addToPacket(n_recent_sample, p);
    addToPacket(e_recent_sample, p);
    addToPacket(samplerate, p);
    addToPacket(toffset, p);
    addToPacket(outof_ring, p);
    addToPacket(outof_feeder_queue, p);
    addToPacket(trigger_found, p);
    addToPacket(into_send_queue, p);
    addToPacket(outof_send_queue, p);
    length -= (p - packet);

    length -= filter_bank.serialize(p, length);
    length -= max_amplitudes.serialize(p, length);
    length -= ne_to_z.serialize(p, length);
    length -= range_post_trig.serialize(p, length);
    length -= zero_crossings.serialize(p, length);
    return p;
}

char * TriggerParams::deserialize(char *packet, int maxlength) throw(string)
{
    EWPacketType type;
    int version;

    if(maxlength < headerSize()) {
	PrintLock::lock();
	cerr << "TriggerParams.deserialize: maxlength < headerSize" << endl;
	PrintLock::unlock();
	throw string("TriggerParams.deserialize: maxlength < headerSize");
    }
    char *p = packet;
    getFromPacket(type, p);
    if(type != TRIGPARAMS) {
	PrintLock::lock();
	cerr << "TriggerParams.deserialize: packet type != TRIGPARAMS" << endl;
	PrintLock::unlock();
	throw string("TriggerParams.deserialize: packet type != TRIGPARAMS");
    }
    getFromPacket(version, p);
    if(version != CURRENT_VERSION) {
	stringstream s;
	s << "TriggerParams.deserialize version mismatch: " << version;
	PrintLock::lock();
	cerr << s.str() << endl;
	PrintLock::unlock();
	throw s.str();
    }

    getFromPacket(sta, sta_len, p);
    getFromPacket(chan, chan_len, p);
    getFromPacket(net, net_len, p);
    getFromPacket(loc, loc_len, p);
    getFromPacket(lat, p);
    getFromPacket(lon, p);
    getFromPacket(trigger_time, p);
    getFromPacket(packet_length, p);
    getFromPacket(z_recent_sample, p);
    getFromPacket(n_recent_sample, p);
    getFromPacket(e_recent_sample, p);
    getFromPacket(samplerate, p);
    getFromPacket(toffset, p);
    getFromPacket(outof_ring, p);
    getFromPacket(outof_feeder_queue, p);
    getFromPacket(trigger_found, p);
    getFromPacket(into_send_queue, p);
    getFromPacket(outof_send_queue, p);
    maxlength -= (p - packet);

    maxlength -= filter_bank.deserialize(p, maxlength);
    maxlength -= max_amplitudes.deserialize(p, maxlength);
    maxlength -= ne_to_z.deserialize(p, maxlength);
    maxlength -= range_post_trig.deserialize(p, maxlength);
    maxlength -= zero_crossings.deserialize(p, maxlength);

    return p;
}

int TriggerParams::packetSize()
{
    return headerSize()
	+ filter_bank.packetSize()
	+ max_amplitudes.packetSize()
	+ ne_to_z.packetSize()
	+ range_post_trig.packetSize()
	+ zero_crossings.packetSize();
}

string TriggerParams::toString(bool long_format)
{
    char c[200];
    string s = TimeString::toString(trigger_time, 3);

    char zlag[10], nlag[10], elag[10];
    double trig_lag;
    trig_lag = (samplerate > 0.) ? z_recent_sample/samplerate : 0.;
    if(trig_lag >= 0.) snprintf(zlag, sizeof(zlag), "+%05.2f", trig_lag);
    else snprintf(zlag, sizeof(zlag), "-%05.2f", fabs(trig_lag));

    trig_lag = (samplerate > 0.) ? n_recent_sample/samplerate : 0.;
    if(trig_lag >= 0.) snprintf(nlag, sizeof(nlag), "+%05.2f", trig_lag);
    else snprintf(nlag, sizeof(nlag), "-%05.2f", fabs(trig_lag));

    trig_lag = (samplerate > 0.) ? e_recent_sample/samplerate : 0.;
    if(trig_lag >= 0.) snprintf(elag, sizeof(elag), "+%05.2f", trig_lag);
    else snprintf(elag, sizeof(elag), "-%05.2f", fabs(trig_lag));

    snprintf(c, sizeof(c), "T: %5s %4s %3s %3s %8.4f %9.4f %s %s %s %s ",
		sta, chan, net, loc, lat, lon, s.c_str(), zlag, nlag, elag);
    if(long_format) {
	ostringstream os;
	os << filter_bank.toString(c) << endl;
	os << c << max_amplitudes.toString() << endl;
	os << c << ne_to_z.toString() << endl;
	os << c << range_post_trig.toString() << endl;
	os << c << zero_crossings.toString();
	return os.str();
    }
    else {
	return string(c);
    }
}

string TriggerParamsBundle::toString(bool long_format)
{
    char s[50];
    ostringstream os;
    snprintf(s, sizeof(s), "%10d TriggerParamsBundle.size: %d", id, (int)trig_params.size());
    os << s;
    for(int i = 0; i < (int)trig_params.size(); i++) {
	os << endl;
	os << trig_params[i].toString(long_format);
    }
    return os.str();
}

string TriggerParamsBundle::toString(string sta, string chan, bool long_format, bool first_only)
{
    ostringstream os;
    int n = 0;
    for(int i = 0; i < (int)trig_params.size(); i++) {
	if( (sta.empty() || !strcasecmp(trig_params[i].sta, sta.c_str())) &&
		(chan.empty() || !strcasecmp(trig_params[i].chan, chan.c_str())))
	{
	    if(!first_only) {
		if(n > 0) os << endl;
		os << trig_params[i].toString(long_format);
		n++;
	    }
	}
    }
    return os.str();
}

int TriggerParamsBundle::serialize(PacketSource source, int packet_id,
				char *serialized_msg, int maxlength)
{
    if(maxlength < headerSize()) {
	PrintLock::lock();
	cerr << "TriggerParamsBundle.serialize: maxlength < headerSize" << endl;
	PrintLock::unlock();
	return -1;
    }
    char *p = serialized_msg;

    EWPacketType type = TRIGPACKET;
    addToPacket(type, p);
    int version = CURRENT_VERSION;
    addToPacket(version, p);
    ps = source;
    addToPacket(ps, p);
    id = packet_id;
    addToPacket(id, p);
    int num_params = trig_params.size();
    addToPacket(num_params, p);

    maxlength -= (p - serialized_msg);

    int msg_length = 0;
    for(int i = 0; i < (int)trig_params.size(); i++) {
	msg_length += sizeof(int);
	msg_length += trig_params[i].packetSize();
    }
    if(maxlength < msg_length) { 
	PrintLock::lock();
	cerr << "TriggerParamsPacket.deserialize:  maxlength < serialized_length" << endl;
	PrintLock::unlock();
	return -1;
    }

    for(int i = 0; i < (int)trig_params.size(); i++) {
	int len = trig_params[i].packetSize();
	addToPacket(len, p);
	maxlength -= sizeof(int);
	char *pm = p;
	p = trig_params[i].serialize(pm, maxlength);
	maxlength -= (p - pm);
    }
    return p - serialized_msg;
}

int TriggerParamsBundle::deserialize(char *msg, int length)
{
    EWPacketType type;
    int version, num_params;
    char *p;

    if(length < headerSize()) {
	PrintLock::lock();
	cerr << "TriggerParamsBundle.deserialize: length < headerSize" << endl;
	PrintLock::unlock();
	return -1;
    }
    p = msg;
    getFromPacket(type, p);
    if(type != TRIGPACKET) {
	PrintLock::lock();
	cerr << "TriggerParamsBundle.deserialize: packet type != TRIGPACKET" << endl;
	PrintLock::unlock();
	return -1;
    }
    getFromPacket(version, p);
    if(version != CURRENT_VERSION) {
	type = BADVERSION;
	return -1;
    }
    getFromPacket(ps, p);
    getFromPacket(id, p);
    getFromPacket(num_params, p);

    length -= headerSize();

    for(int i = 0; i < num_params; i++) {
	if(length < (int)sizeof(int)) {
	    PrintLock::lock();
	    cerr << "TriggerParamsBundle.deserialize-1: length " << length << " message size" << endl;
	    PrintLock::unlock();
	    return -1;
	}
	int msg_len;
	getFromPacket(msg_len, p);
	length -= sizeof(int);
	if(length < msg_len) {
	    PrintLock::lock();
	    cerr << "TriggerParamsBundle.deserialize-2: length " << length <<" message size" << msg_len << endl;
	    PrintLock::unlock();
	    return -1;
	}
	TriggerParams t;
	p = t.deserialize(p, length);
	trig_params.push_back(t);
	length -= msg_len;
    }
    return (int)trig_params.size();
}

void TriggerParamsBundle::clear()
{
    trig_params.clear();
}
