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
#ifndef _EWP2_Sender_h_
#define _EWP2_Sender_h_

#include <math.h>
#include <plog/Log.h>
#include "Duration.h"
#include "Channel.h"
#include "ProcessBuffer.h"
#include "TriggerParams.h"
#include "GMPeak.h"
#include "HBProducer.h"
//#include "EventListener.h"
#include "PrintLock.h"
#include "EWTrigger.h"

#include <string.h>
#include <deque>
#include <pthread.h>
#include <math.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <cms/ExceptionListener.h>

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;

#define MAX_PACKET_INFO 20 // must be >= 1

/*
class TriggerParamsT {
  public:
    TimeStamp queue_time;
    TriggerParams o;
    int num_packets;
    PacketInfo packets[MAX_PACKET_INFO];
    TriggerParamsT(TriggerParams tp, TimeStamp qt, int num_trig_packets, PacketInfo *packet_info)
		: queue_time(qt), o(tp)
    {
	num_packets = (num_trig_packets <= MAX_PACKET_INFO) ? num_trig_packets : MAX_PACKET_INFO;
	for(int i = 0; i < num_packets; i++) {
	    packets[i] = packet_info[i];
	}
    }

    void print(int out_packet_id, int verbose, double tshift=0.0) {
	char s[10000];
	//   1398412 Trigger:  MCCM  HNZ  BK  00 38.1448 -122.8802 2012/11/07 00:05:55.158
	snprintf(s, sizeof(s), "%s %d", o.toString(false).c_str(), out_packet_id);
	if(verbose >= 2) {
	    for(int i = 0; i < num_packets; i++) {
		double t = packets[i].start_time;
		int len = strlen(s);
		INT_TIME it = nepoch_to_int(packets[i].start_time-tshift);
		snprintf(s+len, sizeof(s)-len, "\nP:%4d %6.1f %s %8.4f %8.4f %8.4f %8.4f %8.4f",
		    packets[i].length, packets[i].samplerate,
		    time_to_str(it, MONTHS_FMT),
		    packets[i].outof_ring - t,
		    packets[i].outof_feeder_queue - t,
		    packets[i].trigger_found - t,
		    packets[i].into_send_queue - t,
		    packets[i].outof_send_queue - t);
	    }
	}
	PrintLock::lock();
	cout << s << endl;
	PrintLock::unlock();
    }
};
*/

class EWP2Sender : public ExceptionListener, public DefaultTransportListener
{
  private:

    TimeStamp gmpeak_queue_start;
    TimeStamp trigparam_queue_start;
    pthread_mutex_t trigparams_lock;
    pthread_mutex_t gmpeaks_lock;
    pthread_mutex_t triggers_lock;
    pthread_mutex_t trig_wav_lock;
    pthread_mutex_t channel_list_lock;
    pthread_mutex_t text_msg_lock;
    pthread_t send_thread;
    pthread_t monitor_thread;
    TimeStamp last_queue_warn;
    TimeStamp gmpeak_send;
    string trigger_uri;
    string username;
    string passwd;
    char *serial_buffer;
    int serial_buffer_len;
    char *trig_serial;
    int trig_serial_len;
    time_t last_send_time;
    double max_send_wait;
    double no_data_wait;
    bool restart;
    string email_recipient;
    string mailx;
    static long log_msg_id;

    HBProducer *hb;
    Connection *connection;
    Session *session;
    MessageProducer *trigger_producer;
    MessageProducer *trigparams_producer;
    MessageProducer *gmpeak_producer;
    MessageProducer *log_producer;
    BytesMessage *message;

    Destination *trigger_destination;
    Destination *trigparams_destination;
    Destination *gmpeak_destination;
    Destination *log_destination;

    // properties
    static bool have_properties;
    static bool send_peaks;
    static PacketSource packet_src;
    static string trigger_topic;
    static string trigparams_topic;
    static string gmpeak_topic;
    static string log_topic;
    static string heartbeat_topic;
    static string heartbeat_sender;
    static Duration max_queue_time;
    static bool use_data_time;
    static bool log_latency;
    static double replay_tshift;
    static long gmpeak_msg_id;
    static long trigger_msg_id;
    static long trigparams_msg_id;
    static int verbose;

    bool stop_signal;
    int shutdown;
    bool connected_to_amq;
    bool transport_ok;
    TimeStamp data_time;

