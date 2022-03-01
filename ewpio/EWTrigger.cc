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
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "EWTrigger.h"
#include "TimeString.h"

using namespace std;

EWTrigger::EWTrigger(RawPacket &p, TimeStamp ts)
{
    id = 0;
    pv = CURRENT_VERSION;
    memset(&ps, 0, sizeof(PacketSource));
    memset(sta, 0, sizeof(sta));
    memset(chan, 0, sizeof(chan));
    memset(net, 0, sizeof(net));
    memset(loc, 0, sizeof(loc));
    strncpy(sta, p.ch.station, sta_len);
    strncpy(chan, p.ch.channel, chan_len);
    strncpy(loc, p.ch.location, loc_len);
    strncpy(net, p.ch.network, net_len);
    lat = p.ch.latitude;
    lon = p.ch.longitude;
    time.tv_sec  = ts.seconds(UNIX_TIME);
    time.tv_usec = ts.u_seconds();
}

int EWTrigger::serialize(PacketSource source, int packet_id, char *packet, int length)
{
    if(length < packetSize()) {
	cerr << "EWTrigger.serialize: buffer length < packet size." << endl;
	return -1;
    }
    char *p = packet;
    EWPacketType type = TRIGGER;
    addToPacket(type, p);
    addToPacket(pv, p);
    ps = source;
    addToPacket(source, p);
    id = packet_id;
    addToPacket(packet_id, p);
    addToPacket(sta, sta_len, p);
    addToPacket(chan, chan_len, p);
    addToPacket(net, net_len, p);
    addToPacket(loc, loc_len, p);
    addToPacket(lat, p);
    addToPacket(lon, p);
    addToPacket(time, p);
    return (int)(p - packet);
}

int EWTrigger::deserialize(char *packet, int length)
{
    if(length < packetSize()) {
	cerr << "EWTrigger.deserialize: short packet length." << endl;
	return -1;
    }
    char *p = packet;
    EWPacketType type;
    int version;
    getFromPacket(type, p);
    if(type != TRIGGER) {
	cerr << "EWTrigger.deserialize: packet type != TRIGGER" << endl;
	return -1;
    }
    getFromPacket(version, p);
    if(version != pv) {
	type = BADVERSION;
	return -1;
    }
    memset(&ps, 0, sizeof(PacketSource));
    memset(sta, 0, sizeof(sta));
    memset(chan, 0, sizeof(chan));
    memset(net, 0, sizeof(net));
    memset(loc, 0, sizeof(loc));
    getFromPacket(ps, p);
    getFromPacket(id, p);
    getFromPacket(sta, sta_len, p);
    getFromPacket(chan, chan_len, p);
    getFromPacket(net, net_len, p);
    getFromPacket(loc, loc_len, p);
    getFromPacket(lat, p);
    getFromPacket(lon, p);
    getFromPacket(time, p);
    return (int)(p - packet);
}

string EWTrigger::toString(double tshift)
{
    string s;
    char c[100];

    if(tshift != 0.) {
	double nepoch = (double)time.tv_sec + 1.0e-6 * time.tv_usec;
	nepoch -= tshift;
	s = TimeString::toString(nepoch, 3);
    }
    else {
	s = TimeString::toString(time, 3);
    }
    snprintf(c, sizeof(c), "%10d Trigger: %5s %4s %3s %3s %8.4f %9.4f %s",
                id, sta, chan, net, loc, lat, lon, s.c_str());
    return string(c);
}
