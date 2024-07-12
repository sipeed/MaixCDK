#include "lvgl.h"
#include "app.hpp"
#include "stdio.h"
#include "maix_basic.hpp"
#include "maix_image.hpp"

using namespace maix;

LV_IMG_DECLARE(test_img_128x128);
LV_IMG_DECLARE(test_img_552_368);

typedef struct {
    int disp_w;
    int disp_h;
    char *base_path;
    photos_list_t *photos;
    char *big_img_dir;
    char *big_img_filename;
} priv_t;

priv_t priv;

static char *find_thumbnail_path_by_path(char *path)
{
    photos_list_t *photos = priv.photos;
    if (photos) {
        for (int i = 0; i < photos->photo_directory_num; i ++) {
            photo_directory_t *dir = (photo_directory_t *)&photos->photo_directory[i];
            for (int j = 0; j < dir->photos_num; j ++) {
                photo_t *photo = (photo_t *)&dir->photos[j];
                if (!strcmp(photo->path, path)) {
                    return photo->thumbnail_path;
                }
            }
        }
    }

    return NULL;
}

static lv_image_dsc_t *load_thumbnail_image(char *path, char *thumbnail_path)
{
    image::Image *thumbnail_img = NULL;
    std::string thumbnail_path_string(thumbnail_path);
    if (fs::exists(thumbnail_path_string)) {
        thumbnail_img = image::load(thumbnail_path, image::Format::FMT_BGRA8888);
        if (!thumbnail_img) {
            image::Image *src_img = image::load(path, image::Format::FMT_BGRA8888);
            if (!src_img) {
                log::error("load src image failed!\r\n");
                return NULL;
            }

            thumbnail_img = src_img->resize(128, 128, image::Fit::FIT_COVER);
            thumbnail_img->save(thumbnail_path);
            delete src_img;
        }
    } else {
        image::Image *src_img = image::load(path, image::Format::FMT_BGRA8888);
        if (!src_img) {
            log::error("load src image failed!\r\n");
            return NULL;
        }

        thumbnail_img = src_img->resize(128, 128, image::Fit::FIT_COVER);
        thumbnail_img->save(thumbnail_path);
        delete src_img;
    }

    lv_image_dsc_t *img_dsc = (lv_image_dsc_t *)malloc(sizeof(lv_image_dsc_t));
    if (!img_dsc) {
        perror("load small image failed");
        delete thumbnail_img;
        return NULL;
    }

    if (thumbnail_img->width() != 128 || thumbnail_img->height() != 128) {
        image::Image *temp = thumbnail_img->resize(128, 128, image::Fit::FIT_COVER);
        delete thumbnail_img;
        thumbnail_img = temp;
    }

    memset(img_dsc, 0, sizeof(lv_image_dsc_t));

    img_dsc->header.w = thumbnail_img->width();
    img_dsc->header.h = thumbnail_img->height();
    img_dsc->data_size = thumbnail_img->data_size();
    img_dsc->header.cf = LV_COLOR_FORMAT_ARGB8888;
    img_dsc->data = (uint8_t *)malloc(img_dsc->data_size);
    if (!img_dsc->data) {
        delete thumbnail_img;
        free(img_dsc);
        perror("create image dsc failed");
        return NULL;
    }
    memcpy((uint8_t *)img_dsc->data, (uint8_t *)thumbnail_img->data(), thumbnail_img->data_size());
    delete thumbnail_img;
    return img_dsc;
}

static void free_thumbnail_image(lv_image_dsc_t *dsc)
{
    if (dsc) {
        if (dsc->data) {
            free((uint8_t *)dsc->data);
            dsc->data = NULL;
        }
        free(dsc);
    }
}

int app_pre_init(void)
{
    priv.base_path = (char *)"/maixapp/share/picture";
    return 0;
}

