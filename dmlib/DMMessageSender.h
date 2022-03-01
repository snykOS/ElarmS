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
#ifndef __DMMessageSender_h
#define __DMMessageSender_h

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <sstream>
#include "DMMessageEncoder.h"
#include "DMMessageDecoder.h"
#include "DMLib.h"
#include "EpicMessage.h"
#include "ElarmsMessage.h"
#include "OnSiteMessage.h"
#include "VSMessage.h"
#include "FinDerMessage.h"
#include "GPSlipMessage.h"


/** Send EpicMessage, ElarmsMessage, OnSiteMEssage, VSMessage, or DMMessage objects.
 *  Objects are encoded as XML messages and send as a cms::TextMessage.
 *  @ingroup dm_lib
 */
class DMMessageSender : public cms::ExceptionListener,
			public activemq::transport::DefaultTransportListener,
			public decaf::lang::Runnable
{
 private:
    cms::Connection* connection;
    cms::Session* session;
    cms::Destination* destination;
    cms::MessageProducer* producer;
    DMMessageEncoder encoder;
    string brokerURI;
    string destinationName;
    string username;
    string password;
    string hostname;
    string msg_src;
    int port;
    bool useTopic;
    bool sessionTransacted;
    bool printOnPublish;
    static long alert_msg_id;
    static long delete_msg_id;
    static long dmlib_msg_id;

    void cleanup();
    virtual void onException( const cms::CMSException& ex AMQCPP_UNUSED);
    virtual void transportInterrupted();
    virtual void transportResumed();
    
 public:
    DMMessageSender(const string& brokerUri,
		    const string& username,
		    const string& password,
		    const string& destination);
    DMMessageSender(const string& username,
		    const string& password,
		    const string& destination,
		    const string& hostname,
		    const int port);
    DMMessageSender(cms::Connection *connection, const string& destination);
    /** \deprecated Use the constructor without sysname and with user authentication. */
    DMMessageSender(enum eewSystemName sysname = ELARMS,
		    const string& username = "",
		    const string& password = "",
		    const string& destination = "eew.alg.unknown.data",
		    const string& hostname = "eew2.geo.berkeley.edu",
		    const int port = 61616);

    /** \deprecated Use the constructor with user authentication. */
    DMMessageSender(const string& brokerURI = "failover:(tcp://eew2.geo.berkeley.edu:61616)",
		    const string& destination = "DecisionModule",
		    bool useTopic = true,
		    bool sessionTransacted = false,
		    enum eewSystemName sysname = ELARMS);

    virtual ~DMMessageSender();

    virtual void run();
    void close();

    cms::Connection *getConnection() { return connection; }
    string getEncodedMessage(CoreEventInfo *ceip);

    string sendString(string s);
    string sendString(string s, string type, string id);
    string sendString(string s, string type, int id) {
	char sid[1000];
	snprintf(sid, sizeof(sid), "%d", id);
	return sendString(s, type, sid);
    }
    /** \deprecated Use sendMessage(). All messages are now sent in XML. */
    void sendMessageXML(CoreEventInfo *ceip) { sendMessage(ceip); }
    /** \deprecated Use sendMessage(). All messages are now sent in XML. */
    void sendMessageXML(CoreEventInfo *ceip, string comment) { sendMessage(ceip, comment); }

    string sendMessage(CoreEventInfo *ceip, string comment="");
    
    string sendMessage(OnSiteMessage &m, string comment="") {
	return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(VSMessage &m, string comment="") {
	return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(EpicMessage &m, string comment="") {
	return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(ElarmsMessage &m, string comment="") {
	return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(FinDerMessage &m, string comment="") {
	return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(GPSlipMessage &m, string comment="") {
	return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(DMMessage &m, string comment="") {
	return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(GMMessage &m, string comment="") {
	    return sendMessage((CoreEventInfo *)&m, comment);
    }
    string sendMessage(FiniteFaultMessage &m, string comment="") {
	    return sendMessage((CoreEventInfo *)&m, comment);
    }
    void setSource(string source) { msg_src = source; }

    string sendDeleteMessage(enum eewSystemName sys_name, int id) {
	char sid[1000];
	snprintf(sid, sizeof(sid), "%d", id);
	return sendDeleteMessage(sys_name, sid);
    }
    string sendDeleteMessage(enum eewSystemName sys_name, string id);
    string sendDeleteMessage(CoreEventInfo &cei);
    void setprintOnPublish(bool flag);
};

#endif
