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
#include "DMMessageReceiver.h"
#include "DMLib.h"
#include "plog/Log.h"                               // plog logging library
#include <unistd.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

using namespace cms;
using namespace activemq;
using namespace activemq::core;

// Must call activemq::library::ActiveMQCPP::initializeLibrary() once before DMMessageReceiver

DMMessageReceiver::DMMessageReceiver( 
    const std::string& brokerUri,
    const std::string& username,
    const std::string& password,
    const std::string& dName) :
	connection(NULL),
	session(NULL),
	destination(NULL),
	consumer(NULL),
	brokerURI(brokerUri),
	destinationName(dName),
	username(username),
	password(password),
	hostname(""),
	port(0),
	useTopic(true),
	sessionTransacted(false),
	messageCount(0)
{}

DMMessageReceiver::DMMessageReceiver( 
    const std::string& username,
    const std::string& password,
    const std::string& dName,
    const std::string& hostname,
    const int port) :
	connection(NULL),
	session(NULL),
	destination(NULL),
	consumer(NULL),
	brokerURI(""),
	destinationName(dName),
	username(username),
	password(password),
	hostname(hostname),
	port(port),
	useTopic(true),
	sessionTransacted(false),
	messageCount(0)
{}

DMMessageReceiver::DMMessageReceiver( 
    Connection *conn,
    const std::string& dName) :
	connection(conn),
	session(NULL),
	destination(NULL),
	consumer(NULL),
	brokerURI(""),
	destinationName(dName),
	username(""),
	password(""),
	hostname(""),
	port(0),
	useTopic(true),
	sessionTransacted(false),
	messageCount(0)
{}

// Deprecated - no user authentication.
DMMessageReceiver::DMMessageReceiver( 
    const std::string& bURI,
    const std::string& dName,
    bool uTopic,
    bool sTransacted) :
	connection(NULL),
	session(NULL),
	destination(NULL),
	consumer(NULL),
	brokerURI(bURI),
	destinationName(dName),
	username(""),
	password(""),
	hostname(""),
	port(-1),
	useTopic(uTopic),
	sessionTransacted(sTransacted)
{}


void DMMessageReceiver::run()
{
    this->messageCount = 0;
    try {
	LOGI << "ActiveMQ Receiver is running\n";

	if(connection == NULL) {
	    // Build brokerURI if not explicitly specified.
	    if (brokerURI.empty()) {
		stringstream uri;
		uri << "failover:(tcp://" << hostname << ":" << port << ")?initialReconnectDelay=100";
		brokerURI = uri.str();
	    }
	    LOGI << "brokerURI=" << brokerURI << endl;

	    // Create a ConnectionFactory
	    auto_ptr<ConnectionFactory> connectionFactory(ConnectionFactory::createCMSConnectionFactory( brokerURI ) );
    
	    // Create a Connection
	    if (username.empty() && password.empty()) {
		connection = connectionFactory->createConnection();
	    }
	    else {
		LOGI << "username=" << username << endl;
		connection = connectionFactory->createConnection(username, password);
	    }
	}
    if (connection == NULL) {
        LOGE << "connection is null inside of " << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
    }

    ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
	if( amqConnection != NULL ) {
	    amqConnection->addTransportListener( this );
	}
    if (amqConnection == NULL) {
        LOGE << "amqConnection is null inside of " << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
    }
	
	// Create a Session
	if( sessionTransacted ) {
	    session = connection->createSession( Session::SESSION_TRANSACTED );
	} else {
	    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
	}
    if (session == NULL) {
        LOGE << "session is null inside of " << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
    }
	
	// Create the destination (Topic or Queue)
	if( useTopic ) {
	    destination = session->createTopic( destinationName );
	} else {
	    destination = session->createQueue( destinationName );
	}
    if (destination == NULL) {
        LOGE << "destination is null inside of " << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
    }
	
	// Create a MessageConsumer from the Session
	consumer = session->createConsumer( destination );
    if (consumer == NULL) {
        LOGE << "consumer is null inside of " << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
    }

	connection->setExceptionListener(this);
	connection->start();

    } catch( CMSException& e ) {
        LOGE << e.getStackTraceString();
        LOGF << "Aborting...";
        exit(1);
    }
}

