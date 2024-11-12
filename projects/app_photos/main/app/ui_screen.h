#ifndef __SCREEN_H
#define __SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *path;
    lv_image_dsc_t *img_dsc;
    uint32_t is_touch : 1;
    uint32_t is_video : 1;
} ui_photo_t;

typedef struct {
    char *path;
    char *type;
    char *dir_name;
    int size;
    int src_width;
    int src_height;
    lv_image_dsc_t dsc;
} ui_big_photo_info_t;

void ui_all_screen_init(void);
void ui_all_screen_deinit(void);
void ui_photo_add_dir(char *dir_name);
void ui_photo_del_dir(char *dir_name);
void ui_photo_add_photo(char *dir_name, char *path, lv_image_dsc_t *dsc, bool is_video);
void ui_photo_del_photo(char *dir_name, char *path);
void ui_photo_clear_all_photo_flag(void);
void ui_photo_print(void);
void ui_photo_list_screen_update(void);
int ui_get_need_bulk_delete(void);
int ui_set_need_bulk_delete(int value);
void ui_bulk_delete_path_init(void);
int ui_bulk_delete_path_iter(char **dirname, char **img_path);
void ui_set_view_flag(int value);
int ui_get_view_flag(void);
void ui_update_big_photo(ui_big_photo_info_t *info);
void ui_right_screen_update(void);
void ui_set_total_ms_of_video_bar(int ms);
void ui_clear_video_bar();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // __SCREEN_H