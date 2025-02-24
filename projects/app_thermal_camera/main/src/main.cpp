
#include "global_config.h"
#include "global_build_info_time.h"
#include "global_build_info_version.h"


/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <iostream>
#include <sys/resource.h>

#include "maix_basic.hpp"
#include "maix_display.hpp"
#include "maix_lvgl.hpp"
#include "maix_camera.hpp"
#include "maix_image.hpp"
#include "maix_util.hpp"
#include "lvgl.h"
#include "app.hpp"
#include "maix_i2c.hpp"
#include "maix_camera.hpp"

#include "maix_mlx90640.hpp"
#include "maix_cmap.hpp"

#include "mix.hpp"

using namespace maix;
using namespace maix::image;

using namespace maix::ext_dev::mlx90640;
using namespace maix::peripheral::i2c;
using MLXC = ext_dev::mlx90640::MLX90640Celsius;

enum class DeviceSupport {
    MLX90640,
    UNKNOWN
};

struct BoolResult {
    bool flag;
    DeviceSupport device;
    std::string msg;
};

BoolResult find_device()
{
    std::string suffix = "Support list:\nPMOD_Thermal32\n\nClicked to exit.";
    BoolResult res;

    bool total_flag = false;

    /* find mlx */
    ::system("insmod /mnt/system/ko/i2c-algo-bit.ko");
    ::system("insmod /mnt/system/ko/i2c-gpio.ko");
    {
        auto i2cdev = I2C(5, Mode::MASTER);
        if (!i2cdev.scan(0x33).empty()) {
            total_flag = true;
            res.device = DeviceSupport::MLX90640;
        }
    }

    if (total_flag) {
        res.flag = true;
        return res;
    }

    res.flag = false;
    res.device = DeviceSupport::UNKNOWN;
    res.msg = "Devices Not Found!\n" + suffix;
    return res;
}

int _main(int argc, char **argv)
{

    // init display
    display::Display disp = display::Display(-1, -1, image::FMT_RGB888);
    err::check_bool_raise(disp.is_opened(), "camera open failed");
    display::Display *other_disp = disp.add_channel();  // This object(other_disp) is depend on disp, so we must keep disp.show() running.
    err::check_bool_raise(disp.is_opened(), "display open failed");

    touchscreen::TouchScreen touchscreen = touchscreen::TouchScreen();
    err::check_bool_raise(touchscreen.is_opened(), "touchscreen open failed");

    constexpr int X_SCALE = 2;
    std::unique_ptr<camera::Camera> cam = std::make_unique<camera::Camera>(32*X_SCALE, 24*X_SCALE);

    maix::lvgl_init(other_disp, &touchscreen);

    maix::ext_dev::cmap::Cmap prev_cmap = g_cmap;

    // float temp_min = 5.0f;
    // float temp_max = 60.0f;
    float temp_min = -1;
    float temp_max = -1;
    // float temp_cmap_oft = 2.0f;

    bool _fuf = false;

    auto ret = find_device();

    if (ret.device == DeviceSupport::MLX90640) {
        std::unique_ptr<MLXC> g_mlx_c = nullptr;
        g_mlx_c.reset(new MLXC(5, FPS::FPS_32, prev_cmap, temp_min, temp_max));
        ui_total_init(disp.width(), disp.height());
        point_map_init(32, 24);
        while (!app::need_exit()) {
            if (g_cmap != prev_cmap) {
                prev_cmap = g_cmap;
                g_mlx_c.reset(new MLXC(5, FPS::FPS_32, prev_cmap, temp_min, 50));
            }

            auto matrix = g_mlx_c->matrix();
            if (matrix.empty()) continue;

            std::unique_ptr<Image> img;
            std::vector<uint8_t> mix_data;
            if (g_fusion_mode) {
                if (!_fuf) {
                    g_mlx_c.release();
                    g_mlx_c.reset(new MLXC(5, FPS::FPS_32, prev_cmap, 5.0f, 45.0f));
                    _fuf = true;
                    continue;
                }
                if (cam_need_reset) {
                    println("cam reset!");
                    cam.reset(nullptr);
                    time::sleep(1);
                    cam.reset(new camera::Camera(g_camera_img_info.w*X_SCALE, g_camera_img_info.h*X_SCALE));
                    time::sleep(1);
                    cam_need_reset = false;
                    continue;
                }

                // println("read a cam img");
                std::unique_ptr<Image> cam_img_tmp = std::unique_ptr<Image>(cam->read());
                const auto cam_img_w = (g_camera_img_info.w) < 32 ? 32 : g_camera_img_info.w;
                const auto cam_img_h = (g_camera_img_info.h) < 24 ? 24 : g_camera_img_info.h;
                auto cam_img = std::unique_ptr<Image>(cam_img_tmp->resize(cam_img_w, cam_img_h));

                std::unique_ptr<Image> crop_img;
                if (g_camera_img_info.w != -1 || g_camera_img_info.h != -1) {
                    auto _tmp_img = std::unique_ptr<Image>(cam_img->crop(g_camera_img_info.x1, g_camera_img_info.y1, g_camera_img_info.ww, g_camera_img_info.hh));
                    crop_img.reset(_tmp_img->resize(32, 24));
                }

                // println("mix img");
                // float _t = (std::get<2>(g_mlx_c->max_temp_point()) - std::get<2>(g_mlx_c->min_temp_point())) / 2 + std::get<2>(g_mlx_c->min_temp_point()) + temp_cmap_oft;
                float _tx = 26.0;
                float _tn = 10.0f;
                // auto [_1, _2, _tx] = g_mlx_c->max_temp_point();
                // auto [_3, _4, _tn] = g_mlx_c->min_temp_point();
                // temp_min = _tn;
                // temp_max = _tx;
                mix_data = std::move(mix2(matrix, 45.0f, 5.0f, _tx, _tn, crop_img ? (uint8_t*)crop_img->data() : (uint8_t*)cam_img->data(), 0.5));
                if (mix_data.empty()) continue;
                // println("create img");
                img.reset(new Image(32, 24, FMT_RGB888, mix_data.data(), mix_data.size(), false));
                // img.reset(Image(32, 24, FMT_RGB888, mix_data.data(), mix_data.size(), false).resize(640, 480, FIT_FILL, BILINEAR));
            } else {
                img.reset(g_mlx_c->image_from(matrix));
                _fuf = false;
            }

            // auto img = g_mlx_c->image_from(matrix);
            if (!img) continue;

            /* 32x24 --> 640x480 */
            // std::unique_ptr<Image> reize_img(img->resize(640, 480, FIT_FILL, BICUBIC));
            // println("show img");
            disp.show(*(img.get()));
            // println("update 3crosshair");
            upate_3crosshair_and_label(g_mlx_c->min_temp_point(), g_mlx_c->max_temp_point(), g_mlx_c->center_point());
            // println("update user crosshair");
            update_user_crosshair(matrix);
            // delete img;

            lv_timer_handler();

            // auto fps = time::fps();
            // println("fps: %0.2f", fps);
        }
    } else if (ret.device == DeviceSupport::UNKNOWN) {
        // ui_total_init(disp.width(), disp.height(), false);
        auto img = Image(disp.width(), disp.height());
        img.draw_string(0, 100, ret.msg);
        disp.show(img);
        while (!app::need_exit()) {
            time::sleep_ms(200);
            // lv_timer_handler();
            auto ts = touchscreen.read();
            if (ts.empty()) continue;
            if (ts[2] == 1) break;
        }
    }




    return 0;
}


int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}

