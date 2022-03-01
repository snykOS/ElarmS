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

#include "RangePostTrig.h"
#include "EWPacket.h"

RangePostTrig::RangePostTrig() : Measurement("RangePostTrig"),
			vel_min(0), vel_max(0), acc_min(0), acc_max(0)
{
}

void RangePostTrig::init()
{
    vel_min = 0;
    vel_max = 0;
    acc_min = 0;
    acc_max = 0;
}

int RangePostTrig::serialize(char * &p, int length) throw(std::string)
{
    char *pstart = p;
    startSerialize(p, length);

    addToPacket(status, p);
    addToPacket(vel_min, p);
    addToPacket(vel_max, p);
    addToPacket(acc_min, p);
    addToPacket(acc_max, p);
    return (int)(p - pstart);
}

int RangePostTrig::deserialize(char * &p, int maxlength) throw(std::string)
{   
    char *pstart = p;
    startDeserialize(p, maxlength);

    getFromPacket(status, p);
    getFromPacket(vel_min, p);
    getFromPacket(vel_max, p);
    getFromPacket(acc_min, p);
    getFromPacket(acc_max, p);
    return (int)(p - pstart);
}

int RangePostTrig::packetSize()
{
    return headerSize() +
	sizeof(status) +
	sizeof(vel_min) +
	sizeof(vel_max) +
	sizeof(acc_min) +
	sizeof(acc_max);
}

std::string RangePostTrig::toString()
{
    char s[200];

    snprintf(s, sizeof(s), "%s %13.6e %13.6e %13.6e %13.6e",
        Measurement::toString().c_str(),
	vel_min, vel_max, acc_min, acc_max);
   return std::string(s);
}
