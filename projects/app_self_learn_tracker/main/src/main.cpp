
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_touchscreen.hpp"
#include "maix_nn_nanotrack.hpp"
#include "main.h"

using namespace maix;

int _main(int argc, char *argv[])
{
    log::info("Program start");
    std::string model_type = "unknown";
    int ret = 0;
    char tmp_char[64];


    const char *model_path = "/root/models/nanotrack.mud";

    nn::NanoTrack tracker(model_path);
    log::info("load NanoTrack model %s success", model_path);

    log::info("open camera now");
    display::Display disp = display::Display();
    touchscreen::TouchScreen touch = touchscreen::TouchScreen();
    camera::Camera cam = camera::Camera(disp.width(), disp.height(), tracker.input_format());
    log::info("open camera success");
    int status = 0; // 0: select target box, 1: tracking
    bool pressing = false;
    nn::Object target;
    const char *btn_str = "Select";
    auto font_size = image::string_size(btn_str);
    image::Image *ret_img = image::load("/maixapp/share/icon/ret.png", image::Format::FMT_RGB888);
    if (!ret_img)
    {
        log::error("load ret image failed");
        return -12345;
    }

    uint64_t t3;
    while (!app::need_exit())
    {
        uint64_t t = time::ticks_ms();
        maix::image::Image *img = cam.read();
        err::check_null_raise(img, "read camera failed");
        uint64_t t2 = time::ticks_ms();
        auto touch_status = touch.read();
        if (status == 0)
        {
            if(touch_status[2])
            {
                if(!pressing)
                {
                    target.x = touch_status[0];
                    target.y = touch_status[1];
                    log::info("start select");
                }
                pressing = true;
            }
            else
            {
                if(pressing) // click
                {
                    target.w = touch_status[0] - target.x;
                    target.h = touch_status[1] - target.y;
                    if(target.w > 0 && target.h > 0)
                    {
                        log::info("init tracker with rectangle x: %d, y: %d, w: %d, h: %d", target.x, target.y, target.w, target.h);
                        tracker.init(*img, target.x, target.y, target.w, target.h);
                        log::info("init tracker ok");
                        status = 1;
                    }
                    else
                        log::info("rectangle invalid, x: %d, y: %d, w: %d, h: %d", target.x, target.y, target.w, target.h);
                }
                pressing = false;
            }
            if(pressing)
            {
                img->draw_string(2, img->height() - font_size.height() * 2, "Select and release to complete", maix::image::Color::from_rgb(255, 0, 0), 1.5);
                img->draw_rect(target.x, target.y, touch_status[0] - target.x, touch_status[1] - target.y, maix::image::Color::from_rgb(255, 0, 0), 3);
            }
            else
                img->draw_string(2, img->height() - font_size.height() * 2, "Select target on screen", maix::image::Color::from_rgb(255, 0, 0), 1.5);
            t3 = time::ticks_ms();
        }
        else
        {
            if(touch_status[2])
            {
                pressing = true;
            }
            else
            {
                if(pressing) // click
                {
                    if(touch_status[0] >= disp.width() - 80 && touch_status[1] >= disp.height() - 60)
                    {
                        status = 0;
                    }
                }
                pressing = false;
            }
            nn::Object r = tracker.track(*img);
            t3 = time::ticks_ms();
            // log::info("result: %s", r.to_str().c_str());
            img->draw_rect(r.x, r.y, r.w, r.h, maix::image::Color::from_rgb(255, 0, 0), 4);
            img->draw_rect(r.points[0], r.points[1], r.points[2], r.points[3], maix::image::Color::from_rgb(158, 158, 158), 1); // input area
            img->draw_rect(r.points[4] - r.points[7] / 2, r.points[5] - r.points[7] / 2, r.points[7], r.points[7], maix::image::Color::from_rgb(158, 158, 158), 1); // target size
            snprintf(tmp_char, sizeof(tmp_char), "%.2f", r.score);
            img->draw_string(r.x, r.y - font_size.height() - 2, tmp_char, maix::image::Color::from_rgb(255, 0, 0), 1.5);
            img->draw_rect(disp.width() - 100, disp.height() - 60, 100, 60, maix::image::Color::from_rgb(255, 255, 255), 4);
            img->draw_string(disp.width() - 100 + (100 - font_size.width()) / 2, disp.height() - 60 + (60 - font_size.height()) / 2, btn_str, maix::image::Color::from_rgb(255, 255, 255), 1);
        }
        if(touch_status[2] && touch_status[0] < ret_img->width() && touch_status[1] < ret_img->height())
        {
            app::set_exit_flag(true);
        }
        img->draw_image(0, 0, *ret_img);
        disp.show(*img);
        delete img;
        // log::info("time: all %d ms, track %d ms", time::ticks_ms() - t, t3 - t2);
    }

    delete ret_img;
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
