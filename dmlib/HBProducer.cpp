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
#include "HBProducer.h"
#include "plog/Log.h"                               // plog logging library
#include <unistd.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include "DMLib.h"

using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace std;
using namespace cms;
using namespace xercesc;

bool HBProducer::stop_all = false;
long HBProducer::hb_msg_id = 1;

HBProducer::HBProducer(Connection *conn, const string& sender, const string& topic,
			int time_interval) : connection(conn), session(NULL),
			destination(NULL), producer(NULL), thread(NULL), sender(sender),
			originator(sender), topic(topic), interval(time_interval),
			stop_flag(false), transport_ok(true)
{
    ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
    if( amqConnection != NULL ) {
        amqConnection->addTransportListener( this );
    }
    thread = new decaf::lang::Thread(this);
    thread->start();
}

HBProducer::HBProducer(Connection *conn, const string& sender, const string& originator,
			const string& topic, int time_interval) : connection(conn),
			session(NULL), destination(NULL), producer(NULL), thread(NULL),
			sender(sender), originator(originator), topic(topic),
			interval(time_interval), stop_flag(false), transport_ok(true)
{
    ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
    if( amqConnection != NULL ) {
        amqConnection->addTransportListener( this );
    }
    thread = new decaf::lang::Thread(this);
    thread->start();
}

void HBProducer::run()
{
    XMLPlatformUtils::Initialize();
    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
    destination = session->createTopic( topic );
    producer = session->createProducer( destination );
    producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );
    // the broker only needs to retain the last message sent.
    long time_to_live = interval * 1000 * 3;
    producer->setTimeToLive(time_to_live); // millisecond message time to live
    LOGI << "Sending " << interval << "-second heartbeats to " << topic << endl;
    stop_all = false;

    while( !stop_flag ) {
	if(transport_ok) {
	    sendHeartbeat(producer, session, sender, originator);
	}
	sleep(interval);
    }
}

void HBProducer::sendHeartbeat(const std::string& alt_sender, const std::string &alt_originator, const std::string &timestamp)
{
    sendHeartbeat(producer, session,
            alt_sender.empty() ? sender : alt_sender,
            alt_originator.empty() ? originator : alt_originator,
            timestamp);
}

//static
void HBProducer::sendHeartbeat(MessageProducer *producer, Session *session,
				const string &sender, const string &originator,
				const string &timestamp)
{
    DOMDocument *doc;
    TextMessage* message = NULL;
    XMLCh name[100], value[100];

    if( !producer ) return;

    XMLString::transcode("Core", name, 99);
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(name);

    if( impl == NULL) {
        cerr << "HBProducer:sendHeartbeat: getDOMImplementation failed." << endl;
        return;
    }
    try {
#ifdef XERCES_2
        DOMWriter *serializer = ((DOMImplementationLS*)impl)->createDOMWriter();
#else
        DOMLSSerializer *serializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMConfiguration* config = serializer->getDomConfig();
        if (config->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
	    config->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
	}
#endif
	char buf[50];
	ostringstream os;
	time_t etime;
	struct tm *timeinfo;

	XMLString::transcode("hb", name, 99);
	doc = impl->createDocument(0, name, 0);

	DOMElement *e = doc->getDocumentElement();

	XMLString::transcode("sender", name, 99);
	XMLString::transcode(sender.c_str(), value, 99);
	e->setAttribute(name, value);

	XMLString::transcode("originator", name, 99);
	XMLString::transcode(originator.c_str(), value, 99);
	e->setAttribute(name, value);

	if( !timestamp.empty() ) {
	    XMLString::transcode("timestamp", name, 99);
	    XMLString::transcode(timestamp.c_str(), value, 99);
	    e->setAttribute(name, value);
	}
	else {
	    time(&etime);
	    timeinfo = gmtime(&etime);
#ifdef GNU_COMPILER
	    asctime_r(timeinfo, buf);
#else
	    asctime_r(timeinfo, buf, 50);
#endif
	    // Sun Sep 16 01:03:52 1973\n\0
	    buf[24] = '\0'; // remove '\n'
	    XMLString::transcode("timestamp", name, 99);
	    XMLString::transcode(buf, value, 99);
	    e->setAttribute(name, value);
	}

#ifdef XERCES_2
        XMLCh *t = serializer->writeToString(*doc);
#else
        XMLCh *t = serializer->writeToString(doc);
#endif
        char *msg = XMLString::transcode(t);
	message = session->createTextMessage(msg);
	message->setStringProperty("type", string("hb_") + sender);
	message->setLongProperty("id", hb_msg_id);
	if(++hb_msg_id <= 0) hb_msg_id = 1;
        XMLString::release(&msg);
	XMLString::release(&t);
        serializer->release();

        // call release() to release the entire document resources
        doc->release();
    }
    catch (const OutOfMemoryException&) {
	LOGE << "sendHeartbeat: OutOfMemoryException" << endl;
    }
    catch (const DOMException& e) {
	LOGE << "sendHeartbeat: DOMException code is: " << e.code << endl;
    }
    catch (...) {
	LOGE << "sendHeartbeat: An error occurred creating the message" << endl;
    }

    if(message) {
	pthread_mutex_lock(DMLib::sendLock());
	try {
	    if(!stop_all) producer->send( message );
	} catch (...) {
	    LOGW << "HBProducer::sendHeartbeat: producer.send() failed." << endl;
	}
	pthread_mutex_unlock(DMLib::sendLock());
	delete message;
    }
}

HBProducer::~HBProducer()
{
    close();
    cleanup();
}

void HBProducer::close()
{
    if( !stop_flag ) {
	stop_flag = true;
	thread->join();
    }
}

void HBProducer::cleanup() 
{
    stop_flag = true;

    delete thread;

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
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    
    try{
	if( session != NULL ) delete session;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    session = NULL;
    connection = NULL;

    XMLPlatformUtils::Terminate();
}
