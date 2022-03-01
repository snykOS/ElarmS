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
#include "qlib2.h"
#include "DMMessageSender.h"
#include "plog/Log.h"                           // plog logging library

#include <unistd.h>
#include <decaf/lang/Integer.h>
#include <decaf/util/Date.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

using namespace cms;
using namespace activemq;
using namespace activemq::core;

long DMMessageSender::alert_msg_id = 1;
long DMMessageSender::delete_msg_id = 1;
long DMMessageSender::dmlib_msg_id = 1;

pthread_mutex_t DMLib::send_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t * DMLib::print_lock = NULL;


// Must call activemq::library::ActiveMQCPP::initializeLibrary() once before DMMessageSender

DMMessageSender::DMMessageSender(const string& brokerUri, const string& username,
			const string& password, const string& destination) :
				connection(NULL),
				session(NULL),
				destination(NULL),
				producer(NULL),
				brokerURI(brokerUri),
				destinationName(destination),
				username(username),
				password(password),
				hostname(""),
				port(0),
				useTopic(true),
				sessionTransacted(false),
				printOnPublish(false)
{
}

DMMessageSender::DMMessageSender(const string& username, const string& password,
			const string& destination, const string& hostname, const int port) :
				connection(NULL),
				session(NULL),
				destination(NULL),
				producer(NULL),
				brokerURI(""),
				destinationName(destination),
				username(username),
				password(password),
				hostname(hostname),
				port(port),
				useTopic(true),
				sessionTransacted(false),
				printOnPublish(false)
{
}

DMMessageSender::DMMessageSender(Connection *conn, const string& destination) :
				connection(conn),
				session(NULL),
				destination(NULL),
				producer(NULL),
				brokerURI(""),
				destinationName(destination),
				username(""),
				password(""),
				hostname(""),
				port(0),
				useTopic(true),
				sessionTransacted(false),
				printOnPublish(false)
{
}

/** \deprecated Use the constructor without sysname and with user authentication. */
DMMessageSender::DMMessageSender(enum eewSystemName sysname, const string& username,
			const string& password, const string& destination,
			const string& hostname, const int port) :
				connection(NULL),
				session(NULL),
				destination(NULL),
				producer(NULL),
				brokerURI(""),
				destinationName(destination),
				username(username),
				password(password),
				hostname(hostname),
				port(port),
				useTopic(true),
				sessionTransacted(false),
				printOnPublish(false)
{
(void)sysname;
}

/** \deprecated Use the constructor with user authentication. */
DMMessageSender::DMMessageSender(const string& brokerURI, const string& destination,
			bool useTopic, bool sessionTransacted,
			enum eewSystemName sysname) :
				connection(NULL),
				session(NULL),
				destination(NULL),
				producer(NULL),
				brokerURI(brokerURI),
				destinationName(destination),
				username(""),
				password(""),
				hostname(""),
				port(0),
				useTopic(useTopic),
				sessionTransacted(sessionTransacted),
				printOnPublish(false)
{
(void)sysname;
}

void DMMessageSender::run()
{
    try {

	if(connection == NULL) {
	    // Build brokerURI if not explicitly specified.
	    if (brokerURI.empty()) {
		stringstream uri;
		uri << "failover:(tcp://" << hostname << ":" << port << ")?initialReconnectDelay=100";
		brokerURI = uri.str();
	    }
	    LOGI << "brokerURI=" << brokerURI << endl;

	    // Create a ConnectionFactory.
	    auto_ptr<ConnectionFactory> connectionFactory(
			ConnectionFactory::createCMSConnectionFactory( brokerURI ) );
	
	    // Create a Connection with optional user authentication, and start the connection.
	    if (username.empty() && password.empty()) {
		connection = connectionFactory->createConnection();
	    }
	    else {
		LOGI << "username=" << username << endl;
		connection = connectionFactory->createConnection(username, password);
	    }
	}

        ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
	if( amqConnection != NULL ) {
	    amqConnection->addTransportListener( this );
	}

	// Create a Session.
	if( sessionTransacted ) {
	    session = connection->createSession( Session::SESSION_TRANSACTED );
	} else {
	    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
	}
	
	// Create the destination(s) (Topic or Queue).
	if( useTopic ) {
	    destination = session->createTopic( destinationName );
	} else {
	    destination = session->createQueue( destinationName );
	}
	
	// Create a MessageProducer from the Session to the Topic or Queue.
	producer = session->createProducer( destination );
	producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );

	connection->setExceptionListener(this);
	connection->start();
    } catch ( CMSException& e ) {
    LOGE << e.getStackTraceString();
    }
}


DMMessageSender::~DMMessageSender()
{
    this->cleanup();
}

void DMMessageSender::close()
{
    this->cleanup();
}


void DMMessageSender::onException( const CMSException& ex AMQCPP_UNUSED) {
    printf("CMS Exception occurred.  Shutting down client.\n");
    LOGF << ex.getStackTraceString();
    exit(1);
}

