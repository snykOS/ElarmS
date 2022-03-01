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
#include "DMMessageReceiver.h"
#include "HBConsumer.h"

using namespace cms;

class HBPrinter : public HBConsumer
{
  public:
    HBPrinter(Connection *connection, const string &hb_topic) :
		HBConsumer(connection, hb_topic)
    {
    }
    virtual void onMessage( const Message *message ) throw()
    {
	HBConsumer::onMessage(message); // call superclass method to parse message

	// use HBConsumer functions to get the heartbeat attributes
	cout << "heartbeat: " << getSender() << " " << getOriginator() << " "
		<< getTimestamp() << endl;
    }
};

int main(int argc AMQCPP_UNUSED, char* argv[] AMQCPP_UNUSED)
{
    activemq::library::ActiveMQCPP::initializeLibrary();

    string username = "ha";
    string password = "haIsOK";
    string hostname = "localhost";
    int port = 61616;
    string destination = "eew.sys.dm.data";

    DMMessageReceiver receiver(username, password, destination, hostname, port);
    receiver.run();

    // start heartbeat consumer and printer
    HBPrinter hp(receiver.getConnection(), "eew.sys.dm.hb");

    EventMessage *dm;

    while (true) {
	string xml;
	// receive Event messages from "eew.alg.dm.data"
        if( (dm = (EventMessage *)receiver.receive(100, xml)) != NULL) {
	    cout << xml << endl; // print the xml message
            dm->coutPrint(); // print the parsed class values
	    delete dm;
	}
    }

    receiver.close();

    activemq::library::ActiveMQCPP::shutdownLibrary();
}
