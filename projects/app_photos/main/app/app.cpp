#include "lvgl.h"
#include "app.hpp"
#include "stdio.h"
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_video.hpp"
#include "maix_display.hpp"
#include "maix_audio.hpp"
#include <list>
#include <sys/stat.h>
#include "sophgo_middleware.hpp"

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
                    return false;
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
            if (push_video_photo_list(new_item.second, new_info)) {
                _video_photo_list->push_back(new_item);
            }
        } else {
            auto iter = _video_photo_list->begin();
            for (; iter != _video_photo_list->end(); iter ++) {
                auto pair = iter;
                auto compare_res = _compare_date_string(date, pair->first);
                if (compare_res == 0) {
                    push_video_photo_list(pair->second, new_info);
                    break;
                } else if (compare_res < 0) {
                    std::pair<std::string, std::list<PhotoVideoInfo>> new_item = std::make_pair(date, std::list<PhotoVideoInfo>());
                    if (push_video_photo_list(new_item.second, new_info)) {
                        _video_photo_list->push_front(new_item);
                    }
                    break;
                } else {
                    if (std::next(iter) == _video_photo_list->end()) {
                        std::pair<std::string, std::list<PhotoVideoInfo>> new_item = std::make_pair(date, std::list<PhotoVideoInfo>());
                        if (push_video_photo_list(new_item.second, new_info)) {
                            _video_photo_list->push_front(new_item);
                        }
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
            // log::info("\t[%s] number:%d", date.c_str(), list.size());
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

        // flite the same date
        for (auto iter = date_dirs.begin(); iter != date_dirs.end(); iter ++) {
            auto &item = *iter;
            for (auto iter2 = std::next(iter); iter2 != date_dirs.end(); iter2 ++) {
                auto &item2 = *iter2;
                if (item2 == item) {
                    iter2 = date_dirs.erase(iter);
                }
            }
        }

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

    PhotoVideoInfo *find_prev_photo_video(std::string date, std::string path) {
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
};

#define DEFAULT_NEXT_KEEP_MS      (10)

typedef struct {
    int disp_w;
    int disp_h;
    char *base_path;
    photos_list_t *photos;
    char *big_img_dir;
    char *big_img_filename;
    PhotoVideo *photo_video;
    display::Display *disp;

    video::Decoder *decoder;
    audio::Player *audio_player;
    std::list<video::Context *> *audio_list;
    std::list<video::Context *> *video_list;

    bool next_is_frame;
    bool next_is_default_image;
    image::Image *default_next_image;
    image::Image *next_image;
    VIDEO_FRAME_INFO_S *next_frame;
    uint64_t next_keep_ms;

    bool find_first_pts;
    uint64_t first_play_ms, next_play_ms;
    uint64_t last_pts;

    bool play_video;
    bool pause_video;
} priv_t;

priv_t priv;

static void _audio_video_list_reset()
{
    if (priv.audio_list) {
        for (auto iter = priv.audio_list->begin(); iter != priv.audio_list->end(); iter++) {
            auto ctx = *iter;
            delete ctx;
            iter = priv.audio_list->erase(iter);
        }
    }

    if (priv.video_list) {
        for (auto iter = priv.video_list->begin(); iter != priv.video_list->end(); iter++) {
            auto ctx = *iter;
            delete ctx;
            iter = priv.video_list->erase(iter);
        }
    }
}

static void _audio_video_list_deinit()
{
    _audio_video_list_reset();

    if (priv.audio_list) {
        delete priv.audio_list;
        priv.audio_list = NULL;
    }

    if (priv.video_list) {
        delete priv.video_list;
        priv.video_list = NULL;
    }
}

static void _audio_video_list_init()
{
    _audio_video_list_deinit();
    priv.audio_list = new std::list<video::Context *>();
    priv.video_list = new std::list<video::Context *>();
    err::check_null_raise(priv.audio_list, "audio list init failed!");
    err::check_null_raise(priv.video_list, "video list init failed!");
}

static void config_default_display_image(image::Image *image)
{
    if (priv.default_next_image) {
        delete priv.default_next_image;
        priv.default_next_image = nullptr;
    }

    priv.default_next_image = image->copy();
}

static void release_last_show_frame(void)
{
    if (priv.next_frame) {
        free(priv.next_frame);
        priv.next_frame = nullptr;
        err::check_bool_raise(!mmf_vdec_free(0));
    }
}

static void try_show_frame(VIDEO_FRAME_INFO_S *frame, uint64_t next_keep_ms = 30)
{
    priv.next_is_frame = true;
    priv.next_is_default_image = false;
    priv.next_frame = frame;
    priv.next_keep_ms = next_keep_ms;
}

static void try_show_default_image(void)
{
    priv.next_is_frame = false;
    priv.next_is_default_image = true;
    priv.next_keep_ms = DEFAULT_NEXT_KEEP_MS;
}

// if img = nullptr, use last img
static void try_show_image(image::Image *img, uint64_t next_keep_ms = 30)
{
    if (img) {
        image::Image *next_img = (image::Image *)priv.next_image;
        if (next_img) {
            delete next_img;
            priv.next_image = nullptr;
            next_img = nullptr;
        }
        priv.next_image = img;
    }

    priv.next_is_frame = false;
    priv.next_is_default_image = false;
    priv.next_keep_ms = next_keep_ms;
}

static bool next_is_frame() {
    return priv.next_is_frame;
}

static image::Image *get_next_show_image()
{
    if (priv.next_is_default_image) {
        return priv.default_next_image;
    }
    return priv.next_image;
}

static VIDEO_FRAME_INFO_S *get_next_show_frame()
{
    return priv.next_frame;
}

static uint64_t get_next_keep_ms()
{
    return priv.next_keep_ms;
}

static void decoder_seek(double seek_ms)
{
    if (priv.decoder) {
        priv.decoder->seek(seek_ms);
        _audio_video_list_reset();
        priv.find_first_pts = false;
    }
}

static void decoder_release()
{
    if (priv.decoder) {
        release_last_show_frame();
        delete priv.decoder;
        priv.decoder = NULL;
    }
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

void nv21_to_bgra_crop_resize(uint8_t *src_nv21, uint8_t *dst_rgb, int src_width, int src_height, int dst_width, int dst_height) {
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

            int rgb_index = (j * dst_width + i) * 4;
            dst_rgb[rgb_index] = (uint8_t)b;
            dst_rgb[rgb_index + 1] = (uint8_t)g;
            dst_rgb[rgb_index + 2] = (uint8_t)r;
            dst_rgb[rgb_index + 3] = (uint8_t)0xff;
        }
    }
}

void nv21_resize(uint8_t *src_nv21, uint32_t src_width, uint32_t src_height,
                 uint8_t *dst_nv21, uint32_t dst_width, uint32_t dst_height) {
    float scale_x = (float)dst_width / src_width;
    float scale_y = (float)dst_height / src_height;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;

    uint32_t new_width = (uint32_t)(src_width * scale);
    uint32_t new_height = (uint32_t)(src_height * scale);
    uint32_t offset_x = (dst_width - new_width) / 2;
    uint32_t offset_y = (dst_height - new_height) / 2;

    memset(dst_nv21, 0, dst_width * dst_height);
    memset(dst_nv21 + dst_width * dst_height, 128, dst_width * dst_height / 2);

    uint8_t *src_y = src_nv21;
    uint8_t *dst_y = dst_nv21;
    for (uint32_t y = 0; y < new_height; y++) {
        uint32_t src_y_index = (uint32_t)(y / scale);
        for (uint32_t x = 0; x < new_width; x++) {
            uint32_t src_x_index = (uint32_t)(x / scale);
            dst_y[(y + offset_y) * dst_width + (x + offset_x)] = src_y[src_y_index * src_width + src_x_index];
        }
    }

    uint8_t *src_uv = src_nv21 + src_width * src_height;
    uint8_t *dst_uv = dst_nv21 + dst_width * dst_height;
    for (uint32_t y = 0; y < new_height / 2; y++) {
        uint32_t src_uv_y_index = (uint32_t)(y / scale);
        for (uint32_t x = 0; x < new_width / 2; x++) {
            uint32_t src_uv_x_index = (uint32_t)(x / scale);
            uint32_t src_index = src_uv_y_index * src_width + 2 * src_uv_x_index;
            uint32_t dst_index = (y + offset_y / 2) * dst_width + 2 * (x + offset_x / 2);

            dst_uv[dst_index] = src_uv[src_index];
            dst_uv[dst_index + 1] = src_uv[src_index + 1];
        }
    }
}


void nv21_resize_frame(uint8_t *y, uint8_t *uv, uint32_t src_width, uint32_t src_height,
                 uint8_t *dst_nv21, uint32_t dst_width, uint32_t dst_height) {
    float scale_x = (float)dst_width / src_width;
    float scale_y = (float)dst_height / src_height;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;

    uint32_t new_width = (uint32_t)(src_width * scale);
    uint32_t new_height = (uint32_t)(src_height * scale);
    uint32_t offset_x = (dst_width - new_width) / 2;
    uint32_t offset_y = (dst_height - new_height) / 2;

    memset(dst_nv21, 0, dst_width * dst_height);
    memset(dst_nv21 + dst_width * dst_height, 128, dst_width * dst_height / 2);

    uint8_t *src_y = y;
    uint8_t *dst_y = dst_nv21;
    for (uint32_t y = 0; y < new_height; y++) {
        uint32_t src_y_index = (uint32_t)(y / scale);
        for (uint32_t x = 0; x < new_width; x++) {
            uint32_t src_x_index = (uint32_t)(x / scale);
            dst_y[(y + offset_y) * dst_width + (x + offset_x)] = src_y[src_y_index * src_width + src_x_index];
        }
    }

    uint8_t *src_uv = uv;
    uint8_t *dst_uv = dst_nv21 + dst_width * dst_height;
    for (uint32_t y = 0; y < new_height / 2; y++) {
        uint32_t src_uv_y_index = (uint32_t)(y / scale);
        for (uint32_t x = 0; x < new_width / 2; x++) {
            uint32_t src_uv_x_index = (uint32_t)(x / scale);
            uint32_t src_index = src_uv_y_index * src_width + 2 * src_uv_x_index;
            uint32_t dst_index = (y + offset_y / 2) * dst_width + 2 * (x + offset_x / 2);

            dst_uv[dst_index] = src_uv[src_index];
            dst_uv[dst_index + 1] = src_uv[src_index + 1];
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
                decoder_release();
                priv.decoder = new video::Decoder(src_path);
                err::check_null_raise(priv.decoder, "Decoder init failed!");
                decoder_seek(0);
                auto ctx = priv.decoder->decode_video();
                if (ctx && ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
                    image::Image *img = ctx->image();
                    delete ctx;
                    if (img) {
                        thumbnail_img = new image::Image(128, 128, image::FMT_BGRA8888);
                        nv21_to_bgra_crop_resize((uint8_t *)img->data(), (uint8_t *)thumbnail_img->data(), img->width(), img->height(), thumbnail_img->width(), thumbnail_img->height());
                        thumbnail_img->save(thumbnail_path);
                        delete img;
                    } else {
                        log::error("decode video %s failed!\r\n", &src_path[0]);
                        return NULL;
                    }
                } else {
                    log::error("decode video %s failed!\r\n", &src_path[0]);
                    return NULL;
                }
                decoder_release();
            } catch (std::exception &e) {
                decoder_release();
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
        free((char *)img_dsc->data);
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
        memset((uint8_t *)img->data() + img->width() * img->height(), 128, img->width() * img->height() / 2);
        config_default_display_image(img);
        try_show_default_image();
        delete img;
    } else {
        priv.disp_w = 552;
        priv.disp_h = 368;
    }

    priv.photo_video = new PhotoVideo("/maixapp/share/picture", "/maixapp/share/video");
    priv.photo_video->collect_video_photo();
    // priv.photo_video->print_video_photo_list();

    _audio_video_list_init();
    // log::info("========= PUSH TO UI ==========");
    auto list = priv.photo_video->get_video_photo_list();
    auto iter = list->begin();
    for (; iter != list->end(); iter++) {
        auto &item = *iter;
        auto date = item.first;
        auto &info_list = item.second;
        log::info("\t[%s] number:%d", date.c_str(), info_list.size());
        ui_photo_add_dir((char *)date.c_str());
        auto list_iter = info_list.rbegin();
        for (; list_iter != info_list.rend();) {
            auto list_item = *list_iter;
            log::info("\tpath:%s", list_item.path.c_str());
            lv_image_dsc_t *dsc = load_thumbnail_image((char *)list_item.path.c_str(), (char *)list_item.thumbnail_path.c_str());
            if (dsc) {
                ui_photo_add_photo((char *)date.c_str(), (char *)list_item.path.c_str(), dsc, list_item.is_video());
                free_thumbnail_image(dsc);
                list_iter ++;
            } else {
                list_iter = std::make_reverse_iterator(info_list.erase(std::next(list_iter).base()));
            }
        }
    }
    // log::info("======= PUSH TO UI (END)========");

    ui_photo_print();
    ui_photo_list_screen_update();

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
    int retry_cnt = 3;
_retry:
    try {
        decoder_release();
        priv.decoder = new video::Decoder(path);
        decoder_seek(0);
        err::check_null_raise(priv.decoder, "Decoder init failed!");
        log::info("decoder width:%d height:%d", priv.decoder->width(), priv.decoder->height());

        if (priv.decoder->has_audio()) {
            if (priv.audio_player) {
                delete priv.audio_player;
                priv.audio_player = NULL;
            }

            int sample_rate = priv.decoder->audio_sample_rate();
            int channel = priv.decoder->audio_channels();
            audio::Format format = priv.decoder->audio_format();
            log::info("audio_sample_rate:%d audio_format:%d audio_channels:%d", sample_rate, channel, format);
            priv.audio_player = new audio::Player("", sample_rate, format, channel);
            err::check_null_raise(priv.audio_player, "Audio player init failed!");
        }

        auto ctx = priv.decoder->decode_video();
        if (ctx && ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
            image::Image *img = ctx->image();
            if (img) {
                // log::info("disp wxh:%dx%d image format:%s", priv.disp->width(), priv.disp->height(), image::fmt_names[img->format()].c_str());
                // uint64_t t = time::ticks_ms();
                image::Image *new_img = new image::Image(priv.disp->width(), priv.disp->height(), image::Format::FMT_YVU420SP);
                nv21_resize((uint8_t *)img->data(), img->width(), img->height(), (uint8_t *)new_img->data(),new_img->width(), new_img->height());
                // log::info("============================crop used:%lld", time::ticks_ms() - t);
                try_show_image(new_img, ctx->duration_us() / 1000);
                ui_clear_video_bar();
                ui_set_video_bar_s(0, priv.decoder->duration());
                // delete new_img;     // delete auto
                delete img;
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
    if (retry_cnt > 0) {
        retry_cnt --;
        time::sleep_ms(100);
        log::warn("decode video %s failed, retry...\r\n");
        goto _retry;
    }
    try_show_default_image();
    log::error("decode video %s failed!\r\n", &path[0]);
    return;
}

static void play_video(void)
{
    if (priv.play_video) {
        if (!priv.decoder) {
            priv.decoder = new video::Decoder(priv.big_img_filename);
            err::check_null_raise(priv.decoder, "Decoder init failed!");
            decoder_seek(0);
        }

        if (priv.decoder->has_audio()) {
            if (!priv.audio_player) {
                int sample_rate = priv.decoder->audio_sample_rate();
                int channel = priv.decoder->audio_channels();
                audio::Format format = priv.decoder->audio_format();
                log::info("audio_sample_rate:%d audio_format:%d audio_channels:%d", sample_rate, channel, format);
                priv.audio_player = new audio::Player("", sample_rate, format, channel);
                err::check_null_raise(priv.audio_player, "Audio player init failed!");
            }
        }

        double new_seek = ui_get_video_bar_value() * priv.decoder->duration();
        double old_seek = priv.decoder->seek();
        if (abs(old_seek - new_seek) >= 1) {
            decoder_seek(new_seek);
            log::info("config decoder seek :%f", new_seek);
        }

        video::Decoder *decoder = priv.decoder;
        if (decoder) {
            uint64_t t = time::ticks_ms();
            video::Context *ctx = NULL;
            do {
                while ((ctx = decoder->unpack()) != nullptr) {
                    if (ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
                        priv.video_list->push_back(ctx);
                        break;
                    } else if (ctx->media_type() == video::MEDIA_TYPE_AUDIO) {
                        priv.audio_list->push_back(ctx);
                    }
                }
            } while (priv.audio_list->size() < 1 && ctx);
            // log::info("unpack video/audio use %lld ms, video list size:%d, audio list size:%d", time::ticks_ms() - t, priv.video_list->size(), priv.audio_list->size());

            t = time::ticks_ms();
            std::list<video::Context *>::iterator iter;
            for(iter=priv.video_list->begin();iter!=priv.video_list->end();iter++) {
                video::Context *video_ctx = *iter;

                if (!priv.find_first_pts) {
                    priv.last_pts = video_ctx->pts();
                    priv.find_first_pts = true;
                    priv.first_play_ms = priv.next_play_ms = time::ticks_ms() - video::timebase_to_ms(video_ctx->timebase(), video_ctx->pts());
                }

                if (priv.last_pts == video_ctx->pts()) {
                    priv.last_pts += video_ctx->duration();
                    // log::info("[VIDEO] play pts:%.2f ms next_pts:%d curr wait:%lld need wait:%lld",
                    // video::timebase_to_ms(video_ctx->timebase(), video_ctx->pts()), priv.last_pts,
                    // (time::ticks_us() - priv.last_us) / 1000, video_ctx->duration_us() / 1000);

                    iter = priv.video_list->erase(iter);

                    void *data = video_ctx->get_raw_data();
                    if (data) {
                        size_t data_size = video_ctx->get_raw_data_size();
                        VDEC_STREAM_S stStream = {0};
                        stStream.pu8Addr = (CVI_U8 *)data;
                        stStream.u32Len = data_size;
                        stStream.u64PTS = video_ctx->pts();
                        stStream.bEndOfFrame = CVI_TRUE;
                        stStream.bEndOfStream = CVI_FALSE;
                        stStream.bDisplay = 1;
                        VIDEO_FRAME_INFO_S *frame = (VIDEO_FRAME_INFO_S *)malloc(sizeof(VIDEO_FRAME_INFO_S));
                        err::check_null_raise(frame, "video frame is null!");
                        memset(frame, 0, sizeof(VIDEO_FRAME_INFO_S));

                        release_last_show_frame();
                        err::check_bool_raise(!mmf_vdec_push_v2(0, &stStream));
                        err::check_bool_raise(!mmf_vdec_pop_v2(0, frame));
    #if 0
                        image::Image *new_img = new image::Image(priv.disp->width(), priv.disp->height(), image::Format::FMT_YVU420SP);
                        nv21_resize_frame(  (uint8_t *)frame->stVFrame.pu8VirAddr[0],
                                            (uint8_t *)frame->stVFrame.pu8VirAddr[1],
                                            frame->stVFrame.u32Width, frame->stVFrame.u32Height,
                                            (uint8_t *)new_img->data(),new_img->width(), new_img->height());
                        try_show_image(new_img, ctx->duration_us() / 1000);
                        // delete new_img;     // delete in next call config_next_display_image
                        err::check_bool_raise(!mmf_vdec_free(0));
                        free(frame);
    #else
                        try_show_frame(frame, video_ctx->duration_us() / 1000);
    #endif
                        ui_set_video_bar_s(priv.decoder->seek(), priv.decoder->duration());
                        priv.next_play_ms = priv.first_play_ms + video::timebase_to_ms(video_ctx->timebase(), video_ctx->pts());
                        free(data);
                    }

                    delete video_ctx;
                    break;
                }
            }
            // log::info("playback video use %lld ms, video list size:%d", time::ticks_ms() - t, priv.video_list->size());

            while (priv.audio_list->size() > 0) {
                video::Context *audio_ctx = *priv.audio_list->begin();
                if (audio_ctx) {
                    priv.audio_list->pop_front();
                    if (audio_ctx->media_type() == video::MEDIA_TYPE_AUDIO) {
                        Bytes *pcm = audio_ctx->get_pcm();
                        if (priv.audio_player) priv.audio_player->play(pcm);
                        delete pcm;
                    }
                    delete audio_ctx;
                }
            }
            // log::info("playback audio use %lld ms, audio list size:%d", time::ticks_ms() - t, priv.audio_list->size());

            if (ctx == nullptr && priv.video_list->size() == 0 && priv.audio_list->size() == 0) {
                try_show_image(nullptr);
                _audio_video_list_reset();
                ui_set_video_bar_s(0, priv.decoder->duration());
                decoder_release();
                priv.find_first_pts = false;
                priv.pause_video = true;
                priv.play_video = false;
            }
        }
    }

    // if (priv.found_frame) {
    //     mmf_vo_frame_push2(0, 0, 2, priv.video_frame);
    // }

    int view_flag = ui_get_view_flag();
    if (view_flag == 4 && priv.pause_video && view_flag != 5) {
        ui_set_view_flag(5);
    }

    if (view_flag == 0 && next_is_frame()) {
        try_show_default_image();
    }
}

int app_loop(void)
{
    if (ui_get_touch_video_bar_release_flag()) {
        printf("release video bar\r\n");
        double value = ui_get_video_bar_value();
        if (priv.decoder) {
            decoder_seek(value * priv.decoder->duration());
        }
        printf("percent:%f\r\n", value);

    }

    play_video();

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
            printf("The prev photo is not found!\r\n");
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
            printf("The next photo is not found!\r\n");
        }
    }

    if (priv.disp) {
        static uint64_t last_show_ms = time::ticks_ms();
        if (!next_is_frame()) {
            image::Image *img = get_next_show_image();
            uint64_t try_keep_ms = get_next_keep_ms();
            if (img) {
                priv.disp->show(*img, image::FIT_COVER);
            }

            while (time::ticks_ms() - last_show_ms <= (try_keep_ms > 0 ? try_keep_ms : 10)) {
                time::sleep_ms(1);
            }
            last_show_ms = time::ticks_ms();
        } else {
            VIDEO_FRAME_INFO_S *frame = get_next_show_frame();
            uint64_t try_keep_ms = get_next_keep_ms();
            if (frame) {
                mmf_vo_frame_push2(0, 0, 2, frame);
            }

            while (time::ticks_ms() - last_show_ms <= (try_keep_ms > 0 ? try_keep_ms : 10)) {
                time::sleep_ms(1);
            }
            last_show_ms = time::ticks_ms();
        }
    }

#if 0
    static uint64_t loop_ms = 0;
    if (time::ticks_ms() - loop_ms > 5) {
        log::info(" loop time: %lld", time::ticks_ms() - loop_ms);
    }
    loop_ms = time::ticks_ms();
#endif
    return 0;
}

int app_deinit(void)
{
    _audio_video_list_deinit();

    if (priv.audio_player) {
        delete priv.audio_player;
        priv.audio_player = NULL;
    }

    decoder_release();

    if (priv.photo_video) {
        delete priv.photo_video;
        priv.photo_video = NULL;
    }
    return 0;
}