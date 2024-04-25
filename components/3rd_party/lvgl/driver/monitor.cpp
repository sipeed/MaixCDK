/**
 * @file monitor.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "monitor.h"
// #if USE_MONITOR

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mouse.h"
#include "keyboard.h"
#include "mousewheel.h"
#include "maix_lvgl.hpp"
#include "maix_basic.hpp"

#include <stdexcept>

using namespace maix;

static int hres = 552;
static int vres = 368;

/**
 * Initialize the monitor
 */
void monitor_init(int w, int h)
{
    hres = w;
    vres = h;
}

/**
 * Flush a buffer to the marked area
 * @param drv pointer to driver where this function belongs
 * @param area an area where to copy `color_p`
 * @param color_p an array of pixel to copy to the `area` part of the screen
 */
void monitor_flush(lv_display_t *disp_drv, const lv_area_t * area, uint8_t *px_map)
{
    assert(LV_COLOR_DEPTH == 32);
    uint32_t * color_p = (uint32_t *)px_map; 

//    printf("x1:%d,y1:%d,x2:%d,y2:%d\n", area->x1, area->y1, area->x2, area->y2);

    /*Return if the area is out the screen*/
    if(area->x2 < 0 || area->y2 < 0 || area->x1 >= hres || area->y1 >= vres) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    int32_t y;
    uint32_t w = lv_area_get_width(area);
    assert(maix_image->data());
    for(y = area->y1; y <= area->y2 && y < vres; y++) {
        memcpy(((uint32_t*)maix_image->data()) + y * hres + area->x1, color_p, (area->x2 - area->x1 + 1) * 4);
        color_p += w;
    }

    /* TYPICALLY YOU DO NOT NEED THIS
     * If it was the last part to refresh update the texture of the window.*/
    if(lv_disp_flush_is_last(disp_drv)) {
        maix_display->show(*maix_image);
    }

    /*IMPORTANT! It must be called to tell the system the flush is ready*/
    lv_disp_flush_ready(disp_drv);

}


// #endif /*USE_MONITOR*/
