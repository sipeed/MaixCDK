#ifndef __UI_UTILS_H__
#define __UI_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

void ui_list_valid_picture_path(char *base_path);
char *ui_find_latest_picture(char *base_path);
char *ui_get_sys_date(void);    // need to free the output memory

#ifdef __cplusplus
}
#endif

#endif // __UI_UTILS_H__