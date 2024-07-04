#ifndef __UI_UTILS_H__
#define __UI_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *path;
    char *file_name;
    char *thumbnail_path;
} photo_t;

typedef struct {
    char *path;
    char *name;
    photo_t *photos;
    int photos_num;
} photo_directory_t;

typedef struct {
    photo_directory_t *photo_directory;
    int photo_directory_num;
} photos_list_t;

photos_list_t *get_photo_list(char *path);
int destroy_photos_list(photos_list_t **photos_list);
char *ui_get_sys_date(void);    // need to free the output memory
photo_t *find_last_photo(photos_list_t *photos_list, char *dirname, char *path);
photo_t *find_next_photo(photos_list_t *photos_list, char *dirname, char *path);

#ifdef __cplusplus
}
#endif

#endif // __UI_UTILS_H__