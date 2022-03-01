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
#ifndef __DMMessageEncoder_h
#define __DMMessageEncoder_h

#include <xercesc/dom/DOM.hpp>
#include <cms/TextMessage.h>
#include <cms/ExceptionListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "AlgMessage.h"
#include "DMMessage.h"
#include "GMMessage.h" // JA TESTING

/** Encodes an XML formatted message from one of the CoreEventInfo subclass objects.
 *  @ingroup dm_lib
 */
class DMMessageEncoder
{
 private:
    void cleanup();
    void encodeCoreMessage(xercesc::DOMDocument *doc, CoreEventInfo* cei, std::string comment);
    void encodeContributors(xercesc::DOMDocument *doc, DMContributorIter beg, DMContributorIter end);
    void addContributor(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, DMContributorIter it);
    void encodeGMInfo(xercesc::DOMDocument *doc, AlgMessage &alg);
    void encodeGMInfo(xercesc::DOMDocument *doc, GMMessage &gm);
    void encodeGMMap(xercesc::DOMDocument *doc, xercesc::DOMElement *info, GMMap gmmap);
    void encodeGMContours(xercesc::DOMDocument *doc, xercesc::DOMElement *info, 
            MultiContourIter beg, MultiContourIter end, int nCont);
    void encodeObservations(xercesc::DOMDocument *doc, xercesc::DOMElement *info, GMObservationIter beg,
		GMObservationIter end);
    void encodeObservationNode(xercesc::DOMDocument *doc, xercesc::DOMElement *parent,
		enum ObservationType type, GMObservationIter beg, GMObservationIter end);
    void encodePredictions(xercesc::DOMDocument *doc, xercesc::DOMElement *info, GMPredictionIter beg,
		GMPredictionIter end);
    void encodePredictionNode(xercesc::DOMDocument *doc, xercesc::DOMElement *parent,
		enum PredictionType type, GMPredictionIter beg, GMPredictionIter end);
    void encodeAllFiniteFault(xercesc::DOMDocument *doc, FiniteFaultMessage &finite);
    void encodeAllFiniteFault(xercesc::DOMDocument *doc, DMMessage &dm);
    void encodeFiniteFault(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, FiniteFaultMessage &finite);
    void encodeGlobalUncertainty(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, GlobalUncertainty g);
    void encodeFaultSegment(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, FaultSegment &segment);

    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
			std::string units, int value);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
			std::string units, double value);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
			std::string units, std::string value);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
			double value);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
			int value);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
			std::string value);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
			std::string attnames, std::string attvalues,
            std::vector<double> x, std::vector<double> y);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
            std::string attnames, std::string attvalues, std::string value);
    void addElement(xercesc::DOMDocument *doc, xercesc::DOMElement *parent, const char *name,
            std::vector<gminfo::GM_INFO> griddata);
    xercesc::DOMElement * addNode(xercesc::DOMDocument *doc, xercesc::DOMElement *parent,
			const char *name);
    xercesc::DOMElement * addNode(xercesc::DOMDocument *doc, xercesc::DOMElement *parent,
			const char *name, const char *attribute, int value);
    xercesc::DOMElement * addNode(xercesc::DOMDocument *doc, xercesc::DOMElement *parent,
			const char *name, const char *attribute, string value);
    xercesc::DOMElement *addObsOrPred(xercesc::DOMDocument *doc, const char *obs_or_pred,
			const std::string &sys_name="");

 public:
    DMMessageEncoder() throw (const xercesc::XMLException &);
    virtual ~DMMessageEncoder();
    cms::TextMessage* encodeMessage(cms::TextMessage *message, CoreEventInfo *cei,
		std::string comment="");
    std::string encodeMessage(CoreEventInfo *cei, std::string comment="");
};

#endif
