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
#include "IIRFilter.h"
#include "FilterBands.h"

FilterBands::FilterBands() : vel(NULL), size_vel(0), samplerate(0.)
{
}

void FilterBands::setBands(double samprate, vector<FrequencyBand> &fbands)
{
    int order = 2;
    string type = "BP";
    bool zero_phase = false;

    for(int i = 0; i < (int)iir.size(); i++) delete iir[i];
    iir.clear();

    samplerate = samprate;
    double sample_interval = 1./samplerate;

    frequency_bands.clear();
    for(int i = 0; i < (int)fbands.size(); i++) {
	frequency_bands.push_back(fbands[i]);
	iir.push_back(new IIRFilter(order, type, fbands[i].flow, fbands[i].fhigh,
				sample_interval, zero_phase));
    }
}

FilterBands::~FilterBands()
{
    if(vel != NULL) delete [] vel;
    for(int i = 0; i < (int)iir.size(); i++) delete iir[i];
}

void FilterBands::getMaxAmp(int ndata, ArrayStruct &data, float *pgv)
{
    if(ndata > size_vel) {
	if(vel) delete [] vel;
	vel = new float[ndata];
	size_vel = ndata;
    }

    for(int i = 0; i < (int)iir.size(); i++) {
	for(int j = 0; j < ndata; j++) vel[j] = data.v[j];
	demean(vel, ndata);
	taperBeg(vel, ndata, 10);
	iir[i]->apply(vel, ndata, true);
	pgv[i] = getMax(vel, ndata);
    }
}

void FilterBands::demean(float *vel, int npts)
{
    if(npts > 0) {
	double mean = 0.;
	for(int i = 0; i < npts; i++) mean += vel[i];
	mean /= npts;
	for(int i = 0; i < npts; i++) vel[i] -= mean;
    }
}

// cosine taper the first taper_len points of the data

void FilterBands::taperBeg(float *vel, int npts, int taper_len)
{
    if(taper_len > npts) taper_len = npts;
    if(taper_len > 0)
    {
	double arg = M_PI/(double)taper_len;
	for(int i = 0; i < taper_len; i++)
	{
	    double taper = .5*(1. - cos(i*arg));
	    vel[i] *= taper;
	}
    }
}

float FilterBands::getMax(float *vel, int npts)
{
    float max = 0.;
    for(int i = 0; i < npts; i++) {
 	float m = fabs(vel[i]);
	if(m > max) max = m;
    }
    return (max > 0.) ? log10(max) : 0.;
}
