#include "stdio.h"
#include "lvgl.h"
#include "ui_screen.h"
#include "ui_utils.h"
#include "ui_event_handler.h"

#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
#define DEBUG_EN(x)                                                         \
    [[maybe_unused]]bool g_debug_flag = x;

#define DEBUG_PRT(fmt, ...) do {                                            \
    if (g_debug_flag)                                                       \
        printf("[%s][%d]: " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__);   \
} while(0)
#else
#define DEBUG_EN(fmt, ...)
#define DEBUG_PRT(fmt, ...)
#endif

LV_IMG_DECLARE(img_light_on);
LV_IMG_DECLARE(img_light_off);

extern lv_obj_t *g_camera_video_button;
extern lv_obj_t *g_start_snap_button;
extern lv_obj_t *g_exit_button;
extern lv_obj_t *g_delay_button;
extern lv_obj_t *g_resolution_button;
extern lv_obj_t *g_menu_button;
extern lv_obj_t *g_delay_setting;
extern lv_obj_t *g_resolution_setting;
extern lv_obj_t *g_bitrate_setting;
extern lv_obj_t *g_timelapse_setting;
extern lv_obj_t *g_menu_setting;
extern lv_obj_t *g_shutter_button;
extern lv_obj_t *g_iso_button;
extern lv_obj_t *g_ev_button;
extern lv_obj_t *g_wb_button;
extern lv_obj_t *g_shutter_setting;
extern lv_obj_t *g_iso_setting;
extern lv_obj_t *g_ev_setting;
extern lv_obj_t *g_wb_setting;
extern lv_obj_t *g_small_img;
extern lv_obj_t *g_big_img;
extern lv_obj_t *g_video_running_screen;
extern lv_obj_t *g_focus_button;
extern lv_obj_t *g_shutter_plus_minus_button;
extern lv_obj_t *g_iso_plus_minus_button;
extern lv_obj_t *g_raw_button;
extern lv_obj_t *g_light_button;
extern lv_obj_t *g_bitrate_button;
extern lv_obj_t *g_drop_down_img;
extern lv_obj_t *g_timestamp_button;
extern lv_obj_t *g_timelapse_button;

static struct {
    unsigned int camera_snap_start_flag : 1;
    unsigned int camera_video_start_flag : 1;
    unsigned int camera_video_stop_flag : 1;
    unsigned int camera_video_try_stop_flag : 1;
    unsigned int view_photo_flag : 1;
    unsigned int exit_flag : 1;
    unsigned int delay_setting_flag : 1;
    unsigned int resolution_setting_flag : 1;
    unsigned int shutter_setting_flag : 1;
    unsigned int iso_setting_flag : 1;
    unsigned int ev_setting_flag : 1;
    unsigned int wb_setting_flag : 1;
    unsigned int shutter_auto_flag : 1;
    unsigned int iso_auto_flag : 1;
    unsigned int ev_auto_flag : 1;
    unsigned int wb_auto_flag : 1;
    unsigned int photo_delay_anim_start_flag : 1;
    unsigned int photo_delay_anim_stop_flag : 1;
    unsigned int focus_btn_touched : 1;
    unsigned int focus_btn_update_flag : 1;
    unsigned int raw_btn_touched : 1;
    unsigned int raw_btn_update_flag : 1;
    unsigned int light_btn_touched : 1;
    unsigned int light_btn_update_flag : 1;
    unsigned int timestamp_btn_touched : 1;
    unsigned int timestamp_btn_update_flag : 1;
    unsigned int bitrate_update_flag : 1;
    unsigned int timelapse_update_flag : 1;

    unsigned int resolution_setting_idx;
    unsigned int bitrate;
    unsigned int timelapse_s;
} priv = {
    .iso_auto_flag = 1,
    .shutter_auto_flag = 1,
};

int g_camera_mode = 0;

void event_touch_exit_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("try to exit\n");
        priv.exit_flag = 1;
    }
}

static void _set_hidden_and_clear_checked(lv_obj_t *without_obj)
{
    lv_obj_t *list[] = {
        g_delay_button,
        g_resolution_button,
        g_bitrate_button,
        g_timelapse_button,
    };
    lv_obj_t *list2[] = {
        g_delay_setting,
        g_resolution_setting,
        g_bitrate_setting,
        g_timelapse_setting,
    };

    for (size_t i = 0; i < sizeof(list) / sizeof(list[0]); i ++) {
        lv_obj_t *button = list[i];
        lv_obj_t *setting = list2[i];

        if (button == without_obj) {
            continue;;
        }

        lv_obj_add_flag(setting, LV_OBJ_FLAG_HIDDEN);
        if (lv_obj_get_state(button) == LV_STATE_CHECKED) {
            lv_obj_send_event(button, LV_EVENT_RELEASED, NULL);
        }
    }
}

