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
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <plog/Log.h>
#include "FilterBankMeasure.h"
#include "ProcessBuffer.h"
#include "GetProp.h"
#include "EWPacket.h"
#include "PrintLock.h"
#include "TimeString.h"

bool FilterBankMeasure::z_fbands_only = true;
bool FilterBankMeasure::compute_fbands = true;
double FilterBankMeasure::filter_bank_window_lead = 30.0;
double FilterBankMeasure::samprate_fbands_max[2] = {32., 96.};
vector<TimeWindow> FilterBankMeasure::windows;
vector<FrequencyBand> FilterBankMeasure::fbands1;
vector<FrequencyBand> FilterBankMeasure::fbands2;
vector<FrequencyBand> FilterBankMeasure::fbands3;
bool FilterBankMeasure::verbose = 2;
bool FilterBankMeasure::have_properties = false;

FilterBankMeasure::FilterBankMeasure(Channel chn) : Measure("FilterBank", chn)
{
    PrintLock::lock(); // lock to get properties and to print
    if(!have_properties) {
	have_properties = true;
        GetProp *prop = GetProp::getInstance();
	double window_lags[NUM_FWINDOWS] = {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,2.0};
	double fb1[2*NUM_FBANDS] = {
	    6.,8.,6.,8.,6.,8.,3.,6.,1.5,3.,0.75,1.5,0.375,0.75,0.1875,0.375,0.09375,0.1875
	};
	double fb2[2*NUM_FBANDS] = {
	    12.,16.,8.,12.,6.,8.,3.,6.,1.5,3.,0.75,1.5,0.375,0.75,0.1875,0.375,0.09375,0.1875
	};
	double fb3[2*NUM_FBANDS] = {
	    24.,48.,12.,24.,6.,12.,3.,6.,1.5,3.,0.75,1.5,0.375,0.75,0.1875,0.375,0.09375,0.1875
	};

	z_fbands_only = prop->getBool("ZFBandsOnly");
	compute_fbands = prop->getBool("ComputeFBands");
	if(compute_fbands) {
	    filter_bank_window_lead = prop->getDouble("WindowLead");
	    prop->getDoubleArray("WindowLags", NUM_FWINDOWS, window_lags);
	    prop->getDoubleArray("SamprateFBandsMax", 2, samprate_fbands_max);
	    prop->getDoubleArray("FBands1", 2*NUM_FBANDS, fb1);
	    prop->getDoubleArray("FBands2", 2*NUM_FBANDS, fb2);
	    prop->getDoubleArray("FBands3", 2*NUM_FBANDS, fb3);
	}
	verbose = prop->getInt("Verbose");
	stringstream msg_str;
	msg_str << fixed << std::setprecision(4);

	msg_str << "P: TriggerParams.ZFBandsOnly: " << z_fbands_only << endl;
	msg_str << "P: TriggerParams.ComputeFBands: " << compute_fbands << endl;
	if(compute_fbands) {
	    char buf[20];
	    msg_str << "P: TriggerParams.WindowLead: " << filter_bank_window_lead << endl;
	    msg_str << "P: TriggerParams:WindowLags: ";
            windows.clear();
	    for(int i = 0; i < NUM_FWINDOWS; i++) {
		snprintf(buf, sizeof(buf), "%.2f", window_lags[i]);
		msg_str << buf;
		if(i < NUM_FWINDOWS-1) msg_str << ",";
		windows.push_back(TimeWindow(filter_bank_window_lead, window_lags[i]));
	    }
	    msg_str << endl;
	    snprintf(buf, sizeof(buf), "%.2f,%.2f", samprate_fbands_max[0], samprate_fbands_max[1]);
	    msg_str << "P: TriggerParams:SamprateFBandsMax: " << buf << endl;

	    msg_str << "P: TriggerParams:FBands1: ";
            fbands1.clear();
	    for(int i = 0; i < NUM_FBANDS; i++) {
		snprintf(buf, sizeof(buf), "%.5f,%.5f", fb1[2*i], fb1[2*i+1]);
		msg_str << buf;
		if(i < NUM_FBANDS-1) msg_str << ",";
		if(fb1[2*i] >= fb1[2*i+1]) {
		    msg_str << "Invalid TriggerParams:FBands1"  << endl;
		    exit(1);
		}
		fbands1.push_back(FrequencyBand(fb1[2*i], fb1[2*i+1]));
	    }
	    msg_str << endl;
	    msg_str << "P: TriggerParams:FBands2: ";
            fbands2.clear();
	    for(int i = 0; i < NUM_FBANDS; i++) {
		snprintf(buf, sizeof(buf), "%.5f,%.5f", fb2[2*i], fb2[2*i+1]);
		msg_str << buf;
		if(i < NUM_FBANDS-1) msg_str << ",";
		if(fb2[2*i] >= fb2[2*i+1]) {
		    msg_str << "Invalid TriggerParams:FBands2"  << endl;
		    exit(1);
		}
		fbands2.push_back(FrequencyBand(fb2[2*i], fb2[2*i+1]));
	    }
	    msg_str << endl;
	    msg_str << "P: TriggerParams:FBands3: ";
            fbands3.clear();
	    for(int i = 0; i < NUM_FBANDS; i++) {
		snprintf(buf, sizeof(buf), "%.5f,%.5f", fb3[2*i], fb3[2*i+1]);
		msg_str << buf;
		if(i < NUM_FBANDS-1) msg_str << ",";
		if(fb3[2*i] >= fb3[2*i+1]) {
		    msg_str << "Invalid TriggerParams:FBands3"  << endl;
		    exit(1);
		}
		fbands3.push_back(FrequencyBand(fb3[2*i], fb3[2*i+1]));
	    }
	    msg_str << endl;
	}
	msg_str << "P: TriggerParams.Verbose: " << verbose;
	LOG_INFO << msg_str.str();
//	ElarmSWP2::param_str << msg_str.str();
    }
    PrintLock::unlock();

    zdata.setSize(500); // automatically increased for packet size
}

