
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_yolov5.hpp"
#include "main.h"
#include "maix_touchscreen.hpp"

using namespace maix;

int _main(int argc, char *argv[])
{
    log::info("Program start");
    std::string model_type = "unknown";
    int ret = 0;
    err::Err e;
    maix::image::Format img_fmt = maix::image::FMT_RGB888;
    std::vector<float> mean = {};
    std::vector<float> scale = {};
    char tmp_chars[64] = {0};

    touchscreen::TouchScreen ts;
    int ts_x = 0, ts_y = 0;
    bool ts_pressed = false;
    image::Image *ret_img = image::load("./assets/ret.png", image::Format::FMT_RGB888);
    if (!ret_img)
    {
        log::error("load ret image failed");
        return -12345;
    }

    const char *model_path = "/root/models/yolov5s.mud";
    float conf_threshold = 0.5;
    float iou_threshold = 0.45;

    nn::YOLOv5 detector;
    e = detector.load(model_path);
    err::check_raise(e, "load model failed");
    log::info("load yolov5 model %s success", model_path);

    maix::image::Size input_size = detector.input_size();
    camera::Camera cam = camera::Camera(input_size.width(), input_size.height(), detector.input_format());
    log::info("open camera success");
    display::Display disp = display::Display();
    while (!app::need_exit())
    {
        ts.read(ts_x, ts_y, ts_pressed);
        if (ts_pressed && ts_x < 64 && ts_y < 40)
        {
            break;
        }
        uint64_t t = time::ticks_us();
        maix::image::Image *img = cam.read();
        err::check_null_raise(img, "read camera failed");
        std::vector<nn::Object> *result = detector.detect(*img);
        for (auto &r : *result)
        {
            log::info("result: %s", r.to_str().c_str());
            img->draw_rect(r.x, r.y, r.w, r.h, maix::image::Color::from_rgb(255, 0, 0));
            img->draw_string(r.x, r.y, detector.labels[r.class_id], maix::image::Color::from_rgb(255, 0, 0));
        }
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
