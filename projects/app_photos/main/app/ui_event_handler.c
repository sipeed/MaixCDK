#include "stdio.h"
#include "lvgl.h"
#include "ui_screen.h"
#include "ui_utils.h"
#include "ui_event_handler.h"

#define DEBUG_EN
#ifdef DEBUG_EN
#define DEBUG_PRT(fmt, ...) printf("[ui event]: "fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRT(fmt, ...)
#endif

extern lv_obj_t *g_base_screen;
extern lv_obj_t *g_lower_screen;
extern lv_obj_t *g_upper_screen;
extern lv_obj_t *g_right_screen;
extern lv_obj_t *g_big_photo_screen;
extern lv_obj_t *g_small_photo_screen;
extern lv_obj_t *g_switch_left;
extern lv_obj_t *g_switch_right;

static struct {
    unsigned int exit_flag : 1;
    unsigned int bulk_delete_flag : 1;
    unsigned int touch_small_image : 1;
    unsigned int touch_bulk_delete : 1;
    unsigned int touch_bulk_delete_cancel : 1;
    unsigned int touch_delete_big_photo : 1;
    unsigned int touch_show_big_photo_info : 1;
    unsigned int touch_show_left_big_photo : 1;
    unsigned int touch_show_right_big_photo : 1;

    char *touch_small_image_dir_name;
    char *touch_small_image_photo_path;
} priv;

void event_touch_exit_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        DEBUG_PRT("try to exit\n");
        if (ui_get_view_flag()) {
            ui_set_view_flag(0);
        } else {
            priv.exit_flag = 1;
        }
    }
}

void event_touch_small_image_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    static uint8_t ignore_after_long_press = 0;
    if (code == LV_EVENT_SHORT_CLICKED) {
        if (ignore_after_long_press) {
            ignore_after_long_press = 0;
            return;
        }
        ui_photo_t *data = (ui_photo_t *)lv_event_get_user_data(e);
        if (data) {
            DEBUG_PRT("short click path:%s\n", data->path);
            if (ui_get_need_bulk_delete()) {
                data->is_touch ^= 1;
                ui_photo_list_screen_update();
            } else {
                priv.touch_small_image = 1;
                priv.touch_small_image_photo_path = data->path;
                priv.touch_small_image_dir_name = (char *)lv_obj_get_user_data(lv_event_get_target(e));
            }
        } else {
            DEBUG_PRT("unknow error!\r\n");
        }
    } else if (code == LV_EVENT_LONG_PRESSED) {
        ui_photo_t *data = (ui_photo_t *)lv_event_get_user_data(e);
        if (data) {
            DEBUG_PRT("long pressed path:%s\n", data->path);
            ignore_after_long_press = 1;
            ui_set_need_bulk_delete(1);
            ui_photo_list_screen_update();
            if (g_lower_screen) {
                lv_obj_remove_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            DEBUG_PRT("unknow error!\r\n");
        }
    }
}

void event_touch_big_image_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED) {
        ui_big_photo_info_t *data = (ui_big_photo_info_t *)lv_event_get_user_data(e);
        if (data) {
            DEBUG_PRT("short click path:%s\n", data->path);
            if (g_upper_screen) {
                if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_remove_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_add_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN);
                }
            }

            if (g_lower_screen) {
                if (lv_obj_has_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_remove_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_add_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN);
                }
            }

            if (g_switch_left) {
                if (g_upper_screen) {
                    if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN)) {
                        lv_obj_add_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        lv_obj_remove_flag(g_switch_left, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }

            if (g_switch_right) {
                if (g_upper_screen) {
                    if (lv_obj_has_flag(g_upper_screen, LV_OBJ_FLAG_HIDDEN)) {
                        lv_obj_add_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        lv_obj_remove_flag(g_switch_right, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }
        } else {
            DEBUG_PRT("unknow error!\r\n");
        }
    }
}

void event_touch_show_left_big_photo_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED) {
        DEBUG_PRT("touch left\n");
        priv.touch_show_left_big_photo = 1;
    } else {
        DEBUG_PRT("unknow error!\r\n");
    }
}

void event_touch_show_right_big_photo_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED) {
        DEBUG_PRT("touch right\n");
        priv.touch_show_right_big_photo = 1;
    } else {
        DEBUG_PRT("unknow error!\r\n");
    }
}

void event_touch_delete_big_photo_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("touch delete big photo\n");
        priv.touch_delete_big_photo = 1;
    }
}

void event_touch_show_big_photo_info_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("touch show big photo info\n");
        priv.touch_show_big_photo_info = 1;
        if (g_right_screen) {
            if (lv_obj_has_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN)) {
                ui_right_screen_update();
                lv_obj_remove_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_right_screen, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

void event_touch_bulk_delete_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("touch bulk delete\n");
        priv.touch_bulk_delete = 1;
    }
}

void event_touch_bulk_delete_cancel_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("touch bulk delete cancel\n");
        priv.touch_bulk_delete_cancel = 1;
        ui_set_need_bulk_delete(0);
        ui_photo_list_screen_update();
        ui_photo_clear_all_photo_flag();
        if (g_lower_screen) {
            lv_obj_add_flag(g_lower_screen, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

bool ui_get_exit_flag(void)
{
    bool ret = priv.exit_flag;
    priv.exit_flag = 0;
    return ret;
}

bool ui_get_touch_small_image_flag(void)
{
    bool ret = priv.touch_small_image;
    priv.touch_small_image = 0;
    return ret;
}

void ui_get_touch_small_image_path(char **dir_name, char **img_path)
{
    if (dir_name) {
        *dir_name = priv.touch_small_image_dir_name;
    }

    if (img_path) {
        *img_path = priv.touch_small_image_photo_path;
    }
}

bool ui_get_bulk_delete_flag(void)
{
    bool ret = priv.touch_bulk_delete;
    priv.touch_bulk_delete = 0;
    return ret;
}

bool ui_get_bulk_delete_cancel_flag(void)
{
    bool ret = priv.touch_bulk_delete_cancel;
    priv.touch_bulk_delete_cancel = 0;
    return ret;
}

bool ui_get_show_big_photo_info_flag(void)
{
    bool ret = priv.touch_show_big_photo_info;
    priv.touch_show_big_photo_info = 0;
    return ret;
}

bool ui_get_delete_big_photo_flag(void)
{
    bool ret = priv.touch_delete_big_photo;
    priv.touch_delete_big_photo = 0;
    return ret;
}

char *ui_get_delete_big_photo_path(void)
{
    ui_big_photo_info_t *info = (ui_big_photo_info_t *)lv_obj_get_user_data(g_big_photo_screen);
    return info->path;
}

bool ui_get_touch_show_right_big_photo_flag(void)
{
    bool ret = priv.touch_show_right_big_photo;
    priv.touch_show_right_big_photo = 0;
    return ret;
}

bool ui_get_touch_show_left_big_photo_flag(void)
{
    bool ret = priv.touch_show_left_big_photo;
    priv.touch_show_left_big_photo = 0;
    return ret;
}

