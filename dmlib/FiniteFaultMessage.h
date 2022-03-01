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
#ifndef __FiniteFaultMessage_h
#define __FiniteFaultMessage_h

/** \page finite_fault_page FiniteFault Example XML Message
 *  \section finite_fault_sec An Example FiniteFault Message
 *  The <b>finite_fault_info</b> node will only be present when there are predictions.
 *  \include finite_fault.xml
 */
/*
 *  <atten_geom>: Is this geometry appropriate to be used for attenuation relation distance
 *		calculations (true) or not (false).  E.g. FinDER output would have a “true” value. 
 * <segment_shape>: ”line”, ”triangle”, or ”rectangle”.  Text description of the shape that all segments in
 *		the message will have.  Note that these cannot be mixed within one finite_fault message
 *		(e.g. FinDER = “line”), all segments are expected to have the same shape.
 * <confidence>: A number giving the algorithm’s confidence in its own solution.
 * <segment>: This tag contains all the messages for one fault segment (one line, triangle or rectangle),
 * 		there will be one of these for each line, triangle or rectangle in the fault model.
 * <vertices>: contained within <segment>, it is a tag that contains all the vertex tags for one segment.
 *		If the segment shape is “line”, vertices will contain two <vertex> messages, if the segment
 * 		shape is “triangle”, then it will contain three and if it is “rectangle”, then it will
 *		contain four vertex messages.
 * <vertex>: tag containing <lat>, <lon> and <depth> defining one vertex point.  
 * <slip>: This is a tag found under the <segment> field and contains tags for the slip attributed to the line,
 *		triangle or rectangle defined by the vertices under the same <segment> tag.
 * <ss>, <ss_uncer>, <ds>, <ds_uncer> : strike slip, strike slip uncertainty, dip slip, dip slip uncertainty
 *		for one fault segment (one line, triangle or rectangle)
 * <global uncertainty> : tag containing uncertainty estimates pertaining to the whole fault model (ie the
 *		entire collection of fault segments)
 * <lon_trans>, <lat_trans>, <total_len>, <strike>, <dip> : These tags are found within the <global_uncertainty>
 *		field and contain the uncertainty estimates for the translation, total length, strike and dip of
 *		the overall model. (e.g. 1-sigma bounds on how much the entire model could be translated).
 *		Parameters that don’t make sense for certain algorithms (e.g. total length for a curved fault model)
 *		could be excluded.
 */

#include "AlgMessage.h"
class FiniteFaultMessage;

class FaultVertex
{
  private:
    double Latitude;
    double Longitude;
    double Depth;
    string Latitude_units;
    string Longitude_units;
    string Depth_units;

  public:
    FaultVertex(double latitude, double longitude, double depth, string latitude_units="deg",
		string longitude_units="deg", string depth_units="km") :
	Latitude(latitude), Longitude(longitude), Depth(depth),
	Latitude_units(latitude_units), Longitude_units(longitude_units), Depth_units(depth_units) {}

    virtual ~FaultVertex() {}

    double setLatitude(double latitude) { Latitude = latitude; return Latitude; }
    double setLongitude(double longitude) { Longitude = longitude; return Longitude; }
    double setDepth(double depth) { Depth = depth; return Depth; }
    string setLatitudeUnits(string latitude_units) { Latitude_units = latitude_units; return Latitude_units; } 
    string setLongitudeUnits(string longitude_units) { Longitude_units = longitude_units; return Longitude_units; }
    string setDepthUnits(string depth_units) { Depth_units = depth_units; return Depth_units; }

    double getLatitude() const { return Latitude; }
    double getLongitude() const { return Longitude; }
    double getDepth() const { return Depth; }
    string getLatitudeUnits() const { return Latitude_units; }
    string getLongitudeUnits() const { return Longitude_units; }
    string getDepthUnits() const { return Depth_units; }
};

class FaultSlip
{
  private:
    double Strike_slip;
    double Dip_slip;
    double Strike_slip_uncer;
    double Dip_slip_uncer;
    string Strike_slip_units;
    string Dip_slip_units;
    string Strike_slip_uncer_units;
    string Dip_slip_uncer_units;

