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
#ifndef __DMMessageReceiver_h
#define __DMMessageReceiver_h

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/System.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include "DMMessageDecoder.h"


/** Receive an EpicMessage, ElarmsMessage, OnSiteMEssage, VSMessage, or DMMessage.
 *  XML messages in a cms::TextMessage are decoded and the appropriate
 *  CoreEventInfo subclass object is created.
 *  @ingroup dm_lib
 */
class DMMessageReceiver : public cms::ExceptionListener,
			  public cms::MessageListener,
			  public activemq::transport::DefaultTransportListener
{
  private:
    cms::Connection* connection;
    cms::Session* session;
    cms::Destination* destination;
    cms::MessageConsumer* consumer;
    std::string brokerURI;
    std::string destinationName;
    std::string username;
    std::string password;
    std::string hostname;
    int port;
    bool useTopic;
    bool sessionTransacted;
    unsigned int messageCount;
    DMMessageDecoder decoder;
    
    void cleanup();

  public:
    DMMessageReceiver(const std::string& brokerUri,
		      const std::string& username,
		      const std::string& password,
		      const std::string& dName);
    DMMessageReceiver(const std::string& username,
		      const std::string& password,
		      const std::string& dName,
		      const std::string& hostname,
		      const int port);
    DMMessageReceiver(cms::Connection *connection, const std::string& dName);

    /** \deprecated  Use the constructor with user authentication. */
    DMMessageReceiver(const std::string& brokerURI,
		      const std::string& dName,
		      bool useTopic = true,
		      bool sessionTransacted = false);

    virtual ~DMMessageReceiver() throw();
    void close();
    virtual void run();
    virtual CoreEventInfo* receive(int millisec = 0);
    virtual CoreEventInfo* receive(int millisec, string &xml_message);
    virtual CoreEventInfo* receiveWait();
    virtual CoreEventInfo* receiveWait(string &xml_message);
    virtual void test_receive(int millisec = 0);
    virtual void onMessage( const cms::Message* message );
    virtual void onException( const cms::CMSException& ex AMQCPP_UNUSED);
    virtual void transportInterrupted();
    virtual void transportResumed();
    
    cms::Connection *getConnection() { return connection; }
};

#endif
