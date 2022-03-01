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
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <string>
#include "FinDerMessage.h"
#include "GPSlipMessage.h"
#include "DMMessageEncoder.h"

using namespace xercesc;

DMMessageEncoder::DMMessageEncoder() throw(const XMLException &)
{
    XMLPlatformUtils::Initialize();
}

DMMessageEncoder::~DMMessageEncoder()
{
    XMLPlatformUtils::Terminate();
}

string DMMessageEncoder::encodeMessage(CoreEventInfo *cei, string comment) 
{
    XMLCh tmp[100];
    string message;

    XMLString::transcode("Core", tmp, 99);
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tmp);

    if( impl == NULL) {
	cerr << "encodeMessage: getDOMImplementation failed." << endl;
	return message;
    }

    try {
#ifdef XERCES_2
	DOMWriter *serializer = ((DOMImplementationLS*)impl)->createDOMWriter();
	XMLString::transcode("format-pretty-print", tmp, 99);
	serializer->setFeature(tmp, true);
#else
	DOMLSSerializer   *serializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMConfiguration* serializerConfig = serializer->getDomConfig();

	if (serializerConfig->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
	    serializerConfig->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
	}
#endif

	XMLString::transcode("event_message", tmp, 99);
	DOMDocument* doc = impl->createDocument(0, tmp, 0);
	AlgMessage *alg = (AlgMessage *)cei;
	DMMessage *dm;
	GMMessage *gm;

	encodeCoreMessage(doc, alg, comment);

	switch ( alg->getSystemName() ) {
	case EPIC:
	case ELARMS:
	case ONSITE:
	case VS:
	    encodeGMInfo(doc, *alg);
	    break;
	case GPSLIP:
	case FINDER:
	case BEFORES:
	case FINITE_FAULT:
	    encodeAllFiniteFault(doc, *(FiniteFaultMessage *)alg);
	    encodeGMInfo(doc, *alg);
	    break;
	case DM:
	    gm = (GMMessage *)alg;
	    encodeContributors(doc, gm->getContributorBegin(), gm->getContributorEnd());
	    encodeAllFiniteFault(doc, *gm);
	    encodeGMInfo(doc, *gm);
	    break;
	case SA:
	    dm = (DMMessage *)alg;
	    encodeContributors(doc, dm->getContributorBegin(), dm->getContributorEnd());
	    encodeAllFiniteFault(doc, *dm);
	    encodeGMInfo(doc, *dm);
	    break;
	case EQINFO2GM:
	    gm = (GMMessage *)alg;
	    encodeContributors(doc, gm->getContributorBegin(), gm->getContributorEnd());
	    encodeAllFiniteFault(doc, *gm);
	    encodeGMInfo(doc, *gm);
	    break;
	default:
	    break;
	}

#ifdef XERCES_2
	XMLCh *t = serializer->writeToString(*doc);
#else
	XMLCh *t = serializer->writeToString(doc);
#endif
	char *msg = XMLString::transcode(t);
	message.assign(msg);
	size_t pos = message.find("UTF-16");
	if(pos != string::npos) message.replace(pos, 6, "UTF-8");
	XMLString::release(&msg);
	XMLString::release(&t);
	serializer->release();

	// call release() to release the entire document resources
	doc->release();
    }
    catch (const OutOfMemoryException&)
    {
	cerr << "encodeMessage: OutOfMemoryException" << endl;
    }
    catch (const DOMException& e)
    {
	cerr << "encodeMessage: DOMException code is:  " << e.code << endl;
    }
    catch (...)
    {
	cerr << "encodeMessage: An error occurred creating the document" << endl;
    }

    return message;
}

cms::TextMessage* DMMessageEncoder::encodeMessage(cms::TextMessage *message, CoreEventInfo *cei,
			string comment) 
{
    string msg = encodeMessage(cei, comment);

    try {
	message->setText(msg);
    }
    catch (const OutOfMemoryException&) {
	cerr << "encodeMessage: OutOfMemoryException" << endl;
    }
    return message;
}

