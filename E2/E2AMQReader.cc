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
/**
 * \author <henson@seismo.berkeley.edu>
 */
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <iomanip>
#include <unistd.h>
#include <plog/Log.h>
#include "GMPeak.h"
#include "TriggerParams.h"
#include "EWTrigger.h"
#include "E2AMQReader.h"
#include "E2ModuleManager.h"
#include "E2TriggerManager.h"
#include "E2Event.h"
#include "TimeStamp.h"
#include "TimeString.h"
#include "LogHeaders.h"
#include "E2Prop.h"

#include <decaf/util/concurrent/CountDownLatch.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <cms/ExceptionListener.h>
#include <cms/CMSException.h>
#include <cms/TextMessage.h>

using namespace activemq::core;
using namespace xercesc;

static int verbose = 0;
static int shut_down = false;
long E2AMQReader::station_msg_id = 1;
map<time_t, map<string, RecentSamples> > E2AMQReader::trig_time_map;
DMMessageSender *E2AMQReader::station_hb_sender = NULL;
DMMessageSender *E2AMQReader::log_sender = NULL;
set<string> Sensor::packet_sources;
list<Teleseism *> E2AMQReader::teleseisms;

extern string program_instance;

E2AMQReader::E2AMQReader(string user, string password, string topic, string broker_uri, bool replayMode,
		E2TriggerManager *tm) throw(Error) : E2Reader(tm), amq_user(user), amq_password(password),
		trigger_uri(broker_uri), trigger_topic(topic), transport_ok(true), replay_mode(replayMode),
		send_log(false), startup(true), first_gmpeak_arrived(0), station_hb_interval(60), last_hb_send(0),
		last_version_warning(0), trig_buffer_seconds(0.1), start_at_begin(true), session(NULL),
		consumer(NULL), log_connection(NULL), destination(NULL), amqConnection(NULL)
{
    char name[100];
    string host;

    string send_log_key = E2Prop::getString("SendLogKey");
    if(!strcasecmp(send_log_key.c_str(), "true")) {
        send_log = true;
    }
    else if(!strcasecmp(send_log_key.c_str(), "false")) {
        send_log = false;
    }
    else if(send_log_key.length() > 0 && gethostname(name, (int)sizeof(name)-1) == 0) {
        // if eew-bk-prod1.geo.berkeley.edu, shorten to eew-bk-prod1
        char *c = strstr(name, ".");
        if(c != NULL) *c = '\0';
        host = string(name);
        if(send_log_key.length() > 0) {
            int n = 0;
            for(int i = 0; i < (int)send_log_key.length(); i++) {
                if(host.find(send_log_key.at(i)) != string::npos) n++;
            }
            send_log = (n == (int)send_log_key.length());
        }
    }

    log_uri = E2Prop::getString("LogURI");
    log_topic = E2Prop::getString("LogTopic");
    station_hb_interval = E2Prop::getInt("StationHBInterval");
    station_hb_topic = E2Prop::getString("StationHBTopic");
    heartbeat_sender = E2Prop::getString("HeartbeatSender");
    verbose = E2Prop::getInt("Verbose");

    E2ModuleManager::param_str << fixed << setprecision(2);
    E2ModuleManager::param_str << "P: E2AMQReader.SendLog: " << send_log_key
		<< "(" << send_log << ")" << endl;
    E2ModuleManager::param_str << "P: E2AMQReader.LogURI: " << log_uri << endl;
    E2ModuleManager::param_str << "P: E2AMQReader.LogTopic: " << log_topic << endl;
    E2ModuleManager::param_str << "P: E2AMQReader.StationHBInterval: " << station_hb_interval << endl;
    if(station_hb_interval <= 0) {
	E2ModuleManager::param_str << "P: E2AMQReader: invalid StationHBInteval";
	LOG_INFO << E2ModuleManager::param_str.str();
	E2ModuleManager::param_str << endl;
	E2ModuleManager::sendLogMsg(E2Event::logTime());
	E2ModuleManager::sendLogMsg(E2ModuleManager::param_str.str());
	throw(Error("E2AMQReader: invalid StationHBInteval"));
    }
    if(replay_mode) {
	E2ModuleManager::param_str << "P: E2AMQReader: **** Running in REPLAY MODE ****" << endl;
	try {
	    trig_buffer_seconds = E2Prop::getDouble("TrigBufferSeconds");
	}
	catch(Error e) {} // optional
	E2ModuleManager::param_str << "P: E2AMQReader.TrigBufferSeconds: " << trig_buffer_seconds << endl;
	start_at_begin = true;
    }

    time_t now = time(NULL);
    last_hb_send = (long)(now/station_hb_interval) * station_hb_interval;

    pthread_mutex_init(&data_time_lock, NULL);
    pthread_mutex_init(&buffer_lock, NULL);
    pthread_cond_init(&buffer_has_trigs, NULL);

    current_data_time = 0;

    try {
	// amq_uri = failover:(tcp://localhost:61616)
	// Create a ConnectionFactory
	auto_ptr<cms::ConnectionFactory> connectionFactory(
		cms::ConnectionFactory::createCMSConnectionFactory( trigger_uri ) );

	// Create a Connection with optional user authentication
	cms::Connection *connection;
	if(amq_user.empty() && amq_password.empty()) {
	    connection = connectionFactory->createConnection();
	}
	else {
	    connection = connectionFactory->createConnection(amq_user, amq_password);
	}

	amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
	if( amqConnection != NULL ) {
	      amqConnection->addTransportListener( this );
        }

	amqConnection->setExceptionListener(this);

	// Create a Session
	session = amqConnection->createSession( cms::Session::AUTO_ACKNOWLEDGE );

	// Create the destination Topic
	destination = session->createTopic( trigger_topic );

	log_sender = new DMMessageSender(log_uri, amq_user, amq_password, log_topic);
	log_sender->setSource(heartbeat_sender);
	log_sender->run();
	log_connection = log_sender->getConnection();
	station_hb_sender = new DMMessageSender(log_connection, station_hb_topic);
	station_hb_sender->run();

	// Create a MessageConsumer from the Session to the Topic or Queue
	// Don't start consuming until after DMMessageSender objects are created.
	consumer = session->createConsumer( destination );
	consumer->setMessageListener(this);

    } catch( cms::CMSException& e ) {
    LOG_FATAL << e.getStackTraceString();
	throw(Error("ActiveMQ init failed."));
    }

    stringstream msg_str;
    msg_str << "M: Reading data from activemq at: " << trigger_uri << endl;
    msg_str << "M: user: " << amq_user << "  topic: " << trigger_topic;
    LOG_INFO << msg_str.str();
    msg_str << endl;
    log_sender->sendString(E2Event::logTime());
    log_sender->sendString(msg_str.str());
}

