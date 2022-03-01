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
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <map>

#include "EWP2Receiver.h"

#include "GMPeak.h"
#include "EWTrigger.h"
#include "TriggerParams.h"
#include "EWPacket.h"
#include "Duration.h"
#include "TimeStamp.h"
#include "TimeString.h"
#include "GetProp.h"

#include <decaf/util/concurrent/CountDownLatch.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <cms/ExceptionListener.h>

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;

const string program_version = "3.0.1-2017-05-05";

void usage() {
    cout << "EWP2Receiver: Version " << program_version << endl;
    cout << "usage: EWP2Receiver config_file" << endl;
}


int main(int argc, char *argv[]) 
{
    EWP2Receiver receiver;
    string params;
    GetProp *prop = GetProp::getInstance();

    for(int i = 1; i < argc; i++) {
	if(!strstr(argv[i], "=")) {
	    params.assign(argv[i]);
	    break;
	}
	else if(!strncmp(argv[i], "params=", 7)) {
	    params.assign(argv[i]+7);
	}
    }
    if(params.length() == 0) {
	usage();
	return 0;
    }
    prop->init(params, argc, argv);

    if( !receiver.run() ) return -1;
}

static bool stop_signal = false;
extern "C" {
    void handle_signal(int i) {
	(void)(i);
        stop_signal = true;
    }
}
static bool sort_files(string a, string b)
{
    return (strcmp(a.c_str(), b.c_str()) < 0);
}

static bool sort_latencies(Latency a, Latency b)
{
    int ret;
    if((ret = a.net.compare(b.net)) != 0) {
	return (ret < 0);
    }
    if((ret = a.sta.compare(b.sta)) != 0) {
	return (ret < 0);
    }
    return (a.trigger_time.tv_sec < b.trigger_time.tv_sec);
}

