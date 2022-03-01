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
#include <unistd.h>
#include <iomanip>
#include <sstream>
#include "PickerTSPModule.h"
#include "ElarmSWP2.h"
#include "PrintLock.h"
#include "Duration.h"
#include "GetProp.h"

bool    PickerTSPModule::have_properties = false;
double  PickerTSPModule::para_t_sta = 0.;
double  PickerTSPModule::para_t_lta = 0.;
double  PickerTSPModule::para_tau_k = 0.;
double  PickerTSPModule::para_trig_lev = 0.;
int     PickerTSPModule::para_trig_dur = 0;
bool    PickerTSPModule::para_use_trig_stab = false;
double  PickerTSPModule::para_trig_stab_lev = 0.;
bool    PickerTSPModule::para_use_trig_gap_delay = false;
int     PickerTSPModule::para_trig_gap_dur = 0;


PickerTSPModule::PickerTSPModule(Channel chn) : channel(chn)
{
    PrintLock::lock();
    if(!have_properties) {
	have_properties = true;
        GetProp *prop = GetProp::getInstance();
	para_t_sta = prop->getDouble("TSCTsta");
	if(para_t_sta == 0.0) {
	    LOG_INFO << "TSCTsta(" << para_t_sta
		<< ") is too small. Using value 1.e-10.";
	    para_t_sta = 1.e-10;
	}
	para_t_lta = prop->getDouble("TSCTlta");
	if(para_t_lta == 0.0) {
	    LOG_INFO << "TSCTlta(" << para_t_lta
		<< ") is too small. Using value 1.e-10.";
	    para_t_lta = 1.e-10;
	}
	para_tau_k = prop->getDouble("RICTauK");
	para_trig_lev = prop->getDouble("TrigLev");
	para_trig_dur = prop->getInt("TrigDur");
	para_use_trig_stab = prop->getBool("UseTrigStab");
	para_trig_stab_lev = prop->getDouble("TrigStabLev");
	para_use_trig_gap_delay = prop->getBool("UseTrigGapDelay");
	para_trig_gap_dur = prop->getDouble("TrigGapDur");

	stringstream msg_str;
	msg_str << fixed << setprecision(2);
	msg_str << "P: Picker.TSCTsta: " << para_t_sta << endl;
	msg_str << "P: Picker.TSCTlta: " << para_t_lta << endl;
	msg_str << "P: Picker.RICTauK: " << para_tau_k << endl;
	msg_str << "P: Picker.TrigLev: " << para_trig_lev << endl;
	msg_str << "P: Picker.TrigDur: " << para_trig_dur << endl;
	msg_str << "P: Picker.UseTrigStab: " << para_use_trig_stab << endl;
	msg_str << "P: Picker.TrigStabLev: " << para_trig_stab_lev << endl;
	msg_str << "P: Picker.UseTrigGapDelay: " << para_use_trig_gap_delay << endl;
	msg_str << "P: Picker.TrigGapDur: " << para_trig_gap_dur;
	LOG_INFO << msg_str.str();
	ElarmSWP2::param_str << msg_str.str();
    }
    PrintLock::unlock();

    prev_v = 0.0;
    prev_sta = 0.0;
    prev_lta = 0.0;

    array_size = 100; // adjust below
    sta = new double[array_size];
    lta = new double[array_size];

    stabilized = para_use_trig_stab ? false : true;
    time_untrig = 0.0;
    time_gap_delay = 0.0;
    in_gap = true;

    delta_t = 1.0/chn.samprate;
}

PickerTSPModule::~PickerTSPModule()
{
    delete [] sta;
    delete [] lta;
}

bool PickerTSPModule::process(int nsamps, ArrayStruct &data, double first_sample_time, int *trig_sample)
{
    bool new_trigger = false;

    if(nsamps < 1) {
	return new_trigger;
    }
    else if(nsamps > array_size) {
	delete [] sta;
	delete [] lta;
	array_size = nsamps;
	sta = new double[array_size];
	lta = new double[array_size];
    }
    if(in_gap) {
	in_gap = false;
	time_gap_delay = first_sample_time + fabs((double)para_trig_gap_dur);
    }

    double alpha_1_sta = delta_t/para_t_sta;
    double alpha_1_lta = delta_t/para_t_lta;
    double alpha_sta = 1.0 - alpha_1_sta;
    double alpha_lta = 1.0 - alpha_1_lta;

    double cf0 = pow(data.v[0], 2) + para_tau_k * pow(data.v[0] - prev_v, 2);
    sta[0] = alpha_sta * prev_sta  +  alpha_1_sta * cf0;  
    lta[0] = alpha_lta * prev_lta  +  alpha_1_lta * cf0;

    if(sta[0] < para_trig_stab_lev * lta[0]) stabilized = true;

    if(stabilized && first_sample_time > time_gap_delay && first_sample_time > time_untrig
		&& sta[0] > para_trig_lev * lta[0])
    {
	*trig_sample = 0;
	new_trigger = true;
	time_untrig = first_sample_time + para_trig_dur;
    }

    // save sta,lta for output
    data.sta[0] = sta[0];
    data.lta[0] = lta[0];
    for(int i = 1; i < nsamps; i++) {
        double sample_time = first_sample_time + i*delta_t;
        double cfi = pow(data.v[i], 2) + para_tau_k * pow(data.v[i] - data.v[i-1], 2);
        sta[i] = alpha_sta * sta[i-1]  + alpha_1_sta*cfi;  
        lta[i] = alpha_lta * lta[i-1]  + alpha_1_lta*cfi;  
        data.sta[i] = sta[i];
        data.lta[i] = lta[i];

        if(sta[i] < para_trig_stab_lev * lta[i]) stabilized = true;

        if(stabilized && sample_time > time_gap_delay && sample_time > time_untrig
            && sta[i] > para_trig_lev * lta[i])
        {
            *trig_sample = i;
            new_trigger = true;
            time_untrig = sample_time + para_trig_dur;
        }
    }

    // remember the last values for next call  
    prev_v = data.v[nsamps-1];
    prev_sta = sta[nsamps-1];
    prev_lta = lta[nsamps-1];

    return new_trigger;
}

void PickerTSPModule::restart(double /* sample_time */)
{
    // Called when there is a data gap on this channel
    prev_v = 0.0;
    prev_sta = 0.0;
    prev_lta = 0.0;
    in_gap = true;
    if(para_use_trig_stab) stabilized = false;
}
