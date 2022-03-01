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
#ifndef __EWPacket_
#define __EWPacket_
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include "wordorder.h"

#define  sta_len  5
#define chan_len  4
#define  net_len  3
#define  loc_len  3

#define GMPEAK	  	'G'
#define TRIGGER	  	'T'
#define TRIGPARAMS	'P'
#define TRIGPACKET	'K'
#define UNKNOWN    	'U'
#define BADVERSION 	'V'

#define CURRENT_VERSION 110

typedef char EWPacketType;
typedef struct { char src[20]; } PacketSource;

inline void addToPacket(int i, char * &p) {
    wordorder_master(i);
    serialize(i, p);
}
inline void addToPacket(float f, char * &p) {
    wordorder_master(f);
    serialize(f, p);
}
inline void addToPacket(double d, char * &p) {
    wordorder_master(d);
    serialize(d, p);
}
inline void addToPacket(timeval t, char * &p) {
    int i;
    i = t.tv_sec;
    wordorder_master(i);
    serialize(i, p);
    i = t.tv_usec;
    wordorder_master(i);
    serialize(i, p);
}
inline void addToPacket(char *c, int len, char * &p) {
    strncpy(p, c, len);
    p += len;
}
inline void addToPacket(char c, char * &p) {
    *p++ = c;
}
inline void addToPacket(PacketSource &ps, char * &p) {
    addToPacket(ps.src, sizeof(ps.src), p);
}

inline void getFromPacket(int &i, char * &p) {
    deserialize(i, p);
    wordorder_slave(i);
}
inline void getFromPacket(float &f, char * &p) {
    deserialize(f, p);
    wordorder_slave(f);
}
inline void getFromPacket(double &d, char * &p) {
    deserialize(d, p);
    wordorder_slave(d);
}
inline void getFromPacket(timeval &t, char * &p) {
    int i;
    deserialize(i, p);
    wordorder_slave(i);
    t.tv_sec = i;
    deserialize(i, p);
    wordorder_slave(i);
    t.tv_usec = i;
}
inline void getFromPacket(char *c, int len, char * &p) {
    strncpy(c, p, len);
    p += len;
}
inline void getFromPacket(char &c, char * &p) {
    c = *p++;
}
inline void getFromPacket(PacketSource &ps, char * &p) {
    getFromPacket(ps.src, sizeof(ps.src), p);
}
inline EWPacketType getPacketType(char *msg, int len) {
    if(len < (int)sizeof(EWPacketType) + (int)sizeof(int)) {
	std::cerr << "getPacketType: Short packet." << std::endl;
	return UNKNOWN;
    }
    EWPacketType type;
    getFromPacket(type, msg);
    int version;
    getFromPacket(version, msg);
    if(version != CURRENT_VERSION) return BADVERSION;
    return type;
}

char *readEWMessage(FILE *fpin, int *length);
int writeEWMessage(FILE *fp, char *msg, int length);

#endif
