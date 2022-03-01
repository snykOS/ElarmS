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
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

#include "plog/Log.h"           // plog logging library

#include "DMMessageDecoder.h"
#include "EpicMessage.h"
#include "ElarmsMessage.h"
#include "OnSiteMessage.h"
#include "VSMessage.h"
#include "DMReviewMessage.h"
#include "FinDerMessage.h"
#include "GPSlipMessage.h"
#include "qlib2.h"

static char *stringTrim(char *str);
static bool stringToInt(string &s, int *value);

using namespace xercesc;

DMMessageDecoder::DMMessageDecoder() throw(const XMLException &) :
	do_namespaces(false), do_schema(false), schema_full_checking(false), do_create(false)
{
    XMLPlatformUtils::Initialize();

    parser = new XercesDOMParser();
    error_handler = new DefaultHandler();
    parser->setErrorHandler(error_handler);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(do_namespaces);
    parser->setDoSchema(do_schema);
//    parser->setHandleMultipleImports (true);
    parser->setValidationSchemaFullChecking(schema_full_checking);
    parser->setCreateEntityReferenceNodes(do_create);

    input_source = new MemBufInputSource((const XMLByte *)NULL, 0, "DMMessageDecoder");
}

DMMessageDecoder::~DMMessageDecoder()
{
    close();
}

void DMMessageDecoder::close()
{
    if(parser) {
	delete parser;
	parser = NULL;
	delete input_source;
	input_source = NULL;
	delete error_handler;
	error_handler = NULL;
	XMLPlatformUtils::Terminate();
    }
}

int DMMessageDecoder::decodeAllLogMessages(FILE *fp, vector<CoreEventInfo* > &v, bool new_style)
{
    int n = (int)v.size();
    CoreEventInfo *cei;

    while( (cei = decodeNextLogMessage(fp, NULL, new_style)) ) {
	// append to v
	v.push_back(cei);
    }
    // return number of CoreEventInfo pointers appended to v
    return (int)v.size() - n;
}

CoreEventInfo* DMMessageDecoder::decodeNextLogMessage(FILE *fp, CoreEventInfo* cei, bool new_style)
{
    char *c, line[1000], *s;
    string message, log_time;
    CoreEventInfo *core = NULL;

    while(!core) {
	message.clear();
	if(new_style) {
	    // read until next log time stamp line "2013-02-15T01:16:56.042Z"
	    //                                      012345678901234567890123
	    int n = (int)strlen("2013-02-15T01:16:56.042Z");
	    while((c = fgets(line, sizeof(line), fp))) {
		if((s = strstr(line, "<event_message")) || (s = strstr(line, "sysname:")))
		{
		    break;
		}
		if((int)strlen(c) >= n && c[4] == '-' && c[7] == '-' && c[10] == 'T'
			&& c[13] == ':' && c[16] == ':' && c[23] == 'Z')
		{
		    if(c[(int)strlen(c)-1] == '\n') c[(int)strlen(c)-1] = '\0';
		    c[4] = '/';
		    c[7] = '/';
		    c[10] = ' ';
		    c[23] = '\0';
		    log_time.assign(c);
		}
	    }
	}
	else {
	    // read until the next "<event_message" node is found or, for old style file format,
	    // until "CoreEventInfo Start:" is found.
	    // Some old logfiles have the same event in both old and new formats. This routine will
	    // return duplicate events in that case.
	    while((c = fgets(line, sizeof(line), fp)) && !(s = strstr(line, "<event_message"))
			&& !(s = strstr(line, "sysname:")));
	}
	if(c) {
	    if(!strncmp(s, "<event_message", 14)) {
		message.append(line);
		// read until the "</event_message>" is found
		while((c = fgets(line, sizeof(line), fp))  && !strstr(line, "</event_message>")) {
		    message.append(line);
		}
		if(c) message.append(line);
		if(new_style) {
		    core = decodeMessage(message, cei);
		    if(core) core->setLogTime(log_time);
		}
		else {
		    core = decodeLogMessage(message, cei); // can return NULL if message does not parse
		}
	    }
	    else if(!strncmp(s, "sysname:", 8)) {
		message.append(line);
		// read until the "CoreEventInfo End:" is found
		while((c = fgets(line, sizeof(line), fp))  && !strstr(line, "CoreEventInfo End:")) {
		    message.append(line);
		}
		if(c) message.append(line);
		core = decodeOldLogMessage(message); // can return NULL if message does not parse
	    }
	}
	else { // end of file
	    return NULL;
	}
    }
    return core;
}

CoreEventInfo* DMMessageDecoder::decodeLogMessage(string message, CoreEventInfo* cei)
{
    string s, log_time;
    const char *c = message.c_str();
    const char *eol = c-1;
    
    // remove the time stamps from beginning of each line
    // can have "03:18:18:561|" or "14:52:03:1000"

    for(c =  message.c_str(); *c != '\0'; c++) {
	if(*c == '\n') eol = c; // remember position of last '\n'
	if(*c == '|' && (c-eol == 13 || c-eol == 14)) {
	    if(log_time.empty()) {
		// save the time stamp on the first line
		log_time.assign(eol+1, c-eol-1);
	    }
	    // store up to and including the end-of-line
	    do {
		c++;
		s.append(c, 1);
	    } while(*c != '\n' && *c != '\0');
	    if(*c == '\n') eol = c;
	}
    }

    // fix a format error in the old log files. The 'core_info' tag is not terminated
    // with '>'. This causes a SAXException. For example:
    // <core_info id="2891" <!-- contributors:  [vs,10008335,29] -->
    //		<mag units="Mw"> 0.8208 </mag>
    //		...
    // </core_info>

    for(string::iterator it = s.begin(); it != s.end(); it++) {
	if( !s.compare(it-s.begin(), 10, "<core_info") ) {
	    it++;
	    while(it != s.end() && *it != '>' && *it != '<') it++;
	    if(it != s.end() && *it == '<') {
		it = s.insert(it, '>');
	    }
	}
    }

    CoreEventInfo *core = decodeMessage(s, cei);
    if(core) core->setLogTime(log_time);
    return core;
}

CoreEventInfo* DMMessageDecoder::decodeMessage(string s, CoreEventInfo* cei, bool print) 
{
    try {
	input_source->resetMemBufInputSource((const XMLByte *)s.data(),
			(XMLSize_t)(s.length()*sizeof(char)));
    }
    catch(const XMLException& e) {
	char* msg = XMLString::transcode(e.getMessage());
	cerr << "Exception message is:" << endl << msg << endl;
	XMLString::release(&msg);
	return NULL;
    }
    catch(const DOMException& e) {
	char* msg = XMLString::transcode(e.msg);
	cerr << "Exception message is:" << endl << msg << endl;
	XMLString::release(&msg);
	return NULL;
    }
    catch(...) {
	cerr << "Unexpected Exception \n" ;
	return NULL;
    }

    try {
	parser->parse(*input_source);
    }
    catch(const DOMException& e) {
	char* msg = XMLString::transcode(e.msg);
	cerr << "DOMException message is:" << endl << msg << endl;
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return NULL;
    }
    catch(const XMLException& e) {
	char* msg = XMLString::transcode(e.getMessage());
	cerr << "XMLException message is:" << endl << msg << endl;
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return NULL;
    }
    catch(const SAXException& e) {
	char* msg = XMLString::transcode(e.getMessage());
	cerr << "SAXException message is:" << endl << msg << endl;
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return NULL;
    }
    catch (...) {
	cerr << "DMMessageDecoder parse failed" << endl;
	parser->resetDocumentPool();
	return NULL;
    }

    DOMDocument *doc = parser->getDocument();

    XMLCh *tag = XMLString::transcode("event_message");
    DOMNodeList *event_list = doc->getElementsByTagName(tag);
    XMLString::release(&tag);

    // could have more than one event_message. list->getLength() > 1, but just do one for now.

    for(XMLSize_t i = 0; i < event_list->getLength() && i < 1; i++) {
	DOMNode *event_node = event_list->item(i);
	if(event_node->getNodeType() == DOMNode::ELEMENT_NODE)
	{
	    CoreEventInfo *core = decodeEvent(event_node, cei, print);
	    parser->resetDocumentPool();
	    return core;
	}
    }
    parser->resetDocumentPool();

    return NULL;
}

