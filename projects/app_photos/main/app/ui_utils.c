#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "stdlib.h"
#include <time.h>
#include "ui_utils.h"

static photos_list_t *create_photos_list(photo_directory_t *dir_info, int num) {
    photos_list_t *list = (photos_list_t *)malloc(sizeof(photos_list_t));
    if (!list) {
        perror("malloc failed!\r\n");
        return NULL;
    }

    memset(list, 0, sizeof(photos_list_t));

    photo_directory_t *dir = (photo_directory_t *)malloc(sizeof(photo_directory_t) * num);
    if (!list) {
        perror("malloc failed!\r\n");
        free(list);
        return NULL;
    }

    memcpy(dir, dir_info, sizeof(photo_directory_t) * num);
    list->photo_directory = dir;
    list->photo_directory_num = num;

    return list;
}

int destroy_photos_list(photos_list_t **photos_list) {
    if (photos_list && *photos_list) {
        photos_list_t *list = *photos_list;
        if (list->photo_directory) {
            for (int i = 0; i < list->photo_directory_num; i ++) {
                photo_directory_t *dir = &list->photo_directory[i];
                if (dir->photos) {
                    for (int j = 0; j < dir->photos_num; j ++) {
                        photo_t *photo = &dir->photos[j];
                        if (photo->path) {
                            free(photo->path);
                            photo->path = NULL;
                        }

                        if (photo->file_name) {
                            free(photo->file_name);
                            photo->file_name = NULL;
                        }

                        if (photo->thumbnail_path) {
                            free(photo->thumbnail_path);
                            photo->thumbnail_path = NULL;
                        }
                    }

                    if (dir->path) {
                        free(dir->path);
                        dir->path = NULL;
                    }

                    if (dir->name) {
                        free(dir->name);
                        dir->name = NULL;
                    }

                    free(dir->photos);
                    dir->photos = NULL;
                }
            }
            free(list->photo_directory);
            list->photo_directory = NULL;
        }
        free(*photos_list);
        *photos_list = NULL;
    }

    return 0;
}

static int is_valid_date(const char* date) {
    int year, month, day;
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) == 3) {
        return 1;
    } else if (sscanf(date, "%d.%d.%d", &year, &month, &day) == 3) {
        return 1;
    } else {
        return 0;
    }
}

static int is_directory(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

static int is_valid_pic(const char* name) {
    const char *dot = strrchr(name, '.');
    return dot && (!strcmp(dot, ".jpg") || !strcmp(dot, ".jpeg") || !strcmp(dot, ".png"));
}

static int photo_compare(const void *a, const void *b)
{
    photo_t *a_photo = (photo_t *)a;
    photo_t *b_photo = (photo_t *)b;

    struct stat a_st, b_st;
    stat(a_photo->path, &a_st);
    stat(b_photo->path, &b_st);

    return (a_st.st_ctime < b_st.st_ctime ? 1 : -1);
}

static void find_photos(const char* dirpath, photo_t **out_photos, int *out_num) {
    DIR* dir;
    struct dirent* entry;
    char path[1024];
    int valid_dir_cnt = 0, valid_dir_num = 0;
    photo_t *photos_list = NULL;

    dir = opendir(dirpath);
    if (!dir) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (is_valid_pic(entry->d_name)) {
                valid_dir_cnt ++;
            }
        }
    }
    closedir(dir);

    valid_dir_num = valid_dir_cnt;
    photos_list = malloc(sizeof(photo_t) * valid_dir_num);
    if (photos_list == NULL) {
        perror("malloc failed");
        return;
    }
    valid_dir_cnt = 0;

    dir = opendir(dirpath);
    if (!dir) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (is_valid_pic(entry->d_name)) {
                snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);
                photo_t *photo = (photo_t *)&photos_list[valid_dir_cnt ++];
                photo->path = (char *)malloc(strlen(path) + 1);
                if (!photo->path) {
                    perror("malloc failed!");
                    return;
                }
                photo->file_name = (char *)malloc(strlen(entry->d_name) + 1);
                if (!photo->file_name) {
                    perror("malloc failed!");
                    return;
                }

                strcpy(photo->file_name, entry->d_name);
                strcpy(photo->path, path);

                snprintf(path, sizeof(path), "%s/.thumbnail/%s", dirpath, entry->d_name);
                photo->thumbnail_path = (char *)malloc(strlen(path) + 1);
                if (!photo->thumbnail_path) {
                    perror("malloc failed!");
                    return;
                }
                strcpy(photo->thumbnail_path, path);
            }
        }
    }
    closedir(dir);

    qsort(photos_list, valid_dir_num, sizeof(photo_t), photo_compare);

    if (out_photos) {
        *out_photos = photos_list;
    }

    if (out_num) {
        *out_num = valid_dir_num;
    }
}

