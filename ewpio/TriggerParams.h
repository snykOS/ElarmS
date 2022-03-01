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
#ifndef __TriggerParams_h_
#define __TriggerParams_h_
#include <vector>
#include "EWPacket.h"
#include "FilterBank.h"
#include "MaxAmplitudes.h"
#include "NEtoZ.h"
#include "RangePostTrig.h"
#include "ZeroCrossings.h"
#include "TimeStamp.h"

#define MAX_TRIGPARAM_PER_PKT 4

class TriggerParams
{
  public:
    char sta[sta_len+1];   // station
    char chan[chan_len+1]; // channel
    char net[net_len+1];   // network
    char loc[loc_len+1];   // location
    double lat; // station latitude (degrees)
    double lon; // station longitude (degrees)
    timeval trigger_time; // epoch time (Unix time)
    int packet_length; // length of the raw Z-component packet that contained the trigger
    int z_recent_sample; // sample offset from the trigger-sample of the most recent z sample received
    int n_recent_sample; // sample offset from the trigger-sample of the most recent n sample received
    int e_recent_sample; // sample offset from the trigger-sample of the most recent e sample received
    float samplerate;  // samples per second
    float toffset;  // trigger_time - packet start time
    double outof_ring;         // trigger packet read from earthworm ring
    double outof_feeder_queue; // the processing thread calls process_packet() in ewfeeder
    double trigger_found;      // the trigger is detected
    double into_send_queue;    // the TriggerParams packet goes into the EWP2Sender queue
    double outof_send_queue;   // EWP2Sender calls activemq to send the TriggerParams message

    FilterBank filter_bank;
    MaxAmplitudes max_amplitudes;
    NEtoZ ne_to_z;
    RangePostTrig range_post_trig;
    ZeroCrossings zero_crossings;

    TimeStamp in_send_queue; // for use in EWP2 only. Not included in message.

    TriggerParams();
    TriggerParams(char *packet, int length);

    virtual ~TriggerParams() {}

    void init();

    static int headerSize() { // uncompressed
	int timeval_size = 2*sizeof(int);
	return    sizeof(EWPacketType)
		+ sizeof(int) // CURRENT_VERSION
	        + sta_len + chan_len + net_len + loc_len
		+ 7*sizeof(double)
		+ timeval_size // trigger_time
		+ 4*sizeof(int) + 2*sizeof(float);
    }
    int packetSize();

    char * serialize(char *packet, int length) throw(std::string);
    char * deserialize(char *packet, int length) throw(std::string);
    std::string toString(bool long_format=false);
    double getTime() { return trigger_time.tv_sec + 1.e-06*trigger_time.tv_usec; }
};

class TriggerParamsBundle
{
  protected:
    char *msg;
    int msg_length;

  public:
    int id;
    PacketSource ps;
    std::vector<TriggerParams> trig_params;

    TriggerParamsBundle() : msg(NULL), msg_length(0) {
	id = 0;
    }

    TriggerParamsBundle(PacketSource source, int packet_id) : msg(NULL), msg_length(0) {
	id = packet_id;
	ps = source;
    }
    TriggerParamsBundle(char *message, int len) : msg(message), msg_length(len) {
	deserialize(msg, msg_length);
    }

    ~TriggerParamsBundle() { }

    static int headerSize() { return sizeof(EWPacketType) + 3*sizeof(int) + sizeof(PacketSource); }
    void clear();
    void add(TriggerParams t) { trig_params.push_back(t); }
    int serialize(PacketSource source, int packet_id, char *serialized_msg, int maxlength);
    int deserialize(char *msg, int length);
    int numParams() { return (int)trig_params.size(); }
    static int headerLength() {
	return sizeof(EWPacketType) + sizeof(PacketSource) + 3*sizeof(int);
    }
    std::string toString(bool long_format=true);
    std::string toString(std::string sta, std::string chan, bool long_format=true, bool first_only=false);
};

#endif
