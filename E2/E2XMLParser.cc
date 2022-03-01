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
/**
 * \author <henson@seismo.berkeley.edu>
 */
#include "qlib2.h"
#include "E2XMLParser.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <iostream>

#include "DMMessageDecoder.h"
#include "E2Event.h"
#include "TimeString.h"

using namespace std;
using namespace xercesc;

E2XMLParser::E2XMLParser() : parser(NULL), error_handler(NULL), input_source(NULL), do_namespaces(false),
		do_schema(false), schema_full_checking(false), do_create(false)
{
    // initialize xml parser
    XMLPlatformUtils::Initialize();
    parser = new XercesDOMParser();
    error_handler = new DefaultHandler();
    parser->setErrorHandler(error_handler);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(do_namespaces);
    parser->setDoSchema(do_schema);
    parser->setValidationSchemaFullChecking(schema_full_checking);
    parser->setCreateEntityReferenceNodes(do_create);
    input_source = new MemBufInputSource((const XMLByte *)NULL, 0, "E2AMQReader");
}

E2XMLParser::~E2XMLParser() throw()
{
    delete parser;
    delete input_source;
    delete error_handler;
    XMLPlatformUtils::Terminate();
}

Teleseism * E2XMLParser::processMessage(string &s)
{
    try {
	input_source->resetMemBufInputSource((const XMLByte *)s.data(),
			(XMLSize_t)(s.length()*sizeof(char)));
	parser->parse(*input_source);
    }
    catch(const XMLException& e) {
	char* msg = XMLString::transcode(e.getMessage());
	cerr << "E2XMLParser:Exception message is:" << endl << msg << endl;
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return NULL;
    }
    catch(const DOMException& e) {
	char* msg = XMLString::transcode(e.msg);
	cerr << "E2XMLParser:Exception message is:" << endl << msg << endl;
	XMLString::release(&msg);
	return NULL;
    }
    catch(const SAXException& e) {
	char* msg = XMLString::transcode(e.getMessage());
	cerr << "E2XMLParser:SAXException message is:" << endl << msg << endl;
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return NULL;
    }
    catch(...) {
	cerr << "E2XMLParser:Exception" << endl;
	parser->resetDocumentPool();
	return NULL;
    }

    DOMDocument *doc = parser->getDocument();

    XMLCh *tag = XMLString::transcode("teleseism_message");
    DOMNodeList *event_list = doc->getElementsByTagName(tag);
    XMLString::release(&tag);

    // could have more than one event_message. list->getLength() > 1, but just do one for now.

    for(XMLSize_t i = 0; i < event_list->getLength() && i < 1; i++) {
	DOMNode *event_node = event_list->item(i);
	if(event_node->getNodeType() == DOMNode::ELEMENT_NODE)
	{
	    Teleseism *tele = decodeEvent(event_node);
	    parser->resetDocumentPool();
	    return tele;
	}
    }
    parser->resetDocumentPool();
    return NULL;
}

Teleseism * E2XMLParser::decodeEvent(DOMNode *event_node)
{
    DOMElement *element;
    DOMNodeList *event_children;
    string otime, name, s;
    Teleseism t;

    element = dynamic_cast< DOMElement* >(event_node);

    if( !DMMessageDecoder::getAttributeValue(element, "src", t.src) ) {
        cerr << "E2XMLDecoder::decodeEvent: no 'src'" << endl;
        return NULL;
    }
    if( !DMMessageDecoder::getAttributeValue(element, "id", t.id) ) {
        cerr << "E2XMLDecoder::decodeEvent: no 'id'" << endl;
        return NULL;
    }
    if( !DMMessageDecoder::getAttributeValue(element, "time", otime) ) {
        cerr << "E2XMLDecoder::decodeEvent: no 'time'" << endl;
        return NULL;
    }
    stringToTepoch(otime, &t.time);
    if( !DMMessageDecoder::getAttributeValue(element, "lat", &t.lat) ) {
        cerr << "E2XMLDecoder::decodeEvent: no 'lat'" << endl;
        return NULL;
    }
    if( !DMMessageDecoder::getAttributeValue(element, "lon", &t.lon) ) {
        cerr << "E2XMLDecoder::decodeEvent: no 'lon'" << endl;
        return NULL;
    }
    if( !DMMessageDecoder::getAttributeValue(element, "depth", &t.depth) ) {
        cerr << "E2XMLDecoder::decodeEvent: no 'depth'" << endl;
        return NULL;
    }
    if( !DMMessageDecoder::getAttributeValue(element, "mag", &t.mag) ) {
        cerr << "E2XMLDecoder::decodeEvent: no 'mag'" << endl;
        return NULL;
    }
    if( !(event_children = event_node->getChildNodes()) ) {
        cerr << "E2XMLDecoder::decodeEvent: missing nodes" << endl;
        return NULL;
    }

    Teleseism *tele = new Teleseism(t);

    for(XMLSize_t i = 0; i < event_children->getLength(); i++) {
	DOMNode *node = event_children->item(i);
	DOMElement *e = NULL;
	if(node->getNodeType() == DOMNode::ELEMENT_NODE && (e = dynamic_cast< DOMElement* >(node)) )
	{
	    DMMessageDecoder::getNodeName(node, name);
	    if(DMMessageDecoder::nodeNameIs(name, "window_start")) {
		DMMessageDecoder::getNodeText(node, s);
		stringToTepoch(s, &tele->window_start);
	    }
	    else if(DMMessageDecoder::nodeNameIs(name, "window_end")) {
		DMMessageDecoder::getNodeText(node, s);
		stringToTepoch(s, &tele->window_end);
	    }
	    else if(DMMessageDecoder::nodeNameIs(name, "arrivals")) {
		getArrivals(node, tele);
	    }
	}
    }

    return tele;
}

