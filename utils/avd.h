/*
* Copyright (c) 2011 California Institute of Technology.
* All rights reserved.
* This program is distributed WITHOUT ANY WARRANTY whatsoever.
* Do not redistribute this program without written permission.
*/

#ifndef __avd_h
#define __avd_h
#include "Configuration.h"

/** @class avd 
 * @brief A very simple class designed to hold the different types of ground motion
 * for a single channel.
 */
class avd {
 public:

    std::vector<float> acc; /**< vector of acceleration values */
    std::vector<float> vel; /**< vector of velocity values */
    std::vector<float> disp; /**< vector of displacement values */

    avd();
    avd(int m_size) : acc(m_size, 0), vel(m_size, 0), disp(m_size, 0) {};
};

// rcsid version strings
#define RCSID_avd_h "$Id: avd.h $"
#endif
