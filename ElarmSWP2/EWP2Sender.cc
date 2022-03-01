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
#include "EWP2Sender.h"
#include "ElarmSWP2.h"
#include "PrintLock.h"
#include "unistd.h"
#include <strings.h>
#include <iomanip>
#include <cms/TextMessage.h>
#include "TimeString.h"
#include "DMLib.h"
#include "GetProp.h"

using namespace cms;

int EWP2Sender::packet_id = 1;
TimeStamp EWP2Sender::zero_timestamp = TimeStamp(1970,01,01,0,0,0,0);

bool EWP2Sender::have_properties = false;
bool EWP2Sender::send_peaks = true;
PacketSource EWP2Sender::packet_src = {""};
string EWP2Sender::trigger_topic = "";
string EWP2Sender::trigparams_topic = "";
string EWP2Sender::gmpeak_topic = "";
string EWP2Sender::log_topic = "";
string EWP2Sender::heartbeat_topic = "";
string EWP2Sender::heartbeat_sender = "";
Duration EWP2Sender::max_queue_time = Duration(0.);
bool EWP2Sender::use_data_time = false;
bool EWP2Sender::log_latency = false;
Duration EWP2Sender::gmpeak_wait     = Duration(0.05);
Duration EWP2Sender::trig_param_wait = Duration(0.05);
long EWP2Sender::gmpeak_msg_id = 1;
long EWP2Sender::trigger_msg_id = 1;
long EWP2Sender::trigparams_msg_id = 1;
long EWP2Sender::log_msg_id = 1;
int EWP2Sender::verbose = 1;

extern "C" {
static void * send_data(void *client_data);
static void * monitor(void *client_data);
}

void EWP2Sender::init()
{
    pthread_mutex_init(&trigparams_lock, NULL);
    pthread_mutex_init(&gmpeaks_lock, NULL);
    pthread_mutex_init(&triggers_lock, NULL);
    pthread_mutex_init(&trig_wav_lock, NULL);
    pthread_mutex_init(&channel_list_lock, NULL);
    pthread_mutex_init(&text_msg_lock, NULL);

    PrintLock::lock(); // lock to get properties and to print
    if(!have_properties) {
	have_properties = true;
	GetProp *prop = GetProp::getInstance();
	string pac_src;
	double wait_send_gmpeaks, wait_send_trigparams;
	send_peaks = prop->getBool("SendPeaks");
	heartbeat_sender = prop->getString("HeartbeatSender");
	pac_src = prop->getString("PacketSrc");
	memset(packet_src.src, 0, sizeof(packet_src.src));
	strncpy(packet_src.src, pac_src.c_str(), sizeof(packet_src.src)-1);

	trigger_uri = prop->getString("TriggerURI");
	trigger_topic = prop->getString("TriggerTopic");
	trigparams_topic = prop->getString("TrigParamsTopic");
	gmpeak_topic = prop->getString("GMPeakTopic");
	log_topic = prop->getString("LogTopic");
	heartbeat_topic = prop->getString("HeartbeatTopic");
//	use_data_time = prop->getBool("UseDataTime");
	log_latency = prop->getBool("LogLatency");
	restart = prop->getBool("Restart");
	email_recipient = prop->getString("EmailRecipient");
	if(!email_recipient.empty()) {
	    mailx = prop->getString("Mailx");
	}
	wait_send_gmpeaks = prop->getDouble("WaitSendGMPeaks");
	wait_send_trigparams = prop->getDouble("WaitSendTrigParams");
	verbose = prop->getInt("Verbose");
	gmpeak_wait     = Duration(wait_send_gmpeaks);
	trig_param_wait = Duration(wait_send_trigparams);

	double d = 180.; // 3 minutes
	d = prop->getDouble("MaxQueueTime"); // 3 minutes
	max_queue_time = Duration(d);
	stringstream msg_str;
	msg_str << fixed << setprecision(2);
	msg_str << "P: EWP2Sender.SendPeaks: " << send_peaks << endl;
	msg_str << "P: EWP2Sender.HeartbeatSender: " << heartbeat_sender << endl;
	msg_str << "P: EWP2Sender.PacketSrc: " << pac_src << endl;
	msg_str << "P: EWP2Sender.TriggerURI: " << trigger_uri << endl;
	msg_str << "P: EWP2Sender.TriggerTopic: " << trigger_topic << endl;
	msg_str << "P: EWP2Sender.TrigParamsTopic: " << trigparams_topic << endl;
	msg_str << "P: EWP2Sender.GMPeakTopic: " << gmpeak_topic << endl;
	msg_str << "P: EWP2Sender.LogTopic: " << log_topic << endl;
	msg_str << "P: EWP2Sender.HeartbeatTopic: " << heartbeat_topic << endl;
	msg_str << "P: EWP2Sender.MaxQueueTime: " << d << endl;
	msg_str << "P: EWP2Sender.UseDataTime: " << use_data_time << endl;
	msg_str << "P: EWP2Sender.LogLatency: " << log_latency << endl;
	msg_str << "P: EWP2Sender.Restart: " << restart << endl;
	msg_str << "P: EWP2Sender.EmailRecipient: " << email_recipient << endl;
	if(!email_recipient.empty()) {
	    msg_str << "P: EWP2Sender.Mailx: " << mailx << endl;
	}
	msg_str << "P: EWP2Sender.WaitSendGMPeaks: " << wait_send_gmpeaks << endl;
	msg_str << "P: EWP2Sender.WaitSendTrigParams: " << wait_send_trigparams << endl;
	msg_str << "P: EWP2Sender.Verbose: " << verbose;
	LOG_INFO << msg_str.str();
	ElarmSWP2::param_str << msg_str.str() << endl;
    }
    PrintLock::unlock();

    trig_serial_len = EWTrigger::packetSize();
    trig_serial = new char[trig_serial_len];

    last_queue_warn = zero_timestamp;
    data_time = zero_timestamp;
    last_send_time = 0;
    gmpeak_send = zero_timestamp;
    connected_to_amq = false;
}

