
#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"


/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <iostream>
#include <sys/resource.h>

#include "maix_basic.hpp"
#include "maix_display.hpp"
#include "maix_lvgl.hpp"
#include "maix_camera.hpp"
#include "maix_image.hpp"
#include "maix_util.hpp"
#include "lvgl.h"
#include "app.hpp"

using namespace maix;

int _main(int argc, char **argv)
{
    // app pre init
    app_pre_init();

    // init display
    display::Display disp = display::Display(-1, -1, image::FMT_YVU420SP);
    err::check_bool_raise(disp.is_opened(), "camera open failed");
    display::Display *other_disp = disp.add_channel();  // This object(other_disp) is depend on disp, so we must keep disp.show() running.
    err::check_bool_raise(disp.is_opened(), "display open failed");

    // touch screen
    touchscreen::TouchScreen touchscreen = touchscreen::TouchScreen();
    err::check_bool_raise(touchscreen.is_opened(), "touchscreen open failed");

    // init gui
    maix::lvgl_init(other_disp, &touchscreen);
    app_init(&disp);

    // main ui loop
    while (!app::need_exit())
    {
        // app loop
        app_loop();

        // lvgl loop
        lv_timer_handler();
    }

    app_deinit();

    return 0;
}


int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}