  public:
    FaultSlip(double strike_slip=0.0, double dip_slip=0.0, double strike_slip_uncer=-999.9, double dip_slip_uncer=-999.9,
	string strike_slip_units="m", string dip_slip_units="m", string strike_slip_uncer_units="m",
	string dip_slip_uncer_units="m") :
	    Strike_slip(strike_slip), Dip_slip(dip_slip),
	    Strike_slip_uncer(strike_slip_uncer), Dip_slip_uncer(dip_slip_uncer),
	    Strike_slip_units(strike_slip_units), Dip_slip_units(dip_slip_units),
	    Strike_slip_uncer_units(strike_slip_uncer_units), Dip_slip_uncer_units(dip_slip_uncer_units) {}

    virtual ~FaultSlip() {}

    double setStrikeSlip(double strike_slip) { Strike_slip = strike_slip; return Strike_slip; }
    double setDipSlip(double dip_slip) { Dip_slip = dip_slip; return Dip_slip; }
    double setStrikeSlipUncer(double strike_slip_uncer) { Strike_slip_uncer = strike_slip_uncer; return Strike_slip_uncer; }
    double setDipSlipUncer(double dip_slip_uncer) { Dip_slip_uncer = dip_slip_uncer; return Dip_slip_uncer; }
    string setStrikeSlipUnits(string strike_slip_units) { Strike_slip_units = strike_slip_units; return Strike_slip_units; } 
    string setDipSlipUnits(string dip_slip_units) { Dip_slip_units = dip_slip_units; return Dip_slip_units; } 
    string setStrikeSlipUncerUnits(string strike_slip_uncer_units) {
	Strike_slip_uncer_units = strike_slip_uncer_units; return Strike_slip_uncer_units; } 
    string setDipSlipUncerUnits(string dip_slip_uncer_units) {
	Dip_slip_uncer_units = dip_slip_uncer_units; return Dip_slip_uncer_units; } 

    double getStrikeSlip() const { return Strike_slip; }
    double getDipSlip() const { return Dip_slip; }
    double getStrikeSlipUncer() const { return Strike_slip_uncer; }
    double getDipSlipUncer() const { return Dip_slip_uncer; }
    string getStrikeSlipUnits() const { return Strike_slip_units; }
    string getDipSlipUnits() const { return Dip_slip_units; }
    string getStrikeSlipUncerUnits() const { return Strike_slip_uncer_units; }
    string getDipSlipUncerUnits() const { return Dip_slip_uncer_units; }
};

typedef list<FaultVertex>::iterator FaultVertexIter;


class FaultSegment
{
 public:
    enum FaultSegmentShape {UNKNOWN_SEGMENT, LINE_SEGMENT, TRIANGE_SEGMENT, RECTANGLE_SEGMENT};

 private:
    enum FaultSegmentShape segment_shape;
    list<FaultVertex> vertices;
    FaultSlip fault_slip;
    bool fault_slip_set;

    static const string segmentShapeString[];

 public:
    FaultSegment(enum FaultSegmentShape shape=LINE_SEGMENT) : segment_shape(shape), fault_slip_set(false) {}

    virtual ~FaultSegment() {}

    int addVertex(FaultVertex &v) { vertices.push_back(v); return 1;}
    int addVertex(double latitude, double longitude, double depth,
	string latitude_units = "deg", string longitude_units = "deg", string depth_units = "km");
    void addFaultSlip(FaultSlip slip) { fault_slip = slip; fault_slip_set = true; }
    bool faultSlipSet() { return fault_slip_set; }

    void deleteVertex(FaultVertex &v);
    void deleteVertex(double latitude, double longitude, double depth,
	string latitude_units = "deg", string longitude_units = "deg", string depth_units = "km");

    enum FaultSegmentShape getSegmentShape() { return segment_shape; }
    string getSegmentShapeString() const {
	return segmentShapeString[segment_shape];
    }
    int getNumberVertices() { return vertices.size(); }
    list<FaultVertex>::iterator getVertexIteratorBegin() { return vertices.begin(); }
    list<FaultVertex>::iterator getVertexIteratorEnd() { return vertices.end(); }

    FaultSlip getFaultSlip() { return fault_slip; }

    static string getSegmentShapeString(enum FaultSegmentShape shape) {
	return segmentShapeString[shape];
    }

