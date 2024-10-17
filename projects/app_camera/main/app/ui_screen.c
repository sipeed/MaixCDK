#include "lvgl.h"
#include "stdio.h"
#include "ui_utils.h"
#include "ui_screen.h"
#include "ui_event_handler.h"

typedef struct {
    int max_w;
    int max_h;
    int left_screen_w_pct;
    int right_screen_w_pct;
    int adjust_screen_w_pct;
    int delay_screen_w_pct;
    int resolution_screen_w_pct;
    int shutter_screen_w_pct;
    int iso_screen_w_pct;
    int ev_screen_w_pct;
    int wb_screen_w_pct;

    ui_camera_config_t cam_cfg;
} priv_t;

static priv_t priv;

lv_obj_t *g_camera_video_button;
lv_obj_t *g_start_snap_button;
lv_obj_t *g_exit_button;
lv_obj_t *g_delay_button;
lv_obj_t *g_resolution_button;
lv_obj_t *g_menu_button;
lv_obj_t *g_delay_setting;
lv_obj_t *g_resolution_setting;
lv_obj_t *g_menu_setting;

lv_obj_t *g_shutter_button;
lv_obj_t *g_iso_button;
lv_obj_t *g_ev_button;
lv_obj_t *g_wb_button;

lv_obj_t *g_shutter_setting;
lv_obj_t *g_iso_setting;
lv_obj_t *g_ev_setting;
lv_obj_t *g_wb_setting;
lv_obj_t *g_small_img;
lv_obj_t *g_big_img;
lv_obj_t *g_center_img;

lv_obj_t *g_video_running_screen;
lv_obj_t *g_focus_button;
lv_obj_t *g_light_button;
lv_obj_t *g_raw_button;
lv_obj_t *g_shutter_plus_minus_button;
lv_obj_t *g_iso_plus_minus_button;

LV_IMG_DECLARE(img_delay);
LV_IMG_DECLARE(img_exit);
LV_IMG_DECLARE(img_option);
LV_IMG_DECLARE(img_resolution);
LV_IMG_DECLARE(img_photo);
LV_IMG_DECLARE(img_camera);
LV_IMG_DECLARE(img_video);
LV_IMG_DECLARE(img_photo_release);
LV_IMG_DECLARE(img_photo_clicked);
LV_IMG_DECLARE(img_video_ready);
LV_IMG_DECLARE(img_video_stop);
LV_IMG_DECLARE(img_small_recording);
LV_IMG_DECLARE(img_focus);
LV_IMG_DECLARE(img_raw);
LV_IMG_DECLARE(img_light_on);
LV_IMG_DECLARE(img_light_off);

