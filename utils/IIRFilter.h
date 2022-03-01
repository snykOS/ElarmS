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
#ifndef _IIR_FILTER_H
#define _IIR_FILTER_H
#include <iostream>

typedef struct
{
    double   r;
    double   i;
} Cmplx;

/**
 *  This is a translation of the Fortran code for iir digital filters written
 *  by Dave Harris, Lawrence Livermore National Laboratory.
 */
class IIRFilter
{
    public:
	IIRFilter(int order, const std::string &type, double fl, double fh, double tdel, int zero_phase);
	~IIRFilter(void);

	bool apply(float *data, int data_length, bool reset=true);

	/** Get the filter order.
	 *  @returns the order of the filter.
	 */
	int getOrder() { return order; }
	/** Get the filter type.
	 *  @returns the type of the filter. "BP, "BR", "LP", "HP", or "NA"
	 */
	char *getType() { return type; }
	/** Get the filter low frequency cutoff.
	 *  @returns the low frequency filter cutoff.
	 */
	double getFlow() { return flow; }
	/** Get the filter high frequency cutoff.
	 *  @returns the high frequency filter cutoff.
	 */
	double getFhigh() { return fhigh; }
	/** Get the sample time increment.
	 *  @returns the sample time increment.
	 */
	double getTdel() { return tdel; }
	/** Get the zero-phase flag.
	 *  @returns 1 for a zero phase filter, 0 for a causal filter.
	 */
	int getZeroPhase() { return zero_phase; }

	void applyFilter(float *data, int data_length);
	void Reset(void);

    protected:
	int	order;	    //!< the order of the filter
	char	type[3];    //!< the type: "BP, "BR", "LP", "HP", or "NA"
	double	flow;       //!< the filter low frequency cutoff.
	double	fhigh;      //!< the high frequency filter cutoff.
	double	tdel;       //!< the sample time increment
	int	zero_phase; //!< the zero-phase flag.
	int	nsects;     //!< the number of second-order sections.
	int	nps;	    //!< the number of butter poles.
	double	*sn; //!< numerator polynomials for second order sections.
	double	*sd; //!< denominator polynomials for second order sections.
	double	*x1; //!< nsects coefficients needed for the recursive filter
	double	*x2; //!< nsects coefficients needed for the recursive filter
	double	*y1; //!< nsects coefficients needed for the recursive filter
	double	*y2; //!< nsects coefficients needed for the recursive filter

	void init(int iord, const char *ftype, double fl, double fh,
			double tdel, int zero_phase);

	void reverse(float *data, int data_length, bool reset);
	void bilinear(void);
	int butterPoles(Cmplx *p, char *ptype, int iord);
	void cutoffAlter(double f);
	void lowpass(Cmplx *p, const char *ptype, int np);
	void LPtoBP(Cmplx *p, const char *ptype, int np, double fl,double fh);
	void LPtoBR(Cmplx *p, const char *ptype, int np, double fl,double fh);
	void LPtoHP(Cmplx *p, const char *ptype, int np);

	void doReverse(float *data, int data_length);
	double tangent_warp(double f, double t);
	Cmplx cmplx_add(Cmplx c1, Cmplx c2);
	Cmplx cmplx_sub(Cmplx c1, Cmplx c2);
	Cmplx cmplx_mul(Cmplx c1, Cmplx c2);
	Cmplx cmplx_div(Cmplx c1, Cmplx c2);
	Cmplx cmplx(double fr, double fi);
	Cmplx cmplx_conjg(Cmplx c);
	Cmplx cmplx_sqrt(Cmplx c);
	Cmplx real_cmplx_mul(double r, Cmplx c);
	double real_part(Cmplx c);
};

#endif
