#ifndef __458453489_APP_HPP__
#define __458453489_APP_HPP__

#include "lvgl.h"
#include "dbg.hpp"
#include <memory>
#include <tuple>
#include "maix_cmap.hpp"
#include "maix_tof100.hpp"

#include "calibration.hpp"

struct MapInfo {
    int x_ofs;
    int y_ofs;
    float x;
    float y;
} g_mapinfo;

struct CrosshairObj {
    lv_obj_t* h;
    lv_obj_t* v;
};

inline static const std::map<std::string, uint32_t> cmap_maps = {
    {"WhiteHot",    0},
    {"BlackHot",    1},
    {"Ironbow",     2},
    {"Night",       3},
    {"RedHot",      4},
    {"WhiteHotSD",  5},
    {"BlackHotSD",  6},
    {"RedHotSD",    7},
    {"Jet",         8},
};

inline static const std::vector<uint32_t> resolutions_list = {
    25,
    50,
    100
};

#define LV_OBJ_NEW_WITH_NULL(name) inline static lv_obj_t* g_##name = nullptr


inline static int g_screen_w = 0;
inline static int g_screen_h = 0;
// inline static lv_obj_t* g_base_screen = nullptr;
// inline static lv_obj_t* g_settings = nullptr;
inline static bool g_setting_status = true;
// inline static lv_obj_t* g_settings_bar = nullptr;
LV_OBJ_NEW_WITH_NULL(settings_bar);
LV_OBJ_NEW_WITH_NULL(base_screen);
LV_OBJ_NEW_WITH_NULL(settings);

LV_OBJ_NEW_WITH_NULL(cmap_setting);
LV_OBJ_NEW_WITH_NULL(reso_setting);
LV_OBJ_NEW_WITH_NULL(cmap_bar);
LV_OBJ_NEW_WITH_NULL(reso_bar);
inline static std::vector<lv_obj_t*> g_cmap_item_objs;
inline static std::vector<lv_obj_t*> g_reso_item_objs;

inline static int g_user_crosshair_x = -1;
inline static int g_user_crosshair_y = -1;
inline static bool g_user_crosshair_update = false;

inline static int label_pos_y = 64;
constexpr int label_pos_y_ofts = 24;

inline static bool g_fusion_mode{false};
LV_OBJ_NEW_WITH_NULL(fusion_setting);
LV_OBJ_NEW_WITH_NULL(clear_setting);

/* default cmap */
using namespace maix::ext_dev::cmap;
using namespace maix::ext_dev::tof100;
inline static Cmap g_cmap{Cmap::JET};
inline static Resolution g_res{Resolution::RES_50x50};

inline static bool cali_start = false;
inline static std::vector<std::pair<int, int>> cali_points;
inline static std::vector<lv_obj_t*> cali_labels_list;
inline static CameraImageInfo cam_onfo_o;
inline static bool cam_need_reset{false};

void cali2img()
{
    CaliPoint cam_p;
    std::tie(cam_p.x1, cam_p.y1) = cali_points[0];
    std::tie(cam_p.x2, cam_p.y2) = cali_points[1];
    println("cam point: (%d,%d) (%d, %d)", cam_p.x1, cam_p.y1, cam_p.x2, cam_p.y2);

    CaliPoint oth_p;
    std::tie(oth_p.x1, oth_p.y1) = cali_points[2];
    std::tie(oth_p.x2, oth_p.y2) = cali_points[3];
    println("oth point: (%d,%d) (%d, %d)", oth_p.x1, oth_p.y1, oth_p.x2, oth_p.y2);

    cali(cam_onfo_o.w, cam_onfo_o.h, cam_p, oth_p);

    println("caminfo: w:%d, h:%d, point(%d,%d), %dx%d", g_camera_img_info.w, g_camera_img_info.h, g_camera_img_info.x1, g_camera_img_info.y1, g_camera_img_info.ww, g_camera_img_info.hh);

    cam_need_reset = true;
}

void point_map_init(const int sensor_w, const int sensor_h)
{
    if (sensor_w < sensor_h) {
        panic("Unsupport [%dx%d]", sensor_w, sensor_h);
    }

    g_mapinfo.y = static_cast<float>(g_screen_h) / sensor_h;
    g_mapinfo.y_ofs = 0;
    g_mapinfo.x = g_mapinfo.y;
    float disp_w = g_mapinfo.x * sensor_w;
    g_mapinfo.x_ofs = (g_screen_w-static_cast<int>(::floor(disp_w))) / 2;

    cam_onfo_o.w = sensor_w;
    cam_onfo_o.h = sensor_h;
    cam_onfo_o.x1 = cam_onfo_o.y1 = 0;
    cam_onfo_o.ww = sensor_w - 1;
    cam_onfo_o.ww = sensor_h - 2;
}

