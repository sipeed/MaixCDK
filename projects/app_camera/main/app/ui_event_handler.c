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

extern lv_obj_t *g_camera_video_button;
extern lv_obj_t *g_start_snap_button;
extern lv_obj_t *g_exit_button;
extern lv_obj_t *g_delay_button;
extern lv_obj_t *g_resolution_button;
extern lv_obj_t *g_menu_button;
extern lv_obj_t *g_delay_setting;
extern lv_obj_t *g_resolution_setting;
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
} priv;

void event_touch_exit_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("try to exit\n");
        priv.exit_flag = 1;
    }
}

void event_touch_delay_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_delay_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_delay_setting, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(g_resolution_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_resolution_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_resolution_button, LV_EVENT_RELEASED, NULL);
            }
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

            lv_obj_add_flag(g_delay_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_delay_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_delay_button, LV_EVENT_RELEASED, NULL);
            }
        } else {
            lv_obj_add_flag(g_resolution_setting, LV_OBJ_FLAG_HIDDEN);
        }
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

            lv_obj_add_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_shutter_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_shutter_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);
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

void event_touch_video_camera_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_has_flag(g_start_snap_button, LV_OBJ_FLAG_CHECKABLE)) {
            if (lv_obj_get_state(g_start_snap_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_start_snap_button, LV_EVENT_RELEASED, NULL);
                lv_obj_add_flag(g_video_running_screen, LV_OBJ_FLAG_HIDDEN);
                DEBUG_PRT("video try stop!\n");

                priv.camera_video_try_stop_flag = 1;
            }
            lv_obj_remove_flag(g_start_snap_button, LV_OBJ_FLAG_CHECKABLE);
        } else {
            lv_obj_add_flag(g_start_snap_button, LV_OBJ_FLAG_CHECKABLE);
        }
    }
}

void event_touch_start_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_camera_video_button) == LV_STATE_CHECKED) {
            if (lv_obj_get_state(g_start_snap_button) == LV_STATE_FOCUSED) {
                lv_obj_add_flag(g_video_running_screen, LV_OBJ_FLAG_HIDDEN);
                DEBUG_PRT("video stop\n");
                priv.camera_video_stop_flag = 1;
            } else {
                lv_obj_remove_flag(g_video_running_screen, LV_OBJ_FLAG_HIDDEN);
                DEBUG_PRT("video start\n");
                priv.camera_video_start_flag = 1;
            }
        } else {
            DEBUG_PRT("camera snap start\n");
            priv.camera_snap_start_flag = 1;
        }
    }
}

void event_touch_small_img_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DEBUG_PRT("View big photo\n");
        lv_obj_remove_flag(g_big_img, LV_OBJ_FLAG_HIDDEN);
        priv.view_photo_flag = 1;
    }
}

void event_touch_big_img_cb(lv_event_t * e)
{
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
            lv_obj_remove_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);
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
            lv_obj_add_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_iso_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_iso_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);
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
            lv_obj_add_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_ev_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_ev_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_ev_setting, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_shutter_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_shutter_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);
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


void event_touch_wb_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(g_wb_button) != LV_STATE_FOCUSED) {
            lv_obj_remove_flag(g_wb_setting, LV_OBJ_FLAG_HIDDEN);

            lv_obj_add_flag(g_shutter_setting, LV_OBJ_FLAG_HIDDEN);
            if (lv_obj_get_state(g_shutter_button) == LV_STATE_CHECKED) {
                lv_obj_send_event(g_shutter_button, LV_EVENT_RELEASED, NULL);
            }
            lv_obj_add_flag(g_iso_setting, LV_OBJ_FLAG_HIDDEN);
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

        double shutter_value = 0;
        if (val > 0) {
            shutter_value = val / 1000;
        } else if (val < 0) {
            shutter_value = 1.0 / -val;
        } else {
            shutter_value = 0;
        }

        ui_set_shutter_value(shutter_value);

        priv.shutter_setting_flag = 1;
        DEBUG_PRT("shutter value updating: %f\n", shutter_value);
    } else if (code == LV_EVENT_RELEASED) {
        int val = lv_bar_get_value(obj);
        double shutter_value = 0;
        if (val > 0) {
            shutter_value = val / 1000;
        } else if (val < 0) {
            shutter_value = 1.0 / -val;
        } else {
            shutter_value = 0;
        }

        ui_set_shutter_value(shutter_value);

        priv.shutter_setting_flag = 1;
        DEBUG_PRT("shutter value update: %f\n", shutter_value);
    }
}

void event_iso_bar_update_cb(lv_event_t * e)
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

        int iso_value = 0;
        iso_value = (val / 100) * 100;

        ui_set_iso_value(iso_value);

        DEBUG_PRT("iso value updating: %d\n", iso_value);

        priv.iso_setting_flag = 1;
    } else if (code == LV_EVENT_RELEASED) {
        int val = lv_bar_get_value(obj);
        int iso_value = 0;
        iso_value = (val / 100) * 100;

        ui_set_iso_value(iso_value);

        DEBUG_PRT("iso value update: %d\n", iso_value);
        priv.iso_setting_flag = 1;
    }
}