void E2AMQReader::runReader()
{
    amqConnection->start();
}

void E2AMQReader::stopReader()
{
    try {
	amqConnection->close();
    }
    catch( cms::CMSException &e ) {
    }
}

E2AMQReader::~E2AMQReader() throw()
{
    amqConnection->stop();
    if(log_connection != NULL && log_connection != amqConnection) {
	log_connection->stop();
    }
}

void E2AMQReader::onMessage(const cms::Message *message)
{
    if( !message ) return;

    const cms::TextMessage* text_message;
    const cms::BytesMessage* bytes_message;

    if((bytes_message = dynamic_cast< const cms::BytesMessage* >( message )) != NULL) {
	char *msg = (char *)bytes_message->getBodyBytes();
	if(msg) {
	    int nbytes = bytes_message->getBodyLength();
	    processMessage(msg, nbytes);
	    delete[] msg;
	}
    }
    else if((text_message = dynamic_cast< const cms::TextMessage* >( message )) != NULL) {
	string s = text_message->getText();
	if(s.substr(0,100).find("<teleseism_message") != string::npos) {
	    Teleseism *t;
	    if((t = xml_parser.processMessage(s)) != NULL) {
		string str = t->toString();

		pthread_mutex_lock(&tm->teleseisms_lock);
		teleseisms.push_back(t);
		pthread_mutex_unlock(&tm->teleseisms_lock);

		LOG_INFO << str;

		if(log_sender && !str.empty()) {
		    log_sender->sendString(E2Event::logTime() + "\n" + str);
		}
	    }
	}
    }
}

