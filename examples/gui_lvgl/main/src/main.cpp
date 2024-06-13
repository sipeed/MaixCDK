/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "maix_basic.hpp"
#include "maix_display.hpp"
#include "maix_lvgl.hpp"
#include "lvgl.h"
#include "ui.h"

using namespace maix;


int _main(int argc, char* argv[])
{

    // init display
    display::Display screen = display::Display();

    // touch screen
    touchscreen::TouchScreen touchscreen = touchscreen::TouchScreen();

    // init lvgl
    lvgl_init(&screen, &touchscreen);

    // init lvgl ui
    ui_init();

    // main ui loop
    while (!app::need_exit())
    {
        /* Periodically call the lv_task handler.
            * It could be done in a timer interrupt or an OS task too.
            */
        uint32_t time_till_next = lv_timer_handler();
        time::sleep_ms(time_till_next);
        if(!screen.is_opened())
        {
            break;
        }
    }

    lvgl_destroy();

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