void E2XMLParser::getArrivals(DOMNode *node, Teleseism *t)
{
    string name;
    DOMNodeList *children;
    DOMElement *e;
    double distdeg, travel_time;

    if( !(children = node->getChildNodes()) ) {
	return;
    }
    for(XMLSize_t j = 0; j < children->getLength(); j++) {
	DOMNode *node = children->item(j);
	if(node->getNodeType() == DOMNode::ELEMENT_NODE && (e = dynamic_cast< DOMElement* >(node)) ) {
	    DMMessageDecoder::getNodeName(node, name);
	    if(DMMessageDecoder::nodeNameIs(name, "travel_time")) {
		Arrival a;
		if( DMMessageDecoder::getAttributeValue(e, "phase", a.phase) &&
		    DMMessageDecoder::getAttributeValue(e, "distdeg", &distdeg) &&
		    DMMessageDecoder::getNodeText(e, &travel_time))
		{
		    a.time = t->time + travel_time;
		    t->arrivals[distdeg] = a;
		}
	    }
	}
    }
}

/** Convert a time string to a double epochal time value.
 *  Uses qlib2 parse_date() and int_to_nepoch().
 */
bool E2XMLParser::stringToTepoch(string otime_str, double *otime)
{
//  otime_str must be either a double
//  or in this format: "2011-05-06T18:12:37.038"
//  convert to "2011/05/06/18:12:37.038" for qlib2::parse_date()

    char c[25];
    int n = (int)otime_str.length();
    if(n < 20) return false;
    const char *s = otime_str.c_str();
    if(s[4] != '-' || s[7] != '-' || s[10] != 'T') return false;
    strcpy(c, s);
    c[4] = '/';    // replace '-'
    c[7] = '/';    // replace '-';
    c[10] = '/';   // replace 'T'

    INT_TIME *nt = parse_date(c);
    if(nt == NULL) return false;
    *otime = int_to_nepoch(*nt);
    return true;
}

string Teleseism::toShortString()
{
    char line[200];
    snprintf(line, sizeof(line), "%5s %15s %s %9.4f %9.4f %6.1f %4.1f  %s  %s", src.c_str(), id.c_str(),
	TimeString::toString(time, 3).c_str(), lat, lon, depth, mag, TimeString::toString(window_start, 3).c_str(),
	TimeString::toString(window_end, 3).c_str());
    return string(line);
}

string Teleseism::toString()
{
    char line[200];
    stringstream str;

    str << "TE:H:src         eventid             origin_time       lat       lon  depth  mag             window_start               window_end" << endl;
    snprintf(line, sizeof(line), "TE:%5s %15s %s %9.4f %9.4f %6.1f %4.1f  %s  %s", src.c_str(), id.c_str(),
	TimeString::toString(time, 3).c_str(), lat, lon, depth, mag, TimeString::toString(window_start, 3).c_str(),
	TimeString::toString(window_end, 3).c_str());
    str << line << endl;
    str << "TE:A:H:   phase  distdeg            arrival_time" << endl;
    for(map<double, Arrival>::iterator it = arrivals.begin(); it != arrivals.end(); it++) {
	snprintf(line, sizeof(line), "TE:A:%10s %8.4f %s", (*it).second.phase.c_str(), (*it).first,
			 TimeString::toString((*it).second.time, 3).c_str());
	str << line << endl;
    }
    return str.str();
}
