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
#include <plog/Log.h>
#include "RangePostTrigMeasure.h"
#include "GetProp.h"
#include "PrintLock.h"

float RangePostTrigMeasure::start_lag = 0.1;
float RangePostTrigMeasure::end_lag = 0.2;
bool RangePostTrigMeasure::have_properties = false;

RangePostTrigMeasure::RangePostTrigMeasure(Channel chn) : Measure("RangePostTrig", chn)
{
    PrintLock::lock(); // lock to get properties and to print
    if(!have_properties) {
	have_properties = true;
	GetProp *prop = GetProp::getInstance();
	start_lag = prop->getDouble("StartRangePostTrig");
	end_lag = prop->getDouble("EndRangePostTrig");
	stringstream msg_str;
	msg_str << fixed << std::setprecision(4);
	msg_str << "P: RangePostTrig.StartRangePostTrig: " <<  start_lag << endl;
	msg_str << "P: RangePostTrig.EndRangePostTrig: " <<  end_lag;
	LOG_INFO << msg_str.str();
//	ElarmSWP2::param_str << msg_str.str();
    }
    PrintLock::unlock();

    zdata.setSize(500); // automatically increased for packet size
    ndata.setSize(500); // automatically increased for packet size
    edata.setSize(500); // automatically increased for packet size
}

RangePostTrigMeasure::~RangePostTrigMeasure()
{
}

double RangePostTrigMeasure::triggerLead()
{
    return 0.;
}
double RangePostTrigMeasure::triggerLag()
{
    return end_lag;
}

void RangePostTrigMeasure::start(RangePostTrig *m, double triggerTime, double sampleRate)
{
    m->status = MEASUREMENT_WAITING_ON_DATA;
    trigger_time = triggerTime;
    next_sample = trigger_time + start_lag;
    samplerate = sampleRate;
    dt = 1.0/samplerate;
    half_dt = 0.5*dt;
    m->vel_min = 0.;
    m->vel_max = 0.;
    m->acc_min = 0.;
    m->acc_max = 0.;
}

int RangePostTrigMeasure::update(RangePostTrig *m, ProcessBuffer &zbuffer, ProcessBuffer &nbuffer,
				ProcessBuffer &ebuffer)
{
    if(m->status != MEASUREMENT_WAITING_ON_DATA) return m->status;
    double min = trigger_time + start_lag - half_dt;
    double max = trigger_time + end_lag + half_dt;
    double amp;

    if(next_sample < zbuffer.startTime() - half_dt ||
       next_sample < nbuffer.startTime() - half_dt ||
       next_sample < ebuffer.startTime() - half_dt)
    {
        m->status = MEASUREMENT_GAP;
        return m->status;
    }

    int Zsamps = zbuffer.getSamples(next_sample, zdata);
    int Nsamps = nbuffer.getSamples(next_sample, ndata);
    int Esamps = ebuffer.getSamples(next_sample, edata);

    // nsamps = minimum of Zsamps, Nsamps, Esamps
    int nsamps = (Zsamps < Nsamps) ? Zsamps : Nsamps;
    if(Esamps < nsamps) nsamps = Esamps;

    for(int i = 0; i < nsamps; i++)
    {
	double sample_time = next_sample + i*dt;
	if(sample_time > min && sample_time < max) {
	    amp = sqrt(zdata.v[i]*zdata.v[i] + ndata.v[i]*ndata.v[i] + edata.v[i]*edata.v[i]);
	    if(m->vel_max == 0. || amp < m->vel_min) m->vel_min = amp;
	    if(m->vel_max < amp) m->vel_max = amp;

            amp = sqrt(zdata.a[i]*zdata.a[i] + ndata.a[i]*ndata.a[i] + edata.a[i]*edata.a[i]);
	    if(m->acc_max == 0. || amp < m->acc_min) m->acc_min = amp;
	    if(m->acc_max < amp) m->acc_max = amp;
        }
    }
    next_sample += nsamps*dt;
    if(next_sample > max) m->status = MEASUREMENT_COMPLETE;
    return m->status;
}
