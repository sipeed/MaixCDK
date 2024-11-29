#ifndef __INPUT_DEVICE_HPP__
#define __INPUT_DEVICE_HPP__

// #include "maix_lvgl.hpp"
#include "lvgl.h"

void pointing_device_init(lv_indev_t * indev_drv);
void pointing_device_read(lv_indev_t * indev_drv, lv_indev_data_t * data);

#endif // __INPUT_DEVICE_HPP__