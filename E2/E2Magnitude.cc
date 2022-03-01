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
 * \file   E2Magnitude.cc
 *
 * \author Holly, <hollybrown@berkeley.edu>
 * \date   2010/06/22 Created
 * \date   2011/09/22 Updated for E2research_v3
 * \date   2012 modified by <henson@seismo.berkeley.edu>
 * \brief  Calculates magnitude for events with three or more triggers. 
 * 
 */
#include <iomanip>
#include <plog/Log.h>
#include "E2Magnitude.h"
#include "E2ModuleManager.h"
#include "E2Event.h"
#include "deltaz.h"
#include "E2Prop.h"
#include <math.h>

double E2Magnitude::minTaupSNR;	// minimum TauP SNR to include TauP magnitude when useTaupMag is true
double E2Magnitude::minPdSNR;	// minimum Pd SNR to include Pd magnitude from PdSNRregex channels
double E2Magnitude::minPvSNR;	// minimum Pv SNR to include Pd magnitude from PvSNRregex channels
bool E2Magnitude::useTaupMag;	// use tpmag anywhere
string E2Magnitude::PdSNRregex;	// network:channel regex pattern for channels that use PdSNR
string E2Magnitude::PvSNRregex;	// network:channel regex pattern for channels that use PvSNR
regex_t E2Magnitude::regex_pd;
regex_t E2Magnitude::regex_pv;
int E2Magnitude::verbose;
list<E2Region> E2Magnitude::maxMagDistRegions; // optional polygon regions with different values of MaxMagDistkm
list<E2Region> E2Magnitude::pdMagRegions; // optional polygon regions with different values of PdMag coefficients
list<E2Region> E2Magnitude::tpMagRegions; // optional polygon regions with different values of TaupMag coefficients
double E2Magnitude::max_mag_km = 200.;  // do not compute station magnitude for distances > max_mag_km
double E2Magnitude::pdmag_coefs[3] = {5.39, 1.23, 1.38};
double E2Magnitude::tpmag_coefs[2] = {5.2215, 6.6589};
bool E2Magnitude::weightByTime = true;


E2Magnitude::E2Magnitude() throw(Error)
{
    minTaupSNR = E2Prop::getDouble("MinTaupSNR");  // minimum TauP SNR to include TauP magnitude (0.5)
    minPdSNR = E2Prop::getDouble("MinPdSNR"); // minimum Pd SNR to include Pd magnitude from PdSNRregex channels (0.5)
    minPvSNR = E2Prop::getDouble("MinPvSNR"); // minimum Pv SNR to include Pd magnitude from PvSNRregex channels (0.5)
    E2Prop::getDoubleArray("PdmagCoefficients", 3, pdmag_coefs);
    E2Prop::getDoubleArray("TpmagCoefficients", 2, tpmag_coefs);
    useTaupMag = E2Prop::getBool("UseTaupMag"); // if false, do not use taupmag anywhere.
    max_mag_km = E2Prop::getDouble("MaxMagDistkm"); // do not compute station magnitude for distances > max_mag_km
    // "CI:.*|AZ:.*|.*:.H." network:channel regex pattern for channels that use PdSNR
    PdSNRregex = E2Prop::getString("PdSNRregex");
    // ".*:.L.|.*:.N." network:channel regex pattern for channels that use PvSNR
    PvSNRregex = E2Prop::getString("PvSNRregex");
    verbose = E2Prop::getInt("Verbose");

    E2ModuleManager::param_str << fixed << setprecision(2);
    E2ModuleManager::param_str << "P: E2Magnitude.MinTaupSNR: " << minTaupSNR << endl;
    E2ModuleManager::param_str << "P: E2Magnitude.MinPdSNR: " << minPdSNR << endl;
    E2ModuleManager::param_str << "P: E2Magnitude.MinPvSNR: " << minPvSNR << endl;
    E2ModuleManager::param_str << "P: E2Magnitude.PdmagCoefficients: " <<pdmag_coefs[0]<< ", "<<pdmag_coefs[1]<< ", "<<pdmag_coefs[2]<<endl;
    E2ModuleManager::param_str << "P: E2Magnitude.TpmagCoefficients: " <<tpmag_coefs[0]<< ", "<<tpmag_coefs[1]<< endl;
    E2ModuleManager::param_str << "P: E2Magnitude.UseTaupMag: " << useTaupMag << endl;
    E2ModuleManager::param_str << "P: E2Magnitude.MaxMagDistkm: " << max_mag_km << endl;
    E2ModuleManager::param_str << "P: E2Magnitude.PdSNRregex: " << PdSNRregex << endl;
    E2ModuleManager::param_str << "P: E2Magnitude.PvSNRregex: " << PvSNRregex << endl;

    E2Region::getRegions(string("MaxMagDistRegion"), 1, maxMagDistRegions);
    E2Region::getRegions(string("pdMagRegion"), 3, pdMagRegions);
    E2Region::getRegions(string("tpMagRegion"), 2, tpMagRegions);

    if(regcomp(&regex_pd, PdSNRregex.c_str(), REG_ICASE|REG_EXTENDED)) {
	LOG_INFO << "Could not compile regex for " << PdSNRregex;
        E2ModuleManager::sendLogMsg(E2Event::logTime());
        E2ModuleManager::sendLogMsg(E2ModuleManager::param_str.str());
        throw(Error("E2Magnitude: invalid PdSNRregex"));
    }
    if(regcomp(&regex_pv, PvSNRregex.c_str(), REG_ICASE|REG_EXTENDED)) {
	LOG_INFO << "Could not compile regex for " << PvSNRregex;
        E2ModuleManager::sendLogMsg(E2Event::logTime());
        E2ModuleManager::sendLogMsg(E2ModuleManager::param_str.str());
        throw(Error("E2Magnitude: invalid PvSNRregex"));
    }
    weightByTime = E2Prop::getBool("WeightByTime");
    E2ModuleManager::param_str << "P: E2Magnitude.WeightByTime: " << weightByTime << endl;
}

