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
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include "GMPeak.h"
#include "TimeString.h"

using namespace std;

void GMPeak::init()
{
    memset(sta, 0, sizeof(sta));
    memset(chan, 0, sizeof(chan));
    memset(net, 0, sizeof(net));
    memset(loc, 0, sizeof(loc));
    memset(&lat, 0, sizeof(lat));
    memset(&lon, 0, sizeof(lon));
    memset(&tbeg, 0, sizeof(tbeg));
    memset(&nsamps, 0, sizeof(nsamps));
    memset(&samprate, 0, sizeof(samprate));
    memset(&dmax, 0, sizeof(dmax));
    memset(&vmax, 0, sizeof(vmax));
    memset(&amax, 0, sizeof(amax));
    memset(&dindex, 0, sizeof(dindex));
    memset(&vindex, 0, sizeof(vindex));
    memset(&aindex, 0, sizeof(aindex));
    memset(&latency, 0, sizeof(latency));
    memset(&queue_time, 0, sizeof(queue_time));
}

int GMPeak::serialize(char *packet, int length)
{
    if(length < packetSize()) {
	cerr << "GMPeak.serialize: buffer length < packet size." << endl;
	return -1;
    }
    char *p = packet;
    addToPacket(sta, sta_len, p);
    addToPacket(chan, chan_len, p);
    addToPacket(net, net_len, p);
    addToPacket(loc, loc_len, p);
    addToPacket(lat, p);
    addToPacket(lon, p);
    addToPacket(tbeg, p);
    addToPacket(nsamps, p);
    addToPacket(samprate, p);
    addToPacket(dmax, p);
    addToPacket(vmax, p);
    addToPacket(amax, p);
    addToPacket(dindex, p);
    addToPacket(vindex, p);
    addToPacket(aindex, p);
    addToPacket(latency, p);
    return (int)(p - packet);
}
int GMPeak::deserialize(char *packet, int length)
{
    if(length < packetSize()) {
	cerr << "GMPeak.deserialize: short packet length." << endl;
	return -1;
    }
    char *p = packet;
    getFromPacket(sta, sta_len, p);
    getFromPacket(chan, chan_len, p);
    getFromPacket(net, net_len, p);
    getFromPacket(loc, loc_len, p);
    getFromPacket(lat, p);
    getFromPacket(lon, p);
    getFromPacket(tbeg, p);
    getFromPacket(nsamps, p);
    getFromPacket(samprate, p);
    getFromPacket(dmax, p);
    getFromPacket(vmax, p);
    getFromPacket(amax, p);
    getFromPacket(dindex, p);
    getFromPacket(vindex, p);
    getFromPacket(aindex, p);
    getFromPacket(latency, p);
    return (int)(p - packet);
}

string GMPeak::toString()
{
    char c[100];
    string s = TimeString::toString(tbeg, 3);

    snprintf(c, sizeof(c), "%5s %4s %3s %3s %8.4f %9.4f %s %5d %7.2f %8.2f %13.6e %13.6e %13.6e %6d %6d %6d",
		sta, chan, net, loc, lat, lon, s.c_str(), nsamps, samprate, latency,
		dmax, vmax, amax, dindex, vindex, aindex);
    return string(c);
}

int GMPeakPacket::serialize(PacketSource source, int packet_id, char *packet, int length)
{
    int packet_size = sizeof(EWPacketType) + sizeof(int) + sizeof(PacketSource)
			+ 2*sizeof(int) + 1 + num_peaks*GMPeak::packetSize();
    if(length < packet_size) {
	cerr << "GMPeakPacket.serialize: buffer length < packet size." << endl;
	return -1;
    }
    char *p = packet;
    EWPacketType type = GMPEAK;
    addToPacket(type, p);
    addToPacket(pv, p);
    ps = source;
    addToPacket(source, p);
    id = packet_id;
    addToPacket(packet_id, p);
    addToPacket(num_peaks, p);
    char compressed = 0;
    addToPacket(&compressed, 1, p);
    int header_size = (int)(p - packet);
    length -= header_size;

    for(int i = 0; i < num_peaks; i++) {
	int nbytes = peaks[i].serialize(p, length);
	p += nbytes;
	length -= nbytes;
    }
    int n = (int)(p - packet);
    int src_len = n - header_size;
    Bytef *src = (Bytef *)(packet + header_size);

    int gzlevel = 6;
    unsigned long gzlen = (int)(1.1*src_len + 12);
    Bytef *gzbuf = (Bytef *)malloc(gzlen);

    if(compress2(gzbuf, &gzlen, src, src_len, gzlevel) == Z_OK && (int)gzlen < src_len) {
	compressed = 1;
	memcpy(packet+header_size-1, &compressed, 1);
	memcpy(src, gzbuf, gzlen);
	n = header_size + gzlen;
    }
    free(gzbuf);
    return n;
}

int GMPeakPacket::deserialize(char *packet, int length)
{
    int header_size = sizeof(EWPacketType) + sizeof(int) + sizeof(PacketSource) + 2*sizeof(int) + 1;
    if(length < header_size) {
	cerr << "GMPeakPacket.deserialize: short packet length." << endl;
	return -1;
    }
    EWPacketType type;
    int version;
    char compressed, *dest=NULL;
    char *p = packet;

    getFromPacket(type, p);
    if(type != GMPEAK) {
	cerr << "GMPeakPacket.deserialize: packet type != GMPEAK" << endl;
	return -1;
    }
    getFromPacket(version, p);
    if(version != pv) {
	type = BADVERSION;
	return -1;
    }
    getFromPacket(ps, p);
    getFromPacket(id, p);
    getFromPacket(num_peaks, p);
    if(num_peaks < 0) {
	cerr << "GMPeakPacket.deserialize: number of peaks < 0" << endl;
	return -1;
    }
    if(num_peaks > MAX_GMPEAK_PER_PKT) {
	cerr << "GMPeakPacket.deserialize: number of peaks > MAX_GMPEAK_PER_PKT("
		<< MAX_GMPEAK_PER_PKT << ")" << endl;
	return -1;
    }
    getFromPacket(&compressed, 1, p);
    int uncompressed_size = num_peaks*GMPeak::packetSize();
    length -= header_size;

    if(compressed) {
	const Bytef *src = (const Bytef *)(packet + header_size);
	dest = (char *)malloc(uncompressed_size);
	unsigned long n = uncompressed_size;
	if(uncompress((Bytef *)dest, &n, src, length) != Z_OK || (int)n != uncompressed_size) {
	    cerr << "GMPeakPacket.deserialize: uncompress failed." << endl;
	    free(dest);
	    return -1;
	}
	p = dest;
	length = n;
    }
    else if(length < uncompressed_size) {
	cerr << "GMPeakPacket.deserialize: short packet length." << endl;
	return -1;
    }

    for(int i = 0; i < num_peaks; i++) {
	peaks[i] = GMPeak(p, length);
	p += GMPeak::packetSize();
	length -= GMPeak::packetSize();
    }
    if(dest) free(dest);
    return 1;
}

string GMPeakPacket::toString()
{
    char s[200];
    ostringstream os;
    snprintf(s, sizeof(s), "%10d GMPeakPacket.size: %d\n", id, num_peaks);
    os << s;
    snprintf(s, sizeof(s),
"      sta chan net loc     lat       lon       date          time  nsamps  samprate        dmax          \
vmax          amax dindex vindex aindex\n");
    os << s;
    for(int i = 0; i < num_peaks; i++) {
	os << endl;
	snprintf(s, sizeof(s), "%3d", i);
	os << s << " " << peaks[i].toString();
    }
    return os.str();
}