CoreEventInfo* DMMessageDecoder::decodeEvent(DOMNode *event_node, CoreEventInfo *cei, bool print) 
{
    string sys_name, msg_type, category, time_stamp, alg_ver, instance, ref_src, ref_id;
    enum eewSystemName system_name;
    enum nudMessageType type;
    enum MessageCategory message_category;
    int version=0;
    DOMElement *element;
    DOMNodeList *event_children;

    element = dynamic_cast< DOMElement* >(event_node);

    if( !getAttributeValue(element, "orig_sys", sys_name) ) {
	// error
	cerr << "DMMessageDecoder::decodeEvent: no 'orig_sys'" << endl;
	return NULL;
    }
    if( !getAttributeValue(element, "message_type", msg_type) ) {
	// error
	cerr << "DMMessageDecoder::decodeEvent: no 'message_type'" << endl;
	return NULL;
    }
    if( !getAttributeValue(element, "category", category) ) {
	// not an error for now. default to "live"
	category.assign("live");
//	return NULL;
    }
    if( !getAttributeValue(element, "version", &version) ) {
	// error
	cerr << "DMMessageDecoder::decodeEvent: no 'version'" << endl;
	return NULL;
    }
    if( !getAttributeValue(element, "timestamp", time_stamp) ) {
	// not an error for now.
	time_stamp.assign("");
    }
    if( !getAttributeValue(element, "alg_vers", alg_ver) ) {
	// not an error for now.
	alg_ver.assign("");
    }
    if( !getAttributeValue(element, "instance", instance) ) {
	// not an error for now.
	instance.assign("");
    }
    if( !getAttributeValue(element, "ref_src", ref_src) ) {
	// not an error for now.
	ref_src.assign("");
    }
    if( !getAttributeValue(element, "ref_id", ref_id) ) {
	// not an error for now.
	ref_id = "0";
    }
    bool valid_sys_name = false;
    for(int i = 0; i < (int)(sizeof(eewSystemNameString)/sizeof(string)); i++) {
	if(!strcasecmp(sys_name.c_str(), eewSystemNameString[i].c_str())) {
	    system_name = CoreEventInfo::eewSystemNameArray[i];
	    valid_sys_name = true;
	}
    }
    if(!valid_sys_name) {
//	return NULL;
	// assume FINITE_FAULT. Must find finite_fault block below.
	system_name = FINITE_FAULT;
    }

    bool valid_type = false;
    for(int i = 0; i < (int)(sizeof(MessageTypeString)/sizeof(string)); i++) {
	if(!strcasecmp(msg_type.c_str(), MessageTypeString[i].c_str())) {
	    type = CoreEventInfo::nudMessageTypeArray[i];
	    valid_type = true;
	}
    }
    if(!valid_type) {
	cerr << "DMMessageDecoder::decodeEvent: invalid message_type." << msg_type << endl;
	return NULL;
    }

    bool valid_category = false;
    for(int i = 0; i < (int)(sizeof(MessageCategoryString)/sizeof(string)); i++) {
	if(!strcasecmp(category.c_str(), MessageCategoryString[i].c_str())) {
	    message_category = CoreEventInfo::MessageCategoryArray[i];
	    valid_category = true;
	}
    }
    if(!valid_category) {
	cerr << "DMMessageDecoder::decodeEvent: invalid category." << category << endl;
	return NULL;
    }

    if( !(event_children = event_node->getChildNodes()) ) {
	// missing core_info and elarms, or onsite, etc. nodes.
	cerr << "DMMessageDecoder::decodeEvent: missing core_info and elarms, or onsite, etc. nodes" << endl;
	return NULL;
    }

    // gather information from all "core_info" nodes. There can be more than one.

    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
	DOMNode *event_child = event_children->item(i);
	if(nodeNameIs(event_child, "core_info") &&
		event_child->getNodeType() == DOMNode::ELEMENT_NODE)
	{
	    switch (system_name) {
	    case EPIC:
		if(cei == NULL) cei = new EpicMessage("0");
		decodeCoreMessage(event_child, cei);
	    break;
	    case ELARMS:
		if(cei == NULL) cei = new ElarmsMessage("0");
		decodeCoreMessage(event_child, cei);
	    break;
	    case ONSITE:
		if(cei == NULL) cei = new OnSiteMessage("0");
		decodeCoreMessage(event_child, cei);
		break;
	    case VS:
		if(cei == NULL) cei = new VSMessage("0");
		decodeCoreMessage(event_child, cei);
		break;
	    case FINDER:
		if(cei == NULL) cei = new FinDerMessage(FaultSegment::LINE_SEGMENT, "0");
		decodeCoreMessage(event_child, cei);
		break;
	    case GPSLIP:
		if(cei == NULL) cei = new GPSlipMessage(FaultSegment::LINE_SEGMENT, "0");
		decodeCoreMessage(event_child, cei);
		break;
	    case BEFORES:
	    case FINITE_FAULT:
		if(cei == NULL) cei = new FiniteFaultMessage(FINITE_FAULT, FaultSegment::LINE_SEGMENT, "0");
		decodeCoreMessage(event_child, cei);
		break;
	    case DM:
		if(cei == NULL) cei = new DMMessage("0");
		decodeCoreMessage(event_child, cei);
		break;
	    case SA:
		if(cei == NULL) cei = new DMMessage("0");
		cei->setSystemName(system_name); // DM done in constructor, needs overwriting for SA
		decodeCoreMessage(event_child, cei);
		break;
	    case EQINFO2GM:
		if(cei == NULL) cei = new GMMessage("0"); 
		cei->setSystemName(system_name); // DM done in constructor, needs overwriting for GM
		decodeCoreMessage(event_child, cei);
		break;
	    case DMREVIEW:
		if(cei == NULL) cei = new DMReviewMessage("0");
		decodeCoreMessage(event_child, cei);
		break;
	    default:
		break;
	    }
	}
    }

    // if no core info, return;
    if( cei == NULL ) return cei;

    // set message type and version
    cei->setMessageType(type);
    cei->setVersion(version);
    cei->setMessageCategory(message_category);
    cei->setTimestamp(time_stamp);
    cei->setAlgorithmVersion(alg_ver);
    cei->setProgramInstance(instance);
    cei->setReferenceSrc(ref_src);
    cei->setReferenceId(ref_id);
    cei->setOrigSys(sys_name);

    // gather information from all "gm_info" nodes. There can be more than one.
    if( !decodeGMInfo(event_children, (AlgMessage *)cei) ) return NULL;

    if(system_name == FINITE_FAULT || system_name == FINDER || system_name == GPSLIP
            || system_name == BEFORES) {
	if( !decodeAllFiniteFault(event_children, (FiniteFaultMessage *)cei) ) return NULL;
    }
    else if(system_name == DM || system_name == SA) {
	if( !decodeAllFiniteFault(event_children, (DMMessage *)cei) ||
	    !decodeContributors(event_children, (DMMessage *)cei) )
	{
	    return NULL;
	}
    } else if(system_name == EQINFO2GM) {
	if( !decodeAllFiniteFault(event_children, (GMMessage *)cei) ||
	    !decodeContributors(event_children, (GMMessage *)cei) ||
	    !decodeGM(event_children, (GMMessage *)cei))
	{
	    return NULL;
	}
    }

    if(print) cei->coutPrint();

    return cei;
}

