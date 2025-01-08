#ifndef __EVENT_HANDLER_H
#define __EVENT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "stdint.h"

/**
 * 0: pic
 * 1: video
 */
extern int g_camera_mode;
void touch_start_video(int flag);
void touch_start_pic();

void event_left_screen_scroll_cb(lv_event_t * e);
void event_touch_exit_cb(lv_event_t * e);
void event_touch_delay_cb(lv_event_t * e);
void event_touch_resolution_cb(lv_event_t * e);
void event_touch_option_cb(lv_event_t * e);
void event_touch_bitrate_cb(lv_event_t * e);
void event_touch_video_camera_cb(lv_event_t * e);
void event_touch_start_cb(lv_event_t * e);
void event_touch_small_img_cb(lv_event_t * e);
void event_touch_shutter_cb(lv_event_t * e);
void event_touch_iso_cb(lv_event_t * e);
void event_touch_ev_cb(lv_event_t * e);
void event_touch_wb_cb(lv_event_t * e);
void event_iso_bar_update_cb(lv_event_t * e);
void event_shutter_bar_update_cb(lv_event_t * e);
void event_ev_bar_update_cb(lv_event_t * e);
void event_wb_bar_update_cb(lv_event_t * e);
void event_touch_shutter_to_auto_cb(lv_event_t * e);
void event_touch_wb_to_auto_cb(lv_event_t * e);
void event_touch_iso_to_auto_cb(lv_event_t * e);
void event_touch_ev_to_auto_cb(lv_event_t * e);
void event_touch_wb_to_auto_cb(lv_event_t * e);
void event_touch_select_delay_cb(lv_event_t * e);
void event_touch_select_resolution_cb(lv_event_t * e);
void event_touch_select_bitrate_cb(lv_event_t * e);
void event_touch_big_img_cb(lv_event_t * e);
void event_touch_shutter_plus_cb(lv_event_t *e);
void event_touch_shutter_minus_cb(lv_event_t *e);
void event_touch_iso_plus_cb(lv_event_t *e);
void event_touch_iso_minus_cb(lv_event_t *e);
void event_touch_raw_cb(lv_event_t * e);
void event_touch_light_cb(lv_event_t * e);
void event_touch_timestamp_cb(lv_event_t * e);
void event_touch_timelapse_cb(lv_event_t * e);
void event_touch_select_timelapse_cb(lv_event_t * e);

bool ui_get_cam_snap_flag(void);
bool ui_get_cam_video_start_flag(void);
bool ui_get_cam_video_stop_flag(void);
bool ui_get_cam_video_try_stop_flag(void);
bool ui_get_view_photo_flag(void);
bool ui_get_exit_flag(void);
bool ui_get_delay_setting_flag(void);
bool ui_get_resolution_setting_flag(void);
int ui_get_resolution_setting_idx(void);
bool ui_get_shutter_setting_flag(void);
bool ui_get_iso_setting_flag(void);
bool ui_get_ev_setting_flag(void);
bool ui_get_wb_setting_flag(void);

int ui_get_photo_delay(int *delay_ms);
int ui_get_resulution(int *w, int *h);
int ui_get_shutter_value(double *value);
int ui_get_iso_value(int *value);
int ui_get_ev_value(double *value);
int ui_get_wb_value(int *value);

bool ui_get_wb_auto_flag(void);
bool ui_get_shutter_auto_flag(void);
bool ui_get_iso_auto_flag(void);
bool ui_get_ev_auto_flag(void);

void ui_set_shutter_value(double val);
void ui_set_iso_value(int val);
void ui_set_ev_value(int val);
void ui_set_wb_value(int val);

void ui_update_small_img(void *data, int data_size);
void ui_update_big_img(void *data, int data_size);
void ui_anim_run_save_img(void);
void ui_set_record_time(uint64_t ms);
void ui_anim_photo_delay_start(int delay_s);
bool ui_get_photo_delay_anim_status(void);
bool ui_get_photo_delay_anim_stop_flag(void);

bool ui_get_focus_btn_update_flag();
bool ui_get_focus_btn_touched();
bool ui_get_raw_btn_update_flag();
bool ui_get_raw_btn_touched();
bool ui_get_light_btn_update_flag();
bool ui_get_light_btn_touched();

bool ui_get_bitrate_update_flag();
void ui_set_bitrate(int bitrate, bool need_update);
int ui_get_bitrate();

bool ui_get_timestamp_btn_update_flag();
bool ui_get_timestamp_btn_touched();

bool ui_get_timelapse_update_flag();
int ui_get_timelapse_s();
void ui_set_timelapse_s(int timelapse, bool need_update);


typedef struct {
    unsigned int exposure_time_max;
    unsigned int exposure_time_min;
    unsigned int exposure_time_default;
    unsigned int iso_max;
    unsigned int iso_min;
    unsigned int iso_default;
    double exposure_time_table[31];
    double iso_table[19];
} ui_camera_config_t;
void ui_camera_config_read(ui_camera_config_t *cfg);
void ui_camera_config_update(ui_camera_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif