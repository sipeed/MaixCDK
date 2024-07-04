#ifndef __APP_H
#define __APP_H

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "ui_screen.h"
#include "ui_event_handler.h"
#include "ui_utils.h"
#include "maix_display.hpp"

using namespace maix;

int app_pre_init(void);
int app_init(display::Display *disp);
int app_loop(void);
int app_deinit(void);

// #ifdef __cplusplus
// }
// #endif

#endif // __APP_H