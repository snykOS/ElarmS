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
#ifndef __FilterBank_h_
#define __FilterBank_h_

#include "Measurement.h"

#define NUM_FBANDS 9
#define NUM_FWINDOWS 11

// log(max velocity) in NUM_FBANDS frequency bands using NUM_FWINDOWS time window lengths
// All windows start at trigger_time - 30secs and extend to trigger_time +
// window_lag (.1,.2,.3,.4,.5,.6,.7,.8,.9,1,2 secs)
// bands = 24-48, 12-24, 6-12, 3-6, 1.5-3, 0.75-1.5, 0.375-0.75, 0.375-0.1875, 0.09375-0.1875Hz
// pgv[i] = log(max velocity) in each frequency band

class FilterWindow {
  public:
    FilterWindow();
    int window_npts;
    float window_lead;
    float window_lag;
    float pgv[NUM_FBANDS];
    std::string toString();
    std::string toString(bool *tele);
    bool isMeasured() { return (window_npts > 0); }
    int operator==(const FilterWindow &rhs) const {
	if(window_npts != rhs.window_npts
		|| window_lead != rhs.window_lead
		|| window_lag != rhs.window_lag) return 0;
	for(int i = 0; i < NUM_FBANDS; i++) {
	    if(pgv[i] != rhs.pgv[i]) return 0;
	}
	return 1;
    }
    bool operator!=(const FilterWindow &rhs) const {
	return !(*this == rhs);
    }
};

class FilterBank : public Measurement
{
  public:
    FilterBank();
    void init();
    int serialize(char * &packet, int length) throw(std::string);
    int deserialize(char * &packet, int maxlength) throw(std::string);
    int packetSize();
    std::string toString(std::string prefix="");
    FilterWindow fw[NUM_FWINDOWS];
    int operator==(const FilterBank &rhs) const {
	for(int i = 0; i < NUM_FWINDOWS; i++) {
	    if(fw[i] != rhs.fw[i]) return 0;
	}
	return 1;
    }
    bool operator!=(const FilterBank &rhs) const {
	return !(*this == rhs);
    }
};

#endif
