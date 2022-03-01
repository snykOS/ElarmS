/***********************************************************

File Name :
        SeismoFuncs.h

Original Author:
        Patrick Small

Description:


Creation Date:
        14 June 1999

Modification History:
	10 Dec 2007 Pete Lombard - Modified to use leap-second-aware 
	tntime classes.


Usage Notes:


**********************************************************/

#ifndef seismo_funcs_H
#define seismo_funcs_H

// Various include files
#include "TimeStamp.h"


// Enumeration of the valid types of magnitudes
typedef enum {SEISMO_MAG_ENERGY, SEISMO_MAG_LOCAL} seismoMagType;


class SeismoFuncs
{
 private:

 protected:

 public:
    SeismoFuncs();
    ~SeismoFuncs();

    // OBSOLETE: Travel time using travel_t_ (tt.h)
    //
    static double GetTravelTime(double dist);


    // Travel time using travel_t_ (tt.h)
    //
    // Good for local event P-wave travel times
    //
    static double GetPWaveTravelTime(double dist);


    // Travel time using travel_t_ (tt.h)
    //
    // Good for local event S-wave travel times
    //
    static double GetSWaveTravelTime(double dist);


    // Travel time using parrtime_ (dstaz.h)
    //
    // Good for teleseism travel times
    //
    static double GetTravelTime(double dist, double depth);


    // Travel time using dstaz_ (dstaz.h)
    //
    // Good for teleseism travel times
    //
    static double GetTravelTime(double lat1, double lon1, 
				double depth, double lat2, double lon2);

    // Distance in KM using great.h
    //
    // Good for local events
    //
    static double GetDistanceKM(double lat1, double lon1, 
				double lat2, double lon2);

    // Distance in degrees using great.h
    //
    // Good for local events
    //
    static double GetDistanceDegrees(double lat1, double lon1, 
				     double lat2, double lon2);
    
    // Distance in degrees using dstaz.h
    //
    // Good for teleseism events
    //
    static double GetDistanceDegrees(double lat1, double lon1, 
				     double depth, double lat2, double lon2);


    // Magnitude using m1002m_ (m1002m.h)
    //
    // Takes epicentral distance and depth
    //
    static double GetMagFromMag100(double m100, double dist, double depth,
				   seismoMagType magtype);

    // Magnitude using m1002m_ (m1002m.h)
    //
    // Takes hypocentral distance
    //
    static double GetMagFromMag100(double m100, double dist,
				   seismoMagType magtype);


    static double Min(double m1, double m2);
    static double Max(double m1, double m2);
    static TimeStamp Min(TimeStamp m1, TimeStamp m2);
    static TimeStamp Max(TimeStamp m1, TimeStamp m2);
};


#endif
