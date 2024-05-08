
#include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_display.hpp"
#include "maix_video.hpp"
#include "maix_camera.hpp"

using namespace maix;

static void helper(void)
{
    log::info(
    "==================================\r\n"
    "Please input command:\r\n"
    "0 [record_time] [output_path]: encode image and save to mp4\r\n"
    "1 [record_time] [output_path]: record from camera and save to mp4\r\n"
    "2 [record_time] [output_path]: encode image and save to h265\r\n"
    "3 [record_time] [output_path]: record from camera and save to h265\r\n"
    "\r\n"
    "Example: ./video_demo 0 5 output.mp4     # means record 5s from camera, and save to output.mp4\r\n"
    "==================================\r\n");
}

int _main(int argc, char* argv[])
{
    int cmd = 0;
    if (argc > 1) {
        cmd = atoi(argv[1]);
    } else {
        helper();
        return 0;
    }

    switch (cmd) {
    case 0:
    {
        uint64_t record_s = 5;
        std::string path = "output.mp4";
        if (argc > 2) record_s = atoi(argv[2]);
        if (argc > 3) path = argv[3];
        log::info("Ready to record %ld s, and save to %s\r\n", record_s, path.c_str());

        camera::Camera cam = camera::Camera(2560, 1440, image::Format::FMT_YVU420SP);
        video::Video v = video::Video(path);

        uint64_t start_ms = time::time_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            image::Image *img = cam.read();
            video::Packet packet = v.encode(img);
            if (time::time_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }
            delete img;

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet.data(), packet.data_size(), time::time_ms() - last_loop);
            last_loop = time::time_ms();
        }
        break;
    }
    case 1:
    {
        uint64_t record_s = 5;
        std::string path = "output.mp4";
        if (argc > 2) record_s = atoi(argv[2]);
        if (argc > 3) path = argv[3];
        log::info("Ready to record %ld s, and save to %s\r\n", record_s, path.c_str());

        camera::Camera cam = camera::Camera(2560, 1440, image::Format::FMT_YVU420SP);
        video::Video v = video::Video(path);
        v.bind_camera(&cam);

        uint64_t start_ms = time::time_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            video::Packet packet = v.encode();
            if (time::time_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet.data(), packet.data_size(), time::time_ms() - last_loop);
            last_loop = time::time_ms();
        }
        break;
    }
    case 2:
    {
        uint64_t record_s = 5;
        std::string path = "output.h265";
        if (argc > 2) record_s = atoi(argv[2]);
        if (argc > 3) path = argv[3];
        log::info("Ready to record %ld s, and save to %s\r\n", record_s, path.c_str());

        camera::Camera cam = camera::Camera(2560, 1440, image::Format::FMT_YVU420SP);
        video::Video v = video::Video(path);

        uint64_t start_ms = time::time_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            image::Image *img = cam.read();
            video::Packet packet = v.encode(img);
            if (time::time_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }
            delete img;

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet.data(), packet.data_size(), time::time_ms() - last_loop);
            last_loop = time::time_ms();
        }
        break;
    }
    case 3:
    {
        uint64_t record_s = 5;
        std::string path = "output.h265";
        if (argc > 2) record_s = atoi(argv[2]);
        if (argc > 3) path = argv[3];
        log::info("Ready to record %ld s, and save to %s\r\n", record_s, path.c_str());

        camera::Camera cam = camera::Camera(2560, 1440, image::Format::FMT_YVU420SP);
        video::Video v = video::Video(path);
        v.bind_camera(&cam);

        uint64_t start_ms = time::time_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            video::Packet packet = v.encode();
            if (time::time_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet.data(), packet.data_size(), time::time_ms() - last_loop);
            last_loop = time::time_ms();
        }
        break;
    }
    default:
        helper();
        return 0;
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