void event_left_screen_scroll_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    DEBUG_PRT("code:%d\n", code);

    if (code == LV_EVENT_SCROLL) {
        int oft_y = lv_obj_get_scroll_y(lv_event_get_target(e));
        DEBUG_PRT("scroll number:%d\n", oft_y);

        if (g_drop_down_img) {
            if (oft_y > 0) {
                lv_obj_add_flag(g_drop_down_img, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(g_drop_down_img, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

void event_touch_delay_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_delay_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_delay_setting, LV_OBJ_FLAG_HIDDEN);

            _set_hidden_and_clear_checked(g_delay_button);
        } else {
            lv_obj_add_flag(g_delay_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_resolution_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_resolution_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_resolution_setting, LV_OBJ_FLAG_HIDDEN);

            _set_hidden_and_clear_checked(g_resolution_button);
        } else {
            lv_obj_add_flag(g_resolution_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_bitrate_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_bitrate_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_bitrate_setting, LV_OBJ_FLAG_HIDDEN);

            _set_hidden_and_clear_checked(g_bitrate_button);
        } else {
            lv_obj_add_flag(g_bitrate_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_timelapse_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_timelapse_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_timelapse_setting, LV_OBJ_FLAG_HIDDEN);

            _set_hidden_and_clear_checked(g_timelapse_button);
        } else {
            lv_obj_add_flag(g_timelapse_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void _shutter_setting_set_hidden(bool hidden)
{
    if (hidden) {
        lv_obj_add_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_shutter_plus_minus_button, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(g_shutter_plus_minus_button, LV_OBJ_FLAG_HIDDEN);
    }
}

static void _iso_setting_set_hidden(bool hidden)
{
    if (hidden) {
        lv_obj_add_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_iso_plus_minus_button, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(g_iso_plus_minus_button, LV_OBJ_FLAG_HIDDEN);
    }
}

void event_touch_option_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_menu_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_menu_setting, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_menu_setting, LV_OBJ_FLAG_HIDDEN);

            _shutter_setting_set_hidden(true);
            if (lv_obj_get_state(g_shutter_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_shutter_button, LV_EVENT_RELEASED, NULL);
            }
            _iso_setting_set_hidden(true);
            if (lv_obj_get_state(g_iso_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_iso_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_ev_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_ev_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_ev_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_wb_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_wb_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_wb_button, LV_EVENT_RELEASED, NULL);
            }
        }
    }
}

void touch_video_camera()
{
    DEBUG_EN(0);
    if (lv_obj_has_flag(g_start_snap_button, LV_OBJ_FLAG_CHECKABLE)) {
        if (lv_obj_get_state(g_start_snap_button) == LV_STATE_CHECKED) {
            lv_obj_send_event(g_start_snap_button, LV_EVENT_RELEASED, NULL);
            lv_obj_add_flag(g_video_running_screen, LV_OBJ_FLAG_HIDDEN);
            DEBUG_PRT("video try stop!\n");

            priv.camera_video_try_stop_flag = 1;
        }
        lv_obj_remove_flag(g_start_snap_button, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_remove_state(g_start_snap_button, LV_STATE_USER_1);
        DEBUG_PRT("ready to snap photo!\n");
        g_camera_mode = 0;
    } else {
        lv_obj_add_flag(g_start_snap_button, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_state(g_start_snap_button, LV_STATE_CHECKED);
        DEBUG_PRT("ready to record video!\n");
        g_camera_mode = 1;
    }
}

void event_touch_video_camera_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        touch_video_camera();
    }
}

static void touch_start_video_start(void)
{
    DEBUG_EN(1);
    lv_obj_remove_flag(g_video_running_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_state(g_start_snap_button, LV_STATE_PRESSED);
    lv_obj_add_state(g_start_snap_button, LV_STATE_USER_1);
    DEBUG_PRT("video start\n");
    priv.camera_video_start_flag = 1;
}
static void touch_start_video_stop(void)
{
    DEBUG_EN(1);
    lv_obj_add_flag(g_video_running_screen, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_remove_state(g_start_snap_button, LV_STATE_PRESSED);
    lv_obj_remove_state(g_start_snap_button, LV_STATE_USER_1);
    if (lv_obj_has_state(g_start_snap_button, LV_STATE_PRESSED)) {
        printf("press\n");
    }
    DEBUG_PRT("video stop\n");
    priv.camera_video_stop_flag = 1;
}

LV_IMG_DECLARE(img_photo_release);
LV_IMG_DECLARE(img_photo_clicked);
LV_IMG_DECLARE(img_video_ready);
LV_IMG_DECLARE(img_video_stop);
void touch_start_video(int flag)
{
    DEBUG_EN(0);
    DEBUG_PRT("[video]\n");

    // auto __start = []() {
    //     lv_obj_remove_flag(g_video_running_screen, LV_OBJ_FLAG_HIDDEN);
    //     lv_obj_remove_state(g_start_snap_button, LV_STATE_PRESSED);
    //     lv_obj_add_state(g_start_snap_button, LV_STATE_USER_1);
    //     DEBUG_PRT("video start\n");
    //     priv.camera_video_start_flag = 1;
    // }

    if (flag > 0) {
        lv_image_dsc_t* img_desc = (lv_image_dsc_t*)lv_obj_get_style_bg_image_src(g_start_snap_button, 0);
        if (img_desc == &img_video_ready) {
            touch_start_video_start();
        } else {
            touch_start_video_stop();
        }
    } else {
        if (lv_obj_get_state(g_start_snap_button) == LV_STATE_FOCUSED) {
            touch_start_video_start();
        } else {
            touch_start_video_stop();
        }
    }
}

void touch_start_pic()
{
    DEBUG_EN(0);
    DEBUG_PRT("[photo]\n");
    DEBUG_PRT("camera snap start\n");
    priv.camera_snap_start_flag = 1;
}

void event_touch_start_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_camera_video_button) == LV_STATE_CHECKED) {
            touch_start_video(-1);
        } else {
            touch_start_pic();
        }
    }
}

void event_touch_small_img_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("View big photo\n");
        lv_obj_remove_flag(g_big_img, LV_OBJ_FLAG_HIDDEN);
        priv.view_photo_flag = 1;
    }
}

void event_touch_big_img_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("Hidden big photo\n");
        lv_obj_add_flag(g_big_img, LV_OBJ_FLAG_HIDDEN);
    }
}

void event_touch_shutter_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_shutter_button) != LV_STATE_FOCUSED) {
            _shutter_setting_set_hidden(false);

            _iso_setting_set_hidden(true);
            if (lv_obj_get_state(g_iso_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_iso_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_ev_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_ev_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_ev_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_wb_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_wb_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_wb_button, LV_EVENT_RELEASED, NULL);
            }
        } else {
            _shutter_setting_set_hidden(true);
        }
    }
}

void event_touch_iso_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_iso_button) != LV_STATE_FOCUSED) {
            _iso_setting_set_hidden(false);

            _shutter_setting_set_hidden(true);
            if (lv_obj_get_state(g_shutter_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_shutter_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_ev_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_ev_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_ev_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_wb_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_wb_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_wb_button, LV_EVENT_RELEASED, NULL);
            }
        } else {
            _iso_setting_set_hidden(true);
        }
    }
}

void event_touch_ev_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_ev_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_ev_setting, LV_OBJ_FLAG_HIDDEN);

            _shutter_setting_set_hidden(true);
            if (lv_obj_get_state(g_shutter_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_shutter_button, LV_EVENT_RELEASED, NULL);
            }
            _iso_setting_set_hidden(true);
            if (lv_obj_get_state(g_iso_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_iso_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_wb_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_wb_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_wb_button, LV_EVENT_RELEASED, NULL);
            }
        } else {
            lv_obj_add_flag(g_ev_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}


static double get_index_of_exptime_table(double exposure_time_us)
{
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    int number_of_table = sizeof(camera_config.exposure_time_table) / sizeof(camera_config.exposure_time_table[0]);
    double shutter_exp_s = exposure_time_us / 1000000;
    int index_of_table = 0;
    for (int i = 0; i < number_of_table; i++) {
        // DEBUG_PRT("%f < %f ?", shutter_exp_s, camera_config.exposure_time_table[i]);
        if (shutter_exp_s <= camera_config.exposure_time_table[i]) {
            break;
        }
        index_of_table ++;
    }
    index_of_table = index_of_table >= number_of_table ? number_of_table - 1 : index_of_table;
    return index_of_table;
}

static double get_exptime_from_exptime_table(int index_of_table)
{
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    int number_of_table = sizeof(camera_config.exposure_time_table) / sizeof(camera_config.exposure_time_table[0]);
    index_of_table = index_of_table < 0 ? 0 : index_of_table;
    index_of_table = index_of_table >= number_of_table ? number_of_table - 1 : index_of_table;
    double exposure_time_us = camera_config.exposure_time_table[index_of_table] * 1000000;
    return exposure_time_us;
}


static double get_index_of_iso_table(int iso)
{
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    int number_of_table = sizeof(camera_config.iso_table) / sizeof(camera_config.iso_table[0]);
    int index_of_table = 0;
    for (int i = 0; i < number_of_table; i++) {
        // DEBUG_PRT("%f < %f ?", iso, camera_config.iso_table[i]);
        if (iso <= camera_config.iso_table[i]) {
            break;
        }
        index_of_table ++;
    }
    index_of_table = index_of_table >= number_of_table ? number_of_table - 1 : index_of_table;
    return index_of_table;
}

static int get_iso_from_iso_table(int index_of_table)
{
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    int number_of_table = sizeof(camera_config.iso_table) / sizeof(camera_config.iso_table[0]);
    index_of_table = index_of_table < 0 ? 0 : index_of_table;
    index_of_table = index_of_table >= number_of_table ? number_of_table - 1 : index_of_table;
    int iso = camera_config.iso_table[index_of_table];
    return iso;
}

void event_touch_shutter_plus_cb(lv_event_t *e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        double exposure_time_us;
        ui_get_shutter_value(&exposure_time_us);
        DEBUG_PRT("get exposure time us: %f us\n", exposure_time_us);
        int index_of_table = get_index_of_exptime_table(exposure_time_us);
        DEBUG_PRT("caculated index_of_table: %d\n", index_of_table);
        index_of_table += 1;
        exposure_time_us = get_exptime_from_exptime_table(index_of_table);
        DEBUG_PRT("caculated exposure_time_us: %f us\n", exposure_time_us);
        ui_set_shutter_value(exposure_time_us);

        priv.shutter_setting_flag = 1;

        // set to manual
        DEBUG_PRT("set shutter to manual mode\n");
        priv.shutter_auto_flag = 0;
    }
}

