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
#ifndef __FilterBankMeasure_h_
#define __FilterBankMeasure_h_

#include <vector>
#include "Measure.h"
#include "FilterBank.h"
#include "FilterBands.h"

class TimeWindow {
  public:
    float lead; // time before trigger
    float lag;  // time after trigger;
    TimeWindow(float leadsecs, float lagsecs) {
	lead = leadsecs;
	lag = lagsecs;
    }
};

class FilterBankMeasure : public Measure
{
  public:
    FilterBankMeasure(Channel channel);
    virtual ~FilterBankMeasure();
    void start(FilterBank *m, double trigger_time, double samplerate);
    int update(FilterBank *m, ProcessBuffer &zbuffer, ProcessBuffer &nbuffer, ProcessBuffer &ebuffer);
    double triggerLead();
    double triggerLag();

  protected:
    std::vector<int> window_end;
    FilterBands filters;
    std::vector<FrequencyBand> fbands;
    ArrayStruct zdata;
    bool applyFilters(TimeWindow window, ProcessBuffer &zbuffer, FilterWindow &fw);
    std::string triggerString();
    static bool have_properties;

    // properties
    static bool z_fbands_only;
    static bool compute_fbands;
    static std::vector<TimeWindow> windows;
    static double filter_bank_window_lead;
    static double samprate_fbands_max[2];
    static std::vector<FrequencyBand> fbands1;
    static std::vector<FrequencyBand> fbands2;
    static std::vector<FrequencyBand> fbands3;
    static bool verbose;
};

#endif
