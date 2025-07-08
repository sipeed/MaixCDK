/*
 * Mahony AHRS
 * @author Neucrack
 * @license Apache2.0
 * @update 2016.7.1 by Neucrack, add source code.
 *         2025.7.4 by Neucrack, optimize for MaixPy/MaixCDK
 *
*/
#pragma once

#include "math.h"
#include "maix_type_vector3.hpp"


namespace maix::ahrs
{
    /**
      * Math PI
      * @maixpy maix.ahrs.PI
     */
    const float PI = 3.14159265358979323846f;

    /**
      * angle unit radian to degree.
      * @maixpy maix.ahrs.RAD2DEG
     */
    const float RAD2DEG = 180.0f / PI;

    /**
      * angle unit degree to radian.
      * @maixpy maix.ahrs.DEG2RAD
     */
    const float DEG2RAD = PI / 180.0f;

}; //namespace maix::ahrs