void event_touch_shutter_minus_cb(lv_event_t *e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        double exposure_time_us;
        ui_get_shutter_value(&exposure_time_us);
        DEBUG_PRT("get exposure time us: %f us\n", exposure_time_us);
        int index_of_table = get_index_of_exptime_table(exposure_time_us);
        DEBUG_PRT("caculated index_of_table: %d\n", index_of_table);
        index_of_table -= 1;
        exposure_time_us = get_exptime_from_exptime_table(index_of_table);
        DEBUG_PRT("caculated exposure_time_us: %f us\n", exposure_time_us);
        ui_set_shutter_value(exposure_time_us);


        priv.shutter_setting_flag = 1;

        // set to manual
        DEBUG_PRT("set shutter to manual mode\n");
        priv.shutter_auto_flag = 0;
    }
}

void event_touch_iso_plus_cb(lv_event_t *e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        int iso;
        ui_get_iso_value(&iso);
        DEBUG_PRT("get iso: %d\n", iso);
        int index_of_table = get_index_of_iso_table(iso);
        DEBUG_PRT("caculated index_of_table: %d\n", index_of_table);
        index_of_table += 1;
        iso = get_iso_from_iso_table(index_of_table);
        DEBUG_PRT("caculated iso: %d\n", iso);
        ui_set_iso_value(iso);

        priv.iso_setting_flag = 1;

        // set to manual
        DEBUG_PRT("set iso to manual mode\n");
        priv.iso_auto_flag = 0;
    }
}

void event_touch_iso_minus_cb(lv_event_t *e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        int iso;
        ui_get_iso_value(&iso);
        DEBUG_PRT("get iso: %d\n", iso);
        int index_of_table = get_index_of_iso_table(iso);
        DEBUG_PRT("caculated index_of_table: %d\n", index_of_table);
        index_of_table -= 1;
        iso = get_iso_from_iso_table(index_of_table);
        DEBUG_PRT("caculated iso: %d\n", iso);
        ui_set_iso_value(iso);

        priv.iso_setting_flag = 1;

        // set to manual
        DEBUG_PRT("set iso to manual mode\n");
        priv.iso_auto_flag = 0;
    }
}
void event_touch_wb_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_wb_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_wb_setting, LV_OBJ_FLAG_HIDDEN);

            _shutter_setting_set_hidden(true);
            if (lv_obj_get_state(g_shutter_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_shutter_button, LV_EVENT_RELEASED, NULL);
            }
            _iso_setting_set_hidden(true);
            if (lv_obj_get_state(g_iso_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_iso_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_ev_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_ev_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_ev_button, LV_EVENT_RELEASED, NULL);
            }
        } else {
            lv_obj_add_flag(g_wb_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_focus_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_focus_button) != LV_STATE_FOCUSED) {
            priv.focus_btn_touched = 1;
            priv.focus_btn_update_flag = 1;
        } else {
            priv.focus_btn_touched = 0;
            priv.focus_btn_update_flag = 1;
        }
    }
}

void event_touch_raw_cb(lv_event_t * e)
{
    intptr_t param = (intptr_t)lv_event_get_param(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *on_off = lv_obj_get_child(g_raw_button, -1);
        if (lv_obj_get_state(g_raw_button) != LV_STATE_FOCUSED) {
            if (param != -1) {
                priv.raw_btn_touched = 1;
                priv.raw_btn_update_flag = 1;
            }
            lv_obj_set_style_text_color(on_off, lv_color_hex(0x00ff00), 0);
            lv_label_set_text(on_off, "ON");
        } else {
            if (param != -1) {
                priv.raw_btn_touched = 0;
                priv.raw_btn_update_flag = 1;
            }
            lv_obj_set_style_text_color(on_off, lv_color_hex(0xff0000), 0);
            lv_label_set_text(on_off, "OFF");
        }
    }
}

void event_touch_light_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_light_button) != LV_STATE_FOCUSED) {
            priv.light_btn_touched = 1;
            priv.light_btn_update_flag = 1;

            lv_obj_t *img = lv_obj_get_child(g_light_button, -1);
            lv_image_set_src(img, &img_light_on);
        } else {
            priv.light_btn_touched = 0;
            priv.light_btn_update_flag = 1;

            lv_obj_t *img = lv_obj_get_child(g_light_button, -1);
            lv_image_set_src(img, &img_light_off);
        }
    }
}

void event_touch_timestamp_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_timestamp_button) != LV_STATE_FOCUSED) {
            priv.timestamp_btn_touched = 1;
            priv.timestamp_btn_update_flag = 1;
        } else {
            priv.timestamp_btn_touched = 0;
            priv.timestamp_btn_update_flag = 1;
        }
    }
}

static double bar_value_to_exp_us(lv_obj_t *obj, uint16_t shutter_bar_value)
{
    DEBUG_EN(0);
    lv_bar_t * bar = (lv_bar_t *)obj;
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    DEBUG_PRT("input shutter_bar_value: %d", shutter_bar_value);
    shutter_bar_value = shutter_bar_value > bar->max_value ? bar->max_value : shutter_bar_value;
    shutter_bar_value = shutter_bar_value < bar->min_value ? bar->min_value : shutter_bar_value;
    shutter_bar_value -= bar->min_value;

    int number_of_table = sizeof(camera_config.exposure_time_table) / sizeof(camera_config.exposure_time_table[0]);
    DEBUG_PRT("caculated number_of_table:%d\n", number_of_table);
    int index_of_table = shutter_bar_value / (double)((bar->max_value - bar->min_value) / (number_of_table - 1));
    DEBUG_PRT("caculated index_of_table:%d\n", index_of_table);
    double exposure_time = camera_config.exposure_time_table[index_of_table] * 1000000;
    DEBUG_PRT("found exposure time:%f\n", exposure_time);
    return exposure_time;
}

