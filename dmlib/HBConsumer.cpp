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
#include "HBConsumer.h"
#include "plog/Log.h"                               // plog logging library
#include <unistd.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/TransService.hpp>

static char * stringTrim(char *str);

using namespace std;
using namespace cms;
using namespace xercesc;

HBConsumer::HBConsumer(Connection *conn, const string& hbtopic) :
			connection(conn), session(NULL), destination(NULL),
			consumer(NULL), topic(hbtopic)
{
    XMLPlatformUtils::Initialize();
    parser = new XercesDOMParser();
    error_handler = new DefaultHandler();
    parser->setErrorHandler(error_handler);
    parser->setValidationScheme(XercesDOMParser::Val_Always);
    parser->setDoNamespaces(false);
    parser->setDoSchema(false);
//    parser->setHandleMultipleImports (true);
    parser->setValidationSchemaFullChecking(false);
    parser->setCreateEntityReferenceNodes(false);

    input_source = new MemBufInputSource((const XMLByte *)NULL, 0, "HBConsumer");

    session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
    destination = session->createTopic( topic );
    consumer = session->createConsumer( destination );
    consumer->setMessageListener(this);
}

HBConsumer::~HBConsumer() throw()
{
    close();
}

void HBConsumer::onMessage(const Message *message)
{
    XMLCh tag[100];
    DOMNodeList *node_list;
    const TextMessage *text = dynamic_cast< const TextMessage* >(message);
    string s = text->getText();

    try {
	input_source->resetMemBufInputSource((const XMLByte *)s.data(),
			(XMLSize_t)s.length()*sizeof(char));
    }
    catch(const XMLException& toCatch) {
	char* msg = XMLString::transcode(toCatch.getMessage());
	cerr << "HBConsumer: Exception message is:" << endl << msg << endl;
	XMLString::release(&msg);
	return;
    }
    catch(const DOMException& toCatch) {
	char* msg = XMLString::transcode(toCatch.msg);
	cerr << "HBConsumer: Exception message is:" << endl << msg << endl;
	XMLString::release(&msg);
	return;
    }
    catch(...) {
	cerr << "HBConsumer: Unexpected Exception" << endl;
	return;
    }

    try {
	parser->parse(*input_source);
    }
    catch(const DOMException& e) {
	char* msg = XMLString::transcode(e.msg);
	cerr << "HBConsumer: DOMException message is: \n" << msg << "\n";
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return;
    }
    catch(const XMLException& e) {
	char* msg = XMLString::transcode(e.getMessage());
	cerr << "HBConsumer: XMLException message is: \n" << msg << "\n";
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return;
    }
    catch(const SAXException& e) {
	char* msg = XMLString::transcode(e.getMessage());
	cerr << "HBConsumer: SAXException message is: \n" << msg << "\n";
	XMLString::release(&msg);
	parser->resetDocumentPool();
	return;
    }
    catch (...) {
	cerr << "HBConsumer: parser failed." << endl;
	parser->resetDocumentPool();
	return;
    }

    DOMDocument *doc = parser->getDocument();

    XMLString::transcode("hb", tag, 99);
    node_list = doc->getElementsByTagName(tag);

    for(XMLSize_t i = 0; i < node_list->getLength() && i < 1; i++) {
	DOMNode *node = node_list->item(i);
	if(node->getNodeType() == DOMNode::ELEMENT_NODE)
	{
	    updateHBInfo(node);
	}
    }
    parser->resetDocumentPool();
}

void HBConsumer::updateHBInfo(DOMNode *node)
{
    DOMElement *element = dynamic_cast< DOMElement* >(node);

    if(	!getAttributeValue(element, "sender", sender) ||
	!getAttributeValue(element, "originator", originator) ||
	!getAttributeValue(element, "timestamp", timestamp) )
    {
	cerr << "HBConsumer::updateHBInfo: invalid message." << endl;
    }
}

bool HBConsumer::getAttributeValue(DOMElement *e, const char *name, string &value)
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
	    return true;
	}
    }
    return false;
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

void HBConsumer::close() 
{
    if( !connection ) return;

    connection = NULL;

    // Destroy resources.
    try{
	if( consumer != NULL ) delete consumer;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    consumer = NULL;
    
    try{
	if( destination != NULL ) delete destination;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    destination = NULL;
    
    try{
	if( session != NULL ) session->close();
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    
    try{
	if( session != NULL ) delete session;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    session = NULL;

    try{
	if( input_source != NULL ) delete input_source;
    }catch ( CMSException& e ) { 
        LOGE << e.getStackTraceString();
    }
    input_source = NULL;

    delete parser;
    parser = NULL;

    delete error_handler;

    XMLPlatformUtils::Terminate();
}