CrosshairObj* draw_crosshair(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, lv_coord_t size, lv_color_t color)
{
    constexpr int width = 5;
    lv_obj_t* line_h = lv_obj_create(parent);
    lv_obj_set_size(line_h, size, width);
    lv_obj_set_style_bg_color(line_h, color, 0);
    lv_obj_set_style_pad_all(line_h, 0, 0);
    lv_obj_set_style_radius(line_h, 0, 0);
    lv_obj_remove_flag(line_h, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_side(line_h, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_opa(line_h, LV_OPA_100, LV_PART_MAIN);

    lv_obj_t* line_v = lv_obj_create(parent);
    lv_obj_set_size(line_v, width, size);
    lv_obj_set_style_bg_color(line_v, color, 0);
    lv_obj_set_style_pad_all(line_v, 0, 0);
    lv_obj_set_style_radius(line_v, 0, 0);
    lv_obj_remove_flag(line_v, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_side(line_v, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_opa(line_v, LV_OPA_100, LV_PART_MAIN);

    lv_obj_set_pos(line_h, x - size / 2, y - width / 2);

    lv_obj_set_pos(line_v, x - width / 2, y - size / 2);

    auto res = new CrosshairObj{};
    res->h = line_h;
    res->v = line_v;
    return res;
}

template<bool debug=false>
CrosshairObj* draw_sensor_crosshair(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    int show_x = static_cast<int>(x * g_mapinfo.x) + g_mapinfo.x_ofs;
    int show_y = static_cast<int>(y * g_mapinfo.y) + g_mapinfo.y_ofs;

    lv_color_t color = lv_color_make(r, g, b);
    lv_coord_t size = 32;

    if constexpr (debug) {
        println("[%s] show x:%d, y:%d", __PRETTY_FUNCTION__, show_x, show_y);
    }

    return draw_crosshair(g_base_screen, show_x, show_y, size, color);
}

void crosshair_obj_delete(CrosshairObj* ptr)
{
    if (ptr == nullptr) return;
    if (ptr->h != nullptr) {
        lv_obj_delete(ptr->h);
        ptr->h = nullptr;
    }
    if (ptr->v != nullptr) {
        lv_obj_delete(ptr->v);
        ptr->v = nullptr;
    }
}

LV_FONT_DECLARE(app_fonts);

template<typename T>
void upate_3crosshair_and_label(std::tuple<int,int,T> minp, std::tuple<int,int,T> maxp, std::tuple<int,int,T> centerp)
{
    constexpr uint8_t min_r = 255;
    constexpr uint8_t min_g = 0;
    constexpr uint8_t min_b = 0;
    constexpr uint8_t max_r = 0;
    constexpr uint8_t max_g = 0;
    constexpr uint8_t max_b = 255;
    constexpr uint8_t center_r = 255;
    constexpr uint8_t center_g = 255;
    constexpr uint8_t center_b = 255;
    const char* symbol = "mm";

    using COBJ = std::unique_ptr<CrosshairObj, void(*)(CrosshairObj*)>;
    static COBJ mincobj(nullptr, crosshair_obj_delete);
    static COBJ maxcobj(nullptr, crosshair_obj_delete);
    static COBJ centercobj(nullptr, crosshair_obj_delete);

    auto [min_x, min_y, minv] = minp;
    auto [max_x, max_y, maxv] = maxp;
    auto [center_x, center_y, centerv] = centerp;

    if (g_res == Resolution::RES_25x25) {
        min_x *= 2;
        min_y *= 2;
        max_x *= 2;
        max_y *= 2;
    }

    mincobj.reset(draw_sensor_crosshair(min_x, min_y, min_r, min_g, min_b));
    maxcobj.reset(draw_sensor_crosshair(max_x, max_y, max_r, max_g, max_b));

    check_ptr(g_base_screen);
    static lv_obj_t* min_label = lv_label_create(g_base_screen);
    static lv_obj_t* max_label = lv_label_create(g_base_screen);
    static lv_obj_t* center_label = lv_label_create(g_base_screen);
    static bool labels_init = false;

    if (!labels_init) {
        labels_init = true;
        check_ptr(min_label);
        check_ptr(max_label);
        check_ptr(center_label);
        static lv_style_t min_label_style;
        lv_style_init(&min_label_style);
        static auto max_label_style = min_label_style;
        static auto center_label_style = min_label_style;
        lv_style_set_text_color(&min_label_style, lv_color_make(min_r, min_g, min_b));
        lv_style_set_text_color(&max_label_style, lv_color_make(max_r, max_g, max_b));
        lv_style_set_text_color(&center_label_style, lv_color_make(center_r, center_g, center_b));
        lv_style_set_text_font(&min_label_style, &app_fonts);
        lv_style_set_text_font(&max_label_style, &app_fonts);
        lv_style_set_text_font(&center_label_style, &app_fonts);
        lv_obj_add_style(min_label, &min_label_style, 0);
        lv_obj_add_style(max_label, &max_label_style, 0);
        lv_obj_add_style(center_label, &center_label_style, 0);
        centercobj.reset(draw_sensor_crosshair(center_x, center_y, center_r, center_g, center_b));
        lv_obj_set_pos(min_label, 0, label_pos_y);
        label_pos_y += label_pos_y_ofts;
        lv_obj_set_pos(max_label, 0, label_pos_y);
        label_pos_y += label_pos_y_ofts;
        lv_obj_set_pos(center_label, 0, label_pos_y);
        label_pos_y += label_pos_y_ofts;
    }

    if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, uint16_t>) {
        lv_label_set_text_fmt(min_label, "min: %u%s", minv, symbol);
        lv_label_set_text_fmt(max_label, "max: %u%s", maxv, symbol);
        lv_label_set_text_fmt(center_label, "center: %u%s", centerv, symbol);
    } else if constexpr (std::is_same_v<T, float>) {
        lv_label_set_text_fmt(min_label, "min: %0.2f%s", minv, symbol);
        lv_label_set_text_fmt(max_label, "max: %0.2f%s", maxv, symbol);
        lv_label_set_text_fmt(center_label, "center: %0.2f%s", centerv, symbol);
    }
}

template<typename T>
void update_user_crosshair(const std::vector<std::vector<T>>& matrix)
{
    constexpr uint8_t user_r = 0;
    constexpr uint8_t user_g = 255;
    constexpr uint8_t user_b = 0;
    const char* symbol = "mm";

    if (matrix.empty()) return;
    if (matrix.at(0).empty()) return;

    static int show_x = -1;
    static int show_y = -1;
    static int sensor_x = -1;
    static int sensor_y = -1;

    check_ptr(g_base_screen);
    static lv_obj_t* user_label = lv_label_create(g_base_screen);
    static bool label_init = false;
    using COBJ = std::unique_ptr<CrosshairObj, void(*)(CrosshairObj*)>;
    static COBJ usercobj(nullptr, crosshair_obj_delete);

    if (!label_init) {
        label_init = true;
        check_ptr(user_label);
        static lv_style_t user_labe_style;
        lv_style_init(&user_labe_style);
        lv_style_set_text_color(&user_labe_style, lv_color_make(user_r, user_g, user_b));
        lv_style_set_text_font(&user_labe_style, &app_fonts);
        lv_obj_add_style(user_label, &user_labe_style, 0);
        lv_label_set_text(user_label, "user:Not selected");
        lv_obj_set_pos(user_label, 0, label_pos_y);
        label_pos_y += label_pos_y_ofts;
    }

    if (g_user_crosshair_update) {
        show_x = g_user_crosshair_x;
        show_y = g_user_crosshair_y;
        g_user_crosshair_update = false;

        sensor_x = static_cast<int>((static_cast<float>(show_x) - g_mapinfo.x_ofs) / g_mapinfo.x);
        sensor_y = static_cast<int>((static_cast<float>(show_y) - g_mapinfo.y_ofs) / g_mapinfo.y);

        // println("touch(%d,%d), map(%d,%d)", show_x, show_y, sensor_x, sensor_y);

        int matrix_h = static_cast<int>(matrix.size());
        int matrix_w = static_cast<int>(matrix[0].size());

        if (g_res == Resolution::RES_25x25) {
            matrix_w *= 2;
            matrix_h *= 2;
        }

        if (sensor_y > matrix_h || sensor_x > matrix_w || sensor_x < 0 || sensor_y < 0) {
            // eprintln("%dx%d but (%d,%d)", matrix_w, matrix_h, sensor_x, sensor_y);
            show_x = show_y = sensor_x = sensor_y = -1;
            usercobj.reset(nullptr);
            lv_label_set_text(user_label, "user:Not selected");
            return;
        }
        usercobj.reset(draw_sensor_crosshair(sensor_x, sensor_y, user_r, user_g, user_b));

        if (cali_start) {
            println("get cali point!");
            if (g_res == Resolution::RES_25x25) {
                cali_points.emplace_back(sensor_x*2, sensor_y*2);
            } else {
                cali_points.emplace_back(sensor_x, sensor_y);
            }
            if (cali_points.size() >= 4) {
                cali_start = false;
                println("get 4 cali point");
                cali2img();
                for (const auto& item : cali_labels_list) {
                    if (item != nullptr)
                        lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
                }
                cali_points.clear();
                save_cali_cfg();
                return;
            }
        }
    }

    if (show_x >=0 && show_y >= 0 && sensor_x >= 0 && sensor_y >= 0) {
        int __y = sensor_y;
        int __x = sensor_x;
        if (g_res == Resolution::RES_25x25) {
            __y/=2;
            __x/=2;
        }
        auto value = matrix[__y][__x];
        if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, uint16_t>) {
            lv_label_set_text_fmt(user_label, "user: %u%s", value, symbol);
        } else if constexpr (std::is_same_v<T, float>) {
            lv_label_set_text_fmt(user_label, "user: %0.2f%s", value, symbol);
        }
    }

}

/*******************************CB********************************/
void event_touch_exit_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        maix::app::set_exit_flag(true);
    }
}

