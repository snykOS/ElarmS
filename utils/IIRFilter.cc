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
/** \file IIRFilter.cpp
 *  \brief Defines class IIRFilter.
 */
#include <iostream>
#include <string.h>
#include <math.h>

#include "IIRFilter.h"

using namespace std;

/* These are the comments from the original fortran code.
 * NAME
 *        iirdes -- (filters) design iir digital filters from analog prototypes
 *
 *        Copyright 1988  regents of the university of california
 *
 *
 * SYNOPSIS
 *        call iirdes( iord, type, fl, fh, ts, sn, sd, nsects )
 *
 *
 *        integer iord     (i)    filter order (10 maximum)
 *        character*2 type (i)    filter type
 *                                lowpass (LP)
 *                                highpass (HP)
 *                                bandpass (BP)
 *                                bandreject (BR)
 *        real fl          (i)    low-frequency cutoff
 *        real fh          (i)    high-frequency cutoff
 *        real ts          (i)    sampling interval (in seconds)
 *        real sn(*)       (o)    array containing numerator coefficients of
 *                                second-order sections packed head-to-tail.
 *        real sd(*)       (o)    array containing denominator coefficients
 *                                of second-order sections packed head-to-tail.
 *        integer nsects   (o)    number of second-order sections.
 *
 * DESCRIPTION
 *        Subroutine to design iir digital filters from analog prototypes.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 *
 *	(415) 423-0617
 *	Translated from Fortran to C by:
 *	C. S. Lynnes
 *	Teledyne Geotech
 *	314 Montgomery
 *	Alexandria, VA  22314
 *	(703) 739-7316
 * 
 *	Ported to C++ by Ivan Henson
 */

/** Constructor.
 *  @param[in] iord the filter order (0 to 10)
 *  @param[in] ftype the filter type: "BP", "BR", "LP", "HP
 *  @param[in] fl   the low frequency cut.
 *  @param[in] fh   the high frequency cut.
 *  @param[in] time_interval the sample time interval in seconds.
 *  @param[in] zp 1 for a zero phase filter, 0 for a causal filter.
 */
IIRFilter::IIRFilter(int iord, const string &ftype, double fl, double fh,
		double time_interval, int zp) :
	order(iord), flow(fl), fhigh(fh), tdel(time_interval), zero_phase(zp),
	nsects(0), nps(0), sn(NULL), sd(NULL), x1(NULL), x2(NULL), y1(NULL),
	y2(NULL)
{
    init(iord, ftype.c_str(), fl, fh, time_interval, zp);
}

/** 
 *  Initialize an IIR filter with the specified parameters.
 *  @param[in] iord the filter order (0 to 10)
 *  @param[in] ftype the filter type: "BP", "BR", "LP", "HP
 *  @param[in] fl   the low frequency cut.
 *  @param[in] fh   the high frequency cut.
 *  @param[in] time_interval the sample time interval in seconds.
 *  @param[in] zp 1 for a zero phase filter, 0 for a causal filter.
 */
