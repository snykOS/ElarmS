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
#include <iomanip>
#include "ToGroundTSPModule.h"
#include "ElarmSWP2.h"
#include "PrintLock.h"
#include "dsp.h"
#include "GetProp.h"

bool    ToGroundTSPModule::have_properties = false;
double	ToGroundTSPModule::para_lambda = 0.;
double	ToGroundTSPModule::para_t_signal = 0.;
double	ToGroundTSPModule::para_tau_k = 0.;
double	ToGroundTSPModule::para_t_noise = 0.;
double	ToGroundTSPModule::para_filt_hpfc = 0.;
int	    ToGroundTSPModule::para_filt_order = 0;
double	ToGroundTSPModule::para_filt_lpfc = 0.;
double	ToGroundTSPModule::para_blwin = 0.;


#define check_value(name, value) \
    if(value == 0.0) { \
	LOG_INFO << "ToGroundTSPModule:" << name << "(" << value \
		 << ") is too small. Using value 1.e-10."; \
	value = 1.e-10; \
    }

ToGroundTSPModule::ToGroundTSPModule(Channel chan) : channel(chan)
{
    array_size = 100; // adjust if necessary in process()

    vlp = new float[array_size];
    sigma = new double[array_size];
    y = new double[array_size];
    r = new float[array_size];
    z = new float[array_size];

    switch (chan.channel[1]) {
	case 'N': waveform_type = acceleration; break;
	case 'L': waveform_type = acceleration; break;
	case 'H': waveform_type = velocity; break;
	default : waveform_type = velocity; break;
    }
    delta_t = 1.0/chan.samprate;
    check_value("delta_t", delta_t);

    PrintLock::lock(); // lock to get properties and to print
    if(!have_properties) {
	have_properties = true;
        GetProp *prop = GetProp::getInstance();
    para_t_signal = prop->getDouble("TSCTSignal");
	para_t_noise = prop->getDouble("TSCTNoise");
	para_lambda = prop->getDouble("TSCTauP");
	para_tau_k = prop->getDouble("RICTauK");
    para_filt_hpfc = prop->getDouble("HPFreqCutoff");
    para_filt_order = prop->getInt("HPFilterOrder");
    para_filt_lpfc = prop->getDouble("LPFreqCutoff");
    para_blwin = prop->getDouble("BaselineWin");

	stringstream msg_str;
	msg_str << fixed << setprecision(4);
	msg_str << "P: ToGround.TSCTSignal: " << para_t_signal << endl;
	msg_str << "P: ToGround.TSCTNoise: " << para_t_noise << endl;
	msg_str << "P: ToGround.TSCTauP: " << para_lambda << endl;
	msg_str << "P: ToGround.RICTauK: " << para_tau_k << endl;
	msg_str << "P: ToGround.HPFreqCutoff: " << para_filt_hpfc << endl;
	msg_str << "P: ToGround.HPFilterOrder: " << para_filt_order << endl;
	msg_str << "P: ToGround.LPFreqCutoff: " << para_filt_lpfc << endl;
	msg_str << "P: ToGround.BaselineWin: " << para_blwin;
	LOG_INFO << msg_str.str();
	ElarmSWP2::param_str << msg_str.str();
    }
    PrintLock::unlock();

    gain = chan.gain;
    check_value("gain", gain);

    alpha = (para_lambda - delta_t)/para_lambda ;
    alpha_signal = (para_t_signal - delta_t)/para_t_signal;
    alpha_noise = (para_t_noise - delta_t)/para_t_noise;

    // Baseline removal
    max_samples = para_blwin * chan.samprate;
    bl_array = new int[max_samples];

    // Integrate/differentiate
    dt2 = delta_t/2.0;

    // IIRFilter
    riir = new IIRFilter(para_filt_order, "HP", para_filt_hpfc, 0., delta_t, 0);
    aiir = new IIRFilter(para_filt_order, "HP", para_filt_hpfc, 0., delta_t, 0);
    viir = new IIRFilter(para_filt_order, "HP", para_filt_hpfc, 0., delta_t, 0);
    diir = new IIRFilter(para_filt_order, "HP", para_filt_hpfc, 0., delta_t, 0);
    vlpiir = new IIRFilter(para_filt_order, "LP", 0., para_filt_lpfc, delta_t, 0);

    // zero variables
    restart();
}

