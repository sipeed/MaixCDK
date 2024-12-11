#include "stdio.h"
#include "main.h"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_display.hpp"
#include "maix_rtsp.hpp"
#include "maix_camera.hpp"
#include "maix_basic.hpp"
#include "csignal"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace maix;

int _main(int argc, char* argv[])
{
    int cam_w = -1;
    int cam_h = -1;
    image::Format cam_fmt = image::Format::FMT_YVU420SP;
    int cam_fps = -1;
    int cam_buffer_num = 3;
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            log::info("./rtsp_demo <width> <height> <format> <fps> <buff_num>");
            log::info("example: ./rtsp_demo 640 480");
            exit(0);
        } else {
            cam_w = atoi(argv[1]);
        }
    }
    if (argc > 2) cam_h = atoi(argv[2]);
    if (argc > 3) cam_fmt = (image::Format)atoi(argv[3]);
    if (argc > 4) cam_fps = atoi(argv[4]);
    if (argc > 5) cam_buffer_num = atoi(argv[5]);
    log::info("Camera width:%d height:%d format:%s fps:%d buffer_num:%d", cam_w, cam_h, image::fmt_names[cam_fmt].c_str(), cam_fps, cam_buffer_num);

    camera::Camera cam = camera::Camera(cam_w, cam_h, cam_fmt, "", cam_fps, cam_buffer_num);
    camera::Camera *cam2 = cam.add_channel(640, 480);
    display::Display disp = display::Display();
    auto audio_recorder = audio::Recorder();
    rtsp::Rtsp rtsp = rtsp::Rtsp();
    rtsp.bind_camera(&cam);
    rtsp.bind_audio_recorder(&audio_recorder);
    rtsp::Region *region = rtsp.add_region(0, 0, 200, 100);
    rtsp::Region *region3 = rtsp.add_region(400, 200, 200, 100);

    image::Image *rgn_img = region3->get_canvas();
    rgn_img->draw_rect(0, 0, rgn_img->width(), rgn_img->height(), image::COLOR_BLUE, 5);
    rgn_img->draw_string(0, 0, "hello");
    region3->update_canvas();

    log::info("url:%s", rtsp.get_url().c_str());
    std::vector<std::string> url = rtsp.get_urls();
    for (size_t i = 0; i < url.size(); i ++) {
        log::info("url[%d]:%s", i, url[i].c_str());
    }
    err::check_raise(rtsp.start());

    uint64_t last_ms = time::ticks_ms();
    int cnt = 0;
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
        delete rgn_img;

        maix::image::Image *img = nullptr;
        try {
            img = cam2->read();
        } catch (std::exception &e) {
            time::sleep_ms(10);
            continue;
        }

        disp.show(*img);
        delete img;
        uint64_t curr_ms = time::ticks_ms();
        log::info("loop use %lld ms\r\n", curr_ms - last_ms);
        last_ms = curr_ms;
    }

    rtsp.stop();
    delete cam2;

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