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
#ifndef _EWP2_h_
#define _EWP2_h_

#include <string.h>
#include <math.h>

#include "wp.h"
#include "ProcessBuffer.h"
#include "ToGroundTSPModule.h"
#include "GMPeakAmpsTSPModule.h"
#include "PickerTSPModule.h"
#include "TriggerParamsTSPModule.h"
#include "GMPeak.h"
#include "EWP2Sender.h"

using namespace std;

class EWP2 : public WP
{
  private:
    ToGroundTSPModule *togZ;
    ToGroundTSPModule *togN;
    ToGroundTSPModule *togE;

    GMPeakAmpsTSPModule *gmpeakZ;
    GMPeakAmpsTSPModule *gmpeakN;
    GMPeakAmpsTSPModule *gmpeakE;

    PickerTSPModule *pickerZ;

    TriggerParamsTSPModule *trigpar;

    ProcessBuffer Zbuffer;
    ProcessBuffer Nbuffer;
    ProcessBuffer Ebuffer;

    EWP2Sender *sender;

    bool process_trigger;
//    int size_data;
//    SampleStruct **data;
    ArrayStruct data;
    PacketInfo packet_info;
    map<string, int> file_start;
    double packet_start_time;
    string packet_channel;

    bool createModules();

    void warnOverlap(int overlap, RawPacket &p);

    void writeHeader(FILE *fp, RawPacket &p, int type);
    void writeComponent(string prefix, RawPacket &p, int ndata, ArrayStruct &data, int start);
    void writeData(RawPacket &p, int ndata, ArrayStruct &data);

  public:
    EWP2(Channel Z, Channel E, Channel N, EWP2Sender *ewp2_sender) :
	WP(Z,E,N), togZ(NULL), togN(NULL), togE(NULL), gmpeakZ(NULL), gmpeakN(NULL), gmpeakE(NULL),
	pickerZ(NULL), trigpar(NULL), sender(ewp2_sender)
    {
	use_raw_packet = true;
    }

    virtual ~EWP2();
  
    bool process(RawPacket &p);

    // needed only for WP class
    void record(int duration) { (void)(duration); }
    void reset();
    void stoprecord() {}
    EWP2Sender * getSender() { return sender; }

/*
    void getChannels(Channel &z, Channel &e, Channel &n) {
	z = chanZ;
	e = chanE;
	n = chanN;
    }
*/
};

#endif
