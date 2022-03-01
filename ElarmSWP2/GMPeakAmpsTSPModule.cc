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
#include <math.h>
#include "GMPeakAmpsTSPModule.h"

void GMPeakAmpsTSPModule::process(int nsamps, ArrayStruct &data, double sample_time, PacketInfo &packet_info)
{
    peakInit(nsamps, sample_time, packet_info);
    
    for(int i=0; i < nsamps; i++)
    {
	if(fabs(data.d[i]) > fabs(peak.dmax)) {
	    peak.dmax = data.d[i];
	    peak.dindex = i;
        }
        if(fabs(data.v[i]) > fabs(peak.vmax)) {
	    peak.vmax = data.v[i];
            peak.vindex = i;
        }
        if(fabs(data.a[i]) > fabs(peak.amax)) {
            peak.amax = data.a[i];
            peak.aindex = i;
        }
    }
    sender->send(&peak);
}

void GMPeakAmpsTSPModule::peakInit(int nsamps, double sample_time, PacketInfo &packet_info)
{
    (void)BINARY; (void)MEMORY; (void)ASCII; (void)FILENAME; (void)DATABASE; // avoid compiler unused warning
    peak.init();
    strncpy(peak.sta, channel.station, sta_len);
    strncpy(peak.chan, channel.channel, chan_len);
    strncpy(peak.loc, channel.location, loc_len);
    strncpy(peak.net, channel.network, net_len);
    peak.lat = channel.latitude;
    peak.lon = channel.longitude;
    peak.tbeg = sample_time;
    peak.nsamps = nsamps;
    peak.samprate = (float)packet_info.samplerate;
    double last_sample_time = sample_time + (nsamps-1)/packet_info.samplerate;
    peak.latency = packet_info.outof_ring - last_sample_time;
    peak.queue_time = packet_info.outof_feeder_queue - packet_info.outof_ring;
}