void EWP2Sender::connectToAmq() throw(CMSException)
{
    LOG_INFO << "TriggerURI=" << trigger_uri;

    // Create a ConnectionFactory.
    auto_ptr<ConnectionFactory> connectionFactory(ConnectionFactory::createCMSConnectionFactory( trigger_uri ) );

    // Create a Connection with optional user authentication, and start the connection.
    if(username.empty() && passwd.empty()) {
	connection = connectionFactory->createConnection();
    }
    else {
	LOG_INFO << "user=" << username;
	connection = connectionFactory->createConnection(username, passwd);
    }
    if(connection == NULL) throw(CMSException("NULL Connection"));

    ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
    if( amqConnection != NULL ) {
	amqConnection->addTransportListener( this );
    }
    else {
	throw(CMSException("NULL ActiveMQConnection"));
    }

    connection->start();
    connection->setExceptionListener(this);

    // Create a Session.
    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
    if(session == NULL) throw(CMSException("NULL Session"));

    trigger_destination = session->createTopic( trigger_topic );
    if(trigger_destination == NULL) throw(CMSException("NULL trigger_destination"));
    trigger_producer = session->createProducer( trigger_destination );
    if(trigger_producer == NULL) throw(CMSException("NULL trigger_producer"));
    trigger_producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );

    if(trigparams_topic == trigger_topic) {
	trigparams_producer = trigger_producer;
    }
    else {
	trigparams_destination = session->createTopic( trigparams_topic );
	trigparams_producer = session->createProducer( trigparams_destination );
	if(trigparams_producer == NULL) throw(CMSException("NULL trigparams_producer"));
	trigparams_producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );
    }
    if(gmpeak_topic == trigger_topic) {
	gmpeak_producer = trigger_producer;
    }
    else if(gmpeak_topic == trigparams_topic) {
	gmpeak_producer = trigparams_producer;
    }
    else {
	gmpeak_destination = session->createTopic( gmpeak_topic );
	gmpeak_producer = session->createProducer( gmpeak_destination );
	if(gmpeak_producer == NULL) throw(CMSException("NULL gmpeak_producer"));
	gmpeak_producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );
    }
    log_destination = session->createTopic( log_topic );
    log_producer = session->createProducer( log_destination );
    if(log_producer == NULL) throw(CMSException("NULL log_producer"));
    log_producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );

    message = session->createBytesMessage();

    // send heartbeats
    hb = new HBProducer(connection, heartbeat_sender, heartbeat_topic);

    connected_to_amq = true;
}