int app_init(display::Display *disp)
{
    ui_all_screen_init();

    if (disp) {
        priv.disp_w = disp->width();
        priv.disp_h = disp->height();
    } else {
        priv.disp_w = 552;
        priv.disp_h = 368;
    }

#if 1
    photos_list_t *photos = get_photo_list(priv.base_path);
    if (photos) {
        for (int i = 0; i < photos->photo_directory_num; i ++) {
            photo_directory_t *dir = (photo_directory_t *)&photos->photo_directory[i];
            // printf("[dir] path:%s  name:%s  photo_num:%d\r\n", dir->path, dir->name, dir->photos_num);
            ui_photo_add_dir(dir->name);
            for (int j = 0; j < dir->photos_num; j ++) {
                photo_t *photo = (photo_t *)&dir->photos[j];
                // printf("[photo] path:%s thumbnail_path:%s name:%s\r\n", photo->path, photo->thumbnail_path, photo->file_name);
                lv_image_dsc_t *dsc = load_thumbnail_image(photo->path, photo->thumbnail_path);
                if (dsc) {
                    ui_photo_add_photo(dir->name, photo->path, dsc);
                    free_thumbnail_image(dsc);
                }
            }
        }
    }
    priv.photos = photos;

    ui_photo_print();
    ui_photo_list_screen_update();
#else
    char *base_path = "./photos";
    photos_list_t *photos = get_photo_list(base_path);
    // if (photos) {
    //     for (int i = 0; i < photos->photo_directory_num; i ++) {
    //         photo_directory_t *dir = (photo_directory_t *)&photos->photo_directory[i];
    //         printf("[dir] path:%s  name:%s  photo_num:%d\r\n", dir->path, dir->name, dir->photos_num);

    //         for (int j = 0; j < dir->photos_num; j ++) {
    //             photo_t *photo = (photo_t *)&dir->photos[j];
    //             printf("[photo] path:%s thumbnail_path:%s name:%s\r\n", photo->path, photo->thumbnail_path, photo->file_name);
    //         }
    //     }
    // }

    destroy_photos_list(&photos);

    lv_image_dsc_t *dsc = &test_img_128x128;
    char *dir1 = "2003-03-05";
    ui_photo_add_dir(dir1);
    ui_photo_add_photo(dir1, "pic1", dsc);
    ui_photo_add_photo(dir1, "pic2", dsc);
    ui_photo_add_photo(dir1, "pic3", dsc);
    char *dir2 = "2013-03-15";
    ui_photo_add_dir(dir2);

    ui_photo_add_photo(dir2, "pic4", dsc);
    ui_photo_add_photo(dir2, "pic5", dsc);
    ui_photo_add_photo(dir2, "pic6", dsc);
    ui_photo_add_photo(dir2, "pic9", dsc);
    ui_photo_add_photo(dir2, "pic7", dsc);
    ui_photo_add_photo(dir2, "pic65", dsc);
    ui_photo_add_photo(dir2, "pic546", dsc);
    ui_photo_add_photo(dir2, "pic4564", dsc);
    ui_photo_add_photo(dir2, "pic354345", dsc);

    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);

    ui_photo_add_photo(dir2, "pic343456", dsc);
    ui_photo_add_photo(dir2, "pic343456", dsc);
    char *dir3 = "2026-04-25";
    ui_photo_add_dir(dir3);
    ui_photo_add_photo(dir3, "pic7", dsc);
    ui_photo_add_photo(dir3, "pic8", dsc);
    ui_photo_add_photo(dir3, "pic9", dsc);
    ui_photo_add_photo(dir3, "pic7", dsc);
    ui_photo_add_photo(dir3, "pic8", dsc);
    ui_photo_add_photo(dir3, "pic9", dsc);
    ui_photo_add_photo(dir3, "pic7", dsc);
    ui_photo_add_photo(dir3, "pic8", dsc);

    ui_photo_print();
    ui_photo_list_screen_update();
#endif

    return 0;
}


static void update_char(char **str, char *new_str)
{
    if (!str || *str == new_str) return;

    if (*str) {
        free(*str);
        *str = NULL;
    }
    *str = (char *)malloc(strlen(new_str) + 1);
    if (!*str) {
        printf("malloc failed\r\n");
        return;
    }
    strcpy(*str, new_str);
}

static void ui_set_big_image(char *dir_name, char *path)
{
    ui_big_photo_info_t big_photo_info = {0};
    std::string path_string(path);
    image::Image *big_image = image::load(path, image::Format::FMT_BGRA8888);
    int src_w = 0, src_h = 0;
    if (big_image) {
        image::Image *resize_big_image = NULL;
        if (big_image->width() != priv.disp_w || big_image->height() != priv.disp_h) {
            resize_big_image = big_image->resize(priv.disp_w, priv.disp_h, image::Fit::FIT_CONTAIN);
            src_w = big_image->width();
            src_h = big_image->height();
            delete big_image;
            big_image = NULL;
        } else {
            resize_big_image = big_image;
            src_w = resize_big_image->width();
            src_h = resize_big_image->height();
        }

        if (resize_big_image) {
            big_photo_info.size = fs::getsize(path_string);
            big_photo_info.dir_name = dir_name;
            big_photo_info.path = path;

            std::string type_string = fs::splitext(path_string)[1];
            big_photo_info.type = (char *)((char *)type_string.c_str() + 1);
            big_photo_info.src_width = src_w;
            big_photo_info.src_height = src_h;
            lv_image_dsc_t img_dsc = {0};
            img_dsc.header.w = resize_big_image->width();
            img_dsc.header.h = resize_big_image->height();
            img_dsc.data_size = resize_big_image->data_size();
            img_dsc.header.cf = LV_COLOR_FORMAT_ARGB8888;
            img_dsc.data = (uint8_t *)resize_big_image->data();
            memcpy(&big_photo_info.dsc, (lv_image_dsc_t *)&img_dsc, sizeof(lv_image_dsc_t));
            ui_update_big_photo(&big_photo_info);

            delete resize_big_image;
        }
    }
}

