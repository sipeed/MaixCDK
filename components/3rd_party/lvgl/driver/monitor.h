/**
 * @file monitor.h
 *
 */

#ifndef MONITOR_H
#define MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global_config.h"

/*********************
 *      INCLUDES
 *********************/
// #include "lv_drv_conf.h"

// #if USE_MONITOR

#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void monitor_init(int w, int h);
void monitor_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * color_p);

void monitor_rect(int* w, int* h);
/**********************
 *      MACROS
 **********************/

// #endif /* USE_MONITOR */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MONITOR_H */
