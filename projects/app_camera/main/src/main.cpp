
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
    app_pre_init();
    app_base_init();

    // main ui loop
    while (!app::need_exit())
    {
        int res = 0;
        if ((res = app_base_loop()) < 0) {
            log::info("find some error(%d), try to exit..", res);
            break;
        }
    }

    app_base_deinit();
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

