/***********************************************************

File Name :
        SeismoFuncs.C

Original Author:
        Patrick Small

Description:


Creation Date:
        14 June 1999


Modification History:
	4 April 2007 Pete Lombard - modified to use the new CISN logAo 
	function and dMl produced by Bob Uhrhammer at UCB.

Usage Notes:


**********************************************************/

// Various include files
#include <cmath>
#include "SeismoFuncs.h"
#include "great.h"
#include "tt.h"
#include "dstaz.h"
#include "m1002m.h"
#include "CISN_MlAo.h"
#include "seismic.h"


SeismoFuncs::SeismoFuncs()
{
}



SeismoFuncs::~SeismoFuncs()
{
}


double SeismoFuncs::GetTravelTime(double dist)
{
  return(SeismoFuncs::Min(SeismoFuncs::GetPWaveTravelTime(dist),
			  SeismoFuncs::GetSWaveTravelTime(dist)));
}


double SeismoFuncs::GetPWaveTravelTime(double dist)
{
  float d, p1, p2, s1, s2;

  d = (float)dist;
  // Make call to fortran code here
  travel_t_(&d, &p1, &p2, &s1, &s2);
  return(SeismoFuncs::Min(p1, p2));
}


double SeismoFuncs::GetSWaveTravelTime(double dist)
{
  float d, p1, p2, s1, s2;

  d = (float)dist;
  // Make call to fortran code here
  travel_t_(&d, &p1, &p2, &s1, &s2);
  return(SeismoFuncs::Max(s1, s2));
}


double SeismoFuncs::GetTravelTime(double dist, double depth)
{
  float dt, dep, ptime;

  dt = (float)dist;
  dep = (float)depth;
  // Make call to fortran code here
  parrtime_(&dt, &dep, &ptime);
  return((double)ptime);
}


double SeismoFuncs::GetTravelTime(double lat1, double lon1, double depth,
				  double lat2, double lon2)
{
  float dist, az, ptime;
  float lt1, lt2, dep, ln1, ln2;

  lt1 = (float)lat1;
  ln1 = (float)lon1;
  dep = (float)depth;
  lt2 = (float)lat2;
  ln2 = (float)lon2;
  // Make call to fortran code here
  distaz_(&lt1, &ln1, &dep, &lt2, &ln2, &dist, &az, &ptime);
  return((double)ptime);
}


double SeismoFuncs::GetDistanceKM(double lat1, double lon1, 
				  double lat2, double lon2)
{
  float dist, distd, gc, az, baz;
  float lt1, lt2, ln1, ln2;

  lt1 = (float)lat1;
  ln1 = (float)lon1;
  lt2 = (float)lat2;
  ln2 = (float)lon2;
  // Make call to fortran code here
  great_(&lt1, &ln1, &lt2, &ln2, &dist, &distd, &gc, &az, &baz);
  return((double)dist);
}


double SeismoFuncs::GetDistanceDegrees(double lat1, double lon1, 
				       double lat2, double lon2)
{
  float dist, distd, gc, az, baz;
  float lt1, lt2, ln1, ln2;

  lt1 = (float)lat1;
  ln1 = (float)lon1;
  lt2 = (float)lat2;
  ln2 = (float)lon2;
  // Make call to fortran code here
  great_(&lt1, &ln1, &lt2, &ln2, &dist, &distd, &gc, &az, &baz);
  return((double)distd);
}



double SeismoFuncs::GetDistanceDegrees(double lat1, double lon1, double depth,
				       double lat2, double lon2)
{
  float dist, az, ptime;
  float lt1, lt2, dep, ln1, ln2;

  lt1 = (float)lat1;
  ln1 = (float)lon1;
  dep = (float)depth;
  lt2 = (float)lat2;
  ln2 = (float)lon2;
  // Make call to fortran code here
  distaz_(&lt1, &ln1, &dep, &lt2, &ln2, &dist, &az, &ptime);
  return((double)dist);
}



double SeismoFuncs::GetMagFromMag100(double m100, double dist, double depth,
				     seismoMagType magtype)
{
  float m, res, d, dep;
  int mtype;
  double dres = 0.0;
  double rdist = std::sqrt(dist * dist + depth * depth);
  
  m = (float)m100;
  d = (float)dist;
  dep = (float)depth;

  switch (magtype) {
  case SEISMO_MAG_ENERGY:
    mtype = CONVERT_ME100_TO_ME;
    // Make call to fortran code here
    m1002m_(&m, &res, &d, &dep, &mtype);
    dres = (double)res;
    break;
  case SEISMO_MAG_LOCAL:
    mtype = CONVERT_ML100_TO_ML;  // not used
    // Make call to fortran code here
    dres = m100 - cisn_la100_() + cisn_mlao_(&rdist);
    break;
  default:
    break;
  };

  return(dres);
}



double SeismoFuncs::GetMagFromMag100(double m100, double dist,
				     seismoMagType magtype)
{
  float m, res, d;
  int mtype;
  double dres = 0.0;

  m = (float)m100;
  d = (float)dist;

  switch (magtype) {
  case SEISMO_MAG_ENERGY:
    mtype = CONVERT_ME100_TO_ME;
    // Make call to fortran code here
    m1002m_hyp_(&m, &res, &d, &mtype);
    dres = (double)res;
    break;
  case SEISMO_MAG_LOCAL:
    mtype = CONVERT_ML100_TO_ML;  // not used
    // Make call to fortran code here
    dres = m100 - cisn_la100_() + cisn_mlao_(&dist);
    break;
  default:
    break;
  };

  return(dres);
}



double SeismoFuncs::Min(double m1, double m2)
{
  if (m1 < m2) {
    return(m1);
  } else {
    return(m2);
  }
}


double SeismoFuncs::Max(double m1, double m2)
{
  if (m1 > m2) {
    return(m1);
  } else {
    return(m2);
  }
}


TimeStamp SeismoFuncs::Min(TimeStamp m1, TimeStamp m2)
{
  if (m1 < m2) {
    return(m1);
  } else {
    return(m2);
  }
}


TimeStamp SeismoFuncs::Max(TimeStamp m1, TimeStamp m2)
{
  if (m1 > m2) {
    return(m1);
  } else {
    return(m2);
  }
}
