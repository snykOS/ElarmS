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
 * \file   E2Location.cc
 * \author Holly, <hollybrown@berkeley.edu>
 * \date   2010/06/22 Created
 * \date   2012 modified by <henson@seismo.berkeley.edu>
 * \brief  Calculates location for events with three or more triggers.
 * \ Mimicked from evm_evlocate.f, evlocgrid subroutine.
 */
#include <vector>
#include <set>
#include <iomanip>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <plog/Log.h>
#include "E2Location.h"
#include "E2ModuleManager.h"
#include "E2Event.h"
#include "E2Prop.h"
#include "E2Alert.h"
#include "deltaz.h"

float * E2Location::stt = NULL;

extern "C" {
static void * SearchGrid(void *client_data);
}

E2Location::E2Location(E2TriggerManager *trigm, E2EventManager *evtm, string ttfile) throw(Error)
			: depthsearch(false), tm(trigm), em(evtm)
{
    if(!readTTFile(ttfile)) {
	throw(Error("E2Location: error reading travel time file"));
    }

    grid_size = E2Prop::getInt("GridSize"); // 250
    grid_km = E2Prop::getDouble("GridKm"); // 500.
    search_size = E2Prop::getInt("SearchSize"); // 50
    small_grid_size = E2Prop::getInt("SmallGridSize"); // 3
    p_velocity = E2Prop::getDouble("LocationPVelocity"); // 6.0
    try {
	// true:  use all Z-channel triggers for a station (HNZ,HHZ,...)
	// false: use only the earliest Z-channel trigger for a station
	loc_all_zchans = E2Prop::getBool("LocAllZChans");
    }
    catch(Error e) { loc_all_zchans = true; }

    if(search_size > grid_size) {
	E2ModuleManager::param_str << "W: E2Location:: SearchSize(" << search_size << ") > GridSize(" << grid_size << ")";
	LOG_INFO << E2ModuleManager::param_str.str();
	E2ModuleManager::param_str << endl;
	throw(Error(E2ModuleManager::param_str.str()));
    }
    num_prevlocs = 0;
    size_prevlocs = 1000;
    prevlocs = (Location **)malloc(size_prevlocs*sizeof(Location *));

    // precompute grid travel times. Stations locations become the nearest grid point.
    grid_width = 2*grid_size + 1;
    dx = grid_km/grid_size;
    dy = grid_km/grid_size;
    for(int k = 0; k < (int)tt_depths.size(); k++) {
	tt_depths[k].stt = new float[grid_width*grid_width];
    }

    int ndep = (int)tt_depths.size();
    for(int k = 0; k < ndep; k++) {
	DepthTT *d = &tt_depths[k];
	double dist, xi, yj;
	int l;
	int n = 0;
	for(int i = 0; i <= 2*grid_size; i++) {
	    xi = dx*i;
	    for(int j = 0; j <= 2*grid_size; j++) {
		yj = dy*j;
		dist = sqrt( (double)(xi*xi + yj*yj) );

		for(l = 1; l < (int)d->v.size() && d->v[l].dist < dist; l++);
		if(l < (int)d->v.size()) { // use ttgen and hauksson velocity model
		    d->stt[n++] = d->v[l-1].tt + d->v[l].slow*(dist-d->v[l-1].dist);
		}
		else { // simply use velocity of 6km/s
		    d->stt[n++] = dist/p_velocity;
		}
	    }
	}
    }

    bestex = new double[ndep];
    bestey = new double[ndep];
    bestevOT = new double[ndep];
    misfit_rms = new double[ndep];
    misfit_ave = new double[ndep];
    latitude = new double[ndep];
    longitude = new double[ndep];
    min_sta_dist = new double[ndep];

    E2ModuleManager::param_str << fixed << setprecision(2);
    E2ModuleManager::param_str << "P: E2Location.GridSize: " << grid_size << endl;
    E2ModuleManager::param_str << "P: E2Location.GridKm: " << grid_km << endl;
    E2ModuleManager::param_str << "P: E2Location.SearchSize: " << search_size << endl;
    E2ModuleManager::param_str << "P: E2Location.SmallGridSize: " << small_grid_size << endl;
    E2ModuleManager::param_str << "P: E2Location.LocationPVelocity: " << p_velocity << endl;

    search_thread[0].jbeg = -search_size;
    search_thread[0].jend = -search_size/2.;
    search_thread[1].jbeg = search_thread[0].jend + 1;
    search_thread[1].jend = 0;
    search_thread[2].jbeg = 1;
    search_thread[2].jend = search_size/2.;
    search_thread[3].jbeg = search_thread[2].jend + 1;
    search_thread[3].jend = search_size;

    for(int i = 0; i < 4; i++) {
	search_thread[i].parent = this;
    }
}