extern void event_touch_exit_cb(lv_event_t * e);
extern void event_touch_delay_cb(lv_event_t * e);
extern void event_touch_resolution_cb(lv_event_t * e);
extern void event_touch_option_cb(lv_event_t * e);
extern void event_touch_video_camera_cb(lv_event_t * e);
extern void event_touch_start_cb(lv_event_t * e);
extern void event_touch_small_img_cb(lv_event_t * e);
extern void event_touch_shutter_cb(lv_event_t * e);
extern void event_touch_iso_cb(lv_event_t * e);
extern void event_touch_ev_cb(lv_event_t * e);
extern void event_touch_wb_cb(lv_event_t * e);
extern void event_iso_bar_update_cb(lv_event_t * e);
extern void event_shutter_bar_update_cb(lv_event_t * e);
extern void event_ev_bar_update_cb(lv_event_t * e);
extern void event_wb_bar_update_cb(lv_event_t * e);
extern void event_touch_shutter_to_auto_cb(lv_event_t * e);
extern void event_touch_wb_to_auto_cb(lv_event_t * e);
extern void event_touch_iso_to_auto_cb(lv_event_t * e);
extern void event_touch_ev_to_auto_cb(lv_event_t * e);
extern void event_touch_wb_to_auto_cb(lv_event_t * e);
extern void event_touch_select_delay_cb(lv_event_t * e);
extern void event_touch_select_resolution_cb(lv_event_t * e);
extern void event_touch_big_img_cb(lv_event_t * e);
extern void event_touch_focus_cb(lv_event_t * e);
static void priv_init(void)
{
    priv.max_w = 552;
    priv.max_h = 368;
    priv.left_screen_w_pct = 10;        // %
    priv.right_screen_w_pct = 16;       // %
    priv.adjust_screen_w_pct = 10;      // %
    priv.delay_screen_w_pct = 10;       // %
    priv.resolution_screen_w_pct = 20;  // %
    priv.shutter_screen_w_pct = 10;     // %
    priv.iso_screen_w_pct = 10;         // %
    priv.ev_screen_w_pct = 10;          // %
    priv.wb_screen_w_pct = 10;          // %

    double exposure_time_table[] = {1.0/960,1.0/720,1.0/480, 1.0/360, 1.0/240, 1.0/180, 1.0/120, 1.0/90, 1.0/60, 1.0/40, 1.0/30, 1.0/20, 1.0/15, 1.0/10, 1.0/8, 1.0/5, 1.0/3, 1.0/2, 2.0/3, 1, 1.5, 2, 3, 5, 8, 10, 15, 20, 30, 40, 60};
    memcpy(priv.cam_cfg.exposure_time_table, exposure_time_table, sizeof(priv.cam_cfg.exposure_time_table));
    priv.cam_cfg.exposure_time_default = 3000;
    priv.cam_cfg.exposure_time_max = priv.cam_cfg.exposure_time_table[sizeof(priv.cam_cfg.exposure_time_table) / sizeof(priv.cam_cfg.exposure_time_table[0]) - 1] * 1000000;
    priv.cam_cfg.exposure_time_min = priv.cam_cfg.exposure_time_table[0] * 1000000;

    double iso_table[] = {100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400};
    memcpy(priv.cam_cfg.iso_table, iso_table, sizeof(priv.cam_cfg.iso_table));
    priv.cam_cfg.iso_max = priv.cam_cfg.iso_table[sizeof(priv.cam_cfg.iso_table) / sizeof(priv.cam_cfg.iso_table[0]) - 1];
    priv.cam_cfg.iso_min = priv.cam_cfg.iso_table[0];
    priv.cam_cfg.iso_default = 100;
}

void ui_camera_config_read(ui_camera_config_t *cfg)
{
    if (!cfg) return;
    memcpy(cfg, &priv.cam_cfg, sizeof(ui_camera_config_t));
}

void ui_camera_config_update(ui_camera_config_t *cfg)
{
    if (!cfg) return;
    memcpy(&priv.cam_cfg, cfg, sizeof(ui_camera_config_t));
}

void ui_click_raw_button()
{
    lv_obj_send_event(g_raw_button, LV_EVENT_CLICKED, (void *)-1);
    if (lv_obj_has_state(g_raw_button, LV_STATE_CHECKED)) {
        lv_obj_remove_state(g_raw_button, LV_STATE_CHECKED);
    } else {
        lv_obj_add_state(g_raw_button, LV_STATE_CHECKED);
    }
}

