
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
    "4 [record_time] [output_path]: record from camera and save to h265, then display\r\n"
    "5 : encode h265\r\n"
    "6 : bind camera and encode h265\r\n"
    "7 : encode h264\r\n"
    "8 : bind camera and encode h264\r\n"
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

        uint64_t start_ms = time::ticks_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            image::Image *img = cam.read();
            video::Packet *packet = v.encode(img);
            if (time::ticks_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }
            delete img;

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet->data(), packet->data_size(), time::ticks_ms() - last_loop);
            delete packet;
            last_loop = time::ticks_ms();
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

        uint64_t start_ms = time::ticks_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            video::Packet *packet = v.encode();
            if (time::ticks_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet->data(), packet->data_size(), time::ticks_ms() - last_loop);
            delete packet;
            last_loop = time::ticks_ms();
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

        uint64_t start_ms = time::ticks_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            image::Image *img = cam.read();
            video::Packet *packet = v.encode(img);
            if (time::ticks_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }
            delete img;

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet->data(), packet->data_size(), time::ticks_ms() - last_loop);
            delete packet;
            last_loop = time::ticks_ms();
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

        uint64_t start_ms = time::ticks_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            video::Packet *packet = v.encode();
            if (time::ticks_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet->data(), packet->data_size(), time::ticks_ms() - last_loop);
            delete packet;
            last_loop = time::ticks_ms();
        }
        break;
    }
    case 4:
    {
        uint64_t record_s = 5;
        std::string path = "output.h265";
        if (argc > 2) record_s = atoi(argv[2]);
        if (argc > 3) path = argv[3];
        log::info("Ready to record %ld s, and save to %s\r\n", record_s, path.c_str());

        camera::Camera cam = camera::Camera(640, 480, image::Format::FMT_YVU420SP);
        display::Display disp = display::Display();
        video::Video v = video::Video(path, 640, 480, image::Format::FMT_YVU420SP, 30, 30, true);
        v.bind_camera(&cam);

        uint64_t start_ms = time::ticks_ms();
        uint64_t last_loop = start_ms;
        int count = 0;
        while(!app::need_exit()) {
            video::Packet *packet = v.encode();
            if (time::ticks_ms() - start_ms > record_s * 1000) {
                log::info("finish\r\n");
                v.finish();
                app::set_exit_flag(true);
            }

            image::Image *img = v.capture();
            disp.show(*img);

            printf("Packet[%d] data:%p size:%ld use %ld ms\r\n", count ++, packet->data(), packet->data_size(), time::ticks_ms() - last_loop);
            delete packet;
            last_loop = time::ticks_ms();
        }
        break;
    }
    case 5:
    {
        int width = 640;
        int height = 480;
        video::VideoType type = video::VIDEO_H265_CBR;
        video::Encoder e = video::Encoder(width, height, image::Format::FMT_YVU420SP, type);
        camera::Camera cam = camera::Camera(width, height, image::Format::FMT_YVU420SP);

        while(!app::need_exit()) {
            image::Image *img = cam.read();
            video::Frame *frame = e.encode(img);
            printf("frame data:%p size:%ld pts:%ld dts:%ld\r\n",
                frame->data(), frame->size(), frame->get_pts(), frame->get_dts());
            delete frame;
            delete img;
        }
        break;
    }
    case 6:
    {
        int width = 640;
        int height = 480;
        video::VideoType type = video::VIDEO_H265_CBR;
        int framerate = 30;
        int gop = 50;
        int bitrate = 3000 * 1000;
        int time_base = 1000;
        bool capture = true;
        video::Encoder e = video::Encoder(width, height, image::Format::FMT_YVU420SP, type, framerate, gop, bitrate, time_base, capture);
        camera::Camera cam = camera::Camera(width, height, image::Format::FMT_YVU420SP);
        e.bind_camera(&cam);

        char *file = (char *)"output.h265";
        FILE *f = fopen(file, "wb");
        err::check_null_raise(f, "open file failed!");

        while(!app::need_exit()) {
            video::Frame *frame = e.encode();
            image::Image *img = e.capture();
            printf("frame data:%p size:%ld pts:%ld dts:%ld\r\n",
                frame->data(), frame->size(), frame->get_pts(), frame->get_dts());
            printf("image size:%d\r\n", img->data_size());
            fwrite(frame->data(), frame->size(), 1, f);
            delete frame;
            delete img;
        }
        fclose(f);
        system("sync");
        break;
    }
    case 7:
    {
        int width = 640;
        int height = 480;
        video::VideoType type = video::VIDEO_H264_CBR;
        video::Encoder e = video::Encoder(2560, 1440, image::Format::FMT_YVU420SP, type);
        camera::Camera cam = camera::Camera(width, height, image::Format::FMT_YVU420SP);

        while(!app::need_exit()) {
            image::Image *img = cam.read();
            video::Frame *frame = e.encode(img);
            printf("frame data:%p size:%ld pts:%ld dts:%ld\r\n",
                frame->data(), frame->size(), frame->get_pts(), frame->get_dts());
            delete frame;
            delete img;
        }
        break;
    }
    case 8:
    {
        int width = 640;
        int height = 480;
        video::VideoType type = video::VIDEO_H264_CBR;
        int framerate = 30;
        int gop = 50;
        int bitrate = 3000 * 1000;
        int time_base = 1000;
        bool capture = true;
        video::Encoder e = video::Encoder(width, height, image::Format::FMT_YVU420SP, type, framerate, gop, bitrate, time_base, capture);
        camera::Camera cam = camera::Camera(width, height, image::Format::FMT_YVU420SP);
        display::Display disp = display::Display();
        e.bind_camera(&cam);

        char *file = (char *)"output.h264";
        FILE *f = fopen(file, "wb");
        err::check_null_raise(f, "open file failed!");

        while(!app::need_exit()) {
            video::Frame *frame = e.encode();
            image::Image *img = e.capture();
            disp.show(*img);
            printf("frame data:%p size:%ld pts:%ld dts:%ld\r\n",
                frame->data(), frame->size(), frame->get_pts(), frame->get_dts());
            printf("image size:%d\r\n", img->data_size());
            fwrite(frame->data(), frame->size(), 1, f);
            delete frame;
            delete img;
        }

        fclose(f);
        system("sync");
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
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