bool E2Location::readTTFile(string ttfile)
{
    // depthsearch = false => search only in flat plane
    // hauksson velocity model (fixed 8 km depth)
    FILE *fp;
    stringstream msg_str;

    if((fp = fopen(ttfile.c_str(), "r")) == NULL) {
	msg_str << "W: Cannot open travel time file: " << ttfile << endl << strerror(errno);
	LOG_INFO << msg_str.str();
	msg_str << endl;
	E2ModuleManager::sendLogMsg(msg_str.str());
	return false;
    }
    char line[100], *c=NULL;
    bool have_depth = false;
    DepthTT tt;
    TtPoint tp;
    int lineno = 0;

    while(fgets(line, sizeof(line)-1, fp) != NULL) {
        lineno++;
        if((c = strstr(line, "depth")) != NULL) {
            if(tt.v.size() > 0) {
		tt.v[0].slow = 0.;
                for(int i = 1; i < (int)tt.v.size(); i++) {
		    double d_dist = tt.v[i].dist - tt.v[i-1].dist;
                    tt.v[i].slow = (d_dist != 0) ? (tt.v[i].tt-tt.v[i-1].tt)/d_dist : 0.;
                }
                tt_depths.push_back(tt);
            }
            tt.v.clear();
            if(sscanf(c+6, "%lf", &tt.depth) != 1) {
                msg_str << "W: Format error: " << ttfile << endl << "invalid depth line " << lineno;
		LOG_INFO << msg_str.str();
		msg_str << endl;
		E2ModuleManager::sendLogMsg(msg_str.str());
		fclose(fp);
                return false;
            }
	    have_depth = true;
        }
	else if(!have_depth) {
	    msg_str << "W: Format error: " << ttfile << endl << "depth not specified. line " << lineno;
	    LOG_INFO << msg_str.str();
	    msg_str << endl;
	    E2ModuleManager::sendLogMsg(msg_str.str());
	    fclose(fp);
            return false;
	}
        else if(sscanf(line, "%f %f", &tp.dist, &tp.tt) == 2) {
            tt.v.push_back(tp);
        }
        else {
	    msg_str << "W: Format error: " << ttfile << endl << "line " << lineno;
	    LOG_INFO << msg_str.str();
	    msg_str << endl;
	    E2ModuleManager::sendLogMsg(msg_str.str());
	    fclose(fp);
            return false;
        }
    }
    if((int)tt.v.size() > 0) {
	tt.v[0].slow = 0.;
        for(int i = 1; i < (int)tt.v.size(); i++) {
	    double d_dist = tt.v[i].dist - tt.v[i-1].dist;
	    tt.v[i].slow = (d_dist != 0) ? (tt.v[i].tt-tt.v[i-1].tt)/d_dist : 0.;
        }
        tt_depths.push_back(tt);
    }
    fclose(fp);
    return true;
}

