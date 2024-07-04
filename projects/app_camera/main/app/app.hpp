#ifndef __APP_H
#define __APP_H

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "ui_screen.h"
#include "ui_event_handler.h"
#include "ui_utils.h"
#include "maix_image.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"

int app_pre_init(void);
int app_init(void);
int app_loop(maix::camera::Camera &camera, maix::image::Image *img, maix::display::Display &disp, maix::display::Display *disp2);
int app_deinit(void);

// #ifdef __cplusplus
// }
// #endif

#endif // __APP_H