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
#ifndef __HBProducer_h
#define __HBProducer_h

/** \page hb_page Heartbeat Example XML Message
 *  \section sec1 An Example Heartbeat Message
 *  \include hb.xml
 */

#include <activemq/core/ActiveMQConnection.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>


/** In a separate thread, periodically sends heartbeat messages.
 *  @ingroup dm_lib
 */
class HBProducer : public activemq::transport::DefaultTransportListener, public decaf::lang::Runnable
{
 private:
    cms::Connection* connection;
    cms::Session* session;
    cms::Destination* destination;
    cms::MessageProducer* producer;
    decaf::lang::Thread *thread;
    std::string sender;
    std::string originator;
    std::string topic;
    int interval;
    bool stop_flag;
    bool transport_ok;

    void cleanup();

    virtual void transportInterrupted() {
	transport_ok = false;
    }
    virtual void transportResumed() {
	transport_ok = true;
    }
    static long hb_msg_id;
    static bool stop_all;
    
 public:
    HBProducer(cms::Connection *connection, const std::string& sender, const std::string& topic,
		int time_interval=5);
    HBProducer(cms::Connection *connection, const std::string& sender, const std::string& originator,
		const std::string& topic, int time_interval=5);

    virtual ~HBProducer();

    static void sendHeartbeat(cms::MessageProducer *producer, cms::Session *session,
				const std::string &sender, const std::string &originator,
				const std::string &timestamp="");
    static void * sendHeartbeats(void *client_data);

    void sendHeartbeat(const std::string& alt_sender="", const std::string &alt_originator="", const std::string &timestamp="");

    virtual void run();
    void close();
    int getInterval() { return interval; }

    void stop() {
	stop_all = true;
    }
};

#endif