int E2Location::locate(E2Event *event, LocateInfo &loc_info)
{
    // number of unique stations (each station can have multiple triggers) 
    int nstats = E2Alert::countStations(event, tm);

    // Initialize search variables
    double evlat = event->lat;
    double evlon = event->lon;
    double evdepth;
    double evtime;
    int i, imin, jmin, kmin;

    loc_info.initial_lat = event->lat;
    loc_info.initial_lon = event->lon;
    loc_info.initial_depth = event->depth;
    loc_info.initial_time = event->time;

    // SINGLE STATION LOCATION
    if(nstats == 1)
    {
	for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++) {
	    evlat = (*it)->getLat(); // event latitude is trigger latitude
	    evlon = (*it)->getLon(); // event longitude is trigger longitude
	    evtime = (*it)->trigger_time - event->depth/p_velocity;
	}

	// set new event location 
	event->setLat(evlat); // latitude (degrees)  
	event->setLon(evlon); // longitude (degrees)  
	event->setTime(evtime); // origin time (seconds)  
	event->misfit_rms = 0.;
	event->misfit_ave = 0.;

	loc_info.eventid = event->eventid;
	loc_info.version = event->version;
	loc_info.lat = evlat;
	loc_info.lon = evlon;
	loc_info.depth = event->depth;
	loc_info.time = evtime;
	loc_info.misfit_rms = 0.;
	loc_info.misfit_ave = 0.;
	for(list<LocateTrigger>::iterator it = loc_info.trigs.begin(); it != loc_info.trigs.end(); it++) {
	    (*it).dist = 0.;
	    (*it).tt = 0.;
	    (*it).tterr = 0.;
	    (*it).used = true;
	}
	return event->eventid;
    }

    // TWO STATION LOCATION
    if(nstats == 2)
    {
	double lat1, lon1, trigtime1, lat2, lon2, trigtime2, dist1, dist2;
	double travtime1, travtime2, origtime1, origtime2; 
	int counter = 1;
	for(set<E2Trigger *>::iterator it = event->triggers.begin(); it != event->triggers.end(); it++)
	{
	    if(counter==1) // first station 
	    {
		lat1 = (*it)->getLat();
		lon1 = (*it)->getLon();
		trigtime1 = (*it)->trigger_time;
		counter++;
		continue;
	    }
	    else if( (*it)->getLat() != lat1 && (*it)->getLon() != lon1 ) // second station 
	    {
		lat2 = (*it)->getLat();
		lon2 = (*it)->getLon();
		trigtime2 = (*it)->trigger_time;
		break;
	    }
	}
    
	// event location is closer to station with earlier trigger.
	if(trigtime1 < trigtime2) {
	    evlat = (lat1*2 + lat2) / 3;
	    evlon = (lon1*2 + lon2) / 3;
	}
	else {
	    evlat = (lat2*2 + lat1) / 3;
	    evlon = (lon2*2 + lon1) / 3;
	}

	// event origin time determined from event location and station arrival times
	dist1 = distancekm(lat1, lon1, evlat, evlon);
	dist2 = distancekm(lat2, lon2, evlat, evlon);
	travtime1 = sqrt(dist1*dist1 + event->depth*event->depth) / p_velocity;  
	travtime2 = sqrt(dist2*dist2 + event->depth*event->depth) / p_velocity;
	origtime1 = trigtime1 - travtime1; 
	origtime2 = trigtime2 - travtime2;  
	evtime = (origtime1 + origtime2) / 2;

	// insert new event location 
	event->setLat(evlat); // latitude (degrees)  
	event->setLon(evlon); // longitude (degrees)  
	event->setTime(evtime); // origin time (seconds)  
	event->misfit_rms = sqrt(pow(origtime1 - evtime, 2) + pow(origtime2 - evtime, 2));
	event->misfit_ave = (fabs(origtime1 - evtime) + fabs(origtime2 - evtime))/2.;
 
	loc_info.eventid = event->eventid;
	loc_info.version = event->version;
	loc_info.lat = evlat;
	loc_info.lon = evlon;
	loc_info.depth = event->depth;
	loc_info.time = evtime;
	loc_info.misfit_rms = event->misfit_rms;
	loc_info.misfit_ave = event->misfit_ave;
	for(list<LocateTrigger>::iterator it = loc_info.trigs.begin(); it != loc_info.trigs.end(); it++) {
	    (*it).dist = 0.; // fix
	    (*it).tt = 0.; // fix
	    (*it).tterr = 0.; // fix
	    (*it).used = true;
	}

	return event->eventid;
    }

    // MULTI STATION LOCATION
    num_trigs = 0;
    for(set<E2Trigger *>::iterator jt = event->triggers.begin(); jt != event->triggers.end(); jt++) {
	E2Trigger *t = (*jt);
	if(loc_all_zchans) {
	    if(num_trigs < MAX_EVENT_TRIGS) {
		trigs[num_trigs++] = TrigLoc(t, true);
	    }
	}
	else {
	    // use only the earliest trigger (HHZ, HNZ,...) at a station
	    bool use = true;
	    for(i = 0; i < num_trigs; i++) {
		if(t->staId() == trigs[i].sta_id) {
		    if(trigs[i].use_in_location) {
			if(trigs[i].time <= t->trigger_time) {
			    use = false;
			}
			else {
			    trigs[i].use_in_location = false;
			}
		    }
		}
	    }
	    if(num_trigs < MAX_EVENT_TRIGS) {
		trigs[num_trigs++] = TrigLoc(t, use);
	    }
	}
    }

    if(previousLocation(event, loc_info)) return event->eventid;

    // Find event and station coordinates on x-y grid (flat earth). (evm_evlocate.f line 262)
    latLonToXY(evlat, evlon);

    if(num_on_grid < 3) return 0;

    kmin = 0;
    double min_misfit_rms = 0;
    // Scan across grid looking for best location
    for(int k = 0;  k < (int)tt_depths.size(); k++) {
	depth_k = k;
	scanGrid(&imin, &jmin, &misfit_rms[k]);
	scanSmallGrid(&imin, &jmin, &misfit_rms[k], &misfit_ave[k], &bestex[k], &bestey[k], &bestevOT[k]);
	if(k == 0 || misfit_rms[k] < min_misfit_rms) {
	    kmin = k;
	    min_misfit_rms = misfit_rms[k];
	}
	xyToLatLon(evlat, evlon, bestex[k], bestey[k], &latitude[k], &longitude[k]);

	// get minimum station distance
	double distmin=0;
	for(i = 0; i < num_trigs; i++) {
	    double dist = distancekm(latitude[k], longitude[k], trigs[i].lat, trigs[i].lon);
	    if(i == 0 || dist < distmin) {
		distmin = dist;
	    }
	}
	min_sta_dist[k] = distmin;
    }

