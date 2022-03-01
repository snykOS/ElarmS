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
#include <decaf/lang/Thread.h>
#include <iostream>
#include <fstream>

int main(int argc , char* argv[] ) {
    
    string elarmsstr = "elarms";
    string onsitestr = "onsite";
    string vsstr = "vs";

    string newstr = "new";
    string updatestr = "update";
    string deletestr = "delete";
    
    string allsystemstr = (elarmsstr + " ");
    allsystemstr += (onsitestr + " ");
    allsystemstr += vsstr;
    
    string allversionstr = (newstr + " ");
    allversionstr += (updatestr + " ");
    allversionstr += deletestr;
    
    string usestring =  "cmdline system_name message_status id mag mag_uncer lat lat_uncer lon lon_uncer dep dep_uncer orig_time orig_time_uncer likelyhood version";
    //enum messageType {NEW, UPDATE, DELETE};

    

    enum eewSystemName eewsn = ELARMS;
    int id = -9;
    double mag=-9.9;
    double mag_uncer=-9.9;
    double lat=-999.9;
    double lat_uncer=-999.9;
    double lon=-999.9;
    double lon_uncer=-999.9;
    double dep=-9.9;
    double dep_uncer=-9.9;
    //string o_time="2999-12-31T24:60:60.0000Z";
    double o_time=-99999.99;
    double o_time_uncer=-9.9;
    double lklyhd=-9.9;
    enum nudMessageType mssgt = NEW;
    int version=0;
    

    
    
    if (argc == 16) {
	//////////
	if (!(elarmsstr.compare(*(argv+1)))) {
	    eewsn = ELARMS;
	}
	else if (!(onsitestr.compare(*(argv+1)))) {
	    eewsn = ONSITE;
	}
	else if (!(vsstr.compare(*(argv+1)))) {
	    eewsn = VS;
	} else {
	    cout << usestring << endl;
	    cout << "accepted " << allsystemstr  << endl;
	    return 1;
	}
	/////////
	if (!(newstr.compare(*(argv+2)))) {
	    cout << "assigned NEW" << endl;
	    mssgt = NEW;
	}
	else if (!(updatestr.compare(*(argv+2)))) {
	    cout << "assigned UPDATE" << endl;
	    mssgt = UPDATE;
	} 
	else if (!(deletestr.compare(*(argv+2)))) {
	    cout << "assigned DELETE" << endl;
	    mssgt = DELETE;
	} else  {
	    cout << usestring << endl;
	    cout << "accepted " << allversionstr  << endl;
	    return 1;
	}
	//////////

	//mssgt = *(argv+2);
	id = atoi(*(argv+3));
	mag = atof(*(argv+4));
	mag_uncer = atof(*(argv+5));
	lat = atof(*(argv+6));
	lat_uncer = atof(*(argv+7));
	lon = atof(*(argv+8));
	lon_uncer = atof(*(argv+9));
	dep = atof(*(argv+10));
	dep_uncer = atof(*(argv+11));
	o_time = atof(*(argv+12));
	o_time_uncer = atof(*(argv+13));
	lklyhd = atof(*(argv+14));
	version = atof(*(argv+15));
	
    } else {
	    cout << usestring << endl;
	    return 1;
    }
    
    cout << "EEWSN: " << eewsn << endl;
    cout << "ID: " << id << endl;
    cout << "MAG: " << mag << endl;
    cout << "MAG_UNCER: " << mag_uncer << endl;
    cout << "LAT: " << lat << endl;
    cout << "LAT_UNCER: " << lat_uncer << endl;
    cout << "LON: " << lon << endl;
    cout << "LON_UNCER: " << lon_uncer << endl;
    cout << "DEP: " << dep << endl;
    cout << "DEP_UNCER: " << dep_uncer << endl;
    cout << "O_TIME: " << o_time << endl;
    cout << "O_TIME_UNCER: " << o_time_uncer << endl;
    cout << "LKLYHD: " << lklyhd << endl;
    cout << "MSSGT: " << mssgt << endl;
    cout << "VERSION: " << version << endl;
    
    activemq::library::ActiveMQCPP::initializeLibrary();

    string username = "cmdlineSender";
    string password = "clsIsOK";
    string hostname = "localhost";
    int port = 61616;
    string destinationName = "eew.alg." + 
	((eewsn == ELARMS) ? elarmsstr :
	 (eewsn == ONSITE) ? onsitestr :
	 (eewsn == VS) ? vsstr : "unknown") + ".data";

    DMMessageSender sender( eewsn, username, password, destinationName, hostname, port);
    sender.setprintOnPublish(true);

    sender.run();

    
    if (eewsn == ELARMS) {
	ElarmsMessage* messageptr = new ElarmsMessage (id,
						       mag,
						       mag_uncer,
						       lat,
						       lat_uncer,
						       lon,
						       lon_uncer,
						       dep,
						       dep_uncer,
						       o_time,
						       o_time_uncer,
						       lklyhd,
						       mssgt,
						       version);
	sender.sendMessage(*messageptr);
    }
    else if (eewsn == ONSITE) {
	OnSiteMessage osm (id, mag, mag_uncer, lat, lat_uncer,
			   lon, lon_uncer, dep, dep_uncer,
			   o_time, o_time_uncer, lklyhd, mssgt, version, LIVE,
			   "Mw","Mw","deg","deg","deg","deg","km","km","UTC","sec");
	sender.sendMessage(osm);
#if 0
	OnSiteMessage* messageptr = new OnSiteMessage (id,
						       mag,
						       mag_uncer,
						       lat,
						       lat_uncer,
						       lon,
						       lon_uncer,
						       dep,
						       dep_uncer,
						       o_time,
						       o_time_uncer,
						       lklyhd,
						       mssgt,
						       version);
	sender.sendMessage(*messageptr);
#endif
    }
    else if (eewsn == VS) {
	VSMessage* messageptr = new VSMessage (id,
					       mag,
					       mag_uncer,
					       lat,
					       lat_uncer,
					       lon,
					       lon_uncer,
					       dep,
					       dep_uncer,
					       o_time,
					       o_time_uncer,
					       lklyhd,
					       mssgt,
					       version);
	sender.sendMessage(*messageptr);
    } else {
	cout << usestring << endl;
	cout << "accepted " << allsystemstr  << endl;
	return 1;
    }

    

    

    //sender.sendDeleteMessage(123456);    
    sender.close();
    activemq::library::ActiveMQCPP::shutdownLibrary();
    return 0;
}
