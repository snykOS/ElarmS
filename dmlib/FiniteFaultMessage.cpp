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
#include "FiniteFaultMessage.h"

const string FaultSegment::segmentShapeString[] = { "UNKNOWN", "line", "triangle", "rectangle" };

int FaultSegment::addVertex(double latitude, double longitude, double depth, string latitude_units,
			string longitude_units, string depth_units)
{
   FaultVertex v(latitude, longitude, depth, latitude_units, longitude_units, depth_units);
   addVertex(v);
   return 1;
}

FiniteFaultMessage::FiniteFaultMessage(enum eewSystemName sys_name,
                 enum FaultSegment::FaultSegmentShape shape,
			     string i,
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
			     string o_time_uncer_units) : AlgMessage(sys_name,
								i, 
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
								o_time_uncer_units)
{
    Atten_geom = true;
    Segment_shape = shape;
    Confidence = 0.0;
}

FiniteFaultMessage::FiniteFaultMessage(enum eewSystemName sys_name,
                 enum FaultSegment::FaultSegmentShape shape,
			     int i,
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
			     string o_time_uncer_units) : AlgMessage(sys_name,
								i, 
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
								o_time_uncer_units)
{
    Atten_geom = true;
    Segment_shape = shape;
    Confidence = 0.0;
}

FiniteFaultMessage::~FiniteFaultMessage() 
{
}

int FiniteFaultMessage::addSegment(FaultSegment &seg)
{
    if(seg.getSegmentShape() != Segment_shape) {
	cerr << "FiniteFaultMessage::addSegment: cannot add a " << seg.getSegmentShapeString()
		<< " to a FiniteFaultMessage of type: " << getSegmentShapeString() << endl;
	return 0;
    }
/*
    FaultSegmentIter i = segments.begin();
    FaultSegmentIter e = segments.end();
    for(; i != e; i++) {
        if(seg == *i) {
            return 0;
        }
    }
*/
    segments.push_back(seg);
    this->updateVersion();
    return 1;
}

/*
void FiniteFaultMessage::deleteSegment(FaultSegment &A)
{
    segments.remove(A);
    this->updateVersion();
}
*/

string FiniteFaultMessage::updateFrom(const CoreEventInfo &cei)
{
    string s = AlgMessage::updateFrom(cei);

    if(cei.getSystemName() == FINITE_FAULT) {
	FiniteFaultMessage *finite= (FiniteFaultMessage *)&cei;

	segments.clear();

	for(FaultSegmentIter it = finite->segments.begin(); it != finite->segments.end(); it++) 
	{
	    segments.push_back(*it);
	}
    }
    return s;
}
