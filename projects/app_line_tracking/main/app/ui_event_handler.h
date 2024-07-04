#ifndef __EVENT_HANDLER_H
#define __EVENT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "stdint.h"

typedef struct {
    int l;
    int a;
    int b;
} ui_lab_t;

typedef struct {
    int l_min;
    int l_max;
    int a_min;
    int a_max;
    int b_min;
    int b_max;
} ui_lab_range_t;

void event_active_screen_cb(lv_event_t * e);
void event_touch_exit_cb(lv_event_t * e);
void event_touch_eye_cb(lv_event_t * e);
void event_touch_lab_options_cb(lv_event_t * e);
void event_lab_bar_update_cb(lv_event_t * e);
void event_touch_red_cb(lv_event_t * e);
void event_touch_green_cb(lv_event_t * e);
void event_touch_blue_cb(lv_event_t * e);
void event_touch_user_cb(lv_event_t * e);
void event_touch_lmin_cb(lv_event_t * e);
void event_touch_lmax_cb(lv_event_t * e);
void event_touch_amin_cb(lv_event_t * e);
void event_touch_amax_cb(lv_event_t * e);
void event_touch_bmin_cb(lv_event_t * e);
void event_touch_bmax_cb(lv_event_t * e);
void event_touch_color_btn_cb(lv_event_t * e);

void ui_set_bar_range(int min, int max);
void ui_set_bar_value(char *txt, int value);
void ui_set_lmin_value(int value);
void ui_set_lmax_value(int value);
void ui_set_amin_value(int value);
void ui_set_amax_value(int value);
void ui_set_bmin_value(int value);
void ui_set_bmax_value(int value);

int ui_get_lmin_value(void);
int ui_get_lmax_value(void);
int ui_get_amin_value(void);
int ui_get_amax_value(void);
int ui_get_bmin_value(void);
int ui_get_bmax_value(void);

bool ui_lmin_update(void);
bool ui_lmax_update(void);
bool ui_amin_update(void);
bool ui_amax_update(void);
bool ui_bmin_update(void);
bool ui_bmax_update(void);

void ui_show_color_box(int en);
void ui_set_color_of_color_box(int idx, int r, int g, int b);
void ui_set_value_of_color_box(int idx, int l, int a, int b);
bool ui_get_exit_flag(void);
bool ui_get_eye_update(void);
bool ui_get_eye_flag(void);
bool ui_get_active_screen_update(void);
void ui_get_active_screen_pointer(int *x, int *y);
void ui_set_pointer(int idx, int x, int y, int r, int g, int b);
bool ui_get_options_is_pressed(void);
void ui_set_user_lab_range(int lmin, int lmax, int amin, int amax, int bmin, int bmax);
bool ui_get_user_btn_statue(void);

#ifdef __cplusplus
}
#endif

#endif