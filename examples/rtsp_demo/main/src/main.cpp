
#include "stdio.h"
#include "main.h"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_display.hpp"
#include "maix_rtsp.hpp"
#include "maix_camera.hpp"
#include "csignal"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace maix;

int _main(int argc, char* argv[])
{
    int cnt = 0;
    camera::Camera cam = camera::Camera(640, 480, image::Format::FMT_YVU420SP);
    camera::Camera *cam2 = cam.add_channel(640, 480);
    display::Display disp = display::Display();
    rtsp::Rtsp rtsp = rtsp::Rtsp();
    rtsp.bind_camera(&cam);
    rtsp::Region *region = rtsp.add_region(0, 0, 200, 100);
    rtsp::Region *region3 = rtsp.add_region(400, 200, 200, 100);

    image::Image *rgn_img = region3->get_canvas();
    rgn_img->draw_rect(0, 0, rgn_img->width(), rgn_img->height(), image::COLOR_BLUE, 5);
    rgn_img->draw_string(0, 0, "hello");
    region3->update_canvas();

    log::info("%s\r\n", rtsp.get_url().c_str());
    rtsp.start();
    while(!app::need_exit()) {
        cnt ++;
        image::Color color = image::COLOR_BLACK;
        if (cnt == 1) {
            color = image::COLOR_BLACK;
        } else if (cnt == 2) {
            color = image::COLOR_RED;
        } else if (cnt == 3) {
            color = image::COLOR_GREEN;
        } else {
            color = image::COLOR_BLUE;
            cnt = 0;
        }
        rgn_img = region->get_canvas();
        rgn_img->draw_rect(0, 0, rgn_img->width(), rgn_img->height(), color, -1);
        region->update_canvas();

        maix::image::Image *img = cam2->read();
        disp.show(*img);
        delete img;
    }

    delete cam2;

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
