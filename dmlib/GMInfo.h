/** @file GMInfo.h This file defines a structure used globally in
 * the dm_lib and eqInfo2GM code.
 * */

#ifndef __GMInfo_h
#define __GMInfo_h

#include <string>
#include <vector>

namespace gminfo {

    /** @struct GM_INFO
     * @brief A structure to encapsulate output ground motion parameters */
    struct GM_INFO {
        double PGV;    /**< Peak Ground Velocity */
        double PGA;    /**< Peak Ground Acceleration */
        double SI;     /**< Shaking Intensity (MMI: Modifed Mercalli Intensity) */
        double PGVSig; /**< Peak Ground Velocity Standard Deviation (sigma) */
        double PGASig; /**< Peak Ground Acceleration Standard Deviation (sigma) */
        double latGM;  /**< Latitude of computed Ground motion*/
        double lonGM;  /**< Longitude of computed Ground Motion*/
    };
} // end namespace gminfo

#endif

// end file