EWP2Sender::~EWP2Sender()
{
    free(serial_buffer);
    delete [] trig_serial;
}

void EWP2Sender::closeConnection()
{
    hb->stop();
    try {
	delete trigger_producer;
	delete trigger_destination;
	if(trigparams_producer != trigger_producer) {
	    delete trigparams_producer;
	    delete trigparams_destination;
	}
	if(gmpeak_producer != trigger_producer && gmpeak_producer != trigparams_producer) {
	    delete gmpeak_producer;
	    delete gmpeak_destination;
	}
	delete log_destination;
	delete log_producer;
	delete session;
	delete message;

	delete hb;
	delete connection;
    } catch ( CMSException& e ) { 
        LOG_ERROR << e.getStackTraceString();
    }
}

void EWP2Sender::start()
{
    pthread_create(&monitor_thread, NULL, monitor, (void *)this);
    pthread_create(&send_thread, NULL, send_data, (void *)this);
}

static void *
send_data(void *client_data)
{
    return EWP2Sender::sendData(client_data);
}
static void *
monitor(void *client_data)
{
    return EWP2Sender::sendMonitor(client_data);
}

void *EWP2Sender::sendData(void *client_data)
{
    EWP2Sender *s = (EWP2Sender *)client_data;
    int n;

    s->allocateSerialBuffer();

    do {
	if( !s->connected_to_amq ) {
	    try {
		s->connectToAmq();
	    }
	    catch (CMSException &e) {
        LOG_FATAL << e.getStackTraceString();
		sleep(1);
		if(s->stop_signal) return NULL;
		continue;
	    }
	}
	// send trigparams when first one has been held for > trig_param_wait seconds,
	// or when the number of trigparams in the queue is >= MAX_TRIGPARAM_PER_PKT,
	// or when shutdown > 0
	pthread_mutex_lock(&s->trigparams_lock);
	n = (int)s->trigparams.size();
	if(n >= MAX_TRIGPARAM_PER_PKT || (n > 0 && s->currentTime()
		- s->trigparam_queue_start > trig_param_wait) || s->shutdown)
	{
	    while(!s->trigparams.empty()) {
		s->trigparams_send.push_back(s->trigparams.front());
		s->trigparams.pop_front();
	    }
	    if(s->shutdown) { // send a null TriggerParams
		TriggerParams tp;
		tp.in_send_queue = s->currentTime();
		s->trigparams_send.push_back(tp);
	    }
	}
	pthread_mutex_unlock(&s->trigparams_lock);

	s->sendTrigParams();

	// send gmpeaks when the first one has been held for > gmpeak_wait seconds,
	// or when the number of gmpeaks in the queue is >= MAX_GMPEAK_PER_PKT,
	// or when shutdown > 0
	pthread_mutex_lock(&s->gmpeaks_lock);
	n = (int)s->gmpeaks.size();
	if(n >= MAX_GMPEAK_PER_PKT || (n > 0 && s->currentTime()
		- s->gmpeak_queue_start > gmpeak_wait) || s->shutdown)
	{
	    while(!s->gmpeaks.empty()) {
		s->gmpeaks_send.push_back(s->gmpeaks.front());
		s->gmpeaks.pop_front();
	    }
	}
	pthread_mutex_unlock(&s->gmpeaks_lock);

	s->sendGMPeaks();
	
	// send triggers immediately
	pthread_mutex_lock(&s->triggers_lock);
	while(!s->triggers.empty()) {
	    s->triggers_send.push_back(s->triggers.front());
	    s->triggers.pop_front();
	}
	pthread_mutex_unlock(&s->triggers_lock);

	s->sendTriggers();

	// send channel list message
	pthread_mutex_lock(&s->channel_list_lock);
	for(map<Channel, int>::iterator it = s->channel_list.begin(); it != s->channel_list.end(); it++) {
	    s->channel_list_send[(*it).first] = (*it).second;
	}
	s->channel_list.clear();
	pthread_mutex_unlock(&s->channel_list_lock);

	s->sendChannelList();

	// send text messages
	pthread_mutex_lock(&s->text_msg_lock);
	while(!s->text_messages.empty()) {
	    s->text_messages_send.push_back(s->text_messages.front());
	    s->text_messages.pop_front();
	}
	pthread_mutex_unlock(&s->text_msg_lock);

	s->sendTextMessages();

	if(s->stop_signal) {
	    LOG_INFO << "Sender thread stop_signal. shutdown = " << s->shutdown;
	    s->shutdown++; // loop once more to empty all buffers
	}
	else {
//	    usleep(10000); // 0.01 second
	    usleep(100); // 0.0001 second
	}
    } while(s->shutdown < 2);

    s->closeConnection();

    return NULL;
}