bool DMMessageDecoder::decodeAllFiniteFault(DOMNodeList *event_children, FiniteFaultMessage *ff)
{
    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
        if(nodeNameIs(event_children->item(i), "fault_info") &&
                    event_children->item(i)->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMNodeList *info_children = event_children->item(i)->getChildNodes();
            for(XMLSize_t j = 0; j < info_children->getLength(); j++) {
                if(nodeNameIs(info_children->item(j), "finite_fault")) {
                    decodeFiniteFault(info_children->item(j), ff);
                }
            }
        }
    }
    return true;
}

bool DMMessageDecoder::decodeAllFiniteFault(DOMNodeList *event_children, DMMessage *dm)
{
    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
        if(nodeNameIs(event_children->item(i), "fault_info") &&
                    event_children->item(i)->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMNodeList *info_children = event_children->item(i)->getChildNodes();
            for(XMLSize_t j = 0; j < info_children->getLength(); j++) {
                if(nodeNameIs(info_children->item(j), "finite_fault")) {
                    FiniteFaultMessage ff(dm->getSystemName(), FaultSegment::UNKNOWN_SEGMENT, dm->getID());
                    decodeFiniteFault(info_children->item(j), &ff);
                    dm->addFiniteFaultMessage(ff);
                }
            }
        }
    }
    return true;
}

bool DMMessageDecoder::decodeFiniteFault(DOMNode *node, FiniteFaultMessage *ff)
{
    DOMElement *e;
    if (node->getNodeType() == DOMNode::ELEMENT_NODE && (e = dynamic_cast< DOMElement* >(node))) {
        std::string attStr;
        getAttributeValue(e, "atten_geom", attStr);
        ff->setAttenuationGeometry(attStr);
        getAttributeValue(e, "segment_shape", attStr);
        if (!ff->setSegmentShape(attStr)) {
            LOGE<<__FILE__<<" Warning: cannot parse fault_info with unknown segment_shape"<<std::endl;
            return false;
        }
        DOMNodeList *info_children = node->getChildNodes();
        for(XMLSize_t j = 0; j < info_children->getLength(); j++) {
            if(nodeNameIs(info_children->item(j), "segment") &&
                    info_children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE) {
                if(!decodeFFSegment(info_children->item(j)->getChildNodes(), ff)) { return false; }
            } else if(nodeNameIs(info_children->item(j), "global_uncertainty") &&
                    info_children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE) {
                if(!decodeFFGlobalUncertainty(info_children->item(j)->getChildNodes(), ff)) { return false; }
            } else if(nodeNameIs(info_children->item(j), "confidence") &&
                    info_children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMNode *node = info_children->item(j);
                double value;
                if (node->getNodeType() == DOMNode::ELEMENT_NODE &&
                            (e = dynamic_cast< DOMElement* >(node))) { 
                    if(!getNodeText(node, &value)) { continue; }
                    ff->setConfidence(value);
                }
            }
        }
    }
    return true;
}

bool DMMessageDecoder::decodeFFSegment(DOMNodeList *event_children, FiniteFaultMessage *ff) {
    DOMNodeList *info_children, *children;
    DOMElement *e;
    double value;
    std::string units;
    FaultSegment fs(ff->getSegmentShape());
    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
    if (event_children->item(i)->getNodeType() != DOMNode::ELEMENT_NODE) { continue; }
	if(nodeNameIs(event_children->item(i), "vertices")) {
        info_children = event_children->item(i)->getChildNodes();
        for(XMLSize_t j = 0; j < info_children->getLength(); j++) {
            if(nodeNameIs(info_children->item(j), "vertex") &&
                    info_children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE) {
                children = info_children->item(j)->getChildNodes();
                double lat, lon, depth;
                bool got_lat, got_lon, got_depth;
                std::string lat_units, lon_units, depth_units;
                for(XMLSize_t k = 0; k < children->getLength(); k++) {
                    DOMNode *node = children->item(k);
                    if(node->getNodeType() == DOMNode::ELEMENT_NODE && 
                                        (e = dynamic_cast< DOMElement* >(node))) {
                        if(!getNodeText(node, &value)) { continue; }
                        getAttributeValue(e, "units", units);
                        if(nodeNameIs(node, "lat")) {
                            lat = value;
                            lat_units = units;
                            got_lat = true;
                        } else if(nodeNameIs(node, "lon")) {
                            lon = value;
                            lon_units = units;
                            got_lon = true;
                        } else if(nodeNameIs(node, "depth")) {
                            depth = value;
                            depth_units = units;
                            got_depth = true;
                        }
                    }
                }
                if (got_lat && got_lon && got_depth) {
                    fs.addVertex(lat, lon, depth, lat_units, lon_units, depth_units);
                }
            }
        }
    } else if(nodeNameIs(event_children->item(i), "slip")) {
        info_children = event_children->item(i)->getChildNodes();
        FaultSlip fslip;
        for(XMLSize_t j = 0; j < info_children->getLength(); j++) {
            DOMNode *node = info_children->item(j);
            if(node->getNodeType() == DOMNode::ELEMENT_NODE && 
                        (e = dynamic_cast< DOMElement* >(node))) { 
                if(!getNodeText(node, &value)) { continue; }
                getAttributeValue(e, "units", units);
                if(nodeNameIs(node, "ss")) {
                    fslip.setStrikeSlip(value);
                    fslip.setStrikeSlipUnits(units);
                } else if(nodeNameIs(node, "ss_uncer")) {
                    fslip.setStrikeSlipUncer(value);
                    fslip.setStrikeSlipUncerUnits(units);
                } else if(nodeNameIs(node, "ds")) {
                    fslip.setDipSlip(value);
                    fslip.setDipSlipUnits(units);
                } else if(nodeNameIs(node, "ds_uncer")) {
                    fslip.setDipSlipUncer(value);
                    fslip.setDipSlipUncerUnits(units);
                }
            }
        }
        fs.addFaultSlip(fslip);
    }
    }
    ff->addSegment(fs);
    return true;
}