int app_loop(void)
{
    if (ui_get_touch_small_image_flag()) {
        if (!ui_get_bulk_delete_flag()) {
            // ready jump to big photo
            // prepare big photo
            char *path = NULL, *dir_name = NULL;
            ui_get_touch_small_image_path(&dir_name, &path);

            update_char(&priv.big_img_dir, dir_name);
            update_char(&priv.big_img_filename, path);

            printf("load big photo info. path:%s\r\n", path);
            ui_set_big_image(dir_name, path);

            // jump
            ui_set_view_flag(1);
        }
    }

    if (ui_get_exit_flag()) {
        printf("exit!\r\n");
        destroy_photos_list(&priv.photos);
        ui_all_screen_deinit();
        app::set_exit_flag(true);
    }

    if (ui_get_bulk_delete_flag()) {
        printf("bluk delete image\r\n");
        char *dir_name = NULL, *path = NULL, *thumbnail_path = NULL;
        while (ui_bulk_delete_path_iter(&dir_name, &path) == 0) {
            thumbnail_path = find_thumbnail_path_by_path(path);
            printf("dir_name:%s path:%s thumbnail_path:%s  \r\n", dir_name, path, thumbnail_path);
            if (path) {
                fs::remove(std::string(path));
            }

            if (thumbnail_path) {
                fs::remove(std::string(thumbnail_path));
            }
            ui_photo_del_photo(dir_name, path);
        }
        ui_photo_list_screen_update();
    }

    if (ui_get_delete_big_photo_flag()) {
        printf("delete big image\r\n");

        // get the path(and thumbnail path) of big image
        char *path = NULL, *dir_name = NULL;
        ui_get_touch_small_image_path(&dir_name, &path);

        char *thumbnail_path = NULL;
        if (path) {
            printf("delete path:%s\r\n", path);
            thumbnail_path = find_thumbnail_path_by_path(path);
            if (thumbnail_path) {
                printf("delete thumbnail_path:%s\r\n", thumbnail_path);
            }
        }

        // delete the big image from path
        if (path) {
            fs::remove(std::string(path));
        }

        if (thumbnail_path) {
            fs::remove(std::string(thumbnail_path));
        }

        // update ui
        ui_photo_del_photo(dir_name, path);
        ui_photo_list_screen_update();
        ui_set_view_flag(0);
    }

    if (ui_get_touch_show_right_big_photo_flag()) {
        printf("touch show right big photo\r\n");
        photo_t *last_photo = find_next_photo(priv.photos, priv.big_img_dir, priv.big_img_filename);
        if (last_photo) {
            printf("last_photo:%s\r\n", last_photo->path);

            update_char(&priv.big_img_dir, priv.big_img_dir);
            update_char(&priv.big_img_filename, last_photo->path);

            printf("load big photo info. path:%s\r\n", last_photo->path);
            ui_set_big_image(priv.big_img_dir, priv.big_img_filename);
            ui_right_screen_update();
            ui_set_view_flag(2);
        } else {
            printf("The last photo is not found!\r\n");
        }
    }

    if (ui_get_touch_show_left_big_photo_flag()) {
        printf("touch show left big photo\r\n");

        photo_t *last_photo = find_last_photo(priv.photos, priv.big_img_dir, priv.big_img_filename);
        if (last_photo) {
            printf("last_photo:%s\r\n", last_photo->path);

            update_char(&priv.big_img_dir, priv.big_img_dir);
            update_char(&priv.big_img_filename, last_photo->path);

            ui_set_big_image(priv.big_img_dir, priv.big_img_filename);
            ui_right_screen_update();
            ui_set_view_flag(2);
        } else {
            printf("The last photo is not found!\r\n");
        }
    }
    return 0;
}

int app_deinit(void)
{
    return 0;
}