void *EWP2Sender::sendMonitor(void *client_data)
{
    EWP2Sender *s = (EWP2Sender *)client_data;
    struct timespec wait_until;
    TimeStamp last_gmpeak_send;
    bool sent_data_warning = false;

    if(s->max_send_wait <= 0.) {
	LOG_INFO << "Warning: invalid max_send_wait: " << s->max_send_wait << endl;
	LOG_INFO << "Sends will not be monitored.";
	return NULL;
    }
    LOG_INFO << "running sendMonitor";

    while( !s->stop_signal ) {
	// Wait for the sendLock. It can be locked by sendPacket() or HBProducer.
	timeval current_time;
	gettimeofday(&current_time, NULL);
	wait_until.tv_sec = current_time.tv_sec + (time_t)s->max_send_wait;
	wait_until.tv_nsec = 0;
	if(pthread_mutex_timedlock(DMLib::sendLock(), &wait_until) == ETIMEDOUT) {
	    // A send() is taking too long and could be hung. Could close connection
	    // and reconnect, but the close might also hang. Just exit the program
	    // for a script restart.
	    LOG_INFO << "Send has taken more than " << s->max_send_wait << " seconds";
	    s->sendEmail(0);
	    if(s->restart) {
		LOG_INFO << "Exiting process for restart.";
		exit(1);
	    }
	}
	else if( !s->use_data_time ) {
	    pthread_mutex_unlock(DMLib::sendLock());  

	    pthread_mutex_lock(&s->gmpeaks_lock);
	    last_gmpeak_send = s->gmpeak_send;
	    pthread_mutex_unlock(&s->gmpeaks_lock);

	    if(last_gmpeak_send > zero_timestamp) {
		if(s->currentTime() - last_gmpeak_send > Duration(s->no_data_wait)) {
		    if(!sent_data_warning) {
			s->sendEmail(1);
			sent_data_warning = true;
		    }
		}
		else if(sent_data_warning) {
		    sent_data_warning = false;
		    s->sendEmail(3);
		}
	    }
	}
	sleep(1);
    }

    return NULL;
}

