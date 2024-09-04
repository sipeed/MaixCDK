#include "maix_vision.hpp"
#include "main.h"
#include "sophgo_middleware.hpp"
using namespace maix;
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <list>
void helper(void)
{
    printf( "========================\r\n"
            "Intput param:\r\n"
            "0 <filepath> <seek_s> : decode the video and display. example: ./decoder_demo 0 test.h264\r\n"
            "========================\r\n");
}

int _main(int argc, char* argv[])
{
    if (argc < 2) {
        helper();
        return -1;
    }

    int cmd = atoi(argv[1]);
    switch (cmd)
    {
    case 0: {
        if (argc < 3) {
            helper();
            return -1;
        }
        std::string filepath = argv[2];
        double seek_s = 0.0;
        if (argc > 3) seek_s = atof(argv[3]);
        video::Decoder decoder = video::Decoder(filepath);
        display::Display disp = display::Display();
        uint64_t loop_ms = time::ticks_ms(), last_ms = loop_ms;
        log::info("resolution:%dx%d bitrate:%d duration:%.2f s fps:%d seek_s:%f", decoder.width(), decoder.height(), decoder.bitrate(), decoder.duration(), decoder.fps(), seek_s);

        std::vector<int> timebase = decoder.timebase();
        err::check_bool_raise(decoder.seek(seek_s) >= 0, "decoder.seek failed");
        log::info("decoder.seek:%f", decoder.seek());
        while (!app::need_exit()) {
            video::Context *ctx = decoder.decode_video();
            if (!ctx) {
                log::info("decode video over");
                break;
            }
            if (ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
                image::Image *img = ctx->image();
                disp.show(*img);

                double wait_ms = (double)ctx->duration() * 1000 / (timebase[0] / timebase[1]);
                while ((double)(time::ticks_ms() - last_ms) < wait_ms) {
                    time::sleep_us(1);
                }
                last_ms = time::ticks_ms();

                delete img;
                delete ctx;
            }
            log::info("loop time:%.2d ms", (time::ticks_ms() - loop_ms));
            loop_ms = time::ticks_ms();
        }
        break;
    }
    default:
        helper();
        break;
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

