#include "lvgl.h"
#include "app.hpp"
#include "stdio.h"
#include "unistd.h"
#include "maix_basic.hpp"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_comm.hpp"
#include "maix_app.hpp"
#include "stdio.h"

using namespace maix;

#define APP_CMD_REPORT_FIND_BLOBS   8
#define BUFF_RX_LEN 1024

typedef struct {
    std::vector<std::vector<int>> thresholds;
    bool show_binary;
    comm::CommProtocol *ptl;
    int user_lmin;
    int user_lmax;
    int user_amin;
    int user_amax;
    int user_bmin;
    int user_bmax;
} priv_t;

static priv_t priv;

int app_pre_init(void)
{
    priv.thresholds.push_back({0, 80, 40, 80, 10, 80}); // red
    priv.show_binary = false;
    priv.ptl = new comm::CommProtocol(BUFF_RX_LEN);
    priv.user_lmin = 0;
    priv.user_lmax = 100;
    priv.user_amin = -128;
    priv.user_amax = 127;
    priv.user_bmin = -128;
    priv.user_bmax = 127;
    maix::err::check_null_raise(priv.ptl, "protocol init failed!");
    return 0;
}

int app_init(void)
{
    ui_all_screen_init();

    string path = maix::app::get_app_config_path();
    log::info("app path:%s\n", path.c_str());
    if (!fs::exists(path)) {
        maix::app::set_app_config_kv("app_find_blobs", "user_lmin", "0", false);
        maix::app::set_app_config_kv("app_find_blobs", "user_lmax", "100", false);
        maix::app::set_app_config_kv("app_find_blobs", "user_amin", "-128", false);
        maix::app::set_app_config_kv("app_find_blobs", "user_amax", "127", false);
        maix::app::set_app_config_kv("app_find_blobs", "user_bmin", "-128", false);
        maix::app::set_app_config_kv("app_find_blobs", "user_bmax", "127", true);
        log::info("use default lab config for user\n");
    } else {
        std::string lmin_str, lmax_str, amin_str, amax_str, bmin_str, bmax_str;
        lmin_str = maix::app::get_app_config_kv("app_find_blobs", "user_lmin");
        lmax_str = maix::app::get_app_config_kv("app_find_blobs", "user_lmax");
        amin_str = maix::app::get_app_config_kv("app_find_blobs", "user_amin");
        amax_str = maix::app::get_app_config_kv("app_find_blobs", "user_amax");
        bmin_str = maix::app::get_app_config_kv("app_find_blobs", "user_bmin");
        bmax_str = maix::app::get_app_config_kv("app_find_blobs", "user_bmax");
        if (lmin_str.length())
            priv.user_lmin = std::stoi(lmin_str);
        if (lmax_str.length())
            priv.user_lmax = std::stoi(lmax_str);
        if (amin_str.length())
            priv.user_amin = std::stoi(amin_str);
        if (amax_str.length())
            priv.user_amax = std::stoi(amax_str);
        if (bmin_str.length())
            priv.user_bmin = std::stoi(bmin_str);
        if (bmax_str.length())
            priv.user_bmax = std::stoi(bmax_str);
        log::info("use last lab config for user, {%d, %d, %d, %d, %d, %d}\n",
            priv.user_lmin, priv.user_lmax, priv.user_amin, priv.user_amax, priv.user_bmin, priv.user_bmax);
    }
    ui_set_user_lab_range(priv.user_lmin, priv.user_lmax, priv.user_amin, priv.user_amax, priv.user_bmin, priv.user_bmax);

    return 0;
}