void EWP2Sender::sendEmail(int type)
{
    stringstream body;
    string subject;

    if(email_recipient.empty()) return;

    if(type == 0) {
	subject = "\"ElarmSWP2 send-warning\" ";
        body << "The Send Message function has taken more than " << max_send_wait << " seconds." << endl;
    }
    else if(type == 1) {
	subject = "\"ElarmSWP2 no-data-warning\" ";
        body << "ElarmSWP2 has not received any data for " << no_data_wait << " seconds." << endl;
    }
    else {
	subject = "\"ElarmSWP2 data-resumed\" ";
        body << "ElarmSWP2 is receiving data again.";
    }
    body << "\nmax_send_wait:   " << (int)max_send_wait << " secs";
    body << "\nno_data_wait:    " << (int)no_data_wait << " secs";
    body << "\nPacketSource:    " << packet_src.src;
    body << "\nTriggerURI:      " << trigger_uri;
    body << "\nTriggerTopic:    " << trigger_topic;
    body << "\nTrigParamsTopic: " << trigparams_topic;
    body << "\nGMPeakTopic:     " << gmpeak_topic;
    string cmd = string(mailx + " -s ") + subject + email_recipient + " <<EOM\n" + body.str() + "\nEOM\n";
    system(cmd.c_str());
}

void EWP2Sender::allocateSerialBuffer()
{
    if(serial_buffer == NULL) {
	serial_buffer_len = 10000;
	serial_buffer = (char *)malloc(serial_buffer_len);
    }
}

void EWP2Sender::sendGMPeaks()
{
    GMPeakPacket packet;

    if(!send_peaks) {
	gmpeaks_send.clear();
	return;
    }
    int len;

    while(gmpeaks_send.size() >= MAX_GMPEAK_PER_PKT) {
	for(int i = 0; i < MAX_GMPEAK_PER_PKT; i++) {
	    packet.add(gmpeaks_send[i]);
	}
	int size = packet.packetSize(MAX_GMPEAK_PER_PKT);
	if(size > serial_buffer_len) {
	    serial_buffer_len = size;
	    serial_buffer = (char *)realloc(serial_buffer, serial_buffer_len);
	}
	len = packet.serialize(packet_src, packet_id, serial_buffer, serial_buffer_len);
	if( !sendPacket(gmpeak_producer, serial_buffer, len, "gmpeak_"+string(packet_src.src), gmpeak_msg_id) ) {
	    return;
	}
	packet_id++;
	if(++gmpeak_msg_id < 1) gmpeak_msg_id = 1;
	packet.clear();
	for(int i = 0; i < MAX_GMPEAK_PER_PKT && !gmpeaks_send.empty(); i++) {
	    gmpeaks_send.pop_front();
	}
    }
    for(int i = 0; i < (int)gmpeaks_send.size(); i++) {
	packet.add(gmpeaks_send[i]);
    }
    if(packet.numPeaks() > 0) {
	int size = packet.packetSize(packet.numPeaks());
	if(size > serial_buffer_len) {
	    serial_buffer_len = size;
	    serial_buffer = (char *)realloc(serial_buffer, serial_buffer_len);
	}
	len = packet.serialize(packet_src, packet_id, serial_buffer, serial_buffer_len);
	if( sendPacket(gmpeak_producer, serial_buffer, len, "gmpeak_"+string(packet_src.src), gmpeak_msg_id) ) {
	    packet_id++;
	    if(++gmpeak_msg_id < 1) gmpeak_msg_id = 1;
	    gmpeaks_send.clear();
	}
    }
}

