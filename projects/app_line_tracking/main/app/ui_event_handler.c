#include "stdio.h"
#include "lvgl.h"
#include "ui_screen.h"
#include "ui_utils.h"
#include "ui_event_handler.h"

// #define DEBUG_EN
#ifdef DEBUG_EN
#define DEBUG_PRT(fmt, ...) printf("[ui event][%s][%d]: "fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRT(fmt, ...)
#endif

static struct {
    unsigned int exit_flag : 1;
    unsigned int eye_is_open_flag : 1;
    unsigned int eye_is_update_flag : 1;
    unsigned int lab_options_is_pressed_flag : 1;
    unsigned int red_btn_is_touch_flag : 1;
    unsigned int green_btn_is_touch_flag : 1;
    unsigned int blue_btn_is_touch_flag : 1;
    unsigned int user_btn_is_touch_flag : 1;
    unsigned int lmax_btn_is_touch_flag : 1;
    unsigned int lmin_btn_is_touch_flag : 1;
    unsigned int amax_btn_is_touch_flag : 1;
    unsigned int amin_btn_is_touch_flag : 1;
    unsigned int bmax_btn_is_touch_flag : 1;
    unsigned int bmin_btn_is_touch_flag : 1;

    unsigned int lmin_value_is_update_flag : 1;
    unsigned int lmax_value_is_update_flag : 1;
    unsigned int amin_value_is_update_flag : 1;
    unsigned int amax_value_is_update_flag : 1;
    unsigned int bmin_value_is_update_flag : 1;
    unsigned int bmax_value_is_update_flag : 1;

    unsigned int active_screen_is_touch : 1;
    int activate_screen_touch_x;
    int activate_screen_touch_y;
} priv;

extern lv_obj_t *ui_lab_options_screen;
extern lv_obj_t *ui_bar;
extern lv_obj_t *ui_user_btn;
extern lv_obj_t *ui_bar_screen;
extern lv_obj_t *ui_lmax_btn;
extern lv_obj_t *ui_lmin_btn;
extern lv_obj_t *ui_amax_btn;
extern lv_obj_t *ui_amin_btn;
extern lv_obj_t *ui_bmax_btn;
extern lv_obj_t *ui_bmin_btn;
extern lv_obj_t *ui_color_box_screen;
extern lv_obj_t *ui_color_box[3];
extern lv_obj_t *ui_draw_pointer[3];

void event_active_screen_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point);
        // DEBUG_PRT("point:(%d,%d)\n", point.x, point.y);
        priv.active_screen_is_touch = 1;
        priv.activate_screen_touch_x = point.x;
        priv.activate_screen_touch_y = point.y;

        ui_show_color_box(1);
    }
}

void event_touch_exit_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        priv.exit_flag = 1;
        DEBUG_PRT("exit handle!\n");
    }
}

void event_touch_eye_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            DEBUG_PRT("eye is open!\n");
            priv.eye_is_open_flag = 1;
            priv.eye_is_update_flag = 1;
        } else {
            DEBUG_PRT("eye is close!\n");
            priv.eye_is_open_flag = 0;
            priv.eye_is_update_flag = 1;
        }
    }
}

void event_touch_lab_options_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            DEBUG_PRT("lab options is focused!\n");
            lv_obj_remove_flag(ui_lab_options_screen, LV_OBJ_FLAG_HIDDEN);
            priv.lab_options_is_pressed_flag = 1;
        } else {
            lv_obj_add_flag(ui_lab_options_screen, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_color_box_screen, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
            DEBUG_PRT("lab options is not focused!\n");
            priv.lab_options_is_pressed_flag = 0;
        }
    }
}