static uint16_t exp_us_to_bar_value(lv_obj_t *obj, double exposure_time_us)
{
    DEBUG_EN(0);
    lv_bar_t * bar = (lv_bar_t *)obj;
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    DEBUG_PRT("input shutter_exp_us: %f s", exposure_time_us);
    int number_of_table = sizeof(camera_config.exposure_time_table) / sizeof(camera_config.exposure_time_table[0]);
    DEBUG_PRT("caculated number_of_table:%d\n", number_of_table);
    int index_of_table = get_index_of_exptime_table(exposure_time_us);
    DEBUG_PRT("caculated index_of_table:%d\n", index_of_table);
    uint16_t bar_value = index_of_table * (double)(bar->max_value - bar->min_value) / (number_of_table - 1);
    DEBUG_PRT("found bar_value:%d\n", bar_value);
    return bar_value;
}

static int bar_value_to_iso(lv_obj_t *obj, uint16_t bar_value)
{
    DEBUG_EN(0);
    lv_bar_t * bar = (lv_bar_t *)obj;
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    DEBUG_PRT("input bar_value: %d", bar_value);
    bar_value = bar_value > bar->max_value ? bar->max_value : bar_value;
    bar_value = bar_value < bar->min_value ? bar->min_value : bar_value;
    bar_value -= bar->min_value;

    int number_of_table = sizeof(camera_config.iso_table) / sizeof(camera_config.iso_table[0]);
    DEBUG_PRT("caculated number_of_table:%d\n", number_of_table);
    int index_of_table = bar_value / (double)((bar->max_value - bar->min_value) / (number_of_table - 1));
    DEBUG_PRT("caculated index_of_table:%d\n", index_of_table);
    int iso = camera_config.iso_table[index_of_table];
    DEBUG_PRT("found iso:%d\n", iso);
    return iso;
}

static uint16_t iso_to_bar_value(lv_obj_t *obj, int iso)
{
    DEBUG_EN(0);
    lv_bar_t * bar = (lv_bar_t *)obj;
    ui_camera_config_t camera_config;
    ui_camera_config_read(&camera_config);
    DEBUG_PRT("input iso: %d", iso);
    int number_of_table = sizeof(camera_config.iso_table) / sizeof(camera_config.iso_table[0]);
    DEBUG_PRT("caculated number_of_table:%d\n", number_of_table);
    int index_of_table = get_index_of_iso_table(iso);
    DEBUG_PRT("caculated index_of_table:%d\n", index_of_table);
    uint16_t bar_value = index_of_table * (double)(bar->max_value - bar->min_value) / (number_of_table - 1);
    DEBUG_PRT("found bar_value:%d\n", bar_value);
    return bar_value;
}

void event_shutter_bar_update_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_PRESSING) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);

        int h = obj->coords.y2 - obj->coords.y1;
        int h_oft = obj->coords.y2 - point.y;
        h_oft = h_oft > h ? h : h_oft;
        h_oft = h_oft < 0 ? 0 : h_oft;

        lv_bar_t * bar = (lv_bar_t *)obj;
        int bar_max_val = abs(bar->max_value - bar->min_value);
        int val = bar->min_value + bar_max_val * h_oft / h;
        lv_bar_set_value((lv_obj_t *)bar, val, LV_ANIM_OFF);

        double exposure_time_us = bar_value_to_exp_us(obj, val);
        ui_set_shutter_value(exposure_time_us);

        priv.shutter_setting_flag = 1;
    } else if (code == LV_EVENT_RELEASED) {
        DEBUG_EN(0);
        int val = lv_bar_get_value(obj);
        DEBUG_PRT("found bar value: %d", val);
        double exposure_time_us = bar_value_to_exp_us(obj, val);
        DEBUG_PRT("caculated exposure_time_us: %f us", exposure_time_us);
        ui_set_shutter_value(exposure_time_us);

        priv.shutter_setting_flag = 1;

        // set to manual
        DEBUG_PRT("set shutter to manual mode\n");
        priv.shutter_auto_flag = 0;
    }
}

void event_iso_bar_update_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_PRESSING) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);

        int h = obj->coords.y2 - obj->coords.y1;
        int h_oft = obj->coords.y2 - point.y;
        h_oft = h_oft > h ? h : h_oft;
        h_oft = h_oft < 0 ? 0 : h_oft;

        lv_bar_t * bar = (lv_bar_t *)obj;
        int bar_max_val = abs(bar->max_value - bar->min_value);
        int val = bar->min_value + bar_max_val * h_oft / h;
        lv_bar_set_value((lv_obj_t *)bar, val, LV_ANIM_OFF);

        DEBUG_PRT("found bar value: %d", val);
        int iso = bar_value_to_iso(obj, val);
        DEBUG_PRT("caculated iso: %d", iso);
        ui_set_iso_value(iso);

        priv.iso_setting_flag = 1;
    } else if (code == LV_EVENT_RELEASED) {
        int val = lv_bar_get_value(obj);
        DEBUG_PRT("found bar value: %d", val);
        int iso = bar_value_to_iso(obj, val);
        DEBUG_PRT("caculated iso: %d", iso);
        ui_set_iso_value(iso);

        priv.iso_setting_flag = 1;

        // set to manual
        DEBUG_PRT("set iso to manual mode\n");
        priv.iso_auto_flag = 0;
    }
}

void event_ev_bar_update_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_PRESSING) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);

        int h = obj->coords.y2 - obj->coords.y1;
        int h_oft = obj->coords.y2 - point.y;
        h_oft = h_oft > h ? h : h_oft;
        h_oft = h_oft < 0 ? 0 : h_oft;

        lv_bar_t * bar = (lv_bar_t *)obj;
        int bar_max_val = abs(bar->max_value - bar->min_value);
        int val = bar->min_value + bar_max_val * h_oft / h;
        lv_bar_set_value((lv_obj_t *)bar, val, LV_ANIM_OFF);

        ui_set_ev_value(val);

        DEBUG_PRT("ev value updating: %d\n", val);

        priv.ev_setting_flag = 1;
    } else if (code == LV_EVENT_RELEASED) {
        int val = lv_bar_get_value(obj);

        ui_set_ev_value(val);

        DEBUG_PRT("ev value update: %d\n", val);

        priv.ev_setting_flag = 1;
    }
}

void event_wb_bar_update_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_PRESSING) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);

        int h = obj->coords.y2 - obj->coords.y1;
        int h_oft = obj->coords.y2 - point.y;
        h_oft = h_oft > h ? h : h_oft;
        h_oft = h_oft < 0 ? 0 : h_oft;

        lv_bar_t * bar = (lv_bar_t *)obj;
        int bar_max_val = abs(bar->max_value - bar->min_value);
        int val = bar->min_value + bar_max_val * h_oft / h;
        lv_bar_set_value((lv_obj_t *)bar, val, LV_ANIM_OFF);

        ui_set_wb_value(val);

        DEBUG_PRT("wb value updating: %d\n", val);

        priv.wb_setting_flag = 1;
    } else if (code == LV_EVENT_RELEASED) {
        int val = lv_bar_get_value(obj);

        ui_set_wb_value(val);

        DEBUG_PRT("wb value update: %d\n", val);

        priv.wb_setting_flag = 1;
    }
}

