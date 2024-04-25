

#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "main.h"

using namespace maix;


int _main(int argc, char* argv[])
{
    uint64_t t1, t2, t3;
    float fps = 0;
    char buf[128] = {0};

    camera::Camera cam = camera::Camera();
    display::Display screen = display::Display();
    log::info("camera and display open success\n");
    log::info("camera size: %dx%d\n", cam.width(), cam.height());
    log::info("screen size: %dx%d\n", screen.width(), screen.height());

    while(!app::need_exit())
    {
        // time when start read image from camera
        t1 = time::time_ms();

        // read image from camera
        image::Image *img = cam.read();
        err::check_null_raise(img, "camera read failed");

        // time when read image finished
        t2 = time::time_ms();

        // draw fps on image
        img->draw_string(0, 10, buf, image::Color::from_rgb(255, 0, 0), 1.5);

        // check if screen is closed by user(mostly for PC), and show image on screen
        if(!screen.is_opened())
        {
            log::info("screen closed\n");
            break;
        }
        screen.show(*img);

        // free image data, important!
        delete img;

        // calculate fps
        t3 = time::time_ms();
        fps = 1000.0f/(t3-t1);
        snprintf(buf, sizeof(buf), "cam: %ld, disp: %ld, all: %ld (ms), fps: %.2f", t2-t1, t3-t2, t3-t1, fps);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch SIGINT signal(e.g. Ctrl + C), and set exit flag to true.
    signal(SIGINT, [](int sig){ app::set_exit_flag(true); });

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}