void IIRFilter::init(int iord, const char *ftype, double fl, double fh,
			double time_interval, int zp)
{
    double	flw;
    double	fhw;
    Cmplx	p[10];
    char	ptype[10];

    order = iord;
    strncpy(type, ftype, 2);
    type[2] = '\0';
    flow = fl;
    fhigh = fh;
    tdel = time_interval;
    zero_phase = zp;

    if(!strcasecmp(type, "NA")) { // No filter
	nsects = 0;
	sn = NULL;
	sd = NULL;
	x1 = NULL;
	x2 = NULL;
	y1 = NULL;
	y2 = NULL;
	return;
    }

    nps = butterPoles(p, ptype, iord);

    sn = new double[nps*6];
    sd = new double[nps*6];

    if(!strcasecmp(type, "BP")) // BAND_PASS
    {
	fl = fl*tdel/2.;
	fh = fh*tdel/2.;
	flw = tangent_warp( fl, 2. );
	fhw = tangent_warp( fh, 2. );
	LPtoBP(p, ptype, nps, flw, fhw);
    }
    else if(!strcasecmp(type, "BR")) // BAND_REJECT
    {
	fl = fl*tdel/2.;
	fh = fh*tdel/2.;
	flw = tangent_warp( fl, 2. );
	fhw = tangent_warp( fh, 2. );
	LPtoBR(p, ptype, nps, flw, fhw);
    }
    else if(!strcasecmp(type, "LP")) // LOW_PASS
    {
	fh = fh*tdel/2.;
	fhw = tangent_warp( fh, 2. );
	lowpass(p, ptype, nps);
	cutoffAlter(fhw);
    }
    else if(!strcasecmp(type, "HP")) // HIGH_PASS
    {
	fl = fl*tdel/2.;
	flw = tangent_warp( fl, 2. );
	LPtoHP(p, ptype, nps);
	cutoffAlter(flw);
    }
    bilinear();

    x1 = new double[nsects];
    x2 = new double[nsects];
    y1 = new double[nsects];
    y2 = new double[nsects];

    Reset();
}

IIRFilter::~IIRFilter(void)
{
    if(sn) delete [] sn;
    if(sd) delete [] sd;
    if(x1) delete [] x1;
    if(x2) delete [] x2;
    if(y1) delete [] y1;
    if(y2) delete [] y2;
}

/* These are the comments from the original fortran code.
 * NAME
 *         apiir  -- (filters) apply iir filter to a data sequence
 * 
 *         copyright 1988  regents of the university of california
 * 
 *  SYNOPSIS
 *         call apiir( data, nsamps, zp, sn, sd, nsects )
 *         real data       (i/o) array containing data
 *         integer nsamps  (i) number of data samples
 *         logical zp      (i) true for zero phase filtering,
 *                             false for single pass filtering
 *         real sn         (i) numerator polynomials for second
 *                             order sections.
 *         real sd         (i) denominator polynomials for second
 *                             order sections.
 *         integer nsects  (i) number of second-order sections
 * 
 *  DESCRIPTION
 *         Subroutine to apply an iir filter to a data sequence.
 *         The filter is assumed to be stored as second order sections.
 *         Filtering is in-place.
 *         Zero-phase (forward and reverse) is an option.
 * 
 *  WARNINGS
 *         Zero-phase filtering doubles the falloff rate outside of
 *         the band pass; number of poles is effectively doubled.
 * 
 *  AUTHOR
 *         Dave Harris
 *         Lawrence Livermore National Laboratory
 *         L-205
 *         P.O. Box 808
 *         Livermore, CA  94550
 *         (415) 423-0617
 */

bool IIRFilter::apply(float *data, int data_length, bool reset)
{
    if(data_length <= 0) return true;
    if(data == NULL) return false;

    if(reset) Reset();

    applyFilter(data, data_length);
    if(zero_phase) reverse(data, data_length, 1);
    return true;
}

/** Apply filter to a float array.
 *  @param[in] data The float data to be filtered.
 *  @param[in] data_length the length of data[].
 */
void IIRFilter::applyFilter(float *data, int data_length) {
    int	i, j, jptr;
    double input, output;

    for(i = 0; i < data_length; i++) {
	jptr = 0;
	input = (double)data[i];
	output = input;
	for(j = 0; j < nsects; j++) {
	    output = sn[jptr] * input
			+ sn[jptr+1] * x1[j] + sn[jptr+2] * x2[j]
			- ( sd[jptr+1] * y1[j] + sd[jptr+2] * y2[j] );
	    y2[j] = y1[j];
	    y1[j] = output;
	    x2[j] = x1[j];
	    x1[j] = input;

	    jptr += 3;
	    input = output;
	}
	data[i] = (float)output;
    }
}

/** Do the reverse filter for zero-phase filters.
 *  @param[in] data a float array of length data_length.
 *  @param[in] data_length
 *  @param[in] reset = true to reset the recursive filter coefficients,
 *  = false to continue with the last coefficients.
 */