    deque<string> text_messages;
    deque<string> text_messages_send;
    deque<GMPeak> gmpeaks;
    deque<GMPeak> gmpeaks_send;
    deque<TriggerParams> trigparams;
    deque<TriggerParams> trigparams_send;
    deque<EWTrigger> triggers;
    deque<EWTrigger> triggers_send;
    map<Channel, int> channel_list;
    map<Channel, int> channel_list_send;

    static int packet_id;
    static TimeStamp zero_timestamp;
    static Duration gmpeak_wait;
    static Duration trig_param_wait;

    void allocateSerialBuffer();
    void connectToAmq() throw(CMSException);
    void closeConnection();
    void sendGMPeaks();
    void sendTrigParams();
    void sendTriggers();
    void sendEventWaveMessages();
    bool sendPacket(MessageProducer *producer, const char *buf, int len, string type="", long id=0);
    void sendChannelList();
    void sendEmail(int);
    void sendTextMessages();
    static string channelHeader();

    void queueWarn(TimeStamp ts) {
	if(ts - last_queue_warn > Duration(60.)) {
	    PrintLock::lock();
	    cout << "Warning: Removing old queued packets that have not been sent." << endl;
	    if(!connected_to_amq) {
		cout << "Warning: Not connected to ActiveMQ server." << endl;
	    }
	    PrintLock::unlock();
	    last_queue_warn = ts;
	}
    }

    virtual void onException(const CMSException& ex AMQCPP_UNUSED) {
	LOG_FATAL << "CMS Exception occurred. Shutting down ElarmSWP2.";
    LOG_FATAL << ex.getStackTraceString();
	exit(1);
    }
    virtual void transportInterrupted() {
	transport_ok = false;
	LOG_INFO << "The ElarmSWP2 transport has been Interrupted.";
    }
    virtual void transportResumed() {
	transport_ok = true;
	LOG_INFO << "The ElarmSWP2 transport has been Restored.";
    }
  
  public:

    EWP2Sender(string amq_uri, string amq_user, string amq_password) :
	trigger_uri(amq_uri), username(amq_user), passwd(amq_password), serial_buffer(NULL),
	serial_buffer_len(0), trig_serial(NULL), trig_serial_len(0), last_send_time(0),
	max_send_wait(60.), no_data_wait(60.), restart(false), connection(NULL), session(NULL),
	trigger_producer(NULL), trigparams_producer(NULL), gmpeak_producer(NULL), log_producer(NULL),
	message(NULL), trigger_destination(NULL), trigparams_destination(NULL), gmpeak_destination(NULL),
	log_destination(NULL), stop_signal(false), shutdown(0),
	connected_to_amq(false), transport_ok(true)
    {
	(void)BINARY; (void)MEMORY; (void)ASCII; (void)FILENAME; (void)DATABASE; // avoid compiler used warning
	init();
    }

    void init();

    ~EWP2Sender();

    static void *sendData(void *client_data);
    static void *sendMonitor(void *client_data);

    void start();

    void stop() {
	int rtn;
	void *status;
	shutdown = 0;
	stop_signal = true;
	PrintLock::lock();
	cout << "Waiting for Sender thread to join..." << endl;
	PrintLock::unlock();
	rtn = pthread_join(send_thread, &status);
	PrintLock::lock();
	cout << "Done. Return value: " << rtn << endl;
	cout << "Waiting for Monitor thread to join...";
	PrintLock::unlock();
	rtn = pthread_join(monitor_thread, &status);
	PrintLock::lock();
	cout << "Done. Return value: " << rtn << endl;
	PrintLock::unlock();
    }

    void send(GMPeak *peak);
    void send(TriggerParams *tp);
    void send(EWTrigger *t);
    void sendEventData(Channel ch, TimeStamp &start, TimeStamp end, ProcessBuffer &buffer, bool send_r=false);
    void sendTextMsg(string msg);

    TimeStamp currentTime() {
	if( !use_data_time ) {
	    return TimeStamp::current_time();
	}
	else {
	    return data_time;
	}
    }
    double currentUnixTime() {
	if( !use_data_time ) {
	    return TimeStamp::current_time().ts_as_double(UNIX_TIME);
	}
	else {
	    return data_time.ts_as_double(UNIX_TIME);
	}
    }
    void channelListChanged(map<Channel, int> &channel_list);
    void print(TriggerParams &t, int out_packet_id, int verbose);
    bool connectedToAmq() { return connected_to_amq; }
    static string timeString();
};

#endif