void event_touch_shutter_to_auto_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("set exposure time to auto mode\n");
        priv.shutter_setting_flag = 1;
        priv.shutter_auto_flag = 1;
        ui_set_shutter_value(-1);
    }
}

void event_touch_iso_to_auto_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("set iso to auto mode\n");
        priv.shutter_setting_flag = 1;
        priv.iso_auto_flag = 1;
        ui_set_iso_value(-1);
    }
}

void event_touch_ev_to_auto_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            DEBUG_PRT("set ev to auto mode\n");
            ui_set_ev_value(-1000);

            priv.ev_setting_flag = 1;
            priv.ev_auto_flag = 1;
        } else {
            DEBUG_PRT("set ev to manual mode\n");
            int *bar_val = (int *)lv_obj_get_user_data(g_ev_setting);
            int val = *bar_val;
            ui_set_ev_value(val);

            priv.ev_setting_flag = 1;
            priv.ev_auto_flag = 0;
        }
    }
}

void event_touch_wb_to_auto_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            DEBUG_PRT("set wb to auto mode\n");
            ui_set_wb_value(-1);

            priv.wb_setting_flag = 1;
            priv.wb_auto_flag = 1;
        } else {
            DEBUG_PRT("set wb to manual mode\n");
            int *bar_val = (int *)lv_obj_get_user_data(g_wb_setting);
            int val = *bar_val;
            ui_set_wb_value(val);

            priv.wb_setting_flag = 1;
            priv.wb_auto_flag = 0;
        }
    }
}

void event_touch_select_delay_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            for (size_t i = 0; i < lv_obj_get_child_count(parent); i++) {
                lv_obj_t *child = lv_obj_get_child(parent, i);
                if (child != obj) {
                    lv_obj_remove_state(child, LV_STATE_CHECKED);
                }
            }

            lv_obj_t *label = lv_obj_get_child(obj, -1);
            char *text = lv_label_get_text(label);
            lv_obj_set_user_data(parent, text);
            DEBUG_PRT("select delay: %s\n", text);
            priv.delay_setting_flag = 1;
        } else {
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        }
    }
}

void event_touch_select_resolution_cb(lv_event_t * e)
{
    DEBUG_EN(0);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            for (size_t i = 0; i < lv_obj_get_child_count(parent); i++) {
                lv_obj_t *child = lv_obj_get_child(parent, i);
                if (child != obj) {
                    lv_obj_remove_state(child, LV_STATE_CHECKED);
                }
            }

            lv_obj_t *label = lv_obj_get_child(obj, -2);
            char *text = lv_label_get_text(label);
            lv_obj_set_user_data(parent, text);
            LV_UNUSED(text);
            DEBUG_PRT("select resolution: %s\n", text);

            priv.resolution_setting_idx = lv_obj_get_index(obj);
            priv.resolution_setting_flag = 1;
        } else {
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        }
    }
}

void event_touch_select_bitrate_cb(lv_event_t * e)
{
    DEBUG_EN(1);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            for (size_t i = 0; i < lv_obj_get_child_count(parent); i++) {
                lv_obj_t *child = lv_obj_get_child(parent, i);
                if (child != obj) {
                    lv_obj_remove_state(child, LV_STATE_CHECKED);
                }
            }

            lv_obj_t *label = lv_obj_get_child(obj, -1);
            char *text = lv_label_get_text(label);
            lv_obj_set_user_data(parent, text);
            LV_UNUSED(text);

            priv.bitrate = atoi(text) * 1000;
            priv.bitrate_update_flag = 1;
            DEBUG_PRT("select bitrate: %s (%d)\n", text, priv.bitrate);
        } else {
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        }
    }
}


void event_touch_select_timelapse_cb(lv_event_t * e)
{
    DEBUG_EN(1);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            for (size_t i = 0; i < lv_obj_get_child_count(parent); i++) {
                lv_obj_t *child = lv_obj_get_child(parent, i);
                if (child != obj) {
                    lv_obj_remove_state(child, LV_STATE_CHECKED);
                }
            }

            lv_obj_t *label = lv_obj_get_child(obj, -1);
            char *text = lv_label_get_text(label);
            lv_obj_set_user_data(parent, text);
            LV_UNUSED(text);

            lv_obj_t *left_label = lv_obj_get_child(g_timelapse_button, -1);

            if (!strcmp("OFF", text)) {
                priv.timelapse_s = 0;
                lv_label_set_text(left_label, "OFF");
            } else if (!strcmp("AUTO", text)) {
                priv.timelapse_s = -1;
                lv_label_set_text(left_label, "AUTO");
            } else {
                priv.timelapse_s = atoi(text);
                lv_label_set_text_fmt(left_label, "%d s", priv.timelapse_s);
            }

            priv.timelapse_update_flag = 1;
            DEBUG_PRT("select timelapse s: %s (%d)\n", text, priv.timelapse_s);
        } else {
            lv_obj_add_state(obj, LV_STATE_CHECKED);
        }
    }
}

bool ui_get_cam_snap_flag(void)
{
    bool ret = priv.camera_snap_start_flag;
    priv.camera_snap_start_flag = 0;
    return ret;
}

bool ui_get_cam_video_start_flag(void)
{
    bool ret = priv.camera_video_start_flag;
    priv.camera_video_start_flag = 0;
    return ret;
}

bool ui_get_cam_video_stop_flag(void)
{
    bool ret = priv.camera_video_stop_flag;
    priv.camera_video_stop_flag = 0;
    return ret;
}

bool ui_get_cam_video_try_stop_flag(void)
{
    bool ret = priv.camera_video_try_stop_flag;
    priv.camera_video_try_stop_flag = 0;
    return ret;
}

bool ui_get_view_photo_flag(void)
{
    bool ret = priv.view_photo_flag;
    priv.view_photo_flag = 0;
    return ret;
}

bool ui_get_exit_flag(void)
{
    bool ret = priv.exit_flag;
    priv.exit_flag = 0;
    return ret;
}

bool ui_get_delay_setting_flag(void)
{
    bool ret = priv.delay_setting_flag;
    priv.delay_setting_flag = 0;
    return ret;
}

bool ui_get_resolution_setting_flag(void)
{
    bool ret = priv.resolution_setting_flag;
    priv.resolution_setting_flag = 0;
    return ret;
}

int ui_get_resolution_setting_idx(void)
{
    return priv.resolution_setting_idx;
}

bool ui_get_shutter_setting_flag(void)
{
    bool ret = priv.shutter_setting_flag;
    priv.shutter_setting_flag = 0;
    return ret;
}

bool ui_get_iso_setting_flag(void)
{
    bool ret = priv.iso_setting_flag;
    priv.iso_setting_flag = 0;
    return ret;
}

