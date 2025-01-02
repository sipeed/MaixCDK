
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_hand_landmarks.hpp"
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
    float conf_threshold = 0.7;
    float iou_threshold = 0.45;
    float conf_threshold2 = 0.8;
    bool landmarks_rel = true; // draw relative landmarks on image

    nn::HandLandmarks detector("");
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
        log::info("detect now");
        nn::Objects *result = detector.detect(*img, conf_threshold, iou_threshold, conf_threshold2, landmarks_rel);
        if (result->size() == 0)
        {
            log::info("no object detected !");
        }
        for (auto &r : *result)
        {
            // points:  box_topleft_x, box_topleft_y, box_topright_x, box_topright_y, box_bottomright_x, box_bottomright_y， box_bottomleft_x, box_bottomleft_y,
            //                       x0, y0, z1, x1, y1, z2, ..., x20, y20, z20,[x0_rel, y0_rel,...,x20_rel, y20_rel]
            log::info("result: %s", r->to_str().c_str());
            snprintf(tmp_chars, sizeof(tmp_chars), "%s: %.2f", r->points[0] == 0 ? "left" : "right", r->score);
            img->draw_string(r->points[0], r->points[1], detector.labels[r->class_id],
                                 r->class_id == 0 ? maix::image::Color::from_rgb(255, 0, 0) : maix::image::Color::from_rgb(0, 255, 0),
                                 1.2, 2);
            detector.draw_hand(*img, r->class_id, r->points, 4, 10, true);
        }
        img->save("result.jpg");
        delete result;
        delete img;
    }
    else
    {
        log::info("open camera now");
        camera::Camera cam = camera::Camera(320, 320, detector.input_format());
        log::info("open camera success");
        display::Display disp = display::Display();
        while (!app::need_exit())
        {
            uint64_t t = time::ticks_ms();
            maix::image::Image *img = cam.read();
            err::check_null_raise(img, "read camera failed");
            uint64_t t2 = time::ticks_ms();
            nn::Objects *result = detector.detect(*img, conf_threshold, iou_threshold, conf_threshold2, landmarks_rel);
            uint64_t t3 = time::ticks_ms();
            for (auto &r : *result)
            {
                // points:  box_topleft_x, box_topleft_y, box_topright_x, box_topright_y, box_bottomright_x, box_bottomright_y， box_bottomleft_x, box_bottomleft_y,
                //                       x0, y0, z1, x1, y1, z2, ..., x20, y20, z20,[x0_rel, y0_rel,...,x20_rel, y20_rel]
                log::info("result: %s", r->to_str().c_str());
                snprintf(tmp_chars, sizeof(tmp_chars), "%s: %.2f", r->points[0] == 0 ? "left" : "right", r->score);
                img->draw_string(r->points[0], r->points[1], detector.labels[r->class_id],
                                 r->class_id == 0 ? maix::image::Color::from_rgb(255, 0, 0) : maix::image::Color::from_rgb(0, 255, 0),
                                 1.4, 2);
                // img->draw_rect(r->x, r->y, r->w, r->h, image::COLOR_RED, 1);
                detector.draw_hand(*img, r->class_id, r->points, 4, 10, true);
                if(landmarks_rel)
                {
                    img->draw_rect(0, 0, detector.input_width(false), detector.input_height(false), image::COLOR_YELLOW, 1);
                    for(int i=0; i<21; i++)
                    {
                        img->draw_circle(r->points[8+21*3 + i*2], r->points[9+21*3 + i*2], 2, image::COLOR_YELLOW, -1);
                    }
                }
            }
            disp.show(*img);
            delete result;
            delete img;
            log::info("time: all %d ms, detect %d ms", time::ticks_ms() - t, t3 - t2);
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
