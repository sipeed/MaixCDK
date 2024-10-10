
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_yolo11.hpp"
#include "main.h"

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

    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path <image_path>";

    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }

    const char *model_path = argv[1];
    float conf_threshold = 0.5;
    float iou_threshold = 0.45;
    bool dual_buff = true;


    nn::YOLO11 detector("", dual_buff);
    e = detector.load(model_path);
    err::check_raise(e, "load model failed");
    log::info("load yolo11 model %s success", model_path);

    if (argc >= 3)
    {
        const char *img_path = argv[2];
        log::info("load image now");
        maix::image::Image *img = maix::image::load(img_path, img_fmt);
        err::check_null_raise(img, "load image " + std::string(img_path) + " failed");
        log::info("load image %s success: %s", img_path, img->to_str().c_str());
        if (img->width() != detector.input_size().width() || img->height() != detector.input_size().height())
        {
            log::warn("image size not match model input size, will auto resize from %dx%d to %dx%d", img->width(), img->height(), detector.input_size().width(), detector.input_size().height());
        }
        log::info("detect now");
        nn::Objects *result = detector.detect(*img, conf_threshold, iou_threshold);
        if(result->size() == 0)
        {
            log::info("no object detected !");
        }
        for (auto &r : *result)
        {

            log::info("result: %s", r->to_str().c_str());
            img->draw_rect(r->x, r->y, r->w, r->h, maix::image::Color::from_rgb(255, 0, 0));
            snprintf(tmp_chars, sizeof(tmp_chars), "%s: %.2f", detector.labels[r->class_id].c_str(), r->score);
            img->draw_string(r->x, r->y, detector.labels[r->class_id], maix::image::Color::from_rgb(255, 0, 0));
        }
        img->save("result.jpg");
        delete result;
        delete img;
    }
    else
    {
        log::info("open camera now");
        maix::image::Size input_size = detector.input_size();
        camera::Camera cam = camera::Camera(input_size.width(), input_size.height(), detector.input_format());
        log::info("open camera success");
        display::Display disp = display::Display();
        while (!app::need_exit())
        {
            uint64_t t = time::ticks_ms();
            maix::image::Image *img = cam.read();
            err::check_null_raise(img, "read camera failed");
            uint64_t t2 = time::ticks_ms();
            nn::Objects *result = detector.detect(*img);
            uint64_t t3 = time::ticks_ms();
            for (auto &r : *result)
            {
                log::info("result: %s", r->to_str().c_str());
                img->draw_rect(r->x, r->y, r->w, r->h, maix::image::Color::from_rgb(255, 0, 0));
                img->draw_string(r->x, r->y, detector.labels[r->class_id], maix::image::Color::from_rgb(255, 0, 0));
            }
            disp.show(*img);
            delete result;
            delete img;
            log::info("time: all %d ms, detect %d ms, fps: %.1f", time::ticks_ms() - t, t3 - t2, time::fps());
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