bool ui_get_ev_setting_flag(void)
{
    bool ret = priv.ev_setting_flag;
    priv.ev_setting_flag = 0;
    return ret;
}

bool ui_get_wb_setting_flag(void)
{
    bool ret = priv.wb_setting_flag;
    priv.wb_setting_flag = 0;
    return ret;
}

bool ui_get_wb_auto_flag(void)
{
    return priv.wb_auto_flag;
}

bool ui_get_shutter_auto_flag(void)
{
    return priv.shutter_auto_flag;
}

bool ui_get_iso_auto_flag(void)
{
    return priv.iso_auto_flag;
}

bool ui_get_ev_auto_flag(void)
{
    return priv.ev_auto_flag;
}

int ui_get_photo_delay(int *delay_ms)
{
    if (!delay_ms) return -1;
    char *text = (char *)lv_obj_get_user_data(g_delay_setting);
    char str[20];
    if (!strcmp(text, "off")) {
        *delay_ms = 0;
    } else {
        strcpy(str, text);
        str[strlen(text) - 1] = '\0';
        *delay_ms = atoi(str) * 1000;
    }

    return 0;
}

int ui_get_resulution(int *w, int *h)
{
    if (!w || !h) return -1;
    char *text = (char *)lv_obj_get_user_data(g_resolution_setting);
    if (!strcmp(text, "3.7MP(16:9)")) {
        *w = 2560;
        *h = 1440;
    } else if (!strcmp(text, "2MP(16:9)")) {
        *w = 1920;
        *h = 1080;
    } else if (!strcmp(text, "0.9MP(16:9)")) {
        *w = 1280;
        *h = 720;
    } else if (!strcmp(text, "0.3MP(4:3)")) {
        *w = 640;
        *h = 480;
    } else {
        printf("resolution not support: %s\n", text);
    }
    return 0;
}

int ui_get_shutter_value(double *value)
{
    if (!value) return -1;
    int *bar_val = (int *)lv_obj_get_user_data(g_shutter_setting);
    double val = *bar_val;
    *value = val;
    return 0;
}

int ui_get_iso_value(int *value)
{
    if (!value) return -1;
    int *bar_val = (int *)lv_obj_get_user_data(g_iso_setting);
    *value = *bar_val;
    return 0;
}

int ui_get_ev_value(double *value)
{
    if (!value) return -1;
    int *bar_val = (int *)lv_obj_get_user_data(g_ev_setting);
    *value = *bar_val;
    return 0;
}


int ui_get_wb_value(int *value)
{
    if (!value) return -1;
    int *bar_val = (int *)lv_obj_get_user_data(g_wb_setting);
    *value = *bar_val;
    return 0;
}


// shutter value, unit: us
void ui_set_shutter_value(double val)
{
    DEBUG_EN(0);
    DEBUG_PRT("input value: %lf us", val);
    if (val >= 0) {
        char str[128];
        double exptime_us = val;
        DEBUG_PRT("caculated exposure time: %lf us", exptime_us);
        lv_obj_t * shutter_setting_label = lv_obj_get_child(g_shutter_setting, 1);
        lv_obj_t * shutter_setting_bar = lv_obj_get_child(g_shutter_setting, 2);
        ui_camera_config_t camera_config;
        ui_camera_config_read(&camera_config);
        int index_of_table = get_index_of_exptime_table(exptime_us);
        exptime_us = camera_config.exposure_time_table[index_of_table] * 1000000;
        if (exptime_us < 0)
            snprintf(str, sizeof(str), "auto");
        else if (exptime_us == 1500000.0)
            snprintf(str, sizeof(str), "3/2s");
        else if (exptime_us >= 666666.0 && exptime_us <= 666667.0)
            snprintf(str, sizeof(str), "2/3s");
        else if (exptime_us < 1000000.0)
            snprintf(str, sizeof(str), "1/%ds", (int)((1000000.0 / exptime_us) + 0.5));
        else
            snprintf(str, sizeof(str), "%ds", (int)(exptime_us/1000000.0));
        lv_label_set_text(shutter_setting_label, str);
        DEBUG_PRT("caculated exposure time: %lf us", exptime_us);
        uint16_t bar_val = exp_us_to_bar_value(shutter_setting_bar, exptime_us);
        DEBUG_PRT("caculated bar val: %d", bar_val);
        lv_bar_set_value(shutter_setting_bar, bar_val, LV_ANIM_OFF);
        if (val > 0) { // update last val
            int *bar_last_val = (int *)lv_obj_get_user_data(g_shutter_setting);
            *bar_last_val = val;
        }

        lv_obj_t * menu_setting_child = lv_obj_get_child(g_menu_setting, 0);
        lv_obj_t *menu_setting_child_label = lv_obj_get_child(menu_setting_child, -1);
        lv_obj_t *menu_setting_child_label2 = lv_obj_get_child(menu_setting_child_label, -1);
        lv_label_set_text(menu_setting_child_label2, str);
    } else {
        char *str = "auto";
        lv_obj_t * shutter_setting_label = lv_obj_get_child(g_shutter_setting, 1);
        lv_obj_t * shutter_setting_bar = lv_obj_get_child(g_shutter_setting, 2);
        lv_label_set_text(shutter_setting_label, str);
        lv_bar_set_value(shutter_setting_bar, 0, LV_ANIM_OFF);

        lv_obj_t * menu_setting_child = lv_obj_get_child(g_menu_setting, 0);
        lv_obj_t *menu_setting_child_label = lv_obj_get_child(menu_setting_child, -1);
        lv_obj_t *menu_setting_child_label2 = lv_obj_get_child(menu_setting_child_label, -1);
        lv_label_set_text(menu_setting_child_label2, str);
    }
}

// iso = 100~800
void ui_set_iso_value(int val)
{
    DEBUG_EN(0);
    DEBUG_PRT("input value: %d", val);
    if (val >= 0)
    {
        int iso = val;
        DEBUG_PRT("caculated iso:%d", iso);
        ui_camera_config_t camera_config;
        ui_camera_config_read(&camera_config);
        int index_of_table = get_index_of_iso_table(iso);
        iso = camera_config.iso_table[index_of_table];
        DEBUG_PRT("caculated iso:%d", iso);
        {
            lv_obj_t * obj = lv_obj_get_child(g_menu_setting, 1);
            lv_obj_t *label = lv_obj_get_child(obj, -1);
            lv_obj_t *label2 = lv_obj_get_child(label, -1);
            lv_label_set_text_fmt(label2, "%d", iso);
        }

        {
            lv_obj_t *obj = g_iso_setting;
            lv_obj_t * label = lv_obj_get_child(obj, 1);
            lv_label_set_text_fmt(label, "%d", iso);

            lv_obj_t * bar = lv_obj_get_child(obj, 2);
            uint16_t bar_val = iso_to_bar_value(bar, iso);
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
            DEBUG_PRT("caculated bar_val:%d", bar_val);
            int *bar_last_val = (int *)lv_obj_get_user_data(obj);
            *bar_last_val = iso;
        }
    } else {
        char *str = "auto";
        {
            lv_obj_t * obj = lv_obj_get_child(g_menu_setting, 1);
            lv_obj_t *label = lv_obj_get_child(obj, -1);
            lv_obj_t *label2 = lv_obj_get_child(label, -1);
            lv_label_set_text_fmt(label2, str);
        }

        {
            lv_obj_t *obj = g_iso_setting;
            lv_obj_t * label = lv_obj_get_child(obj, 1);
            lv_label_set_text_fmt(label, str);

            lv_obj_t * bar = lv_obj_get_child(obj, 2);
            lv_bar_set_value(bar, 0, LV_ANIM_OFF);
        }
    }
}

