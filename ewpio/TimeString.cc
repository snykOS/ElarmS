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

#include "TimeString.h"
#include "qlib2.h"

using namespace std;

string TimeString::toString(double time, int decimals)
{
    double t;
    string fmt = "%04d-%02d-%02dT%02d:%02d:";

    // print the epoch time using 0 to 4 decimals for the seconds.
    // First round to the nearest .decimals seconds before INT_TIME,
    // so the print %f does not round up to 60 seconds
    if(decimals < 0) decimals = 0;
    switch(decimals) {
        case 0: t = round(time); fmt.append("%02.0f"); break;
        case 1: t = round(10*time)/10.; fmt.append("%04.1f"); break;
        case 2: t = round(100*time)/100.; fmt.append("%05.2f"); break;
        case 3: t = round(1000*time)/1000.; fmt.append("%06.3f"); break;
        default: t = round(10000*time)/10000.; fmt.append("%07.4f"); break;
    }

    INT_TIME it = nepoch_to_int(t);
    EXT_TIME et = int_to_ext(it);
    char c[100];
    snprintf(c, sizeof(c), fmt.c_str(), et.year, et.month, et.day,
                et.hour, et.minute, (float)et.second + 1.e-06*et.usec);
    return string(c);
}