void IIRFilter::reverse(float *data, int data_length, bool reset)
{
    if(reset) Reset();
    doReverse(data, data_length);
}

/** Reverse Filter a float array for zero phase filtering.
 *  @param[in] data The float data to be filtered.
 *  @param[in] data_length the length of data[].
 */
void IIRFilter::
doReverse(float *data, int data_length)
{
    int jptr, ir;
    double input, output;

    for(int i = 0; i < data_length; i++) {
	jptr = 0;
	ir = data_length - 1 - i;
	input = (double)data[ir];
	output = input;
	for(int j = 0; j < nsects; j++) {
	    output = sn[jptr] * input
			+ sn[jptr+1] * x1[j] + sn[jptr+2] * x2[j]
			- ( sd[jptr+1] * y1[j] + sd[jptr+2] * y2[j] );
	    y2[j] = y1[j];
	    y1[j] = output;
	    x2[j] = x1[j];
	    x1[j] = input;

	    jptr += 3;
	    input = output;
	}
	data[ir] = (float)output;
    }
}

/** Reset the recursive filter coefficients to zero.
 */
void IIRFilter::Reset()
{
    for(int i = 0; i < nsects; i++) {
	x1[i] = x2[i] = y1[i] = y2[i] = 0.;
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        bilin2  -- (filters) transforms an analog filter to a digital
 *        filter via the bilinear transformation. Assumes both are stored
 *        as second order sections.  The transformation is done in-place.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call bilin2( sn, sd, nsects )
 *        real sn(*) (i) Array containing numerator polynomial 
 *                       coefficients for second order sections.
 *                       Packed head-to-tail.
 *        real sd(*) (i) Array containing denominator polynomial 
 *                       coefficients for second order sections. 
 *                       Packed head-to-tail.
 *        integer nsects  (i) Number of second order sections.
 *
 * DESCRIPTION
 *        Transforms an analog filter to a digital filter via the bilinear
 *        transformation. Assumes both are stored as second order sections.  
 *        The transformation is done in-place.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *
 */
    
/** Transforms an analog filter to a digital filter via the bilinear
 *  transformation. Assumes both are stored as second order sections.
 *  The transformation is done in-place.
 */
void IIRFilter::bilinear(void)
{
    int i, iptr;
    double a0, a1, a2, scale;
    
    for(i = 0, iptr = 0; i < nsects; i++)
    {
	a0 = sd[iptr];
	a1 = sd[iptr+1];
	a2 = sd[iptr+2];
	scale = a2 + a1 + a0;
	sd[iptr]   = 1.;
	sd[iptr+1] = (2.*(a0 - a2)) / scale;
	sd[iptr+2] = (a2 - a1 + a0) / scale;
	a0 = sn[iptr];
	a1 = sn[iptr+1];
	a2 = sn[iptr+2];
	sn[iptr]   = (a2 + a1 + a0) / scale;
	sn[iptr+1] = (2.*(a0 - a2)) / scale;
	sn[iptr+2] = (a2 - a1 + a0) / scale;
	iptr = iptr + 3;
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        bupoles -- (filters) compute butterworth poles for lowpass filter
 *
 *        Copyright 1988  regents of the university of california
 *
 *
 * SYNOPSIS
 *        call  bupoles( p, type, n, iord )
 *        real p  (o) complex array containing poles contains only one 
 *                    from each complex conjugate pair, and all real poles
 *      character*1(1) type (o) character array indicating pole type:
 *                                'S' -- single real
 *                                'C' -- complex conjugate pair
 *        integer n  (o) number of second order sections
 *        iord       (i) desired number of poles
 *
 * DESCRIPTION
 *        bupoles -- subroutine to compute butterworth poles for
 *        normalized lowpass filter
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 * last modified:  august 4, 1988
 */

/** Compute butterworth poles for lowpass filter.
 *  @param[in] p complex array containing poles contains only one from each
 *	complex conjugate pair, and all real poles
 *  @param[in] ftype character array indicating pole type: 'S' for a single
 *	real, 'C' for a complex conjugate pair
 *  @param[in] iord desired number of poles
 */
int IIRFilter::
butterPoles(Cmplx *p, char *ptype, int iord)
{
    double angle;
    int half, k, n;

    half = iord/2;

    /* test for odd order, and add pole at -1 */

    n = 0;
    if ( 2*half < iord ) {
	p[0] = cmplx( -1., 0. );
	ptype[0] = 'S';
	n = 1;
    }
    for(k = 0; k < half; k++) {
	angle = M_PI * ( .5 + (double)(2*(k+1)-1) / (double)(2*iord) );
	p[n] = cmplx( cos(angle), sin(angle) );
	ptype[n] = 'C';
	n++;
    }
    return(n);
}

/* These are the comments from the original fortran code.
 * NAME
 *        cutoffs -- (filters) Subroutine to alter the cutoff of a filter.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call cutoffs( sn, sd, ns, f )
 *
 *        real sn   (i/o) numerator polynomials for second order sections.
 *        real sd   (i/o) denominator polynomials for second order sections.
 *        integer ns (i/o) number of second-order sections
 *        real f    (i)   new cutoff frequency
 *
 * DESCRIPTION
 *        Subroutine to alter the cutoff of a filter. Assumes that the
 *        filter is structured as second order sections.  Changes
 *        the cutoffs of normalized lowpass and highpass filters through
 *        a simple polynomial transformation.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 */

/** Function to alter the cutoff of a filter. Changes the cutoffs of normalized
 *  lowpass and highpass filters through a simple polynomial transformation.
 *  @param[in] f new cutoff frequency
 */
void IIRFilter::cutoffAlter(double f)
{
    int i, iptr;
    double scale;
    
    scale = 2.*M_PI*f;
    for(i = 0, iptr = 0; i < nsects; i++) {
	sn[ iptr + 1 ] = sn[ iptr + 1 ] / scale;
	sn[ iptr + 2 ] = sn[ iptr + 2 ] / (scale*scale);
	sd[ iptr + 1 ] = sd[ iptr + 1 ] / scale;
	sd[ iptr + 2 ] = sd[ iptr + 2 ] / (scale*scale);
	iptr += 3;
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        lpa -- (filters) generate second order sections from all-pole
 *        description for lowpass filters.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call lpa( p, ptype, np, sn, sd, ns )
 *        real p(*)            (i) array containing poles
 *        character*1 ptype(1) (i) character array containing type
 *                                    information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second order sections
 *
 * DESCRIPTION
 *
 *        Subroutine to generate second order section parameterization
 *        from an all-pole description for lowpass filters.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */
    
/** Generate second order sections from all-pole description for lowpass
 *  filters. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 */
void IIRFilter::lowpass(Cmplx *p, const char *ptype, int np)
{
    int i, iptr;

    nsects = 0;
    for(i = 0, iptr = 0; i < np; i++) {
	if(ptype[i] == 'C') {
	    sn[iptr]     = 1.;
	    sn[iptr + 1] = 0.;
	    sn[iptr + 2] = 0.;
	    sd[iptr] = real_part(cmplx_mul(p[i], cmplx_conjg(p[i])));
	    sd[iptr + 1] = -2. * real_part( p[i] );
	    sd[iptr + 2] = 1.;
	    iptr += 3;
	    nsects++;
	}
	else if(ptype[i] == 'S') {
	    sn[ iptr ]     = 1.;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 0.;
	    sd[ iptr ]     = -real_part( p[i] );
	    sd[ iptr + 1 ] = 1.;
	    sd[ iptr + 2 ] = 0.;
	    iptr += 3;
	    nsects++;
	}
    }
}
/* These are the comments from the original fortran code.
 * NAME
 *        lptbpa -- (filters)  convert all-pole lowpass to bandpass filter
 *        via the analog polynomial transformation.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        call lptbpa( p, ptype, np, fl, fh, sn, sd, ns )
 *        real p(*)            (i)    array containing poles
 *        character*1(1) ptype (i)    array containing type information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real fl     (i) low-frequency cutoff
 *        real fh     (i) high-frequency cutoff
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second-order sections
 *
 * DESCRIPTION
 *
 *        Subroutine to convert an all-pole lowpass filter to a bandpass
 *        filter via the analog polynomial transformation. The lowpass
 *        filter is described in terms of its poles (as input to this
 *        routine). the output consists of the parameters for second order
 *        sections.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */

/** Convert all-pole lowpass to bandpass filter via the analog polynomial
 *  transformation. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 *  @param[in] fl low-frequency cutoff
 *  @param[in] fh high-frequency cutoff
 */
void IIRFilter::LPtoBP(Cmplx *p, const char *ptype, int np, double fl,
				double fh)
{
    Cmplx ctemp, p1, p2;
    double  twopi, a, b;
    int     i, iptr;

    twopi = 2.*M_PI;
    a = twopi*twopi*fl*fh;
    b = twopi*( fh - fl );
    nsects = 0;
    iptr = 0;
    for(i = 0; i < np; i++)
    {
	if(ptype[i] == 'C')
	{
	    ctemp = real_cmplx_mul(b, p[i]);
	    ctemp = cmplx_mul(ctemp, ctemp);
	    ctemp = cmplx_sub(ctemp, cmplx(4.*a, 0.));
	    ctemp = cmplx_sqrt( ctemp );
	    p1 = real_cmplx_mul(0.5, cmplx_add(real_cmplx_mul(b,p[i]), ctemp));
	    p2 = real_cmplx_mul(0.5, cmplx_sub(real_cmplx_mul(b,p[i]), ctemp));
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = b;
	    sn[ iptr + 2 ] = 0.;
	    sd[iptr] = real_part(cmplx_mul(p1, cmplx_conjg(p1)));
	    sd[ iptr + 1 ] = -2. * real_part( p1 );
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = b;
	    sn[ iptr + 2 ] = 0.;
	    sd[iptr] = real_part(cmplx_mul(p2, cmplx_conjg(p2)));
	    sd[ iptr + 1 ] = -2. * real_part( p2 );
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    nsects += 2;
	}
	if(ptype[i] == 'S')
	{
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = b;
	    sn[ iptr + 2 ] = 0.;
	    sd[ iptr ]     = a;
	    sd[ iptr + 1 ] = -b*real_part( p[i] );
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    nsects++;
	}
    }
}
/* These are the comments from the original fortran code.
 * NAME
 *        lptbra -- (filters)  convert all-pole lowpass to band reject
 *        filter via the analog polynomial transformation.
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *
 *        call lptbra( p, ptype, np, fl, fh, sn, sd, ns )
 *        real p(*)            (i) array containing poles
 *        character*1(1) ptype (i) array containing type information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real fl     (i) low-frequency cutoff
 *        real fh     (i) high-frequency cutoff
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second-order sections
 *
 * DESCRIPTION
 *
 *        Subroutine to convert an all-pole lowpass filter to a band
 *        reject filter via the analog polynomial transformation. The
 *        lowpass filter is described in terms of its poles (as input to
 *        this routine). the output consists of the parameters for second
 *        order sections.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */
    
/** Convert all-pole lowpass to band reject filter via the analog polynomial
 *  transformation. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 *  @param[in] fl low-frequency cutoff
 *  @param[in] fh high-frequency cutoff
 */
void IIRFilter::LPtoBR(Cmplx *p, const char *ptype, int np, double fl,
			double fh)
{
    Cmplx pinv, ctemp, p1, p2;
    double  twopi, a, b;
    int     i, iptr;
    
    twopi = 2.*M_PI;
    a = twopi*twopi*fl*fh;
    b = twopi*( fh - fl );
    nsects = 0;
    iptr = 0;
    for(i = 0; i < np; i++)
    {
	if(ptype[i] == 'C')
	{
	    pinv = cmplx_div(cmplx(1.,0.), p[i]);
	    ctemp = cmplx_mul(real_cmplx_mul(b,pinv), real_cmplx_mul(b, pinv));
	    ctemp = cmplx_sub(ctemp, cmplx(4.*a, 0.));
	    ctemp = cmplx_sqrt( ctemp );
	    p1 = real_cmplx_mul(0.5, cmplx_add(real_cmplx_mul(b,pinv), ctemp));
	    p2 = real_cmplx_mul(0.5, cmplx_sub(real_cmplx_mul(b,pinv), ctemp));
	    sn[ iptr ]     = a;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[iptr] = real_part(cmplx_mul(p1, cmplx_conjg(p1)));
	    sd[ iptr + 1 ] = -2. * real_part(p1);
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    sn[ iptr ]     = a;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[iptr] = real_part(cmplx_mul(p2, cmplx_conjg(p2)));
	    sd[ iptr + 1 ] = -2. * real_part(p2);
	    sd[ iptr + 2 ] = 1.;
	    iptr = iptr + 3;
	    nsects += 2;
	}
	else if(ptype[i] == 'S')
	{
	    sn[ iptr ]     = a;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[ iptr ]     = -a*real_part( p[i] );
	    sd[ iptr + 1 ] = b;
	    sd[ iptr + 2 ] = -real_part( p[i] );
	    iptr = iptr + 3;
	    nsects++;
	}
    }
}
/* These are the comments from the original fortran code.
 * NAME
 *        lpthpa -- (filters)  convert all-pole lowpass to high pass
 *        filter via the analog polynomial transformation.
 *
 *        Copyright 1988  regents of the university of california
 *
 *
 * SYNOPSIS
 *        call lpthpa( p, ptype, np, fl, fh, sn, sd, ns )
 *        real p(*)            (i) array containing poles
 *        character*1(1) ptype (i) array containing type information
 *                                    'S' -- single real pole or
 *                                    'C' -- complex conjugate pair
 *        integer np  (i) number of real poles and complex conjugate pairs
 *        real fl     (i) low-frequency cutoff
 *        real fh     (i) high-frequency cutoff
 *        real sn     (o) numerator polynomials for second order sections.
 *        real sd     (o) denominator polynomials for second order sections.
 *        integer ns  (o) number of second-order sections
 *
 * DESCRIPTION
 *        Subroutine to convert an all-pole lowpass filter to a high pass
 *        filter via the analog polynomial transformation. The lowpass
 *        filter is described in terms of its poles (as input to this
 *        routine). the output consists of the parameters for second
 *        order sections.
 *        
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */
    
/** Convert all-pole lowpass to high pass filter via the analog polynomial
 *  transformation. This functions set the nsects, sn[] and sd[] values.
 *  @param[in] p array containing poles.
 *  @param[in] ptype  character array containing type information. 'S' for a
 *	single real pole or 'C' for a complex conjugate pair.
 *  @param[in] np number of real poles and complex conjugate pairs.
 */
void IIRFilter::LPtoHP(Cmplx *p, const char *ptype, int np)
{
    int i, iptr;

    nsects = 0;
    for(i = 0, iptr = 0; i < np; i++) {
	if(ptype[i] == 'C') {
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = 0.;
	    sn[ iptr + 2 ] = 1.;
	    sd[ iptr ]     = 1.;
	    sd[ iptr + 1 ] = -2. * real_part( p[i] );
	    sd[ iptr + 2 ] = real_part(cmplx_mul(p[i], cmplx_conjg(p[i])));
	    iptr += 3;
	    nsects++;
	}
	else if(ptype[i] == 'S') {
	    sn[ iptr ]     = 0.;
	    sn[ iptr + 1 ] = 1.;
	    sn[ iptr + 2 ] = 0.;
	    sd[ iptr ]     = 1.;
	    sd[ iptr + 1 ] = -real_part( p[i] );
	    sd[ iptr + 2 ] = 0.;
	    iptr += 3;
	    nsects++;
	}
    }
}

/* These are the comments from the original fortran code.
 * NAME
 *        warp -- (filters) function, applies tangent frequency warping
 *        to compensate for bilinear analog -> digital transformation
 *
 *        Copyright 1988  regents of the university of california
 *
 * SYNOPSIS
 *        real function warp(f,t)
 *
 *                f  original design frequency specification (hertz)
 *                t  sampling interval
 *
 *
 * DESCRIPTION
 *
 *        warp -- function, applies tangent frequency warping to
 *                compensate for bilinear analog -> digital transformation
 *
 * DIAGNOSTICS
 *        none
 *
 * RESTRICTIONS
 *        unknown.
 *
 * BUGS
 *        unknown.
 *
 * AUTHOR
 *        Dave Harris
 *
 *        Lawrence Livermore National Laboratory
 *        L-205
 *        P.O. Box 808
 *        Livermore, CA  94550
 *        USA
 *        (415) 423-0617
 */

/** Applies tangent frequency warping to compensate for bilinear analog to
 *  digital transformation.
 *  @param[in] f original design frequency specification (hertz)
 *  @param[in] t sampling interval
 */
double IIRFilter::
tangent_warp(double f, double t)
{
    double twopi;
    double angle;
    double warp;
    double fac;
    
    twopi = 2.*M_PI;
    fac = .5*f*t;
    if(fac >= .25) fac = .2499999;
    angle = fac*twopi;
    warp = 2.*tan(angle)/t;
    warp = warp/twopi;
    return(warp);
}

/** Complex addition.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 + c2
 */
Cmplx IIRFilter::cmplx_add(Cmplx c1, Cmplx c2)
{
    Cmplx	csum;
    csum.r = c1.r + c2.r;
    csum.i = c1.i + c2.i;
    return (csum);
}
/** Complex subtraction.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 - c2
 */
Cmplx IIRFilter::cmplx_sub(Cmplx c1, Cmplx c2)
{
    Cmplx	csum;
    csum.r = c1.r - c2.r;
    csum.i = c1.i - c2.i;
    return (csum);
}
/** Complex multiplication.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 * c2
 */
Cmplx IIRFilter::cmplx_mul(Cmplx c1, Cmplx c2)
{
    Cmplx	c;
    c.r = c1.r * c2.r - c1.i * c2.i;
    c.i = c1.i * c2.r + c1.r * c2.i;
    return (c);
}
/** Complex division.
 *  @param[in] c1 a complex number.
 *  @param[in] c2 a complex number.
 *  @returns c1 / c2
 */
Cmplx IIRFilter::cmplx_div(Cmplx c1, Cmplx c2)
{
    double	a, b, c, d;
    double	f;
    Cmplx	ratio;
    a = c1.r;
    b = c1.i;
    c = c2.r;
    d = c2.i;
    f = c*c + d*d;
    ratio.r = (a*c + b*d) / f;
    ratio.i = (b*c - a*d) / f;
    return (ratio);
}
/** Return the complex structure.
 */
Cmplx IIRFilter::cmplx(double fr, double fi)
{
    Cmplx	c;
    c.r = fr;
    c.i = fi;
    return (c);
}
/** Return the complex conjugate.
 */
Cmplx IIRFilter::cmplx_conjg(Cmplx c)
{
    c.i = -c.i;
    return (c);
}
/** Return the sqrt of a complex number.
 */
Cmplx IIRFilter::cmplx_sqrt(Cmplx c)
{
    double	radius, theta;
    double	c_r, c_i;
    c_r = c.r;
    c_i = c.i;
    radius = sqrt(sqrt(c_r*c_r + c_i*c_i));
    if (c_r == 0.)
    {
	theta = M_PI / 4.;
    }
    theta = 0.5 * atan2(c_i, c_r);
    c.r = radius * cos(theta);
    c.i = radius * sin(theta);
    return (c);
}
/** Multiply a complex  number by a real number.
 */
Cmplx IIRFilter::real_cmplx_mul(double r, Cmplx c)
{
    c.r *= r;
    c.i *= r;
    return (c);
}
/** Return the real part of a complex number.
 */
double IIRFilter::real_part(Cmplx c)
{
    return (c.r);
}
