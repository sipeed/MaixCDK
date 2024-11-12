#include "lvgl.h"
#include "app.hpp"
#include "stdio.h"
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_video.hpp"
#include "maix_display.hpp"
#include <list>
#include <sys/stat.h>

using namespace maix;

LV_IMG_DECLARE(test_img_128x128);
LV_IMG_DECLARE(test_img_552_368);

class PhotoVideoInfo {
public:
    int type;                       // 0,photo; 1,video
    std::string date;
    std::string path;
    std::string thumbnail_path;

    PhotoVideoInfo(int new_type, std::string new_date, std::string new_path, std::string new_thumbnail_path) {
        type = new_type;
        date = new_date;
        path = new_path;
        thumbnail_path = new_thumbnail_path;
    }

    PhotoVideoInfo *clone() {
        return new PhotoVideoInfo(type, date, path, thumbnail_path);
    }

    bool is_picture() {
        return type == 0;
    }

    bool is_video() {
        return type == 1;
    }
};

class PhotoVideo {
    std::string _video_path;
    std::string _picture_path;
    std::list<std::pair<std::string, std::list<PhotoVideoInfo>>> *_video_photo_list;
public:
    PhotoVideo(std::string picture_path, std::string video_path) {
        _video_path = video_path;
        _picture_path = picture_path;
        _video_photo_list = new std::list<std::pair<std::string, std::list<PhotoVideoInfo>>>();
        err::check_bool_raise(fs::exists(video_path), "video path not exist!");
        err::check_bool_raise(fs::exists(picture_path), "photo path not exist!");
    }

    ~PhotoVideo() {
        if (_video_photo_list)  {
            delete _video_photo_list;
            _video_photo_list = NULL;
        }
    }

    // date format: "2021-01-01"
    // a > b, return 1;
    // a == b, return 0;
    // a < b, return -1;
    static int _compare_date_string(std::string &a, std::string &b) {
        auto a_date = _get_date_number_from_string(a);
        auto b_date = _get_date_number_from_string(b);
        if (a_date.size() >= 3 && b_date.size() >= 3) {
            if (a_date[0] == b_date[0] && a_date[1] == b_date[1] && a_date[2] == b_date[2]) {
                return 0;
            }

            if (a_date[0] != b_date[0]) {
                return a_date[0] < b_date[0] ? -1 : 1;
            } else if (a_date[1] != b_date[1]) {
                return a_date[1] < b_date[1] ? -1 : 1;
            } else {
                return a_date[2] < b_date[2] ? -1 : 1;
            }
        }
        return 0;
    }

    // compare last change time
    // a > b, return 1;
    // a == b, return 0;
    // a < b, return -1;
    static int _compare_file_date(std::string &a, std::string &b) {
        struct stat a_st, b_st;
        stat(a.c_str(), &a_st);
        stat(b.c_str(), &b_st);

        if (a_st.st_ctime == b_st.st_ctime) {
            return 0;
        } else {
            return (a_st.st_ctime < b_st.st_ctime ? -1 : 1);
        }

        return 0;
    }

    static bool _delete_video_phto_info(PhotoVideoInfo &info) {
        return true;
    }

    static bool _copy_video_photo_info(PhotoVideoInfo &old_info, PhotoVideoInfo &new_info) {
        old_info.path = new_info.path;
        old_info.thumbnail_path = new_info.thumbnail_path;
        old_info.type = new_info.type;
        return true;
    }

    static bool push_video_photo_list(std::list<PhotoVideoInfo> &list, PhotoVideoInfo &new_info) {
        if (list.size() == 0) {
            list.push_back(new_info);
        } else {
            for (auto iter = list.rbegin(); iter != list.rend(); iter++) {
                if (iter->path == new_info.path) {
                    // If found the same path, do nothing!
                    return true;
                }
            }
            auto iter = list.rbegin();
            for (; iter != list.rend(); iter++) {
                auto info = *iter;
                if (new_info.path != info.path) {
                    auto compare_res = _compare_file_date(new_info.path, info.path);
                    if (compare_res >= 0) {
                        list.insert(iter.base(), new_info);
                        break;
                    } else {
                        if (std::next(iter) == list.rend()) {
                            list.push_front(new_info);
                            break;
                        }
                    }
                }
            }
        }

        return true;
    }