static void left_screen_init(void)
{
    int w = priv.left_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scr, lv_pct(w), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    // lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_hor(scr, 1, 0);

    lv_obj_t *img;
    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(25));
        lv_obj_set_pos(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_exit_cb, LV_EVENT_CLICKED, NULL);
        g_exit_button = obj;

        img = lv_image_create(obj);
        lv_image_set_src(img, &img_exit);
        lv_obj_center(img);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(25));
        lv_obj_set_pos(obj, 0, lv_pct(25));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_delay_cb, LV_EVENT_CLICKED, NULL);
        g_delay_button = obj;

        img = lv_image_create(obj);
        lv_image_set_src(img, &img_delay);
        lv_obj_center(img);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(25));
        lv_obj_set_pos(obj, 0, lv_pct(50));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        // lv_obj_set_style_opa(obj, LV_OPA_0, 0);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_resolution_cb, LV_EVENT_CLICKED, NULL);
        g_resolution_button = obj;

        img = lv_image_create(obj);
        lv_image_set_src(img, &img_resolution);
        lv_obj_center(img);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(25));
        lv_obj_set_pos(obj, 0, lv_pct(75));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_option_cb, LV_EVENT_CLICKED, NULL);
        g_menu_button = obj;

        img = lv_image_create(obj);
        lv_image_set_src(img, &img_option);
        lv_obj_center(img);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(25));
        lv_obj_set_pos(obj, 0, lv_pct(100));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_focus_cb, LV_EVENT_CLICKED, NULL);
        g_focus_button = obj;

        img = lv_image_create(obj);
        lv_image_set_src(img, &img_focus);
        lv_obj_center(img);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(25));
        lv_obj_set_pos(obj, 0, lv_pct(125));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_raw_cb, LV_EVENT_CLICKED, NULL);
        g_raw_button = obj;
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(obj, 0, 0);

        img = lv_image_create(obj);
        lv_image_set_src(img, &img_raw);
        lv_obj_center(img);

        lv_obj_t *label = lv_label_create(obj);
        lv_obj_set_style_text_color(label, lv_color_hex(0xff0000), 0);
        lv_label_set_text(label, "OFF");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(25));
        lv_obj_set_pos(obj, 0, lv_pct(150));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_light_cb, LV_EVENT_CLICKED, NULL);
        g_light_button = obj;

        img = lv_image_create(obj);
        lv_image_set_src(img, &img_light_off);
        lv_obj_center(img);
    }
}

static void right_screen_init(void)
{
    int w = priv.right_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_x(scr, lv_pct(100 - w));
    lv_obj_set_size(scr, lv_pct(w) + 1, lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    {
        static lv_style_t click_style, release_style;
        lv_style_init(&click_style);
        lv_style_set_bg_image_src(&click_style, &img_video);
        lv_style_set_bg_image_opa(&click_style, LV_OPA_100);
        lv_style_set_bg_color(&click_style, lv_color_hex(0x0));

        lv_style_init(&release_style);
        lv_style_set_bg_image_src(&release_style, &img_camera);
        lv_style_set_bg_image_opa(&release_style, LV_OPA_100);
        lv_style_set_border_side(&release_style, LV_BORDER_SIDE_NONE);
        lv_style_set_bg_color(&release_style, lv_color_hex(0x0));

        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 37);
        lv_obj_set_size(obj, img_camera.header.w, img_camera.header.h);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_video_camera_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_style(obj, &click_style, LV_STATE_CHECKED);
        lv_obj_add_style(obj, &release_style, LV_STATE_DEFAULT);

        g_camera_video_button = obj;
    }

    {
        static lv_style_t click_style, release_style, check_style, video_ready_style;
        lv_style_init(&click_style);
        lv_style_set_bg_image_src(&click_style, &img_photo_clicked);
        lv_style_set_bg_image_opa(&click_style, LV_OPA_0);
        lv_style_set_bg_color(&click_style, lv_color_hex(0x0));

        lv_style_init(&release_style);
        lv_style_set_bg_image_src(&release_style, &img_photo_release);
        lv_style_set_bg_image_opa(&release_style, LV_OPA_100);
        lv_style_set_border_side(&release_style, LV_BORDER_SIDE_NONE);
        lv_style_set_bg_color(&release_style, lv_color_hex(0x0));

        lv_style_init(&check_style);
        lv_style_set_bg_image_src(&check_style, &img_video_ready);
        lv_style_set_bg_image_opa(&check_style, LV_OPA_100);
        lv_style_set_border_side(&check_style, LV_BORDER_SIDE_NONE);
        lv_style_set_bg_color(&check_style, lv_color_hex(0x0));

        lv_style_init(&video_ready_style);
        lv_style_set_bg_image_src(&video_ready_style, &img_video_stop);
        lv_style_set_bg_image_opa(&video_ready_style, LV_OPA_100);
        lv_style_set_border_side(&video_ready_style, LV_BORDER_SIDE_NONE);
        lv_style_set_bg_color(&video_ready_style, lv_color_hex(0x0));

        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 136);
        lv_obj_set_size(obj, img_photo_release.header.w, img_photo_release.header.h);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(obj, event_touch_start_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_style(obj, &release_style, LV_STATE_DEFAULT);
        lv_obj_add_style(obj, &click_style, LV_STATE_PRESSED);
        lv_obj_add_style(obj, &check_style, LV_STATE_CHECKED);
        lv_obj_add_style(obj, &video_ready_style, LV_STATE_USER_1);

        g_start_snap_button = obj;
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 246);
        lv_obj_set_size(obj, 56, 56);
        lv_obj_set_style_radius(obj, 5, 0);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(obj, event_touch_small_img_cb, LV_EVENT_CLICKED, NULL);
        g_small_img = obj;

        lv_obj_t *img = lv_image_create(obj);
        lv_obj_center(img);
        static lv_image_dsc_t img_dsc;
        memset(&img_dsc, 0, sizeof(lv_image_dsc_t));
        img_dsc.header.cf = LV_COLOR_FORMAT_ARGB8888;
        img_dsc.header.w = 48;
        img_dsc.header.h = 48;
        img_dsc.data_size = 48 * 48 * 4;
        img_dsc.data = (uint8_t *)malloc(img_dsc.data_size);
        if (!img_dsc.data) {
            printf("malloc failed\n");
            return;
        }
        memset((void *)img_dsc.data, 0x88, img_dsc.data_size);
        lv_image_set_src(img, &img_dsc);
    }
}

