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
#include <math.h>

#include "deltaz.h"

static double deg_to_radians = M_PI/180.;
static double half_pi = M_PI/2.;

static double azimuth(double rx, double ry, double rz, double sx, double sy, double sz);

// return distance in km
double
distancekm(double lat1, double lon1, double lat2, double lon2)
{
    return getDelta(lat1, lon1, lat2, lon2) * 111.19492664;
}

/* For the input geographic coordinates slat, slon, rlat, rlon, compute
 * delta, az, baz
 *
 * AUTHOR
 *      I. Henson
 */

/** 
 *  Compute source-receiver distance, azimuth and back-azimuth.
 *  @param slat	Source latitude in degrees.
 *  @param slon Source longitude in degrees.
 *  @param rlat Receiver latitude in degrees.
 *  @param rlon Receiver longitude in degrees.
 *  @param delta Returned distance from source to receiver in degrees.
 *  @param az	Returned azimuth from source to receiver in degrees.
 *  @param baz	Returned azimuth from receiver to source in degrees.
 */
void
deltaz(double slat, double slon, double rlat, double rlon, double *delta,
	double *az, double *baz)
{
    double sx, sy, sz, rx, ry, rz, cx, cy, cz, dot, norm, theta, phi;

    *delta = *az = *baz = 0.;
    if(slat == rlat && slon == rlon) return;

    phi = slon*deg_to_radians;
    theta = (90. - geocentric(slat))*deg_to_radians;
    sx = sin(theta)*cos(phi);
    sy = sin(theta)*sin(phi);
    sz = cos(theta);

    phi = rlon*deg_to_radians;
    theta = (90. - geocentric(rlat))*deg_to_radians;
    rx = sin(theta)*cos(phi);
    ry = sin(theta)*sin(phi);
    rz = cos(theta);

    dot = sx*rx + sy*ry + sz*rz;

    if(dot == 0.) return;

    /* delta = atan( |s x r| / (s * r) )
     */
    cx = sy*rz - sz*ry;
    cy = sz*rx - sx*rz;
    cz = sx*ry - sy*rx;
    norm = sqrt(cx*cx + cy*cy + cz*cz);

    *delta = atan2(norm, dot)/deg_to_radians;

    *az  = azimuth(rx, ry, rz, sx, sy, sz);
    *baz = azimuth(sx, sy, sz, rx, ry, rz);

    return;
}

double
getDelta(double slat, double slon, double rlat, double rlon)
{
//    slat = geocentric(slat);
//    rlat = geocentric(rlat);

    slat *= deg_to_radians;
    rlat *= deg_to_radians;
    slon *= deg_to_radians;
    rlon *= deg_to_radians;

    double arg = sin(slat)*sin(rlat) + cos(slat)*cos(rlat)*cos(slon-rlon);

    return (arg < 1.0) ? acos(arg)/deg_to_radians : 0.0;
}

static double
azimuth(double rx, double ry, double rz, double sx, double sy, double sz)
{
    double az, nx, ny, nz, bx, by, bz, cx, cy, cz, dot, norm;

    /*
     * azimuth = the angle between the pole to the great circle through
     *	(0,0,1) and s and the pole to the great circle through
     *	r and s.
     *
     *  n = north pole cross s (normalized)
     *  c = r cross s (normalized)
     *  az = atan( |c x n| / (c * n) )
     */

    cx = ry*sz - rz*sy;
    cy = rz*sx - rx*sz;
    cz = rx*sy - ry*sx;
    norm = sqrt(cx*cx + cy*cy + cz*cz);

    /* normalize c
     */
    cx /= norm;
    cy /= norm;
    cz /= norm;

    nx = -sy;
    ny = sx;
    nz = 0.;
    norm = sqrt(nx*nx + ny*ny);
    nx /= norm;
    ny /= norm;
	
    /* b = c x n
     */
    bx = cy*nz - cz*ny;
    by = cz*nx - cx*nz;
    bz = cx*ny - cy*nx;

    norm = sqrt(bx*bx + by*by + bz*bz);

    dot = cx*nx + cy*ny + cz*nz;

    az = atan2(norm, dot)/deg_to_radians;

    /* if (s dot b) < 0. az = -az
     */
    dot = sx*bx + sy*by + sz*bz;

    if(dot < 0.) az = -az;

    if(az < 0.) az += 360.;

    return(az);
}

#define sign(a,b) ((b >= 0.) ? a : -a)

/** 
 *  Convert geographic latitude to geocentric latitude.
 *  @param geographic_lat geographic latitude in degrees.
 *  @return geocentric latitude in degrees.
 */
double
geocentric(double geographic_lat)
{
    double geocentric_lat;

    geographic_lat *= deg_to_radians;

    geocentric_lat = (half_pi - fabs(geographic_lat) >= .05) ?
		atan(0.993277*sin(geographic_lat)/cos(geographic_lat)) :
		geographic_lat/0.993277 - sign(0.01063193, geographic_lat);

    return(geocentric_lat/deg_to_radians);
}

/** 
 *  Convert geocentric latitude to geographic latitude.
 *  @param geocentric_lat geocentric latitude in degrees.
 *  @return geographic latitude in degrees.
 */
double
geographic(double geocentric_lat)
{
    double geographic_lat;

    geocentric_lat *= deg_to_radians;

    geographic_lat = (half_pi - fabs(geocentric_lat) >= .05) ?
		atan(sin(geocentric_lat)/(0.993277*cos(geocentric_lat))) :
		0.993277*(geocentric_lat + sign(0.01063193, geocentric_lat));

    return(geographic_lat/deg_to_radians);
}
