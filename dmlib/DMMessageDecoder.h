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
#ifndef __DMMessageDecoder_h
#define __DMMessageDecoder_h

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include "DMMessage.h"
#include "FiniteFaultMessage.h"
#include "GMMessage.h"


/** Decodes an XML formatted message to one of the CoreEventInfo subclass objects.
 *  @ingroup dm_lib
 */
class DMMessageDecoder
{
 private:
    xercesc::XercesDOMParser *parser;
    xercesc::DefaultHandler *error_handler;
    xercesc::MemBufInputSource *input_source;
    bool do_namespaces;
    bool do_schema;
    bool schema_full_checking;
    bool do_create;

    CoreEventInfo * decodeEvent(xercesc::DOMNode *event_node,
			CoreEventInfo *cei, bool print = false);
    CoreEventInfo * decodeCoreMessage(xercesc::DOMNode* core_node, CoreEventInfo* cei);
    CoreEventInfo * decodeOldLogMessage(string message);

    bool decodeGMInfo(xercesc::DOMNodeList *event_children, AlgMessage *alg);
    bool decodeObservations(xercesc::DOMNode *info_node, AlgMessage *alg);
    bool decodePredictions(xercesc::DOMNode *info_node, AlgMessage *alg);
    bool decodeGM(xercesc::DOMNodeList *event_children, GMMessage *alg);

    bool decodeContributors(xercesc::DOMNodeList *event_children, DMMessage *dm);
    bool addContributor(xercesc::DOMElement *e, DMMessage *dm);

    bool addGMObservation(xercesc::DOMNode *obs_node, string obs_name, AlgMessage *alg);
    bool addGMPrediction(xercesc::DOMNode *pred_node, string pred_name, AlgMessage *alg);
    bool addGMMap(xercesc::DOMNode *in_node, GMMessage *alg);
    bool addGMContour(xercesc::DOMNode *in_node, GMMessage *alg);

    bool decodeAllFiniteFault(xercesc::DOMNodeList *event_children, DMMessage *dm);
    bool decodeAllFiniteFault(xercesc::DOMNodeList *event_children, FiniteFaultMessage *ff);
    bool decodeFiniteFault(xercesc::DOMNode *in_node, FiniteFaultMessage *ff);
    bool decodeFFSegment(xercesc::DOMNodeList *in_node, FiniteFaultMessage *ff);
    bool decodeFFGlobalUncertainty(xercesc::DOMNodeList *in_node, FiniteFaultMessage *ff);

 public:
    DMMessageDecoder() throw (const xercesc::XMLException &);
    virtual ~DMMessageDecoder();

    CoreEventInfo* decodeMessage(string message,
			CoreEventInfo* cei = NULL, bool print = false);
    CoreEventInfo* decodeMessage(const cms::TextMessage* message,
			CoreEventInfo* cei = NULL, bool print = false) {
	return decodeMessage(message->getText(), cei, print);
    }
    CoreEventInfo* decodeLogMessage(string message, CoreEventInfo* cei = NULL);
    CoreEventInfo* decodeNextLogMessage(FILE *fp, CoreEventInfo* cei = NULL, bool new_style=true);
    int decodeAllLogMessages(FILE *fp, vector<CoreEventInfo *> &v, bool new_style=true);
    void close();

    static bool getAttributeValue(xercesc::DOMElement *e, const char *name, string &value);
    static bool getAttributeValue(xercesc::DOMElement *e, const char *name, int *value);
    static bool getAttributeValue(xercesc::DOMElement *e, const char *name, double *value);
    static bool getNodeText(xercesc::DOMNode *node, double *value);
    static bool getNodeText(xercesc::DOMNode *node, int *value);
    static bool getNodeText(xercesc::DOMNode *node, string &value);
    static bool getNodeName(xercesc::DOMNode *node, string &name);
    static bool nodeNameIs(xercesc::DOMNode *node, const char *s);
    static bool nodeNameIs(string &node_name, const char *s);
    static bool getSNCL(string &sncl, string &sta, string &net, string &chan, string &loc);
};

#endif
