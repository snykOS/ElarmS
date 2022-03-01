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
#include "NEtoZMeasure.h"
#include "GetProp.h"
#include "PrintLock.h"

float NEtoZMeasure::start_lag = 0.0;  // ne_to_z window starts at trigger_time + start_lag
float NEtoZMeasure::end_lag = 0.05;   // ne_to_z window ends at trigger_time + end_lag
bool NEtoZMeasure::have_properties = false;

NEtoZMeasure::NEtoZMeasure(Channel chn) : Measure("NEtoZ", chn)
{
    PrintLock::lock(); // lock to get properties and to print
    if(!have_properties) {
	have_properties = true;
	GetProp *prop = GetProp::getInstance();
	start_lag = prop->getDouble("StartNEtoZ");
	end_lag = prop->getDouble("EndNEtoZ");
	stringstream msg_str;
	msg_str << fixed << std::setprecision(4);
	msg_str << "P: NEtoZ.StartNEtoZ: " <<  start_lag << endl;
	msg_str << "P: NEtoZ.EndNEtoZ: " <<  end_lag;
	LOG_INFO << msg_str.str();
    }
    PrintLock::unlock();

    ndata.setSize(500); // automatically increased for packet size
    edata.setSize(500); // automatically increased for packet size
    zdata.setSize(500); // automatically increased for packet size
}

NEtoZMeasure::~NEtoZMeasure()
{
}

double NEtoZMeasure::triggerLead()
{
    return 0.;
}

double NEtoZMeasure::triggerLag()
{
    return end_lag;
}

void NEtoZMeasure::start(NEtoZ *m, double triggerTime, double sampleRate)
{
    m->status = MEASUREMENT_WAITING_ON_DATA;
    trigger_time = triggerTime;
    next_sample = trigger_time + start_lag;
    samplerate = sampleRate;
    dt = 1.0/samplerate;
    half_dt = 0.5*dt;
    m->init();
    m->start = true;
}

int NEtoZMeasure::update(NEtoZ *m, ProcessBuffer &zbuffer, ProcessBuffer &nbuffer, ProcessBuffer &ebuffer)
{
    if(m->status != MEASUREMENT_WAITING_ON_DATA) return m->status;
    double min = trigger_time + start_lag - half_dt;
    double max = trigger_time + end_lag + half_dt;

    if(next_sample < nbuffer.startTime() - half_dt ||
       next_sample < ebuffer.startTime() - half_dt ||
       next_sample < zbuffer.startTime() - half_dt)
    {
        m->status = MEASUREMENT_GAP;
        return m->status;
    }
    int Nsamps = nbuffer.getSamples(next_sample, ndata);
    int Esamps = ebuffer.getSamples(next_sample, edata);
    int Zsamps = zbuffer.getSamples(next_sample, zdata);

    // nsamps = minimum of Zsamps, Nsamps, Esamps
    int nsamps = (Zsamps < Nsamps) ? Zsamps : Nsamps;
    if(Esamps < nsamps) nsamps = Esamps;

    for(int i = 0; i < nsamps; i++)
    {
	double sample_time = next_sample + i*dt;
	if(sample_time > min && sample_time < max) {
	    if(m->start) {
		m->n_min = m->n_max = ndata.v[i];
		m->e_min = m->e_max = edata.v[i];
		m->z_min = m->z_max = zdata.v[i];
		m->start = false;
	    }
	    else {
		if(m->n_min > ndata.v[i]) m->n_min = ndata.v[i];
		if(m->n_max < ndata.v[i]) m->n_max = ndata.v[i];
		if(m->e_min > edata.v[i]) m->e_min = edata.v[i];
		if(m->e_max < edata.v[i]) m->e_max = edata.v[i];
		if(m->z_min > zdata.v[i]) m->z_min = zdata.v[i];
		if(m->z_max < zdata.v[i]) m->z_max = zdata.v[i];
	    }
        }
    }

    next_sample += nsamps*dt;
    if(next_sample > max) m->status = MEASUREMENT_COMPLETE;
    return m->status;
}
