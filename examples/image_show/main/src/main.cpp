

#include "maix_basic.hpp"
#include "maix_display.hpp"
#include "maix_image.hpp"
#include "main.h"

using namespace maix;


int _main(int argc, char* argv[])
{
    if(argc < 2)
    {
        log::error("Usage: %s <image file>\n", argv[0]);
        return -1;
    }
    uint64_t t1, t2, t3;
    char buf[128] = {0};

    display::Display screen = display::Display();
    log::info("screen size: %dx%d\n", screen.width(), screen.height());

    // time when start read image from camera
    t1 = time::ticks_ms();

    // read image from camera
    maix::image::Image *img = maix::image::load(argv[1]);
    err::check_null_raise(img, "camera read failed");

    // time when read image finished
    t2 = time::ticks_ms();

    screen.show(*img);

    // free image data, important!
    delete img;

    // calculate fps
    t3 = time::ticks_ms();
    snprintf(buf, sizeof(buf), "load: %ld, disp: %ld, all: %ld (ms)", t2-t1, t3-t2, t3-t1);
    while(!app::need_exit())
    {
        // check if screen is closed by user(mostly for PC), and show image on screen
        if(!screen.is_opened())
        {
            log::info("screen closed\n");
            break;
        }
        time::sleep_ms(50);
    }

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

