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
#ifndef __E2Location__
#define __E2Location__

#include <pthread.h>
#include <vector>
#include "Exceptions.h"

class E2Event;
class E2Trigger;
class E2TriggerManager;
class E2EventManager;
class LocateInfo;

#define MAX_EVENT_TRIGS 100

typedef struct {
    float dist;
    float tt;
    double slow;
} TtPoint;

class DepthTT
{
  public:
    double depth;
    std::vector<TtPoint> v;
    float *stt;
};

class TrigLoc {
  public:
    bool use_in_location;
    int sta_id;
    int stachan_id;
    double lat;
    double lon;
    double time;
    double coslat;
    double sinlat;
    double coslon;
    double sinlon;
    double x;
    double y;
    double dist;
    double tt;
    double tterr;
    TrigLoc(E2Trigger *t, bool use);
    TrigLoc() : use_in_location(true), sta_id(-1), stachan_id(-1), lat(0.0), lon(0.0), time(0.0),
	coslat(0.0), sinlat(0.0), coslon(0.0), sinlon(0.0), x(0.0), y(0.0), tterr(0.) { }
};

class Location {
  public:
    double initial_lat;
    double initial_lon;
    double initial_time;
    double evlat;
    double evlon;
    double evtime;
    double evdepth;
    double bestex;
    double bestey;
    double misfit_rms;
    double misfit_ave;
    int depth_kmin;
    float *grid;
    double glat1;
    double glon1;
    double glat2;
    double glon2;
    int num_trigs;
    TrigLoc trigs[MAX_EVENT_TRIGS];
    Location() : initial_lat(0.), initial_lon(0.), initial_time(0.), evlat(0.),
	evlon(0.), evtime(0.), evdepth(0.), bestex(0.), bestey(0.),
	misfit_rms(0.), misfit_ave(0.), depth_kmin(0), grid(NULL),
	glat1(0.), glon1(0.), glat2(0.), glon2(0.), num_trigs(0) {}
    ~Location() { if(grid) delete grid; }
};

class E2Location;

class SearchThread {
  public:
    pthread_t id;
    E2Location *parent;
    int jbeg, jend;
    int imin, jmin;
    double misfit_rms;
    int kndex[MAX_EVENT_TRIGS], kjndex[MAX_EVENT_TRIGS];
    double ot[MAX_EVENT_TRIGS];
    SearchThread() : parent(NULL) {}
};

class E2Location
{
 private:
    bool depthsearch;  // default is false => search only in flat plane
    static float *stt;
    int grid_size;  // grid dimensions are 2*grid_size + 1 (50)
    int search_size;  // grid dimensions of search area. must be <= grid_size
    int grid_width; // 2*grid_size + 1
    double grid_km; // half the grid width (100km)
    double dx, dy;  // grid spacing = grid_km / grid_size
    int small_grid_size; // 3
    double p_velocity;
    bool loc_all_zchans; // locate using all Z channels (HNZ, HHZ, ...) for a station
		         // if false, locate using only the earliest Z trigger
    E2TriggerManager *tm;
    E2EventManager *em;
    int num_trigs, num_on_grid;
    TrigLoc trigs[MAX_EVENT_TRIGS];
    TrigLoc trigs_on_grid[MAX_EVENT_TRIGS];
    int trigx[MAX_EVENT_TRIGS], trigy[MAX_EVENT_TRIGS];
    std::vector<DepthTT> tt_depths;
    int depth_k;
    int num_prevlocs, size_prevlocs;
    double *bestex;
    double *bestey;
    double *bestevOT;
    double *misfit_rms;
    double *misfit_ave;
    double *latitude;
    double *longitude;
    double *min_sta_dist;
    Location **prevlocs;

    SearchThread search_thread[4];

    bool readTTFile(std::string ttfile);
    void latLonToXY(double evlat, double evlon);
    void xyToLatLon(double evlat, double evlon, double bestex, double bestey,
			double *lat, double *lon);
    bool previousLocation(E2Event *event, LocateInfo &loc_info);
    void scanGrid(int *imin, int *jmin, double *misfit_rms);
    void scanSmallGrid(int *imin, int *jmin, double *misfit_rms, double *misfit_ave,
			double *bestex, double *bestey, double *bestevOT);
    void removeOldLocations(E2Event *event);

 public:
    E2Location(E2TriggerManager *tm, E2EventManager *em, std::string ttfile) throw(Error);
 
    ~E2Location() {
	if(bestex) delete[] bestex;
	if(bestey) delete[] bestey;
	if(bestevOT) delete[] bestevOT;
	if(misfit_rms) delete[] misfit_rms;
	if(misfit_ave) delete[] misfit_ave;
	if(latitude) delete[] latitude;
	if(longitude) delete[] longitude;
	if(min_sta_dist) delete[] min_sta_dist;
    }
    int locate(E2Event *event, LocateInfo &loc_info);
    void searchGrid(SearchThread *t);
    void stationTTError(LocateInfo &loc, std::vector<E2Trigger *> triggers);
};

#endif