void out_of_focus()
{
    check_ptr(g_settings);
    lv_obj_remove_flag(g_settings, LV_OBJ_FLAG_HIDDEN);
    g_setting_status = true;
    check_ptr(g_settings_bar);
    lv_obj_add_flag(g_settings_bar, LV_OBJ_FLAG_HIDDEN);
    check_ptr(g_cmap_bar);
    lv_obj_add_flag(g_cmap_bar, LV_OBJ_FLAG_HIDDEN);
    check_ptr(g_reso_bar);
    lv_obj_add_flag(g_reso_bar, LV_OBJ_FLAG_HIDDEN);
}

void out_of_focus_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        check_ptr(g_settings);
        if (!g_setting_status) {
            out_of_focus();
        } else {
            /* user crosshair */
            lv_point_t point;
            lv_indev_get_point(lv_indev_get_act(), &point);
            g_user_crosshair_x = point.x;
            g_user_crosshair_y = point.y;
            g_user_crosshair_update = true;
        }
    }
}

void settings_touch_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_setting_status) {
            lv_obj_add_flag(g_settings, LV_OBJ_FLAG_HIDDEN);
            g_setting_status = false;
            lv_obj_remove_flag(g_settings_bar, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void cmap_setting_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        check_ptr(g_cmap_bar);
        lv_obj_remove_flag(g_cmap_bar, LV_OBJ_FLAG_HIDDEN);
    }
}

