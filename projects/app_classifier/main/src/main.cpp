
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_classifier.hpp"
#include "main.h"
#include "maix_touchscreen.hpp"

using namespace maix;

int _main(int argc, char *argv[])
{
    int ret = 0;
    log::info("Program start");

    const char *model_path = "/root/models/mobilenetv2.mud";

    touchscreen::TouchScreen ts;
    int ts_x = 0, ts_y = 0;
    bool ts_pressed = false;
    image::Image *ret_img = image::load("./assets/ret.png", image::Format::FMT_RGB888);
    if (!ret_img)
    {
        log::error("load ret image failed");
        return -12345;
    }

    log::info("model path: %s", model_path);
    nn::Classifier classifier(model_path);
    log::info("load classifier model %s success", model_path);

    char msg[64];
    display::Display disp = display::Display();
    int w = disp.width();
    int h = disp.height();
    int min_len = w > h ? h : w;
    int max_len = w > h ? w : h;
    log::info("open camera now");
    // image::Size input_size = classifier.input_size();
    camera::Camera cam = camera::Camera(min_len, min_len, classifier.input_format());
    log::info("open camera success");
    while (!app::need_exit())
    {
        ts.read(ts_x, ts_y, ts_pressed);
        int x = (max_len - min_len) / 2;
        if (ts_pressed && ts_x >= x && ts_x < x + 60 && ts_y < 40)
        {
            break;
        }
        uint64_t t = time::ticks_us();
        image::Image *img = cam.read();
        err::check_null_raise(img, "read camera failed");
        std::vector<std::pair<int, float>> *result = classifier.classify(*img);
        int max_idx = result->at(0).first;
        float max_score = result->at(0).second;
        snprintf(msg, sizeof(msg), "%5.2f: %s", max_score, classifier.labels[max_idx].c_str());
        img->draw_string(2, 36, msg, image::COLOR_RED, 1.5);

        img->draw_image(0, 0, *ret_img);
        disp.show(*img);
        delete result;
        delete img;
        log::info("time: %d ms", time::ticks_us() - t);
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
