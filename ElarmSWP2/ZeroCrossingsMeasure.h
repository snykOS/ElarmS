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
#ifndef __ZeroCrossingsMeasure__
#define __ZeroCrossingsMeasure__

#include "Measure.h"
#include "ZeroCrossings.h"
#include "ProcessBuffer.h"

/* the message packet contains:
 * status
 * mean_vel
 * mean_acc
 * zero_cross_vel
 * zero_cross_acc
 */

class ZeroCrossingsMeasure : public Measure
{
  public:
    ZeroCrossingsMeasure(Channel channel);
    virtual ~ZeroCrossingsMeasure();
    void start(ZeroCrossings *m, double trigger_time, double samplerate);
    int update(ZeroCrossings *m, ProcessBuffer &zbuffer, ProcessBuffer &nbuffer, ProcessBuffer &ebuffer);
    double triggerLead();
    double triggerLag();

  protected:
    bool crossing_greater_vel;
    bool crossing_greater_acc;
    int mean_nsamps;
    ArrayStruct zdata;
    static bool have_properties;

    // properties
    static int demean;	      // if true, demean data before counting zero-crossings
    static float demean_lead; // demean starting at trigger_time - zero_cross_demean_lead
    static float lead;	      // zero-cross window begins at trigger_time - zero_cross_lead
    static float lag;	      // zero-cross window ends at trigger_time + zero_cross_lag
};

#endif