static void adjust_screen_init(void)
{
    int right_w = priv.right_screen_w_pct;
    int w = priv.adjust_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_x(scr, lv_pct(100 - right_w - w));
    lv_obj_set_size(scr, lv_pct(w) + 1, lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_pad_hor(scr, 3, 0);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    g_menu_setting = scr;

    lv_obj_t *obj, *label, *label2;
    int obj_w = lv_pct(100);
    {
        obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_CENTER);
        lv_obj_set_width(obj, obj_w);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_shutter_cb, LV_EVENT_CLICKED, NULL);
        g_shutter_button = obj;

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 16, 0);
        lv_obj_center(label);
        lv_label_set_text(label, "S");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);

        label2 = lv_label_create(label);
        lv_obj_align_to(label2, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_label_set_text_fmt(label2, "%d/%ds", 1, 20);
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_CENTER);
        lv_obj_set_width(obj, obj_w);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_iso_cb, LV_EVENT_CLICKED, NULL);
        g_iso_button = obj;

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 8, 0);
        lv_obj_center(label);
        lv_label_set_text(label, "ISO");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);

        label2 = lv_label_create(label);
        lv_obj_align_to(label2, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_label_set_text_fmt(label2, "%d", 480);
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_CENTER);
        lv_obj_set_width(obj, obj_w);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_opa(obj, LV_OPA_0, 0);
        // lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        // lv_obj_add_event_cb(obj, event_touch_ev_cb, LV_EVENT_CLICKED, NULL);
        g_ev_button = obj;

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 10, 0);
        lv_obj_center(label);
        lv_label_set_text(label, "EV");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);

        label2 = lv_label_create(label);
        lv_obj_align_to(label2, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        char str[32];
        snprintf(str, sizeof(str), "%c%.2f", '+', 1.0);
        lv_label_set_text(label2,str);
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_CENTER);
        lv_obj_set_width(obj, obj_w);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_opa(obj, LV_OPA_0, 0);
        // lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CHECKABLE);
        // lv_obj_add_event_cb(obj, event_touch_wb_cb, LV_EVENT_CLICKED, NULL);
        g_wb_button = obj;

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 5, 0);
        lv_obj_center(label);
        lv_label_set_text(label, "WB");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);

        label2 = lv_label_create(label);
        lv_obj_align_to(label2, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_label_set_text(label2, "Auto");
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);
    }
}

static void screen_delay_init(void)
{
    int left_screen_w = priv.left_screen_w_pct;
    int w = priv.delay_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(scr, lv_pct(left_screen_w), 0);
    lv_obj_set_size(scr, lv_pct(w), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    g_delay_setting = scr;

    lv_obj_t *obj, *label;
    int obj_w = lv_pct(100);
    int obj_h = lv_pct(25);

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_delay_cb, LV_EVENT_CLICKED, NULL);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "10s");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_delay_cb, LV_EVENT_CLICKED, NULL);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "7s");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_delay_cb, LV_EVENT_CLICKED, NULL);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "3s");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_delay_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_state(obj, LV_STATE_CHECKED);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "off");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    }
}

