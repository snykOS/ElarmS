/** @file geocalc.cc
 * @brief This file implements various functions for geospatial calculations.
*/

/*
* Copyright (c) 2011 California Institute of Technology.
* All rights reserved.
* This program is distributed WITHOUT ANY WARRANTY whatsoever.
* Do not redistribute this program without written permission.
*/
#include "geocalc.h"

const std::string RCSID_geocalc_cc = "$Id: geocalc.cc $";

namespace geocalc {

    #ifndef atand
    #define atand(x) (atan(x) * 180.0 / PI)
    #endif

/** Takes in data arrays representing three components of a seismic sensor and determines the
 * eigenvectors which represent the particle motion. The arrays are expected to contain P-wave
 * data, so the eigenvector with the largest eigenvector is taken to represent ray direction.
 * This is returned as an azimuth back to epicenter.
 * @param N vector containing waveform data for North component
 * @param E vector containing waveform data for East component
 * @param Z vector containing waveform data for Vertical (Z) component
 * @param epi_azimuth ray azimuth to be returned
 * @return bool indication of success, false if eigenvectors could not be computed
 */    
    bool calcaz(std::vector<float>& N, std::vector<float>& E, std::vector<float>& Z, double &epi_azimuth) {    // Distance, azimuth and coordinates of the epicenter
        double azi = 0.0;
        double covmat[3][3];
        double eigenvec[3][3];
        double eigenval[3];
        for (int j=0; j<3; j++) {
            for (int k=0; k<3; k++) {
                covmat[j][k] = 0.0;
            }
        }
        for (unsigned int i=0; i<N.size(); i++) {
            covmat[0][0] += Z[i]*Z[i];
            covmat[0][1] += Z[i]*N[i];
            covmat[0][2] += Z[i]*E[i];
            //covmat[1][0] += N[i]*Z[i];
            covmat[1][1] += N[i]*N[i];
            covmat[1][2] += N[i]*E[i];
            //covmat[2][0] += E[i]*Z[i];
            //covmat[2][1] += E[i]*N[i];
            covmat[2][2] += E[i]*E[i];

            //covmat[0][0] += dis_Z[i]*dis_Z[i];
            //covmat[0][1] += dis_Z[i]*dis_N[i];
            //covmat[0][2] += dis_Z[i]*dis_E[i];
            ////covmat[1][0] += dis_N[i]*dis_Z[i];
            //covmat[1][1] += dis_N[i]*dis_N[i];
            //covmat[1][2] += dis_N[i]*dis_E[i];
            ////covmat[2][0] += dis_E[i]*dis_Z[i];
            ////covmat[2][1] += dis_E[i]*dis_N[i];
            //covmat[2][2] += dis_E[i]*dis_E[i];
        }
        if (dsyevv3(covmat,eigenvec,eigenval) == -1) {
            return false;
        }
        azi = atand(eigenvec[1][0]/eigenvec[2][0]);

        int shift = 0;
        //double epi_azimuth = 0.0;
        if (eigenvec[2][0] > 0) { // correct for azimuth quadrant
            shift = 90.0;
            epi_azimuth = shift - azi;
        }
        else {
            shift = 270.0;
            epi_azimuth = shift - azi;
        }
        if (eigenvec[0][0] > 0) { // correct 180. azimuthal uncertainty using 
            if (epi_azimuth < 180.) {
                epi_azimuth += 180.0;
            }
            else {
                epi_azimuth -= 180.0;
            }

            //LOGD << "geocalc: " << eigenvec[0][0] << " " << eigenvec[1][0] << " " << eigenvec[2][0] << " " << azi << " " << epi_azimuth;
            //LOGD <<"geocalc: "<<dis_N[0]<<" , "<<dis_E[0];
        }

        return true;
    }
 
/** Compute the latitude, longitude of a point that is at a given distance and direction for a given
 * latititude, longitude point. 
 * @param ALATC given point latitude
 * @param ALONC given point longitude
 * @param AZIMUTH given azimuth (0 North, in degrees)
 * @param RANGE given distance (km)
 * @param ALAT latitude of point to be calculated
 * @param ALON longitude of point to be calculated
 */
    void azran_ps(double ALATC, double ALONC, double AZIMUTH, double RANGE, double &ALAT, double &ALON){
        RANGE   = RANGE*1000.0; // in m
        double REARTH  = 6.3712E6;
        double PI      = 3.14159;

// Do the repetitive trig computations once, up-front.

        double DG2RAD = PI / 180.0;
        double RAD2DG = 180.0 / PI;
        double SINLAC = sin( ALATC * DG2RAD );
        double COSLAC = cos( ALATC * DG2RAD );

// DETERMINE X AND Y DISPLACEMENTS FROM AZIMUTH AND RANGE
        double X = RANGE*cos(DG2RAD*(AZIMUTH-90.0));
        double Y = RANGE*sin(DG2RAD*(AZIMUTH+90.0));

// Compute maximum range from center to define map boundaries (half of
// earth's circumference).

        //double MAXRHO = PI * REARTH;
        double RHO = sqrt( X*X + Y*Y );
        double C = RHO / REARTH;
        double SINC = sin(C);
        double COSC = cos(C);

// Computations blow up at origin, so check RHO.

        if (RHO==0.0) {
            ALAT=ALATC; // ! Trivial case for ALAT and ALON at origin of map.
            ALON=ALONC;
        }
        else { //! Normal computation of ALAT and ALON.
            ALAT = asin( COSC*SINLAC + Y*SINC*COSLAC/RHO ) * RAD2DG;
            if ( ALATC==90.0 ) {// Case for North Pole.
                ALON = ALONC + atan( X/(-Y) ) * RAD2DG;
            }
            else if ( ALATC==-90.0 ) {        // Case for South Pole.
                ALON = ALONC + atan( X/Y ) * RAD2DG;
    	}
            else { // Map center not at pole (Oblique or Equatorial Case).
                ALON = ALONC + atan( X*SINC/(RHO*COSLAC*COSC-Y*SINLAC*SINC) ) * RAD2DG;
            }
        }     
    } // azran_ps

/** Compute the azimuth and distance between latitude, longitude of a point that is at a given distance and direction for a given
 * latititude, longitude point. 
 * @param thei given point 1 latitude
 * @param alei given point 1 longitude
 * @param thsi given point 2 latitude
 * @param alsi given point 2 longitude
 * @param azesdg azimuth from point 1 to point 2 (0 North, in degrees)
 * @return distance between point 1 and point 2 (km)
 */
    double latlon_distance(double thei, double alei, double thsi, double alsi, double& azesdg){
        double delt = 0;
        // double deltdg = 0;
        double deltkm = 0;
        double azes = 0;
        // double azesdg = 0;
        double azse = 0;
        // double azsedg = 0;
        double i = 0;

        /* Local variables */
        double a, b, c, d, e, g, h;
        double c1, c3, c4, c5, c6;
        double ak, ap, bp, cp, dp, ep, gp, hp;
        double aaa, ale;
        double akp;
        double als, the, ths;
        double d__1, d__2, d__3;
    
        if (i <= 0) {
            goto L50;
        } else {
            goto L51;
        }
    /* IF COORDINATES ARE GEOGRAPH DEG I=0 */
    /* IF COORDINATES ARE GEOCENT RADIAN I=1 */
     L50:
        the = thei * .01745329252;
        ale = alei * .01745329252;
        ths = thsi * .01745329252;
        als = alsi * .01745329252;
        aaa = tan(the) * .9931177;
        the = atan(aaa);
        aaa = tan(ths) * .9931177;
        ths = atan(aaa);
        goto L32;
     L51:
        the = thei;
        ale = alei;
        ths = thsi;
        als = alsi;
     L32:
        c = sin(the);
        ak = (-1)*cos(the);
        d = sin(ale);
        e = (-1)*cos(ale);
        a = ak * e;
        b = (-1)*ak * d;
        g = (-1)*c * e;
        h = c * d;
        cp = sin(ths);
        akp = (-1)*cos(ths);
        dp = sin(als);
        ep = (-1)*cos(als);
        ap = akp * ep;
        bp = (-1)*akp * dp;
        gp = (-1)*cp * ep;
        hp = cp * dp;
        c1 = a * ap + b * bp + c * cp;
        if (c1 - .94 >= 0.) {
    	    goto L31;
        } else {
	    goto L30;
        }
     L30:
        if (c1 + .94 <= 0.) {
       	    goto L28;
        } else {
            goto L29;
        }
     L29:
        delt = acos(c1);
     L33:
        deltkm = delt * 6371.;
        d__1 = ap - d;
        d__2 = bp - e;
        d__3 = cp;
        c3 = d__1 * d__1 + d__2 * d__2 + d__3 * d__3 - 2.;
        d__1 = ap - g;
        d__2 = bp - h;
        d__3 = cp - ak;
        c4 = d__1 * d__1 + d__2 * d__2 + d__3 * d__3 - 2.;
        d__1 = a - dp;
        d__2 = b - ep;
        d__3 = c;
        c5 = d__1 * d__1 + d__2 * d__2 + d__3 * d__3 - 2.;
        d__1 = a - gp;
        d__2 = b - hp;
        d__3 = c - akp;
        c6 = d__1 * d__1 + d__2 * d__2 + d__3 * d__3 - 2.;
        // deltdg = delt * 57.29577951;
        azes = atan2(c3, c4);
        if (azes >= 0.) {
            goto L81;
        } else {
            goto L80;
        }
     L80:
        azes += 6.283185308;
     L81:
        azse = atan2(c5, c6);
        if (azse >= 0.) {
            goto L71;
        } else {
            goto L70;
        }
     L70:
        azse += 6.283185308;
     L71:
        azesdg = azes * 57.29577951;
        // azsedg = azse * 57.29577951;
        return deltkm;
     L31:
        d__1 = a - ap;
        d__2 = b - bp;
        d__3 = c - cp;
        c1 = d__1 * d__1 + d__2 * d__2 + d__3 * d__3;
        c1 = sqrt(c1);
        c1 /= 2.;
        delt = asin(c1);
        delt *= 2.;
        goto L33;
     L28:
        d__1 = a + ap;
        d__2 = b + bp;
        d__3 = c + cp;
        c1 = d__1 * d__1 + d__2 * d__2 + d__3 * d__3;
        c1 = sqrt(c1);
        c1 /= 2.;
        delt = acos(c1);
        delt *= 2.;
        goto L33;
    } // latlon_distance

}
