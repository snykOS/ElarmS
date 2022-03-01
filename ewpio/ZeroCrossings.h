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
#ifndef __ZeroCrossings__
#define __ZeroCrossings__

#include <string>
#include "Measurement.h"

/* the message packet contains:
 * status
 * mean_vel
 * mean_acc
 * zero_cross_vel
 * zero_cross_acc
 */

class ZeroCrossings : public Measurement
{
  public:
    ZeroCrossings();
    void init();
    int serialize(char * &packet, int length) throw(std::string);
    int deserialize(char * &packet, int maxlength) throw(std::string);
    int packetSize();
    std::string toString();

    double mean_vel; // velocity mean
    double mean_acc; // acceleration mean
    int zero_cross_vel; // number of velocity zero-crossings in zero-cross window
    int zero_cross_acc; // number of acceleration zero-crossings in zero-cross window

    int operator==(const ZeroCrossings &rhs) const {
        if(mean_vel != rhs.mean_vel) return 0;
        if(mean_acc != rhs.mean_acc) return 0;
        if(zero_cross_vel != rhs.zero_cross_vel) return 0;
        if(zero_cross_acc != rhs.zero_cross_acc) return 0;
        return 1;
    }
    bool operator!=(const ZeroCrossings &rhs) const {
	return !(*this == rhs);
    }
};

#endif