static void screen_resolution_init(void)
{
    int left_screen_w = priv.left_screen_w_pct;
    int w = priv.resolution_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(scr, lv_pct(left_screen_w), 0);
    lv_obj_set_size(scr, lv_pct(w), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN_WRAP);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);

    g_resolution_setting = scr;

    lv_obj_t *obj, *label;
    int obj_w = lv_pct(100);
    int obj_h = lv_pct(25);

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_resolution_cb, LV_EVENT_CLICKED, NULL);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "3.7MP(16:9)");    // 2560x1440
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 20);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "2560x1440");    // 2560x1440
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_resolution_cb, LV_EVENT_CLICKED, NULL);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "2MP(16:9)");      // 1920x1080
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 20);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "1920x1080");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_resolution_cb, LV_EVENT_CLICKED, NULL);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "0.9MP(16:9)");    // 1280x720
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 20);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "1280x720");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }

    {
        obj = lv_obj_create(scr);
        lv_obj_set_size(obj, obj_w, obj_h);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_CHECKED);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_flex_grow(obj, 1);
        lv_obj_add_event_cb(obj, event_touch_select_resolution_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_state(obj, LV_STATE_CHECKED);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 0);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "0.3MP(4:3)");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

        label = lv_label_create(obj);
        lv_obj_set_pos(label, 0, 20);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text(label, "640x480");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }
}

static lv_obj_t *ui_plus_and_minus_button(lv_event_cb_t plus_btn_cb, lv_event_cb_t minus_btn_cb)
{
    lv_obj_t *base = lv_obj_create(lv_scr_act());
    int pct_w = 100 - priv.left_screen_w_pct - priv.right_screen_w_pct - priv.adjust_screen_w_pct - priv.shutter_screen_w_pct;
    int pct_x = priv.left_screen_w_pct;
    lv_obj_set_size(base, lv_pct(pct_w), lv_pct(30));
    lv_obj_align(base, LV_ALIGN_BOTTOM_LEFT, lv_pct(pct_x), lv_pct(0));
    lv_obj_set_style_bg_opa(base, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(base, LV_OPA_0, 0);
    lv_obj_set_scrollbar_mode(base, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(base, 0, 0);
    {
        lv_obj_t *btn = lv_btn_create(base);
        lv_obj_set_size(btn, 80, 80);
        lv_obj_align(btn, LV_ALIGN_LEFT_MID, lv_pct(15), 0);
        lv_obj_set_style_radius(btn, 90, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x5fbeef), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x016697), LV_STATE_PRESSED);
        lv_obj_add_event_cb(btn, minus_btn_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *txt = lv_label_create(btn);
        lv_obj_set_style_text_color(txt, lv_color_hex(0xffffff), 0);
        lv_label_set_text(txt, LV_SYMBOL_MINUS);
        lv_obj_align(txt, LV_ALIGN_CENTER, 0, 0);
    }

    {
        lv_obj_t *btn = lv_btn_create(base);
        lv_obj_set_size(btn, 80, 80);
        lv_obj_align(btn, LV_ALIGN_RIGHT_MID, lv_pct(-15), 0);
        lv_obj_set_style_radius(btn, 90, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x5fbeef), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x016697), LV_STATE_PRESSED);
        lv_obj_add_event_cb(btn, plus_btn_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *txt = lv_label_create(btn);
        lv_obj_set_style_text_color(txt, lv_color_hex(0xffffff), 0);
        lv_label_set_text(txt, LV_SYMBOL_PLUS);
        lv_obj_align(txt, LV_ALIGN_CENTER, 0, 0);
    }

    return base;
}