void reso_setting_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        check_ptr(g_reso_bar);
        lv_obj_remove_flag(g_reso_bar, LV_OBJ_FLAG_HIDDEN);
    }
}

void cmap_item_cb(lv_event_t * e)
{
    auto obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    check_ptr(obj);
    auto label = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
    check_ptr(obj);
    const char* label_text = lv_label_get_text(label);
    check_ptr(obj);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        const std::string key(label_text);
        auto iter = cmap_maps.find(key);
        if (iter == cmap_maps.cend())
            return;
        auto [_, index] = *iter;
        // println("%s:%d", label_text, index);
        for (const auto item : g_cmap_item_objs) {
            if (item == nullptr) continue;
            lv_obj_remove_state(item, LV_STATE_CHECKED);
        }
        lv_obj_add_state(obj, LV_STATE_CHECKED);
        g_cmap = static_cast<Cmap>(index);
        out_of_focus();
    }
}

void reso_item_cb(lv_event_t * e)
{
    auto obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
    check_ptr(obj);
    auto label = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
    check_ptr(obj);
    const int reso = (int)(uintptr_t)lv_obj_get_user_data(label);
    println("click reso: %d", reso);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        for (const auto item: g_reso_item_objs) {
            if (item == nullptr) continue;
            lv_obj_remove_state(item, LV_STATE_CHECKED);
        }
        lv_obj_add_state(obj, LV_STATE_CHECKED);
        g_res = static_cast<Resolution>(reso);
        out_of_focus();
    }
}

