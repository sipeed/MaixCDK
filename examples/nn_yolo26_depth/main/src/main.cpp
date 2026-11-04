/**
 * YOLO26-depth example
 *
 * Usage:
 *   - 单张图片推理并保存热力图(可选传 cal_a cal_b 校准):
 *       ./nn_yolo26_depth mud_model_path image_path [output_path] [cal_a cal_b]
 *       例: ./nn_yolo26_depth /tmp/yolo26n-depth.mud /tmp/bus.jpg /tmp/bus_heatmap.jpg 0.85 0.40
 *   - 实时摄像头推理并显示热力图(可选传 cal_a cal_b):
 *       ./nn_yolo26_depth mud_model_path [cal_a cal_b]
 *       点击画面可添加/取消距离探针，最多保留 5 个，距离随画面实时更新。
 *
 * 依赖(设备上需存在):
 *   /tmp/yolo26n-depth.mud
 *   /tmp/yolo26n-depth_w8a8_mix.axmodel
 *
 * 退出: 摄像头模式下发送 SIGINT/SIGTERM(kill -INT <pid>) 或设备退出信号。
 */

#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "maix_image.hpp"
#include "maix_nn_yolo26_depth.hpp"
#include "maix_time.hpp"
#include "maix_touchscreen.hpp"
#include "main.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

using namespace maix;

namespace
{
    constexpr size_t MAX_PROBES = 5;
    constexpr int PROBE_HIT_RADIUS = 20;
    constexpr int PROBE_HIT_RADIUS_SQUARED = PROBE_HIT_RADIUS * PROBE_HIT_RADIUS;

    struct Probe
    {
        int x;
        int y;
    };

    struct DisplayTransform
    {
        float scale;
        int offset_x;
        int offset_y;
        int content_width;
        int content_height;
    };

    DisplayTransform get_display_transform(int image_width, int image_height,
                                           int display_width, int display_height)
    {
        float scale = std::min((float)display_width / image_width,
                               (float)display_height / image_height);
        int content_width = (int)std::round(image_width * scale);
        int content_height = (int)std::round(image_height * scale);
        return {scale,
                (display_width - content_width) / 2,
                (display_height - content_height) / 2,
                content_width,
                content_height};
    }

    bool screen_to_image(const DisplayTransform &transform, int screen_x, int screen_y,
                         int image_width, int image_height, Probe &point)
    {
        if (screen_x < transform.offset_x || screen_y < transform.offset_y ||
            screen_x >= transform.offset_x + transform.content_width ||
            screen_y >= transform.offset_y + transform.content_height)
            return false;

        point.x = std::clamp((int)((screen_x - transform.offset_x) / transform.scale),
                             0, image_width - 1);
        point.y = std::clamp((int)((screen_y - transform.offset_y) / transform.scale),
                             0, image_height - 1);
        return true;
    }

    Probe image_to_screen(const DisplayTransform &transform, const Probe &point)
    {
        return {transform.offset_x + (int)(point.x * transform.scale),
                transform.offset_y + (int)(point.y * transform.scale)};
    }

    void update_probes(std::vector<Probe> &probes, int screen_x, int screen_y,
                       int image_width, int image_height,
                       int display_width, int display_height)
    {
        DisplayTransform transform = get_display_transform(image_width, image_height,
                                                           display_width, display_height);
        Probe new_probe;
        if (!screen_to_image(transform, screen_x, screen_y, image_width, image_height, new_probe))
            return;

        size_t nearest = probes.size();
        int nearest_distance = PROBE_HIT_RADIUS_SQUARED + 1;
        for (size_t i = 0; i < probes.size(); ++i)
        {
            Probe screen_probe = image_to_screen(transform, probes[i]);
            int dx = screen_x - screen_probe.x;
            int dy = screen_y - screen_probe.y;
            int distance = dx * dx + dy * dy;
            if (distance <= PROBE_HIT_RADIUS_SQUARED && distance < nearest_distance)
            {
                nearest = i;
                nearest_distance = distance;
            }
        }

        if (nearest < probes.size())
        {
            probes.erase(probes.begin() + nearest);
            return;
        }

        if (probes.size() == MAX_PROBES)
            probes.erase(probes.begin());
        probes.push_back(new_probe);
    }

    void draw_probe(image::Image &heatmap, const Probe &probe, float distance)
    {
        const image::Color marker_color = image::Color::from_rgb(255, 255, 255);
        heatmap.draw_circle(probe.x, probe.y, 8, image::COLOR_BLACK, -1);
        heatmap.draw_circle(probe.x, probe.y, 5, marker_color, -1);

        char label[32];
        if (std::isfinite(distance))
            std::snprintf(label, sizeof(label), "%.2f m", distance);
        else
            std::snprintf(label, sizeof(label), "N/A");

        image::Size text_size = image::string_size(label);
        int label_x = probe.x + 10;
        int label_y = probe.y - text_size.height() - 8;
        if (label_y < 2)
            label_y = probe.y + 10;
        label_x = std::clamp(label_x, 2, std::max(2, heatmap.width() - text_size.width() - 2));
        label_y = std::clamp(label_y, 2, std::max(2, heatmap.height() - text_size.height() - 2));
        heatmap.draw_rect(label_x - 2, label_y - 2,
                          text_size.width() + 4, text_size.height() + 4,
                          image::COLOR_BLACK, -1);
        heatmap.draw_string(label_x, label_y, label, marker_color);
    }
}

