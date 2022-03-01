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
#ifndef _E2XMLParser_h__
#define	_E2XMLParser_h__

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/dom/DOM.hpp>
#include <string>
#include <map>

class Arrival
{
  public:
    std::string phase;
    double time;
};

class Teleseism
{
  public:
    std::string src;
    std::string id;
    double time;
    double lat;
    double lon;
    double depth;
    double mag;
    double window_start;
    double window_end;
    std::map<double, Arrival> arrivals;

    Teleseism(const Teleseism &t) {
	src = t.src;
	id = t.id;
	time = t.time;
	lat = t.lat;
	lon = t.lon;
	depth = t.depth;
	mag = t.mag;
	window_start = t.window_start;
	window_end = t.window_end;
    }
    Teleseism() {}
    std::string toShortString();
    std::string toString();
};

class E2XMLParser
{
  private:
    xercesc::XercesDOMParser *parser;
    xercesc::DefaultHandler *error_handler;
    xercesc::MemBufInputSource *input_source;
    bool do_namespaces;
    bool do_schema;
    bool schema_full_checking;
    bool do_create;

    Teleseism * decodeEvent(xercesc::DOMNode *event_node);
    void getArrivals(xercesc::DOMNode *node, Teleseism *t);

 public:
    E2XMLParser();

    virtual ~E2XMLParser() throw();

    Teleseism * processMessage(std::string &msg);
    static bool stringToTepoch(std::string otime_str, double *otime);

};

#endif
