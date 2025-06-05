
#include "maix_basic.hpp"
#include "maix_nn_melotts.hpp"
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
    std::string help = "Usage: " + std::string(argv[0]) + " <mud_model_path> <text> <wav_path> <language> <output_pcm> <speed> <noise_scale> <noise_scale_w> <sdp_ratio>\n";
    // ./nn_melotts /root/models/melotts/melotts-zh.mud "端侧视觉，快速部署" output.wav zh  0.8 0.3 0.6 0.2 0

    if (argc < 3)
    {
        log::info(help.c_str());
        return -1;
    }

    std::string model_path = argv[1];
    std::string text = argv[2];
    std::string wav_path = argc < 3 ? "output.wav" : argv[3];
    std::string language = argc < 4 ? "zh" : argv[4];
    double speed = argc < 5 ? 0.8f : std::stof(argv[5]);
    double noise_scale = argc < 6 ? 0.3f : std::stof(argv[6]);
    double noise_scale_w = argc < 7 ? 0.6f : std::stof(argv[7]);
    double sdp_ratio = argc < 8 ? 0.2f : std::stof(argv[8]);
    bool output_pcm = argc < 9 ? false : std::stoi(argv[9]);

    log::info("model path: %s", model_path.c_str());
    log::info("text: %s", text.c_str());
    log::info("wav_path: %s", wav_path.c_str());
    log::info("language: %s", language.c_str());
    log::info("speed: %f", speed);
    log::info("noise_scale: %f", noise_scale);
    log::info("noise_scale_w: %f", noise_scale_w);
    log::info("sdp_ratio: %f", sdp_ratio);
    log::info("output_pcm: %d", output_pcm);

    nn::MeloTTS melotts("", language, speed, noise_scale, noise_scale_w, sdp_ratio);
    e = melotts.load(model_path);
    err::check_raise(e, "load model failed");
    log::info("load melotts model %s success", model_path.c_str());

    log::info("melotts start now");
    auto t = time::ticks_ms();
    auto pcm = melotts.forward(text, wav_path, output_pcm);
    auto t2 = time::ticks_ms();
    log::info("melotts forward cost %d ms", t2 - t);
    log::info("save wav file to %s", wav_path.c_str());
    if (pcm) {
        log::info("Get pcm data:%p size:%d", pcm->data, pcm->data_len);
    }

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