
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_yolov8.hpp"
#include "maix_nn_face_landmarks.hpp"
#include "main.h"

using namespace maix;

int _main(int argc, char *argv[])
{
    log::info("Program start");
    int ret = 0;
    maix::image::Format img_fmt = maix::image::FMT_RGB888;
    char tmp_chars[64] = {0};

    std::string help = "Usage: " + std::string(argv[0]) + " detect_model_path landmarks_model_path <image_path>";

    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }

    const char *detect_model_path = argv[1];
    const char *landmarks_model_path = argv[2];
    float conf_threshold = 0.5;
    float iou_threshold = 0.45;
    float landmarks_conf_th = 0.5;

    nn::YOLOv8 detector(detect_model_path, false);
    log::info("load yolov8 model %s success", detect_model_path);
    nn::FaceLandmarks landmarks_detector(landmarks_model_path);
    log::info("load landmarks model %s success", landmarks_model_path);

    if (argc >= 4)
    {
        const char *img_path = argv[3];
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
            detector.draw_pose(*img, r->points, 4, image::COLOR_RED);
        }
        img->save("result.jpg");
        log::info("saved result image to result.jpg");
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
            nn::Objects *result = detector.detect(*img, conf_threshold, iou_threshold);
            std::vector<nn::FaceLandmarksObject*> results;
            for (auto &r : *result)
            {
                // detector.draw_pose(*img, r->points, 4, image::COLOR_RED);
                // log::info("result: %s", r->to_str().c_str());
                // img->draw_rect(r->x, r->y, r->w, r->h, maix::image::Color::from_rgb(255, 0, 0));
                maix::image::Image *img_std = landmarks_detector.crop_image(*img, r->x, r->y, r->w, r->h, r->points);
                if(img_std)
                {
                    nn::FaceLandmarksObject *res = landmarks_detector.detect(*img_std, landmarks_conf_th, true, false);
                    if(res && res->valid)
                    {
                        results.push_back(res);
                        // snprintf(tmp_chars, sizeof(tmp_chars), "%.1f", res->score);
                        // img->draw_string(r->x, r->y, , maix::image::Color::from_rgb(255, 0, 0));
                    }
                    else if(res)
                    {
                        delete res;
                    }

                    // for debug, draw std image on img.
                    // maix::image::Image *resized = img_std->resize(128, 128);
                    // img->draw_image(0, 0, *resized);
                    // delete resized;

                    delete img_std;
                }
            }
            uint64_t t3 = time::ticks_ms();
            for (auto &r : results)
            {
                landmarks_detector.draw_face(*img, r->points, landmarks_detector.landmarks_num, r->points_z);
                delete r;
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