void fusion_setting_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        check_ptr(g_cmap_setting);
        check_ptr(g_reso_setting);
        check_ptr(g_fusion_setting);
        check_ptr(g_clear_setting);
        lv_obj_add_flag(g_cmap_setting, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_reso_setting, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_fusion_setting, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(g_clear_setting, LV_OBJ_FLAG_HIDDEN);

        println("Set reso to 50x50");
        g_res = Resolution::RES_50x50;

        println("Try to load cfg");
        if (load_cali_cfg() >= 0) {
            cam_need_reset = true;
            g_fusion_mode = true;
            out_of_focus();
            return;
        }

        println("does not have cfg, cali");
        std::vector<lv_obj_t*>* label_list = &cali_labels_list;
        for (const auto& item : *label_list) {
            lv_obj_remove_flag(item, LV_OBJ_FLAG_HIDDEN);
        }
        cali_start = true;
        g_fusion_mode = true;
        out_of_focus();
    }
}

void clear_setting_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        clear_cali_cfg();
        check_ptr(g_clear_setting);
        lv_obj_add_flag(g_clear_setting, LV_OBJ_FLAG_HIDDEN);
        check_ptr(g_base_screen);
        lv_obj_t * msgbox = lv_msgbox_create(g_base_screen);
        lv_msgbox_add_text(msgbox, "Restart to apply.");
        lv_msgbox_add_close_button(msgbox);
        lv_obj_set_width(msgbox, 200);
        lv_obj_center(msgbox);
        out_of_focus();
    }
}
/*******************************UI********************************/

void ui_prefix_init()
{
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
    lv_display_set_color_format(NULL, LV_COLOR_FORMAT_ARGB8888);
    lv_screen_load(lv_layer_top());
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);

    g_base_screen = lv_screen_active();

    lv_obj_set_style_pad_all(g_base_screen, 0, 0);
    lv_obj_add_flag(g_base_screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_base_screen, out_of_focus_cb, LV_EVENT_CLICKED, NULL);
}

