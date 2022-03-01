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

#include "ZeroCrossings.h"
#include "EWPacket.h"


ZeroCrossings::ZeroCrossings() : Measurement("ZeroCrossings"),
		mean_vel(0), mean_acc(0), zero_cross_vel(0), zero_cross_acc(0)
{
}

void ZeroCrossings::init()
{
    mean_vel = 0;
    mean_acc = 0;
    zero_cross_vel = 0;
    zero_cross_acc = 0;
}

int ZeroCrossings::serialize(char * &p, int length) throw(std::string)
{
    char *pstart = p;
    startSerialize(p, length);

    addToPacket(status, p);
    addToPacket(mean_vel, p);
    addToPacket(mean_acc, p);
    addToPacket(zero_cross_vel, p);
    addToPacket(zero_cross_acc, p);
    return (int)(p - pstart);
}

int ZeroCrossings::deserialize(char * &p, int maxlength) throw(std::string)
{   
    char *pstart = p;
    startDeserialize(p, maxlength);

    getFromPacket(status, p);
    getFromPacket(mean_vel, p);
    getFromPacket(mean_acc, p);
    getFromPacket(zero_cross_vel, p);
    getFromPacket(zero_cross_acc, p);
    return (int)(p - pstart);
}

int ZeroCrossings::packetSize()
{
    return headerSize() +
	sizeof(status) +
	sizeof(mean_vel) +
	sizeof(mean_acc) +
	sizeof(zero_cross_vel) +
	sizeof(zero_cross_acc);
}

std::string ZeroCrossings::toString()
{
    char s[200];
    snprintf(s, sizeof(s), "%s %13.6e %13.6e %d %d",
        Measurement::toString().c_str(), mean_vel, mean_acc, zero_cross_vel, zero_cross_acc);
   return std::string(s);
}