void DMMessageEncoder::encodeCoreMessage(DOMDocument *doc, CoreEventInfo* ceip, string comment) 
{
    ostringstream os;
    XMLCh name[100], value[100];
    DOMElement *core_info;
    DOMElement *e = doc->getDocumentElement();

    os << setiosflags(ios::fixed) << setprecision(4);

    XMLString::transcode("orig_sys", name, 99);
    XMLString::transcode(ceip->getSystemNameString().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("message_type", name, 99);
    XMLString::transcode(ceip->getMessageTypeString().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("category", name, 99);
    XMLString::transcode(ceip->getMessageCategoryString().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("timestamp", name, 99);
    XMLString::transcode(ceip->getTimestamp().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("alg_vers", name, 99);
    XMLString::transcode(ceip->getAlgorithmVersion().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("instance", name, 99);
    XMLString::transcode(ceip->getProgramInstance().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("ref_id", name, 99);
    XMLString::transcode(ceip->getReferenceId().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("ref_src", name, 99);
    XMLString::transcode(ceip->getReferenceSrc().c_str(), value, 99);
    e->setAttribute(name, value);

    os.str("");
    os << ceip->getVersion();
    XMLString::transcode("version", name, 99);
    XMLString::transcode(os.str().c_str(), value, 99);
    e->setAttribute(name, value);

    core_info = addNode(doc, e, "core_info", "id", ceip->getID());

    if( !comment.empty() ) {
	XMLCh *xm = XMLString::transcode(comment.c_str());
	DOMComment *c = doc->createComment(xm);
	core_info->appendChild(c);
	XMLString::release(&xm);
    }

    addElement(doc, core_info, "mag", ceip->getMagnitudeUnits(), ceip->getMagnitude());
    addElement(doc, core_info, "mag_uncer", ceip->getMagnitudeUncertaintyUnits(),
		ceip->getMagnitudeUncertainty());
    addElement(doc, core_info, "lat", ceip->getLatitudeUnits(), ceip->getLatitude());
    addElement(doc, core_info, "lat_uncer", ceip->getLatitudeUncertaintyUnits(),
		ceip->getLatitudeUncertainty());
    addElement(doc, core_info, "lon", ceip->getLongitudeUnits(), ceip->getLongitude());
    addElement(doc, core_info, "lon_uncer", ceip->getLongitudeUncertaintyUnits(),
		ceip->getLongitudeUncertainty());
    addElement(doc, core_info, "depth", ceip->getDepthUnits(), ceip->getDepth());
    addElement(doc, core_info, "depth_uncer", ceip->getDepthUncertaintyUnits(),
		ceip->getDepthUncertainty());
    addElement(doc, core_info, "orig_time", ceip->getOriginTimeUnits(), ceip->getOriginTimeString());
    addElement(doc, core_info, "orig_time_uncer", ceip->getOriginTimeUncertaintyUnits(),
		ceip->getOriginTimeUncertainty());
    addElement(doc, core_info, "likelihood", ceip->getLikelyhood());
    addElement(doc, core_info, "num_stations", ceip->getNumberStations());
}

void DMMessageEncoder::encodeContributors(DOMDocument *doc, DMContributorIter beg, DMContributorIter end)
{
    XMLCh name[100];
    DOMElement *contrib;
    DOMElement *e = doc->getDocumentElement();
    int num = 0;
   
    for(DMContributorIter it = beg; it != end; it++) num++;

    if(num == 0) return;

    XMLString::transcode("contributors", name, 99);
    contrib = doc->createElement(name);
    e->appendChild(contrib);

    for(DMContributorIter it = beg; it != end; it++) 
    {
	addContributor(doc, contrib, it);
    }
}

void DMMessageEncoder::addContributor(DOMDocument *doc, DOMElement *parent, DMContributorIter it)
{
    ostringstream os;
    XMLCh name[100], value[100];
    DOMElement *e;

    XMLString::transcode("contributor", name, 99);
    e = doc->createElement(name);
    parent->appendChild(e);

    XMLString::transcode("alg_name", name, 99);
    XMLString::transcode(it->getAlgorithmNameString().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("alg_instance", name, 99);
    XMLString::transcode(it->getAlgorithmInstance().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("alg_version", name, 99);
    XMLString::transcode(it->getAlgorithmVersion().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("event_id", name, 99);
    os.str("");
    os << it->getEventID();
    XMLString::transcode(os.str().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("version", name, 99);
    os.str("");
    os << it->getVersion();
    XMLString::transcode(os.str().c_str(), value, 99);
    e->setAttribute(name, value);

    XMLString::transcode("category", name, 99);
    XMLString::transcode(it->getMessageCategoryString().c_str(), value, 99);
    e->setAttribute(name, value);
}

void DMMessageEncoder::encodeGMInfo(DOMDocument *doc, AlgMessage &alg)
{
    XMLCh name[100];
    DOMElement *info;
    DOMElement *e = doc->getDocumentElement();

    if(alg.getNumberObservations() == 0 && alg.getNumberPredictions() == 0) return;

    XMLString::transcode("gm_info", name, 99);
    info = doc->createElement(name);
    e->appendChild(info);

    if(alg.getNumberObservations() > 0) {
        encodeObservations(doc, info, alg.getGMObservationIteratorBegin(),
		    alg.getGMObservationIteratorEnd());
    }

    if(alg.getNumberPredictions() > 0) {
        encodePredictions(doc, info, alg.getGMPredictionIteratorBegin(),
		    alg.getGMPredictionIteratorEnd());
    }
}

/** Version of encode GMInfo that takes in a GMMessage object and encodes 
 * point observatons, point predictions, GMContours and GMMap.
 * @param doc DOMDocument 
 * @param gm GMMessage object containing ground motion objects to be encoded
 * */
void DMMessageEncoder::encodeGMInfo(DOMDocument *doc, GMMessage &gm)
{
    XMLCh name[100];
    DOMElement *info;
    DOMElement *e = doc->getDocumentElement();

    // Checks for valid gmInfo objects to encode
    if(gm.getNumberObservations() <= 0 
            && gm.getNumberPredictions() <= 0 
            && gm.getNumberGMContPreds() <= 0
            && gm.getNumberGMMapPreds() <= 0) return;

    XMLString::transcode("gm_info", name, 99);
    info = doc->createElement(name);
    e->appendChild(info);

    if(gm.getNumberObservations() > 0) {
        encodeObservations(doc, info, gm.getGMObservationIteratorBegin(),
	    	gm.getGMObservationIteratorEnd());
    }

    if(gm.getNumberPredictions() > 0) {
        encodePredictions(doc, info, gm.getGMPredictionIteratorBegin(),
		    gm.getGMPredictionIteratorEnd());
    }

    if(gm.getNumberGMContPreds() > 0) {
        encodeGMContours(doc, info, gm.getMultiContourIteratorBegin(), 
            gm.getMultiContourIteratorEnd(), gm.getNumberGMContPreds());
    }

    if(gm.getNumberGMMapPreds() > 0) {
        encodeGMMap(doc, info, gm.getGMMap());
    }
}

/** Encodes the GMContour objects as XML. Loops over vector of GMContours.
 * @param doc DOMDocument 
 * @param info DOMElement to which node will be added
 * @param beg MultiContourIter object pointing to head of vector containing
 * GMContour objects
 * @param end MultiContourIter object pointing to end of vector containing
 * GMContour objects
 * */
void DMMessageEncoder::encodeGMContours(DOMDocument *doc, xercesc::DOMElement *info,
        MultiContourIter beg, MultiContourIter end, int nCont) {
    DOMElement *e, *eo;
    ostringstream os;

    os << nCont;
    eo = addNode(doc, info, "gmcontour_pred", "number", os.str());
    for (MultiContourIter it = beg; it != end; it++) {
        os.str(std::string());
        e = addNode(doc, eo, "contour");
        addElement(doc, e, "MMI", it->getMMIunits(), it->getMMI());
        addElement(doc, e, "PGA", it->getPGAunits(), it->getPGA());
        addElement(doc, e, "PGV", it->getPGVunits(), it->getPGV());
	vector<double> lats = it->getLats();
	vector<double> lons = it->getLons();
	if(lats.front() == lats.back() && lons.front() == lons.back()) {
	    lats.pop_back();
	    lons.pop_back();
	    os << it->getNumberContourVertices() - 1;
	}
	else {
	    os << it->getNumberContourVertices();
	}
//        addElement(doc, e, "polygon", "number", os.str(), it->getLats(), it->getLons());
        addElement(doc, e, "polygon", "number", os.str(), lats, lons);
    }
}

/** Encodes the GMMap object as XML
 * @param doc DOMDocument 
 * @param info DOMElement to which node will be added
 * @param gmmap GMMap to be encoded
 * */
void DMMessageEncoder::encodeGMMap(DOMDocument *doc, xercesc::DOMElement *info,
        GMMap gmmap) {
    DOMElement *e;

    if (gmmap.getNumGridPts() <= 0) return;

    ostringstream ns; // names
    ostringstream vsln, vslt, vspga, vspgv, vsmmi; // values
    ns.str("");
    ns << "index name units";

    e = addNode(doc, info, "gmmap_pred", "number", gmmap.getNumGridPts());
    vsln << gmmap.getLonIndex() << " LON " << gmmap.getLonUnits();
    addElement(doc, e, "grid_field", ns.str(), vsln.str(), "");
    vslt << gmmap.getLatIndex() << " LAT " << gmmap.getLatUnits();
    addElement(doc, e, "grid_field", ns.str(), vslt.str(), "");
    vspga << gmmap.getPGAIndex() << " PGA " << gmmap.getPGAunits();
    addElement(doc, e, "grid_field", ns.str(), vspga.str(), "");
    vspgv << gmmap.getPGVIndex() << " PGV " << gmmap.getPGVunits();
    addElement(doc, e, "grid_field", ns.str(), vspgv.str(), "");
    vsmmi << gmmap.getMMIIndex() << " MMI " << gmmap.getMMIunits();
    addElement(doc, e, "grid_field", ns.str(), vsmmi.str(), "");
    addElement(doc, e, "grid_data", gmmap.getMapData());
}


void DMMessageEncoder::encodeObservations(DOMDocument *doc, DOMElement *hinfo,
				GMObservationIter beg, GMObservationIter end)
{
    DOMElement *e;
    int num;

    XMLCh name[100];
    DOMElement *info;
    XMLString::transcode("gmpoint_obs", name, 99);
    info = doc->createElement(name);
    hinfo->appendChild(info);
 
    num = 0;
    for(GMObservationIter it = beg; it != end; it++) {
	if((*it).getObservationType() == DISPLACEMENT_OBS) num++;
    }
    if(num > 0) {
	e = addNode(doc, info, "pgd_obs", "number", num);
	encodeObservationNode(doc, e, DISPLACEMENT_OBS, beg, end);
    }

    num = 0;
    for(GMObservationIter it = beg; it != end; it++) {
	if((*it).getObservationType() == VELOCITY_OBS) num++;
    }
    if(num > 0) {
	e = addNode(doc, info, "pgv_obs", "number", num);
	encodeObservationNode(doc, e, VELOCITY_OBS, beg, end);
    }

    num = 0;
    for(GMObservationIter it = beg; it != end; it++) {
	if((*it).getObservationType() == ACCELERATION_OBS) num++;
    }
    if(num > 0) {
	e = addNode(doc, info, "pga_obs", "number", num);
	encodeObservationNode(doc, e, ACCELERATION_OBS, beg, end);
    }
}

void DMMessageEncoder::encodeObservationNode(DOMDocument *doc, DOMElement *parent,
			enum ObservationType type, GMObservationIter beg, GMObservationIter end)
{
    DOMElement *e;

    for(GMObservationIter it = beg; it != end; it++) {
	if(it->getObservationType() == type) {
	    e = addObsOrPred(doc, "obs", it->getOrigSys());
	    parent->appendChild(e);
	    addElement(doc, e, "SNCL", it->getSNCL());
	    addElement(doc, e, "value", it->getObservationUnits(), it->getObservation());
	    addElement(doc, e, "lat", it->getLatitudeUnits(), it->getLatitude());
	    addElement(doc, e, "lon", it->getLongitudeUnits(), it->getLongitude());
	    addElement(doc, e, "time", it->getObservationTimeUnits(), it->getObservationTimeString());
	}
    }
}

void DMMessageEncoder::encodePredictions(DOMDocument *doc, DOMElement *hinfo,
				GMPredictionIter beg, GMPredictionIter end)
{
    DOMElement *e;
    int num;

    XMLCh name[100];
    DOMElement *info;
    XMLString::transcode("gmpoint_pred", name, 99);
    info = doc->createElement(name);
    hinfo->appendChild(info);

    num = 0;
    for(GMPredictionIter it = beg; it != end; it++) {
	if((*it).getPredictionType() == DISPLACEMENT_PRED) num++;
    }
    if(num > 0) {
	e = addNode(doc, info, "pgd_pred", "number", num);
	encodePredictionNode(doc, e, DISPLACEMENT_PRED, beg, end);
    }

    num = 0;
    for(GMPredictionIter it = beg; it != end; it++) {
	if((*it).getPredictionType() == VELOCITY_PRED) num++;
    }
    if(num > 0) {
	e = addNode(doc, info, "pgv_pred", "number", num);
	encodePredictionNode(doc, e, VELOCITY_PRED, beg, end);
    }

    num = 0;
    for(GMPredictionIter it = beg; it != end; it++) {
	if((*it).getPredictionType() == ACCELERATION_PRED) num++;
    }
    if(num > 0) {
	e = addNode(doc, info, "pga_pred", "number", num);
	encodePredictionNode(doc, e, ACCELERATION_PRED, beg, end);
    }
}

void DMMessageEncoder::encodePredictionNode(DOMDocument *doc, DOMElement *parent,
			enum PredictionType type, GMPredictionIter beg, GMPredictionIter end)
{
    DOMElement *e;

    for(GMPredictionIter it = beg; it != end; it++) {
	if(it->getPredictionType() == type) {
	    e = addObsOrPred(doc, "pred", it->getOrigSys());
	    parent->appendChild(e);
	    addElement(doc, e, "SNCL", it->getSNCL());
	    addElement(doc, e, "value", it->getPredictionUnits(), it->getPrediction());
	    addElement(doc, e, "value_uncer", it->getPredictionUncertaintyUnits(),
					it->getPredictionUncertainty());
	    addElement(doc, e, "lat", it->getLatitudeUnits(), it->getLatitude());
	    addElement(doc, e, "lon", it->getLongitudeUnits(), it->getLongitude());
	    addElement(doc, e, "app_rad", it->getAppliedRadiusUnits(), it->getAppliedRadius());
	    addElement(doc, e, "time", it->getPredictionTimeUnits(), it->getPredictionTimeString());
	    addElement(doc, e, "time_uncer", it->getPredictionTimeUncertaintyUnits(),
					it->getPredictionTimeUncertainty());
	}
    }
}

void DMMessageEncoder::encodeAllFiniteFault(DOMDocument *doc, FiniteFaultMessage &ff) {
    XMLCh name[100];
    DOMElement *fault_info;
    DOMElement *e = doc->getDocumentElement();

    if (ff.getNumberSegments() == 0) { return; }

    XMLString::transcode("fault_info", name, 99);
    fault_info = doc->createElement(name);
    e->appendChild(fault_info);

    encodeFiniteFault(doc, fault_info, ff);
}

void DMMessageEncoder::encodeAllFiniteFault(DOMDocument *doc, DMMessage &dm) {
    XMLCh name[100];
    DOMElement *fault_info;
    DOMElement *e = doc->getDocumentElement();

    if (dm.getNumberFiniteFaults() == 0) { return; } 

    XMLString::transcode("fault_info", name, 99);
    fault_info = doc->createElement(name);
    e->appendChild(fault_info);

    for(FFMsgIter it = dm.getFiniteFaultIteratorBegin();
        it != dm.getFiniteFaultIteratorEnd(); it++) {
        encodeFiniteFault(doc, fault_info, (*it));
    }
}

void DMMessageEncoder::encodeFiniteFault(DOMDocument *doc, DOMElement *parent, FiniteFaultMessage &finite)
{
    XMLCh name[100], value[100];
    DOMElement *finite_element;

    XMLString::transcode("finite_fault", name, 99);
    finite_element = doc->createElement(name);
    parent->appendChild(finite_element);

    XMLString::transcode("atten_geom", name, 99);
    const char *b = finite.getAttenuationGeometry() ? "true" : "false";
    XMLString::transcode(b, value, 99);
    finite_element->setAttribute(name, value);

    XMLString::transcode("segment_shape", name, 99);
    XMLString::transcode(finite.getSegmentShapeString().c_str(), value, 99);
    finite_element->setAttribute(name, value);

    stringstream ss;
    ss << finite.getNumberSegments();
    XMLString::transcode("segment_number", name, 99);
    XMLString::transcode(ss.str().c_str(), value, 99);
    finite_element->setAttribute(name, value);

    // encode confidence
    if (finite.getConfidence() > 0.) {
        addElement(doc, finite_element, "confidence", finite.getConfidence()); 
    }

    // encode segments
    for(list<FaultSegment>::iterator it = finite.getSegmentIteratorBegin(); it != finite.getSegmentIteratorEnd(); it++) {
	encodeFaultSegment(doc, finite_element, *it);
    }

    encodeGlobalUncertainty(doc, finite_element, finite.getGlobalUncertainty());
}

void DMMessageEncoder::encodeGlobalUncertainty(DOMDocument *doc, DOMElement *parent, GlobalUncertainty g)
{
    XMLCh name[100];
    DOMElement *e;

    if (!g.getGUValidLonTrans() && !g.getGUValidLatTrans() && !g.getGUValidTotalLen() && 
            !g.getGUValidStrike() && !g.getGUValidDip()) { return; }

    XMLString::transcode("global_uncertainty", name, 99);
    e = doc->createElement(name);
    parent->appendChild(e);
    if (g.getGUValidLonTrans()) { 
        addElement(doc, e, "lon_trans", g.getGULonTransUnits(), g.getGULonTrans()); 
    }
    if (g.getGUValidLatTrans()) { 
        addElement(doc, e, "lat_trans", g.getGULatTransUnits(), g.getGULatTrans());
    }
    if (g.getGUValidTotalLen()) { 
        addElement(doc, e, "total_len", g.getGUTotalLenUnits(), g.getGUTotalLen());
    }
    if (g.getGUValidLonTrans()) { 
        addElement(doc, e, "strike", g.getGUStrikeUnits(), g.getGUStrike());
    }
    if (g.getGUValidLonTrans()) { 
        addElement(doc, e, "dip", g.getGUDipUnits(), g.getGUDip());
    }
}

void DMMessageEncoder::encodeFaultSegment(DOMDocument *doc, DOMElement *parent, FaultSegment &segment)
{
    XMLCh name[100];
    DOMElement *seg, *vertices, *slip, *vertex;
    
    XMLString::transcode("segment", name, 99);
    seg = doc->createElement(name);
    parent->appendChild(seg);

    XMLString::transcode("vertices", name, 99);
    vertices = doc->createElement(name);
    seg->appendChild(vertices);

    for(list<FaultVertex>::iterator it = segment.getVertexIteratorBegin(); it != segment.getVertexIteratorEnd(); it++) {
	XMLString::transcode("vertex", name, 99);
	vertex = doc->createElement(name);
	vertices->appendChild(vertex);
	addElement(doc, vertex, "lat", (*it).getLatitudeUnits(), (*it).getLatitude());
	addElement(doc, vertex, "lon", (*it).getLongitudeUnits(), (*it).getLongitude());
	addElement(doc, vertex, "depth", (*it).getDepthUnits(), (*it).getDepth());
    }

    if(segment.faultSlipSet()) {
	FaultSlip fault_slip = segment.getFaultSlip();
	XMLString::transcode("slip", name, 99);
	slip = doc->createElement(name);
	seg->appendChild(slip);

	addElement(doc, slip, "ss", fault_slip.getStrikeSlipUnits(), fault_slip.getStrikeSlip());
	addElement(doc, slip, "ss_uncer", fault_slip.getStrikeSlipUncerUnits(), fault_slip.getStrikeSlipUncer());
	addElement(doc, slip, "ds", fault_slip.getDipSlipUnits(), fault_slip.getDipSlip());
	addElement(doc, slip, "ds_uncer", fault_slip.getDipSlipUncerUnits(), fault_slip.getDipSlipUncer());
    }
}

void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
				string units, int value)
{
    ostringstream os;
    XMLCh xm1[100], xm2[100];
    DOMElement *e;

    XMLString::transcode(name, xm1, 99);
    e = doc->createElement(xm1);
    parent->appendChild(e);
    XMLString::transcode("units", xm1, 99);
    XMLString::transcode(units.c_str(), xm2, 99);
    e->setAttribute(xm1, xm2);
    os.str("");
    os << value;
    XMLString::transcode(os.str().c_str(), xm1, 99);
    e->setTextContent(xm1);
}

void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
				string units, double value)
{
    ostringstream os;
    XMLCh xm1[100], xm2[100];
    DOMElement *e;

    XMLString::transcode(name, xm1, 99);
    e = doc->createElement(xm1);
    parent->appendChild(e);
    XMLString::transcode("units", xm1, 99);
    XMLString::transcode(units.c_str(), xm2, 99);
    e->setAttribute(xm1, xm2);
    os.str("");
    os << setiosflags(ios::fixed) << setprecision(4);
    os << value;
    XMLString::transcode(os.str().c_str(), xm1, 99);
    e->setTextContent(xm1);
}

void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
				string units, string value)
{
    XMLCh xm1[100], xm2[100];
    DOMElement *e;

    XMLString::transcode(name, xm1, 99);
    e = doc->createElement(xm1);
    parent->appendChild(e);
    XMLString::transcode("units", xm1, 99);
    XMLString::transcode(units.c_str(), xm2, 99);
    e->setAttribute(xm1, xm2);
    XMLString::transcode(value.c_str(), xm1, 99);
    e->setTextContent(xm1);
}

void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
				double value)
{
    ostringstream os;
    XMLCh xm[100];
    DOMElement *e;

    XMLString::transcode(name, xm, 99);
    e = doc->createElement(xm);
    parent->appendChild(e);
    os.str("");
    os << setiosflags(ios::fixed) << setprecision(4);
    os << value;
    XMLString::transcode(os.str().c_str(), xm, 99);
    e->setTextContent(xm);
}

void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
				int value)
{
    ostringstream os;
    XMLCh xm[100];
    DOMElement *e;

    XMLString::transcode(name, xm, 99);
    e = doc->createElement(xm);
    parent->appendChild(e);
    os.str("");
    os << value;
    XMLString::transcode(os.str().c_str(), xm, 99);
    e->setTextContent(xm);
}

void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
				string value)
{
    XMLCh xm[100];
    DOMElement *e;

    XMLString::transcode(name, xm, 99);
    e = doc->createElement(xm);
    parent->appendChild(e);
    XMLString::transcode(value.c_str(), xm, 99);
    e->setTextContent(xm);
}

/** addElement where two vectors of equal length are encoded as space-separated
 * pairs, with the items of the pair being comma-separated. These form the value
 * of the encoded element. Note the head pair is duplicated as the tail, which
 * should be reconsidered.
 * @param doc DOMDocument
 * @param parent DOMElement to which children will be appended
 * @param name const char of the name of the element
 * @param attnames std::string of attribute names
 * @param attvalues std::string of attribute values
 * @param x std::vector<double> of first variable
 * @param y std::vector<double> of second variable
  */
void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
				std::string attnames, std::string attvalues,
                std::vector<double> x, std::vector<double> y)
{
    ostringstream os;
    XMLCh xm[1000], xm1[1000], xm2[1000];
    DOMElement *e;
    stringstream ns(attnames);
    stringstream vs(attvalues);
    std::string attname;
    std::string attval;
    std::vector<double>::iterator itx = x.begin();
    std::vector<double>::iterator ity = y.begin();

    XMLString::transcode(name, xm, 999);
    e = doc->createElement(xm);
    parent->appendChild(e);
    for (; ns >> attname;) {
        XMLString::transcode(attname.c_str(), xm1, 999);
        if (vs >> attval) { 
            XMLString::transcode(attval.c_str(), xm2, 999);
        } else {
            XMLString::transcode("", xm2, 999);
        }
        e->setAttribute(xm1, xm2);
    }
    os.str("");
    os << setiosflags(ios::fixed) << setprecision(4);
    for (; itx != x.end(); itx++, ity++) {
        os << *itx << "," << *ity << " ";
    }
    os << x.front() << "," << y.front(); // replicate first element
    XMLString::transcode(os.str().c_str(), xm, 999);
    e->setTextContent(xm);
}

/** addElement with multiple attributes and string value.
 * @param doc DOMDocument
 * @param parent DOMElement to which children will be appended
 * @param name const char of the name of the element
 * @param attnames std::string of attribute names
 * @param attvalues std::string of attribute values
 * @param value std::string of element value
  */
void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name,
        std::string attnames, std::string attvalues, std::string value) 
{
    XMLCh xm1[100], xm2[100];
    DOMElement *e;
    stringstream ns(attnames);
    stringstream vs(attvalues);
    std::string attname;
    std::string attval;

    XMLString::transcode(name, xm1, 99);
    e = doc->createElement(xm1);
    parent->appendChild(e);
    for (; ns >> attname;) {
        XMLString::transcode(attname.c_str(), xm1, 99);
        if (vs >> attval) { 
            XMLString::transcode(attval.c_str(), xm2, 99);
        } else {
            XMLString::transcode("", xm2, 99);
        }
        e->setAttribute(xm1, xm2);
    }
    if (!value.empty()) {
        XMLString::transcode(value.c_str(), xm1, 99);
    }
}

/** addElement with grid data defined in vector of GM_INFO structs. The element
 * is encoded as space-delimited grid fields, with one line per grid point
 * @param doc DOMDocument
 * @param parent DOMElement to which children will be appended
 * @param name const char of the name of the element
 * @param griddata std::vector<gminfo::GM_INFO> element value
  */
void DMMessageEncoder::addElement(DOMDocument *doc, DOMElement *parent, const char *name, 
        std::vector<gminfo::GM_INFO> griddata) 
{
    ostringstream os;
    XMLCh xm[1000000];
    DOMElement *e;

    XMLString::transcode(name, xm, 999999);
    e = doc->createElement(xm);
    parent->appendChild(e);
    os.str("");
    os << setiosflags(ios::fixed);
    for (size_t n = 0; n < griddata.size(); n++) {
        gminfo::GM_INFO pGM = griddata.at(n);
        if (n == 0) {
            os << "\n";
        }
        os << std::setprecision(4) << pGM.lonGM << " " << pGM.latGM << " " 
            << std::setprecision(6) << pGM.PGA << " " << pGM.PGV << " " 
            << std::setprecision(2) << pGM.SI << "\n";
    }
    XMLString::transcode(os.str().c_str(), xm, 999999); // set max chars more sensibly!?
    e->setTextContent(xm);
}

DOMElement * DMMessageEncoder::addNode(DOMDocument *doc, DOMElement *parent, const char *name)
{
    XMLCh *xm;
    DOMElement *e;

    xm = XMLString::transcode(name);
    e = doc->createElement(xm);
    XMLString::release(&xm);
    parent->appendChild(e);
    return e;
}

DOMElement * DMMessageEncoder::addNode(DOMDocument *doc, DOMElement *parent, const char *name,
				const char *attribute, int value)
{
    ostringstream os;
    XMLCh *xm, *xm2;
    DOMElement *e;

    xm = XMLString::transcode(name);
    e = doc->createElement(xm);
    XMLString::release(&xm);
    parent->appendChild(e);
    os.str("");
    os << setiosflags(ios::fixed) << setprecision(4);
    os << value;
    xm = XMLString::transcode(attribute);
    xm2 = XMLString::transcode(os.str().c_str());
    e->setAttribute(xm, xm2);
    XMLString::release(&xm);
    XMLString::release(&xm2);
    return e;
}

DOMElement * DMMessageEncoder::addNode(DOMDocument *doc, DOMElement *parent, const char *name,
				const char *attribute, string value)
{
    ostringstream os;
    XMLCh *xm, *xm2;
    DOMElement *e;

    xm = XMLString::transcode(name);
    e = doc->createElement(xm);
    XMLString::release(&xm);
    parent->appendChild(e);
    xm = XMLString::transcode(attribute);
    xm2 = XMLString::transcode(value.c_str());
    e->setAttribute(xm, xm2);
    XMLString::release(&xm);
    XMLString::release(&xm2);
    return e;
}

DOMElement * DMMessageEncoder::addObsOrPred(DOMDocument *doc, const char *obs_or_pred,
					const string &sys_name)
{
    XMLCh *xm, *xm2;
    XMLCh name[100];
    DOMElement *e;

    XMLString::transcode(obs_or_pred, name, 99);
    e = doc->createElement(name);
//    parent->appendChild(e);
    if(sys_name.length() > 0) {
	xm = XMLString::transcode("orig_sys");
	xm2 = XMLString::transcode(sys_name.c_str());
	e->setAttribute(xm, xm2);
	XMLString::release(&xm);
	XMLString::release(&xm2);
    }
    return e;
}
