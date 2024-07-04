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

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // __SCREEN_H