// ev = -400~400
void ui_set_ev_value(int val)
{
    if (val < -400 || val > 400) {
        val = -1000;
    }
    char str[50];
    {
        lv_obj_t * obj = lv_obj_get_child(g_menu_setting, 2);
        lv_obj_t *label = lv_obj_get_child(obj, -1);
        lv_obj_t *label2 = lv_obj_get_child(label, -1);

        if (val == -1000) {
            lv_label_set_text_fmt(label2, "auto");
        } else {
            double f_val = (double)val / 100;
            if (val >= 0) {
                snprintf(str, sizeof(str), "+%.2f", f_val);
                lv_label_set_text(label2, str);
            } else {
                snprintf(str, sizeof(str), "-%.2f", -f_val);
                lv_label_set_text(label2, str);
            }
        }
    }

    {
        lv_obj_t * obj = g_ev_setting;
        lv_obj_t * label = lv_obj_get_child(obj, 1);
        if (val == -1000) {
            lv_label_set_text_fmt(label, "auto");
        } else {
            double f_val = (double)val / 100;
            if (val >= 0) {
                snprintf(str, sizeof(str), "+%.2f", f_val);
                lv_label_set_text(label, str);
            } else {
                snprintf(str, sizeof(str), "-%.2f", -f_val);
                lv_label_set_text(label, str);
            }
        }

        lv_obj_t * bar = lv_obj_get_child(obj, 2);
        int bar_val;
        if (val == -1000) {
            bar_val = lv_bar_get_min_value(bar);
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        } else {
            bar_val = val;
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        }

        if (val != -1000) {
            int *bar_last_val = (int *)lv_obj_get_user_data(obj);
            *bar_last_val = bar_val;
        }
    }
}

// wb = 1000K~10000K
void ui_set_wb_value(int val)
{
    if (val < 0) {
        val = -1;
    } else {
        val = val < 1000 ? 1000 : val;
        val = val > 10000 ? 10000 : val;
    }

    {
        lv_obj_t * obj = lv_obj_get_child(g_menu_setting, 3);
        lv_obj_t *label = lv_obj_get_child(obj, -1);
        lv_obj_t *label2 = lv_obj_get_child(label, -1);

        if (val == -1) {
            lv_label_set_text_fmt(label2, "auto");
        } else {
            lv_label_set_text_fmt(label2, "%dK", val);
        }
    }

    {
        lv_obj_t * obj = g_wb_setting;
        lv_obj_t * label = lv_obj_get_child(obj, 1);
        if (val == -1) {
            lv_label_set_text_fmt(label, "auto");
        } else {
            lv_label_set_text_fmt(label, "%dK", val);
        }

        lv_obj_t * bar = lv_obj_get_child(obj, 2);
        int bar_val;
        if (val == -1) {
            bar_val = lv_bar_get_min_value(bar);
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        } else {
            bar_val = val;
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        }

        if (val != -1) {
            int *bar_last_val = (int *)lv_obj_get_user_data(obj);
            *bar_last_val = bar_val;
        }
    }
}

void ui_update_small_img(void *data, int data_size)
{
    lv_obj_t *img = lv_obj_get_child(g_small_img, -1);
    lv_image_dsc_t *img_dsc = (lv_image_dsc_t *)lv_image_get_src(img);
    if (!data || !img_dsc)   return;

    if (img_dsc->data_size != data_size) {
        printf("img_dsc->data_size(%d) != data_size(%d)\n", img_dsc->data_size, data_size);
        printf("you need input ARGB8888 image, width: %d height: %d\n", img_dsc->header.w, img_dsc->header.h);
        return;
    }

    memcpy((void *)img_dsc->data, data, data_size);
    lv_obj_update_layout(g_small_img);
}

void ui_update_big_img(void *data, int data_size)
{
    lv_obj_t *img = lv_obj_get_child(g_big_img, -1);
    lv_image_dsc_t *img_dsc = (lv_image_dsc_t *)lv_image_get_src(img);
    if (!data || !img_dsc)   return;

    if (img_dsc->data_size != data_size) {
        printf("img_dsc->data_size(%d) != data_size(%d)\n", img_dsc->data_size, data_size);
        printf("you need input ARGB8888 image, width: %d height: %d\n", img_dsc->header.w, img_dsc->header.h);
        return;
    }

    memcpy((void *)img_dsc->data, data, data_size);
    lv_obj_update_layout(g_big_img);
}

static void _save_img_anim_completed_cb(lv_anim_t *anim)
{
    lv_obj_delete(anim->var);
}

static void _save_img_anim_set_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void _save_img_anim_set_y_cb(void * var, int32_t v)
{
    lv_obj_set_y(var, v);
}

static void _save_img_anim_set_size_cb(void * var, int32_t v)
{
    lv_obj_set_size(var, v, v);
}

void ui_anim_run_save_img(void)
{
    lv_obj_t * obj = lv_obj_create(lv_screen_active());
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_opa(obj, LV_OPA_20, 0);

    lv_obj_set_pos(obj, lv_pct(10), 0);
    lv_obj_set_size(obj, lv_pct(70), lv_pct(80));
    lv_obj_center(obj);

    lv_obj_t * dst_obj = g_small_img;
    int w = lv_obj_get_width(dst_obj);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_duration(&a, 300);

    lv_anim_set_values(&a, 358, w);
    lv_anim_set_exec_cb(&a, _save_img_anim_set_size_cb);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, _save_img_anim_set_x_cb);
    lv_anim_set_values(&a, lv_obj_get_x(obj), 235);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, _save_img_anim_set_y_cb);
    lv_anim_set_values(&a, lv_obj_get_y(obj), 105);
    lv_anim_set_completed_cb(&a, _save_img_anim_completed_cb);
    lv_anim_start(&a);
}

static void _anim_photo_delay_update_cb(void * var, int32_t v)
{
    lv_obj_t *label = lv_obj_get_child((lv_obj_t *)var, -1);
    lv_label_set_text_fmt(label, "%d", (v - 1) / 1000);
}

static void _anim_photo_delay_exit_cb(lv_anim_t *anim)
{
    lv_obj_delete(anim->var);
    priv.photo_delay_anim_start_flag = 0;
    priv.photo_delay_anim_stop_flag = 1;
}

