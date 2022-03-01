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
#include "qlib2.h"
#include "DMMessageSender.h"

int main(int argc , char *argv[])
{
    string user = "decimod";
    string password = "weAreOK";
    string destination = "eew.sys.dm.data";
    string hostname = "localhost";
    int port = 61616;
  
    activemq::library::ActiveMQCPP::initializeLibrary();

    DMMessageSender se(DM, user, password, destination, hostname, port);
    se.run();

    int id = 4557299;
    double mag = 3.4;
    double mag_uncer = 0.5;
    double lat = 38.8;
    double lat_uncer = 0.5;
    double lon = -122.82;
    double lon_uncer = 0.5;
    double depth = 8.0;
    double depth_uncer = 5.0;
    double otime = time(NULL) - 5.236; // nominal epoch time
    double origin_time = nepoch_to_tepoch(otime);
    double otime_uncer = 20.0;
    double likelihood = 0.8;
    enum nudMessageType type = NEW; 
    int version = 0;

    // origin_time must be true epoch time. Convert from nominal to true epoch time.
    EventMessage dm(id, mag, mag_uncer, lat, lat_uncer, lon, lon_uncer, depth, depth_uncer,
			origin_time, otime_uncer, likelihood, type, version);

    // Elarms message that contributed to the Decision Module Event
    ElarmsMessage elarms(726, mag, mag_uncer, lat, lat_uncer, lon, lon_uncer, depth, depth_uncer,
			origin_time, otime_uncer, likelihood, type, version);

    // elarms velocity observations
    elarms.addPGVObservation("STA1", "NET", "CHAN", "LOC", 1234.56, 33.12, -123.45, origin_time+3.);
    elarms.addPGVObservation("STA2", "NET", "CHAN", "LOC", 2345.67, 32.85, -124.02, origin_time+4.);

    // elarms acceleration observations
    elarms.addPGAObservation("STA1", "NET", "CHAN", "LOC", 2334.45, 33.12, -123.45, origin_time+3.);
    elarms.addPGAObservation("STA2", "NET", "CHAN", "LOC", 1893.20, 32.85, -124.02, origin_time+4.);

    // OnSite message that contributed to the Decision Module Event
    OnSiteMessage onsite(44853, mag, mag_uncer, lat, lat_uncer, lon, lon_uncer, depth, depth_uncer,
			origin_time, otime_uncer, likelihood, type, version);

    // onsite velocity predictions
    onsite.addPGVPrediction("STA1", "NET", "CHAN", "LOC", 32158.549, 215.1581, origin_time+3.,
			50., 10.5, 33.12, -123.45);
    onsite.addPGVPrediction("STA2", "NET", "CHAN", "LOC", 2158.549, 21.15, origin_time+4.,
			50., 10.5, 32.85, -122.02);

    // VS message that contributed to the Decision Module Event
    VSMessage vs(5828, mag, mag_uncer, lat, lat_uncer, lon, lon_uncer, depth, depth_uncer,
			origin_time, otime_uncer, likelihood, type, version);

    // vs velocity observations
    vs.addPGVObservation("STA1", "NET", "CHAN", "LOC", 1234.56, 33.12, -123.45, origin_time+3.);
    vs.addPGVObservation("STA2", "NET", "CHAN", "LOC", 2345.67, 32.85, -124.02, origin_time+4.);

    // vs acceleration observations
    vs.addPGAObservation("STA1", "NET", "CHAN", "LOC", 2334.45, 33.12, -123.45, origin_time+3.);
    vs.addPGAObservation("STA2", "NET", "CHAN", "LOC", 1893.20, 32.85, -124.02, origin_time+4.);

    dm.loadObsAndPred(&elarms, &vs, &onsite);

    string message = se.sendMessage(dm);
    cout << message << endl;
    se.close();
    activemq::library::ActiveMQCPP::shutdownLibrary();
}
