
#include "maix_basic.hpp"
#include "maix_nn_whisper.hpp"
#include "main.h"
#ifndef PLATFORM_MAIXCAM2
#error "This demo only support maixcam2"
#else
using namespace maix;

int _main(int argc, char *argv[])
{
    image::Image img;

    int ret = 0;
    err::Err e;
    std::string help = "Usage: " + std::string(argv[0]) + " <mud_model_path> <wav_path>";

    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }

    const char *model_path = argv[1];
    std::string language = "zh";

    nn::Whisper whisper("", language);
    e = whisper.load(model_path);
    err::check_raise(e, "load model failed");
    log::info("load whisper model %s success", model_path);

    if (argc >= 3)
    {
        std::string wav_path = argv[2];
        log::info("start converting now");

        auto t = time::ticks_ms();
        auto result = whisper.forward(wav_path);
        auto t2 = time::ticks_ms();
        log::info("whisper forward cost %d ms", t2 - t);

        if(result.size() == 0)
        {
            log::info("whisper result is null!");
        } else {
            log::info("whisper result: %s", result.c_str());
        }
    }
    log::info("Program exit");

    return ret;
}

int main(int argc, char *argv[])
{
    // Catch SIGINT signal(e.g. Ctrl + C), and set exit flag to true.
    signal(SIGINT, [](int sig)
           { app::set_exit_flag(true); });

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
#endif