    bool push_video_photo_info(std::string date, PhotoVideoInfo &new_info)
    {
        if (_video_photo_list->size() == 0) {
            std::pair<std::string, std::list<PhotoVideoInfo>> new_item = std::make_pair(date, std::list<PhotoVideoInfo>());
            push_video_photo_list(new_item.second, new_info);
            _video_photo_list->push_back(new_item);
        } else {
            auto iter = _video_photo_list->rbegin();
            for (; iter != _video_photo_list->rend(); iter ++) {
                auto pair = iter;
                auto compare_res = _compare_date_string(date, pair->first);
                if (compare_res == 0) {
                    push_video_photo_list(pair->second, new_info);
                    break;
                } else if (compare_res > 0) {
                    std::pair<std::string, std::list<PhotoVideoInfo>> new_item = std::make_pair(date, std::list<PhotoVideoInfo>());
                    push_video_photo_list(new_item.second, new_info);
                    _video_photo_list->push_back(new_item);
                    break;
                } else {
                    if (std::next(iter) == _video_photo_list->rend()) {
                        std::pair<std::string, std::list<PhotoVideoInfo>> new_item = std::make_pair(date, std::list<PhotoVideoInfo>());
                        push_video_photo_list(new_item.second, new_info);
                        _video_photo_list->push_front(new_item);
                        break;
                    }
                }
            }
        }

        return true;
    }

    static std::vector<int> _get_date_number_from_string(std::string str) {
        std::vector<int> result;
        int a_year, a_month, a_day;
        if (sscanf(str.c_str(), "%d-%d-%d", &a_year, &a_month, &a_day) != 3) {
            if (sscanf(str.c_str(), "%d.%d.%d", &a_year, &a_month, &a_day) != 3) {
                printf("compare with a bad path:%s\r\n", str.c_str());
                return result;
            }
        }

        result.push_back(a_year);
        result.push_back(a_month);
        result.push_back(a_day);
        return result;
    }

    static void _sort_date_vector(std::vector<std::string> &vec)
    {
        std::sort(vec.begin(), vec.end(), [](std::string a, std::string b) {
            return _compare_date_string(a, b) <= 0 ? true : false;
        });
    }

    static bool _picture_path_is_valid(std::string path) {
        const char *dot = strrchr(path.c_str(), '.');
        return dot && (!strcmp(dot, ".jpg") || !strcmp(dot, ".jpeg") || !strcmp(dot, ".png"));
    }

    static bool _video_path_is_valid(std::string path) {

        size_t pos = path.rfind(".mp4");
        if (pos != std::string::npos) {
            return true;
        }

        return false;
    }

    static void _print_video_photo_info(PhotoVideoInfo *info)
    {
        log::info(  "\ttype: %s\r\n"
                    "\tpath: %s\r\n"
                    "\tthumbnail_path: %s, exists: %s\r\n",
                    info->is_picture() ? "picture" : (info->is_video() ? "video" : "unknow"),
                    info->path.c_str(), info->thumbnail_path.c_str(), fs::exists(info->thumbnail_path) ? "true" : "false");
    }

    void print_video_photo_list() {
        log::info(" === VIDEO AND PHOTO LIST ===");
        auto iter = _video_photo_list->begin();
        for (; iter != _video_photo_list->end(); iter++) {
            auto item = *iter;
            auto date = item.first;
            auto list = item.second;
            log::info("\t[%s]", date.c_str());
            auto list_iter = list.begin();
            for (; list_iter != list.end(); list_iter ++) {
                auto list_item = *list_iter;
                _print_video_photo_info(&list_item);
            }
        }
        log::info(" ============================");
    }