int app_loop(maix::image::Image *img)
{
    if (ui_get_exit_flag()) {
        log::info("exit!\n");
        app::set_exit_flag(true);
    }

    if (ui_lmin_update() || ui_lmax_update() || ui_amin_update() || ui_amax_update() || ui_bmin_update() || ui_bmax_update()) {
        ui_lmin_update();
        ui_lmax_update();
        ui_amin_update();
        ui_amax_update();
        ui_bmin_update();
        ui_bmax_update();

        int lmin, lmax, amin, amax, bmin, bmax;
        lmin = ui_get_lmin_value();
        lmax = ui_get_lmax_value();
        amin = ui_get_amin_value();
        amax = ui_get_amax_value();
        bmin = ui_get_bmin_value();
        bmax = ui_get_bmax_value();

        if (ui_get_user_btn_statue()) {
            priv.user_lmin = lmin;
            priv.user_lmax = lmax;
            priv.user_amin = amin;
            priv.user_amax = amax;
            priv.user_bmin = bmin;
            priv.user_bmax = bmax;
        }

        log::info("{%d, %d, %d, %d, %d, %d}\n", lmin, lmax, amin, amax, bmin, bmax);

        if (priv.thresholds.size() > 0) {
            std::vector<int> threshold = {
                lmin, lmax, amin, amax, bmin, bmax
            };
            priv.thresholds[0] = threshold;
        }
    }

    if (ui_get_active_screen_update()) {
        int x, y;
        ui_get_active_screen_pointer(&x, &y);
        log::info("touch (%d, %d)\n", x, y);
        std::vector<int> rgb_list = img->get_pixel(x, y, true);
        uint8_t rgb_values[3] = {(uint8_t)rgb_list[0], (uint8_t)rgb_list[1], (uint8_t)rgb_list[2]};
        int8_t lab[3];
        ui_utils_rgb_to_lab(rgb_values, lab);
        ui_show_color_box(1);

        static int cnt = 0;
        int idx = cnt % 3;
        ui_set_color_of_color_box(idx, rgb_values[0], rgb_values[1], rgb_values[2]);
        ui_set_value_of_color_box(idx, lab[0], lab[1], lab[2]);
        ui_set_pointer(idx, x, y, 255 - rgb_values[0], 255 - rgb_values[1], 255 - rgb_values[2]);
        cnt ++;
    }

    if (ui_get_eye_update()) {
        if (ui_get_eye_flag()) {
            log::info("eye is open!\n");
            priv.show_binary = true;
        } else {
            log::info("eye is close!\n");
            priv.show_binary = false;
        }
    }

    if (priv.show_binary) {
        img->binary(priv.thresholds);
    } else {
        std::vector<maix::image::Blob> blobs;
        bool invert = false;
        int x_stride = 2;
        int y_stride = 1;
        int area_threshold = 500;
        int pixels_threshold = 500;
        std::vector<int> roi = {1, 1, img->width()- 1, img->height() - 1};
        blobs = img->find_blobs(priv.thresholds, invert, roi, x_stride, y_stride, area_threshold, pixels_threshold);
        for (auto &a : blobs) {
            std::vector<std::vector<int>> mini_corners = a.mini_corners();
            for (int i = 0; i < 4; i ++) {
                img->draw_line(mini_corners[i][0], mini_corners[i][1], mini_corners[(i + 1) % 4][0], mini_corners[(i + 1) % 4][1], maix::image::Color::from_rgb(0, 255, 0), 2);
            }

            uint8_t data[16];
            for (int i = 0; i < 4; i ++) {
                data[i * 4 + 0] = mini_corners[i][0] & 0xff;
                data[i * 4 + 1] = (mini_corners[i][0] >> 8) & 0xff;
                data[i * 4 + 2] = mini_corners[i][1] & 0xff;
                data[i * 4 + 3] = (mini_corners[i][1] >> 8) & 0xff;
            }

            maix::Bytes bytes(data, sizeof(data));
            priv.ptl->report(APP_CMD_REPORT_FIND_BLOBS, &bytes);
        }
    }

    return 0;
}

int app_deinit(void)
{
    maix::app::set_app_config_kv("app_find_blobs", "user_lmin", std::to_string(priv.user_lmin), false);
    maix::app::set_app_config_kv("app_find_blobs", "user_lmax", std::to_string(priv.user_lmax), false);
    maix::app::set_app_config_kv("app_find_blobs", "user_amin", std::to_string(priv.user_amin), false);
    maix::app::set_app_config_kv("app_find_blobs", "user_amax", std::to_string(priv.user_amax), false);
    maix::app::set_app_config_kv("app_find_blobs", "user_bmin", std::to_string(priv.user_bmin), false);
    maix::app::set_app_config_kv("app_find_blobs", "user_bmax", std::to_string(priv.user_bmax), true);
    log::info("save user's lab config, {%d, %d, %d, %d, %d, %d}\n",
            priv.user_lmin, priv.user_lmax, priv.user_amin, priv.user_amax, priv.user_bmin, priv.user_bmax);
    delete priv.ptl;
    return 0;
}