/*
    // use the min_misfit_rms solution only if the corresponding min_sta_dist is < 10.
//    if(kmin == 0 && min_sta_dist[kmin] > 12.) {
    if(kmin == 0 && min_sta_dist[kmin] > 20.) {
	kmin = 1; // the depth = 8 index
    }
//    else if(kmin == 2 && min_sta_dist[kmin] > 32.) {
    else if(kmin == 2 && min_sta_dist[kmin] > 40.) {
	kmin = 1; // the depth = 8 index
    }
*/
    evlat = latitude[kmin];
    evlon = longitude[kmin];
    evdepth = tt_depths[kmin].depth;

    for(i = 0; i < num_trigs; i++) {
	trigs[i].dist = distancekm(evlat, evlon, trigs[i].lat, trigs[i].lon);
	trigs[i].tt = trigs[i].time - bestevOT[kmin];
    }

    // remove old locations
    removeOldLocations(event);

    Location *loc = new Location();
    loc->initial_lat = event->lat;
    loc->initial_lon = event->lon;
    loc->initial_time = event->time;
    for(i = 0; i < num_trigs; i++) {
	loc->trigs[i] = trigs[i];
    }
    loc->num_trigs = num_trigs;

    // set new location
    event->setLat(evlat);
    event->setLon(evlon);
    event->setDepth(evdepth);
    event->setTime(bestevOT[kmin]);
    event->misfit_rms = misfit_rms[kmin];
    event->misfit_ave = misfit_ave[kmin];

    loc_info.eventid = event->eventid;
    loc_info.version = event->version;
    loc_info.lat = event->lat;
    loc_info.lon = event->lon;
    loc_info.bestex = bestex[kmin];
    loc_info.bestey = bestey[kmin];
    loc_info.depth = event->depth;
    loc_info.depth_kmin = kmin;
    loc_info.time = event->time;
    loc_info.misfit_rms = event->misfit_rms;
    loc_info.misfit_ave = event->misfit_ave;
    for(list<LocateTrigger>::iterator it = loc_info.trigs.begin(); it != loc_info.trigs.end(); it++) {
	(*it).used = false; // some triggers might not have been used
	for(int j = 0; j < num_trigs; j++) {
	    if(trigs[j].stachan_id == (*it).stachan_id)
	    {
		(*it).used = true;
		(*it).dist = trigs[j].dist;
		(*it).tt = trigs[j].tt;
		(*it).tterr = trigs[j].tterr;
		break;
	    }
	}
    }

    loc->evlat = event->lat;
    loc->evlon = event->lon;
    loc->evtime = event->time;
    loc->evdepth = event->depth;
    loc->bestex = bestex[kmin];
    loc->bestey = bestey[kmin];
    loc->depth_kmin = kmin;
    loc->misfit_rms = event->misfit_rms;
    loc->misfit_ave = event->misfit_ave;

    if(num_prevlocs >= size_prevlocs) {
	size_prevlocs = num_prevlocs + 100;
	prevlocs = (Location **)realloc(prevlocs, size_prevlocs*sizeof(Location *));
    }
    prevlocs[num_prevlocs++] = loc;
    return event->eventid;
}

