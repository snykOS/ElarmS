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
#ifndef __ToGroundTSPModule_h
#define __ToGroundTSPModule_h

#include <string>
#include "ProcessBuffer.h"
#include "Channel.h"
#include "TimeStamp.h"
#include "IIRFilter.h"

class ToGroundTSPModule
{
 private:
    Channel channel;
    int array_size;
    double delta_t; // Sampling interval =  1/channel.samplerate
    double gain;    // Gain factor 

    enum { velocity, acceleration } waveform_type;    
    
    static bool have_properties;
    static double para_lambda;
    static double para_t_signal;
    static double para_tau_k;
    static double para_t_noise;
    static double para_filt_hpfc;
    static int para_filt_order;
    static double para_filt_lpfc;
    static double para_blwin;

    double alpha;
    double alpha_signal;
    double alpha_noise;
    
    // Variables 
    bool was_gap;
    double var_z0;
    double var_a0;
    double var_v0;
    double var_vlp0;
    double var_sigma0;
    double var_y0;
    double var_r0;
    double var_s0;
    double var_n0;
    float var_vuf0;
    float var_duf0;
    
    float *vlp;
    double *sigma;
    double *y;
    float *r;
    float* z;

    // baseline removal
    int* bl_array;
    size_t bl_index;
    size_t bl_size;
    long running_sum;
    double long_term_av;
    size_t max_samples;
    float dt2;

    // external filters
    IIRFilter* riir;
    IIRFilter* aiir;
    IIRFilter* viir;
    IIRFilter* diir;
    IIRFilter* vlpiir;

 public:
    ToGroundTSPModule(Channel);
    virtual ~ToGroundTSPModule() {
	delete [] vlp;
	delete [] sigma;
	delete [] y;
	delete [] r;
    delete [] bl_array;
    delete [] z;
	(void)BINARY; (void)MEMORY; (void)ASCII; (void)FILENAME; (void)DATABASE; // avoid compiler used warning
    }

    void process(int nsamps, ArrayStruct &data);
    void restart();
};

#endif