bool EWP2Receiver::run()
{
    GetProp *prop = GetProp::getInstance();
    string packet_src;
    double last_data_save = 0.;

    // required params
    in = prop->getString("in");
    topic = prop->getString("topic");
    user = prop->getString("user");
    passwd = prop->getString("password");
    fmt = prop->getString("fmt");

    // optional params
    try { out = prop->getString("out"); } catch(Error e) { }
    try { otopic = prop->getString("otopic"); } catch(Error e) { }
    try { ouser = prop->getString("ouser"); } catch(Error e) { }
    try { opasswd = prop->getString("opasswd"); } catch(Error e) { }
    try { ofmt = prop->getString("ofmt"); } catch(Error e) { }
    try { pfmt = prop->getString("pfmt"); } catch(Error e) { }
    try { sta = prop->getString("sta"); } catch(Error e) { }
    try { chan = prop->getString("chan"); } catch(Error e) { }
    try { toffset = prop->getString("toffset"); } catch(Error e) { }
    try { packet_src = prop->getString("PacketSrc"); } catch(Error e) { }
    try { msg_src = prop->getString("MsgSrc"); } catch(Error e) { }
    try { read_txt_msg = prop->getBool("TxtMsg"); } catch(Error e) { }

    char *s, *c, *tok, *last;
    s = strdup(packet_src.c_str());
    tok = s;
    while((c = strtok_r(tok, " ,\t", &last)) != NULL) {
	tok = NULL;
	packet_src_set.insert(string(c));
    }
    free(s);

    if(!toffset.empty()) {
	char *endptr;
	time_offset = strtod(toffset.c_str(), &endptr);
	if(*endptr != '\0') {
	    cerr << "Invalid argument toffset" << toffset << endl;
	    return false;
	}
    }

    char *o = (char *)out.c_str();
    out_sockfd = -1;
    if( (c = strstr(o, ":")) ) {
	string out_ip_addr;
	char *endptr;
	char *port_str = c+1;
	int out_port;

	out_ip_addr.assign(o, c-o);
	out_port = strtol(port_str, &endptr, 10);

	if(endptr - port_str != (int)strlen(port_str)) {
	    cerr << "invalid argument out=" << out << endl;
	    return false;
	}
	if( (out_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    cerr << "socket() failed." << endl << strerror(errno) << endl;
	    return false;
	}

	out_addr.sin_family = AF_INET;
	out_addr.sin_addr.s_addr = inet_addr(out_ip_addr.c_str());
	out_addr.sin_port = htons(out_port);
    }

    signal(SIGHUP,handle_signal);
    signal(SIGINT,handle_signal);
    signal(SIGQUIT,handle_signal);
/*
    signal(SIGILL,handle_signal);
    signal(SIGTRAP,handle_signal);
    signal(SIGFPE,handle_signal);
    signal(SIGKILL,handle_signal);
    signal(SIGBUS,handle_signal);
    signal(SIGSEGV,handle_signal);
    signal(SIGSYS,handle_signal);
    signal(SIGTERM,handle_signal);
*/

    saving_ewdata = false;
    stop_signal = false;
    last_time.tv_sec = 0;
    last_time.tv_usec = 0;
    arid = 1;
    time_offset = 0.;

    if(strstr(in.c_str(), ":")) {
	Connection *connection;
	Session *session;
	Destination *destination;
	MessageConsumer *consumer;

	activemq::library::ActiveMQCPP::initializeLibrary();

	try {
	    string broker;
	    stringstream uri;

	    uri << "failover:(tcp://" << in << ")?initialReconnectDelay=100";
	    broker = uri.str();
	    cout << "brokerURI=" << broker << endl;

	    // Create a ConnectionFactory
	    auto_ptr<ConnectionFactory> connectionFactory(
			ConnectionFactory::createCMSConnectionFactory( broker ) );

	    // Create a Connection with optional user authentication, and start the connection.
	    if (user.empty() && passwd.empty()) {
		connection = connectionFactory->createConnection();
	    }
	    else {
		cout << "user=" << user << endl;
	        connection = connectionFactory->createConnection(user, passwd);
	    }

	    ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
	    if( amqConnection != NULL ) {
		amqConnection->addTransportListener(this);
	    }

	    connection->start();
	    connection->setExceptionListener(this);

	    // Create a Session
	    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );

	    // Create the destination Topic
	    cout << "topic: " << topic << endl;
	    destination = session->createTopic( topic );

	    // Create a MessageConsumer from the Session to the Topic or Queue
	    consumer = session->createConsumer( destination );
	} catch( CMSException &e ) {
        LOG_FATAL << e.getStackTraceString();
	}
	while( !stop_signal ) {
	    Message *message = consumer->receive(1000);
	    if(message) {
		const BytesMessage* bytes_message;
		const TextMessage* text_message;
		if((bytes_message = dynamic_cast< const BytesMessage* >( message )) != NULL) {
		    char *msg = (char *)bytes_message->getBodyBytes();
		    double msg_latency = 0.;
		    try {
			double current_time = TimeStamp::current_time().ts_as_double(UNIX_TIME);
			long long t = message->getCMSTimestamp();
			double ts = (double)t / 1000.;
			msg_latency = current_time - ts;
		    }
		    catch(CMSException &e) {
            LOG_FATAL << e.getStackTraceString();
		    }
		    handleMessage(msg, bytes_message->getBodyLength(), msg_latency, pfmt, sta, chan);
		    delete[] msg;
		}
		else if((text_message = dynamic_cast< const TextMessage* >( message )) != NULL) {
		    string s = text_message->getText();
		    if(s.compare(0, 5, "<?xml") == 0 && s.find("<event_message ") != string::npos) {
			processEventMessage(s);
		    }
		    if(read_txt_msg && !s.empty()) {
			if(!msg_src.empty()) {
			    if(text_message->propertyExists("source")) {
				string src = text_message->getStringProperty("source");
				if(msg_src == src) {
				    cout << s;
				    if(s[s.length()-1] != '\n') cout << endl;
				}
			    }
			}
			else {
			    cout << s;
			    if(s[s.length()-1] != '\n') cout << endl;
			}
		    }
		}
	    }
	    delete message;

	    double now = TimeStamp::current_time().ts_as_double(UNIX_TIME);
	    if(saving_ewdata && now - last_data_save >= 60.)
	    {
		last_data_save = now;
		writeTriggers(out);
		writeEvents(out);
	    }
	}
	activemq::library::ActiveMQCPP::shutdownLibrary();

	if(pfmt.find('j') != string::npos) {
	    writeTriggers(out);
	    writeEvents(out);
	}
    }
    else {
	vector<string> files;
	DIR *dirp;

	if( (dirp = opendir(in.c_str())) ) {
	    struct dirent *dp;
	    DIR *subdir;
	    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) if(dp->d_name[0] != '.')
	    {
		string path = in + "/" + string(dp->d_name);
		if( (subdir = opendir(path.c_str())) ) {
		    closedir(subdir); // don't go into subdirectories.
		}
		else {
		    files.push_back(path);
		}
	    }
	    closedir(dirp);
	    // if filenames include date, like triggers_20111105.trig, then sort will work
	    sort(files.begin(), files.end(), sort_files);
	}
	else {
	    files.push_back(in);
	}

	char *msg;
	int msg_length;
	FILE *fpin;

	for(int i = 0; i < (int)files.size(); i++) {
	    cout << "Reading file: " << files[i] << endl;
	    if( (fpin = fopen(files[i].c_str(), "r")) == NULL) {
		cerr << "Cannot open " << files[i] << endl << strerror(errno) << endl;
		return false;
	    }
	    while( !stop_signal && (msg = readEWMessage(fpin, &msg_length)) != NULL ) {
		handleMessage(msg, msg_length, 0., pfmt, sta, chan);
		free(msg);
	    }
	    if(ferror(fpin)) {
		cerr << "read error file: " << files[i] << endl;
		if(errno) cerr << strerror(errno) << endl;
	    }
	    fclose(fpin);

	    if(pfmt.find('j') != string::npos) {
		writeTriggers(out);
		writeEvents(out);
	    }
	}
	if(pfmt.find('l') != string::npos) {
	    sort(latencies.begin(), latencies.end(), sort_latencies);

	    string last_sta = "";
	    double trig_time, last_time = 0; // filter out duplicate triggers
	    int n = (int)latencies.size();
	    for(int i = 0; i < n; i++) {
		trig_time = (double)latencies[i].trigger_time.tv_sec
				+ 1.0e-6 * latencies[i].trigger_time.tv_usec;
		if(last_sta.compare(latencies[i].sta) || fabs(trig_time - last_time) > 1.0) {
		    string s = TimeString::toString(trig_time, 3);
		    string ns = latencies[i].net + "." + latencies[i].sta;
		    printf("%10s %12.4f  %s\n", ns.c_str(), latencies[i].latency, s.c_str());
		}
		last_sta = latencies[i].sta;
		last_time = trig_time;
	    }
	}
    }

    return true;
}