void E2AMQReader::processMessage(char *msg, int nbytes)
{
    EWPacketType type;

    type = getPacketType(msg, nbytes);

    if(type == GMPEAK)
    {
	processGMPeak(msg, nbytes);
    } 
    else if(type == TRIGPACKET)
    {
	processTrigPacket(msg, nbytes);
    }
    else if(type == TRIGGER)
    {
    } 
    else if(type == BADVERSION) {
	time_t now = time(NULL);
	if(now - last_version_warning > 600) {
	    LOG_INFO << "W: Bad message version";
	    log_sender->sendString("W: Bad message version");
	    last_version_warning = now;
	}
    }
    else 
    {
/*
	LOG_INFO << "Unknown packet type: "<< type;
*/
    }
}

bool E2AMQReader::newTrigParam(TriggerParams &tp)
{
    map<time_t, map<string, RecentSamples> >::iterator it;
    map<string, RecentSamples>::iterator jt;
    bool new_tp = true;
    time_t tv_sec = tp.trigger_time.tv_sec;
    int n = (int)strlen(tp.chan);
    if(n > 1) n--;
    string chan2 = string(tp.chan, n);

    string scnl = string(tp.sta) + "." + chan2 + "." + string(tp.net) + "." + string(tp.loc);

    if((it = trig_time_map.find(tv_sec)) != trig_time_map.end()) {
	if((jt = (*it).second.find(scnl)) != (*it).second.end()) {
	    if((*jt).second.z_recent_sample >= tp.z_recent_sample &&
	       (*jt).second.n_recent_sample >= tp.n_recent_sample &&
	       (*jt).second.e_recent_sample >= tp.e_recent_sample)
	    {
		new_tp = false;
	    }
	    else {
		(*it).second[scnl] = RecentSamples(tp);
	    }
	}
	else {
	    (*it).second[scnl] = RecentSamples(tp);
	}
    }
    else {
	map<string, RecentSamples> s;
	s[scnl] = RecentSamples(tp);
	trig_time_map[tv_sec] = s;
    }
    // remove old elements
    if(trig_time_map.size() > 1) {
	map<time_t, map<string, RecentSamples> >::reverse_iterator ri = trig_time_map.rbegin();
	time_t last = (*ri).first;
	for(it = trig_time_map.begin(); it != trig_time_map.end(); it++) {
	    if(last - (*it).first <= 60) break; // keep all update for last 60 seconds 
	}
	if(it != trig_time_map.end()) {
	    trig_time_map.erase(trig_time_map.begin(), it);
	}
    }
    return new_tp;
}

int E2AMQReader::read(int last_evid)
{
    if(!replay_mode) {
	return readNormal(last_evid);
    }
    else {
	return readReplay(last_evid);
    }
}

