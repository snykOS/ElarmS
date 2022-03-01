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

#include "MaxAmplitudes.h"
#include "EWPacket.h"

MaxAmplitudes::MaxAmplitudes() : Measurement("MaxAmplitudes"),
		tp_sample(0), pd_sample(0), pv_sample(0), pa_sample(0),
		tp(0), pd(0), pv(0), pa(0), tp_snr(0), pd_snr(0), pv_snr(0), pa_snr(0)
{
}

void MaxAmplitudes::init()
{
    tp_sample = 0;
    pd_sample = 0;
    pv_sample = 0;
    pa_sample = 0;
    tp = 0;
    pd = 0;
    pv = 0;
    pa = 0;
    tp_snr = 0;
    pd_snr = 0;
    pv_snr = 0;
    pa_snr = 0;
}

int MaxAmplitudes::serialize(char * &p, int length) throw(std::string)
{
    char *pstart = p;
    startSerialize(p, length);

    addToPacket(status, p);
    addToPacket(tp_sample, p);
    addToPacket(pd_sample, p);
    addToPacket(pv_sample, p);
    addToPacket(pa_sample, p);
    addToPacket(tp, p);
    addToPacket(pd, p);
    addToPacket(pv, p);
    addToPacket(pa, p);
    addToPacket(tp_snr, p);
    addToPacket(pd_snr, p);
    addToPacket(pv_snr, p);
    addToPacket(pa_snr, p);
    return (int)(p - pstart);
}

int MaxAmplitudes::deserialize(char * &p, int length) throw(std::string)
{
    char *pstart = p;
    startDeserialize(p, length);

    getFromPacket(status, p);
    getFromPacket(tp_sample, p);
    getFromPacket(pd_sample, p);
    getFromPacket(pv_sample, p);
    getFromPacket(pa_sample, p);
    getFromPacket(tp, p);
    getFromPacket(pd, p);
    getFromPacket(pv, p);
    getFromPacket(pa, p);
    getFromPacket(tp_snr, p);
    getFromPacket(pd_snr, p);
    getFromPacket(pv_snr, p);
    getFromPacket(pa_snr, p);
    return (int)(p - pstart);
}

int MaxAmplitudes::packetSize()
{
    return headerSize() +
	sizeof(status) +
	sizeof(tp_sample) +
	sizeof(pd_sample) +
	sizeof(pv_sample) +
	sizeof(pa_sample) +
	sizeof(tp) +
	sizeof(pd) +
	sizeof(pv) +
	sizeof(pa) +
	sizeof(tp_snr) +
	sizeof(pd_snr) +
	sizeof(pv_snr) +
	sizeof(pa_snr);
}

std::string MaxAmplitudes::toString()
{
    char s[200];

    snprintf(s, sizeof(s), "%s %3d %3d %3d %3d %13.6e %13.6e %13.6e %13.6e %13.6e %13.6e %13.6e %13.6e",
	Measurement::toString().c_str(), tp_sample, pd_sample, pv_sample, pa_sample, tp,
	pd, pv, pa, tp_snr, pd_snr, pv_snr, pa_snr);
    return std::string(s);
}