void EWP2Receiver::writeTriggers(string out)
{
    FILE *fp=NULL;
    string access, file;
    string dir = "";
    TimeStamp now = TimeStamp::current_time();

    if( !out.empty() ) {
	file = out + ".arrival";
	access = "w";
    }
    else {
	char path[MAXPATHLEN+1];
	if(!dir.empty()) dir.append("/");
	snprintf(path, sizeof(path), "%s%4d%02d%02d%02d.arrival", dir.c_str(), now.year(),
		now.month_of_year(), now.day_of_month(), now.hour_of_day());
	file.assign(path);
	access = "a+";
    }
    if(trigger_map.size() == 0) {
	cout << "No TRIGGER packets converted for 'j' option." << endl;
	return;
    }
    else if((fp = fopen(file.c_str(), access.c_str())) == NULL) {
	cerr << "Cannot open file " << file << " " << strerror(errno) << endl;
	return;
    }

    map<string, Trigger *>::iterator it;

    for(it = trigger_map.begin(); it != trigger_map.end(); it++) {
	Trigger *t = (*it).second;
	writeArrival(fp, t->sta, t->chan, t->loc, t->net, t->src, t->t, arid++);
    }
    fclose(fp);
    trigger_map.clear();
}

void EWP2Receiver::writeEvents(string out)
{
    FILE *fp=NULL;
    string access, file;
    string dir = "";
    TimeStamp now = TimeStamp::current_time();

    if( !out.empty() ) {
	file = out + ".origin";
	access = "w";
    }
    else {
	char path[MAXPATHLEN+1];
	if(!dir.empty()) dir.append("/");
	snprintf(path, sizeof(path), "%s%4d%02d%02d%02d.origin", dir.c_str(), now.year(),
		now.month_of_year(), now.day_of_month(), now.hour_of_day());
	file.assign(path);
	access = "a+";
    }
    if(event_map.size() == 0) {
	cout << "No Event messages converted for 'j' option." << endl;
	return;
    }
    else if((fp = fopen(file.c_str(), access.c_str())) == NULL) {
	cerr << "Cannot open file " << file << " " << strerror(errno) << endl;
	return;
    }

    map<string, CoreEventInfo *>::iterator it;

    for(it = event_map.begin(); it != event_map.end(); it++) {
	CoreEventInfo *o = (*it).second;
	writeOrigin(fp, o->getID(), o->getLatitude(), o->getLongitude(), o->getDepth(), o->getOriginTime(),
		o->getMagnitude(), o->getProgramInstance());
	delete o;
    }
    fclose(fp);
    event_map.clear();
}