int E2AMQReader::readNormal(int last_evid)
{
    int num = 0;
    list<TriggerParams>::iterator it;

    pthread_mutex_lock(&buffer_lock);

    for(map<string, GMPeakStation>::iterator jt = gmpeak_sta_buffer.begin(); jt != gmpeak_sta_buffer.end(); jt++) {
	tm->addStation(jt->second.net, jt->second.sta, jt->second.lat, jt->second.lon);
    }
    gmpeak_sta_buffer.clear();

    if(last_evid > 0) { // still work to do elsewhere. don't block here
	time_t old_time = currentTime() - tm->unassoc_trigger_timeout;
	for(it = trig_buffer.begin(); it != trig_buffer.end(); it++) {
	    if((*it).getTime() > old_time) {
		tm->insertTrigger(*it);
		num++;
	    }
	}
    }
    else { // can wait for next message if buffer is empty
	if(trig_buffer.size() > 0) {
	    time_t old_time = currentTime() - tm->unassoc_trigger_timeout;
	    for(it = trig_buffer.begin(); it != trig_buffer.end(); it++) {
		if((*it).getTime() > old_time) {
		    tm->insertTrigger(*it);
		    num++;
		}
	    }
	}
	else { // wait until the buffer has some triggers
	    struct timespec ts;
	    clock_gettime(CLOCK_REALTIME, &ts);
	    ts.tv_sec += 1;	// maximum wait is 1 second
	    pthread_cond_timedwait(&buffer_has_trigs, &buffer_lock, &ts);

	    time_t old_time = currentTime() - tm->unassoc_trigger_timeout;
	    for(it = trig_buffer.begin(); it != trig_buffer.end(); it++) {
		if((*it).getTime() > old_time) {
		    tm->insertTrigger(*it);
		    num++;
		}
	    }
	}
    }
    trig_buffer.clear();
    pthread_mutex_unlock(&buffer_lock);
    return num;
}

int E2AMQReader::readReplay(int last_evid)
{
    int num = 0;
    double start_time;
    list<TriggerParams>::iterator it;

    pthread_mutex_lock(&buffer_lock);

    for(map<string, GMPeakStation>::iterator jt = gmpeak_sta_buffer.begin(); jt != gmpeak_sta_buffer.end(); jt++) {
	tm->addStation(jt->second.net, jt->second.sta, jt->second.lat, jt->second.lon);
    }
    gmpeak_sta_buffer.clear();

    if(last_evid > 0) { // still work to do elsewhere. don't block here
	it = start_at_begin ? trig_buffer.begin() : beg_it;
	for(; it != trig_buffer.end(); it++) {
	    if(num == 0) {
		start_time = (*it).getTime();
	    }
	    else if((*it).getTime() - start_time > trig_buffer_seconds) {
		beg_it = it;
		start_at_begin = false;
		pthread_mutex_unlock(&buffer_lock);
		return num;
	    }
	    tm->insertTrigger(*it);
	    num++;
	}
    }
    else { // can wait for next message if buffer is empty
	if(trig_buffer.size() > 0) {
	    it = start_at_begin ? trig_buffer.begin() : beg_it;
	    for(; it != trig_buffer.end(); it++) {
		if(num == 0) {
		    start_time = (*it).getTime();
		}
		else if((*it).getTime() - start_time > trig_buffer_seconds) {
		    beg_it = it;
		    start_at_begin = false;
		    pthread_mutex_unlock(&buffer_lock);
		    return num;
		}
		tm->insertTrigger(*it);
		num++;
	    }
	}
	else { // wait until the buffer has some triggers
	    struct timespec ts;
	    clock_gettime(CLOCK_REALTIME, &ts);
	    ts.tv_sec += 1;	// maximum wait is 1 second
	    pthread_cond_timedwait(&buffer_has_trigs, &buffer_lock, &ts);

	    it = start_at_begin ? trig_buffer.begin() : beg_it;
	    for(; it != trig_buffer.end(); it++) {
		if(num == 0) {
		    start_time = (*it).getTime();
		}
		else if((*it).getTime() - start_time > trig_buffer_seconds) {
		    beg_it = it;
		    start_at_begin = false;
		    pthread_mutex_unlock(&buffer_lock);
		    return num;
		}
		tm->insertTrigger(*it);
		num++;
	    }
	    if(replay_mode && num == 0 && shut_down) num = -1;
	}
    }
    trig_buffer.clear();
    start_at_begin = true;
    pthread_mutex_unlock(&buffer_lock);
    return num;
}

