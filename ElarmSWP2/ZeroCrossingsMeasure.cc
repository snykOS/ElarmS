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
#include "ZeroCrossingsMeasure.h"
#include "GetProp.h"
#include "PrintLock.h"

// Remove the mean starting at trigger time - zCrossDemeanLead before computing the zero crossings
int ZeroCrossingsMeasure::demean = 1;
float ZeroCrossingsMeasure::demean_lead = 4.;

// Count zero-crossings in window from zCrossLead before trigger until zCrossLag seconds after the trigger
float ZeroCrossingsMeasure::lead = 2.0;
float ZeroCrossingsMeasure::lag = 0.2;
bool ZeroCrossingsMeasure::have_properties = false;

ZeroCrossingsMeasure::ZeroCrossingsMeasure(Channel chn) : Measure("ZeroCrossings", chn),
		crossing_greater_vel(false), crossing_greater_acc(false)
{
    PrintLock::lock(); // lock to get properties and to print
    if(!have_properties) {
	have_properties = true;
	GetProp *prop = GetProp::getInstance();
	bool remove_mean;
	remove_mean = prop->getBool("ZCrossDemean");
	demean = (int)remove_mean;
	demean_lead = prop->getDouble("ZCrossDemeanLead");
	lead = prop->getDouble("ZCrossLead");
	lag = prop->getDouble("ZCrossLag");
	stringstream msg_str;
	msg_str << fixed << std::setprecision(4);
	msg_str << "P: ZeroCrossings.ZCrossDemean: " <<  demean << endl;
        msg_str << "P: ZeroCrossings.ZCrossDemeanLead: " <<  demean_lead << endl;
        msg_str << "P: ZeroCrossings.ZCrossLead: " <<  lead << endl;
        msg_str << "P: ZeroCrossings.ZCrossLag: " <<  lag;
	LOG_INFO << msg_str.str();
//	ElarmSWP2::param_str << msg_str.str();
    }
    PrintLock::unlock();

    mean_nsamps = 0;
    zdata.setSize(500); // automatically increased for packet size
}

ZeroCrossingsMeasure::~ZeroCrossingsMeasure()
{
}

double ZeroCrossingsMeasure::triggerLead()
{
    return (demean_lead > lead) ? demean_lead : lead;
}

double ZeroCrossingsMeasure::triggerLag()
{
    return lag;
}

void ZeroCrossingsMeasure::start(ZeroCrossings *m, double triggerTime, double sampleRate)
{
    m->status = MEASUREMENT_WAITING_ON_DATA;
    trigger_time = triggerTime;
    next_sample = trigger_time - demean_lead;
    samplerate = sampleRate;
    dt = 1.0/samplerate;
    half_dt = 0.5*dt;
    mean_nsamps = 0;
    crossing_greater_vel = false;
    crossing_greater_acc = false;
    m->mean_vel = 0.;
    m->mean_acc = 0.;
    m->zero_cross_vel = -1;
    m->zero_cross_acc = -1;
}

int ZeroCrossingsMeasure::update(ZeroCrossings *m, ProcessBuffer &zbuffer, ProcessBuffer &nbuffer,
				ProcessBuffer &ebuffer)
{
    (void)nbuffer; (void)ebuffer;

    if(m->status != MEASUREMENT_WAITING_ON_DATA) return m->status;

    if(trigger_time - zbuffer.startTime() < demean_lead - half_dt) {
	m->status = MEASUREMENT_INSUFFICIENT_LEAD;
	return m->status;
    }
    if(next_sample < zbuffer.startTime() - half_dt)
    {
        m->status = MEASUREMENT_GAP;
        return m->status;
    }

    int nsamps = zbuffer.getSamples(next_sample, zdata);

    if(demean)
    {
        // update the vel and acc means
        for(int i = 0; i < nsamps; i++)
        {
            double sample_time = next_sample + i*dt;
            if(sample_time > trigger_time - demean_lead - half_dt) {
                mean_nsamps++;
                m->mean_vel = ((mean_nsamps-1)*m->mean_vel + zdata.v[i])/mean_nsamps;
                m->mean_acc = ((mean_nsamps-1)*m->mean_acc + zdata.a[i])/mean_nsamps;
            }
        }
    }

    // count zero-crossings
    for(int i = 0; i < nsamps; i++)
    {
        double sample_time = next_sample + i*dt;

        if(sample_time > trigger_time - lead - half_dt &&
		sample_time < trigger_time + lag + half_dt)
        {
            if(m->zero_cross_vel < 0) {
                m->zero_cross_vel = 0;
                crossing_greater_vel = zdata.v[i] > m->mean_vel;
            }
            else {
                if((zdata.v[i] > m->mean_vel) != crossing_greater_vel) {
                    crossing_greater_vel = (zdata.v[i] > m->mean_vel);
                    m->zero_cross_vel++;
                }
            }

            if(m->zero_cross_acc < 0) {
                m->zero_cross_acc = 0;
                crossing_greater_acc = zdata.v[i] > m->mean_acc;
            }
            else {
                if((zdata.v[i] > m->mean_acc) != crossing_greater_acc) {
                    crossing_greater_acc = (zdata.v[i] > m->mean_acc);
                    m->zero_cross_acc++;
                }
            }
        }
    }
    next_sample += nsamps*dt;
    if(next_sample > trigger_time + lag + half_dt) m->status = MEASUREMENT_COMPLETE;
    return m->status;
}