LV_IMG_DECLARE(img_return);
void ui_exit_init()
{
    check_ptr(g_base_screen);

    auto obj = lv_obj_create(g_base_screen);
    check_ptr(obj);
    lv_obj_set_size(obj, img_return.header.w*2, img_return.header.h*2);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(obj, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(obj, event_touch_exit_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *img = lv_image_create(obj);
    lv_image_set_src(img, &img_return);
    lv_obj_center(img);
}

LV_IMG_DECLARE(img_option);
void ui_settings_init()
{
    check_ptr(g_base_screen);
    auto obj = lv_obj_create(g_base_screen);
    check_ptr(obj);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_size(obj, img_option.header.w*2, img_option.header.h*2);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(obj, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(obj, settings_touch_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *img = lv_image_create(obj);
    lv_image_set_src(img, &img_option);
    lv_obj_center(img);

    g_settings = obj;
    lv_obj_remove_flag(g_settings, LV_OBJ_FLAG_HIDDEN);
    g_setting_status = true;

    // println("screen w:%d, h:%d", g_screen_w, g_screen_h);

    auto settings_bar = lv_obj_create(g_base_screen);
    check_ptr(settings_bar);
    lv_obj_set_style_pad_all(settings_bar, 0, 0);
    lv_obj_set_size(settings_bar, lv_pct(100), img_option.header.h*2);
    lv_obj_set_style_radius(settings_bar, 0, 0);
    // lv_obj_remove_flag(settings_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(settings_bar, lv_color_hex(0x0), 0);
    // lv_obj_set_style_bg_color(settings_bar, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(settings_bar, LV_BORDER_SIDE_NONE, 0);
    lv_obj_align(settings_bar, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_remove_flag(settings_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(settings_bar, LV_OPA_100, LV_PART_MAIN);

    g_settings_bar = settings_bar;
    lv_obj_add_flag(g_settings_bar, LV_OBJ_FLAG_HIDDEN);

    auto cmap_setting = lv_obj_create(g_settings_bar);
    check_ptr(cmap_setting);
    lv_obj_set_style_pad_all(cmap_setting, 0, 0);
    lv_obj_set_size(cmap_setting, lv_pct(33), lv_pct(100));
    lv_obj_set_style_radius(cmap_setting, 0, 0);
    lv_obj_remove_flag(cmap_setting, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(cmap_setting, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_color(cmap_setting, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(cmap_setting, LV_BORDER_SIDE_NONE, 0);
    lv_obj_align(cmap_setting, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(cmap_setting, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(cmap_setting, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(cmap_setting, cmap_setting_cb, LV_EVENT_CLICKED, nullptr);
    auto cmap_setting_label = lv_label_create(cmap_setting);
    check_ptr(cmap_setting_label);
    lv_label_set_text_static(cmap_setting_label, "cmap");
    lv_obj_center(cmap_setting_label);
    g_cmap_setting = cmap_setting;

    auto resolution_setting = lv_obj_create(g_settings_bar);
    check_ptr(resolution_setting);
    lv_obj_set_style_pad_all(resolution_setting, 0, 0);
    lv_obj_set_size(resolution_setting, lv_pct(33), lv_pct(100));
    lv_obj_set_style_radius(resolution_setting, 0, 0);
    lv_obj_remove_flag(resolution_setting, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(resolution_setting, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_color(resolution_setting, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(resolution_setting, LV_BORDER_SIDE_NONE, 0);
    lv_obj_align_to(resolution_setting, cmap_setting, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_add_flag(resolution_setting, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(resolution_setting, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(resolution_setting, reso_setting_cb, LV_EVENT_CLICKED, nullptr);
    auto resolution_setting_label = lv_label_create(resolution_setting);
    check_ptr(resolution_setting_label);
    lv_label_set_text_static(resolution_setting_label, "resolution");
    lv_obj_center(resolution_setting_label);
    g_reso_setting = resolution_setting;

    auto fusion_setting = lv_obj_create(g_settings_bar);
    check_ptr(fusion_setting);
    lv_obj_set_style_pad_all(fusion_setting, 0, 0);
    lv_obj_set_size(fusion_setting, lv_pct(33), lv_pct(100));
    lv_obj_set_style_radius(fusion_setting, 0, 0);
    lv_obj_remove_flag(fusion_setting, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(fusion_setting, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_color(fusion_setting, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(fusion_setting, LV_BORDER_SIDE_NONE, 0);
    lv_obj_align_to(fusion_setting, resolution_setting, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_add_flag(fusion_setting, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(fusion_setting, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(fusion_setting, fusion_setting_cb, LV_EVENT_CLICKED, nullptr);
    auto fusion_setting_label = lv_label_create(fusion_setting);
    check_ptr(fusion_setting_label);
    lv_label_set_text_static(fusion_setting_label, "fusion");
    lv_obj_center(fusion_setting_label);
    g_fusion_setting = fusion_setting;

    auto clear_setting = lv_obj_create(g_settings_bar);
    check_ptr(clear_setting);
    lv_obj_set_style_pad_all(clear_setting, 0, 0);
    lv_obj_set_size(clear_setting, lv_pct(33), lv_pct(100));
    lv_obj_set_style_radius(clear_setting, 0, 0);
    lv_obj_remove_flag(clear_setting, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(clear_setting, lv_color_hex(0x0), 0);
    lv_obj_set_style_bg_color(clear_setting, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(clear_setting, LV_BORDER_SIDE_NONE, 0);
    lv_obj_align_to(clear_setting, resolution_setting, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_add_flag(clear_setting, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(clear_setting, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_event_cb(clear_setting, clear_setting_cb, LV_EVENT_CLICKED, nullptr);
    auto clear_setting_label = lv_label_create(clear_setting);
    check_ptr(clear_setting_label);
    lv_label_set_text_static(clear_setting_label, "clear");
    lv_obj_center(clear_setting_label);
    lv_obj_add_flag(clear_setting, LV_OBJ_FLAG_HIDDEN);
    g_clear_setting = clear_setting;

    {
        static std::vector<lv_obj_t*> step_labels;
        lv_obj_t* cali_step1_label = lv_label_create(g_base_screen);
        check_ptr(cali_step1_label);
        lv_label_set_text_static(cali_step1_label, "Step1: Click the coordinates of the upper\n\tleft corner of the object in the camera image.");
        lv_obj_set_style_pad_all(cali_step1_label, 0, 0);
        lv_obj_set_style_radius(cali_step1_label, 0, 0);
        lv_obj_remove_flag(cali_step1_label, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(cali_step1_label, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(cali_step1_label, LV_BORDER_SIDE_NONE, 0);
        lv_obj_align(cali_step1_label, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_opa(cali_step1_label, LV_OPA_100, LV_PART_MAIN);
        step_labels.push_back(cali_step1_label);

        lv_obj_t* cali_step2_label = lv_label_create(g_base_screen);
        check_ptr(cali_step2_label);
        lv_label_set_text_static(cali_step2_label, "Step2: Click the coordinates of the upper\n\tright corner of the object in the camera image.");
        lv_obj_set_style_pad_all(cali_step2_label, 0, 0);
        lv_obj_set_style_radius(cali_step2_label, 0, 0);
        lv_obj_remove_flag(cali_step2_label, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(cali_step2_label, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(cali_step2_label, LV_BORDER_SIDE_NONE, 0);
        lv_obj_align_to(cali_step2_label, cali_step1_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_obj_set_style_opa(cali_step2_label, LV_OPA_100, LV_PART_MAIN);
        step_labels.push_back(cali_step2_label);

        lv_obj_t* cali_step3_label = lv_label_create(g_base_screen);
        check_ptr(cali_step3_label);
        lv_label_set_text_static(cali_step3_label, "Step3: Click on the coordinates of the upper\n\tleft corner of the object in another image.");
        lv_obj_set_style_pad_all(cali_step3_label, 0, 0);
        lv_obj_set_style_radius(cali_step3_label, 0, 0);
        lv_obj_remove_flag(cali_step3_label, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(cali_step3_label, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(cali_step3_label, LV_BORDER_SIDE_NONE, 0);
        lv_obj_align_to(cali_step3_label, cali_step2_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_obj_set_style_opa(cali_step3_label, LV_OPA_100, LV_PART_MAIN);
        step_labels.push_back(cali_step3_label);

        lv_obj_t* cali_step4_label = lv_label_create(g_base_screen);
        check_ptr(cali_step4_label);
        lv_label_set_text_static(cali_step4_label, "Step4: Click on the coordinates of the upper\n\tright corner of the object in another image.");
        lv_obj_set_style_pad_all(cali_step4_label, 0, 0);
        lv_obj_set_style_radius(cali_step4_label, 0, 0);
        lv_obj_remove_flag(cali_step4_label, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(cali_step4_label, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(cali_step4_label, LV_BORDER_SIDE_NONE, 0);
        lv_obj_align_to(cali_step4_label, cali_step3_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_obj_set_style_opa(cali_step4_label, LV_OPA_100, LV_PART_MAIN);
        step_labels.push_back(cali_step4_label);

        for (const auto& item : step_labels) {
            lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_set_user_data(fusion_setting, (void*)&step_labels);

        cali_start = false;
        cali_points.clear();
        cali_labels_list = step_labels;
    }

    // int cmap_num = static_cast<int>(cmap_maps.size());
    int cmap_item_w = 16*8;

    auto cmap_bar = lv_obj_create(g_base_screen);
    check_ptr(cmap_bar);
    lv_obj_set_style_pad_all(cmap_bar, 0, 0);
    lv_obj_set_size(cmap_bar, lv_pct(100), img_option.header.h*2);
    lv_obj_set_style_radius(cmap_bar, 0, 0);
    // lv_obj_remove_flag(cmap_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(cmap_bar, lv_color_hex(0x0), 0);
    // lv_obj_set_style_bg_color(cmap_bar, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(cmap_bar, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_line_width(cmap_bar, 0, 0);
    lv_obj_set_style_outline_width(cmap_bar, 0, 0);
    // lv_obj_align(cmap_bar, LV_ALIGN_BOTTOM_LEFT, 0, 0);/
    lv_obj_align_to(cmap_bar, g_settings_bar, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_remove_flag(cmap_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(cmap_bar, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_flag(cmap_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(cmap_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_hor(cmap_bar, 1, 0);

    int reso_item_w = 16*16;
    auto reso_bar = lv_obj_create(g_base_screen);
    check_ptr(reso_bar);
    lv_obj_set_style_pad_all(reso_bar, 0, 0);
    lv_obj_set_size(reso_bar, lv_pct(100), img_option.header.h*2);
    lv_obj_set_style_radius(reso_bar, 0, 0);
    // lv_obj_remove_flag(reso_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(reso_bar, lv_color_hex(0x0), 0);
    // lv_obj_set_style_bg_color(reso_bar, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
    lv_obj_set_style_border_side(reso_bar, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_line_width(reso_bar, 0, 0);
    lv_obj_set_style_outline_width(reso_bar, 0, 0);
    // lv_obj_align(reso_bar, LV_ALIGN_BOTTOM_LEFT, 0, 0);/
    lv_obj_align_to(reso_bar, g_settings_bar, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_remove_flag(reso_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_opa(reso_bar, LV_OPA_100, LV_PART_MAIN);
    lv_obj_add_flag(reso_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(reso_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_hor(reso_bar, 1, 0);

    g_cmap_bar = cmap_bar;
    g_reso_bar = reso_bar;

    {
        lv_obj_t* prev_item = nullptr;
        for (const auto& [name, index] : cmap_maps) {
            auto item = lv_obj_create(cmap_bar);
            check_ptr(item);
            lv_obj_set_style_pad_all(item, 0, 0);
            lv_obj_set_size(item, cmap_item_w, lv_pct(100));
            lv_obj_set_style_radius(item, 0, 0);
            lv_obj_remove_flag(item, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x0), 0);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
            lv_obj_set_style_border_side(item, LV_BORDER_SIDE_NONE, 0);
            if (prev_item == nullptr) {
                lv_obj_align(item, LV_ALIGN_LEFT_MID, 0, 0);
            } else {
                lv_obj_align_to(item, prev_item, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
            }
            prev_item = item;
            lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(item, LV_OPA_100, LV_PART_MAIN);
            // lv_label_set_text(item, name.c_str());
            lv_obj_t* item_label = lv_label_create(item);
            check_ptr(item);
            lv_obj_set_style_pad_all(item, 0, 0);
            lv_label_set_text(item_label, name.c_str());
            lv_obj_center(item_label);
            lv_obj_add_event_cb(item, cmap_item_cb, LV_EVENT_CLICKED, item_label);
            g_cmap_item_objs.push_back(item);
            if (index == static_cast<uint32_t>(g_cmap)) {
                println("found default cmap [%s,%d]", name.c_str(), index);
                lv_obj_add_state(item, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(item, LV_STATE_CHECKED);
            }
        }
    }

    {
        lv_obj_t* prev_item = nullptr;
        for (const auto& reso : resolutions_list) {
            auto item = lv_obj_create(reso_bar);
            check_ptr(item);
            lv_obj_set_style_pad_all(item, 0, 0);
            lv_obj_set_size(item, reso_item_w, lv_pct(100));
            lv_obj_set_style_radius(item, 0, 0);
            lv_obj_remove_flag(item, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x0), 0);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(item, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
            lv_obj_set_style_border_side(item, LV_BORDER_SIDE_NONE, 0);
            if (prev_item == nullptr) {
                lv_obj_align(item, LV_ALIGN_LEFT_MID, 0, 0);
            } else {
                lv_obj_align_to(item, prev_item, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
            }
            prev_item = item;
            lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_opa(item, LV_OPA_100, LV_PART_MAIN);
            // lv_label_set_text(item, name.c_str());
            lv_obj_t* item_label = lv_label_create(item);
            check_ptr(item);
            lv_obj_set_style_pad_all(item, 0, 0);
            lv_label_set_text_fmt(item_label, "Res%dx%d", reso, reso);
            lv_obj_set_user_data(item_label, (void*)(uintptr_t)reso);
            lv_obj_center(item_label);
            lv_obj_add_event_cb(item, reso_item_cb, LV_EVENT_CLICKED, item_label);
            g_reso_item_objs.push_back(item);
            if (reso == static_cast<uint32_t>(g_res)) {
                println("found default resolution [%dx%d]", reso, reso);
                lv_obj_add_state(item, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(item, LV_STATE_CHECKED);
            }
        }
    }
}


void ui_total_init(int w, int h, bool device_exist=true)
{
    if (w <=0 || h <= 0) {
        panic("Screen width and height parameters are wrong! w:%d,h:%d", w, h);
    }
    g_screen_w = w;
    g_screen_h = h;

    ui_prefix_init();
    ui_exit_init();

    if (!device_exist) return;

    ui_settings_init();
}

void ui_total_deinit()
{
    if (g_base_screen != nullptr)
        lv_obj_delete(g_base_screen);
}



#endif // __458453489_APP_HPP__