    void delete_photo_video_with_path(std::string path)
    {
        auto iter = _video_photo_list->begin();
        for (; iter != _video_photo_list->end(); iter++) {
            auto date = iter->first;
            auto list_iter = iter->second.begin();
            for (; list_iter != iter->second.end(); list_iter ++) {
                auto list_item = *list_iter;
                if (list_item.path == path) {
                    if (fs::exists(path)) {
                        fs::remove(path);
                    }

                    if (fs::exists(list_item.thumbnail_path)) {
                        fs::remove(list_item.thumbnail_path);
                    }
                    list_iter = iter->second.erase(list_iter);
                    break;
                }
            }
        }
    }

    bool collect_video_photo()
    {
        std::vector<std::string> date_dirs;

        // uint64_t t = time::ticks_ms();
        auto picture_dirs = fs::listdir(_picture_path);
        if (picture_dirs) {
            date_dirs.insert(date_dirs.end(), picture_dirs->begin(), picture_dirs->end());
            delete picture_dirs;
        }

        auto video_dirs = fs::listdir(_video_path);
        if (video_dirs) {
            date_dirs.insert(date_dirs.end(), video_dirs->begin(), video_dirs->end());
            delete video_dirs;
        }
        // log::info("[%d] use %lld ms", __LINE__, time::ticks_ms() - t);

        // t = time::ticks_ms();
        _sort_date_vector(date_dirs);
        // log::info("[%d] use %lld ms", __LINE__, time::ticks_ms() - t);

        // log::info("found date dirs: ");
        // t = time::ticks_ms();
        for (auto date : date_dirs) {
            std::string picture_path = _picture_path + "/" + date;
            std::string video_path = _video_path + "/" + date;
            // log::info("date: %s picture_path:%s video_path:%s", date.c_str(), picture_path.c_str(), video_path.c_str());

            // find all picture
            if (fs::exists(picture_path)) {
                auto pictures = fs::listdir(picture_path);
                if (pictures) {
                    for (auto picture : *pictures) {
                        std::string picture_full_path = picture_path + "/" + picture;
                        if (_picture_path_is_valid(picture_full_path)) {
                            std::string thumbnail_path = picture_path + "/.thumbnail/" + picture;
                            PhotoVideoInfo new_info = {
                                .type = 0,
                                .date = date,
                                .path = picture_full_path,
                                .thumbnail_path = thumbnail_path};
                            push_video_photo_info(date, new_info);
                            // _print_video_photo_info(&new_info);
                        }
                    }
                    delete pictures;
                }
            }

            // find all video
            if (fs::exists(video_path)) {
                auto videos = fs::listdir(video_path);
                if (videos) {
                    for (auto video : *videos) {
                        std::string video_full_path = video_path + "/" + video;
                        if (_video_path_is_valid(video_full_path)) {
                            std::string thumbnail_path = video_path + "/.thumbnail/" + video;
                            size_t pos = thumbnail_path.rfind(".mp4");
                            if (pos != std::string::npos) {
                                thumbnail_path.replace(pos, 4, ".jpg");
                                PhotoVideoInfo new_info = {
                                    .type = 1,
                                    .date = date,
                                    .path = video_full_path,
                                    .thumbnail_path = thumbnail_path};
                                push_video_photo_info(date, new_info);
                                // _print_video_photo_info(&new_info);
                            }
                        }
                    }
                    delete videos;
                }
            }
        }
        // log::info("[%d] use %lld ms", __LINE__, time::ticks_ms() - t);

        return true;
    }

    std::list<std::pair<std::string, std::list<PhotoVideoInfo>>> *get_video_photo_list() {
        return _video_photo_list;
    }