int E2Magnitude::calcMag(E2Event *event)
{
    double sumPDmag=0, sumTPmag=0;
    double pd_total_time=0, tp_total_time=0;
    double coefs[3];
    double mindist, maxdist;
    list<E2Trigger *> pd_trigs, tp_trigs;

    event->pdmag = 0.;
    event->tpmag = 0.;
    event->evmag = 0.;

    mindist = 1.e+60;
    for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++)
    {
	E2Trigger *t = (*it);
	checkSNR(t);
	t->distkm = distancekm(t->getLat(), t->getLon(), event->lat, event->lon);
	if(t->pdSNRok) {
	    if(t->distkm < mindist) mindist = t->distkm;
	}
    }

    if(mindist > 150.) maxdist = maxMagKm(event);
    else if(mindist > 100.) maxdist = 150.;
    else maxdist = 100.;

    for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++)
    {
	E2Trigger *t = (*it);

	t->pdmag_used = false;
	t->tpmag_used = false;

	// Do not use triggers which are farther than maxdist from epicenter
	t->magdistok = true;
	double dist;
	if(inRegion(event, &dist)) {
	    t->magdistok = (t->distkm <= dist);
	}
	else {
	    t->magdistok = (t->distkm <= maxdist);
	}

	checkSNR(t);
	if(!t->magchan_ok) {
	    stringstream msg_str;
	    msg_str << "W: E2Magnitude: cannot compute magnitude for channel type: " << t->getStaChan();
	    LOG_INFO << msg_str.str();
	    msg_str << endl;
	    E2ModuleManager::sendLogMsg(msg_str.str());
	}

	// calculate Pd Mag 
	if(inRegion(pdMagRegions, t, coefs)) {
	    if(t->distkm > 0.) {
		t->pdmag = coefs[0] + coefs[1]*log10(fabs(t->getPd())) + coefs[2]*log10(t->distkm);
	    }
	    else { // distkm = 0 for single station event
		t->pdmag = coefs[0] + coefs[1]*log10(fabs(t->getPd()));
	    }
	}
	else {
	    if(t->distkm > 0.) {
		t->pdmag = pdmag_coefs[0] + pdmag_coefs[1]*log10(fabs(t->getPd())) + pdmag_coefs[2]*log10(t->distkm);
	    }
	    else { // distkm = 0 for single station event
		t->pdmag = pdmag_coefs[0] + pdmag_coefs[1]*log10(fabs(t->getPd()));
	    }
	}
	t->pdmag_ok = (t->pdmag > 0 && t->pdmag < 9);

	if(t->magdistok && t->magchan_ok && t->pdSNRok && t->pdmag_ok) {
	    t->pdmag_used = true;
            pd_trigs.push_back(t);
        }

	// calculate Tp Mag 
	if(inRegion(tpMagRegions, t, coefs)) {
	    t->tpmag = coefs[0] + coefs[1]*t->getLogTaup();
	}
	else {
	    t->tpmag = tpmag_coefs[0] + tpmag_coefs[1]*t->getLogTaup();
	}
	t->tpmag_ok = (t->tpmag > 0 && t->tpmag < 9);
	t->tpSNRok = (t->getTaupSnr() >= minTaupSNR); // don't use triggers with SNR too small

	if(t->magdistok && t->magchan_ok && t->tpSNRok && t->tpmag_ok) {
	    t->tpmag_used = true;
            tp_trigs.push_back(t);
	}
    }

    // weight the trigger magnitudes by the time window available after the trigger

    pd_total_time = 0.;
    sumPDmag = 0.;
    for(list<E2Trigger *>::iterator it = pd_trigs.begin(); it != pd_trigs.end(); it++) {
        double time_window = weightByTime ? (*it)->z_recent_sample/(*it)->samprate : 1.0;
        sumPDmag += (*it)->pdmag*time_window;
        pd_total_time += time_window;
    }

    tp_total_time = 0.;
    sumTPmag = 0.;
    for(list<E2Trigger *>::iterator it = tp_trigs.begin(); it != tp_trigs.end(); it++) {
        double time_window = weightByTime ? (*it)->z_recent_sample/(*it)->samprate : 1.0;
        sumTPmag += (*it)->tpmag*time_window;
        tp_total_time += time_window;
    }
        

    int nmag = 0;
    if(pd_total_time > 0.) {
	event->pdmag = sumPDmag/pd_total_time;
	event->evmag += event->pdmag;
	nmag++;
    }
    if(tp_total_time > 0.) {
	event->tpmag = sumTPmag/tp_total_time;
	if(useTaupMag) {
	    event->evmag += event->tpmag;
	    nmag++;
	}
    }
    if(nmag > 0) {
	event->evmag /= (double)nmag;
    }

    if(verbose >= 5) {
	stringstream msg_str;
	char s[200];
	for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++)
	{
	    E2Trigger *t = (*it);
	    snprintf(s, sizeof(s), "M: E2Magnitude: %s  dist: %7.2f  pdmag: %5.2f used: %d  tpmag: %5.2f used: %d",
		t->timeString().c_str(), t->distkm, t->pdmag, t->pdmag_used, t->tpmag, t->tpmag_used);
	    msg_str << s << endl;
	}
	snprintf(s, sizeof(s), "M: E2Magnitude: Event mag: %5.2f  pdmag: %5.2f  tpmag: %5.2f",
		event->evmag, event->pdmag, event->tpmag);
	msg_str << s;
	LOG_INFO << msg_str.str();
	msg_str << endl;
	E2ModuleManager::sendLogMsg(msg_str.str());
    }

    if(nmag > 0) { // if there is a good magnitude, return the event id 
	return 1;
    }
    else { // if there is not a good magnitude, return 0 
	if(verbose >= 5) {
	    stringstream msg_str;
	    msg_str << "M: E2Magnitude: evmag=0 for " << event->toShortString();
	    LOG_INFO << msg_str.str();
	    msg_str << endl;
	    E2ModuleManager::sendLogMsg(msg_str.str());
	}
	return 0;
    }
}

