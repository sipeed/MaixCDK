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
/**
 * Initialize the mouse
 */
void mouse_init(lv_indev_t * indev_drv)
{
    // maix::thread::Thread t = maix::thread::Thread([](void *args){
    //     lv_indev_t *indev_drv = (lv_indev_t *)args;
    //     while (!maix::app::need_exit())
    //     {
    //         maix::maix_touchscreen->available(-1);
    //         printf("mouse thread read\n");
    //         // lv_indev_read(indev_drv);
    //     }
    // }, indev_drv);
    // t.detach();
}

/**
 * Get the current position and state of the mouse
 * @param indev_drv pointer to the related input device driver
 * @param data store the mouse data here
 */
void mouse_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

    static int x, y;
    static bool pressed, continue_reading;
    if(maix::maix_touchscreen->read0(x, y, pressed) == maix::err::ERR_NOT_READY)
    {
        data->point.x = (lv_coord_t)x;
        data->point.y = (lv_coord_t)y;
        data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        return;
    }
    continue_reading = maix::maix_touchscreen->available();

    data->point.x = (lv_coord_t)x;
    data->point.y = (lv_coord_t)y;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->continue_reading = continue_reading;
}
#endif
/**********************
 *   STATIC FUNCTIONS
 **********************/

// #endif
