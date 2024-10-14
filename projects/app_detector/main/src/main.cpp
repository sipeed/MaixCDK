
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_yolov5.hpp"
#include "main.h"
#include "maix_touchscreen.hpp"

using namespace maix;

bool is_in_button(int x, int y, std::vector<int> btn_pos)
{
    return x > btn_pos[0] && x < btn_pos[0] + btn_pos[2] && y > btn_pos[1] && y < btn_pos[1] + btn_pos[3];
}


int _main(int argc, char *argv[])
{
    log::info("Program start");
    std::string model_type = "unknown";
    int ret = 0;
    err::Err e;
    std::vector<float> mean = {};
    std::vector<float> scale = {};
    char tmp_chars[128] = {0};

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
    uint64_t t, t2, t3, t_show, t_all = 0;
    int exit_btn_pos[4] = {0, 0, ret_img->width(), ret_img->height()};
    std::vector<int> exit_btn_disp_pos = maix::image::resize_map_pos(cam.width(), cam.height(), disp.width(), disp.height(), image::FIT_CONTAIN, exit_btn_pos[0], exit_btn_pos[1], exit_btn_pos[2], exit_btn_pos[3]);

    while (!app::need_exit())
    {
        ts.read(ts_x, ts_y, ts_pressed);
        if (ts_pressed && is_in_button(ts_x, ts_y, exit_btn_disp_pos))
        {
            break;
        }
        t = time::ticks_ms();
        maix::image::Image *img = cam.read();
        err::check_null_raise(img, "read camera failed");
        t2 = time::ticks_ms();
        std::vector<nn::Object> *result = detector.detect(*img, conf_threshold, iou_threshold);
        t3 = time::ticks_ms();
        for (auto &r : *result)
        {
            img->draw_rect(r.x, r.y, r.w, r.h, maix::image::Color::from_rgb(255, 0, 0));
            img->draw_string(r.x, r.y, detector.labels[r.class_id], maix::image::Color::from_rgb(255, 0, 0));
        }
        img->draw_image(0, 0, *ret_img);
        snprintf(tmp_chars, sizeof(tmp_chars), "All: %ldms, fps: %ld\ncam: %ldms, detect: %ldms, show: %ldms", t_all, 1000 / t_all, t2 - t, t3 - t2, t_show);
        img->draw_string(2, img->height() - 40, tmp_chars, image::COLOR_RED);
        disp.show(*img);
        t_show = time::ticks_ms() - t3;
        t_all = time::ticks_ms() - t;
        delete result;
        delete img;
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
