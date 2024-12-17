#include <bits/stdc++.h>
#include <functional>

using namespace std;

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_display.hpp"
#include "maix_camera.hpp"
#include "maix_uvc_stream.hpp"
#include "maix_basic.hpp"

using namespace maix;

#include "uvcServer.hpp"

static std::unique_ptr<camera::Camera> pCam = nullptr;
// extern "C" 
int my_uvc_video_fill_mjpg_buffer(void *buf, uint32_t *size) {
    static uint64_t frame_count = 0;
    static uint64_t last_ms;
    if (!pCam) return 1;

uint64_t curr_ms_thisframe = time::ticks_ms();
    image::Image *img = pCam->read();
    if (!img) return 1;
uint64_t curr_ms_takeframe = time::ticks_ms();
    std::ostringstream oss;
    oss << "frame: " << frame_count;
    img->draw_string(4, 32, oss.str(), image::Color::from_rgb(255, 0, 0), 1.3);

    image::Image *img_mjpg = img;
    if (img->format() != image::Format::FMT_JPEG) {
        img_mjpg = img->to_jpeg(95);
        delete img;
    }
    if (!img_mjpg) return 1;
uint64_t curr_ms_mjpgframe = time::ticks_ms();

    memcpy(buf, img_mjpg->data(), img_mjpg->data_size());
    *size = img_mjpg->data_size();
#if 0
    char result_bin_name[32];
    sprintf((char *)result_bin_name, "/root/res/res_%llu.jpg", frame_count);
    ofstream((const char*)result_bin_name, std::ios::binary)
        .write(reinterpret_cast<char*>(img_mjpg->data()), img_mjpg->data_size());
#endif
    delete img_mjpg;

    uint64_t curr_ms = time::ticks_ms();
    static uint32_t flip = 1;
    if(flip^=1) log::info("[%llu]loop use %lld ms, %.2ffps, %d bytes\r\n"
                        "\tstart: %lld ms\r\n"
                        "\ttake: %lld ms\r\n"
                        "\tto_mjpg: %lld ms\r\n", frame_count++, curr_ms - last_ms, 1000.f/(curr_ms - last_ms), *size,
                        curr_ms_thisframe - last_ms, curr_ms_takeframe - curr_ms_thisframe, curr_ms_mjpgframe - curr_ms_takeframe);
    last_ms = curr_ms;
    return 0;
}

// extern "C" int uvc_main_exist;
// extern "C" int uvc_main(int argc, char *argv[]);

int _main(int argc, char* argv[])
{
    int cam_w = -1;
    int cam_h = -1;
    int cam_fps = -1;
    int cam_buffer_num = 3;
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            log::info("./uvc_demo <width> <height> <fps> <buff_num>");
            log::info("example: ./uvc_demo 640 480 60 3");
            exit(0);
        } else {
            cam_w = atoi(argv[1]);
        }
    }
    if (argc > 2) cam_h = atoi(argv[2]);
    if (argc > 3) cam_fps = atoi(argv[3]);
    if (argc > 4) cam_buffer_num = atoi(argv[4]);
    log::info("Camera width:%d height:%d fps:%d buffer_num:%d", cam_w, cam_h, cam_fps, cam_buffer_num);

    // image::Format cam_fmt = image::Format::FMT_YVU420SP;
    image::Format cam_fmt = image::Format::FMT_RGB888;
    camera::Camera cam = camera::Camera(cam_w, cam_h, cam_fmt, "", cam_fps, cam_buffer_num);
    if(!pCam) pCam = make_unique<camera::Camera>(std::move(cam));

    // std::thread uvc_daemon_thrd([&argv] () {
    //     log::info("Switch to uvc_main...");
    //     char* uvc_main_args[] = {argv[0], "-u", "/dev/video0", "-d", "-i", "/bin/cat_224.jpg"};
    //     uvc_main(sizeof(uvc_main_args)/sizeof(uvc_main_args[0]), uvc_main_args);
    // });

    UvcServer *pUvc = UvcServer::getInstance();
    pUvc->set_cb(my_uvc_video_fill_mjpg_buffer);
    pUvc->run();
    while (!app::need_exit()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    pUvc->stop();
    // uvc_main_exist = 1;
    
    // uvc_daemon_thrd.join();

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