void DMMessageSender::transportInterrupted() 
{
    LOGI << "The DMSender transport has been Interrupted." << std::endl;
}

void DMMessageSender::transportResumed()
{
    LOGI << "The DMSender transport has been Restored." << std::endl;
}

void DMMessageSender::cleanup() 
{
    // Destroy resources.
    try{
	if( producer != NULL ) delete producer;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    producer = NULL;
    
    try{
	if( destination != NULL ) delete destination;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    destination = NULL;
    
    // Close open resources.
    try{
	if( session != NULL ) session->close();
	if( connection != NULL ) connection->close();
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    
    try{
	if( session != NULL ) delete session;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    session = NULL;
    
    try{
	if( connection != NULL ) delete connection;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    connection = NULL;

//    activemq::library::ActiveMQCPP::shutdownLibrary();
}

string DMMessageSender::sendMessage(CoreEventInfo* ceip, string comment) 
{
    if(ceip->getTimestamp().empty()) {
	timeval tv;
	gettimeofday(&tv, NULL);
	ceip->setTimestamp((double)tv.tv_sec + 1.e-06*tv.tv_usec);
    }

    TextMessage *message = session->createTextMessage();
    message->setStringProperty("type", string("alert_") + ceip->getProgramInstance());
    message->setLongProperty("id", alert_msg_id);
    if(++alert_msg_id <= 0) alert_msg_id = 1;
    encoder.encodeMessage(message, ceip, comment);
    pthread_mutex_lock(DMLib::sendLock());
    producer->send( message );
    pthread_mutex_unlock(DMLib::sendLock());

    if (printOnPublish) {
	LOGD << "sendMessage: destination=" << destinationName << endl;
	LOGD << message->getText();
    }
    string msg = message->getText();
    delete message;
    return msg;
}

string DMMessageSender::getEncodedMessage(CoreEventInfo *ceip)
{
    return encoder.encodeMessage(ceip);
}


string DMMessageSender::sendString(string s)
{
    char hostname[1000];
    TextMessage *message = session->createTextMessage( s );
    gethostname(hostname, (int)sizeof(hostname)-1);
    message->setStringProperty("type", string("dmlib_") + string(hostname));
    message->setLongProperty("id", dmlib_msg_id);
    if(++dmlib_msg_id <= 0) dmlib_msg_id = 1;
    if(!msg_src.empty()) {
	message->setStringProperty("source", msg_src);
    }
    pthread_mutex_lock(DMLib::sendLock());
    if(producer == NULL) {
        LOGE << "DMMessageSender::sendString: producer == NULL" << endl;
        exit(0);
    }
    producer->send( message );
    pthread_mutex_unlock(DMLib::sendLock());
    if (printOnPublish) {
	LOGD << "sendString: destination=" << destinationName << endl;
	LOGD << message->getText();
    }
    string msg = message->getText();
    delete message;
    return msg;
}

string DMMessageSender::sendString(string s, string type, string id)
{
    TextMessage *message = session->createTextMessage( s );
    message->setStringProperty("type", type);
    message->setStringProperty("id", id);
    pthread_mutex_lock(DMLib::sendLock());
    producer->send( message );
    pthread_mutex_unlock(DMLib::sendLock());
    string msg = message->getText();
    delete message;
    return msg;
}

string DMMessageSender::sendDeleteMessage(enum eewSystemName sys_name, string id)
{
    CoreEventInfo *cei;

    if(sys_name == EPIC) {
	cei = (CoreEventInfo *)new ElarmsMessage(id);
    }
    else if(sys_name == ELARMS) {
	cei = (CoreEventInfo *)new ElarmsMessage(id);
    }
    else if(sys_name == ONSITE) {
	cei = (CoreEventInfo *)new OnSiteMessage(id);
    }
    else if(sys_name == VS) {
	cei = (CoreEventInfo *)new OnSiteMessage(id);
    }
    else if(sys_name == DM) {
	cei = (CoreEventInfo *)new DMMessage(id);
    }
    else {
	return string("");
    }
    TextMessage* message = session->createTextMessage();
    message->setStringProperty("type", string("delete_") + cei->getProgramInstance());
    message->setLongProperty("id", delete_msg_id);
    if(++delete_msg_id <= 0) delete_msg_id = 1;

    cei->setMessageType(DELETE);
    cei->setOriginTime(nepoch_to_tepoch(time(NULL))); // now
    encoder.encodeMessage(message, cei);
    delete cei;
    pthread_mutex_lock(DMLib::sendLock());
    producer->send( message );
    pthread_mutex_unlock(DMLib::sendLock());
    if (printOnPublish) {
	LOGD << "sendMessage DELETE id: " << id << " destination="
		<< destinationName << endl;
	LOGD << message->getText();
    }
    string msg = message->getText();
    delete message;
    return msg;
}

string DMMessageSender::sendDeleteMessage(CoreEventInfo &cei)
{
    return sendDeleteMessage(cei.getSystemName(), cei.getID());
}

void DMMessageSender::setprintOnPublish(bool f)
{
    printOnPublish = f;
}
