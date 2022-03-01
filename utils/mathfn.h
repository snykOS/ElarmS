/*
* Copyright (c) 2011 California Institute of Technology.
* All rights reserved.
* This program is distributed WITHOUT ANY WARRANTY whatsoever.
* Do not redistribute this program without written permission.
*/

#include <math.h>		// sqrt, fabs, atan2, cos, sin 
#include <cfloat>       // DBL_EPSILON

using namespace std;

#ifndef __DSYEVV3_H
#define __DSYEVV3_H
int dsyevv3(double A[3][3], double Q[3][3], double w[3]);
#endif
#ifndef __DSYEVC3_H
#define __DSYEVC3_H
int dsyevc3(double A[3][3], double w[3]);
#endif

