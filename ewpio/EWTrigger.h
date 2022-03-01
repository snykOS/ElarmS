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
#ifndef __EWTrigger_h_
#define __EWTrigger_h_
#include "EWPacket.h"
#include "RawPacket.h"
#include "TimeStamp.h"

class EWTrigger
{
  public:
    int pv;
    PacketSource ps;
    int id;
    char sta[sta_len+1];
    char chan[chan_len+1];
    char net[net_len+1];
    char loc[loc_len+1];
    double lat;
    double lon;
    timeval time; // epoch time (Unix time)

    TimeStamp in_send_queue;

    EWTrigger(RawPacket &p, TimeStamp ts);
    EWTrigger(char *packet, int length) {
	pv = CURRENT_VERSION;
	deserialize(packet, length);
    }

    static int packetSize() {
	int timeval_size = 2*sizeof(int);
	return sizeof(EWPacketType) + sizeof(int) + sizeof(PacketSource)
		+ sizeof(int) + sta_len + chan_len + net_len + loc_len
		+ 2*sizeof(double) + timeval_size;
    }

    int serialize(PacketSource source, int packet_id, char *packet, int length);
    int deserialize(char *packet, int length);
    std::string toString(double tshift=0.0);
};

#endif