void E2AMQReader::processGMPeak(char *msg, int nbytes)
{
    double gmpeak_time, lat, lon;
    string net, sta, chan, loc, netsta, packet_src;
    map<int, Station>::iterator it;
    timeval current_time;
    gettimeofday(&current_time, NULL);
    double now = current_time.tv_sec + 1.e-06*current_time.tv_usec;

    if(replay_mode) {
	pthread_mutex_lock(&buffer_lock);
	pthread_mutex_unlock(&buffer_lock);
    }

    GMPeakPacket *packet = new GMPeakPacket(msg, nbytes);
    packet_src = string(packet->ps.src);

    for(int i = 0; i < packet->numPeaks(); i++) {
	gmpeak_time = packet->peaks[i].tend();
	pthread_mutex_lock(&data_time_lock);
	if(current_data_time < gmpeak_time) {
	    current_data_time = gmpeak_time;
	}
	pthread_mutex_unlock(&data_time_lock);

	// lock trigger buffer since E2AMQReader::read (from another thread) can call
	// tm->insertTrigger(*it), which can add a new station and change tm->stations map.
	pthread_mutex_lock(&buffer_lock);

	net.assign(packet->peaks[i].net);
	sta.assign(packet->peaks[i].sta);
	chan.assign(packet->peaks[i].chan);
	loc.assign(packet->peaks[i].loc);
	netsta = string(net) + "." + string(sta);
	lat = packet->peaks[i].lat;
	lon = packet->peaks[i].lon;

	int sta_id = E2TriggerManager::staId(netsta);

	if((it = tm->stationsFind(sta_id)) != tm->stationsEnd()) {
	    if((*it).second.last_data_time < gmpeak_time) {
		pthread_mutex_lock(&tm->stations_lock);
		(*it).second.last_data_time = gmpeak_time;
		(*it).second.use = true;
		pthread_mutex_unlock(&tm->stations_lock);
	    }
	}
	else {
	    map<string, GMPeakStation>::iterator jt = gmpeak_sta_buffer.find(netsta);
	    if(jt == gmpeak_sta_buffer.end()) {
		gmpeak_sta_buffer[netsta] = GMPeakStation(net, sta, lat, lon);
	    }
	    pthread_cond_signal(&buffer_has_trigs);
	}
	pthread_mutex_unlock(&buffer_lock);

	if(chan.length() > 0 && (chan[chan.length()-1] == 'Z' || chan[chan.length()-1] == 'z')) {
	    Sensor::packet_sources.insert(packet_src);
	    string key = net + "." + sta + "." + chan;
	    map<string, Sensor>::iterator jt = sensors.find(key);
	    if(jt == sensors.end()) {
		Sensor s = Sensor(net, sta, chan, loc, lat, lon, gmpeak_time, now - packet->peaks[i].tend());
		GMPeakStats stats;
		stats.num = 1;
		stats.mean = packet->peaks[i].latency;
		stats.variance = 0.;
		stats.maximum = packet->peaks[i].latency;
		if(packet->peaks[i].samprate != 0.) {
		    stats.max_packet_secs = packet->peaks[i].nsamps/packet->peaks[i].samprate;
		    stats.adj_mean = packet->peaks[i].latency + 0.5*stats.max_packet_secs;
		}
		s.wplatency[packet_src] = stats;
		sensors[key] = s;
	    }
	    else if(!replay_mode) { // don't compute latency statistics in replay_mode
		(*jt).second.last_data_time = gmpeak_time;
		(*jt).second.use = true;

		// compute the maximum e2latency since the last call to sendStationHB
		double latency = now - packet->peaks[i].tend();
		if(latency > (*jt).second.e2latency) (*jt).second.e2latency = latency;

		// compute the mean, variance and maximum of the wplatencies since the last call to sendStationHB
		// also compute the mean, variance and maximum of the packet-size adjusted wplatencies
		// and the maximum packet-size
		map<string, GMPeakStats>::iterator kt = (*jt).second.wplatency.find(packet_src);
		if(kt != (*jt).second.wplatency.end()) {
		    (*kt).second.num++;
		    double x = packet->peaks[i].latency;
		    double m = (*kt).second.mean;
		    double v = (*kt).second.variance;
		    (*kt).second.mean +=  (x - m)/(*kt).second.num;
		    (*kt).second.variance += m*m - (*kt).second.mean*(*kt).second.mean + (x*x - v - m*m)/(*kt).second.num;
		    if(packet->peaks[i].latency > (*kt).second.maximum) (*kt).second.maximum = packet->peaks[i].latency;
		    if(packet->peaks[i].samprate != 0.) {
			double packet_secs = packet->peaks[i].nsamps/packet->peaks[i].samprate;
			if(packet_secs > (*kt).second.max_packet_secs) (*kt).second.max_packet_secs = packet_secs;
			// compute latency adjusted for packet size
			x = packet->peaks[i].latency + 0.5*packet_secs;
			m = (*kt).second.adj_mean;
			(*kt).second.adj_mean += (x - m)/(*kt).second.num;
		    }
		}
		else {
		    GMPeakStats stats;
		    stats.num = 1;
		    stats.mean = packet->peaks[i].latency;
		    stats.variance = 0.;
		    stats.maximum = packet->peaks[i].latency;
		    if(packet->peaks[i].samprate != 0.) {
			stats.max_packet_secs = packet->peaks[i].nsamps/packet->peaks[i].samprate;
			stats.adj_mean = packet->peaks[i].latency + 0.5*stats.max_packet_secs;
		    }
		    (*jt).second.wplatency[packet_src] = stats;
		}
	    }
	}
    }
    delete packet;

    sendLogHeaders();

    if(now - last_hb_send >= station_hb_interval) {
	sendStationHB();
	last_hb_send += station_hb_interval;
    }
}

