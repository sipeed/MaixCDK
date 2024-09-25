
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_pp_ocr.hpp"
#include "main.h"

using namespace maix;

int _main(int argc, char *argv[])
{
    log::info("Program start");
    std::string model_type = "unknown";
    int ret = 0;
    std::vector<float> mean = {};
    std::vector<float> scale = {};

    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path <image_path>";

    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }

    const char *model_path = argv[1];
    float conf_threshold = 0.5;
    float iou_threshold = 0.45;

    nn::PP_OCR ocr(model_path);
    log::info("load PP OCR model %s success", model_path);

    maix::image::Format img_fmt = ocr.input_format();
    display::Display disp = display::Display();

    int font_size = 20;
    image::load_font("ppocr", "/maixapp/share/font/ppocr_keys_v1.ttf", font_size);
    image::set_default_font("ppocr");

    if (argc >= 3)
    {
        const char *img_path = argv[2];
        log::info("load image now");
        maix::image::Image *img = maix::image::load(img_path, img_fmt);
        err::check_null_raise(img, "load image " + std::string(img_path) + " failed");
        log::info("load image %s success: %s", img_path, img->to_str().c_str());
        if (img->width() != ocr.input_size().width() || img->height() != ocr.input_size().height())
        {
            log::warn("image size not match model input size, will auto resize from %dx%d to %dx%d", img->width(), img->height(), ocr.input_size().width(), ocr.input_size().height());
        }
        log::info("detect now");
        if(ocr.det)
        {
            nn::OCR_Objects *result = ocr.detect(*img, conf_threshold, iou_threshold);
            if(result->size() == 0)
            {
                log::info("no object detected !");
            }
            for (auto &r : *result)
            {

                log::info("result: %s", r->to_str().c_str());
                std::vector<int> points = r->box.to_list();
                img->draw_keypoints(points, image::COLOR_RED, 4, -1, 1);
                img->draw_string(r->box.x4, r->box.y4, r->char_str(), image::COLOR_RED);
            }
            delete result;
        }
        else
        {
            nn::OCR_Object *result = ocr.recognize(*img);
            img->draw_string(10, 10, result->char_str(), image::COLOR_RED);
            log::info("result: %s", result->to_str().c_str());
            delete result;
        }
        img->save("result.jpg");
        disp.show(*img);
        delete img;
        log::info("input any words to exit");
        char tmp[10];
        fgets(tmp, sizeof(tmp), stdin);
    }
    else
    {
        log::info("open camera now");
        maix::image::Size input_size = ocr.input_size();
        camera::Camera cam = camera::Camera(input_size.width(), input_size.height(), ocr.input_format());
        log::info("open camera success");
        while (!app::need_exit())
        {
            uint64_t t = time::ticks_ms();
            maix::image::Image *img = cam.read();
            err::check_null_raise(img, "read camera failed");
            uint64_t t2 = time::ticks_ms();
            nn::OCR_Objects *result = ocr.detect(*img);
            uint64_t t3 = time::ticks_ms();
            for (auto &r : *result)
            {
                // log::info("result: %s", r->to_str().c_str());
                std::vector<int> points = r->box.to_list();
                img->draw_keypoints(points, image::COLOR_RED, 4, -1, 1);
                img->draw_string(r->box.x4, r->box.y4, r->char_str(), image::COLOR_RED);
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
