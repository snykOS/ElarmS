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
#ifndef __HBConsumer_h
#define __HBConsumer_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include <activemq/core/ActiveMQConnection.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <activemq/library/ActiveMQCPP.h>
#include <cms/ExceptionListener.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>


/** Receives heartbeat messages.
 *  @ingroup dm_lib
 */
class HBConsumer : public cms::MessageListener
{
 private:
    cms::Connection* connection;
    cms::Session* session;
    cms::Destination* destination;
    cms::MessageConsumer* consumer;
    xercesc::XercesDOMParser *parser;
    xercesc::DefaultHandler *error_handler;
    xercesc::MemBufInputSource *input_source;
    std::string topic;
    std::string sender;
    std::string originator;
    std::string timestamp;

    bool getAttributeValue(xercesc::DOMElement *e, const char *name, std::string &value);
    void updateHBInfo(xercesc::DOMNode *node);
    void cleanup();
    
 public:
    HBConsumer(cms::Connection *connection, const std::string& hb_topic="eew.alg.*.hb");

    virtual ~HBConsumer() throw();

    virtual void onMessage(const cms::Message *message);

    void close();

    cms::Connection *getConnection() const { return connection; }
    std::string getTopic() const { return topic; }
    std::string getSender() const { return sender; }
    std::string getOriginator() const { return originator; }
    std::string getTimestamp() const { return timestamp; }
};

#endif
