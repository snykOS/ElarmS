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
#include "TriggerParamsTSPModule.h"
#include "TimeStamp.h"

#include "MaxAmplitudes.h"
#include "FilterBank.h"
#include "NEtoZ.h"
#include "RangePostTrig.h"
#include "ZeroCrossings.h"

TriggerParamsTSPModule::TriggerParamsTSPModule(Channel chn, EWP2Sender *send) : channel(chn), sender(send)
{
    fb_measure = new FilterBankMeasure(chn);
    ma_measure = new MaxAmpMeasure(chn);
    ne_measure = new NEtoZMeasure(chn);
    rp_measure = new RangePostTrigMeasure(chn);
    zc_measure = new ZeroCrossingsMeasure(chn);

    size_Zdata = 500; // automatically increased for packet size
    Zdata = new SampleStruct * [size_Zdata];
    size_Ndata = 500; // automatically increased for packet size
    Ndata = new SampleStruct * [size_Ndata];
    size_Edata = 500; // automatically increased for packet size
    Edata = new SampleStruct * [size_Edata];
}

TriggerParamsTSPModule::~TriggerParamsTSPModule()
{
    delete fb_measure;
    delete ma_measure;
    delete ne_measure;
    delete rp_measure;
    delete zc_measure;

    delete[] Zdata;
    delete[] Ndata;
    delete[] Edata;
}

double TriggerParamsTSPModule::bufferLength()
{
    double lead_max = 0;
    double lag_max = 0;

    if(lead_max < fb_measure->triggerLead()) lead_max = fb_measure->triggerLead();
    if(lag_max < fb_measure->triggerLag()) lag_max = fb_measure->triggerLag();

    if(lead_max < ma_measure->triggerLead()) lead_max = ma_measure->triggerLead();
    if(lag_max < ma_measure->triggerLag()) lag_max = ma_measure->triggerLag();

    if(lead_max < ne_measure->triggerLead()) lead_max = ne_measure->triggerLead();
    if(lag_max < ne_measure->triggerLag()) lag_max = ne_measure->triggerLag();

    if(lead_max < rp_measure->triggerLead()) lead_max = rp_measure->triggerLead();
    if(lag_max < rp_measure->triggerLag()) lag_max = rp_measure->triggerLag();

    if(lead_max < zc_measure->triggerLead()) lead_max = zc_measure->triggerLead();
    if(lag_max < zc_measure->triggerLag()) lag_max = zc_measure->triggerLag();

    return(lead_max + lag_max);
}

void TriggerParamsTSPModule::startTrigger(TimeStamp trigger_time, PacketInfo packet_info,
			ProcessBuffer &Zbuffer, ProcessBuffer &Nbuffer, ProcessBuffer &Ebuffer)
{
    double trigger_epoch = trigger_time.ts_as_double(UNIX_TIME);
    memset(tp.sta, 0, sizeof(tp.sta));
    memset(tp.chan, 0, sizeof(tp.chan));
    memset(tp.net, 0, sizeof(tp.net));
    memset(tp.loc, 0, sizeof(tp.loc));
    strncpy(tp.sta, channel.station, sta_len);
    strncpy(tp.chan, channel.channel, chan_len);
    strncpy(tp.loc, channel.location, loc_len);
    strncpy(tp.net, channel.network, net_len);
    tp.lat = channel.latitude;
    tp.lon = channel.longitude;
    tp.trigger_time.tv_sec = (time_t)trigger_time.seconds(UNIX_TIME);
    tp.trigger_time.tv_usec = trigger_time.u_seconds();
    tp.packet_length = packet_info.length;
    // number of samples from the trigger sample to the most recent sample received
    double duration;
    duration = Zbuffer.endTime() - trigger_epoch;
    tp.z_recent_sample = (int)(duration*tp.samplerate + 0.5);
    duration = Nbuffer.endTime() - trigger_epoch;
    tp.n_recent_sample = (int)(duration*tp.samplerate + 0.5);
    duration = Ebuffer.endTime() - trigger_epoch;
    tp.e_recent_sample = (int)(duration*tp.samplerate + 0.5);

    tp.samplerate = packet_info.samplerate;
    tp.toffset = trigger_epoch - packet_info.start_time;
    tp.outof_ring = packet_info.outof_ring;
    tp.outof_feeder_queue = packet_info.outof_feeder_queue;
    tp.trigger_found = sender->currentTime().ts_as_double(UNIX_TIME);
    tp.into_send_queue = 0.;
    tp.outof_send_queue = 0.;

    fb_measure->start(&tp.filter_bank, trigger_epoch, tp.samplerate);
    ma_measure->start(&tp.max_amplitudes, trigger_epoch, tp.samplerate);
    ne_measure->start(&tp.ne_to_z, trigger_epoch, tp.samplerate);
    rp_measure->start(&tp.range_post_trig, trigger_epoch, tp.samplerate);
    zc_measure->start(&tp.zero_crossings, trigger_epoch, tp.samplerate);
}

bool TriggerParamsTSPModule::updateTrigger(string chan, ProcessBuffer &Zbuffer,
				ProcessBuffer &Nbuffer, ProcessBuffer &Ebuffer)
{
    strcpy(tp.chan, chan.c_str());
    double trigger_epoch = tp.trigger_time.tv_sec + 1.e-06*tp.trigger_time.tv_usec;
    double duration;

    duration = Zbuffer.endTime() - trigger_epoch;
    tp.z_recent_sample = (int)(duration*tp.samplerate + 0.5);
    duration = Nbuffer.endTime() - trigger_epoch;
    tp.n_recent_sample = (int)(duration*tp.samplerate + 0.5);
    duration = Ebuffer.endTime() - trigger_epoch;
    tp.e_recent_sample = (int)(duration*tp.samplerate + 0.5);

    int fb = fb_measure->update(&tp.filter_bank, Zbuffer, Nbuffer, Ebuffer);
    int ma = ma_measure->update(&tp.max_amplitudes, Zbuffer, Nbuffer, Ebuffer);
    int ne = ne_measure->update(&tp.ne_to_z, Zbuffer, Nbuffer, Ebuffer);
    int rp = rp_measure->update(&tp.range_post_trig, Zbuffer, Nbuffer, Ebuffer);
    int zc = zc_measure->update(&tp.zero_crossings, Zbuffer, Nbuffer, Ebuffer);

    bool more_updates;
    // if ma is finished, then do not wait for the horizontal components
    if(ma != MEASUREMENT_WAITING_ON_DATA && ma != MEASUREMENT_UPDATING) {
	more_updates = false;
    }
    else {
	more_updates = 
	    (fb == MEASUREMENT_WAITING_ON_DATA || fb == MEASUREMENT_UPDATING ||
             ma == MEASUREMENT_WAITING_ON_DATA || ma == MEASUREMENT_UPDATING ||
             ne == MEASUREMENT_WAITING_ON_DATA || ne == MEASUREMENT_UPDATING ||
             rp == MEASUREMENT_WAITING_ON_DATA || rp == MEASUREMENT_UPDATING ||
             zc == MEASUREMENT_WAITING_ON_DATA || zc == MEASUREMENT_UPDATING);
    }

    sender->send(&tp);
    return more_updates;
}