    PhotoVideoInfo *find_next_photo_video(std::string date, std::string path) {
        bool return_next_item = false;
        auto iter = _video_photo_list->rbegin();
        for (; iter != _video_photo_list->rend(); iter++) {
            auto item = *iter;
            auto list = item.second;
            if (date == item.first || return_next_item) {
                auto list_iter = list.begin();
                for (; list_iter != list.end(); list_iter ++) {
                    auto &list_item = *list_iter;
                    if (return_next_item) {
                        return list_item.clone();
                    }

                    if (list_item.path == path) {
                        return_next_item = true;
                    }
                }
            }
        }

        return NULL;
    }

    PhotoVideoInfo *find_prev_photo_video(std::string date, std::string path) {
        bool return_next_item = false;
        auto iter = _video_photo_list->begin();
        for (; iter != _video_photo_list->end(); iter++) {
            auto item = *iter;
            auto list = item.second;
            if (date == item.first || return_next_item) {
                auto list_iter = list.rbegin();
                for (; list_iter != list.rend(); list_iter ++) {
                    auto &list_item = *list_iter;
                    if (return_next_item) {
                        return list_item.clone();
                    }

                    if (list_item.path == path) {
                        return_next_item = true;
                    }
                }
            }
        }

        return NULL;
    }
};

typedef struct {
    int disp_w;
    int disp_h;
    char *base_path;
    photos_list_t *photos;
    char *big_img_dir;
    char *big_img_filename;
    PhotoVideo *photo_video;
    display::Display *disp;
    image::Image *next_image;
    uint64_t next_image_try_keep_ms;
    video::Decoder *decoder;

    bool play_video;
    bool pause_video;
} priv_t;

priv_t priv;

static void config_next_display_image(image::Image *image, uint64_t try_keep_ms)
{
    if (priv.next_image) {
        delete priv.next_image;
        priv.next_image = NULL;
    }

    priv.next_image_try_keep_ms = try_keep_ms;
    priv.next_image = image;
}

static image::Image *get_next_display_image()
{
    return priv.next_image;
}

static uint64_t get_next_image_try_keep_ms()
{
    return priv.next_image_try_keep_ms;
}

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

void nv21_to_rgb_crop_resize(uint8_t *src_nv21, uint8_t *dst_rgb, int src_width, int src_height, int dst_width, int dst_height) {
    int src_frame_size = src_width * src_height;
    int uv_offset = src_frame_size;

    float src_aspect = (float)src_width / src_height;
    float dst_aspect = (float)dst_width / dst_height;

    int crop_width, crop_height, x_offset, y_offset;
    if (src_aspect > dst_aspect) {
        crop_width = (int)(src_height * dst_aspect);
        crop_height = src_height;
        x_offset = (src_width - crop_width) / 2;
        y_offset = 0;
    } else {
        crop_width = src_width;
        crop_height = (int)(src_width / dst_aspect);
        x_offset = 0;
        y_offset = (src_height - crop_height) / 2;
    }

    float x_ratio = (float)crop_width / dst_width;
    float y_ratio = (float)crop_height / dst_height;

    for (int j = 0; j < dst_height; j++) {
        for (int i = 0; i < dst_width; i++) {
            int src_x = x_offset + (int)(i * x_ratio);
            int src_y = y_offset + (int)(j * y_ratio);

            int y_index = src_y * src_width + src_x;
            int uv_index = uv_offset + (src_y / 2) * src_width + (src_x & ~1);

            int y = src_nv21[y_index];
            int v = src_nv21[uv_index];
            int u = src_nv21[uv_index + 1];

            int c = y - 16;
            int d = u - 128;
            int e = v - 128;

            int r = (298 * c + 409 * e + 128) >> 8;
            int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
            int b = (298 * c + 516 * d + 128) >> 8;

            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);

            int rgb_index = (j * dst_width + i) * 3;
            dst_rgb[rgb_index] = (uint8_t)r;
            dst_rgb[rgb_index + 1] = (uint8_t)g;
            dst_rgb[rgb_index + 2] = (uint8_t)b;
        }
    }
}

