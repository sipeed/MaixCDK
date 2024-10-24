
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
    "0 <path> <width> <height>  <fps> <bitrate> : video recorder\r\n"
    "\r\n"
    "Example: ./video 0 output.mp4\r\n"
    "Example: ./encode_demo 0 output.mp4 640 480 8 30 50 3000000 1000 1\r\n"
    "==================================\r\n", image::FMT_YVU420SP, video::VIDEO_H264, video::VIDEO_H265);
}

int _main(int argc, char* argv[])
{
    image::Image img = image::Image();  // do nothing
    int cmd = 0;
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            helper();
            return 0;
        } else {
            cmd = atoi(argv[1]);
        }
    } else {
        helper();
        return 0;
    }

    switch (cmd) {
    case 0:
    {
        std::string path = "/root/output.mp4";
        int width = 2560;
        int height = 1440;
        int fps = 30;
        int bitrate = 3000 * 1000;
        if (argc > 2) path = argv[2];
        if (argc > 3) width = atoi(argv[3]);
        if (argc > 4) height = atoi(argv[4]);
        if (argc > 5) fps = atoi(argv[7]);
        if (argc > 6) bitrate = atoi(argv[9]);
        log::info("path:%s width:%d height:%d fps:%d bitrate:%d\r\n",
            path.c_str(), width, height, fps, bitrate);
        camera::Camera cam = camera::Camera(width, height, image::Format::FMT_YVU420SP);
        display::Display disp = display::Display();
        audio::Recorder audio_recorder = audio::Recorder();
        video::VideoRecorder video_recorder = video::VideoRecorder();
        // ext_dev::imu::IMU imu = ext_dev::imu::IMU("qmi8658", 4, 0x6B, 400000,
        //                                         ext_dev::imu::Mode::DUAL,
        //                                         ext_dev::imu::AccScale::ACC_SCALE_16G,
        //                                         ext_dev::imu::AccOdr::ACC_ODR_8000,
        //                                         ext_dev::imu::GyroScale::GYRO_SCALE_1024DPS,
        //                                         ext_dev::imu::GyroOdr::GYRO_ODR_8000);
        video_recorder.bind_display(&disp);
        video_recorder.bind_camera(&cam);
        video_recorder.bind_audio(&audio_recorder);
        // video_recorder.bind_imu(&imu);

        video_recorder.reset();
        video_recorder.mute(false);
        video_recorder.volume(10);
        video_recorder.config_path(path);
        video_recorder.config_bitrate(bitrate);
        video_recorder.config_fps(fps);
        video_recorder.config_snapshot(true, {640, 480}, image::Format::FMT_RGB888);
        auto resolution = video_recorder.get_resolution();
        log::info("path:%s\n"
                "resolution:%dx%d\n"
                "fps:%d\n"
                "bitrate:%d\n"
                "mute:%d\n"
                "volume:%d\n",
                video_recorder.get_path().c_str(),
                resolution[0], resolution[1],
                video_recorder.get_fps(),
                video_recorder.get_bitrate(),
                video_recorder.mute(),
                video_recorder.volume());
        video_recorder.record_start();

        while(!app::need_exit()) {
            auto seek = video_recorder.seek();
            log::info("record time:%ld\r\n", seek);
            video_recorder.draw_rect(0, 50, 50, 500, 300, image::COLOR_WHITE, 2);
            time::sleep(1);
        }
        log::info("record time total:%ld\r\n", video_recorder.seek());
        video_recorder.record_finish();

        app::set_exit_flag(false);
        log::info("Enter idle status..");
        while(!app::need_exit()) {
            image::Image *snapshot_img = video_recorder.snapshot();
            if (snapshot_img) {
                log::info("snapshot image(%p) resolution:%dx%d size:%d format:%s", snapshot_img, snapshot_img->width(), snapshot_img->height(), snapshot_img->data_size(),image::fmt_names[snapshot_img->format()].c_str());
                delete snapshot_img;
            } else {
                log::info("Not found image.");
            }

            video_recorder.draw_rect(0, 50, 50, 300, 100, image::COLOR_RED, 4);
            time::sleep_ms(100);
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
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