void EWP2Receiver::handleMessage(char *msg, int len, double msg_latency, string pfmt, string sta, string chan)
{
    EWPacketType type = getPacketType(msg, len);
    string str;

    // string pfmt can contain these one or more or the following chars
    // 'g': gmpeak packet single-line info
    // 'G': gmpeak packet single-line info and contents
    // 'p': Trigger Packet single-line info
    // 'P': Trigger Packet single-line info and contents
    // 't': Trigger message single-line info
    // 'T': Trigger message single-line info and contents
    // 'ucm': print only for specified sources ('u','c','m')

    /* types
    #define GMPEAK          'G'
    #define TRIGGER         'T'
    #define TRIGPARAMS      'P'
    #define TRIGPACKET      'K'
    #define UNKNOWN         'U'
    #define BADVERSION      'V'
    */
    if(type == GMPEAK) {
	if(pfmt.find('g') != string::npos || pfmt.find('G') != string::npos)
	{
	    GMPeakPacket packet(msg, len);
	    printf("    GMPeak %s %10d %10d  %8.6f\n", packet.ps.src, packet.id, len, msg_latency);
	    fflush(stdout);
	    if(pfmt.find('G') != string::npos) {
		cout << packet.toString() << endl;
	    }
	}
	else if(pfmt.find('j') != string::npos) {
//May2015	    joinGMPeak(packet);
	}
/*
	if(out_sockfd >= 0) {
	    if(sendto(out_sockfd, msg, len, 0, (struct sockaddr *)&out_addr,
			sizeof(struct sockaddr)) < 0)
	    {
		cerr << "EWP2Recevier: sendto failed." << endl << strerror(errno) << endl;
	    }
	}
	else {
	    cout << packet.toString() << endl;
	}
*/
    }
//    else if(type == TRIGPARAMS) {
    else if(type == TRIGPACKET) {
	if(pfmt.find('p') != string::npos && pfmt.find('e') != string::npos)
	{
	    TriggerParamsBundle packet(msg, len);
	    char c[100];
//	    long off, gap_period = 10*60; // 10 minutes
	    long off, gap_period = 2*60; // 2 minutes
	    int days, hours, minutes, seconds;

	    for(int i = 0; i < packet.numParams(); i++) {
		if(last_time.tv_sec > 0 && packet.trig_params[i].trigger_time.tv_sec - last_time.tv_sec > gap_period)
		{
		    string last = TimeString::toString(last_time, 3);
		    snprintf(c, sizeof(c), "off: %s", last.c_str());
		    cout << c << " to ";

		    string tp_time = TimeString::toString(packet.trig_params[i].trigger_time, 3);

		    off = (long)(packet.trig_params[i].trigger_time.tv_sec +
				1.0e-6 * packet.trig_params[i].trigger_time.tv_usec -
				(last_time.tv_sec + 1.0e-6 * last_time.tv_usec));
		    days = off/(24*3600);
		    hours = (off - 24*3600*days)/3600;
		    minutes = (off - 24*3600*days - 3600*hours)/60;
		    seconds =  off - 24*3600*days - 3600*hours - 60*minutes;
		    snprintf(c, sizeof(c), "%s  %02d %02d:%02d:%02d", tp_time.c_str(), days, hours, minutes, seconds);
		    cout << c << endl;
		}
		if(packet.trig_params[i].trigger_time.tv_sec + 1.0e-6 * packet.trig_params[i].trigger_time.tv_usec  >
			last_time.tv_sec + 1.0e-6 * last_time.tv_usec)
		{
		    last_time = packet.trig_params[i].trigger_time;
		}
	    }
	}
	else if(pfmt.find('p') != string::npos || pfmt.find('P') != string::npos)
	{
	    TriggerParamsBundle packet(msg, len);
	    if(pfmt.find('j') != string::npos) {
		char s[100];
		int msec;
		for(int i = 0; i < packet.numParams(); i++) {
		    if(packet.trig_params[i].chan[(int)strlen(packet.trig_params[i].chan)-1] == 'Z') {
			TimeStamp ts(UNIX_TIME, packet.trig_params[i].trigger_time);
			msec = (int)(packet.trig_params[i].trigger_time.tv_usec/1000. + .5);
			snprintf(s, sizeof(s), "%s.%s.%ld.%d", packet.trig_params[i].sta,
				packet.trig_params[i].chan, (long)packet.trig_params[i].trigger_time.tv_sec, msec);
			trigger_map[s] = new Trigger(packet.trig_params[i].sta, packet.trig_params[i].chan,
						packet.trig_params[i].loc, packet.trig_params[i].net,
						packet.ps.src, ts.ts_as_double(UNIX_TIME));
		    }
		}
	    }
	    else if(pfmt.find('l') != string::npos) { // latency
		for(int i = 0; i < packet.numParams(); i++) {
		    if((!strcasecmp(packet.trig_params[i].chan,"HNZ")
				|| !strcasecmp(packet.trig_params[i].chan,"HHZ"))) {
			Latency l;
			l.net.assign(packet.trig_params[i].net);
			l.sta.assign(packet.trig_params[i].sta);
			l.trigger_time = packet.trig_params[i].trigger_time;
			l.latency = packet.trig_params[i].trigger_found; // the detection time minus the trigger time
			latencies.push_back(l);
		    }
		}
	    }
	    else {
		if(sta.empty() && chan.empty()) {
		    string s;
		    if(packet.numParams() > 0) {
			s = TimeString::toString(packet.trig_params[0].trigger_time, 3);
		    }
		    printf("TrigParams %s %3d %s %10d %10d  %8.6f\n", packet.ps.src, packet.numParams(),
				s.c_str(), packet.id, len, msg_latency);
		    fflush(stdout);
		}
		if(pfmt.find("P1") != string::npos) {
		    str = packet.toString(sta, chan, true, true);
		    if(str.length() > 0) {
//			cout << packet.toString(sta, chan, true, true) << endl;
			cout << str << endl;
		    }
		}
		else if(pfmt.find("P2") != string::npos) {
		    str = packet.toString(sta, chan, false, false);
		    if(str.length() > 0) {
			cout << str << endl;
		    }
		}
		else if(pfmt.find('P') != string::npos) {
		    str = packet.toString(sta, chan);
		    if(str.length() > 0) {
//			cout << packet.toString(sta, chan) << endl;
			cout << str << endl;
		    }
		}
	    }
	}
/*
	if(out_sockfd >= 0) {
	    if(sendto(out_sockfd, msg, len, 0, (struct sockaddr *)&out_addr,
			sizeof(struct sockaddr)) < 0)
	    {
		cerr << "EWP2Receiver: sendto failed." << endl << strerror(errno) << endl;
	    }
	}
*/
    }
    else if(type == TRIGGER) {
	if(pfmt.find('t') != string::npos ||
	   pfmt.find('T') != string::npos)
	{
	    EWTrigger packet(msg, len);
if(pfmt.find('T') == string::npos) {
	    printf("   Trigger %s %10d %10d  %8.6f\n", packet.ps.src, packet.id,
			len, msg_latency);
	    fflush(stdout);
}
	    if(pfmt.find('T') != string::npos) {
		cout << packet.toString() << endl;
	    }
	}
    }
    else if(type == BADVERSION) {
	cerr << "Packet has incompatible version number." << endl;
    }
    else {
	cerr << "Unknown packet type." << endl;
    }
}

