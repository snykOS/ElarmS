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
#ifndef _E2AMQReader_h_
#define	_E2AMQReader_h_

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <cms/ExceptionListener.h>
#include "Exceptions.h"
#include <vector>
#include <set>
#include <pthread.h>
#include <list>
#include <map>

#include "E2Reader.h"
#include "DMMessageSender.h"
#include "TriggerParams.h"
#include "E2XMLParser.h"

class E2Event;
class E2TriggerManager;

class RecentSamples
{
  public:
    RecentSamples() {
	z_recent_sample = -1;
	n_recent_sample = -1;
	e_recent_sample = -1;
    }
    RecentSamples(TriggerParams &tp) {
	z_recent_sample = tp.z_recent_sample;
	n_recent_sample = tp.n_recent_sample;;
	e_recent_sample = tp.e_recent_sample;;
    }
    int z_recent_sample;
    int n_recent_sample;
    int e_recent_sample;
};

class GMPeakStats
{
  public:
    int num;
    double latency;
    double mean;
    double variance;
    double maximum;
    double adj_mean;
    double max_packet_secs;
    GMPeakStats() : num(0), latency(0.), mean(0.), variance(0.), maximum(0.), adj_mean(0.), max_packet_secs(0.) {}
};

class Sensor
{
  public:
    std::string net;
    std::string sta;
    std::string zchan;
    std::string loc;
    double lat;
    double lon;
    double last_data_time;
    bool use;
    double e2latency;
    std::map<std::string, GMPeakStats> wplatency;

    Sensor() : lat(0.), lon(0.), last_data_time(0.), use(true), e2latency(0.) {}
    Sensor(std::string network, std::string station, std::string zchannel, std::string location, double latitude, double longitude,
                double data_time, double e2late) :
        net(network), sta(station), zchan(zchannel), loc(location), lat(latitude), lon(longitude),
        last_data_time(data_time), use(true), e2latency(e2late) {}

    static std::set<std::string> packet_sources;
};

class GMPeakStation
{
  public:
    std::string net;
    std::string sta;
    double lat;
    double lon;
    GMPeakStation(std::string Net, std::string Sta, double Lat, double Lon) {
	net = Net;
	sta = Sta;
	lat = Lat;
	lon = Lon;
    }
    GMPeakStation() {
	lat = lon = 0.;
    }
};

class E2AMQReader : public E2Reader, public cms::ExceptionListener,
			public activemq::transport::DefaultTransportListener,
			public cms::MessageListener
{
 private:
    pthread_mutex_t data_time_lock;
    pthread_mutex_t buffer_lock;
    pthread_cond_t buffer_has_trigs;
    string amq_user;
    string amq_password;
    string trigger_uri;
    string trigger_topic;
    string log_uri;
    string log_topic;
    string station_hb_topic;
    string heartbeat_sender;
    double current_data_time;
    bool transport_ok;
    bool replay_mode;
    bool send_log;
    bool startup;
    time_t first_gmpeak_arrived;
    int station_hb_interval;
    long last_hb_send;
    long last_version_warning;
    double trig_buffer_seconds;
    bool start_at_begin;
    std::list<TriggerParams>::iterator beg_it;

    cms::Session *session;
    cms::MessageConsumer *consumer;
    cms::Connection *log_connection;
    cms::Topic *destination;
    activemq::core::ActiveMQConnection *amqConnection;
    static DMMessageSender *station_hb_sender;
    static DMMessageSender *log_sender;

    std::map<std::string, Sensor> sensors;
    std::list<TriggerParams> trig_buffer;
    std::map<string, GMPeakStation> gmpeak_sta_buffer;
    std::list<string> trig_log_buf;

    void processMessage(char *msg, int nbytes);
    void sendStationHB();
    void sendLogHeaders();
    int readNormal(int last_evid);
    int readReplay(int last_evid);

    E2XMLParser xml_parser;

    static long station_msg_id;
    static std::map<time_t, std::map<string, RecentSamples> > trig_time_map;

 public:
    E2AMQReader(string user, string password, string topic, string broker_uri,
		bool replay_mode, E2TriggerManager *tm) throw(Error);

    virtual ~E2AMQReader() throw();

    void runReader();
    void stopReader();
    int read(int last_evid);

    double currentTime();
    void stop();
    void processGMPeak(char *msg, int nbytes);
    void processTrigPacket(char *msg, int nbytes);

    virtual void onMessage(const cms::Message *message);

    virtual void onException(const cms::CMSException &ex AMQCPP_UNUSED) {
	LOG_FATAL << "CMS Exception occurred. Shutting down E2.";
    LOG_FATAL << ex.getStackTraceString();
	exit(1);
    }
    virtual void transportInterrupted() {
	transport_ok = false;
	LOG_INFO << "The E2 transport has been Interrupted.";
    }
    virtual void transportResumed() {
	transport_ok = true;
	LOG_INFO << "The E2 transport has been Restored.";
    }
    activemq::core::ActiveMQConnection *getConnection() { return amqConnection; }

    static bool newTrigParam(TriggerParams &tp);

    static std::list<Teleseism *> teleseisms;
};

#endif