void E2Location::latLonToXY(double evlat, double evlon)
{
    double ct, st, cf, sf, ca, sa, co, so, som, sfo, cfo, teller, rnoemer, azout;
    double theta, phi, x, y;
    double rad = 0.017453293; 
    double evazimuth = rad * 90; 
    int nx, ny;

    evlat *= rad;
    evlon *= rad;

    ca = cos(evlat);
    sa = sin(evlat);
    co = cos(evlon);
    so = sin(evlon);

    num_on_grid = 0;

    for(int i = 0; i < num_trigs; i++)
    { 
	// project spherical coordinates onto x-y plane centered at evlat,evlon
	// subroutine "rotate" in evm_evlocate.f
	ct = trigs[i].coslat;
	st = trigs[i].sinlat;
	cf = trigs[i].coslon;
	sf = trigs[i].sinlon;
	som = ct*ca*(cf*co + sf*so) + st*sa;
	theta = asin(som);
//	sfo = sin(triglon - evlon) = sin(triglon)*cos(evlon) - cos(triglon)*sin(evlon)
//	cfo = cos(triglon - evlon) = cos(triglon)*cos(evlon) + sin(triglon)*sin(evlon)
	sfo = sf*co - cf*so;
	cfo = cf*co + sf*so;
	teller = ct*sfo;
	rnoemer = st*ca - ct*sa*cfo;
	if(teller==0 && rnoemer==0) {
	    azout = 0;
	}
	else {
	    azout = atan2(teller,rnoemer);
	}
	phi = evazimuth - azout;
	// subroutine "rtfxyz" in evm_evlocate.f
	ct = cos(theta);
	x = 6371*ct*cos(phi);
	y = 6371*ct*sin(phi);
	trigs[i].x = x;
	trigs[i].y = y;
//	depth = 6371*sin(theta);

	if(x > 0.) nx = (int)(x/dx + .5);
	else nx = (int)(x/dx - .5);

	if(y > 0.) ny = (int)(y/dy + .5);
	else ny = (int)(y/dy - .5);

	if(trigs[i].use_in_location) {
	    if(nx >= -grid_size && nx <= grid_size && ny >= -grid_size && ny <= grid_size)
	    {
		trigx[num_on_grid] = nx;
		trigy[num_on_grid] = ny;
		trigs_on_grid[num_on_grid] = trigs[i];
		trigs_on_grid[num_on_grid].x = x;
		trigs_on_grid[num_on_grid].y = y;
		num_on_grid++;
	    }
	    else {
//		msg_str << "trigger off grid: " << trigx[i] << " " << trigy[i] << endl;
	    }
	}
    }
}

