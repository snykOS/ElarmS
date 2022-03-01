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
#include "DMMessageSender.h"
#include "TimeStamp.h"
#include <decaf/lang/Thread.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv)
{
/*
    if(argc < 2) {
	cout << "Usage: " << argv[0] << " uri=ACTIVEMQ_URI" << endl;
	cout << "       " << argv[0] << " uri=failover:(tcp://localhost:PORT)" << endl;
	return(0);
    }
*/

    activemq::library::ActiveMQCPP::initializeLibrary();

//    string brokerURI = argv[1];
string brokerURI = "failover:(tcp://localhost:PORT)";
string passwd = "";

    
    DMMessageSender senderElarms( brokerURI, "elarms", passwd, "eew.alg.elarms.data");
    DMMessageSender senderEpic(   brokerURI, "elarms", passwd, "eew.alg.epic.data");
    DMMessageSender senderOnsite( brokerURI, "elarms", passwd, "eew.alg.onsite.data");

    double otime = TimeStamp::current_time().ts_as_double(UNIX_TIME);

// ElarmsMessage(int id, double mag, double mag_uncer, double lat, double lat_uncer, double lon, double lon_uncer,
// 	double dep, double dep_uncer, double o_time, double o_time_uncer, double lklihd,...


    ElarmsMessage *elarms = new ElarmsMessage(123234, 3.5, 1.454, 34.00, 0.345, -118.7, 0.356, 9.1, .789, otime, 23.45, .90);
    elarms->addPGVObservation("ST1", "NET", "CHAN", "LOC", 1234.56, 33.12, -118.45, otime);
    elarms->addPGVObservation("ST2", "NET", "CHAN", "LOC", 2345.67, 34.23, -118.56, otime+15.);
    elarms->addPGAObservation("ST1", "NET", "CHAN", "LOC", 1234.56, 33.12, -118.45, otime+20.);
    elarms->addPGAObservation("ST2", "NET", "CHAN", "LOC", 2345.67, 34.23, -118.56, otime+5); 

    EpicMessage *epic = new EpicMessage (234566, 3.5, 1.454, 34.23, 0.345, -118.7, 0.356, 9.1, .789, otime + 3, 23.45, .90);
    epic->addPGVObservation("ST1", "NET", "CHAN", "LOC", 234.56, 34.12, -118.45, otime + 5.);
    epic->addPGVObservation("ST2", "NET", "CHAN", "LOC", 345.67, 44.23, -118.56, otime + 11.);
    epic->addPGAObservation("ST1", "NET", "CHAN", "LOC", 1234.56, 33.12, -118.45, otime + 11.);
    epic->addPGAObservation("ST2", "NET", "CHAN", "LOC", 2345.67, 44.23, -118.56, otime + 11.);

    OnSiteMessage *onsite = new OnSiteMessage (234, 3.5, 1.454, 34.18, 0.345, -118.7, 0.356, 9.1, .789, otime+3.5, 23.45, .90);
    onsite->addPGVPrediction("ST1", "NET", "CHAN", "LOC", 32158.549, 215.1581, otime + 12., 1003, 10.5, 33.912, -118.01);
    onsite->addPGVPrediction("ST2", "NET", "CHAN", "LOC", 2158.549, 21.15, otime + 13., 1003, 10.5, 34.562, -118.99);

    senderElarms.run();
    senderEpic.run();
    senderOnsite.run();

    string s;
    s = senderElarms.sendMessage(*elarms);
    cout << "Elarms:" << endl << s << endl;
    s = senderEpic.sendMessage(*epic);
    cout << "Epic:" << endl << s << endl;
    s = senderOnsite.sendMessage(*onsite);
    cout << "Onsite:" << endl << s << endl;

    senderElarms.close();
    senderEpic.close();
    senderOnsite.close();
    activemq::library::ActiveMQCPP::shutdownLibrary();
    return 0;
}