void ToGroundTSPModule::process(int nsamps, ArrayStruct &data)
{
    if(nsamps < 1 ) {
        return;
    } 
    if(nsamps > array_size) {
        delete [] vlp;
        delete [] sigma;
        delete [] y;
        delete [] r;
        delete [] z;
        array_size = nsamps;
        vlp = new float[array_size];
        sigma = new double[array_size];
        y = new double[array_size];
        r = new float[array_size];
        z = new float[array_size];
    }

    // Gap handling
    if (was_gap) {
        was_gap = false;
    }

    // Array baseline correction
    for(int i = 0; i < nsamps; i ++ ) {
        running_sum -= bl_array[bl_index];
        running_sum += data.raw[i];
        if (bl_size < max_samples) bl_size++;
        bl_array[bl_index] = data.raw[i];
        bl_index++;
        if (bl_index == max_samples) bl_index = 0;
        long_term_av = (long double)running_sum / (double)bl_size;
        z[i] = (data.raw[i]-long_term_av)/gain;
        r[i] = data.raw[i]; // for SNR
        if(waveform_type == acceleration) {
            data.a[i] = z[i];
        } else {
            data.v[i] = z[i]; 
        }
    }

    if(waveform_type == acceleration) {
        // 2-pole filter raw -> acc
        aiir->apply(data.a, nsamps, false);
        // integration acc -> vel
        mathfn::integrate<float>(data.a, data.v, nsamps, (float)var_a0, var_vuf0, dt2);
        var_vuf0 = data.v[nsamps-1];
    } else { // velocity
        // differentiate vel -> acc
        mathfn::differentiate<float>(z, data.a, nsamps, (float)var_z0, (float)delta_t);
        // 2-pole filter acc
        aiir->apply(data.a, nsamps, false);
    }

    var_a0 = data.a[nsamps-1];
    // 2-pole filter vel
    viir->apply(data.v, nsamps, false);
    // fill vlp array ready for lowpass filtered vel
    for (int i=0; i<nsamps; i++) {
        vlp[i] = data.v[i];
    }

    // integrate vel -> disp
    mathfn::integrate<float>(data.v, data.d, nsamps, (float)var_v0, var_duf0, dt2);
    var_v0 = data.v[nsamps-1];
    var_duf0 = data.d[nsamps-1];
    // 2-pole filter disp
    diir->apply(data.d, nsamps, false);

    // Low-passed velocity (not output)
    vlpiir->apply(vlp, nsamps, false);
    // High-Passed raw value (not output)
    riir->apply(r, nsamps, false);
    for (int i=0; i<nsamps; i++) {
        sigma[i] = alpha * var_sigma0 + pow((vlp[i] - var_vlp0)/delta_t, 2);
        y[i] = alpha * var_y0 + vlp[i] * vlp[i];

        // limit taup to (0.01, 100) to avoid overflow problems caused by near flat-line data
        if(y[i] > pow(0.01/(2.0*M_PI), 2)*sigma[i] && y[i] < pow(100.0/(2.0*M_PI), 2)*sigma[i]) {
            // equation (1) of Wurman et al 2007
            data.t[i] = (sigma[i] == 0.0) ? 0.0 : 2.0*M_PI*sqrt(fabs(y[i]/sigma[i])); 
        } else {
            data.t[i] = 0.0;
        }

        double delta_z = (z[i] - var_z0);

        // Signal & Noise
        double  cf = r[i] * r[i] +  para_tau_k  * delta_z * delta_z;
        data.s[i] = alpha_signal * var_s0 + (1. - alpha_signal) * cf; 
        data.n[i] = alpha_noise * var_n0 + (1. - alpha_noise) * cf;
        var_z0 = z[i];

        var_y0 = y[i];
        var_sigma0 = sigma[i];
        var_s0 = data.s[i];
        var_n0 = data.n[i];
        var_z0 = z[i];
        var_vlp0 = vlp[i];
    }
}

void ToGroundTSPModule::restart()
{
    was_gap = true;  
 
    for (int i=0; i<array_size; i++) {
        vlp[i] = 0.;
        sigma[i] = 0.;
        y[i] = 0.;
        r[i] = 0.;
        z[i] = 0.;
    }

    // Set history values for next timestep
    var_z0 = 0.0;
    var_a0 = 0.0;
    var_v0 = 0.0;
    var_vuf0 = 0.0;
    var_duf0 = 0.0;
    var_vlp0 = 0.0;
    var_sigma0 = 0.0;
    var_y0 = 0.0;
    var_s0 = 0.0;
    var_n0 = 0.0;

    long_term_av = 0.;
    running_sum = 0;
    for (size_t i=0; i<max_samples; i++) {
        bl_array[i] = 0;
    }
    bl_index = 0;
    bl_size = 0;

    riir->Reset();
    aiir->Reset();
    viir->Reset();
    diir->Reset();
    vlpiir->Reset();
}
