
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

#include "maix_cmap.hpp"
#include "maix_tof100.hpp"
#include "mix.hpp"

using namespace maix;
using namespace maix::image;

using namespace maix::ext_dev::tof100;

void point_map_init_with_res(Resolution res)
{
    switch (res) {
    case Resolution::RES_100x100:
        println("remap -> (100, 100)");
        point_map_init(100, 100);
        break;
    case Resolution::RES_25x25:
        [[fallthrough]];
    case Resolution::RES_50x50:
        println("remap -> (50, 50)");
        point_map_init(50, 50);
        break;
    default: break;
    }
}

void show_error(display::Display& disp, touchscreen::TouchScreen& ts)
{
    ui_total_deinit();
    auto img = Image(disp.width(), disp.height());
    std::string msg = "Devices Not Found!\nSupport list:\nPMOD_TOF100\n\nClicked to exit.";
    img.draw_string(0, 100, msg);
    disp.show(img);
    while (!app::need_exit()) {
        time::sleep_ms(200);
        auto tsd = ts.read();
        if (tsd.empty()) continue;
        if (tsd[2] == 1) break;
    }
}

int _main(int argc, char **argv)
{
    display::Display disp = display::Display(-1, -1, image::FMT_RGB888);
    err::check_bool_raise(disp.is_opened(), "camera open failed");
    display::Display *other_disp = disp.add_channel();
    err::check_bool_raise(disp.is_opened(), "display open failed");
    touchscreen::TouchScreen touchscreen = touchscreen::TouchScreen();
    err::check_bool_raise(touchscreen.is_opened(), "touchscreen open failed");
    maix::lvgl_init(other_disp, &touchscreen);

    maix::ext_dev::cmap::Cmap prev_cmap = g_cmap;
    Resolution prev_res = g_res;

    float aspect_ratio = static_cast<float>(disp.width()) / disp.height();

    std::unique_ptr<Tof100> opns;
    int OPNS_MIN_DIS_MM = 40; // 4cm
    int OPNS_MAX_DIS_MM = 750; // 100cm

    try {
        /* try to init opns */
        opns.reset(new Tof100(4, prev_res, prev_cmap, OPNS_MIN_DIS_MM, OPNS_MAX_DIS_MM));
    } catch (...) {
        eprintln("Device Not Found!");
        show_error(disp, touchscreen);
    }

    // opns.reset(new Tof100(4, prev_res, prev_cmap, OPNS_MIN_DIS_MM, OPNS_MAX_DIS_MM));
    ui_total_init(disp.width(), disp.height());
    point_map_init_with_res(prev_res);
    std::unique_ptr<camera::Camera> cam = std::make_unique<camera::Camera>(cam_onfo_o.w, cam_onfo_o.h);
    while (!app::need_exit()) {
        if (g_cmap != prev_cmap || g_res != prev_res) {
            prev_cmap = g_cmap;
            prev_res = g_res;
            point_map_init_with_res(prev_res);
            opns.reset(new Tof100(4, prev_res, prev_cmap, 40, 1000));
        }

        auto matrix = opns->matrix();
        if (matrix.empty()) {
            continue;
        }

        std::unique_ptr<Image> img;
        std::vector<uint8_t> mix_data;

        if (g_fusion_mode) {
            if (cam_need_reset) {
                // println("cam need reset --> %dx%d", g_camera_img_info.w, g_camera_img_info.h);
                cam.reset(nullptr);
                time::sleep(1);
                cam.reset(new camera::Camera(g_camera_img_info.w, g_camera_img_info.h));
                // cam.reset(new camera::Camera(500, 320));
                time::sleep(1);
                cam_need_reset = false;
                println("cam rest finish, %dx%d", cam->width(), cam->height());
                continue;
            }

            std::unique_ptr<Image> cam_img = std::unique_ptr<Image>(cam->read());
            std::unique_ptr<Image> crop_image;
            if (g_camera_img_info.w != -1 || g_camera_img_info.h != -1) {
                auto _tmp_img = std::unique_ptr<Image>(cam_img->crop(g_camera_img_info.x1, g_camera_img_info.y1, g_camera_img_info.ww, g_camera_img_info.hh));
                crop_image.reset(_tmp_img->resize(50, 50));
            }

            int _dx = 3400;
            int _dn = 100;
            mix_data = std::move(mix2<uint32_t>(matrix, OPNS_MAX_DIS_MM, OPNS_MIN_DIS_MM, _dx, _dn, crop_image ? (uint8_t*)crop_image->data() : (uint8_t*)cam_img->data(), 0.5f, true));
            if (mix_data.empty()) continue;
            img.reset(new Image(50, 50, FMT_RGB888, mix_data.data(), mix_data.size(), false));
        } else {
            img.reset(opns->image_from(matrix));
        }

        // auto img = opns->image_from(matrix);
        if (img.get() == nullptr) continue;

        int __new_w = static_cast<int>(img->width()*aspect_ratio);
        __new_w += __new_w%2;
        auto show_img = Image(__new_w, img->height(), FMT_RGB888);
        // println("iw:%d, ih:%d, sw:%d, sh:%d", img->width(), img->height(), show_img.width(), show_img.height());
        int img_draw_x_oft = (show_img.width()-show_img.height()) / 2;
        show_img.draw_image(img_draw_x_oft, 0, *img);

        disp.show(show_img);
        upate_3crosshair_and_label(opns->min_dis_point(), opns->max_dis_point(), opns->center_point());
        update_user_crosshair(matrix);

        lv_timer_handler();

        // auto fps = time::fps();
        // println("fps: %0.2f", fps);
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