static void screen_shutter_init(void)
{
    int right_screen_w = priv.right_screen_w_pct;
    int adjust_screen_w = priv.adjust_screen_w_pct;
    int w = priv.shutter_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(scr, lv_pct(100 - w - adjust_screen_w - right_screen_w), 0);
    lv_obj_set_size(scr, lv_pct(w), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    // lv_obj_set_style_pad_row(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    static int bar_last_val;
    lv_obj_set_user_data(scr, &bar_last_val);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_shutter_setting = scr;

    {
        lv_obj_t *label, *label2;
        label = lv_label_create(scr);
        lv_label_set_text(label, "Shutter");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);

        label2 = lv_label_create(scr);
        lv_label_set_text_fmt(label2, "1/%ds", 1000);
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);

        ui_camera_config_t camera_config;
        ui_camera_config_read(&camera_config);
        int number_of_table = sizeof(camera_config.exposure_time_table) / sizeof(camera_config.exposure_time_table[0]);

        lv_obj_t * bar = lv_bar_create(scr);
        lv_obj_set_align(bar, LV_ALIGN_CENTER);
        lv_obj_set_size(bar, 20, lv_pct(60));
        lv_bar_set_range(bar, 0, number_of_table - 1);
        lv_obj_add_event_cb(bar, event_shutter_bar_update_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(bar, event_shutter_bar_update_cb, LV_EVENT_RELEASED, NULL);
        lv_bar_set_value(bar, 20, LV_ANIM_OFF);
        lv_obj_set_user_data(bar, label2);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(10));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_shutter_to_auto_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(obj);
        lv_label_set_text(label, "AUTO");
        lv_obj_center(label);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    }


    g_shutter_plus_minus_button = ui_plus_and_minus_button(event_touch_shutter_plus_cb, event_touch_shutter_minus_cb);
    lv_obj_add_flag(g_shutter_plus_minus_button, LV_OBJ_FLAG_HIDDEN);
}


static void screen_iso_init(void)
{
    int right_screen_w = priv.right_screen_w_pct;
    int adjust_screen_w = priv.adjust_screen_w_pct;
    int w = priv.iso_screen_w_pct;
    ui_camera_config_t *cam_cfg = &priv.cam_cfg;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(scr, lv_pct(100 - w - adjust_screen_w - right_screen_w), 0);
    lv_obj_set_size(scr, lv_pct(w), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    // lv_obj_set_style_pad_row(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    static int bar_last_val;
    lv_obj_set_user_data(scr, &bar_last_val);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_iso_setting = scr;
    {
        lv_obj_t *label, *label2;
        label = lv_label_create(scr);
        lv_label_set_text(label, "ISO");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);

        label2 = lv_label_create(scr);
        lv_label_set_text_fmt(label2, "%d", cam_cfg->iso_default);
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);

        ui_camera_config_t camera_config;
        ui_camera_config_read(&camera_config);
        int number_of_table = sizeof(camera_config.iso_table) / sizeof(camera_config.iso_table[0]);

        lv_obj_t * bar = lv_bar_create(scr);
        lv_obj_set_align(bar, LV_ALIGN_CENTER);
        lv_obj_set_size(bar, 20, lv_pct(60));
        lv_bar_set_range(bar, 0, number_of_table - 1);
        lv_obj_add_event_cb(bar, event_iso_bar_update_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(bar, event_iso_bar_update_cb, LV_EVENT_RELEASED, NULL);
        lv_bar_set_value(bar, 100, LV_ANIM_OFF);
        lv_obj_set_user_data(bar, label2);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_size(obj, lv_pct(100), lv_pct(15));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_iso_to_auto_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(obj);
        lv_label_set_text(label, "AUTO");
        lv_obj_center(label);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    }

    g_iso_plus_minus_button = ui_plus_and_minus_button(event_touch_iso_plus_cb, event_touch_iso_minus_cb);
    lv_obj_add_flag(g_iso_plus_minus_button, LV_OBJ_FLAG_HIDDEN);
}


