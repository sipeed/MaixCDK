
#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "main.h"


using namespace maix;


int _main(int argc, char* argv[])
{
    err::Err e;

    camera::Camera cam = camera::Camera();
    display::Display screen = display::Display();

    e = cam.open();
    err::check_raise(e, "camera open failed");

    e = screen.open();
    err::check_raise(e, "display open failed");


    log::info("camera and display open success\n");
    log::info("screen size: %dx%d\n", screen.width(), screen.height());

    int angle = 0;
    int x, y;
    int r = 30; // 30%
    int block_h = 15; // 15%
    while(!app::need_exit())
    {

        // read image from camera
        image::Image *img = cam.read();
        err::check_null_raise(img, "camera read failed");
        // if not use camera, you can use image::Image() to create a image

        // draw animation on center of image
        x = img->width() / 2;
        y = img->height() / 2;
        img->draw_circle(x, y, 2, image::Color::from_rgb(255, 255, 255), -1);
        int x2 = x + r * img->width() / 100.0 * sin(angle * 3.14 / 180);
        int y2 = y - r * img->width() / 100.0 * cos(angle * 3.14 / 180);
        img->draw_circle(x2, y2, 10, image::Color::from_rgb(255, 255, 255), -1);
        if(++angle == 360)
            angle = 0;
        img->draw_line(x, y, x2, y2, image::Color::from_rgb(255, 255, 255), 1);

        // draw text on image
        y = img->height() - block_h * img->height() / 100;
        img->draw_rect(0, y, img->width(), block_h * img->height() / 100, image::Color::from_rgb(255, 0, 0), -1);
        image::Size size = image::string_size("Application", 1.5);
        img->draw_string((img->width() - size.width()) / 2, y + (block_h * img->height() / 100 - size.height()) / 2, "Application", image::Color::from_rgb(255, 255, 255), 1.5);

        // check if screen is closed by user(mostly for PC), and show image on screen
        if(!screen.is_opened())
        {
            log::info("screen closed\n");
            break;
        }
        screen.show(*img);

        // free image data, important!
        delete img;
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

