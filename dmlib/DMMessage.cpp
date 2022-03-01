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
#include <map>
#include "DMMessage.h"

DMMessage::DMMessage(string id,
		     double mag,
		     double mag_uncer,
		     double lat,
		     double lat_uncer,
		     double lon,
		     double lon_uncer,
		     double dep,
		     double dep_uncer,
		     double o_time,
		     double o_time_uncer,
		     double lklyhd,
		     enum nudMessageType type,
		     int ver,
		     enum MessageCategory category,
		     string time_stamp,
		     string alg_ver,
		     string instance,
		     int num_stations,
		     string mag_units,
		     string mag_uncer_units,
		     string lat_units,
		     string lat_uncer_units,
		     string lon_units,
		     string lon_uncer_units,
		     string dep_units,
		     string dep_uncer_units,
		     string o_time_units,
		     string o_time_uncer_units) : AlgMessage(DM,
							id, 
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
							type,
							ver,
							category,
							time_stamp,
							alg_ver,
							instance,
							num_stations,
							"0",
							"",
							"",
							mag_units ,
							mag_uncer_units,
							lat_units,
							lat_uncer_units,
							lon_units,
							lon_uncer_units,
							dep_units,
							dep_uncer_units,
							o_time_units,
							o_time_uncer_units
							) {}
DMMessage::DMMessage(int id,
		     double mag,
		     double mag_uncer,
		     double lat,
		     double lat_uncer,
		     double lon,
		     double lon_uncer,
		     double dep,
		     double dep_uncer,
		     double o_time,
		     double o_time_uncer,
		     double lklyhd,
		     enum nudMessageType type,
		     int ver,
		     enum MessageCategory category,
		     string time_stamp,
		     string alg_ver,
		     string instance,
		     int num_stations,
		     string mag_units,
		     string mag_uncer_units,
		     string lat_units,
		     string lat_uncer_units,
		     string lon_units,
		     string lon_uncer_units,
		     string dep_units,
		     string dep_uncer_units,
		     string o_time_units,
		     string o_time_uncer_units) : AlgMessage(DM,
							id,
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
							type,
							ver,
							category,
							time_stamp,
							alg_ver,
							instance,
							num_stations,
							0,
							"",
							"",
							mag_units ,
							mag_uncer_units,
							lat_units,
							lat_uncer_units,
							lon_units,
							lon_uncer_units,
							dep_units,
							dep_uncer_units,
							o_time_units,
							o_time_uncer_units
							) {}

DMMessage::~DMMessage() 
{
    gm_observations.clear();
    gm_predictions.clear();
    finite_faults.clear();
    contributors.clear();
}

bool DMMessage::loadAlgMessages(vector<AlgMessage *> &alg, bool load_fault_info)
{
    gm_observations.clear();
    gm_predictions.clear();
    contributors.clear();
    finite_faults.clear();

    for(int i = 0; i < (int)alg.size(); i++) {
	for(GMObservationIter it = alg[i]->getGMObservationIteratorBegin();
		it != alg[i]->getGMObservationIteratorEnd(); it++) {
	    addGMObservation(*it);
	}
	for(GMPredictionIter it = alg[i]->getGMPredictionIteratorBegin();
		it != alg[i]->getGMPredictionIteratorEnd(); it++) {
	    addGMPrediction(*it);
	}
	addContributor(alg[i]);
    }
    // take the most recent version of the same algorithm
    map<enum eewSystemName, AlgMessage *> system_map;
    map<enum eewSystemName, AlgMessage *>::iterator map_it;

    for(int i = 0; i < (int)alg.size(); i++) {
        if((map_it = system_map.find(alg[i]->getSystemName())) == system_map.end()) {
            system_map[alg[i]->getSystemName()] = alg[i];
        }
        else if(CoreEventInfo::compareTimeString(alg[i], map_it->second) > 0) {
            system_map[alg[i]->getSystemName()] = alg[i];
        }
    }

    if(load_fault_info) {
        DMMessage *dm;
        list<FiniteFaultMessage>::iterator it, end;

        for(map_it = system_map.begin(); map_it != system_map.end(); map_it++)
        {
            switch(map_it->second->getSystemName())
            {
            case FINITE_FAULT:
            case FINDER:
            case BEFORES:
                finite_faults.push_back(*(FiniteFaultMessage *)map_it->second);
                break;
            case EQINFO2GM:
            case SA:
                dm = (DMMessage *)map_it->second;
                it = dm->getFiniteFaultIteratorBegin();
                end = dm->getFiniteFaultIteratorEnd();
                while(it != end) { finite_faults.push_back(*it); it++; }
                break;
            default:
                break;
            }
        }   
    }

    // currently not checking for changes in observations/predictions
    return false; // return no change
}

string DMMessage::updateFrom(const CoreEventInfo &cei)
{
    string s = AlgMessage::updateFrom(cei);

    if(cei.getSystemName() == SA || cei.getSystemName() == DM) {
        DMMessage *dm= (DMMessage *)&cei;

        finite_faults.clear();

        for(FFMsgIter it = dm->finite_faults.begin(); it != dm->finite_faults.end(); it++)
        {
            finite_faults.push_back(*it);
        }

        contributors.clear();

        for(DMContributorIter it = dm->getContributorBegin(); it != dm->getContributorEnd(); it++)
        {
            contributors.push_back(*it);
        }
    }
    return s;
}

string DMMessage::toString(bool long_form)
{
    stringstream s;
    string str;

    s << CoreEventInfo::toString(long_form);

    str = AlgMessage::obsToString();
    if(str.length() > 0) s << endl << str;

    str = AlgMessage::predToString();
    if(str.length() > 0) s << endl << str;

    return s.str();
}