static void screen_ev_init(void)
{
    int right_screen_w = priv.right_screen_w_pct;
    int adjust_screen_w = priv.adjust_screen_w_pct;
    int w = priv.ev_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(scr, lv_pct(100 - w - adjust_screen_w - right_screen_w), 0);
    lv_obj_set_size(scr, lv_pct(w), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    static int bar_last_val;
    lv_obj_set_user_data(scr, &bar_last_val);

    g_ev_setting = scr;

    {
        lv_obj_t *label, *label2;
        label = lv_label_create(scr);
        lv_obj_set_pos(label, 15, 0);
        lv_label_set_text(label, "EV");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);

        label2 = lv_label_create(scr);
        lv_obj_align_to(label2, label, LV_ALIGN_OUT_BOTTOM_MID, 4, 0);
        lv_label_set_text_fmt(label2, "+0.0");
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);

        lv_obj_t * bar = lv_bar_create(scr);
        lv_obj_set_align(bar, LV_ALIGN_CENTER);
        lv_obj_set_size(bar, 20, lv_pct(60));
        lv_obj_center(bar);
        lv_bar_set_range(bar, -400, 400);
        lv_obj_add_event_cb(bar, event_ev_bar_update_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(bar, event_ev_bar_update_cb, LV_EVENT_RELEASED, NULL);
        lv_bar_set_value(bar, 100, LV_ANIM_OFF);
        lv_obj_set_user_data(bar, label2);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_pos(obj, 0, lv_pct(85));
        lv_obj_set_size(obj, lv_pct(100), lv_pct(15));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_ev_to_auto_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(obj);
        lv_label_set_text(label, "AUTO");
        lv_obj_center(label);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    }
}

static void screen_wb_init(void)
{
    int right_screen_w = priv.right_screen_w_pct;
    int adjust_screen_w = priv.adjust_screen_w_pct;
    int w = priv.wb_screen_w_pct;

    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(scr, lv_pct(100 - w - adjust_screen_w - right_screen_w), 0);
    lv_obj_set_size(scr, lv_pct(w), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2e2e2e), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    static int bar_last_val;
    lv_obj_set_user_data(scr, &bar_last_val);

    g_wb_setting = scr;

    {
        lv_obj_t *label, *label2;
        label = lv_label_create(scr);
        lv_obj_set_pos(label, 15, 0);
        lv_label_set_text(label, "WB");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);

        label2 = lv_label_create(scr);
        lv_obj_align_to(label2, label, LV_ALIGN_OUT_BOTTOM_MID, 4, 0);
        lv_label_set_text_fmt(label2, "%dK", 1000);
        lv_obj_set_style_text_color(label2, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label2, &lv_font_montserrat_12, 0);

        lv_obj_t * bar = lv_bar_create(scr);
        lv_obj_set_align(bar, LV_ALIGN_CENTER);
        lv_obj_set_size(bar, 20, lv_pct(60));
        lv_obj_center(bar);
        lv_bar_set_range(bar, 1000, 10000);
        lv_obj_add_event_cb(bar, event_wb_bar_update_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(bar, event_wb_bar_update_cb, LV_EVENT_RELEASED, NULL);
        lv_bar_set_value(bar, 100, LV_ANIM_OFF);
        lv_obj_set_user_data(bar, label2);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_pos(obj, 0, lv_pct(85));
        lv_obj_set_size(obj, lv_pct(100), lv_pct(15));
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x7e7e7e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(obj, event_touch_wb_to_auto_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(obj);
        lv_label_set_text(label, "AUTO");
        lv_obj_center(label);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    }
}

void ui_show_center_image(uint8_t *data, int data_size, int width, int height)
{
    if (data_size != width * height * 3) {
        printf("data_size != width * height * 3\n");
        return;
    }

    static lv_obj_t *img = NULL;
    static lv_img_dsc_t img_dsc;
    if (img != NULL) {
        lv_obj_delete(img);
        img = NULL;
        if (img_dsc.data) {
            free((uint8_t *)img_dsc.data);
            img_dsc.data = NULL;
        }
    }

    img = lv_image_create(lv_layer_top());
    memset(&img_dsc, 0, sizeof(lv_img_dsc_t));
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB888;
    img_dsc.header.w = width;
    img_dsc.header.h = height;
    img_dsc.data_size = img_dsc.header.w * img_dsc.header.h * 3;
    img_dsc.data = malloc(img_dsc.data_size);
    if (!img_dsc.data) {
        printf("malloc failed\n");
        return;
    }
    memcpy((uint8_t *)img_dsc.data, data, img_dsc.data_size);
    lv_img_set_src(img, &img_dsc);
    lv_obj_center(img);
}


static void screen_big_image(void)
{
    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scr, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_hor(scr, 1, 0);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(scr, event_touch_big_img_cb, LV_EVENT_CLICKED, NULL);

    g_big_img = scr;

    lv_obj_t *img = lv_image_create(scr);
    static lv_img_dsc_t img_dsc;
    memset(&img_dsc, 0, sizeof(lv_img_dsc_t));
    img_dsc.header.cf = LV_COLOR_FORMAT_ARGB8888;
    img_dsc.header.w = lv_disp_get_hor_res(NULL);
    img_dsc.header.h = lv_disp_get_ver_res(NULL);
    img_dsc.data_size = img_dsc.header.w * img_dsc.header.h * 4;
    img_dsc.data = malloc(img_dsc.data_size);
    if (!img_dsc.data) {
        printf("malloc failed\n");
        return;
    }
    lv_img_set_src(img, &img_dsc);
    lv_obj_center(img);
}

void ui_set_select_option(int idx)
{
    if (!g_resolution_setting) return;
    lv_obj_t *parent = g_resolution_setting;
    for (size_t i = 0; i < lv_obj_get_child_count(parent); i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        if (i == idx) {
            lv_obj_add_state(child, LV_STATE_CHECKED);
        } else {
            lv_obj_remove_state(child, LV_STATE_CHECKED);
        }
    }
}

static void screen_video_running(void)
{
    lv_obj_t *scr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scr, lv_pct(15), lv_pct(5));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_hor(scr, 1, 0);
    lv_obj_align(scr, LV_ALIGN_TOP_MID, 0, 3);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);

    g_video_running_screen = scr;

    lv_obj_t *img = lv_image_create(scr);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 1, 0);
    lv_image_set_src(img, &img_small_recording);

    lv_obj_t *label = lv_label_create(scr);
    lv_obj_align(label, LV_ALIGN_RIGHT_MID, -1, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label, "00:00:00");
}

