
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_classifier.hpp"
#include "main.h"

using namespace maix;

int _main(int argc, char* argv[])
{
    int ret = 0;
    log::info("Program start");
    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path image_path";

    if(argc < 2)
    {
        log::error(help.c_str());
        return -1;
    }

    const char *model_path = argv[1];
    bool dual_buff = true;

    log::info("model path: %s", model_path);
    nn::Classifier classifier(model_path, dual_buff);
    log::info("load classifier model %s success", model_path);

    if(argc >= 3)
    {
        const char *img_path = argv[2];
        log::info("load image now");
        maix::image::Image *img = maix::image::load(img_path, classifier.input_format());
        err::check_null_raise(img, "load image " + std::string(img_path) + " failed");
        log::info("load image %s success: %s", img_path, img->to_str().c_str());
        if(img->width() != classifier.input_size().width() || img->height() != classifier.input_size().height())
        {
            log::warn("image size not match model input size, will auto resize from %dx%d to %dx%d", img->width(), img->height(), classifier.input_size().width(), classifier.input_size().height());
        }
        std::vector<std::pair<int, float>> * result = classifier.classify(*img);
        for(int i = 0; i < 5; ++i)
        {
            log::info("top %d: %5.2f%% %s, ", i + 1, result->at(i).second * 100, classifier.labels[result->at(i).first].c_str());
        }
        delete result;
        delete img;
    }
    else
    {
        char msg[64];
        log::info("open camera now");
        image::Size input_size = classifier.input_size();
        camera::Camera cam = camera::Camera(input_size.width(), input_size.height(), classifier.input_format());
        log::info("open camera success");
        display::Display disp = display::Display();
        while(!app::need_exit())
        {
            uint64_t t = time::ticks_ms();
            image::Image *img = cam.read();
            err::check_null_raise(img, "read camera failed");
            std::vector<std::pair<int, float>> * result = classifier.classify(*img);
            int max_idx = result->at(0).first;
            float max_score = result->at(0).second;
            snprintf(msg, sizeof(msg), "%5.2f: %s", max_score, classifier.labels[max_idx].c_str());
            img->draw_string(10, 10, msg, image::COLOR_RED);
            disp.show(*img);
            delete result;
            delete img;
            log::info("time: %d ms", time::ticks_ms() - t);
        }
    }

    log::info("Program exit");

    return ret;
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