void E2AMQReader::processTrigPacket(char *msg, int nbytes)
{
    if(startup && !replay_mode) {
	// wait 5 seconds after the first gmpeak before processing triggers
	if(time(NULL) - first_gmpeak_arrived < 5) return;
	startup = false;
    }

    TriggerParamsBundle packet(msg, nbytes);
    stringstream msg_str;
//    packet.print();

    pthread_mutex_lock(&buffer_lock);

    pthread_mutex_lock(&data_time_lock);

    for(int i = 0; i < packet.numParams(); i++)
    {
	if(verbose >= 3) {
	    LOG_INFO << packet.trig_params[i].toString(true);
	}
	if(replay_mode && i == packet.numParams()-1 && packet.trig_params[i].sta[0] == '\0') {
	    LOG_INFO << "E2AMQReader: received null trigger from EWP2. Shutdown.";
	    shut_down = true;
	}
	// check for repeated trigger. E2 will work without this check, but it saves
	// processing time and log lines.
	else if(newTrigParam(packet.trig_params[i])) {
	    trig_buffer.push_back(packet.trig_params[i]);

	    if(packet.trig_params[i].trigger_found > 0 &&
		current_data_time < packet.trig_params[i].trigger_time.tv_sec) {
		current_data_time = packet.trig_params[i].trigger_time.tv_sec;
	    }
	}
    }
    pthread_mutex_unlock(&data_time_lock);

    pthread_cond_signal(&buffer_has_trigs);
    pthread_mutex_unlock(&buffer_lock);
    if(log_sender && !msg_str.str().empty()) {
	log_sender->sendString(E2Event::logTime() + "\n" + msg_str.str());
    }
}