void ui_refresh(void)
{
    lv_obj_invalidate(lv_scr_act());
    lv_obj_invalidate(lv_layer_top());
    lv_obj_invalidate(lv_layer_bottom());
    lv_refr_now(NULL);
}

void ui_all_screen_init(void)
{
    priv_init();

    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
    lv_display_set_color_format(NULL, LV_COLOR_FORMAT_ARGB8888);
    lv_screen_load(lv_layer_top());
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);

    left_screen_init();
    right_screen_init();
    adjust_screen_init();
    screen_delay_init();
    screen_resolution_init();
    screen_shutter_init();
    screen_iso_init();
    screen_ev_init();
    screen_wb_init();
    screen_big_image();
    screen_video_running();

    // ui_set_shutter_value((double)1/1000);
    // ui_set_iso_value(400);
    // ui_set_ev_value(0);
    // ui_set_wb_value(2000);

    // ui_set_shutter_value((double)1/1000);
    // ui_set_iso_value(400);
    // ui_set_ev_value(0);
    // ui_set_wb_value(2000);

    // {
    //     int width = 48, height = 48;
    //     uint8_t *img_data = (uint8_t *)malloc(width * height * 4);
    //     for (int h = 0; h < height; h ++) {
    //         for (int w = 0; w < width; w ++) {
    //             img_data[(h * width + w) * 4 + 0] = 0x00;
    //             img_data[(h * width + w) * 4 + 1] = 0x00;
    //             img_data[(h * width + w) * 4 + 2] = 0xFF;
    //             img_data[(h * width + w) * 4 + 3] = 0xff;
    //         }
    //     }
    //     ui_update_small_img(img_data, width * height * 4);
    //     free(img_data);
    // }
    // {
    //     int width = 552, height = 368;
    //     uint8_t *img_data = (uint8_t *)malloc(width * height * 4);
    //     for (int h = 0; h < height; h ++) {
    //         for (int w = 0; w < width; w ++) {
    //             img_data[(h * width + w) * 4 + 0] = 0x00;
    //             img_data[(h * width + w) * 4 + 1] = 0x00;
    //             img_data[(h * width + w) * 4 + 2] = 0xFF;
    //             img_data[(h * width + w) * 4 + 3] = 0xff;
    //         }
    //     }
    //     ui_update_big_img(img_data, width * height * 4);
    //     free(img_data);
    // }

    ui_set_record_time(2000 * 1000);
}