void nv21_resize(uint8_t *src_nv21, int src_width, int src_height,
                 uint8_t *dst_nv21, int dst_width, int dst_height) {
    int src_frame_size = src_width * src_height;
    int dst_frame_size = dst_width * dst_height;

    uint8_t *src_y_plane = src_nv21;
    uint8_t *src_uv_plane = src_nv21 + src_frame_size;
    uint8_t *dst_y_plane = dst_nv21;
    uint8_t *dst_uv_plane = dst_nv21 + dst_frame_size;

    int x_scale = (src_width << 16) / dst_width;
    int y_scale = (src_height << 16) / dst_height;

    for (int dy = 0; dy < dst_height; dy++) {
        int sy = (dy * y_scale) >> 16;
        for (int dx = 0; dx < dst_width; dx++) {
            int sx = (dx * x_scale) >> 16;
            dst_y_plane[dy * dst_width + dx] = src_y_plane[sy * src_width + sx];
        }
    }

    for (int dy = 0; dy < dst_height / 2; dy++) {
        int sy = ((dy * y_scale) >> 16) / 2;
        for (int dx = 0; dx < dst_width / 2; dx++) {
            int sx = ((dx * x_scale) >> 16) / 2;
            int src_uv_index = (sy * src_width + (sx * 2));

            uint8_t u = src_uv_plane[src_uv_index];
            uint8_t v = src_uv_plane[src_uv_index + 1];

            int dst_uv_index = (dy * dst_width + (dx * 2));
            dst_uv_plane[dst_uv_index] = u;
            dst_uv_plane[dst_uv_index + 1] = v;
        }
    }
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
        std::string src_path(path);
        size_t pos = src_path.rfind(".mp4");
        if (pos != std::string::npos) {
            try {
                if (priv.decoder) {
                    delete priv.decoder;
                    priv.decoder = NULL;
                }
                priv.decoder = new video::Decoder(src_path);
                err::check_null_raise(priv.decoder, "Decoder init failed!");
                auto ctx = priv.decoder->decode_video();
                if (ctx && ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
                    image::Image *img = ctx->image();
                    delete ctx;
                    if (img) {
                        image::Image thumbnail_img(128, 128, image::FMT_RGB888);
                        nv21_to_rgb_crop_resize((uint8_t *)img->data(), (uint8_t *)thumbnail_img.data(), img->width(), img->height(), thumbnail_img.width(), thumbnail_img.height());
                        thumbnail_img.save(thumbnail_path);
                        delete img;
                    } else {
                        log::error("decode video %s failed!\r\n", &src_path[0]);
                        return NULL;
                    }
                } else {
                    log::error("decode video %s failed!\r\n", &src_path[0]);
                    return NULL;
                }
                delete priv.decoder;
                priv.decoder = NULL;
            } catch (std::exception &e) {
                log::error("decode video %s failed!\r\n", &src_path[0]);
                return NULL;
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
        priv.disp = disp;
        image::Image *img = new image::Image(disp->width(), disp->height(), maix::image::FMT_YVU420SP);
        img->clear();
        config_next_display_image(img, 0);
    } else {
        priv.disp_w = 552;
        priv.disp_h = 368;
    }

#if 1
    priv.photo_video = new PhotoVideo("/maixapp/share/picture", "/maixapp/share/video");
    priv.photo_video->collect_video_photo();
    auto list = priv.photo_video->get_video_photo_list();
    auto iter = list->rbegin();
    for (; iter != list->rend(); iter++) {
        auto item = *iter;
        auto date = item.first;
        auto list = item.second;
        // log::info("\t[%s]", date.c_str());
        ui_photo_add_dir((char *)date.c_str());
        auto list_iter = list.begin();
        for (; list_iter != list.end(); list_iter ++) {
            auto list_item = *list_iter;
            log::info("path:%s", list_item.path.c_str());
            lv_image_dsc_t *dsc = load_thumbnail_image((char *)list_item.path.c_str(), (char *)list_item.thumbnail_path.c_str());
            if (dsc) {
                ui_photo_add_photo((char *)date.c_str(), (char *)list_item.path.c_str(), dsc, list_item.is_video());
                free_thumbnail_image(dsc);
            }
        }
    }

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

static void ui_set_video_first_image(char *dir_name, char *path)
{
    try {
        if (priv.decoder) {
            delete priv.decoder;
            priv.decoder = NULL;
        }
        priv.decoder = new video::Decoder(path);
        err::check_null_raise(priv.decoder, "Decoder init failed!");
        auto ctx = priv.decoder->decode_video();
        if (ctx && ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
            image::Image *img = ctx->image();
            if (img) {
                // uint64_t t = time::ticks_ms();
                image::Image *new_img = new image::Image(priv.disp->width(), priv.disp->height(), image::Format::FMT_YVU420SP);
                nv21_resize((uint8_t *)img->data(), img->width(), img->height(), (uint8_t *)new_img->data(),new_img->width(), new_img->height());
                config_next_display_image(new_img, ctx->duration_us() / 1000);
                ui_clear_video_bar();
                ui_set_video_bar_s(0, priv.decoder->duration());
                // delete new_img;     // delete auto
                delete img;
                // log::info("============================crop used:%lld", time::ticks_ms() - t);
                delete ctx;
            } else {
                delete ctx;
                goto _error;
            }
        } else {
            goto _error;
        }
        // delete priv.decoder;     // continue use
        // priv.decoder = NULL;
    } catch (std::exception &e) {
        goto _error;
    }

    return;
_error:
    log::error("decode video %s failed!\r\n", &path[0]);
    return;
}

static void play_video(void)
{
    if (!priv.decoder) {
        priv.decoder = new video::Decoder(priv. big_img_filename);
        err::check_null_raise(priv.decoder, "Decoder init failed!");
    }

    double new_seek = ui_get_video_bar_value() * priv.decoder->duration();
    double old_seek = priv.decoder->seek();
    if (abs(old_seek - new_seek) >= 1) {
        priv.decoder->seek(new_seek);
    }

    if (priv.decoder) {
        do {
            auto ctx = priv.decoder->decode_video();
            if (ctx) {
                if (ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
                    image::Image *img = ctx->image();
                    if (img) {
                        // uint64_t t = time::ticks_ms();
                        image::Image *new_img = new image::Image(priv.disp->width(), priv.disp->height(), image::Format::FMT_YVU420SP);
                        nv21_resize((uint8_t *)img->data(), img->width(), img->height(), (uint8_t *)new_img->data(),new_img->width(), new_img->height());
                        config_next_display_image(new_img, ctx->duration_us() / 1000);
                        ui_set_video_bar_s(priv.decoder->seek(), priv.decoder->duration());
                        // delete new_img;     // delete auto
                        delete img;
                        delete ctx;
                        break;
                    }
                } else {
                    delete ctx;
                    continue;
                }
            } else {
                delete priv.decoder;
                priv.decoder = NULL;
                priv.pause_video = true;
                priv.play_video = false;
                break;
            }
        } while (1);
    }
}

int app_loop(void)
{
    if (ui_get_touch_video_bar_release_flag()) {
        printf("release video bar\r\n");
        double value = ui_get_video_bar_value();
        if (priv.decoder) {
            priv.decoder->seek(value * priv.decoder->duration());
        }
        printf("percent:%f\r\n", value);
    }

    int view_flag = ui_get_view_flag();
    if (priv.play_video) {
        play_video();
    }

    if (view_flag == 4 && priv.pause_video && view_flag != 5) {
        ui_set_view_flag(5);
    }

    if (priv.disp) {
        image::Image *img = get_next_display_image();
        uint64_t try_keep_ms = get_next_image_try_keep_ms();
        if (img) {
            priv.disp->show(*img, image::FIT_COVER);
            // delete img;  // delete in config_next_display_image()
            // time::sleep_ms(try_keep_ms);
        }

        static uint64_t last_show_ms = time::ticks_ms();
        while (time::ticks_ms() - last_show_ms <= (try_keep_ms > 0 ? try_keep_ms : 10)) {
            time::sleep_ms(1);
        }
        last_show_ms = time::ticks_ms();
    }

    if (ui_get_touch_small_image_flag()) {
        if (!ui_get_bulk_delete_flag()) {
            if (!ui_touch_is_video_image_flag()) {
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
            } else {
                char *path = NULL, *dir_name = NULL;
                ui_get_touch_small_image_path(&dir_name, &path);

                update_char(&priv.big_img_dir, dir_name);
                update_char(&priv.big_img_filename, path);

                printf("load video info. path:%s\r\n", path);
                ui_set_video_first_image(dir_name, path);
                // jump
                ui_set_view_flag(5);
            }

        }
    }

    if (ui_get_video_view_pressed_flag()) {
        printf("video view pressed\r\n");
        int flag = ui_get_view_flag();
        switch (flag) {
        case 4:
            ui_set_view_flag(5);
            printf("pause..\r\n");
            priv.pause_video = true;
            priv.play_video = false;
            break;
        case 5:
            ui_set_view_flag(4);
            printf("play..\r\n");
            priv.play_video = true;
            priv.pause_video = false;
            break;
        default:break;
        }
    }

    if (ui_get_exit_flag()) {
        printf("exit!\r\n");
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
        log::info("touch right..");
        log::info("curr image dir:%s, file path:%s", priv.big_img_dir, priv.big_img_filename);
        PhotoVideoInfo *info = priv.photo_video->find_next_photo_video(priv.big_img_dir, priv.big_img_filename);
        if (info) {
            printf("next photo/video:%s\r\n", info->path.c_str());
            bool is_video = info->is_video();
            if (is_video) {
                update_char(&priv.big_img_dir, (char *)info->date.c_str());
                update_char(&priv.big_img_filename, (char *)info->path.c_str());
                printf("load video info. path:%s\r\n", priv.big_img_filename);
                ui_set_video_first_image(priv.big_img_dir, priv.big_img_filename);
                // jump
                ui_set_view_flag(5);
            } else {
                update_char(&priv.big_img_dir, (char *)info->date.c_str());
                update_char(&priv.big_img_filename, (char *)info->path.c_str());
                ui_set_big_image(priv.big_img_dir, priv.big_img_filename);
                ui_right_screen_update();
                ui_set_view_flag(2);
            }

            delete info;
        }  else {
            printf("The last photo is not found!\r\n");
        }
    }

    if (ui_get_touch_show_left_big_photo_flag()) {
        log::info("touch right..");
        log::info("curr image dir:%s, file path:%s", priv.big_img_dir, priv.big_img_filename);
        PhotoVideoInfo *info = priv.photo_video->find_prev_photo_video(priv.big_img_dir, priv.big_img_filename);
        if (info) {
            printf("prev photo/video:%s\r\n", info->path.c_str());
            bool is_video = info->is_video();
            if (is_video) {
                update_char(&priv.big_img_dir, (char *)info->date.c_str());
                update_char(&priv.big_img_filename, (char *)info->path.c_str());

                printf("load video info. path:%s\r\n", priv.big_img_filename);
                ui_set_video_first_image(priv.big_img_dir, priv.big_img_filename);
                // jump
                ui_set_view_flag(5);
            } else {
                update_char(&priv.big_img_dir, (char *)info->date.c_str());
                update_char(&priv.big_img_filename, (char *)info->path.c_str());

                ui_set_big_image(priv.big_img_dir, priv.big_img_filename);
                ui_right_screen_update();
                ui_set_view_flag(2);
            }

            delete info;
        } else {
            printf("The last photo is not found!\r\n");
        }
    }
    return 0;
}

int app_deinit(void)
{
    if (priv.decoder) {
        delete priv.decoder;
        priv.decoder = NULL;
    }

    if (priv.photo_video) {
        delete priv.photo_video;
        priv.photo_video = NULL;
    }
    return 0;
}