void E2Location::xyToLatLon(double evlat, double evlon, double bestex, double bestey, double *lat, double *lon)
{
    double theta, phi, ct, st, cf, sf, ca, sa, co, so, som, sfo, cfo, teller, rnoemer, azout;
    double rad = 0.017453293; 
    double evazimuth = rad * 90; 
    double bestez = 6371;

    evlat *= rad;
    evlon *= rad;

    // rotate back to real coordinates 
    // evm_evlocate.f subroutine "rtfxyz"
    phi = atan2(bestey,bestex);
    theta = atan2(bestez,sqrt( pow(bestex,2) + pow(bestey,2) ));
//    rr = sqrt( pow(bestex,2) + pow(bestey,2) + pow(bestez,2) );
    // evm_evlocate.f subroutine "rotate"
    ct = cos(theta);
    st = sin(theta);
    cf = cos(phi);
    sf = sin(phi);
    ca = cos(evlat);
    sa = sin(evlat);
    co = cos(evazimuth);
    so = sin(evazimuth);
    som = ct*ca*(cf*co + sf*so) + st*sa;
    theta = asin(som);
    sfo = sin(phi - evazimuth);
    cfo = cos(phi - evazimuth);
    teller = ct*sfo;
    rnoemer = st*ca - ct*sa*cfo;
    if(teller==0 && rnoemer==0) {
	azout = 0;
    }
    else {
	azout = atan2(teller,rnoemer);
    }
    phi = evlon - azout;

    // set new location
    *lat = theta/rad;
    *lon = phi/rad;
}

static void *
SearchGrid(void *client_data)
{
    SearchThread *t = (SearchThread *)client_data;
    t->parent->searchGrid(t);
    return NULL;
}

void E2Location::scanGrid(int *imin, int *jmin, double *misfit_rms)
{
    for(int i = 0; i < 4; i++) {
	pthread_create(&search_thread[i].id, NULL, SearchGrid, (void *)&search_thread[i]);
    }
    for(int i = 0; i < 4; i++) {
	pthread_join(search_thread[i].id, NULL);
    }

    // find imin, jmin for minimum thread misfit_rms
    *imin = 0;
    *jmin = 0;
    *misfit_rms = 1.e60;
    for(int i = 0; i < 4; i++) {
	if(search_thread[i].misfit_rms <= *misfit_rms)
	{
	    *misfit_rms = search_thread[i].misfit_rms;
	    *imin = search_thread[i].imin;
	    *jmin = search_thread[i].jmin;
	}
    }
}

void E2Location::searchGrid(SearchThread *t)
{
    double sumevOT, avgevOT, sumsq, misfit1;
    DepthTT *d = &tt_depths[depth_k];

    t->imin = 0;
    t->jmin = 0;
    t->misfit_rms = 1.e60;

    // replacing int n = abs(trigy[k]*grid_width - j*grid_width) + abs(trigx[k]-i);
    for(int k=0; k < num_on_grid; k++) {
	t->kndex[k] = trigy[k]*grid_width;
    }

    for(int j = t->jbeg; j <= t->jend; j++)
    {
	int jgrid = j*grid_width;
	for(int k=0; k < num_on_grid; k++) {
	    t->kjndex[k] = abs(t->kndex[k] - jgrid);
	}
	for(int i = -search_size; i <= search_size; i++)
	{
	    // for each grid point calculate rms of arrival time errors
	    sumevOT = 0;
	    for(int k=0; k < num_on_grid; k++)
	    {
//		int n = abs(trigy[k]-j)*grid_width + abs(trigx[k]-i);
		int n = t->kjndex[k] + abs(trigx[k]-i);
		t->ot[k] = trigs_on_grid[k].time - d->stt[n]; // station OT = station arrival time - travel time
		sumevOT += t->ot[k];
	    }

	    avgevOT = sumevOT/num_on_grid;

	    sumsq = 0.;
	    for(int k=0; k < num_on_grid; k++) // evm_evlocate.f line 314
            {
		sumsq += pow(t->ot[k] - avgevOT, 2);
            }

	    misfit1 = sumsq/num_on_grid;

	    if(misfit1 <= t->misfit_rms)
	    {
		t->misfit_rms = misfit1;
		t->imin = i;
		t->jmin = j;
	    }
	}
    }
}

