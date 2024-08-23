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
void helper(void)
{
    printf( "========================\r\n"
            "Intput param:\r\n"
            "0 <filepath> : decode the video and display. example: ./decoder_demo 0 test.h264\r\n"
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
        video::Decoder decoder = video::Decoder(filepath);
        display::Display disp = display::Display();
        uint64_t last_ms = time::ticks_ms();
        uint64_t wait_ms = 1000 / decoder.fps();
        log::info("resolution:%dx%d bitrate:%d duration:%.2f s fps:%d", decoder.width(), decoder.height(), decoder.bitrate(), decoder.duration(), decoder.fps());
        decoder.seek(0);
        while (!app::need_exit()) {
            image::Image *img = decoder.decode_video();
            if (img == nullptr) {
                if (decoder.seek(0) != err::ERR_NONE) {
                    break;
                }
                continue;
            }
            log::info("last pts:%.2f", decoder.last_pts());
            while (time::ticks_ms() - last_ms < wait_ms) {
                time::sleep_ms(1);
            }
            last_ms = time::ticks_ms();
            disp.show(*img);
            delete img;
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

