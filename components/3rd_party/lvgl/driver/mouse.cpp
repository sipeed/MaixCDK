/**
 * @file mouse.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "mouse.h"
#include "maix_lvgl.hpp"
#include "maix_basic.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>

#include "monitor.h"

#include <stdexcept>

/*********************
 *      DEFINES
 *********************/
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#if CONFIG_LVGL_USE_MOUSE


int mouse_init([[maybe_unused]]lv_indev_t * indev_drv)
{
    std::stringstream ss;
    ss << "In " << __FILE__ << " line:" << __LINE__ << "\n\tUse deprecated function: " << __PRETTY_FUNCTION__;
    throw std::runtime_error(ss.str());
    return 0;
}

void mouse_read([[maybe_unused]]lv_indev_t * indev, lv_indev_data_t * data)
{
    std::stringstream ss;
    ss << "In " << __FILE__ << " line:" << __LINE__ << "\n\tUse deprecated function: " << __PRETTY_FUNCTION__;
    throw std::runtime_error(ss.str());
}

#endif
/**********************
 *   STATIC FUNCTIONS
 **********************/

// #endif