void EWP2Receiver::joinGMPeak(GMPeakPacket &packet)
{
    map<string, PeakData *>::iterator it;
    char s[200];
    string chan;

    for(int i = 0; i < packet.numPeaks(); i++) {
	snprintf(s, sizeof(s), "%s.%s.%s.%s.d", packet.peaks[i].sta,
		packet.peaks[i].chan, packet.peaks[i].net, packet.peaks[i].loc);
	chan.assign(s);
	if((it = peak_data_map.find(chan)) == peak_data_map.end()) {
	    PeakData *pk = new PeakData(packet.peaks[i]);
/*
	    pk->npts = 0;
	    pk->last_sec = packet.peaks[i].time.tv_sec;
	    pk->start_index.push_back(0);
	    pk->start_time.push_back(pk->last_sec);
*/
	    peak_data_map[chan] = pk;
	    if((it = peak_data_map.find(chan)) == peak_data_map.end()) {
		cout << "peak_data_map error." << endl;
		exit(1);
	    }
	}
	if(packet.peaks[i].tbeg > (*it).second->last_sec)
	{
	    int i1 = (int)(packet.peaks[i].tbeg + .5);
	    int i2 = (int)((*it).second->last_sec + .5);

	    if((*it).second->npts > 0 && i1 - i2 > 1) {
		(*it).second->start_index.push_back((*it).second->npts);
		(*it).second->start_time.push_back(packet.peaks[i].tbeg);
	    }

	    if((*it).second->size_data <= (*it).second->npts) {
		(*it).second->size_data += 1000;
		(*it).second->data = (float *)realloc((*it).second->data, (*it).second->size_data*sizeof(float));
	    }
	    (*it).second->data[(*it).second->npts++] = packet.peaks[i].dmax;
	    (*it).second->last_sec = packet.peaks[i].tbeg;
	}
    }
}

