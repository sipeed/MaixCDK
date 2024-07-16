
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_classifier.hpp"
#include "main.h"
#include "maix_touchscreen.hpp"

using namespace maix;

int _main(int argc, char *argv[])
{
    int ret = 0;
    char tmp_chars[128] = {0};
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
    // int max_len = w > h ? w : h;
    log::info("open camera now");
    // image::Size input_size = classifier.input_size();
    camera::Camera cam = camera::Camera(w, h, classifier.input_format());
    log::info("open camera success");
    uint64_t t, t2, t3, t_show, t_all = 0;
    while (!app::need_exit())
    {
        ts.read(ts_x, ts_y, ts_pressed);
        if (ts_pressed && ts_x < 40 + 60 && ts_y < 40)
        {
            break;
        }
        t = time::ticks_ms();
        image::Image *img = cam.read();
        err::check_null_raise(img, "read camera failed");
        image::Image *img_final = img->resize(classifier.input_width(), classifier.input_height(), image::Fit::FIT_COVER);
        err::check_null_raise(img_final, "resize failed");
        t2 = time::ticks_ms();
        std::vector<std::pair<int, float>> *result = classifier.classify(*img_final);
        t3 = time::ticks_ms();
        int max_idx = result->at(0).first;
        float max_score = result->at(0).second;
        img->draw_rect((w - min_len) / 2, (h - min_len) / 2, min_len, min_len, image::COLOR_WHITE, 2);
        snprintf(msg, sizeof(msg), "%4.1f %%:\n%s", max_score * 100, classifier.labels[max_idx].c_str());
        img->draw_string((w - min_len) / 2, disp.height() - 80, msg, image::COLOR_RED, 2, 2);
        img->draw_image(0, 0, *ret_img);
        snprintf(tmp_chars, sizeof(tmp_chars), "All: %ldms, cam: %ldms\ndetect: %ldms, show: %ldms", t_all, t2 - t, t3 - t2, t_show);
        img->draw_string((w - min_len) / 2, 4, tmp_chars, image::COLOR_RED, 1.5, 2);
        disp.show(*img);
        t_show = time::ticks_ms() - t3;
        t_all = time::ticks_ms() - t;
        delete result;
        delete img;
        delete img_final;
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
