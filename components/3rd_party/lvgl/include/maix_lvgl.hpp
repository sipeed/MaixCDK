#pragma once
#include "maix_display.hpp"
#include "maix_touchscreen.hpp"

namespace maix
{
    extern display::Display *maix_display;
    extern image::Image     *maix_image;
    extern touchscreen::TouchScreen *maix_touchscreen;

    /**
     * @brief init lvgl
     * @param display display device, display must init first
    */
    void lvgl_init(display::Display *display, touchscreen::TouchScreen *touchscreen);

    void lvgl_destroy();
}

