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

#include <sstream>
#include <iomanip>
#include <math.h>
#include <plog/Log.h>
#include "MaxAmpMeasure.h"
#include "GetProp.h"
#include "PrintLock.h"

double MaxAmpMeasure::clip_level = 6000000.0*1000000; // 6.0e+12
double MaxAmpMeasure::clip_duration = 0.5;
double MaxAmpMeasure::taup_output_delay = 0.5;
double MaxAmpMeasure::beg_lag = 4.0; // amplitude measurements begin at trigger_time. Can be used after beg_lag
double MaxAmpMeasure::end_lag = 4.0; // amplitude measurements end at trigger_time + end_lag
bool MaxAmpMeasure::have_properties = false;

MaxAmpMeasure::MaxAmpMeasure(Channel chn) : Measure("MaxAmplitudes", chn)
{
    PrintLock::lock(); // lock to get properties and to print
    if(!have_properties) {
	have_properties = true;
	GetProp *prop = GetProp::getInstance();
	clip_level = prop->getDouble("ClipLevel");
	clip_duration = prop->getDouble("ClipDuration");
	taup_output_delay = prop->getDouble("TaupOutputDelay");
	beg_lag = prop->getDouble("MaxAmpBegLag");
	end_lag = prop->getDouble("MaxAmpEndLag");
	stringstream msg_str;
	msg_str << fixed << std::setprecision(4);
	msg_str << "P: MaxAmplitudes.ClipLevel: " << clip_level << endl;
	msg_str << "P: MaxAmplitudes.ClipDuration: " << clip_duration << endl;
	msg_str << "P: MaxAmplitudes.TaupOutputDelay: " << taup_output_delay << endl;
	msg_str << "P: MaxAmplitudes.MaxAmpBegLag: " << beg_lag << endl;
	msg_str << "P: MaxAmplitudes.MaxAmpEndLag: " << end_lag;
	LOG_INFO << msg_str.str();
    }
    PrintLock::unlock();

    zdata.setSize(500); // automatically increased for packet size
}

MaxAmpMeasure::~MaxAmpMeasure()
{
}

double MaxAmpMeasure::triggerLead()
{
    return 0.;
}

double MaxAmpMeasure::triggerLag()
{
    return end_lag;
}

void MaxAmpMeasure::start(MaxAmplitudes *m, double triggerTime, double sampleRate)
{
    m->init();
    m->status = MEASUREMENT_WAITING_ON_DATA;
    trigger_time = triggerTime;
    next_sample = trigger_time;
    samplerate = sampleRate;
    dt = 1.0/samplerate;
    half_dt = 0.5*dt;
    snr = -1.;
    noise = 0.0;
    time_unclip = 0;
}

int MaxAmpMeasure::update(MaxAmplitudes *m, ProcessBuffer &zbuffer, ProcessBuffer &nbuffer,
				ProcessBuffer &ebuffer)
{
    (void)nbuffer; (void)ebuffer;

    if(m->status != MEASUREMENT_WAITING_ON_DATA &&
	m->status != MEASUREMENT_UPDATING) return m->status;

    if(next_sample < zbuffer.startTime() - half_dt) {
        m->status = MEASUREMENT_GAP;
        return m->status;
    }

    int nsamps = zbuffer.getSamples(next_sample, zdata);
    double sample_time = next_sample;

    for(int i = 0; i < nsamps; i++, sample_time += dt)
    {
	if(sample_time > trigger_time + end_lag + half_dt) break;

	if(fabs(sample_time - trigger_time) < half_dt) {
            noise = zdata.n[i];
        }

        // compute snr at this point
        if(time_unclip > 0) {
            if(sample_time > time_unclip) {
                // out of clip window, check if clip_elev is still exceeded
                if(fabs(zdata.s[i]) > clip_level) {
                    time_unclip += clip_duration;
                    snr = -1.0;
                }
                else {
                    time_unclip = 0;
                    snr = (noise == 0.0) ? 0.0 : zdata.s[i]/noise;
                }
            }
            else {
                snr = -1.0;
            }
        }
        else {
            if(fabs(zdata.s[i]) > clip_level) {
                // data exceeds clip level. set clip window
                time_unclip = sample_time + clip_duration;
                snr = -1.0;
            }
            else {
                time_unclip = 0.0;
                snr = (noise == 0.0) ? 0.0 : zdata.s[i]/noise;
            }
        }

        if(fabs(m->pa) < fabs(zdata.a[i])) {
            m->pa = zdata.a[i];
            m->pa_snr = snr;
            m->pa_sample = (int)((sample_time - trigger_time)*samplerate + 0.5);
        }
        if(fabs(m->pv) < fabs(zdata.v[i])) {
            m->pv = zdata.v[i];
            m->pv_snr = snr;
            m->pv_sample = (int)((sample_time - trigger_time)*samplerate + 0.5);
        }

        if(fabs(m->pd) < fabs(zdata.d[i])) {
            m->pd = zdata.d[i];
            m->pd_snr = snr;
            m->pd_sample = (int)((sample_time - trigger_time)*samplerate + 0.5);
        }
        if(sample_time - trigger_time > taup_output_delay && m->tp < zdata.t[i]) {
            m->tp = zdata.t[i];
            m->tp_snr = snr;
            m->tp_sample = (int)((sample_time - trigger_time)*samplerate + 0.5);
        }
    }
    next_sample += nsamps*dt;
    if(next_sample > trigger_time + end_lag + half_dt) m->status = MEASUREMENT_COMPLETE;
    else if(next_sample > trigger_time + beg_lag + half_dt) m->status = MEASUREMENT_UPDATING;
    return m->status;
}