void event_touch_lmin_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_remove_state(ui_lmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmin_btn, LV_STATE_CHECKED);

            priv.lmin_btn_is_touch_flag = 1;
            priv.lmax_btn_is_touch_flag = 0;
            priv.amin_btn_is_touch_flag = 0;
            priv.amax_btn_is_touch_flag = 0;
            priv.bmin_btn_is_touch_flag = 0;
            priv.bmax_btn_is_touch_flag = 0;

            int *user_data = lv_obj_get_user_data(obj);
            lv_obj_remove_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
            ui_set_bar_range(0, 100);
            ui_set_bar_value("L Min", *user_data);

            DEBUG_PRT("touch lmin btn! (%d)\n", *user_data);
        } else {
            lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_lmax_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_remove_state(ui_lmin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmin_btn, LV_STATE_CHECKED);

            DEBUG_PRT("touch lmax btn!\n");
            priv.lmax_btn_is_touch_flag = 1;
            priv.lmin_btn_is_touch_flag = 0;
            priv.amin_btn_is_touch_flag = 0;
            priv.amax_btn_is_touch_flag = 0;
            priv.bmin_btn_is_touch_flag = 0;
            priv.bmax_btn_is_touch_flag = 0;

            int *user_data = lv_obj_get_user_data(obj);
            lv_obj_remove_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
            ui_set_bar_range(0, 100);
            ui_set_bar_value("L Max", *user_data);
        } else {
            lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_amin_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_remove_state(ui_lmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_lmin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmin_btn, LV_STATE_CHECKED);

            DEBUG_PRT("touch amin btn!\n");
            priv.amin_btn_is_touch_flag = 1;
            priv.lmax_btn_is_touch_flag = 0;
            priv.lmin_btn_is_touch_flag = 0;
            priv.amax_btn_is_touch_flag = 0;
            priv.bmin_btn_is_touch_flag = 0;
            priv.bmax_btn_is_touch_flag = 0;

            int *user_data = lv_obj_get_user_data(obj);
            lv_obj_remove_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
            ui_set_bar_range(-128, 127);
            ui_set_bar_value("A Min", *user_data);
        } else {
            lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_amax_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_remove_state(ui_lmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_lmin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmin_btn, LV_STATE_CHECKED);

            DEBUG_PRT("touch amax btn!\n");
            priv.amax_btn_is_touch_flag = 1;
            priv.lmax_btn_is_touch_flag = 0;
            priv.amin_btn_is_touch_flag = 0;
            priv.lmin_btn_is_touch_flag = 0;
            priv.bmin_btn_is_touch_flag = 0;
            priv.bmax_btn_is_touch_flag = 0;

            int *user_data = lv_obj_get_user_data(obj);
            lv_obj_remove_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
            ui_set_bar_range(-128, 127);
            ui_set_bar_value("A Max", *user_data);
        } else {
            lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_bmin_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_remove_state(ui_lmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_lmin_btn, LV_STATE_CHECKED);

            DEBUG_PRT("touch bmin btn!\n");
            priv.lmin_btn_is_touch_flag = 0;
            priv.lmax_btn_is_touch_flag = 0;
            priv.amin_btn_is_touch_flag = 0;
            priv.amax_btn_is_touch_flag = 0;
            priv.bmin_btn_is_touch_flag = 1;
            priv.bmax_btn_is_touch_flag = 0;

            int *user_data = lv_obj_get_user_data(obj);
            lv_obj_remove_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
            ui_set_bar_range(-128, 127);
            ui_set_bar_value("B Min", *user_data);
        } else {
            lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_touch_bmax_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_get_state(obj) != LV_STATE_FOCUSED) {
            lv_obj_remove_state(ui_lmax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amax_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_amin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_lmin_btn, LV_STATE_CHECKED);
            lv_obj_remove_state(ui_bmin_btn, LV_STATE_CHECKED);

            DEBUG_PRT("touch bmax btn!\n");
            priv.lmin_btn_is_touch_flag = 0;
            priv.lmax_btn_is_touch_flag = 0;
            priv.amin_btn_is_touch_flag = 0;
            priv.amax_btn_is_touch_flag = 0;
            priv.bmin_btn_is_touch_flag = 0;
            priv.bmax_btn_is_touch_flag = 1;

            int *user_data = lv_obj_get_user_data(obj);
            lv_obj_remove_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
            ui_set_bar_range(-128, 127);
            ui_set_bar_value("B Max", *user_data);
        } else {
            lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void event_lab_bar_update_cb(lv_event_t * e)
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

        DEBUG_PRT("value updating: %d\n", val);

        if (priv.lmin_btn_is_touch_flag) {
            ui_set_lmin_value(val);
            ui_set_bar_value("L Min", val);
            priv.lmin_value_is_update_flag = 1;
        } else if (priv.lmax_btn_is_touch_flag) {
            ui_set_lmax_value(val);
            ui_set_bar_value("L Max", val);
            priv.lmax_value_is_update_flag = 1;
        } else if (priv.amin_btn_is_touch_flag) {
            ui_set_amin_value(val);
            ui_set_bar_value("A Min", val);
            priv.amin_value_is_update_flag = 1;
        } else if (priv.amax_btn_is_touch_flag) {
            ui_set_amax_value(val);
            ui_set_bar_value("A Max", val);
            priv.amax_value_is_update_flag = 1;
        } else if (priv.bmin_btn_is_touch_flag) {
            ui_set_bmin_value(val);
            ui_set_bar_value("B Min", val);
            priv.bmin_value_is_update_flag = 1;
        } else if (priv.bmax_btn_is_touch_flag) {
            ui_set_bmax_value(val);
            ui_set_bar_value("B Max", val);
            priv.bmax_value_is_update_flag = 1;
        }
    }
}

