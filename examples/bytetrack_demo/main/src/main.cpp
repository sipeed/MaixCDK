
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_yolov8.hpp"
#include "main.h"
#include "maix_bytetrack.hpp"

using namespace maix;

std::vector<std::vector<int>> colors = {
    {255, 0, 0},       // 红色
    {0, 255, 0},       // 绿色
    {0, 0, 255},       // 蓝色
    {255, 255, 0},     // 黄色
    {0, 255, 255},     // 青色
    {255, 0, 255},     // 品红色
    {128, 0, 0},       // 深红色
    {0, 128, 0},       // 深绿色
    {0, 0, 128},       // 深蓝色
    {128, 128, 0},     // 橄榄色
    {0, 128, 128},     // 深青色
    {128, 0, 128},     // 紫色
    {255, 128, 0},     // 橙色
    {255, 0, 128},     // 玫瑰色
    {128, 255, 0},     // 黄绿色
    {0, 255, 128},     // 蓝绿色
    {128, 0, 255},     // 紫罗兰色
    {0, 128, 255},     // 天蓝色
    {255, 128, 128},   // 浅红色
    {128, 255, 128},   // 浅绿色
    {128, 128, 255},   // 浅蓝色
    {255, 255, 128},   // 浅黄色
    {128, 255, 255},   // 浅青色
    {255, 128, 255},   // 浅品红色
    {64, 0, 0},        // 暗红色
    {0, 64, 0},        // 暗绿色
    {0, 0, 64},        // 暗蓝色
    {64, 64, 0},       // 暗橄榄色
    {0, 64, 64},       // 暗青色
    {64, 0, 64},       // 暗紫色
    {255, 64, 0},      // 暗橙色
    {255, 0, 64},      // 暗玫瑰色
    {64, 255, 0},      // 暗黄绿色
    {0, 255, 64},      // 暗蓝绿色
    {64, 0, 255},      // 暗紫罗兰色
    {0, 64, 255},      // 暗天蓝色
    {255, 64, 64},     // 暗浅红色
    {64, 255, 64},     // 暗浅绿色
    {64, 64, 255},     // 暗浅蓝色
    {255, 255, 64},    // 暗浅黄色
    {64, 255, 255},    // 暗浅青色
    {255, 64, 255},    // 暗浅品红色
    {192, 192, 192},   // 银色
    {128, 128, 128},   // 灰色
    {64, 64, 64},      // 深灰色
    {255, 255, 255}    // 白色
};

int _main(int argc, char *argv[])
{
    log::info("Program start");
    std::string model_type = "unknown";
    int ret = 0;
    err::Err e;
    maix::image::Format img_fmt = maix::image::FMT_RGB888;
    char tmp_chars[64] = {0};

    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path";

    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }

    const char *model_path = argv[1];
    float conf_threshold = 0.3;
    float iou_threshold = 0.45;
    bool dual_buff = true;
    int max_lost_buff_time = 120;
    float track_thresh = 0.4;
    float high_thresh = 0.6;
    float match_thresh = 0.8;
    int max_history_num = 40;
    bool show_detect = false;
    bool only_first_class = true;


    nn::YOLOv8 detector("", dual_buff);
    e = detector.load(model_path);
    err::check_raise(e, "load model failed");
    log::info("load yolov8 model %s success", model_path);

    log::info("open camera now");
    maix::image::Size input_size = detector.input_size();
    camera::Camera cam = camera::Camera(input_size.width(), input_size.height(), detector.input_format());
    log::info("open camera success");
    display::Display disp = display::Display();
    int font_h = image::string_size("aA123,.!").height();

    // tracker
    tracker::ByteTracker tracker(max_lost_buff_time, track_thresh, high_thresh, match_thresh, max_history_num);

    while (!app::need_exit())
    {
        // capture image from camera
        uint64_t t = time::ticks_ms();
        maix::image::Image *img = cam.read();
        err::check_null_raise(img, "read camera failed");
        uint64_t t2 = time::ticks_ms();

        // detect objects
        nn::Objects *result = detector.detect(*img, conf_threshold, iou_threshold);
        uint64_t t3 = time::ticks_ms();

        // prepare args
        std::vector<tracker::Object> objs;
        for (const auto &r : *result)
        {
            if(only_first_class && r->class_id > 0)
                continue;
            if(show_detect)
            {
                img->draw_rect(r->x, r->y, r->w, r->h, maix::image::COLOR_YELLOW);
                snprintf(tmp_chars, sizeof(tmp_chars), "%s: %.2f", detector.labels[r->class_id].c_str(), r->score);
                img->draw_string(r->x, r->y + r->h - font_h, tmp_chars, maix::image::COLOR_YELLOW);
            }
            // track
            objs.push_back(tracker::Object(r->x, r->y, r->w, r->h, r->class_id, r->score));
        }

        // track
        const auto tracks = tracker.update(objs);

        // draw results
        for (const auto &track : tracks)
        {
            if(track.lost)
                continue;
            std::vector<int> &rgb = colors[track.id % colors.size()];
            image::Color color = image::Color::from_rgb(rgb[0], rgb[1], rgb[2]);

            const auto &obj = track.history.back();
            const auto &points = track.history; // old to new
            img->draw_rect(obj.x, obj.y, obj.w, obj.h, color, 2);
            snprintf(tmp_chars, sizeof(tmp_chars), "%ld %.1f", track.id, track.score);
            for(const auto &i : points)
            {
                img->draw_circle(i.x + i.w / 2 , i.y + i.h / 2, 1, color, -1);
            }
            img->draw_string(obj.x, obj.y, tmp_chars, color, 1.3);
        }
        disp.show(*img);
        delete result;
        delete img;
        log::info("time: all %d ms, detect %d ms", time::ticks_ms() - t, t3 - t2);
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
