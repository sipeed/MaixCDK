#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "stdlib.h"
#include <time.h>
#include "ui_utils.h"

typedef struct {
    char path[100];
} path_info_t;

static int is_valid_date(const char* date) {
    if (strlen(date) != 10) {
        return 0;
    }

    if (date[4] != '-' || date[7] != '-') {
        return 0;
    }

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) {
            continue;
        }
        if (date[i] < '0' || date[i] > '9') {
            return 0;
        }
    }

    return 1;
}

static int is_directory(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

static int is_valid_pic(const char* name) {
    const char *dot = strrchr(name, '.');
    return dot && (!strcmp(dot, ".jpg") || !strcmp(dot, ".jpeg"));
}

static void find_picture_files(const char* dirpath, path_info_t **out_dir_list, int *out_dir_num) {
    DIR* dir;
    struct dirent* entry;
    char path[1024];
    int valid_dir_cnt = 0, valid_dir_num = 0;
    path_info_t *valid_dir_list = NULL;

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
    valid_dir_list = malloc(sizeof(path_info_t) * valid_dir_num);
    if (valid_dir_list == NULL) {
        perror("malloc failed");
        return;
    }
    memset(valid_dir_list, 0, sizeof(char *) * valid_dir_num);
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
                strncpy(valid_dir_list[valid_dir_cnt ++].path, path, sizeof(valid_dir_list[valid_dir_cnt].path));
            }
        }
    }
    closedir(dir);

    if (out_dir_list) {
        *out_dir_list = valid_dir_list;
    }

    if (out_dir_num) {
        *out_dir_num = valid_dir_num;
    }
}

static void find_directories(const char* basepath, path_info_t **out_dir_list, int *out_dir_num) {
    DIR* dir;
    struct dirent* entry;
    char path[1024];

    int valid_dir_cnt = 0, valid_dir_num = 0;
    path_info_t *valid_dir_list = NULL;

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
    valid_dir_list = malloc(sizeof(path_info_t) * valid_dir_num);
    if (valid_dir_list == NULL) {
        perror("malloc failed");
        return;
    }
    memset(valid_dir_list, 0, sizeof(char *) * valid_dir_num);
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
                snprintf(path, sizeof(path), "%s/%s", basepath, entry->d_name);

                if (is_directory(path)) {
                    strncpy(valid_dir_list[valid_dir_cnt ++].path, path, sizeof(valid_dir_list[valid_dir_cnt].path));
                }
            }
        }
    }
    closedir(dir);

    if (out_dir_list) {
        *out_dir_list = valid_dir_list;
    }

    if (out_dir_num) {
        *out_dir_num = valid_dir_num;
    }
}

char *ui_find_latest_picture(char *base_path)
{
    int max_y = 0, max_m = 0, max_d = 0;
    path_info_t *date_dir_list = NULL;
    int date_dir_num = 0;
    find_directories(base_path, &date_dir_list, &date_dir_num);
    if (date_dir_num == 0) {
        return NULL;
    }

    for (int i = 0; i < date_dir_num; ++i) {
        const char *dot = strrchr(date_dir_list[i].path, '/');
        int y, m, d;
        sscanf(dot + 1, "%d-%d-%d", &y, &m, &d);
        if (y > max_y) {
            max_y = y;
            max_m = m;
            max_d = d;
        } else if (y == max_y) {
            if (m > max_m) {
                max_m = m;
                max_d = d;
            } else if (m == max_m) {
                if (d > max_d) {
                    max_d = d;
                }
            }
        }
    }
    free(date_dir_list);

    char latest_dir[1024];
    snprintf(latest_dir, sizeof(latest_dir), "%s/%.4d-%.2d-%.2d", base_path, max_y, max_m, max_d);

    time_t max_pic_time = 0;
    char *max_pic_path = NULL;
    path_info_t *pic_list = NULL;
    int pic_dir_num = 0;
    find_picture_files(latest_dir, &pic_list, &pic_dir_num);
    if (pic_dir_num == 0) {
        return NULL;
    }

    for (int j = 0; j < pic_dir_num; ++j) {
        struct stat t_stat;
        stat(pic_list[j].path, &t_stat);

        if (max_pic_time < t_stat.st_ctime) {
            max_pic_time = t_stat.st_ctime;
            max_pic_path = pic_list[j].path;
        }
    }

    char *latest_pic_path = malloc(strlen(max_pic_path) + 1);
    if (latest_pic_path == NULL) {
        perror("malloc failed");
        return NULL;
    }
    strcpy(latest_pic_path, max_pic_path);

    free(pic_list);

    return latest_pic_path;
}

void ui_list_valid_picture_path(char *base_path)
{
    path_info_t *date_dir_list = NULL;
    int date_dir_num = 0;
    find_directories(base_path, &date_dir_list, &date_dir_num);
    for (int i = 0; i < date_dir_num; ++i) {
        printf("date_dir_list[%d]: %s\n", i, date_dir_list[i].path);

        path_info_t *pic_list = NULL;
        int pic_dir_num = 0;
        find_picture_files(date_dir_list[i].path, &pic_list, &pic_dir_num);
        for (int j = 0; j < pic_dir_num; ++j) {
            printf("pic_list[%d]: %s\n", j, pic_list[j].path);
        }
        free(pic_list);
    }

    free(date_dir_list);
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