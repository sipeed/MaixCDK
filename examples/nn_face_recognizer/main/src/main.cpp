
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "main.h"

#include "maix_nn_face_recognizer.hpp"

using namespace maix;

int _main(int argc, char *argv[])
{
    log::info("Program start");
    std::string model_type = "unknown";
    int ret = 0;
    err::Err e;
    char tmp_chars[64] = {0};

    std::string help = "Usage: " + std::string(argv[0]) + " detect_model_path feature_model_path";

    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }

    const char *detect_model_path = argv[1];
    const char *feature_model_path = argv[2];
    float conf_threshold = 0.4;
    float iou_threshold = 0.45;

    nn::FaceRecognizer recognizer;

    e = recognizer.load(detect_model_path, feature_model_path);
    err::check_raise(e, "load model failed");
    log::info("open camera now");
    maix::image::Size input_size = recognizer.input_size();
    camera::Camera cam = camera::Camera(input_size.width(), input_size.height(), recognizer.input_format());
    log::info("open camera success");
    display::Display disp = display::Display();
    while (!app::need_exit())
    {
        uint64_t t = time::ticks_ms();
        maix::image::Image *img = cam.read();
        err::check_null_raise(img, "read camera failed");
        std::vector<nn::FaceObject> *result = recognizer.recognize(*img, conf_threshold, iou_threshold);
        for (auto &r : *result)
        {
            img->draw_rect(r.x, r.y, r.w, r.h, maix::image::Color::from_rgb(255, 0, 0));
            snprintf(tmp_chars, sizeof(tmp_chars), "%s:%.2f", recognizer.labels[r.class_id].c_str(), r.score);
            img->draw_string(r.x, r.y, tmp_chars, maix::image::Color::from_rgb(255, 0, 0));
            int radius = ceil(r.w / 10);
            img->draw_keypoints(r.points, image::COLOR_RED, radius > 4 ? 4 : radius);
        }
        disp.show(*img);
        delete result;
        delete img;
        log::info("time: %d ms", time::ticks_ms() - t);
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
