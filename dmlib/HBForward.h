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
#ifndef __HBForward_h
#define __HBForward_h

#include <activemq/transport/DefaultTransportListener.h>
#include "HBConsumer.h"

/** Receives and forwards heartbeat messages.
 *  @ingroup dm_lib
 */
class HBForward : public activemq::transport::DefaultTransportListener, public HBConsumer
{
 private:
    cms::Connection *forward_connection;
    cms::Session* forward_session;
    cms::Destination* forward_destination;
    cms::MessageProducer* forward_producer;
    std::string sender;
    int interval;
    std::string consumer_topic;
    std::string forward_topic;
    bool forward;
    bool transport_ok;

    bool compareTopics(std::string consumer_topic, std::string forward_topic);

    virtual void transportInterrupted() {
	transport_ok = false;
    }
    virtual void transportResumed() {
	transport_ok = true;
    }
    
 public:
    HBForward(cms::Connection *consumer_connection, cms::Connection *forward_connection,
		const std::string& sender, int hb_interval=5,
		const std::string& consumer_topic="eew.alg.*.hb",
		const std::string& forward_topic="eew.sys.dm.hb");

    virtual ~HBForward() throw();
    virtual void onMessage( const cms::Message* message );

    std::string getSender() { return sender; }
    std::string getConsumerTopic() { return consumer_topic; }
    std::string getForwardTopic() { return forward_topic; }
    cms::Connection *getConsumerConnection() { return getConnection(); }
    cms::Connection *getForwardConnection() { return forward_connection; }
};

#endif
