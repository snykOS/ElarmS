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

#include "NEtoZ.h"
#include "EWPacket.h"

NEtoZ::NEtoZ() : Measurement("NEtoZ")
{
    init();
}

void NEtoZ::init()
{
    n_min = n_max = 0;
    e_min = e_max = 0;
    z_min = z_max = 0;
    start = true;
}

int NEtoZ::serialize(char * &p, int length) throw(std::string)
{
    char *pstart = p;
    startSerialize(p, length);

    addToPacket(status, p);
    addToPacket(n_min, p);
    addToPacket(n_max, p);
    addToPacket(e_min, p);
    addToPacket(e_max, p);
    addToPacket(z_min, p);
    addToPacket(z_max, p);
    return (int)(p - pstart);
}

int NEtoZ::deserialize(char * &p, int maxlength) throw(std::string)
{
    char *pstart = p;
    startDeserialize(p, maxlength);

    getFromPacket(status, p);
    getFromPacket(n_min, p);
    getFromPacket(n_max, p);
    getFromPacket(e_min, p);
    getFromPacket(e_max, p);
    getFromPacket(z_min, p);
    getFromPacket(z_max, p);
    return (int)(p - pstart);
}

int NEtoZ::packetSize()
{
    return headerSize() + sizeof(status) +
	sizeof(n_min) + sizeof(n_max) +
	sizeof(e_min) + sizeof(e_max) +
	sizeof(z_min) + sizeof(z_max);
}

std::string NEtoZ::toString()
{
    char s[200];
    snprintf(s, sizeof(s), "%s %13.6e %13.6e %13.6e",
        Measurement::toString().c_str(), n_max-n_min, e_max-e_min, z_max-z_min);
    return std::string(s);
}

double NEtoZ::ne_to_z()
{
    float n_range = n_max - n_min;
    float e_range = e_max - e_min;
    float z_range = z_max - z_min;
    float max_ne = (n_range > e_range) ? n_range : e_range;
    return (z_range != 0.) ? max_ne/z_range : 0.;
}