void EWP2Receiver::processEventMessage(string msg)
{
    char event[200];
    string src;

    CoreEventInfo* cei = NULL;
    if((cei = decoder.decodeMessage(msg, cei)) != NULL) {
	if(cei->getVersion() == 0) {
	    snprintf(event, sizeof(event), "%9.4f,%9.4f,%9.4f,%17.5f,%7.2f", cei->getLatitude(), cei->getLongitude(),
			cei->getDepth(), cei->getOriginTime(), cei->getMagnitude());
	    map<string, CoreEventInfo *>::iterator it;
	    if((it = event_map.find(event)) == event_map.end()) {
		event_map[event] = cei;
	    }
	    else {
		delete cei;
	    }
	}
	else {
	    delete cei;
	}
    }
}

void EWP2Receiver::writeOrigin(FILE *fp, string id, double latitude, double longitude, double odepth, double otime, double mag, string instance)
{
    double lat=latitude, lon=longitude, depth=odepth, time=otime, depdp= -999., mb=mag, ms= -1., ml= -1.;
    long jdate= -1, nass= -1, ndef= -1, ndp= -1, grn= -1, srn = -1, mbid = -1, msid = -1, mlid = -1, commid = -1;
    string orid=id, evid=id;
    const char *etype = "-", *dtype = "-", *algorithm = "-", *lddate = "-";
    const char *auth = (!instance.empty()) ? instance.c_str() : "-";

    fprintf(fp, "%9.4f %9.4f %9.4f %17.5f %8s %8s %8ld %4ld %4ld %4ld %8ld %8ld %-7.7s %9.4f %1s %7.2f %8ld %7.2f %8ld %7.2f %8ld %-15.15s %-15.15s %8ld %-17.17s\n",
		lat, lon, depth, time, orid.c_str(), evid.c_str(), jdate, nass, ndef, ndp, grn, srn, etype, depdp, dtype,
		mb, mbid, ms, msid, ml, mlid, algorithm, auth, commid, lddate);
}