void EWP2Sender::sendTrigParams()
{
    TriggerParamsBundle packet;
    int len, size;
    double now = currentTime().ts_as_double(UNIX_TIME);

    while(trigparams_send.size() >= MAX_TRIGPARAM_PER_PKT) {
	size = TriggerParamsBundle::headerLength();
	for(int i = 0; i < MAX_TRIGPARAM_PER_PKT; i++) {
	    trigparams_send[i].outof_send_queue = now;
	    packet.add(trigparams_send[i]);
	    size += trigparams_send[i].packetSize();
	    if(verbose > 1) {
		LOG_INFO << trigparams_send[i].toString(true);
	    }
	}
	if(size > serial_buffer_len) {
	    serial_buffer_len = size;
	    serial_buffer = (char *)realloc(serial_buffer, serial_buffer_len);
	}
	len = packet.serialize(packet_src, packet_id, serial_buffer, serial_buffer_len);
	if( !sendPacket(trigparams_producer, serial_buffer, len, "trigparams_"+string(packet_src.src), trigparams_msg_id) ) {
	    packet.clear();
	    return;
	}
	packet_id++;
	if(++trigparams_msg_id < 1) trigparams_msg_id = 1;
	for(int i = 0; i < MAX_TRIGPARAM_PER_PKT && !trigparams_send.empty(); i++) {
	    trigparams_send.front().outof_send_queue = now;
	    if(log_latency) print(trigparams_send.front(), packet_id-1, verbose);
	    trigparams_send.pop_front();
	}
	packet.clear();
    }

    size = TriggerParamsBundle::headerLength();
    for(int i = 0; i < (int)trigparams_send.size(); i++) {
	trigparams_send[i].outof_send_queue = now;
	packet.add(trigparams_send[i]);
	size += trigparams_send[i].packetSize();
	if(verbose > 1) {
	    LOG_INFO << trigparams_send[i].toString(true);
	}
    }
    if(packet.numParams() > 0) {
	if(size > serial_buffer_len) {
	    serial_buffer_len = size;
	    serial_buffer = (char *)realloc(serial_buffer, serial_buffer_len);
	}
	len = packet.serialize(packet_src, packet_id, serial_buffer, serial_buffer_len);
	if( sendPacket(trigparams_producer, serial_buffer, len, "trigparams_"+string(packet_src.src), trigparams_msg_id) ) {
	    packet_id++;
	    if(++trigparams_msg_id < 1) trigparams_msg_id = 1;
	    for(int i = 0; i < (int)trigparams_send.size(); i++) {
		if(log_latency) print(trigparams_send[i], packet_id-1, verbose);
	    }
	    trigparams_send.clear();
	}
	packet.clear();
    }
}

void EWP2Sender::sendTriggers()
{
    int len;

    while(!triggers_send.empty()) {
	EWTrigger t = triggers_send.front();
	len = t.serialize(packet_src, packet_id, serial_buffer, serial_buffer_len);
	if( !sendPacket(trigger_producer, serial_buffer, len, "trigger_"+string(packet_src.src), trigger_msg_id) ) break;
	triggers_send.pop_front();
	packet_id++;
	if(++trigger_msg_id < 1) trigger_msg_id = 1;
	if(verbose > 0) {
	    LOG_INFO << t.toString(0.); // log triggers
	}
    }
}

bool EWP2Sender::sendPacket(MessageProducer *producer, const char *buf, int len, string type, long id)
{
    if( !transport_ok ) return false;

    if(message) {
	bool success = false;
	pthread_mutex_lock(DMLib::sendLock());  
	try {
	    message->setBodyBytes((const unsigned char *)buf, len);
	    if(!type.empty()) {
		message->setStringProperty("type", type);
		message->setLongProperty("id", id);
	    }
	    producer->send( message );
	    success = true;
	    last_send_time = time(NULL);
	}
	catch( CMSException &e ) {
	    LOG_INFO << "W: EWP2Sender::sendPacket failed.";
        LOG_ERROR << e.getStackTraceString();
	}
	catch (...) {
	    LOG_INFO << "W: EWP2Sender::sendPacket failed with unknown exception.";
	}
	last_send_time = time(NULL);
	pthread_mutex_unlock(DMLib::sendLock());  
	return success;
    }
    return true;
}

void EWP2Sender::send(GMPeak *peak)
{
    pthread_mutex_lock(&gmpeaks_lock);
    gmpeak_send = currentTime();
    if(gmpeaks.empty()) {
	gmpeak_queue_start = gmpeak_send;
    }
    // remove old packets that haven't been sent.
    while(!gmpeaks.empty() && gmpeak_send - gmpeaks.front().in_send_queue > max_queue_time) {
	gmpeaks.pop_front();
	queueWarn(gmpeak_send);
    }
    TimeStamp ts = currentTime();
    peak->in_send_queue = ts;
    gmpeaks.push_back(*peak);
    pthread_mutex_unlock(&gmpeaks_lock);
}