void E2Location::scanSmallGrid(int *imin, int *jmin, double *misfit, double *misfit_ave,
				double *bestex, double *bestey, double *bestevOT)
{
    double ex, ey, sumevOT, avgevOT, sumsq, sum, misfit1, Stt;
    double trig_ot[MAX_EVENT_TRIGS], trig_tterr[MAX_EVENT_TRIGS];
    int i1, i2, j1, j2, l;
    DepthTT *d = &tt_depths[depth_k];

    i1 = *imin - small_grid_size;
    if(i1 < -grid_size) i1 = -grid_size;
    i2 = *imin + small_grid_size;
    if(i2 > grid_size) i2 = grid_size;

    j1 = *jmin - small_grid_size;
    if(j1 < -grid_size) j1 = -grid_size;
    j2 = *jmin + small_grid_size;
    if(j2 > grid_size) j2 = grid_size;

    *misfit = 1.e10;

    int num_tt = (int)d->v.size();

    for(int j = j1; j <= j2; j++) {
	ey = dy*j;
	for(int i = i1; i <= i2; i++) {
	    ex = dx*i;
	    // for each grid point calculate rms of arrival time errors
	    sumevOT = 0;
	    int num_used = 0;
	    for(int k=0; k < num_trigs; k++)
	    {
		double skm = sqrt( pow(trigs[k].x-ex,2) + pow(trigs[k].y-ey,2) );
		for(l = 1; l < num_tt && d->v[l].dist < skm; l++);
		if(l < num_tt) { // use ttgen and hauksson velocity model
		    Stt = d->v[l-1].tt + d->v[l].slow*(skm - d->v[l-1].dist);
		}
		else { // simply use velocity of 6km/s
		    Stt = skm/p_velocity;  // evm_evlocate.f line 302
		}
		trig_ot[k] = trigs[k].time - Stt; // station OT = station arrival time - travel time
		if(trigs[k].use_in_location) {
		    sumevOT += fabs(trig_ot[k]);
		    num_used++;
		}
	    }

	    avgevOT = sumevOT/num_used;

	    sumsq = 0;
	    sum = 0;
	    for(int k=0; k < num_trigs; k++) // evm_evlocate.f line 314
	    {
		trig_tterr[k] = avgevOT - trig_ot[k];
		if(trigs[k].use_in_location) {
		    sumsq += pow(trig_tterr[k], 2);
		    sum += fabs(trig_tterr[k]);
		}
	    }

	    misfit1 = sumsq/num_trigs;

	    if(misfit1 <= *misfit)
	    {
		*misfit = misfit1;
		*misfit_ave = sum/num_trigs;
		*bestex = ex;
		*bestey = ey;
		*bestevOT = avgevOT;
		*imin = i;
		*jmin = j;
		for(int k=0; k < num_trigs; k++) {
		    trigs[k].tterr = trig_tterr[k];
		}
	    }
	}
    }
}

/* Get the station travel time errors for the input LocateInfo.
 */
void E2Location::stationTTError(LocateInfo &loc, vector<E2Trigger *> triggers)
{
    int kmin = loc.depth_kmin;

    if(kmin < 0 || kmin >= (int)tt_depths.size()) {
	stringstream msg_str;
	msg_str << "W: stationTTError: invalid depth index: " << kmin
		<< " eventid: " << loc.eventid;
	LOG_INFO << msg_str.str();
	return;
    }
    DepthTT *d = &tt_depths[kmin];
    int num_tt = (int)d->v.size();
    double ex = loc.bestex;
    double ey = loc.bestey;

    double rad = 0.017453293; 
    double evazimuth = rad * 90; 

    double evlat = loc.initial_lat*rad;
    double evlon = loc.initial_lon*rad;

    double ca = cos(evlat);
    double sa = sin(evlat);
    double co = cos(evlon);
    double so = sin(evlon);

    for(vector<E2Trigger *>::iterator it = triggers.begin(); it < triggers.end(); it++) {
	E2Trigger *t = *it;

	// project spherical coordinates onto x-y plane centered at evlat,evlon
	double ct = t->coslat;
	double st = t->sinlat;
	double cf = t->coslon;
	double sf = t->sinlon;
	double som = ct*ca*(cf*co + sf*so) + st*sa;
	double theta = asin(som);
//	sfo = sin(triglon - evlon) = sin(triglon)*cos(evlon) - cos(triglon)*sin(evlon)
//	cfo = cos(triglon - evlon) = cos(triglon)*cos(evlon) + sin(triglon)*sin(evlon)
	double sfo = sf*co - cf*so;
	double cfo = cf*co + sf*so;
	double teller = ct*sfo;
	double rnoemer = st*ca - ct*sa*cfo;
	double azout;
	if(teller==0 && rnoemer==0) {
	    azout = 0;
	}
	else {
	    azout = atan2(teller,rnoemer);
	}
	double phi = evazimuth - azout;
	ct = cos(theta);
	double x = 6371*ct*cos(phi);
	double y = 6371*ct*sin(phi);

	double Stt;
	double skm = sqrt( pow(x-ex,2) + pow(y-ey,2) );
	int l;
	for(l = 1; l < num_tt && d->v[l].dist < skm; l++);
	if(l < num_tt) { // use ttgen and hauksson velocity model
	    Stt = d->v[l-1].tt + d->v[l].slow*(skm - d->v[l-1].dist);
	}
	else { // simply use velocity of 6km/s
	    Stt = skm/p_velocity;  // evm_evlocate.f line 302
	}
	t->tterr = loc.time + Stt - t->trigger_time;
    }
}