void EWP2Receiver::writeArrival(FILE *fp, string sta, string chan, string loc, string net, string src, double time, int arid)
{
    long jdate = -1, stassid = -1, chanid = -1, commid = -1;
    const char *iphase = "P", *stype = "-", *clip = "-", *fm = "-", *qual = "-";
    const char *lddate = (!net.empty()) ? net.c_str() : "-";
    const char *auth = (!src.empty()) ? src.c_str() : "-";
    double deltim = -1., azimuth = -1., delaz = -1., slow = -1., delslo = -1., ema = -1., rect = -1.;
    double amp = -1., per = -1., logat = -1., snr = -1.;
    string channel = (loc.empty() || loc == "--") ? chan : loc + string(".") + chan;
    
// sta.c_str() time arid jdate stassid chanid   chan iphase stype deltim azimuth delaz  slow delslo  ema   rect    amp  per logat, clip, fm, snr, qual, auth, commid, lddate);
//   "%-6.6s %17.5f  %8d   %8d     %8d    %8d %-8.8s %-8.8s   %1s  %6.3f   %7.2f %7.2f %7.2f  %7.2f %7.2f %7.3f %10.3e %7.2f %7.2f %1s %2s %10.3e %1s %-15.15s %8d %-17.17s\n",
    fprintf(fp, "%-6.6s %17.5f %8d %8ld %8ld %8ld %-8.8s %-8.8s %1s %6.3f %7.2f %7.2f %7.2f %7.2f %7.2f %7.3f %10.3e %7.2f %7.2f %1s %2s %10.3e %1s %-15.15s %8ld %-17.17s\n",
	sta.c_str(), time, arid, jdate, stassid, chanid, channel.c_str(), iphase, stype, deltim, azimuth, delaz, slow, delslo, ema,
	rect, amp, per, logat, clip, fm, snr, qual, auth, commid, lddate);
}

void EWP2Receiver::flipBytes(float *data, int npts)
{
    for(int i = 0; i < npts; i++) {
	wordorder_master(data[i]);
    }
}
void EWP2Receiver::flipIBytes(int *data, int npts)
{
    for(int i = 0; i < npts; i++) {
	wordorder_master(data[i]);
    }
}
void EWP2Receiver::onException(const cms::CMSException &ex AMQCPP_UNUSED) {
    LOG_FATAL << "CMS Exception.";
    LOG_FATAL << ex.getStackTraceString();
    exit(1);
}
void EWP2Receiver::transportInterrupted() {
    transport_ok = false;
    cout << "Transport has been Interrupted." << endl;
}
void EWP2Receiver::transportResumed() {
    transport_ok = true;
    cout << "Transport has been Restored." << endl;
}

PeakData::PeakData(GMPeak &p)
{
    sta.assign(p.sta);
    chan.assign(p.chan);
    npts = 0;
    last_sec = p.tbeg;
    start_index.push_back(0);
    start_time.push_back(p.tbeg);
    size_data = 4000;
    data = (float *)malloc(size_data*sizeof(float));
}