bool DMMessageDecoder::decodeFFGlobalUncertainty(DOMNodeList *event_children, FiniteFaultMessage *ff) {
    double value;
    std::string units;
    DOMElement* e;
    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
        DOMNode *node = event_children->item(i);
        if (node->getNodeType() == DOMNode::ELEMENT_NODE &&
                    (e = dynamic_cast< DOMElement* >(node))) { 
            if(!getNodeText(node, &value)) { continue; }
            getAttributeValue(e, "units", units);
            if(nodeNameIs(node, "lon_trans")) {
                ff->setGULonTrans(value);
                ff->setGULonTransUnits(units);
            } else if (nodeNameIs(node, "lat_trans")) {
                ff->setGULatTrans(value);
                ff->setGULatTransUnits(units);
            } else if(nodeNameIs(node, "total_len")) {
                ff->setGUTotalLen(value);
                ff->setGUTotalLenUnits(units);
            } else if(nodeNameIs(node, "strike")) {
                ff->setGUStrike(value);
                ff->setGUStrikeUnits(units);
            } else if(nodeNameIs(node, "dip")) {
                ff->setGUDip(value);
                ff->setGUDipUnits(units);
            }
        }
    }
    return true;
}

bool DMMessageDecoder::decodeGMInfo(DOMNodeList *in_event_children, AlgMessage *alg)
{
    for(XMLSize_t i = 0; i < in_event_children->getLength(); i++) {
	if(nodeNameIs(in_event_children->item(i), "gm_info") &&
		in_event_children->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
	{
        DOMNodeList *event_children = in_event_children->item(i)->getChildNodes();
        for(XMLSize_t j = 0; j < event_children->getLength(); j++) {
            if (event_children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE) {
                if(nodeNameIs(event_children->item(j), "gmpoint_pred")) {
                    if( !decodePredictions(event_children->item(j), alg) ) return false;
                } else if(nodeNameIs(event_children->item(j), "gmpoint_obs")) {
                    if( !decodeObservations(event_children->item(j), alg) ) return false;
                } else {
                    if( !decodeObservations(in_event_children->item(i), alg) ) return false;
                    if( !decodePredictions(in_event_children->item(i), alg) ) return false;
                    break;
                } 
            }
        }
	}
    }
    return true;
}

bool DMMessageDecoder::decodeObservations(DOMNode *info_node, AlgMessage *alg)
{
    string obs_name;
    DOMNodeList *info_children, *children;

    info_children = info_node->getChildNodes();

    for(XMLSize_t i = 0; info_children && i < info_children->getLength(); i++)
	if(info_children->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
    {

	getNodeName(info_children->item(i), obs_name);

	if(nodeNameIs(obs_name, "pgd_obs") || nodeNameIs(obs_name, "pgv_obs") ||
		nodeNameIs(obs_name, "pga_obs"))
	{
	    children = info_children->item(i)->getChildNodes();
	    for(XMLSize_t j = 0; children && j < children->getLength(); j++) {
		// skip extra non-element nodes with name "#text"
		if(children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE &&
		    !addGMObservation(children->item(j), obs_name, alg)) return false;
	    }
	}
    }
    return true;
}

bool DMMessageDecoder::decodePredictions(DOMNode *info_node, AlgMessage *alg)
{
    string pred_name;
    DOMNodeList *info_children, *children;

    info_children = info_node->getChildNodes();

    for(XMLSize_t i = 0; info_children && i < info_children->getLength(); i++)
	if(info_children->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
    {

	getNodeName(info_children->item(i), pred_name);

	if(nodeNameIs(pred_name, "pgd_pred") || nodeNameIs(pred_name, "pgv_pred") ||
		nodeNameIs(pred_name, "pga_pred"))
	{
	    children = info_children->item(i)->getChildNodes();
	    for(XMLSize_t j = 0; children && j < children->getLength(); j++) {
		if(children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE &&
		    !addGMPrediction(children->item(j), pred_name, alg)) return false;
	    }
	}
    }
    return true;
}

/** Wraps the decoding of ground motion contour and ground motion map tags within the gm_info 
 * tag.
 * @param event_children DOMNodeList includes all tags at gm_info level
 * @param alg GMMessage* pointer to GMMessage object to be filled
 * @return bool for function success/failure
 */
bool DMMessageDecoder::decodeGM(DOMNodeList *event_children, GMMessage *alg)
{
    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
	if(nodeNameIs(event_children->item(i), "gm_info") &&
		event_children->item(i)->getNodeType() == DOMNode::ELEMENT_NODE) {

            string pred_name;
            DOMNodeList *info_children;
            info_children = event_children->item(i)->getChildNodes();

            for(XMLSize_t j = 0; info_children && j < info_children->getLength(); j++) {
                if(info_children->item(j)->getNodeType() == DOMNode::ELEMENT_NODE) {
                    getNodeName(info_children->item(j), pred_name);
                    if(nodeNameIs(pred_name, "gmcont_pred") || nodeNameIs(pred_name, "gmcontour_pred")) {
                        DOMNodeList *info_grandchildren;
                        info_grandchildren = info_children->item(j)->getChildNodes();
                        bool bOldContourTags = true;
                        for(XMLSize_t k = 0; info_grandchildren && k < info_grandchildren->getLength(); k++) {
                            DOMNode *node = info_grandchildren->item(k);
                            std::string name;
                            getNodeName(node, name);
                            if (nodeNameIs(name, "contour")) { 
                                bOldContourTags = false;
                                if(!addGMContour(info_grandchildren->item(k), alg)) return false;
                            }
                        }
                        if (bOldContourTags) {
                            if(!addGMContour(info_children->item(j), alg)) return false;
                        }
                    } else if(nodeNameIs(pred_name, "gmmap_pred")) {
                        if(!addGMMap(info_children->item(j), alg)) return false;
                    }
                }
            }
        }
    }

    return true;
}


CoreEventInfo* DMMessageDecoder::decodeCoreMessage(DOMNode* core_node, CoreEventInfo* cei)
{
    int ivalue;
    double value;
    string id, name, units, otime;
    DOMNodeList *core_children;
    DOMElement *core_e = dynamic_cast< DOMElement* >(core_node);

    if( getAttributeValue(core_e, "id", id) ) {
	cei->setID(id);
    }
    if( !(core_children = core_node->getChildNodes()) ) {
	// no core values
	return NULL;
    }

    for(XMLSize_t i = 0; i < core_children->getLength(); i++) {
	DOMNode *node = core_children->item(i);
	DOMElement *e = NULL;
	if(node->getNodeType() == DOMNode::ELEMENT_NODE && (e = dynamic_cast< DOMElement* >(node)) )
	{
	    getNodeName(node, name);
	    if(nodeNameIs(name, "mag")) {
		if(getAttributeValue(e, "units", units)) cei->setMagnitudeUnits(units);
		if(getNodeText(node, &value)) cei->setMagnitude(value);
	    }
	    else if(nodeNameIs(name, "mag_uncer")) {
		if(getAttributeValue(e, "units", units)) cei->setMagnitudeUncertaintyUnits(units);
		if(getNodeText(node, &value)) cei->setMagnitudeUncertainty(value);
	    }
	    else if(nodeNameIs(name, "lat")) {
		if(getAttributeValue(e, "units", units)) cei->setLatitudeUnits(units);
		if(getNodeText(node, &value)) cei->setLatitude(value);
	    }
	    else if(nodeNameIs(name, "lat_uncer")) {
		if(getAttributeValue(e, "units", units)) cei->setLatitudeUncertaintyUnits(units);
		if(getNodeText(node, &value)) cei->setLatitudeUncertainty(value);
	    }
	    else if(nodeNameIs(name, "lon")) {
		if(getAttributeValue(e, "units", units)) cei->setLongitudeUnits(units);
		if(getNodeText(node, &value)) cei->setLongitude(value);
	    }
	    else if(nodeNameIs(name, "lon_uncer")) {
		if(getAttributeValue(e, "units", units)) cei->setLongitudeUncertaintyUnits(units);
		if(getNodeText(node, &value)) cei->setLongitudeUncertainty(value);
	    }
	    else if(nodeNameIs(name, "depth")) {
		if(getAttributeValue(e, "units", units)) cei->setDepthUnits(units);
		if(getNodeText(node, &value)) cei->setDepth(value);
	    }
	    else if(nodeNameIs(name, "depth_uncer")) {
		if(getAttributeValue(e, "units", units)) cei->setDepthUncertaintyUnits(units);
		if(getNodeText(node, &value)) cei->setDepthUncertainty(value);
	    }
	    else if(nodeNameIs(name, "orig_time")) {
		if(getAttributeValue(e, "units", units)) cei->setOriginTimeUnits(units);
		if(getNodeText(node, &value)) cei->setOriginTime(value);
		else if(getNodeText(node, otime)) cei->setOriginTime(otime);
	    }
	    else if(nodeNameIs(name, "orig_time_uncer")) {
		if(getAttributeValue(e, "units", units)) cei->setOriginTimeUncertaintyUnits(units);
		if(getNodeText(node, &value)) cei->setOriginTimeUncertainty(value);
	    }
	    else if(nodeNameIs(name, "likelihood") || nodeNameIs(name, "likelyhood")) {
		if(getNodeText(node, &value)) cei->setLikelyhood(value);
	    }
	    else if(nodeNameIs(name, "num_stations")) {
		if(getNodeText(node, &ivalue)) cei->setNumberStations(ivalue);
	    }
	    else {
		cerr << "DMMessageDecoder:: invalid element in core_info: " << name << endl;
		return NULL;
	    }
	}
    }
    return cei;
}

bool DMMessageDecoder::decodeContributors(DOMNodeList *event_children, DMMessage *dm)
{
    string name;
    DOMNodeList *children;
    DOMElement *e;

    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
	if(nodeNameIs(event_children->item(i), "contributors") &&
		event_children->item(i)->getNodeType() == DOMNode::ELEMENT_NODE)
	{
	    if( (children = event_children->item(i)->getChildNodes()) != NULL)
	    {
		for(XMLSize_t j = 0; j < children->getLength(); j++) {
		    DOMNode *node = children->item(j);
		    if(node->getNodeType() == DOMNode::ELEMENT_NODE && (e = dynamic_cast< DOMElement* >(node)) )
		    {
			getNodeName(node, name);

			if(nodeNameIs(name, "contributor")) {
			    if(!addContributor(e, dm)) return false;
			}
			else {
			    cerr << "DMMessageDecoder:: node contributors: invalid child node: " << name << endl;
			    return false;
			}
		    }
		}
	    }
	}
    }
    return true;
}

bool DMMessageDecoder::addContributor(DOMElement *e, DMMessage *dm)
{
    int version;
    string event_id;
    string alg_name, alg_version, alg_instance, cat;
    enum eewSystemName algorithm_name;
    enum MessageCategory category;

    if(!getAttributeValue(e, "alg_name", alg_name)) {
	cerr << "DMMessageDecoder::decodeContributors: alg_name not found." << endl;
	return false;
    }
    else if(!getAttributeValue(e, "alg_instance", alg_instance)) {
	cerr << "DMMessageDecoder::decodeContributors: alg_instance not found." << endl;
	return false;
    }
    else if(!getAttributeValue(e, "alg_version", alg_version)) {
	cerr << "DMMessageDecoder::decodeContributors: alg_version not found." << endl;
	return false;
    }
    else if( !getAttributeValue(e, "event_id", event_id) ) {
	cerr << "DMMessageDecoder::decodeContributors: event_id not found." << endl;
	return false;
    }
    else if( !getAttributeValue(e, "version", &version) ) {
	cerr << "DMMessageDecoder::decodeContributors: invalid version." << endl;
	return false;
    }
    else if( !getAttributeValue(e, "category", cat) ) {
	cerr << "DMMessageDecoder::decodeContributors: category not found." << endl;
	return false;
    }

    bool valid_alg_name = false;
    for(int i = 0; i < (int)(sizeof(eewSystemNameString)/sizeof(string)); i++) {
	if(!strcasecmp(alg_name.c_str(), eewSystemNameString[i].c_str())) {
	    algorithm_name = CoreEventInfo::eewSystemNameArray[i];
	    valid_alg_name = true;
	}
    }
    if(!valid_alg_name) {
	cerr << "DMMessageDecoder::decodeContributors: invalid alg_name." << alg_name << endl;
	return false;
    }

    bool valid_category = false;
    for(int i = 0; i < (int)(sizeof(MessageCategoryString)/sizeof(string)); i++) {
	if(!strcasecmp(cat.c_str(), MessageCategoryString[i].c_str())) {
	    category = CoreEventInfo::MessageCategoryArray[i];
	    valid_category = true;
	}
    }
    if(!valid_category) {
	cerr << "DMMessageDecoder::decodeContributors: invalid category." << cat << endl;
	return false;
    }
    dm->addContributor(DMContributor(algorithm_name, alg_instance, alg_version, event_id, version, category));
    return true;
}

bool DMMessageDecoder::addGMObservation(DOMNode *obs_node, string obs_name, AlgMessage *alg)
{
    string name, sncl;
    DOMNodeList *obs_children;
    DOMElement *e;
    string sta, net, chan, loc, units, lat_units, lon_units, time_units, stime, orig_sys;
    double value, lat, lon, otime;
    bool got_value=false, got_lat=false, got_lon=false, got_time=false;
    enum ObservationType type;

    if(nodeNameIs(obs_name, "pgd_obs")) {
	type = DISPLACEMENT_OBS;
    }
    else if(nodeNameIs(obs_name, "pgv_obs")) {
	type = VELOCITY_OBS;
    }
    else if(nodeNameIs(obs_name, "pga_obs")) {
	type = ACCELERATION_OBS;
    }
    else {
	cerr << "DMMessageDecoder::decodeObservation: invalid node " << obs_name << endl;
	return false;
    }

    if( !(obs_children = obs_node->getChildNodes()) ) {
	cerr << "DMMessageDecoder::decodeObservation: missing child nodes." << endl;
	return false;
    }
    e = dynamic_cast< DOMElement* >(obs_node);
    getAttributeValue(e, "orig_sys", orig_sys);

    for(XMLSize_t i = 0; i < obs_children->getLength(); i++) {
	DOMNode *node = obs_children->item(i);

	if(node->getNodeType() == DOMNode::ELEMENT_NODE && (e = dynamic_cast< DOMElement* >(node)) )
	{
	    getNodeName(node, name);

	    if(nodeNameIs(name, "SNCL")) {
		if(getNodeText(node, sncl)) getSNCL(sncl, sta, net, chan, loc);
	    }
	    else if(nodeNameIs(name, "value")) {
		getAttributeValue(e, "units", units);
		if(getNodeText(node, &value)) got_value = true;
	    }
	    else if(nodeNameIs(name, "lat")) {
		getAttributeValue(e, "units", lat_units);
		if(getNodeText(node, &lat)) got_lat = true;
	    }
	    else if(nodeNameIs(name, "lon")) {
		getAttributeValue(e, "units", lon_units);
		if(getNodeText(node, &lon)) got_lon = true;
	    }
	    else if(nodeNameIs(name, "time")) {
		getAttributeValue(e, "units", time_units);
		if(getNodeText(node, &value)) { otime = value; got_time = true; }
		else if(getNodeText(node, stime) && CoreEventInfo::stringToTepoch(stime, &otime)) {
		    got_time = true;
		}
	    }
	    else {
		cerr << "DMMessageDecoder::decodeObservation: invalid node: " << name << endl;
		return false;
	    }
	}
    }
    if(sta.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing SNCL" << endl;
    }
    else if(net.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: invalid SNCL" << endl;
    }
    else if(chan.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: invalid SNCL" << endl;
    }
    else if(loc.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: invalid SNCL" << endl;
    }
    else if(units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing units" << endl;
    }
    else if(lat_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing lat_units" << endl;
    }
    else if(lon_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing lon_units" << endl;
    }
    else if(time_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing time_units" << endl;
    }
    else if(!got_value) {
	cerr << "DMMessageDecoder::decodeObservation: missing value" << endl;
    }
    else if(!got_lat) {
	cerr << "DMMessageDecoder::decodeObservation: missing lat" << endl;
    }
    else if(!got_lon) {
	cerr << "DMMessageDecoder::decodeObservation: missing lon" << endl;
    }
    else if(!got_time) {
	cerr << "DMMessageDecoder::decodeObservation: missing time" << endl;
    }
    else
    {
	alg->addGMObservation(type, sta, net, chan, loc, value, lat, lon, otime,
				units, lat_units, lon_units, time_units, orig_sys);
	return true;
    }
    return false;
}

bool DMMessageDecoder::addGMPrediction(DOMNode *pred_node, string pred_name, AlgMessage *alg)
{
    string name, sncl;
    DOMNodeList *pred_children;
    DOMElement *e;
    string sta, net, chan, loc, pred_units, pred_uncer_units, ptime_units, ptime_uncer_units;
    string app_rad_units, lat_units, lon_units, stime;
    double pred, pred_uncer, ptime, ptime_uncer, app_rad, lat, lon;
    bool got_pred=false, got_pred_uncer=false, got_ptime=false, got_ptime_uncer=false,
		got_app_rad=false, got_lat=false, got_lon=false;
    enum PredictionType type;

    if(nodeNameIs(pred_name, "pgd_pred")) {
	type = DISPLACEMENT_PRED;
    }
    else if(nodeNameIs(pred_name, "pgv_pred")) {
	type = VELOCITY_PRED;
    }
    else if(nodeNameIs(pred_name, "pga_pred")) {
	type = ACCELERATION_PRED;
    }
    else {
	cerr << "DMMessageDecoder::decodePrediction: invalid node " << pred_name << endl;
	return false;
    }

    if( !(pred_children = pred_node->getChildNodes()) ) {
	cerr << "DMMessageDecoder::decodePrediction: missing child nodes." << endl;
	return false;
    }

    for(XMLSize_t i = 0; i < pred_children->getLength(); i++) {
	DOMNode *node = pred_children->item(i);

	if(node->getNodeType() == DOMNode::ELEMENT_NODE && (e = dynamic_cast< DOMElement* >(node)) )
	{
	    getNodeName(node, name);

	    if(nodeNameIs(name, "SNCL")) {
		if(getNodeText(node, sncl)) getSNCL(sncl, sta, net, chan, loc);
	    }
	    else if(nodeNameIs(name, "value")) {
		getAttributeValue(e, "units", pred_units);
		if(getNodeText(node, &pred)) got_pred = true;
	    }
	    else if(nodeNameIs(name, "value_uncer")) {
		getAttributeValue(e, "units", pred_uncer_units);
		if(getNodeText(node, &pred_uncer)) got_pred_uncer = true;
	    }
	    else if(nodeNameIs(name, "lat")) {
		getAttributeValue(e, "units", lat_units);
		if(getNodeText(node, &lat)) got_lat = true;
	    }
	    else if(nodeNameIs(name, "lon")) {
		getAttributeValue(e, "units", lon_units);
		if(getNodeText(node, &lon)) got_lon = true;
	    }
	    else if(nodeNameIs(name, "app_rad")) {
		getAttributeValue(e, "units", app_rad_units);
		if(getNodeText(node, &app_rad)) got_app_rad = true;
	    }
	    else if(nodeNameIs(name, "time")) {
		getAttributeValue(e, "units", ptime_units);
		if(getNodeText(node, &ptime)) { got_ptime = true; }
		else if(getNodeText(node, stime) && CoreEventInfo::stringToTepoch(stime, &ptime)) {
		    got_ptime = true;
		}
	    }
	    else if(nodeNameIs(name, "time_uncer")) {
		getAttributeValue(e, "units", ptime_uncer_units);
		if(getNodeText(node, &ptime_uncer)) got_ptime_uncer = true;
	    }
	    else {
		cerr << "DMMessageDecoder::decodePrediction: invalid node: " << name << endl;
		return false;
	    }
	}
    }
    if(sta.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing SNCL" << endl;
    }
    else if(net.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: invalid SNCL" << endl;
    }
    else if(chan.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: invalid SNCL" << endl;
    }
    else if(loc.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: invalid SNCL" << endl;
    }
    else if(!got_pred) {
	cerr << "DMMessageDecoder::decodeObservation: missing value" << endl;
    }
    else if(pred_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing value units" << endl;
    }
    else if(!got_pred_uncer) {
	cerr << "DMMessageDecoder::decodeObservation: missing value_uncer" << endl;
    }
    else if(pred_uncer_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing value_uncer units" << endl;
    }
    else if(!got_lat) {
	cerr << "DMMessageDecoder::decodeObservation: missing lat" << endl;
    }
    else if(lat_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing lat units" << endl;
    }
    else if(!got_lon) {
	cerr << "DMMessageDecoder::decodeObservation: missing lon" << endl;
    }
    else if(lon_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing lon units" << endl;
    }
    else if(!got_app_rad) {
	cerr << "DMMessageDecoder::decodeObservation: missing app_rad" << endl;
    }
    else if(app_rad_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing app_rad units" << endl;
    }
    else if(!got_ptime) {
	cerr << "DMMessageDecoder::decodeObservation: missing time" << endl;
    }
    else if(ptime_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing time units" << endl;
    }
    else if(!got_ptime_uncer) {
	cerr << "DMMessageDecoder::decodeObservation: missing time_uncer" << endl;
    }
    else if(ptime_uncer_units.empty()) {
	cerr << "DMMessageDecoder::decodeObservation: missing time_uncer units" << endl;
    }
    else {
	alg->addGMPrediction(type, sta, net, chan, loc, pred, pred_uncer, ptime, ptime_uncer, app_rad,
		lat, lon, pred_units, pred_uncer_units, ptime_units, ptime_uncer_units,
		app_rad_units, lat_units, lon_units);
	return true;
    }
    return false;
}

/** Decodes the gmmap_pred tag and fills the GMMap class held by GMMessage
 * @param in_node DOMNode* the gmmap_pred node in XML
 * @param alg GMMessage* pointer to GMMessage class object to be filled
 * @param bool for function success/failure
 * */
bool DMMessageDecoder::addGMMap(DOMNode *in_node, GMMessage *alg)
{

    string name, field_units, field_name, griddata, gdstr;
    DOMNodeList *children;
    DOMElement *e;
    int idx, n_grid_pts;
    std::vector<gminfo::GM_INFO> GMInfoVect;
    bool got_grid=false;

    if( !(children = in_node->getChildNodes()) ) {
	    cerr << "DMMessageDecoder::decodeGMMap: missing child nodes." << endl;
	    return false;
    }

    if ((e = dynamic_cast< DOMElement* >(in_node))) {
    	getAttributeValue(e, "number", &n_grid_pts);
    } else {
   	    cerr<<"DMMessageDecoder::decodeGMMap invalid grid_data length"<<endl;
    }

    for(XMLSize_t i = 0; i < children->getLength(); i++) {
	    DOMNode *node = children->item(i);
        if(node->getNodeType() == DOMNode::ELEMENT_NODE && 
                            (e = dynamic_cast< DOMElement* >(node)) )
        {
            getNodeName(node, name);
            if(nodeNameIs(name, "grid_field")) {
                getAttributeValue(e, "index", &idx);
                getAttributeValue(e, "name", field_name);
                getAttributeValue(e, "units", field_units);
            }
            else if(nodeNameIs(name, "grid_data")) {
            if(getNodeText(node, griddata)) got_grid = true;
                stringstream ss(griddata);
                while(std::getline(ss,gdstr,'\n')) { // break on line
                    stringstream lss(gdstr);
                    gminfo::GM_INFO myGMInfo;
                    // Note here the fixed order entries in the map!
                    lss >> myGMInfo.lonGM;
                    lss >> myGMInfo.latGM;
                    lss >> myGMInfo.PGA;
                    lss >> myGMInfo.PGV;
                    lss >> myGMInfo.SI;
                    GMInfoVect.push_back(myGMInfo);
                }
            }
            else {
                cerr << "DMMessageDecoder::decodeGMMap: invalid node: " << name << endl;
                return false;
            }
        }
    }
    if (!got_grid) {
	    cerr << "DMMessageDecoder::decodeGMMap: missing griddata" << endl;
    } else {
        GMMap myGMMap;
        myGMMap.setMapData(GMInfoVect);
        alg->setGMFullMap(myGMMap);
	    return true;
    }
    return false;

}

/** Decodes the gmcont_pred tag and fills the GMContour class held by GMMessage
 * @param in_node DOMNode* the gmcont_pred node in XML
 * @param alg GMMessage* pointer to GMMessage class object to be filled
 * @param bool for function success/failure
 * */
bool DMMessageDecoder::addGMContour(DOMNode *in_node, GMMessage *alg)
{

    DOMNodeList *children;
    DOMElement *e;
    std::string name, pga_units, pgv_units, polygonstr, llstr, latstr, lonstr;
    int mmi;
    double pga, pgv, lat, lon;
    bool got_pga=false, got_pgv=false, got_mmi=false, got_polygon=false;
    std::vector<double> lats, lons;

    if( !(children = in_node->getChildNodes()) ) {
	    cerr << "DMMessageDecoder::decodeGMContour: missing child nodes." << endl;
	    return false;
    }

    for(XMLSize_t i = 0; i < children->getLength(); i++) {
        DOMNode *node = children->item(i);

        if(node->getNodeType() == DOMNode::ELEMENT_NODE && 
                            (e = dynamic_cast< DOMElement* >(node)) )
        {
            getNodeName(node, name);
            if(nodeNameIs(name, "MMI")) {
                if(getNodeText(node, &mmi)) got_mmi = true;
            } else if(nodeNameIs(name, "PGA")) {
                if(getNodeText(node, &pga)) got_pga = true;
                getAttributeValue(e, "units", pga_units);
            } else if(nodeNameIs(name, "PGV")) {
                if(getNodeText(node, &pgv)) got_pgv = true;
                getAttributeValue(e, "units", pgv_units);
            } else if(nodeNameIs(name, "polygon")) {
                if(getNodeText(node, polygonstr)) got_polygon = true;
                stringstream ss(polygonstr);
                while(ss >> llstr) { 
                    size_t cma = llstr.find(",");
                    latstr = llstr.substr(0,cma);
                    lonstr = llstr.substr(cma+1,std::string::npos);
                    CoreEventInfo::stringToDouble(latstr,&lat);
                    CoreEventInfo::stringToDouble(lonstr,&lon);
                    lats.push_back(lat);
                    lons.push_back(lon);
                }
            } else {
                cerr << "DMMessageDecoder::decodeGMContour: invalid node: " << name << endl;
                return false;
            }
        }
    }
    if (!got_pga) {
	    cerr << "DMMessageDecoder::decodeGMContour: missing PGA" << endl;
    } else if(pga_units.empty()) {
	    cerr << "DMMessageDecoder::decodeGMContour: missing PGA units" << endl;
    } else if(!got_pgv) {
	    cerr << "DMMessageDecoder::decodeGMContour: missing PGV" << endl;
    } else if(pgv_units.empty()) {
	    cerr << "DMMessageDecoder::decodeGMContour: missing PGV units" << endl;
    } else if(!got_mmi) {
	    cerr << "DMMessageDecoder::decodeGMContour: missing MMI" << endl;
    } else if(!got_polygon) {
	    cerr << "DMMessageDecoder::decodeGMContour: missing polygon" << endl;
    } else {
        GMContour myGMContour;
        myGMContour.setPGV(pgv);
        myGMContour.setPGA(pga);
        myGMContour.setMMI(mmi);
        myGMContour.setLats(lats);
        myGMContour.setLons(lons);
        alg->addGMContourPredictions(myGMContour);
        return true;	
    }
    return false;
}


bool DMMessageDecoder::getAttributeValue(DOMElement *e, const char *name, string &value)
{
    value.assign("");
    XMLCh *xml_name = XMLString::transcode(name);
    if(xml_name) {
	const XMLCh *v = e->getAttribute(xml_name);
	XMLString::release(&xml_name);
	if(v) {
	    char *s = XMLString::transcode(v);
	    value.assign(stringTrim(s));
	    XMLString::release(&s);
	    return !value.empty();
	}
    }
    return false;
}

bool DMMessageDecoder::getAttributeValue(DOMElement *e, const char *name, int *value)
{
    string s;
    if(getAttributeValue(e, name, s)) {
	return stringToInt(s, value);
    }
    return false;
}

static bool
stringToInt(string &s, int *value)
{
    char *endptr, last_char;
    const char *c;
    long l;
    int n;

    for(c = s.c_str(); *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c != '\0') {
	l = strtol(c, &endptr, 10);
	last_char = *endptr;
	if(last_char == c[n]) {
	    *value = (long)l;
	    return true;
	}
    }
    return false;
}

bool DMMessageDecoder::getAttributeValue(DOMElement *e, const char *name, double *value)
{
    string s;
    if(getAttributeValue(e, name, s)) {
	return CoreEventInfo::stringToDouble(s, value);
    }
    return false;
}

bool DMMessageDecoder::getNodeText(DOMNode *node, string &value)
{
    const XMLCh *xml_value = node->getTextContent();
    if(xml_value) {
	string string_value;
	char *s = XMLString::transcode(xml_value);
	value.assign(stringTrim(s));
	XMLString::release(&s);
	return true;
    }
    return false;
}

bool DMMessageDecoder::getNodeText(DOMNode *node, double *value)
{
    const XMLCh *xml_value = node->getTextContent();
    if(xml_value) {
	string string_value;
	char *s = XMLString::transcode(xml_value);
	string_value.assign(stringTrim(s));
	XMLString::release(&s);
	return CoreEventInfo::stringToDouble(string_value, value);
    }
    return false;
}

bool DMMessageDecoder::getNodeText(DOMNode *node, int *value)
{
    const XMLCh *xml_value = node->getTextContent();
    if(xml_value) {
	string string_value;
	char *s = XMLString::transcode(xml_value);
	string_value.assign(stringTrim(s));
	XMLString::release(&s);
	return CoreEventInfo::stringToInt(string_value, value);
    }
    return false;
}

bool DMMessageDecoder::getNodeName(DOMNode *node, string &name)
{
    const XMLCh *xml_name = node->getNodeName();
    if(xml_name) {
	char *s = XMLString::transcode(xml_name);
	name.assign(stringTrim(s));
	XMLString::release(&s);
	return true;
    }
    return false;
}

bool DMMessageDecoder::nodeNameIs(DOMNode *node, const char *s)
{
    string name;
    return (getNodeName(node, name) && !strcasecmp(name.c_str(), s));
}

bool DMMessageDecoder::nodeNameIs(string &node_name, const char *s)
{
    return !strcasecmp(node_name.c_str(), s);
}

static char *
stringTrim(char *str)
{
    int i, j, k, n;

    n = (int)strlen(str);
    for(j = n-1; j >= 0 && isspace((int)str[j]); j--) n--;
    if(n > 0) {
        for(i = 0; i < n && isspace((int)str[i]); i++);
        if(i > 0) for(k = i; k < n; k++) str[k-i] = str[k];
        n -= i;
    }
    if(str[n] != '\0') str[n] = '\0';
    return str;
}

bool DMMessageDecoder::getSNCL(string &sncl, string &sta, string &net, string &chan, string &loc)
{
    char *s = strdup(sncl.c_str());
    char *c, *last;

    if( !(c = strtok_r(s, ".", &last)) ) return false;
    sta.assign(stringTrim(c));
    if( !(c = strtok_r(NULL, ".", &last)) ) return false;
    net.assign(stringTrim(c));
    if( !(c = strtok_r(NULL, ".", &last)) ) return false;
    chan.assign(stringTrim(c));
    if( !(c = strtok_r(NULL, ".", &last)) ) return false;
    loc.assign(stringTrim(c));

    free(s);

    return true;
}

CoreEventInfo * DMMessageDecoder::decodeOldLogMessage(string message)
{
/* old format:
00:20:47:752|CoreEventInfo Start:
00:20:47:752|sysname: vs id: 30008032 ver: 0
00:20:47:752| mtype: new mag: 3.48 mag_u: -9.90 lat: 37.65 lat_u: -999.90 lon: -122.26 lon_u: -999.90 dep: 4.00 dep_u: -9.90 otime: 1305505240.99 otime_u: -9.90 lh: 0.30
00:20:47:752|CoreEventInfo End:
*/
    int n, ver=0;
    string id = "0";
    size_t pos;
    double mag=-999., mag_u=-999., lat=-999., lat_u=-999., lon=-999., lon_u=-999.;
    double dep=-999., dep_u=-999., otime=-999., otime_u=-999., lh=-999.;
    enum nudMessageType type = NEW;
    CoreEventInfo *cei = NULL;
    string s, sysname, log_time ;
    stringstream ss(message);
    vector<string> tokens;

    while(ss >> s) tokens.push_back(s);
    n = (int)tokens.size();

    for(int i = 0; i < n; i++) {
	if(tokens[i].length() >= 8 &&
	    !strcmp(tokens[i].c_str()+tokens[i].length()-8, "sysname:") && i < n-1)
	{
	    sysname = tokens[i+1];
	    if((pos = tokens[i].find_first_of("|")) != string::npos) {
		log_time = tokens[i].substr(0, pos);
	    }
	}
	else if(!tokens[i].compare("id:") && i < n-1) {
	    id = tokens[i+1];
	}
	else if(!tokens[i].compare("ver:") && i < n-1) {
	    if( !stringToInt(tokens[i+1], &ver) ) {
		cerr << "Invalid format for ver:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("mtype:") && i < n-1) {
	    if(!strcasecmp(tokens[i+1].c_str(), "new")) {
		type = NEW;
	    }
	    else if(!strcasecmp(tokens[i+1].c_str(), "UPDATE")) {
		type = UPDATE;
	    }
	    else if(!strcasecmp(tokens[i+1].c_str(), "DELETE")) {
		type = DELETE;
	    }
	    else {
		cerr << "Invalid type:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("mag:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &mag) ) {
		cerr << "Invalid format for mag:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("mag_u:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &mag_u) ) {
		cerr << "Invalid format for mag_u:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("lat:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &lat) ) {
		cerr << "Invalid format for lat:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("lat_u:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &lat_u) ) {
		cerr << "Invalid format for lat_u:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("lon:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &lon) ) {
		cerr << "Invalid format for lon:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("lon_u:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &lon_u) ) {
		cerr << "Invalid format for lon_u:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("dep:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &dep) ) {
		cerr << "Invalid format for dep:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("dep_u:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &dep_u) ) {
		cerr << "Invalid format for dep_u:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("otime:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &otime) ) {
		cerr << "Invalid format for otime:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("otime_u:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &otime_u) ) {
		cerr << "Invalid format for otime_u:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
	else if(!tokens[i].compare("lh:") && i < n-1) {
	    if( !CoreEventInfo::stringToDouble(tokens[i+1], &lh) ) {
		cerr << "Invalid format for lh:" << tokens[i+1] << endl;
		return NULL;
	    }
	}
    }
    if(sysname == "elarms") {
	cei = new ElarmsMessage(id, mag, mag_u, lat, lat_u, lon, lon_u, dep, dep_u,
				otime, otime_u, lh, type, ver);
    }
    else if(sysname == "vs") {
	cei = new VSMessage(id, mag, mag_u, lat, lat_u, lon, lon_u, dep, dep_u,
				otime, otime_u, lh, type, ver);
    }
    else if(sysname == "onsite") {
	cei = new OnSiteMessage(id, mag, mag_u, lat, lat_u, lon, lon_u, dep, dep_u,
				otime, otime_u, lh, type, ver);
    }
    else if(sysname == "dm") {
	cei = new DMMessage(id, mag, mag_u, lat, lat_u, lon, lon_u, dep, dep_u,
				otime, otime_u, lh, type, ver);
    }
    else {
	cerr << "Invalid sysname: " << sysname << endl;
    }
    if(cei) cei->setLogTime(log_time);
    return cei;
}