void E2AMQReader::sendLogHeaders()
{
    static int doy = -1;
    if(verbose >= 2) { // Print log header description once per day
        if(TimeStamp::current_time().day_of_year() != doy) {
            doy = TimeStamp::current_time().day_of_year();
            LOG_INFO << log_headers;
	    log_sender->sendString(log_headers);
        }
    }
}

void E2AMQReader::sendStationHB()
{
    char line[1000];
    string s;
    stringstream msg, log_msg;

    log_msg << "S:H:          E2_instance              start_time duration num_packet_sources" << endl;
    msg << "#         E2_instance              start_time duration num_packet_sources" << endl;

    s = TimeString::toString(last_hb_send, 3);

    snprintf(line, sizeof(line), "%20s %s %8d         %10d", program_instance.c_str(), s.c_str(),
		station_hb_interval, (int)Sensor::packet_sources.size());

    log_msg << "S:T: "<< line << endl;
    log_msg << "S:H net   sta chan loc      lat       lon U      last_received_time mean,std,max,pacdur,adjmean for ";

    msg << line << endl;
    msg << "#net   sta chan loc      lat       lon  U      last_received_time mean,std,max,pacdur,adjmean for ";

    for(set<string>::iterator it = Sensor::packet_sources.begin(); it != Sensor::packet_sources.end(); it++) {
	msg << " " + (*it);
	log_msg << " " + (*it);
    }
    msg << endl;
    log_msg << endl;

    for(map<string, Sensor>::iterator it = sensors.begin(); it != sensors.end(); it++) {
        s = TimeString::toString((*it).second.last_data_time);

	if(it != sensors.begin()) {
	    msg << endl;
	    log_msg << endl;
	}

	snprintf(line, sizeof(line), " %3s %5s %4s %3s %8.4f %9.4f T %s",
		(*it).second.net.c_str(), (*it).second.sta.c_str(), (*it).second.zchan.c_str(),
		(*it).second.loc.c_str(), (*it).second.lat, (*it).second.lon, s.c_str());
	msg << string(line);
	log_msg << "S: " << string(line);
	(*it).second.e2latency = -1.; // reset to compute the maximum for the next interval
	
	for(set<string>::iterator jt = Sensor::packet_sources.begin(); jt != Sensor::packet_sources.end(); jt++) {
	    map<string, GMPeakStats>::iterator kt = (*it).second.wplatency.find(*jt);
	    if(kt != (*it).second.wplatency.end() && (*kt).second.num > 0) {
		snprintf(line, sizeof(line), " %8.2f %8.2f %8.2f %8.2f %8.2f", (*kt).second.mean, sqrt((*kt).second.variance),
			(*kt).second.maximum, (*kt).second.max_packet_secs, (*kt).second.adj_mean);
		// reset to the compute num, mean, variance and maximum for the next interval
		(*kt).second.num = 0;
		(*kt).second.mean = 0.;
		(*kt).second.variance = 0.;
		(*kt).second.maximum = -1.;
		(*kt).second.max_packet_secs = -1.;
		(*kt).second.adj_mean = 0.;
	    }
	    else { // no GMPeaks have been received for this station and this EWP2 source
		snprintf(line, sizeof(line), "       NA       NA       NA       NA       NA");
	    }
	    msg << string(line);
	    log_msg << string(line);
	}
    }

    station_hb_sender->sendString(msg.str(), string("station_hb_") + program_instance, station_msg_id);
    if(++station_msg_id < 1) station_msg_id = 1;

    if(send_log) log_sender->sendString(log_msg.str());

    if(verbose >= 2) {
	LOG_INFO << log_msg.str();
    }
}

double E2AMQReader::currentTime()
{
    if(!replay_mode) {
	return TimeStamp::current_time().ts_as_double(UNIX_TIME);
    }
    else {
	double current_time;
	pthread_mutex_lock(&data_time_lock);
	current_time = current_data_time;
	pthread_mutex_unlock(&data_time_lock);
	return current_time;
    }
}