    enum FaultSegmentShape getSegmentEnum(std::string segment_shape) {
        for (int i=UNKNOWN_SEGMENT; i<=RECTANGLE_SEGMENT; i++) {
   	        FaultSegmentShape fss = static_cast<FaultSegmentShape>(i);  
            if (segment_shape == getSegmentShapeString(fss)) { return fss; }
        }
        return UNKNOWN_SEGMENT;
    }

/*
    virtual bool operator==(const FaultSegment &A) const {
	if( A.segment_shape == this->segment_shape && A.fault_slip == this->fault_slip &&
	    A.fault_slip_set == this->fault_slip_set && A.vertices.size() == this->vertices.size())
	{
	    for(int i = 0; i < (int)vertices.size(); i++) {
		if(A.vertices[i] != this->vertices[i]) return false;
	    }
	    return true;
	}
	return false;
    }

    virtual bool operator!=(const FaultSegment &A) const { return !(*this==A); }
*/

/*
    void logPrint() {
	LOGD << setiosflags(ios::fixed) << setprecision(4);
	LOGD << " lat: " << getLatitude() << " " << getLatitudeUnits();
	LOGD << " lon: " << getLongitude() << " " << getLongitudeUnits() << endl;
    }
*/
};

class GlobalUncertainty
{
  private:
    double lon_trans;
    double lat_trans;
    double total_len;
    double strike;
    double dip;
    string lon_trans_units;
    string lat_trans_units;
    string total_len_units;
    string strike_units;
    string dip_units;
    bool bLon_trans;
    bool bLat_trans;
    bool bTotal_len;
    bool bStrike;
    bool bDip;

  public:
    GlobalUncertainty() : lon_trans(-1.), lat_trans(-1.), total_len(-1.), strike(-999.), dip(-999.),
		lon_trans_units("deg"), lat_trans_units("deg"), total_len_units("km"),
		strike_units("deg"), dip_units("deg"),
        bLon_trans(false), bLat_trans(false), bTotal_len(false), bStrike(false), bDip(false)
        { }

    void setGULonTrans(double in_lon_trans) { lon_trans = in_lon_trans; bLon_trans = true; };
    void setGULatTrans(double in_lat_trans) { lat_trans = in_lat_trans; bLat_trans = true; };
    void setGUTotalLen(double in_total_len) { total_len = in_total_len; bTotal_len = true; };
    void setGUStrike(double in_strike) { strike = in_strike; bStrike = true; };
    void setGUDip(double in_dip) { lon_trans = in_dip; bDip = true; };

    void setGULonTransUnits(string in_lon_trans_units) {lon_trans_units = in_lon_trans_units;};
    void setGULatTransUnits(string in_lat_trans_units) {lat_trans_units = in_lat_trans_units;};
    void setGUTotalLenUnits(string in_total_len_units) {total_len_units = in_total_len_units;};
    void setGUStrikeUnits(string in_strike_units) { strike_units = in_strike_units; };
    void setGUDipUnits(string in_dip_units) { dip_units = in_dip_units; };
     
    double getGULonTrans() { return lon_trans; };
    double getGULatTrans() { return lat_trans; };
    double getGUTotalLen() { return total_len; };
    double getGUStrike() { return strike; };
    double getGUDip() { return dip; };

    string getGULonTransUnits() { return lon_trans_units; };
    string getGULatTransUnits() { return lat_trans_units; };
    string getGUTotalLenUnits() { return total_len_units; };
    string getGUStrikeUnits() { return strike_units; };
    string getGUDipUnits() { return dip_units; };

    bool getGUValidLonTrans() { return bLon_trans; };
    bool getGUValidLatTrans() { return bLat_trans; };
    bool getGUValidTotalLen() { return bTotal_len; };
    bool getGUValidStrike() { return bStrike; };
    bool getGUValidDip() { return bDip; };

};

typedef list<FaultSegment>::iterator FaultSegmentIter;

/** A AlgMessage subclass for FiniteFault event messages.
 *  @ingroup dm_lib
 */
class FiniteFaultMessage : public AlgMessage
{
 private:
    bool Atten_geom;
    enum FaultSegment::FaultSegmentShape Segment_shape;
    double Confidence;
    
    list<FaultSegment> segments; 

    GlobalUncertainty Global_uncertainty;