void event_ev_bar_update_cb(lv_event_t * e)
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
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            DEBUG_PRT("set exposure time to auto mode\n");
            ui_set_shutter_value(-1);
            ui_set_iso_value(-1);

            priv.shutter_setting_flag = 1;
            priv.shutter_auto_flag = 1;
        } else {
            DEBUG_PRT("set exposure time to manual mode\n");
            {
                int *bar_val = (int *)lv_obj_get_user_data(g_shutter_setting);
                double val = *bar_val;
                double shutter_value = 0;
                if (val > 0) {
                    shutter_value = val / 1000;
                } else if (val < 0) {
                    shutter_value = 1.0 / -val;
                } else {
                    shutter_value = 0;
                }
                ui_set_shutter_value(shutter_value);
            }

            {
                int *bar_val = (int *)lv_obj_get_user_data(g_iso_setting);
                int val = *bar_val;
                ui_set_iso_value(val);
            }

            priv.shutter_setting_flag = 1;
            priv.shutter_auto_flag = 0;
        }
    }
}

void event_touch_iso_to_auto_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            DEBUG_PRT("set exposure time to auto mode\n");
            ui_set_shutter_value(-1);
            ui_set_iso_value(-1);

            priv.shutter_setting_flag = 1;
            priv.shutter_auto_flag = 1;
        } else {
            DEBUG_PRT("set exposure time to manual mode\n");
            {
                int *bar_val = (int *)lv_obj_get_user_data(g_shutter_setting);
                double val = *bar_val;
                double shutter_value = 0;
                if (val > 0) {
                    shutter_value = val / 1000;
                } else if (val < 0) {
                    shutter_value = 1.0 / -val;
                } else {
                    shutter_value = 0;
                }
                ui_set_shutter_value(shutter_value);
            }

            {
                int *bar_val = (int *)lv_obj_get_user_data(g_iso_setting);
                int val = *bar_val;
                ui_set_iso_value(val);
            }

            priv.shutter_setting_flag = 1;
            priv.shutter_auto_flag = 0;
        }
    }
}

void event_touch_ev_to_auto_cb(lv_event_t * e)
{
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
            DEBUG_PRT("select resolution: %s\n", text);

            priv.resolution_setting_flag = 1;
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

int ui_get_shutter_value(uint64_t *value)
{
    if (!value) return -1;
    int *bar_val = (int *)lv_obj_get_user_data(g_shutter_setting);
    double val = *bar_val;
    double shutter_value = 0;
    if (val > 0) {
        shutter_value = val / 1000;
    } else if (val < 0) {
        shutter_value = 1.0 / -val;
    } else {
        shutter_value = 0;
    }
    *value = shutter_value * 1000 * 1000;
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


// shutter = 1/10000s ~ 5s
void ui_set_shutter_value(double val)
{
    if (val < 0) {
        val = -1;
    } else {
        val = val < (1 / 10000) ? (1 / 10000) : val;
        val = val > 5 ? 5 : val;
    }

    {
        lv_obj_t * obj = lv_obj_get_child(g_menu_setting, 0);
        lv_obj_t *label = lv_obj_get_child(obj, -1);
        lv_obj_t *label2 = lv_obj_get_child(label, -1);

        if (val == -1)
            lv_label_set_text_fmt(label2, "auto");
        else if (val == 0)
            lv_label_set_text_fmt(label2, "0s");
        else if (val < 1)
            lv_label_set_text_fmt(label2, "1/%ds", (int)(1 / val));
        else
            lv_label_set_text_fmt(label2, "%ds", (int)val);
    }

    {
        lv_obj_t * obj = g_shutter_setting;
        lv_obj_t * label = lv_obj_get_child(obj, 1);
        if (val == -1)
            lv_label_set_text(label, "auto");
        else if (val == 0)
            lv_label_set_text_fmt(label, "0s");
        else if (val < 1)
            lv_label_set_text_fmt(label, "1/%ds", (int)(1 / val));
        else
            lv_label_set_text_fmt(label, "%ds", (int)val);

        lv_obj_t * bar = lv_obj_get_child(obj, 2);
        int bar_val;
        if (val == -1) {
            bar_val = lv_bar_get_min_value(bar);
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        } else if (val == 0) {
            bar_val = 0;
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        } else if (val < 1) {
            bar_val = -(1 / val);
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        } else {
            bar_val = val * 1000;
            lv_bar_set_value(bar, bar_val, LV_ANIM_OFF);
        }

        if (val != -1) {
            int *bar_last_val = (int *)lv_obj_get_user_data(obj);
            *bar_last_val = bar_val;
        }
    }

}

// iso = 100~800
void ui_set_iso_value(int val)
{
    if (val < 0) {
        val = -1;
    } else {
        val = val < 100 ? 100 : val;
        val = val > 800 ? 800 : val;
    }

    {
        lv_obj_t * obj = lv_obj_get_child(g_menu_setting, 1);
        lv_obj_t *label = lv_obj_get_child(obj, -1);
        lv_obj_t *label2 = lv_obj_get_child(label, -1);

        if (val == -1)
            lv_label_set_text_fmt(label2, "auto");
        else
            lv_label_set_text_fmt(label2, "%d", val);
    }

    {
        lv_obj_t *obj = g_iso_setting;
        lv_obj_t * label = lv_obj_get_child(obj, 1);
        if (val == -1)
            lv_label_set_text_fmt(label, "auto");
        else
            lv_label_set_text_fmt(label, "%d", val);

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
