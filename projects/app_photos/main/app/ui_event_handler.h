#ifndef __EVENT_HANDLER_H
#define __EVENT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "stdint.h"

void event_touch_exit_cb(lv_event_t * e);
void event_touch_small_image_cb(lv_event_t * e);
void event_video_bar_event_cb(lv_event_t * e);
void event_video_view_event_cb(lv_event_t * e);

bool ui_get_exit_flag(void);
bool ui_get_touch_small_image_flag(void);
void ui_get_touch_small_image_path(char **dir_name, char **img_path);
bool ui_get_bulk_delete_flag(void);
bool ui_get_bulk_delete_cancel_flag(void);
bool ui_get_show_big_photo_info_flag(void);
bool ui_get_delete_big_photo_flag(void);
char *ui_get_delete_big_photo_path(void);
bool ui_get_touch_show_right_big_photo_flag(void);
bool ui_get_touch_show_left_big_photo_flag(void);
bool ui_touch_is_video_image_flag(void);
void ui_set_video_bar_s(double curr_s, double total_s);
bool ui_get_touch_video_bar_press_flag(void);
bool ui_get_touch_video_bar_release_flag(void);
double ui_get_video_bar_value(void);
bool ui_get_video_view_pressed_flag(void);

#ifdef __cplusplus
}
#endif

#endif