void event_touch_color_btn_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        ui_lab_t *lab = lv_obj_get_user_data(obj);
        int lmin = lab->l - 30 < 0 ? 0 : lab->l - 30;
        int lmax = lab->l + 30 > 100 ? 100 : lab->l + 30;
        int amin = lab->a - 10 < -128 ? -128 : lab->a - 10;
        int amax = lab->a + 10 > 127 ? 127 : lab->a + 10;
        int bmin = lab->b - 10 < -128 ? -128 : lab->b - 10;
        int bmax = lab->b + 10 > 127 ? 127 : lab->b + 10;
        ui_set_lmin_value(lmin);
        ui_set_lmax_value(lmax);
        ui_set_amin_value(amin);
        ui_set_amax_value(amax);
        ui_set_bmin_value(bmin);
        ui_set_bmax_value(bmax);

        lv_obj_remove_state(ui_lmin_btn, LV_STATE_CHECKED);
        lv_obj_remove_state(ui_lmax_btn, LV_STATE_CHECKED);
        lv_obj_remove_state(ui_amax_btn, LV_STATE_CHECKED);
        lv_obj_remove_state(ui_amin_btn, LV_STATE_CHECKED);
        lv_obj_remove_state(ui_bmax_btn, LV_STATE_CHECKED);
        lv_obj_remove_state(ui_bmin_btn, LV_STATE_CHECKED);
        lv_obj_add_flag(ui_bar_screen, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_set_bar_range(int min, int max)
{
    min = min < -128 ? -128 : min;
    max = max > 127 ? 127 : max;
    lv_bar_set_range(ui_bar, min, max);
}

void ui_set_bar_value(char *txt, int value)
{
    lv_obj_t *obj = ui_bar;
    lv_bar_set_value(obj, value, LV_ANIM_OFF);

    lv_obj_t *label1 = lv_obj_get_child(obj, 0);
    lv_label_set_text(label1, txt);

    lv_obj_t *label2 = lv_obj_get_child(obj, 1);
    lv_label_set_text_fmt(label2, "%d", value);
}

void ui_set_lmin_value(int value)
{
    priv.lmin_value_is_update_flag = 1;

    value = value < 0 ? 0 : value;
    value = value > 100 ? 100 : value;

    lv_obj_t *obj = ui_lmin_btn;
    lv_obj_t *label = lv_obj_get_child(obj, -1);
    lv_label_set_text_fmt(label, "L Min\n(%d)", value);
    int *user_data = lv_obj_get_user_data(obj);
    *user_data = value;

    if (priv.user_btn_is_touch_flag) {
        lv_obj_t *user = ui_user_btn;
        ui_lab_range_t *lab = (ui_lab_range_t *)lv_obj_get_user_data(user);
        lab->l_min = value;
    }
}

void ui_set_lmax_value(int value)
{
    priv.lmax_value_is_update_flag = 1;

    value = value < 0 ? 0 : value;
    value = value > 100 ? 100 : value;

    lv_obj_t *obj = ui_lmax_btn;
    lv_obj_t *label = lv_obj_get_child(obj, -1);
    lv_label_set_text_fmt(label, "L Max\n(%d)", value);
    int *user_data = lv_obj_get_user_data(obj);
    *user_data = value;

    if (priv.user_btn_is_touch_flag) {
        lv_obj_t *user = ui_user_btn;
        ui_lab_range_t *lab = (ui_lab_range_t *)lv_obj_get_user_data(user);
        lab->l_max = value;
    }
}

void ui_set_amin_value(int value)
{
    priv.amin_value_is_update_flag = 1;

    value = value < -128 ? -128 : value;
    value = value > 127 ? 127 : value;

    lv_obj_t *obj = ui_amin_btn;
    lv_obj_t *label = lv_obj_get_child(obj, -1);
    lv_label_set_text_fmt(label, "A Min\n(%d)", value);
    int *user_data = lv_obj_get_user_data(obj);
    *user_data = value;

    if (priv.user_btn_is_touch_flag) {
        lv_obj_t *user = ui_user_btn;
        ui_lab_range_t *lab = (ui_lab_range_t *)lv_obj_get_user_data(user);
        lab->a_min = value;
    }
}

void ui_set_amax_value(int value)
{
    priv.amax_value_is_update_flag = 1;

    value = value < -128 ? -128 : value;
    value = value > 127 ? 127 : value;

    lv_obj_t *obj = ui_amax_btn;
    lv_obj_t *label = lv_obj_get_child(obj, -1);
    lv_label_set_text_fmt(label, "A Max\n(%d)", value);
    int *user_data = lv_obj_get_user_data(obj);
    *user_data = value;

    if (priv.user_btn_is_touch_flag) {
        lv_obj_t *user = ui_user_btn;
        ui_lab_range_t *lab = (ui_lab_range_t *)lv_obj_get_user_data(user);
        lab->a_max = value;
    }
}

void ui_set_bmin_value(int value)
{
    priv.bmin_value_is_update_flag = 1;

    value = value < -128 ? -128 : value;
    value = value > 127 ? 127 : value;

    lv_obj_t *obj = ui_bmin_btn;
    lv_obj_t *label = lv_obj_get_child(obj, -1);
    lv_label_set_text_fmt(label, "B Min\n(%d)", value);
    int *user_data = lv_obj_get_user_data(obj);
    *user_data = value;

    if (priv.user_btn_is_touch_flag) {
        lv_obj_t *user = ui_user_btn;
        ui_lab_range_t *lab = (ui_lab_range_t *)lv_obj_get_user_data(user);
        lab->b_min = value;
    }
}

void ui_set_bmax_value(int value)
{
    priv.bmax_value_is_update_flag = 1;

    value = value < -128 ? -128 : value;
    value = value > 127 ? 127 : value;

    lv_obj_t *obj = ui_bmax_btn;
    lv_obj_t *label = lv_obj_get_child(obj, -1);
    lv_label_set_text_fmt(label, "B Max\n(%d)", value);
    int *user_data = lv_obj_get_user_data(obj);
    *user_data = value;

    if (priv.user_btn_is_touch_flag) {
        lv_obj_t *user = ui_user_btn;
        ui_lab_range_t *lab = (ui_lab_range_t *)lv_obj_get_user_data(user);
        lab->b_max = value;
    }
}

int ui_get_lmin_value(void)
{
    lv_obj_t *obj = ui_lmin_btn;
    int *user_data = lv_obj_get_user_data(obj);
    return *user_data;
}

int ui_get_lmax_value(void)
{
    lv_obj_t *obj = ui_lmax_btn;
    int *user_data = lv_obj_get_user_data(obj);
    return *user_data;
}

int ui_get_amin_value(void)
{
    lv_obj_t *obj = ui_amin_btn;
    int *user_data = lv_obj_get_user_data(obj);
    return *user_data;
}

int ui_get_amax_value(void)
{
    lv_obj_t *obj = ui_amax_btn;
    int *user_data = lv_obj_get_user_data(obj);
    return *user_data;
}

int ui_get_bmin_value(void)
{
    lv_obj_t *obj = ui_bmin_btn;
    int *user_data = lv_obj_get_user_data(obj);
    return *user_data;
}

int ui_get_bmax_value(void)
{
    lv_obj_t *obj = ui_bmax_btn;
    int *user_data = lv_obj_get_user_data(obj);
    return *user_data;
}

bool ui_lmin_update(void)
{
    bool flag = priv.lmin_value_is_update_flag;
    priv.lmin_value_is_update_flag = 0;
    return flag;
}

bool ui_lmax_update(void)
{
    bool flag = priv.lmax_value_is_update_flag;
    priv.lmax_value_is_update_flag = 0;
    return flag;
}

bool ui_amin_update(void)
{
    bool flag = priv.amin_value_is_update_flag;
    priv.amin_value_is_update_flag = 0;
    return flag;
}

bool ui_amax_update(void)
{
    bool flag = priv.amax_value_is_update_flag;
    priv.amax_value_is_update_flag = 0;
    return flag;
}

bool ui_bmin_update(void)
{
    bool flag = priv.bmin_value_is_update_flag;
    priv.bmin_value_is_update_flag = 0;
    return flag;
}

bool ui_bmax_update(void)
{
    bool flag = priv.bmax_value_is_update_flag;
    priv.bmax_value_is_update_flag = 0;
    return flag;
}

void ui_show_color_box(int en)
{
    if (en) {
        lv_obj_remove_flag(ui_color_box_screen, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ui_color_box_screen, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_set_color_of_color_box(int idx, int r, int g, int b)
{
    if (idx > 2) return;
    lv_obj_set_style_bg_color(ui_color_box[idx], lv_color_make(r, g, b), 0);
}

void ui_set_value_of_color_box(int idx, int l, int a, int b)
{
    if (idx > 2) return;
    lv_obj_t *obj = ui_color_box[idx];
    lv_obj_t *label = lv_obj_get_child(obj, -1);
    l = l > 100 ? 100 : l;
    l = l < 0 ? 0 : l;
    a = a > 127 ? 127 : a;
    a = a < -128 ? -128 : a;
    b = b > 127 ? 127 : b;
    b = b < -128 ? -128 : b;
    lv_label_set_text_fmt(label, "%.2d  %.2d  %.2d", l, a ,b);
    ui_lab_t *lab = lv_obj_get_user_data(obj);
    lab->l = l;
    lab->a = a;
    lab->b = b;
}

bool ui_get_exit_flag(void)
{
    bool exit_flag = priv.exit_flag;
    priv.exit_flag = 0;
    return exit_flag;
}

bool ui_get_eye_update(void)
{
    bool flag = priv.eye_is_update_flag;
    priv.eye_is_update_flag = 0;
    return flag;
}

bool ui_get_eye_flag(void)
{
    bool flag = priv.eye_is_open_flag;
    priv.eye_is_open_flag = 0;
    return flag;
}

bool ui_get_active_screen_update(void)
{
    bool flag = priv.active_screen_is_touch;
    priv.active_screen_is_touch = 0;
    return flag;
}

void ui_get_active_screen_pointer(int *x, int *y)
{
    if (x) {
        *x = priv.activate_screen_touch_x;
    }

    if (y) {
        *y = priv.activate_screen_touch_y;
    }
}

void ui_set_pointer(int idx, int x, int y, int r, int g, int b)
{
    if (idx > 2) return;

    lv_obj_t *obj = ui_draw_pointer[idx];
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_style_bg_color(obj, lv_color_make(r, g, b), 0);
}

bool ui_get_options_is_pressed(void)
{
    bool flag = priv.lab_options_is_pressed_flag;
    return flag;
}

void ui_set_user_lab_range(int lmin, int lmax, int amin, int amax, int bmin, int bmax)
{
    lv_obj_t *obj = ui_user_btn;
    ui_lab_range_t *lab = (ui_lab_range_t *)lv_obj_get_user_data(obj);
    lab->l_min = lmin;
    lab->l_max = lmax;
    lab->a_min = amin;
    lab->a_max = amax;
    lab->b_min = bmin;
    lab->b_max = bmax;

    ui_set_lmin_value(lab->l_min);
    ui_set_lmax_value(lab->l_max);
    ui_set_amin_value(lab->a_min);
    ui_set_amax_value(lab->a_max);
    ui_set_bmin_value(lab->b_min);
    ui_set_bmax_value(lab->b_max);
}

bool ui_get_user_btn_statue(void)
{
    return priv.user_btn_is_touch_flag;
}