DMMessageReceiver::~DMMessageReceiver() throw()
{
    cleanup();
}

void DMMessageReceiver::close() {
    decoder.close();
    cleanup();
}

void DMMessageReceiver::onMessage( const Message* message )
{
    if( !message ) return;
    try
    {
	CoreEventInfo* ceip = NULL;
	const TextMessage *text_message = dynamic_cast< const TextMessage* >( message );
	if (text_message != NULL) {
	    ceip = decoder.decodeMessage(text_message, ceip);
	}
    } catch (CMSException& e) {
    LOGE << e.getStackTraceString();
    }

    // Commit all messages.
    if( sessionTransacted ) {
	session->commit();
    }
}

CoreEventInfo* DMMessageReceiver::receive(int millisec)
{
    string msg;
    return receive(millisec, msg);
}

CoreEventInfo* DMMessageReceiver::receive(int millisec, string &xml_message) 
{
    CoreEventInfo* ceip = NULL;
    Message* message;

    if (consumer == NULL) {
        LOGE << "consumer is null inside of " << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
    }
    message = consumer->receive(millisec);

    if( !message ) return NULL;

    const TextMessage* text_message = dynamic_cast< const TextMessage* >( message );
    if (text_message != NULL) {
	ceip = decoder.decodeMessage(text_message, ceip);
	xml_message.assign(text_message->getText());
    }
    delete message;

    return ceip;
}

CoreEventInfo* DMMessageReceiver::receiveWait()
{
    string msg;
    return receiveWait(msg);
}

CoreEventInfo* DMMessageReceiver::receiveWait(string &xml_message)
{
    CoreEventInfo* ceip = NULL;

    while( ceip == NULL ) {
	Message *message = consumer->receive(10000);

	if( message ) {
	    const TextMessage* text_message = dynamic_cast< const TextMessage* >( message );
	    if(text_message != NULL) {
		ceip = decoder.decodeMessage(text_message, ceip);
		xml_message.assign(text_message->getText());
	    }
	    delete message;
	}
    }
    return ceip;
}

void DMMessageReceiver::test_receive(int millisec) 
{
    Message* mess = consumer->receive(millisec);

    if (mess != NULL) {
	LOGI << endl << "DMMessageReceiver start" << endl
		<< ((TextMessage*)mess)->getText() << endl << "DMMessageReceiver end" << endl;
	delete mess;
    }
}


// If something bad happens you see it here as this class has also been
// registered as an ExceptionListener with the connection.
void DMMessageReceiver::onException( const CMSException& ex AMQCPP_UNUSED) {
    LOGF <<  "CMS Exception occurred.  Shutting down client." << endl;
    LOGF << ex.getStackTraceString();
    exit(1);
}

void DMMessageReceiver::transportInterrupted() 
{
    LOGI << "The DMReceiver transport has been Interrupted." << std::endl;
}

void DMMessageReceiver::transportResumed()
{
    LOGI << "The DMReceiver transport has been Restored." << std::endl;
}

void DMMessageReceiver::cleanup()
{
    // Destroy resources.
    try{
	if( destination != NULL ) delete destination;
    }catch (CMSException& e) { 
        LOGE << e.getStackTraceString();
    }
    destination = NULL;
    try{
	if( consumer != NULL ) delete consumer;
    }catch (CMSException& e) { 
        LOGE << e.getStackTraceString();
    }
    consumer = NULL;
    // Close open resources.
    try{
	if( session != NULL ) session->close();
	if( connection != NULL ) connection->close();
    }catch (CMSException& e) { 
        LOGE << e.getStackTraceString();
    }
    // Now Destroy them
    try{
	if( session != NULL ) delete session;
    }catch (CMSException& e) { 
        LOGE << e.getStackTraceString();
    }
    session = NULL;
    try{
	if( connection != NULL ) delete connection;
    }catch (CMSException& e) { 
        LOGE << e.getStackTraceString();
    }
    connection = NULL;
}