void ui_anim_photo_delay_start(int delay_s)
{
    if (priv.photo_delay_anim_start_flag) {
        return;
    }
    priv.photo_delay_anim_stop_flag = 0;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scr, lv_pct(20), lv_pct(20));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(scr);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "0");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_opa(label, LV_OPA_70, 0);
    lv_obj_center(label);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, scr);
    lv_anim_set_values(&a, (delay_s + 1) * 1000, 1000);
    lv_anim_set_duration(&a, delay_s * 1000);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_exec_cb(&a, _anim_photo_delay_update_cb);
    lv_anim_set_completed_cb(&a, _anim_photo_delay_exit_cb);
    lv_anim_start(&a);

    priv.photo_delay_anim_start_flag = 1;
}

bool ui_get_photo_delay_anim_status(void)
{
    return priv.photo_delay_anim_start_flag;
}

bool ui_get_photo_delay_anim_stop_flag(void)
{
    bool flag = priv.photo_delay_anim_stop_flag;
    priv.photo_delay_anim_stop_flag = false;
    return flag;
}

void ui_set_record_time(uint64_t ms)
{
    int ms_per_second = 1000;
    int ms_per_minute = 60 * ms_per_second;
    int ms_per_hour = 60 * ms_per_minute;

    int hours = ms / ms_per_hour;
    ms %= ms_per_hour;
    int minutes = ms / ms_per_minute;
    ms %= ms_per_minute;
    int seconds = ms / ms_per_second;
    ms %= ms_per_second;

    char str[20];
    snprintf(str, sizeof(str), "%.2d:%.2d:%.2d", hours, minutes, seconds);

    lv_obj_t *label = lv_obj_get_child(g_video_running_screen, -1);
    lv_label_set_text(label, str);
}

bool ui_get_focus_btn_update_flag() {
    bool flag = priv.focus_btn_update_flag ? true : false;
    priv.focus_btn_update_flag = false;
    return flag;
}

bool ui_get_focus_btn_touched() {
    return priv.focus_btn_touched ? true : false;
}

bool ui_get_raw_btn_update_flag() {
    bool flag = priv.raw_btn_update_flag ? true : false;
    priv.raw_btn_update_flag = false;
    return flag;
}

bool ui_get_raw_btn_touched() {
    return priv.raw_btn_touched ? true : false;
}

bool ui_get_light_btn_update_flag() {
    bool flag = priv.light_btn_update_flag ? true : false;
    priv.light_btn_update_flag = false;
    return flag;
}

bool ui_get_light_btn_touched() {
    return priv.light_btn_touched ? true : false;
}

bool ui_get_bitrate_update_flag() {
    bool flag = priv.bitrate_update_flag ? true : false;
    priv.bitrate_update_flag = false;
    return flag;
}

int ui_get_bitrate() {
    return priv.bitrate;
}

void ui_set_bitrate(int bitrate, bool need_update)
{
    DEBUG_EN(0);
    lv_obj_t *scr = g_bitrate_setting;
    int child_num_of_scr = lv_obj_get_child_count(scr);
    int child_bitrate = 0;
    // find the index of the bitrate
    size_t i = 0;
    for (;i < child_num_of_scr; i++) {
        lv_obj_t *child = lv_obj_get_child(scr, i);
        if (child) {
            lv_obj_t *label = lv_obj_get_child(child, -1);
            if (label) {
                int value = atoi(lv_label_get_text(label)) * 1000;
                if (value >= bitrate) {
                    child_bitrate = value;
                    break;
                }
            }
        }
    }

    // clear all of checked flag
    for (size_t i = 0; i < child_num_of_scr; i++) {
        lv_obj_t *child = lv_obj_get_child(scr, i);
        if (child) {
            lv_obj_remove_state(child, LV_STATE_CHECKED);
        }
    }

    // set bitrate
    i = i >= child_num_of_scr ? child_num_of_scr - 1 : i;
    DEBUG_PRT("select child index: %ld", i);

    lv_obj_t *child = lv_obj_get_child(scr, i);
    if (child) {
        lv_obj_t *label = lv_obj_get_child(child, -1);
        if (label) {
            lv_obj_add_state(child, LV_STATE_CHECKED);
            priv.bitrate = child_bitrate;
            priv.bitrate_update_flag = need_update;
        }
    }
}

bool ui_get_timestamp_btn_update_flag() {
    bool flag = priv.timestamp_btn_update_flag ? true : false;
    priv.timestamp_btn_update_flag = false;
    return flag;
}

bool ui_get_timestamp_btn_touched() {
    return priv.timestamp_btn_touched ? true : false;
}


bool ui_get_timelapse_update_flag() {
    bool flag = priv.timelapse_update_flag ? true : false;
    priv.timelapse_update_flag = false;
    return flag;
}

int ui_get_timelapse_s() {
    return priv.timelapse_s;
}

void ui_set_timelapse_s(int timelapse, bool need_update)
{
    DEBUG_EN(1);
    lv_obj_t *scr = g_timelapse_setting;
    int child_num_of_scr = lv_obj_get_child_count(scr);
    int child_timelapse_s = 0;
    // find the index of the timelapse
    size_t idx = 0;
    if (timelapse == 0) {
        idx = 0;
    } else if (timelapse < 0) {
        idx = child_num_of_scr - 1;
    } else {
        idx = 1;
        for (;idx < child_num_of_scr - 1; idx++) {
            lv_obj_t *child = lv_obj_get_child(scr, idx);
            if (child) {
                lv_obj_t *label = lv_obj_get_child(child, -1);
                if (label) {
                    int value = atoi(lv_label_get_text(label));
                    if (value >= timelapse) {
                        break;
                    }
                }
            }
        }
    }

    // clear all of checked flag
    for (size_t i = 0; i < child_num_of_scr; i++) {
        lv_obj_t *child = lv_obj_get_child(scr, i);
        if (child) {
            lv_obj_remove_state(child, LV_STATE_CHECKED);
        }
    }

    // set timelapse
    idx = idx >= child_num_of_scr ? child_num_of_scr - 1 : idx;
    if (idx == child_num_of_scr - 1) {
        child_timelapse_s = -1;
    } else {
        child_timelapse_s = atoi(lv_label_get_text(lv_obj_get_child(lv_obj_get_child(scr, idx), -1)));
    }
    DEBUG_PRT("select child index: %ld  timelapse s:%d", idx, child_timelapse_s);

    lv_obj_t *child = lv_obj_get_child(scr, idx);
    if (child) {
        lv_obj_t *label = lv_obj_get_child(child, -1);
        if (label) {
            lv_obj_add_state(child, LV_STATE_CHECKED);
            priv.timelapse_s = child_timelapse_s;
            priv.timelapse_update_flag = need_update;
        }
    }

    DEBUG_PRT("set timelapse update flag");
    lv_obj_t *left_label = lv_obj_get_child(g_timelapse_button, -1);
    if (left_label) {
        if (child_timelapse_s == 0) {
            lv_label_set_text(left_label, "OFF");
        } else if (child_timelapse_s == -1) {
            lv_label_set_text(left_label, "AUTO");
        } else {
            lv_label_set_text_fmt(left_label, "%d s", child_timelapse_s);
        }
    }
    DEBUG_PRT("config label of timelapse button");
}