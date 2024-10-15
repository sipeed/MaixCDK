#ifndef __SCREEN_H
#define __SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

void ui_all_screen_init(void);
void ui_set_shutter_value(double val);
void ui_set_iso_value(int val);
void ui_set_ev_value(int val);
void ui_set_wb_value(int val);
void ui_update_small_img(void *data, int data_size);
void ui_refresh(void);
void ui_set_select_option(int idx);
void ui_show_center_image(uint8_t *data, int data_size, int width, int height);
void ui_click_raw_button();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // __SCREEN_H