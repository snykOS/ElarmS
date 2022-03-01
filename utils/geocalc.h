/*
* Copyright (c) 2011 California Institute of Technology.
* All rights reserved.
* This program is distributed WITHOUT ANY WARRANTY whatsoever.
* Do not redistribute this program without written permission.
*/
#ifndef __geocalc_h
#define __geocalc_h
#include <math.h>
#include <string>
#include <vector>
#include "mathfn.h"

namespace geocalc {
    static const double PI = 3.1415926;
    bool calcaz(std::vector<float>& N, std::vector<float>& E, std::vector<float>& Z, double &epiaz);
    double latlon_distance(double thei,double alei,double thsi,double alsi,double& azesdg);
    void azran_ps(double ALATC,double ALONC,double AZIMUTH,double RANGE, double &ALAT,double &ALON);
};

// rcsid version strings
#define RCSID_geocalc_h "$Id: geocalc.h $"
extern const std::string RCSID_geocalc_cc;
#endif
