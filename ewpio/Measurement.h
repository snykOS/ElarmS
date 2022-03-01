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
#ifndef __Measurement__
#define __Measurement__

#include <string.h>
#include <string>

#define MEASUREMENT_NAME_SIZE 24
#define MEASUREMENT_WAITING_ON_DATA 0
#define MEASUREMENT_UPDATING 1
#define MEASUREMENT_COMPLETE 2
#define MEASUREMENT_INSUFFICIENT_LEAD 3
#define MEASUREMENT_GAP 4

class Measurement
{
  public:
    Measurement(std::string name);
    virtual void init() {}
    virtual ~Measurement() {}
    virtual int serialize(char * &packet, int length) throw(std::string);
    virtual int deserialize(char * &packet, int maxlength) throw(std::string);
    void startSerialize(char * &p, int length) throw(std::string);
    void startDeserialize(char * &p, int length) throw(std::string);
    virtual int packetSize();
    std::string name() { return measurement_name; }
    int headerSize() { return MEASUREMENT_NAME_SIZE + sizeof(int); }
    int getStatus() { return status; }
    virtual std::string toString();
    static void messageType(char *packet, int packet_len, std::string &name, int *length) throw(std::string);
    int status; // 0: waiting for more data; 1: measurements complete; 2: insufficient data before trigger

  protected:
    std::string measurement_name;
};

#endif
