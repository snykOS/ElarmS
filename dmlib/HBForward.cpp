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
#include "plog/Log.h"                           // plog logging library
#include "HBForward.h"
#include "HBProducer.h"
#include "DMLib.h"
#include <sys/types.h>
#include <regex.h>
#include <activemq/core/ActiveMQConnection.h>

using namespace std;
using namespace cms;
using namespace activemq::core;

HBForward::HBForward(Connection *consumer_connection, Connection *forward_conn,
			const string& sender, int hb_interval, const string& consumerTopic,
			const string& forwardTopic) : HBConsumer(consumer_connection, consumerTopic),
			forward_connection(forward_conn), forward_session(NULL),
			forward_destination(NULL), forward_producer(NULL), sender(sender),
			interval(hb_interval), consumer_topic(consumerTopic),
			forward_topic(forwardTopic), forward(true), transport_ok(true)
{
    forward_session = forward_connection->createSession( Session::AUTO_ACKNOWLEDGE );
    forward_destination = forward_session->createTopic( forward_topic );
    forward_producer = forward_session->createProducer( forward_destination );
    forward_producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );

    if(compareTopics(consumer_topic, forward_topic)) {
	forward = false;
	LOGE << "HBForward Error: consumer_topic (" << consumer_topic << ")" << endl;
	LOGE << "          matches forward_topic (" << forward_topic << ")" << endl;
	LOGE << "          Cannot forward heartbeats." << endl;
    }
    else {
	forward = true;
	LOGI << "Forwarding heartbeats from " << consumer_topic << " to " << forward_topic << endl;
    }

    ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( forward_connection );
    if( amqConnection != NULL ) {
        amqConnection->addTransportListener( this );
    }
}

HBForward::~HBForward() throw()
{
    // Destroy resources.
    try{
        if( forward_producer != NULL ) delete forward_producer;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    forward_producer = NULL;

    try{
        if( forward_destination != NULL ) delete forward_destination;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    forward_destination = NULL;

    // Close open resources.
    try{
        if( forward_session != NULL ) forward_session->close();
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }

    try{
        if( forward_session != NULL ) delete forward_session;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    forward_session = NULL;
}

void HBForward::onMessage( const Message* message )
{
    HBConsumer::onMessage(message);

    if( forward && transport_ok ) {
	HBProducer::sendHeartbeat(forward_producer, forward_session, sender, getOriginator());
    }
}

static int
match(const char *string, const char *pattern)
{
   int     status;
   regex_t re;

   if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0)  {
           return(0) ;              /* report error */
   }
   status = regexec(&re, string, (size_t) 0, NULL, 0);
   regfree(&re);
   if (status != 0)  {
           return(0) ;              /* report error */
   }
   return(1);
}

bool HBForward::compareTopics(string consumerTopic, string forwardTopic)
{
    if(match(consumerTopic.c_str(), forwardTopic.c_str()) ||
	match(forwardTopic.c_str(), consumerTopic.c_str()))
    {
	return true;
    }
    return false;
}

