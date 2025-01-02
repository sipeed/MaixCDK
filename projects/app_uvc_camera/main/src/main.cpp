#include <bits/stdc++.h>
#include <functional>

using namespace std;
using namespace chrono_literals;

#include "maix_basic.hpp"
#include "maix_fs.hpp"
#include "maix_vision.hpp"
#include "maix_touchscreen.hpp"
#include "maix_uvc_stream.hpp"

using namespace maix;

#include "main.h"

static std::unique_ptr<camera::Camera> pCam = nullptr;

int my_uvc_video_fill_mjpg_buffer(void *buf, uint32_t *size) {
    static uint64_t frame_count = 0;
    static uint64_t last_ms;
    if (!pCam) return 1;

    image::Image *img = pCam->read();
    if (!img) return 1;
    uint64_t curr_ms_takeframe = time::ticks_ms();

    std::ostringstream oss;
    oss << "frame[" << img->width() << "x" << img->height() << "]: " << frame_count;
    img->draw_string(4, 32, oss.str(), image::Color::from_rgb(255, 0, 0), 1.3);

    uvc::helper_fill_mjpg_image(buf, size, img);
    uint64_t curr_ms_mjpgframe = time::ticks_ms();

    delete img;
#if 0
    char result_bin_name[32];
    sprintf((char *)result_bin_name, "/root/res/res_%llu.jpg", frame_count);
    ofstream((const char*)result_bin_name, std::ios::binary)
        .write(reinterpret_cast<char*>(buf), *size);
#endif

    uint64_t curr_ms = time::ticks_ms();
    static uint32_t flip = 1;
    if(flip^=1) log::info("[%llu]loop use %lld ms, %.2ffps, %d bytes\r\n"
                        "\ttake: %lld ms\r\n"
                        "\tto_mjpg: %lld ms\r\n", frame_count++, curr_ms - last_ms, 1000.f/(curr_ms - last_ms), *size,
                        curr_ms_takeframe - last_ms, curr_ms_mjpgframe - curr_ms_takeframe);
    last_ms = curr_ms;
    return 0;
}

int _main(int argc, char *argv[])
{
    int ret = 0;
    log::info("Program start");


    image::Image *ret_img = image::load("./assets/ret.png", image::Format::FMT_RGB888);
    if (!ret_img)
    {
        log::error("load ret image failed");
        return -12345;
    }    
    display::Display disp = display::Display();
    image::Image img(disp.width(), disp.height());
    img.draw_image(0, 0, *ret_img);
    disp.show(img);

    log::info("open camera now");
    camera::Camera cam = camera::Camera(1280, 720, image::Format::FMT_RGB888, "", 60, 3);
    if(!pCam) pCam = make_unique<camera::Camera>(std::move(cam));
    log::info("open camera success");

    touchscreen::TouchScreen ts;
    int ts_x = 0, ts_y = 0;
    bool ts_pressed = false;

    std::unique_ptr<uvc::UvcServer> pUvc = std::make_unique<uvc::UvcServer>(my_uvc_video_fill_mjpg_buffer);
    if (fs::exists("/boot/usb.uvc")) {
        pUvc->run();
        img.draw_string(100, disp.height()/2, std::string("UVC started. Please use 'Guvcview'\n and mjpeg channel.\n'Cheese' on Ubuntu is incompatible."), image::Color::from_rgb(0, 255, 0), 1.3);
        disp.show(img);
    } else {
        img.draw_string(100, disp.height()/2, std::string("Enable UVC first.\n [App Settings/USB settings/UVC]"), image::Color::from_rgb(255, 255, 255), 1.3);
        disp.show(img);
    }
    while (!app::need_exit())
    {
        std::this_thread::sleep_for(300ms);
        ts.read(ts_x, ts_y, ts_pressed);
        if (ts_pressed && ts_x < 40 + 60 && ts_y < 40)
            break;
    }
    pUvc->stop();

    log::info("Program exit");

    return ret;
}

int main(int argc, char *argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