int _main(int argc, char *argv[])
{
    int ret = 0;
    log::info("Program start");
    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path [image_path [output_path]]";

    if (argc < 2)
    {
        log::error(help.c_str());
        return -1;
    }

    const char *model_path = argv[1];
    bool dual_buff = false;
    image::CMap cmap = image::CMap::JET;
    // 可选深度校准参数(默认不校准): d_real = exp(cal_a * log(d_raw) + cal_b)
    float cal_a = 1.0f;
    float cal_b = 0.0f;
    if (argc >= 6)
    {
        cal_a = std::stof(argv[4]);
        cal_b = std::stof(argv[5]);
        log::info("depth calibration enabled: cal_a=%f cal_b=%f", cal_a, cal_b);
    }

    log::info("model path: %s", model_path);
    nn::YOLO26Depth model(model_path, dual_buff);
    log::info("load model %s success", model_path);
    log::info("model input size: %dx%d, format: %d",
              model.input_width(), model.input_height(), (int)model.input_format());
    log::info("dual buff mode: %d", dual_buff);

    if (argc >= 3)
    {
        // ---------- 模式一: 单张图片推理, 保存热力图 ----------
        const char *img_path = argv[2];
        std::string output_path = (argc >= 4) ? argv[3] : "depth_heatmap.jpg";
        log::info("load image now");
        maix::image::Image *img = maix::image::load(img_path, model.input_format());
        err::check_null_raise(img, "load image " + std::string(img_path) + " failed");
        log::info("load image %s success: %s", img_path, img->to_str().c_str());
        if (img->width() != model.input_width() || img->height() != model.input_height())
        {
            log::warn("image size not match model input size, will auto resize from %dx%d to %dx%d",
                      img->width(), img->height(), model.input_width(), model.input_height());
        }

        // 推理得到深度热力图(自动 resize 回原图尺寸; 可传 cal_a/cal_b 校准)
        maix::image::Image *heatmap = model.get_depth_image(*img, image::FIT_CONTAIN, cmap, cal_a, cal_b);
        err::check_null_raise(heatmap, "get_depth_image failed");

        // 保存热力图
        err::Err e = heatmap->save(output_path.c_str());
        if (e != err::ERR_NONE)
        {
            log::error("save heatmap to %s failed", output_path.c_str());
            ret = -1;
        }
        else
        {
            log::info("save heatmap to %s success", output_path.c_str());
        }

        delete heatmap;
        delete img;
    }
    else
    {
        // ---------- 模式二: 实时摄像头推理并显示热力图 ----------
        camera::Camera cam = camera::Camera(model.input_width(), model.input_height(), model.input_format());
        display::Display disp = display::Display();
        touchscreen::TouchScreen touch = touchscreen::TouchScreen();
        log::info("camera %dx%d", cam.width(), cam.height());

        uint64_t frame_id = 0;
        uint64_t t_last = time::ticks_ms();
        int fps = 0;
        bool was_pressed = false;
        std::vector<Probe> probes;

        while (!app::need_exit())
        {
            uint64_t t0 = time::ticks_ms();
            std::unique_ptr<image::Image> img(cam.read());
            if (!img)
            {
                time::sleep_ms(10);
                continue;
            }

            int touch_x = 0;
            int touch_y = 0;
            bool pressed = false;
            err::Err touch_err = touch.read(touch_x, touch_y, pressed);
            if (touch_err == err::ERR_NONE)
            {
                if (was_pressed && !pressed)
                {
                    update_probes(probes, touch_x, touch_y,
                                  img->width(), img->height(), disp.width(), disp.height());
                }
                was_pressed = pressed;
            }

            std::unique_ptr<tensor::Tensor> depth(model.get_depth(*img, image::FIT_CONTAIN, cal_a, cal_b));
            if (!depth)
                continue;

            std::unique_ptr<image::Image> heatmap(
                model.depth_to_image(*depth, img->width(), img->height(), image::FIT_CONTAIN, cmap));
            if (heatmap)
            {
                for (const Probe &probe : probes)
                {
                    float distance = model.get_distance(*depth, probe.x, probe.y,
                                                        img->width(), img->height(), image::FIT_CONTAIN);
                    draw_probe(*heatmap, probe, distance);
                }
                disp.show(*heatmap);
            }

            frame_id++;
            if (frame_id % 20 == 0)
            {
                uint64_t now = time::ticks_ms();
                fps = (int)(20000.0f / (now - t_last));
                t_last = now;
                log::info("frame=%llu e2e=%llu ms fps=%d", (unsigned long long)frame_id,
                          (unsigned long long)(time::ticks_ms() - t0), fps);
            }
        }
    }

    log::info("Program exit");
    return ret;
}

int main(int argc, char *argv[])
{
    sys::register_default_signal_handle();
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