bool E2Magnitude::inRegion(E2Event *event, double *distance)
{
    for(list<E2Region>::iterator it = maxMagDistRegions.begin(); it != maxMagDistRegions.end(); it++) {
	if((*it).inRegion(event->lat, event->lon)) {
	    *distance = (*it).getDoubleParam();
	    return true;
        }
    }
    return false;
}

bool E2Magnitude::inRegion(list<E2Region> &regions, E2Trigger *t, double *coefs)
{
    for(list<E2Region>::iterator it = regions.begin(); it != regions.end(); it++) {
	if((*it).inRegion(t->getLat(), t->getLon())) {
	    (*it).getDoubleParams(coefs);
	    return true;
        }
    }
    return false;
}

void E2Magnitude::checkSNR(E2Trigger *t)
{
    string netchan = string(t->getNet()) + ":" + string(t->twoCharChan()) + "Z";

    t->magchan_ok = false;
    t->pdSNRok = false;
    if(regexec(&regex_pd, netchan.c_str(), 0, NULL, 0)==0) {
	t->pdSNRok = (t->getPdSnr() >= minPdSNR);
	t->magchan_ok = true;
    }
    else if(regexec(&regex_pv, netchan.c_str(), 0, NULL, 0)==0) {
	t->pdSNRok = (t->getPvSnr() >= minPdSNR);
	t->magchan_ok = true;
    }
}

double E2Magnitude::maxMagKm(E2Event *event)
{
    double region_max_dist;
    if(inRegion(event, &region_max_dist)) {
       return region_max_dist;
    }
    return max_mag_km;
}
