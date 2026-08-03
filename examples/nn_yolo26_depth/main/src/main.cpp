/**
 * YOLO26-depth example
 *
 * Usage:
 *   - 单张图片推理并保存热力图:
 *       ./nn_yolo26_depth mud_model_path image_path [output_path]
 *       例: ./nn_yolo26_depth /tmp/yolo26n-depth.mud /tmp/bus.jpg /tmp/bus_heatmap.jpg
 *   - 实时摄像头推理并显示热力图:
 *       ./nn_yolo26_depth mud_model_path
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
#include "main.h"

using namespace maix;

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

    log::info("model path: %s", model_path);
    nn::Yolo26Depth model(model_path, dual_buff);
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

        // 推理得到深度热力图(自动 resize 回原图尺寸)
        maix::image::Image *heatmap = model.get_depth_image(*img, image::FIT_CONTAIN, cmap);
        err::check_null_raise(heatmap, "get_depth_image failed");

        // 保存热力图
        err::Err e = heatmap->save(output_path);
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
        log::info("camera %dx%d", cam.width(), cam.height());

        uint64_t frame_id = 0;
        uint64_t t_last = time::ticks_ms();
        int fps = 0;

        while (!app::need_exit())
        {
            uint64_t t0 = time::ticks_ms();
            image::Image *img = cam.read();
            if (!img)
            {
                time::sleep_ms(10);
                continue;
            }

            image::Image *heatmap = model.get_depth_image(*img, image::FIT_CONTAIN, cmap);
            if (heatmap)
            {
                disp.show(*heatmap);
                delete heatmap;
            }
            delete img;

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