static int photo_directory_compare(const void *a, const void *b)
{
    photo_directory_t *a_dir = (photo_directory_t *)a;
    photo_directory_t *b_dir = (photo_directory_t *)b;

    int a_year, a_month, a_day;
    int b_year, b_month, b_day;
    if (sscanf(a_dir->name, "%d-%d-%d", &a_year, &a_month, &a_day) != 3) {
        if (sscanf(a_dir->name, "%d.%d.%d", &a_year, &a_month, &a_day) != 3) {
            printf("compare with a bad path:%s\r\n", a_dir->path);
            return 0;
        }
    }

    if (sscanf(b_dir->name, "%d-%d-%d", &b_year, &b_month, &b_day) != 3) {
        if (sscanf(b_dir->name, "%d.%d.%d", &b_year, &b_month, &b_day) != 3) {
            printf("compare with a bad path:%s\r\n", b_dir->path);
            return 0;
        }
    }

    int res = 0;
    if (a_year == b_year) {
        if (a_month == b_month) {
            res = a_day < b_day ? 1 : -1;
        } else {
            res = a_month < b_month ? 1 : -1;
        }
    } else {
        res = a_year < b_year ? 1 : -1;
    }
    return res;
}

static void find_directories(const char* basepath, photo_directory_t **out_dir_list, int *out_dir_num) {
    DIR* dir = NULL;
    struct dirent* entry;
    char path[1024];

    int valid_dir_cnt = 0, valid_dir_num = 0;
    photo_directory_t *valid_dir_list = NULL;

    dir = opendir(basepath);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            if (is_valid_date(entry->d_name)) {
                snprintf(path, sizeof(path), "%s/%s", basepath, entry->d_name);

                if (is_directory(path)) {
                    valid_dir_cnt++;
                }
            }
        }
    }
    closedir(dir);

    valid_dir_num = valid_dir_cnt;
    valid_dir_list = malloc(sizeof(photo_directory_t) * valid_dir_num);
    if (valid_dir_list == NULL) {
        perror("malloc failed");
        return;
    }
    memset(valid_dir_list, 0, sizeof(photo_directory_t) * valid_dir_num);
    valid_dir_cnt = 0;

    dir = opendir(basepath);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            if (is_valid_date(entry->d_name)) {
                if (is_directory(path)) {
                    snprintf(path, sizeof(path), "%s/%s", basepath, entry->d_name);

                    photo_directory_t *photo_dir = (photo_directory_t *)&valid_dir_list[valid_dir_cnt ++];
                    photo_dir->path = (char *)malloc(strlen(path) + 1);
                    if (!photo_dir->path) {
                        perror("malloc failed!");
                        return;
                    }

                    photo_dir->name = (char *)malloc(strlen(entry->d_name) + 1);
                    if (!photo_dir->name) {
                        perror("malloc failed!");
                        return;
                    }

                    strcpy(photo_dir->name, entry->d_name);
                    strcpy(photo_dir->path, path);
                }
            }
        }
    }
    closedir(dir);

    qsort(valid_dir_list, valid_dir_num, sizeof(photo_directory_t), photo_directory_compare);

    if (out_dir_list) {
        *out_dir_list = valid_dir_list;
    }

    if (out_dir_num) {
        *out_dir_num = valid_dir_num;
    }
}

char *ui_get_sys_date(void)
{
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);

    char *date_str = (char *)malloc(16);
    if (date_str == NULL) return NULL;

    strftime(date_str, 20, "%Y-%m-%d", lt);

    return date_str;
}

photos_list_t *get_photo_list(char *path)
{
    photo_directory_t *photo_dir = NULL;
    int photo_dir_num = 0;
    find_directories(path, &photo_dir, &photo_dir_num);

    for (int i = 0; i < photo_dir_num; i ++) {
        photo_directory_t *dir = &photo_dir[i];
        photo_t *photos = NULL;
        int photos_num = 0;
        find_photos(dir->path, &photos, &photos_num);
        dir->photos = photos;
        dir->photos_num = photos_num;
    }

    photos_list_t *photos_list = create_photos_list(photo_dir, photo_dir_num);
    free(photo_dir);

    return photos_list;
}

photo_t *find_last_photo(photos_list_t *photos_list, char *dirname, char *path)
{printf("[%s][%d] dirname:%s filename:%s\r\n", __func__, __LINE__, dirname, path);
    if (photos_list == NULL)
        return NULL;
printf("[%s][%d] photos_list->photo_directory_num:%d\r\n", __func__, __LINE__, photos_list->photo_directory_num);
    for (int i = 0; i < photos_list->photo_directory_num; i ++) {printf("[%s][%d]\r\n", __func__, __LINE__);
        photo_directory_t *dir = &photos_list->photo_directory[i];printf("[%d] dir:%s dirname:%s\r\n", i, dir->name, dirname);
        if (strcmp(dir->name, dirname) == 0) {
            for (int j = 0; j < dir->photos_num; j ++) {
                photo_t *photos = &dir->photos[j];
                if (strcmp(photos->path, path) == 0) {
                    if (j > 0) {
                        return &dir->photos[j - 1];
                    } else {
                        return NULL;
                    }
                }
            }
        }
    }

    return NULL;
}

photo_t *find_next_photo(photos_list_t *photos_list, char *dirname, char *path)
{
    if (photos_list == NULL)
        return NULL;

    for (int i = 0; i < photos_list->photo_directory_num; i ++) {
        photo_directory_t *dir = &photos_list->photo_directory[i];
        if (strcmp(dir->name, dirname) == 0) {
            for (int j = 0; j < dir->photos_num; j ++) {
                photo_t *photos = &dir->photos[j];
                if (strcmp(photos->path, path) == 0) {
                    if (j + 1 < dir->photos_num) {
                        return &dir->photos[j + 1];
                    } else {
                        return NULL;
                    }
                }
            }
        }
    }

    return NULL;
}