FilterBankMeasure::~FilterBankMeasure()
{
}

double FilterBankMeasure::triggerLead()
{
    return filter_bank_window_lead;
}

double FilterBankMeasure::triggerLag()
{
    double lag = 0;
    for(int i = 0; i < (int)windows.size(); i++) {
	if(windows[i].lag > lag) lag = windows[i].lag;
    }
    return lag;
}

void FilterBankMeasure::start(FilterBank *m, double triggerTime, double sampleRate)
{
    m->init();
    m->status = MEASUREMENT_WAITING_ON_DATA;
    trigger_time = triggerTime;
    next_sample = trigger_time - filter_bank_window_lead;
    samplerate = sampleRate;
    dt = 1.0/samplerate;
    half_dt = 0.5*dt;

    if(fabs(samplerate - filters.getSamplerate()) > 0.1)
    {
	fbands.clear();
	// no more than NUM_FBANDS
	if(samplerate < samprate_fbands_max[0]) {
	    for(int i = 0; i < NUM_FBANDS; i++) fbands.push_back(fbands1[i]);
	}
	else if(samplerate < samprate_fbands_max[1]) {
	    for(int i = 0; i < NUM_FBANDS; i++) fbands.push_back(fbands2[i]);
	}
	else {
	    for(int i = 0; i < NUM_FBANDS; i++) fbands.push_back(fbands3[i]);
	}
	filters.setBands(samplerate, fbands);
    }
}

int FilterBankMeasure::update(FilterBank *m, ProcessBuffer &zbuffer, ProcessBuffer &nbuffer,
				ProcessBuffer &ebuffer)
{
    (void)nbuffer; (void)ebuffer;

    if(m->status == MEASUREMENT_COMPLETE) return m->status;

    if(trigger_time - zbuffer.startTime() < filter_bank_window_lead - half_dt) {
	m->status = MEASUREMENT_INSUFFICIENT_LEAD;
	return m->status;
    }

    for(int i = 0; i < (int)windows.size(); i++) if(!m->fw[i].isMeasured())
    {
	applyFilters(windows[i], zbuffer, m->fw[i]);
    }

    for(int i = 0; i < (int)windows.size(); i++) {
	if(!m->fw[i].isMeasured()) {
	    if(i > 0) m->status = MEASUREMENT_UPDATING;
	    return m->status;
	}
    }

    m->status = MEASUREMENT_COMPLETE;
    return m->status;
}

bool FilterBankMeasure::applyFilters(TimeWindow window, ProcessBuffer &zbuffer, FilterWindow &fw)
{
    int npts = (int)((window.lead + window.lag)*samplerate+.5) + 1;
    double tbeg = trigger_time - window.lead;

    fw.window_npts = 0; // If there are not lead+lag seconds in the buffer, return npts=0.
    fw.window_lead = window.lead; // set lead and lag to indicate that lag seconds have passed
    fw.window_lag = window.lag;

    int ndata = zbuffer.getSamples(tbeg, zdata);
    if(ndata < npts)
    {
	if(verbose >= 2) {
	    char buf[100];
	    snprintf(buf, sizeof(buf), "X: %5s %4s %3s %3s %s buffer duration: %.2lf", chn.station,
		chn.channel, chn.network, chn.location, triggerString().c_str(), zbuffer.duration());
	    LOG_INFO << buf;
        }
        return false;
    }
    fw.window_npts = npts;

    filters.getMaxAmp(npts, zdata, fw.pgv);

    if(verbose >= 2) {
	char buf[100];
	snprintf(buf, sizeof(buf), "F: %5s %4s %3s %3s %s %s", chn.station, chn.channel,
		chn.network, chn.location, triggerString().c_str(), fw.toString().c_str());
	LOG_INFO << buf;
    }
    return true;
}

string FilterBankMeasure::triggerString()
{
    return TimeString::toString(trigger_time, 3);
}