 public:
    FiniteFaultMessage(enum eewSystemName sys_name = FINITE_FAULT,
          enum FaultSegment::FaultSegmentShape shape = FaultSegment::UNKNOWN_SEGMENT,
		  string id="-9",
		  double mag=-9.9,
		  double mag_uncer=-9.9,
		  double lat=-99.9,
		  double lat_uncer=-99.9,
		  double lon=-999.9,
		  double lon_uncer=-999.9,
		  double dep=-9.9,
		  double dep_uncer=-9.9,
		  double o_time=-99999.99,
		  double o_time_uncer=-9.9,
		  double likelihood=-9.9,
		  enum nudMessageType type = NEW,
		  int ver = 0,
		  enum MessageCategory category = LIVE,
		  string time_stamp = "",
		  string alg_ver = "-",
		  string instance = "",
		  int num_stations = 0,
		  string mag_units = "Mw",
		  string mag_uncer_units = "Mw",
		  string lat_units = "deg",
		  string lat_uncer_units = "deg",
		  string lon_units = "deg",
		  string lon_uncer_units = "deg",
		  string dep_units = "km",
		  string dep_uncer_units = "km",
		  string o_time_units = "UTC",
		  string o_time_uncer_units = "sec"
		  );
    FiniteFaultMessage(enum eewSystemName sys_name = FINITE_FAULT,
          enum FaultSegment::FaultSegmentShape shape = FaultSegment::UNKNOWN_SEGMENT,
		  int id=-9,
		  double mag=-9.9,
		  double mag_uncer=-9.9,
		  double lat=-99.9,
		  double lat_uncer=-99.9,
		  double lon=-999.9,
		  double lon_uncer=-999.9,
		  double dep=-9.9,
		  double dep_uncer=-9.9,
		  double o_time=-99999.99,
		  double o_time_uncer=-9.9,
		  double likelihood=-9.9,
		  enum nudMessageType type = NEW,
		  int ver = 0,
		  enum MessageCategory category = LIVE,
		  string time_stamp = "",
		  string alg_ver = "-",
		  string instance = "",
		  int num_stations = 0,
		  string mag_units = "Mw",
		  string mag_uncer_units = "Mw",
		  string lat_units = "deg",
		  string lat_uncer_units = "deg",
		  string lon_units = "deg",
		  string lon_uncer_units = "deg",
		  string dep_units = "km",
		  string dep_uncer_units = "km",
		  string o_time_units = "UTC",
		  string o_time_uncer_units = "sec"
		  );

    virtual ~FiniteFaultMessage();

    bool getAttenuationGeometry() { return Atten_geom; }
    enum FaultSegment::FaultSegmentShape getSegmentShape() { return Segment_shape; }
    string getSegmentShapeString() const {
	return FaultSegment::getSegmentShapeString(Segment_shape);
    }

    double getConfidence() { return Confidence; }

    bool setAttenuationGeometry(std::string atten_geom) { 
        if (atten_geom == "true") {
            Atten_geom = true; 
        } else {
            Atten_geom = false;
        }
        return Atten_geom; 
    }
    bool setAttenuationGeometry(bool atten_geom) { Atten_geom = atten_geom; return Atten_geom; }
    enum FaultSegment::FaultSegmentShape setSegmentShape(enum FaultSegment::FaultSegmentShape segment_shape) {
	    Segment_shape = segment_shape; 
        return Segment_shape; 
    }
    enum FaultSegment::FaultSegmentShape setSegmentShape(std::string segment_shape) { 
        FaultSegment fs;
        Segment_shape = fs.getSegmentEnum(segment_shape); 
        return Segment_shape;
    }
    
    double setConfidence(double confidence) { Confidence = confidence; return Confidence; }

    int getNumberSegments() { return segments.size(); }
    list<FaultSegment>::iterator getSegmentIteratorBegin() { return segments.begin(); }
    list<FaultSegment>::iterator getSegmentIteratorEnd() { return segments.end(); }

