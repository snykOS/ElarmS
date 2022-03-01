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
#include <iostream>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <plog/Log.h>

#include "E2Region.h"
#include "E2ModuleManager.h"
#include "E2Event.h"
#include "E2Prop.h"

using namespace std;

bool E2Region::inRegion(double lat, double lon)
{
    bool in_region = false;
    for(int i = 0; i < (int)coord.size()-1; i++) {
	if( ((coord[i].lat > lat) != (coord[i+1].lat > lat)) &&
	    (lon < coord[i].slope*(lat-coord[i].lat) + coord[i].lon) )
	{
	    in_region = !in_region;
	}
    }
    return in_region;
}

int E2Region::getRegions(const string &name, int nparams, list<E2Region> &regions) throw(Error)
{
    char propname[100];
    string property;
    double lat, lon;
    bool ok = true;
    int i = 0;

    while(++i < 100) {
	E2Region r(nparams);
	snprintf(propname, sizeof(propname), "%s%d", name.c_str(), i);
	try {
	    property = E2Prop::getString(propname);
	}
	catch(Error e) { break; }
	if(property.empty()) break;
        E2ModuleManager::param_str << "P: E2Region." << propname << ": " << property << endl;

	char *c, *d, *tok, *last, *s = strdup(property.c_str());
	tok = s;

	ok = false;
	for(int j = 0; j < nparams; j++) {
	    double param;
	    if((c = strtok_r(tok, ",", &last)) && sscanf(c, "%lf", &param) == 1) {
		tok = NULL;
		r.params.push_back(param);
	    }
	    else {
		break;
	    }
	}
	if((int)r.params.size() != nparams) break;

	while((c = strtok_r(NULL, ",", &last)) && (d = strtok_r(NULL, ",", &last)) &&
		sscanf(c, "%lf", &lat) == 1 && sscanf(d, "%lf", &lon) == 1)
	{
	    r.coord.push_back(Coord(lat, lon));
	}
	free(s);

	int n = r.coord.size();
	if(n <= 1) break;
	if(r.coord[n-1].lat != r.coord[0].lat || r.coord[n-1].lon != r.coord[0].lon) {
	    r.coord.push_back(r.coord[0]); // close the polygon
	}
	// precompute slope
	for(int j = 0; j < n-1; j++) {
	    r.coord[j].slope = (r.coord[j+1].lat != r.coord[j].lat) ?
			(r.coord[j+1].lon-r.coord[j].lon)/(r.coord[j+1].lat-r.coord[j].lat) : 0.;
	}

	regions.push_back(r);
	ok = true;
    }
    if(!ok) {
	E2ModuleManager::param_str << "W: Invalid " << name << ": " << property;
	LOG_INFO << E2ModuleManager::param_str.str();
	E2ModuleManager::param_str << endl;
	E2ModuleManager::sendLogMsg(E2Event::logTime());
	E2ModuleManager::sendLogMsg(E2ModuleManager::param_str.str());
        throw(Error("E2Alert: invalid " + name));
    }
    return (int)regions.size();
}

double E2Region::getDoubleParam()
{
    return params[0];
}

int E2Region::getIntParam()
{
    double param = params[0];
    return (param >= 0.) ? (int)(param + 0.5): -(int)(fabs(param) + 0.5);
}

void E2Region::getDoubleParams(double *values)
{
    for(int i = 0; i < (int)params.size(); i++) values[i] = params[i];
}

void E2Region::getIntParams(int *values)
{
    for(int i = 0; i < (int)params.size(); i++) {
	double param = params[i];
	values[i] = (param >= 0.) ? (int)(param + 0.5): -(int)(fabs(param) + 0.5);
    }
}
