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
#include <sstream>
#include "FilterBank.h"
#include "EWPacket.h"

FilterWindow::FilterWindow()
{
    window_npts = 0;
    window_lead = 0;
    window_lag = 0;
    memset(pgv, 0, NUM_FBANDS*sizeof(float));
}
 
FilterBank::FilterBank() : Measurement("FilterBank")
{
    init();
}

void FilterBank::init()
{
    for(int i = 0; i < NUM_FWINDOWS; i++) {
	fw[i].window_npts = 0;
    	fw[i].window_lead = 0;
    	fw[i].window_lag = 0;
	memset(fw[i].pgv, 0, NUM_FBANDS*sizeof(float));
    }
}

int FilterBank::serialize(char * &p, int length) throw(std::string)
{
    char *pstart = p;
    startSerialize(p, length);

    addToPacket(status, p);
    int num_windows = (int)(sizeof(fw)/sizeof(FilterWindow));
    int num_fbands = (num_windows > 0) ? (int)(sizeof(fw[0].pgv)/sizeof(float)) : 0;
    addToPacket(num_windows, p);
    addToPacket(num_fbands, p);
    for(int i = 0; i < num_windows; i++)
    {
	addToPacket(fw[i].window_npts, p);
	addToPacket(fw[i].window_lead, p);
	addToPacket(fw[i].window_lag, p);
	for(int j = 0; j < num_fbands; j++) {
	    addToPacket(fw[i].pgv[j], p);
	}
    }
    return (int)(p - pstart);
}

int FilterBank::deserialize(char * &p, int maxlength) throw(std::string)
{
    char *pstart = p;
    startDeserialize(p, maxlength);

    getFromPacket(status, p);
    int num_windows = (int)(sizeof(fw)/sizeof(FilterWindow));
    int num_fbands = (num_windows > 0) ? (int)(sizeof(fw[0].pgv)/sizeof(float)) : 0;
    int num;
    getFromPacket(num, p);
    if(num != num_windows) {
	std::cerr << std::string("FilterBank.deserialize: invalid num_windows: " + num) << std::endl;
        throw std::string("FilterBank.deserialize: invalid num_windows: " + num);
    }
    getFromPacket(num, p);
    if(num != num_fbands) {
	std::cerr << std::string("FilterBank.deserialize: invalid num_fbands: " + num) << std::endl;
        throw std::string("FilterBank.deserialize: invalid num_fbands: " + num);
    }

    for(int i = 0; i < num_windows; i++) {
	getFromPacket(fw[i].window_npts, p);
	getFromPacket(fw[i].window_lead, p);
	getFromPacket(fw[i].window_lag, p);
	for(int j = 0; j < num_fbands; j++) {
	    getFromPacket(fw[i].pgv[j], p);
	}
    }
    return (int)(p - pstart);
}

int FilterBank::packetSize()
{
    int size = headerSize();
    size += sizeof(status);
    int num_windows = (int)(sizeof(fw)/sizeof(FilterWindow));
    int num_fbands = (num_windows > 0) ? (int)(sizeof(fw[0].pgv)/sizeof(fw[0].pgv[0])) : 0;
    size += sizeof(num_windows);
    size += sizeof(num_fbands);
    for(int i = 0; i < num_windows; i++)
    {
	size += sizeof(fw[i].window_npts);
	size += sizeof(fw[i].window_lead);
	size += sizeof(fw[i].window_lag);
	for(int j = 0; j < num_fbands; j++) {
	    size += sizeof(fw[i].pgv[j]);
	}
    }
    return size;
}

std::string FilterBank::toString(std::string prefix)
{
    std::stringstream s;
    for(int i = 0; i < NUM_FWINDOWS; i++) {
	s << prefix << Measurement::toString().c_str() << fw[i].toString();
	if(i < NUM_FWINDOWS-1) s << std::endl;
    }
    return s.str();
}

std::string FilterWindow::toString(bool *tele)
{
    char buf[200];

    snprintf(buf, sizeof(buf), "%.2f %.2f %5d", window_lead, window_lag, window_npts);
    for(int i = 0; i < NUM_FBANDS; i++) {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.3f %s", pgv[i], tele[i] ? "T" : "F");
    }
    return std::string(buf);
}

std::string FilterWindow::toString()
{
    char buf[200];

    snprintf(buf, sizeof(buf), "%.2f %.2f %5d", window_lead, window_lag, window_npts);
    for(int i = 0; i < NUM_FBANDS; i++) {
        snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), " %6.3f", pgv[i]);
    }
    return std::string(buf);
}
