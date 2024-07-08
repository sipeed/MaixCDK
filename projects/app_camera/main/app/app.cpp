#include "lvgl.h"
#include "app.hpp"
#include "stdio.h"
#include "unistd.h"
#include "maix_basic.hpp"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_app.hpp"
#include "maix_fs.hpp"
#include "sophgo_middleware.hpp"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace maix;

static struct {
    uint32_t cam_start_snap_flag : 1;
    uint32_t video_start_prepare_flag : 1;
    uint32_t video_start_flag : 1;
    uint32_t video_stop_flag : 1;

    uint32_t sensor_ae_mode : 1;    // 0,auto; 1,manual
    uint32_t sensor_awb_mode : 1;   // 0,auto; 1,manual

    uint32_t sensor_shutter_value;  // us
    int sensor_iso_value;           // 100~800
    int sensor_ev;                  // -400~400

    uint8_t cam_snap_delay_s;
    int video_start_ms;

    bool camera_resolution_update_flag;
    int camera_resolution_w;
    int camera_resolution_h;
    int resolution_index;
    camera::Camera *camera;
    display::Display *disp;
    display::Display *other_disp;
    touchscreen::TouchScreen *touchscreen;
} priv;

static int save_buff_to_file(char *filename, uint8_t *filebuf, uint32_t filebuf_len)
{
    int fd = -1;
    fd = open(filename, O_WRONLY | O_CREAT, 0777);
    if (fd <= 2) {
        printf("Open filed, fd = %d\r\n", fd);
        return -1;
    }

    int res = 0;
    if ((res = write(fd, filebuf, filebuf_len)) < 0) {
        printf("Write failed");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}


int app_pre_init(void)
{
    priv.camera_resolution_w = 640;
    priv.camera_resolution_h = 480;
    priv.resolution_index = 3;  // 0: 2560x1440; 1: 1920x1080; 2: 1280x720; 3: 640x480
    return 0;
}

static void _ui_update_pic_img(maix::image::Image *img)
{
    printf("update small img\n");
    maix::image::Image *resize_img = NULL, *bgra_img = NULL;
    resize_img = img->resize(48, 48, maix::image::Fit::FIT_CONTAIN);
    if (resize_img) {
        bgra_img = resize_img->to_format(maix::image::FMT_BGRA8888);
        if (bgra_img) {
            ui_update_small_img(bgra_img->data(), bgra_img->data_size());
            delete bgra_img;
        }
        delete resize_img;
    }

    printf("update big img\n");
    resize_img = img->resize(552, 368, maix::image::Fit::FIT_CONTAIN);
    if (resize_img) {
        bgra_img = resize_img->to_format(maix::image::FMT_BGRA8888);
        if (bgra_img) {
            ui_update_big_img(bgra_img->data(), bgra_img->data_size());
            delete bgra_img;
        }
        delete resize_img;
    }
}

static void _ui_update_new_image_from_maix_path(void)
{
    std::string pic_path = maix::app::get_picture_path();
    printf("pic_path: %s\n", pic_path.c_str());
}

int app_base_init(void)
{
    // init camera
    priv.camera = new camera::Camera(priv.camera_resolution_w, priv.camera_resolution_h, image::Format::FMT_YVU420SP);
    err::check_bool_raise(priv.camera->is_opened(), "camera open failed");

    // init display
    priv.disp = new display::Display();
    priv.other_disp = priv.disp->add_channel();  // This object(other_disp) is depend on disp, so we must keep disp.show() running.
    err::check_bool_raise(priv.disp->is_opened(), "display open failed");

    // touch screen
    priv.touchscreen = new touchscreen::TouchScreen();
    err::check_bool_raise(priv.touchscreen->is_opened(), "touchscreen open failed");

    // init gui
    maix::lvgl_init(priv.other_disp, priv.touchscreen);
    app_init();
    return 0;
}

int app_base_deinit(void)
{
    maix::lvgl_destroy();

    if (priv.touchscreen) {
        delete priv.touchscreen;
        priv.touchscreen = NULL;
    }

    if (priv.other_disp) {
        delete priv.other_disp;
        priv.other_disp = NULL;
    }

    if (priv.disp) {
        delete priv.disp;
        priv.disp = NULL;
    }

    if (priv.camera) {
        delete priv.camera;
        priv.camera = NULL;
    }

    app_deinit();
    return 0;
}

int app_base_loop(void)
{
    maix::image::Image *img = priv.camera->read();
    err::check_null_raise(img, "camera read failed");

    priv.disp->show(*img, image::Fit::FIT_FILL);

    // Must run after disp.show
    lv_timer_handler();

    // app loop
    app_loop(*priv.camera, img, *priv.disp, priv.other_disp);

    delete img;
    return 0;
}

int app_init(void)
{
    ui_all_screen_init();

    _ui_update_new_image_from_maix_path();

    usleep(1000 * 1000); // wait sensor init
    uint32_t exposure_time = 0, iso_num = 0;
    mmf_get_exptime_and_iso(0, &exposure_time, &iso_num);
    priv.sensor_shutter_value = exposure_time;
    priv.sensor_iso_value = iso_num;
    priv.sensor_ae_mode = mmf_get_exp_mode(0);
    ui_set_shutter_value((double)exposure_time / 1000000);
    ui_set_iso_value(iso_num);
    ui_set_select_option(priv.resolution_index);
    return 0;
}

static int app_config_param(void)
{
    if (ui_get_cam_snap_flag()) {
        printf("Take a photo\n");
        priv.cam_start_snap_flag = true;

        if (priv.cam_snap_delay_s > 0) {
            ui_anim_photo_delay_start(priv.cam_snap_delay_s);
        }
    }

    if (ui_get_cam_video_start_flag()) {
        printf("Start video\n");
        priv.video_start_prepare_flag = true;
        priv.video_start_flag = true;
        priv.video_start_ms = time::ticks_us();
        ui_set_record_time(0);
    }

    if (ui_get_cam_video_stop_flag()) {
        printf("Stop video\n");
        priv.video_start_prepare_flag = true;
        priv.video_stop_flag = true;
        priv.video_start_ms = 0;
        ui_set_record_time(0);
    }

    if (ui_get_cam_video_try_stop_flag()) {
        printf("Try to stop video\n");
        priv.video_start_prepare_flag = true;
        priv.video_stop_flag = true;
        priv.video_start_ms = 0;
        ui_set_record_time(0);
    }

    if (ui_get_view_photo_flag()) {
        printf("View photo\n");
    }

    if (ui_get_exit_flag()) {
        printf("Exit\n");
        app::set_exit_flag(true);
    }

    if (ui_get_delay_setting_flag()) {
        int delay_ms;
        ui_get_photo_delay(&delay_ms);
        printf("Delay setting: %d ms\n", delay_ms);
        priv.cam_snap_delay_s = delay_ms / 1000;
    }

    if (ui_get_resolution_setting_flag()) {
        int w, h;
        ui_get_resulution(&w, &h);
        priv.resolution_index = ui_get_resolution_setting_idx();
        printf("Resolution setting: %d x %d\n", w, h);

        priv.camera_resolution_update_flag = 1;
        priv.camera_resolution_w = w;
        priv.camera_resolution_h = h;
    }

    if (ui_get_shutter_setting_flag()) {
        if (ui_get_shutter_auto_flag()) {
            printf("Shutter setting: Auto\n");
            if (priv.sensor_ae_mode != 0) {
                mmf_set_exp_mode(0, 0);
                priv.sensor_ae_mode = 0;
            }
        } else {
            if (priv.sensor_ae_mode == 0) {
                uint32_t exposure_time = 0, iso_num = 0;
                mmf_set_exp_mode(0, 1);
                mmf_get_exptime_and_iso(0, &exposure_time, &iso_num);
                priv.sensor_shutter_value = exposure_time;
                priv.sensor_iso_value = iso_num;
                priv.sensor_ae_mode = 1;
            }

            uint64_t shutter_value;
            ui_get_shutter_value(&shutter_value);
            printf("Shutter setting: %ld us\n", shutter_value);
            if (shutter_value != 0) {
                mmf_set_exptime_and_iso(0, shutter_value, priv.sensor_iso_value);
            }
            priv.sensor_shutter_value = shutter_value;
        }
    }

    if (ui_get_iso_setting_flag()) {
        if (ui_get_iso_auto_flag()) {
            printf("ISO setting: Auto\n");
            if (priv.sensor_ae_mode != 0) {
                mmf_set_exp_mode(0, 0);
                priv.sensor_ae_mode = 0;
            }
        } else {
            if (priv.sensor_ae_mode == 0) {
                uint32_t exposure_time = 0, iso_num = 0;
                mmf_set_exp_mode(0, 1);
                mmf_get_exptime_and_iso(0, &exposure_time, &iso_num);
                priv.sensor_shutter_value = exposure_time;
                priv.sensor_iso_value = iso_num;
                priv.sensor_ae_mode = 1;
            }
            int iso_value;
            ui_get_iso_value(&iso_value);
            printf("ISO setting: %d\n", iso_value);
            mmf_set_exptime_and_iso(0, priv.sensor_shutter_value, iso_value);
            priv.sensor_iso_value = iso_value;
        }
    }

    if (ui_get_ev_setting_flag()) {
        if (ui_get_ev_auto_flag()) {
            printf("EV setting: Auto\n");
        } else {
            double ev_value;
            ui_get_ev_value(&ev_value);
            printf("EV setting: %f\n", ev_value);
        }
    }

    if (ui_get_wb_setting_flag()) {
        if (ui_get_wb_auto_flag()) {
            printf("WB setting: Auto\n");
        } else {
            int wb_value;
            ui_get_wb_value(&wb_value);
            printf("WB setting: %d\n", wb_value);
        }
    }

    return 0;
}

int app_loop(maix::camera::Camera &camera, maix::image::Image *img, maix::display::Display &disp, maix::display::Display *disp2)
{
    app_config_param();

    if (priv.cam_start_snap_flag) {
        if (priv.cam_snap_delay_s == 0 || (priv.cam_snap_delay_s > 0 && ui_get_photo_delay_anim_stop_flag())) {
            printf("save photo\n");
            priv.cam_start_snap_flag = false;
            char *date = ui_get_sys_date();
            if (date) {
                string picture_root_path = maix::app::get_picture_path();
                string picture_date(date);
                string picture_path = picture_root_path + "/" + picture_date;
                printf("picture_path path:%s\n", picture_path.c_str());
                if (!fs::exists(picture_path)) {
                    fs::mkdir(picture_path);
                }
                std::vector<std::string> *file_list = fs::listdir(picture_path);
                printf("file_list_cnt:%ld\n", file_list->size());
                string picture_save_path = picture_path + "/" + std::to_string(file_list->size()) +".jpg";

                printf("picture_path path:%s  picture_save_path:%s\n", picture_path.c_str(), picture_save_path.c_str());
                maix::image::Image *jpg_img = img->to_format(maix::image::FMT_JPEG);
                if (jpg_img) {
                    save_buff_to_file((char *)picture_save_path.c_str(), (uint8_t *)jpg_img->data(), jpg_img->data_size());
                    system("sync");
                    delete jpg_img;

                    printf("update small and big img\n");
                    maix::image::Image *new_img = maix::image::load((char *)picture_save_path.c_str(), maix::image::FMT_RGB888);
                    ui_anim_run_save_img();
                    _ui_update_pic_img(new_img);
                    delete new_img;
                } else {
                    printf("encode nv21 to jpg failed!\n");
                }
                free(file_list);
                free(date);
            } else {
                printf("get date failed!\n");
            }
        }
    }

    if (priv.video_start_flag && !priv.video_start_prepare_flag) {
        printf("Prepare record video\n");
        priv.video_start_prepare_flag = true;
    }

    if (priv.video_start_flag && priv.video_start_prepare_flag) {
        printf("Start video\n");
        uint64_t record_time = time::ticks_us() - priv.video_start_ms;
        ui_set_record_time(record_time);
    }

    if (priv.video_stop_flag) {
        printf("Stop video\n");
        priv.video_stop_flag = false;
        priv.video_start_prepare_flag = false;
        priv.video_start_flag = false;
        priv.video_start_ms = 0;
    }

    if (priv.camera_resolution_update_flag) {
        priv.camera_resolution_update_flag = false;
        app_base_deinit();
        app_base_init();
    }

    return 0;
}

int app_deinit(void)
{
    return 0;
}