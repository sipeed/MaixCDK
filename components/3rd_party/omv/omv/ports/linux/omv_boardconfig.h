/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Board configuration and pin definitions.
 */
#ifndef __OMV_BOARDCONFIG_H__
#define __OMV_BOARDCONFIG_H__

#include "arm_compat.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

// minimum fb alloc size
#define OMV_FB_ALLOC_SIZE                   (1 * 1024 * 1024)

#ifndef PI
#define PI                                  (3.1415926)
#endif

#ifdef __cplusplus
}
#endif

#endif //__OMV_BOARDCONFIG_H__
