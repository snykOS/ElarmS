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
using namespace std;
#include "AlgorithmEvent.h"

AlgorithmEvent::AlgorithmEvent(CoreEventInfo *cei, bool use_core)
{
    message_category = cei->getMessageCategory();
    system_name = cei->getSystemName();
    system_name_string = cei->getSystemNameString();
    id = cei->getID();
    program_instance = cei->getProgramInstance();
    use_core_info = use_core;
}

bool AlgorithmEvent::operator==(const AlgorithmEvent &A) const
{
    return ( A.message_category == message_category && A.system_name == system_name
		&& A.id == id && A.program_instance == program_instance);
}

bool AlgorithmEvent::operator!=(const AlgorithmEvent &A) const
{
    return !(*this==A);
}

// static 
bool AlgorithmEvent::valuesOK(CoreEventInfo *cei, DMConstants &c, bool verbose, stringstream &os)
{
    if (!(c.min_mag <= cei->getMagnitude() && cei->getMagnitude() <= c.max_mag)) {
	if(verbose) os << "Magnitude must be between: " << c.min_mag << " and " << c.max_mag << endl;
	return false;
    }
    if (!(c.min_depth_km <= cei->getDepth() && cei->getDepth() <= c.max_depth_km)) {
	if(verbose) os << "Depth must be between: " << c.min_depth_km << " and " << c.max_depth_km << endl;
	return false;
    }
    if (!(c.min_likelihood <= cei->getLikelyhood() && cei->getLikelyhood() <= 1.0)) {
	if(verbose) os << "Likelihood must be between: 0. and " << 1.0 << endl;
	return false;
    }

    if(cei->getSystemName() == GPSLIP) return true;

    if (!(0. <= cei->getMagnitudeUncertainty() && cei->getMagnitudeUncertainty() <= c.max_mag_uncer)) {
	if(verbose) os << "Magnitude Uncertainty must be between: 0. and " << c.max_mag_uncer << endl;
	return false;
    }
    if (!(0. <= cei->getLatitudeUncertainty() && cei->getLatitudeUncertainty() <= c.max_lat_uncer)) {
	if(verbose) os << "Latitude Uncertainty must be between: 0. and " << c.max_lat_uncer << endl;
	return false;
    }
    if (!(0. <= cei->getLongitudeUncertainty() && cei->getLongitudeUncertainty() <= c.max_lon_uncer)) {
	if(verbose) os << "Longitude Uncertainty must be between: 0. and " << c.max_lon_uncer << endl;
	return false;
    }
    if (!(0. <= cei->getDepthUncertainty() && cei->getDepthUncertainty() <= c.max_depth_uncer)) {
	if(verbose) os << "Depth Uncertainty must be between: 0. and " << c.max_depth_uncer << endl;
	return false;
    }
    if (!(0. <= cei->getOriginTimeUncertainty() && cei->getOriginTimeUncertainty() <= c.max_otime_uncer)) {
	if(verbose) os << "Origin Time Uncertainty must be between: 0. and " << c.max_otime_uncer << endl;
	return false;
    }
    return true;
}

string AlgorithmEvent::toString()
{
    string category = (message_category == LIVE) ? "live" : "test";

    return system_name_string + "," + program_instance + "," + id + "," + category;
}
