
#include "maix_rtmp.hpp"
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_video.hpp"
#include "maix_wifi.hpp"
#include "main.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <list>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}
#include "sophgo_middleware.hpp"

using namespace maix;
using namespace maix::network;

static int helper(void)
{
    printf( "========================\r\n"
            "Intput param:\r\n"
            "0 <host> <app> <stream> <file> : rtmp client, push file\r\n"
            "   example: ./rtmp_demo 0 192.168.0.30 myapp stream ./test.flv\r\n"
            "1 <host> <port> <app> <stream> <bitrate>: rtmp client, push camera image\r\n"
            "   example: ./rtmp_demo 1 192.168.0.30 1935 live stream 10000000\r\n"
            "========================\r\n");
    fflush(stdin);
    return 0;
}

int _main(int argc, char* argv[])
{
    int cmd = 2;
    if (argc > 1) {
        cmd = atoi(argv[1]);
    } else {
        helper();
        return 0;
    }

    switch (cmd) {
    case 0:
    {
        image::Image img = image::Image();
        if (argc < 6) {
            helper();
            break;
        }
        std::string host = argv[2];
        int port = 1935;
        std::string app = argv[3];
        std::string stream = argv[4];
        std::string file = argv[5];
        printf("push %s to rtmp://%s:%d/%s/%s!\r\n", &file[0], &host[0], port, &app[0], &stream[0]);

        rtmp::Rtmp rtmp = rtmp::Rtmp(host, port, app, stream);

        log::info("start\r\n");
        rtmp.start(file);

        while (!app::need_exit()) {
            log::info("run..\r\n");
            time::sleep(1);
        }
        rtmp.stop();
        log::info("stop\r\n");

        break;
    }
    case 1:
    {
        if (argc < 6) {
            helper();
            break;
        }
        std::string host = argv[2];
        int port = atoi(argv[3]);
        std::string app = argv[4];
        std::string stream = argv[5];
        int bitrate = 1000 * 1000;
        if (argc > 6) bitrate = atoi(argv[6]);


        camera::Camera cam = camera::Camera(1280, 720, image::Format::FMT_YVU420SP);
        audio::Recorder audio_recorder = audio::Recorder();
        display::Display disp = display::Display();
        rtmp::Rtmp rtmp = rtmp::Rtmp(host, port, app, stream, bitrate);

        rtmp.bind_camera(&cam);
        rtmp.bind_audio_recorder(&audio_recorder);
        rtmp.bind_display(&disp);

        log::info("start\r\n");
        log::info("rtmp://%s:%d/%s/%s", &host[0], port, &app[0], &stream[0]);
        rtmp.start();

        while (!app::need_exit()) {
            time::sleep(5);
        }
        rtmp.stop();
        log::info("stop\r\n");

        break;
    }
    default:
    {
        helper();
        break;
    }
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