bool E2Location::previousLocation(E2Event *event, LocateInfo &loc_info)
{
    int j, k;
    // seach previous locations. locate() is called repeatedly for the same trigger set and initial location
    for(int i = 0; i < num_prevlocs; i++) { 
	if(event->lat == prevlocs[i]->initial_lat && event->lon == prevlocs[i]->initial_lon &&
		event->time == prevlocs[i]->initial_time && num_trigs == prevlocs[i]->num_trigs)
	{
	    for(j = 0; j < prevlocs[i]->num_trigs; j++) {
		for(k = 0; k < num_trigs; k++) {
		    if( prevlocs[i]->trigs[j].lat == trigs[k].lat &&
			prevlocs[i]->trigs[j].lon == trigs[k].lon &&
			prevlocs[i]->trigs[j].time == trigs[k].time) break;
		}
		if(k == num_trigs) break; // did not find it
	    }
	    if(j == prevlocs[i]->num_trigs) { // found all triggers. Can use previous results
		event->setLat(prevlocs[i]->evlat);
		event->setLon(prevlocs[i]->evlon);
		event->setDepth(prevlocs[i]->evdepth);
		event->setTime(prevlocs[i]->evtime);
		event->misfit_rms = prevlocs[i]->misfit_rms;
		event->misfit_ave = prevlocs[i]->misfit_ave;
		loc_info.eventid = event->eventid;
		loc_info.version = event->version;
		loc_info.depth_kmin = prevlocs[i]->depth_kmin;
		loc_info.bestex = prevlocs[i]->bestex;
		loc_info.bestey = prevlocs[i]->bestey;
		loc_info.lat = event->lat;
		loc_info.lon = event->lon;
		loc_info.depth = event->depth;
		loc_info.time = event->time;
		loc_info.misfit_rms = event->misfit_rms;
		loc_info.misfit_ave = event->misfit_ave;
		for(list<LocateTrigger>::iterator it = loc_info.trigs.begin(); it != loc_info.trigs.end(); it++) {
		    (*it).tterr = -999.; // some triggers might not have been used
		    for(j = 0; j < prevlocs[i]->num_trigs; j++) {
			if( prevlocs[i]->trigs[j].lat == (*it).lat &&
			    prevlocs[i]->trigs[j].lon == (*it).lon &&
			    prevlocs[i]->trigs[j].time == (*it).time)
			{
			    (*it).tterr = prevlocs[i]->trigs[j].tterr;
			    break;
			}
		    }
		}
		return true;
	    }
	}
    }
    return false;
}

void E2Location::removeOldLocations(E2Event *event)
{
    int j = 0;
    for(int i = 0; i < num_prevlocs; i++) { 
	if(event->time - prevlocs[i]->initial_time > 600) {
	    delete prevlocs[i];
	}
	else {
	    prevlocs[j++] = prevlocs[i];
	}
    }
    num_prevlocs = j;
}

TrigLoc::TrigLoc(E2Trigger *t, bool use)
{
    use_in_location = use;
    sta_id = t->staId();
    stachan_id = t->staChanId();
    lat = t->getLat();
    lon = t->getLon();
    time = t->getTime();
    coslat = t->coslat;
    sinlat = t->sinlat;
    coslon = t->coslon;
    sinlon = t->sinlon;
    x = 0.;
    y = 0.;
    dist = 0.;
    tt = 0.;
    tterr = 0.;
}

