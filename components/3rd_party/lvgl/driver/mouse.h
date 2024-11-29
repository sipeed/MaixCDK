/**
 * @file mouse.h
 *
 */

#ifndef MOUSE_H
#define MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global_config.h"

/*********************
 *      INCLUDES
 *********************/
// #ifndef LV_DRV_NO_CONF
// #ifdef LV_CONF_INCLUDE_SIMPLE
// #include "lv_drv_conf.h"
// #else
// #include "../../lv_drv_conf.h"
// #endif
// #endif

#if CONFIG_LVGL_USE_MOUSE


#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#if PLATFORM_LINUX
#ifndef MONITOR_SDL_INCLUDE_PATH
#define MONITOR_SDL_INCLUDE_PATH <SDL2/SDL.h>
#endif

#include MONITOR_SDL_INCLUDE_PATH
#endif // PLATFORM_LINUX

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
enum class MouseInputDevice {
    TOUCHSCREEN,
    USB_MOUSE,
    UNKNOWN
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the mouse
 */
int mouse_init(lv_indev_t * indev_drv);

/**
 * Get the current position and state of the mouse
 * @param indev_drv pointer to the related input device driver
 * @param data store the mouse data here
 */
void mouse_read(lv_indev_t * indev, lv_indev_data_t * data);


/**********************
 *      MACROS
 **********************/

#endif /* CONFIG_LVGL_USE_MOUSE */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MOUSE_H */