    void clearSegments() { segments.clear(); }
    int addSegment(FaultSegment &s);
    int addSegment(
	double latitude,
	double longitude,
	string latitude_units = "deg",
	string longitude_units = "deg");
/*
    void deleteSegment(FaultSegment &s);
    void deleteSegment(
	double latitude,
	double longitude,
	string latitude_units = "deg",
	string longitude_units = "deg");
*/
    GlobalUncertainty getGlobalUncertainty() { return Global_uncertainty; }
    void setGlobalUncertainty(GlobalUncertainty global_uncertainty) { 
        if (global_uncertainty.getGUValidLonTrans()) {
        Global_uncertainty.setGULonTrans(global_uncertainty.getGULonTrans()); 
        Global_uncertainty.setGULonTransUnits(global_uncertainty.getGULonTransUnits()); 
        }
        if (global_uncertainty.getGUValidLatTrans()) {
        Global_uncertainty.setGULatTrans(global_uncertainty.getGULatTrans()); 
        Global_uncertainty.setGULatTransUnits(global_uncertainty.getGULatTransUnits()); 
        }
        if (global_uncertainty.getGUValidTotalLen()) {
        Global_uncertainty.setGUTotalLen(global_uncertainty.getGUTotalLen()); 
        Global_uncertainty.setGUTotalLenUnits(global_uncertainty.getGUTotalLenUnits()); 
        }
        if (global_uncertainty.getGUValidStrike()) {
        Global_uncertainty.setGUStrike(global_uncertainty.getGUStrike()); 
        Global_uncertainty.setGUStrikeUnits(global_uncertainty.getGUStrikeUnits()); 
        }
        if (global_uncertainty.getGUValidDip()) {
        Global_uncertainty.setGUDip(global_uncertainty.getGUDip()); 
        Global_uncertainty.setGUDipUnits(global_uncertainty.getGUDipUnits()); 
        }
    }

    void setGULonTrans(double in_lon_trans) { Global_uncertainty.setGULonTrans(in_lon_trans); }
    void setGULatTrans(double in_lat_trans) { Global_uncertainty.setGULatTrans(in_lat_trans); }
    void setGUTotalLen(double in_total_len) { Global_uncertainty.setGUTotalLen(in_total_len); }
    void setGUStrike(double in_strike) { Global_uncertainty.setGUStrike(in_strike); }
    void setGUDip(double in_dip) { Global_uncertainty.setGUDip(in_dip); }

    void setGULonTransUnits(string in_lon_trans_units) { Global_uncertainty.setGULonTransUnits(in_lon_trans_units); }
    void setGULatTransUnits(string in_lat_trans_units) { Global_uncertainty.setGULatTransUnits(in_lat_trans_units); }
    void setGUTotalLenUnits(string in_total_len_units) { Global_uncertainty.setGUTotalLenUnits(in_total_len_units); }
    void setGUStrikeUnits(string in_strike_units) { Global_uncertainty.setGUStrikeUnits(in_strike_units); }
    void setGUDipUnits(string in_dip_units) { Global_uncertainty.setGUDipUnits(in_dip_units); }
     
    double getGULonTrans() { return Global_uncertainty.getGULonTrans(); }
    double getGULatTrans() { return Global_uncertainty.getGULatTrans(); }
    double getGUTotalLen() { return Global_uncertainty.getGUTotalLen(); }
    double getGUStrike() { return Global_uncertainty.getGUStrike(); }
    double getGUDip() { return Global_uncertainty.getGUDip(); }

    string getGULonTransUnits() { return Global_uncertainty.getGULonTransUnits(); }
    string getGULatTransUnits() { return Global_uncertainty.getGULatTransUnits(); }
    string getGUTotalLenUnits() { return Global_uncertainty.getGUTotalLenUnits(); }
    string getGUStrikeUnits() { return Global_uncertainty.getGUStrikeUnits(); }
    string getGUDipUnits() { return Global_uncertainty.getGUDipUnits(); }

    bool getGUValidLonTrans() { return Global_uncertainty.getGUValidLonTrans(); }
    bool getGUValidLatTrans() { return Global_uncertainty.getGUValidLatTrans(); }
    bool getGUValidTotalLen() { return Global_uncertainty.getGUValidTotalLen(); }
    bool getGUValidStrike() { return Global_uncertainty.getGUValidStrike(); }
    bool getGUValidDip() { return Global_uncertainty.getGUValidDip(); }


    virtual string updateFrom(const CoreEventInfo &);

/*
    void logPrint() {
	for(FaultSegmentIter it = segments.begin(); it != segments.end(); it++) {
	    it->logPrint();
	}
    }
*/
};

#endif
