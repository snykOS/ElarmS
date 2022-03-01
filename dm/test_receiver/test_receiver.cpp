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
#include <stdlib.h>
#include <unistd.h>

#include "DMMessageReceiver.h"

int main(int argc AMQCPP_UNUSED, char* argv[] AMQCPP_UNUSED)
{
    activemq::library::ActiveMQCPP::initializeLibrary();

    string username = "testReceiver";
    string password = "trIsOK";
    string hostname = "localhost";
    int port = 61616;
    string destinationName = "eew.sys.dm.data";

    char c;
    // process command line args
    while ((c = getopt(argc, argv, "u:p:h:P:t:")) != -1)
    {
        switch(c) {
            case 'u': username = optarg; break;
            case 'p': password = optarg; break;
            case 'h': hostname = optarg; break;
            case 'P': {int tmp; if (sscanf(optarg, "%d", &tmp) == 1) port = tmp; break;}
            case 't': destinationName = optarg; break;
            case '?': cout << "Usage: [-u user][-p pwd][-h host][-P port][-t topic]" << endl;
                      exit(2);
        }
    }
    // allow for a single topic to be backwards compatible
    if (optind < argc) {
        destinationName = argv[optind++];
    }
    // error right?
    if (optind < argc) {
        cout << "Warning, command line still contains args at optind=" << optind << endl;
        cout << "Use -? for options" << endl;
    }

    DMMessageReceiver receiver( username, password, destinationName, hostname, port );
    receiver.run();

    cout << "DM TEST Receiver Running" << endl 
        << "Reading topic: " << destinationName << " from " << hostname << ":" << port << endl;

    CoreEventInfo* ceip;
    string xml_message;

    while (true) {
	if( (ceip = receiver.receive(50000, xml_message)) != NULL) {
	    cout << xml_message << endl;
// Uncomment to print the parsed EventMessage object.
//	    ceip->coutPrint();

	    delete ceip;
	}
    }
    
    receiver.close();

    activemq::library::ActiveMQCPP::shutdownLibrary();
    exit(0);
}

