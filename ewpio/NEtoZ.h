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
#ifndef __NEtoZ__
#define __NEtoZ__

#include <string>
#include "Measurement.h"

/* the message packet contains:
 * status
 * start_lag
 * end_lag
 * max_n
 * max_e
 * max_z
 */

class NEtoZ : public Measurement
{
  public:
    NEtoZ();
    void init();
    int serialize(char * &packet, int length) throw(std::string);
    int deserialize(char * &packet, int maxlength) throw(std::string);
    int packetSize();
    std::string toString();

    float n_min;   // maximum vel N component in start_ne_to_z to end_ne_to_z window
    float n_max;   // maximum vel N component in start_ne_to_z to end_ne_to_z window
    float e_min;   // maximum vel E component in start_ne_to_z to end_ne_to_z window
    float e_max;   // maximum vel E component in start_ne_to_z to end_ne_to_z window
    float z_min;   // maximum vel Z component in start_ne_to_z to end_ne_to_z window
    float z_max;   // maximum vel Z component in start_ne_to_z to end_ne_to_z window
    bool start;

    int operator==(const NEtoZ &rhs) const {
	if(n_min != rhs.n_min) return 0;
	if(n_max != rhs.n_max) return 0;
	if(e_min != rhs.e_min) return 0;
	if(e_max != rhs.e_max) return 0;
	if(z_min != rhs.z_min) return 0;
	if(z_max != rhs.z_max) return 0;
	return 1;
    }
    bool operator!=(const NEtoZ &rhs) const {
	return !(*this == rhs);
    }
    double ne_to_z();
};

#endif