void EWP2Sender::send(TriggerParams *tp)
{
    pthread_mutex_lock(&trigparams_lock);
    TimeStamp ts = currentTime();
    if(trigparams.empty()) {
	trigparam_queue_start = ts;
    }
    // remove old packets that haven't been sent.
    while(!trigparams.empty() && ts - trigparams.front().in_send_queue > max_queue_time) {
	trigparams.pop_front();
	queueWarn(ts);
    }
    tp->into_send_queue = ts.ts_as_double(UNIX_TIME);
    tp->in_send_queue = ts;
    trigparams.push_back(*tp);
    pthread_mutex_unlock(&trigparams_lock);
}

void EWP2Sender::send(EWTrigger *t)
{
    pthread_mutex_lock(&triggers_lock);
    TimeStamp ts = currentTime();
    // remove old packets that haven't been sent.
    while(!triggers.empty() && ts - triggers.front().in_send_queue > max_queue_time) {
	triggers.pop_front();
	queueWarn(ts);
    }
    t->in_send_queue = ts;
    triggers.push_back(*t);
    pthread_mutex_unlock(&triggers_lock);
}

void EWP2Sender::sendChannelList()
{
    char line[1024];
    stringstream msg_str;

    if(channel_list_send.empty()) return;

    msg_str << channelHeader() << endl;
    for(map<Channel, int>::iterator it = channel_list_send.begin(); it != channel_list_send.end(); it++) {
	snprintf(line, sizeof(line), "WPC: %2s %5s %2s %3s %9.4f %9.4f %7.1f %8.2f %13.6e %20s %d",
	    (*it).first.network, (*it).first.station, (*it).first.location, (*it).first.channel,
	    (*it).first.latitude, (*it).first.longitude, (*it).first.elevation, (*it).first.samprate,
	    (*it).first.gain, (*it).first.gain_units, (*it).second);
	msg_str << line << endl;
    }
    channel_list_send.clear();

    TextMessage *message = session->createTextMessage( msg_str.str() );
    message->setStringProperty("type", "EWP2_LOG");
    message->setStringProperty("source", heartbeat_sender);
    message->setLongProperty("id", log_msg_id++);
    log_producer->send( message );
    delete message;
    LOG_INFO << msg_str.str();
}

void EWP2Sender::channelListChanged(map<Channel, int> &channelList)
{
    pthread_mutex_lock(&channel_list_lock);
    channel_list.clear();
    for(map<Channel, int>::iterator it = channelList.begin(); it != channelList.end(); it++) {
	channel_list[(*it).first] = (*it).second;
    }
    pthread_mutex_unlock(&channel_list_lock);
}

string EWP2Sender::timeString()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return TimeString::toString(tv, 3);
}

string EWP2Sender::channelHeader()
{
    extern string program_version;
    return string("WPChannels: ") + timeString() + " " + program_version + " " + string(packet_src.src);
}

void EWP2Sender::sendTextMessages()
{
    while(!text_messages_send.empty()) {
	TextMessage *message = session->createTextMessage( text_messages_send.front() );
	message->setStringProperty("type", "EWP2_LOG");
	message->setStringProperty("source", heartbeat_sender);
	message->setLongProperty("id", log_msg_id++);
	log_producer->send( message );
	delete message;
	text_messages_send.pop_front();
    }
}

void EWP2Sender::sendTextMsg(string msg)
{
    pthread_mutex_lock(&text_msg_lock);
    text_messages.push_back(msg);
    pthread_mutex_unlock(&text_msg_lock);
}

void EWP2Sender::print(TriggerParams &t, int out_packet_id, int verbose)
{
    (void)verbose;
    char s[100];
    //   1398412 Trigger:  MCCM  HNZ  BK  00 38.1448 -122.8802 2012/11/07 00:05:55.158
    snprintf(s, sizeof(s), "%s %d", t.toString(false).c_str(), out_packet_id);
    LOG_INFO << string(s);
}
