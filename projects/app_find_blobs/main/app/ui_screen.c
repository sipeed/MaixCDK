#include "lvgl.h"
#include "stdio.h"
#include "ui_screen.h"
#include "ui_event_handler.h"

LV_IMG_DECLARE(img_exit);
LV_IMG_DECLARE(img_eye_open);
LV_IMG_DECLARE(img_eye_close);
LV_IMG_DECLARE(img_option);

lv_obj_t *ui_lab_options_screen;
lv_obj_t *ui_bar_screen;
lv_obj_t *ui_bar;
lv_obj_t *ui_red_btn;
lv_obj_t *ui_green_btn;
lv_obj_t *ui_blue_btn;
lv_obj_t *ui_user_btn;
lv_obj_t *ui_lmax_btn;
lv_obj_t *ui_lmin_btn;
lv_obj_t *ui_amax_btn;
lv_obj_t *ui_amin_btn;
lv_obj_t *ui_bmax_btn;
lv_obj_t *ui_bmin_btn;
lv_obj_t *ui_color_box_screen;
lv_obj_t *ui_color_box[3];
lv_obj_t *ui_draw_pointer[3];

static void _ui_upper_screen_init(void)
{
    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    lv_obj_set_size(obj, lv_pct(100) + 1, lv_pct(12));
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_size(sub_obj, lv_pct(12), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(sub_obj, event_touch_exit_cb, LV_EVENT_PRESSED, NULL);

        lv_obj_t *img = lv_image_create(sub_obj);
        lv_image_set_src(img, &img_exit);
        lv_obj_center(img);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(12), 0);
        lv_obj_set_size(sub_obj, lv_pct(76) + 1, lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text(label, "Please put the color block in the box.");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(88), 0);
        lv_obj_set_size(sub_obj, lv_pct(12), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);

        lv_obj_t *imgbtn = lv_imagebutton_create(sub_obj);
        lv_obj_add_flag(imgbtn, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_imagebutton_set_src(imgbtn, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &img_eye_close, NULL);
        lv_imagebutton_set_src(imgbtn, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL, &img_eye_open, NULL);
        lv_obj_add_event_cb(imgbtn, event_touch_eye_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_center(imgbtn);
    }
}

static void _ui_lower_screen_init(void)
{
    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    lv_obj_set_align(obj, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_size(obj, lv_pct(100) + 1, lv_pct(15));
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_size(sub_obj, lv_pct(12), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_add_event_cb(sub_obj, event_touch_lab_options_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t *img = lv_image_create(sub_obj);
        lv_image_set_src(img, &img_option);
        lv_obj_center(img);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(12), 0);
        lv_obj_set_size(sub_obj, lv_pct(22) + 1, lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_add_event_cb(sub_obj, event_touch_red_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_state(sub_obj, LV_STATE_CHECKED);
        ui_red_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text(label, "red");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(34), 0);
        lv_obj_set_size(sub_obj, lv_pct(22)+1, lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_add_event_cb(sub_obj, event_touch_green_cb, LV_EVENT_CLICKED, NULL);

        ui_green_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text(label, "green");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(56), 0);
        lv_obj_set_size(sub_obj, lv_pct(22) + 1, lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_add_event_cb(sub_obj, event_touch_blue_cb, LV_EVENT_CLICKED, NULL);

        ui_blue_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text(label, "blue");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(78), 0);
        lv_obj_set_size(sub_obj, lv_pct(22), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_add_event_cb(sub_obj, event_touch_user_cb, LV_EVENT_CLICKED, NULL);

        static ui_lab_range_t lab = {0, 100, -128, 127, -128, 127};
        lv_obj_set_user_data(sub_obj, &lab);

        ui_user_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text(label, "user");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }
}

static void _ui_lab_option_screen_init(void)
{
    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    lv_obj_set_pos(obj, lv_pct(0), lv_pct(70) + 1);
    lv_obj_set_size(obj, lv_pct(100), lv_pct(15));
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), 0);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

    ui_lab_options_screen = obj;

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_size(sub_obj, lv_pct(16), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_set_flex_grow(sub_obj, 1);
        lv_obj_add_event_cb(sub_obj, event_touch_lmin_cb, LV_EVENT_CLICKED, NULL);
        static int value = 0;
        lv_obj_set_user_data(sub_obj, &value);
        ui_lmin_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text_fmt(label, "L Min\n(%d)", 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(16), 0);
        lv_obj_set_size(sub_obj, lv_pct(16), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_set_flex_grow(sub_obj, 1);
        lv_obj_add_event_cb(sub_obj, event_touch_lmax_cb, LV_EVENT_CLICKED, NULL);
        static int value = 80;
        lv_obj_set_user_data(sub_obj, &value);
        ui_lmax_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text_fmt(label, "L Max\n(%d)", 80);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(32), 0);
        lv_obj_set_size(sub_obj, lv_pct(16), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_set_flex_grow(sub_obj, 1);
        lv_obj_add_event_cb(sub_obj, event_touch_amin_cb, LV_EVENT_CLICKED, NULL);
        static int value = 0;
        lv_obj_set_user_data(sub_obj, &value);
        ui_amin_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text_fmt(label, "A Min\n(%d)", 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(48), 0);
        lv_obj_set_size(sub_obj, lv_pct(16) + 1, lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_set_flex_grow(sub_obj, 1);
        lv_obj_add_event_cb(sub_obj, event_touch_amax_cb, LV_EVENT_CLICKED, NULL);
        static int value = 80;
        lv_obj_set_user_data(sub_obj, &value);
        ui_amax_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text_fmt(label, "A Max\n(%d)", 80);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(64), 0);
        lv_obj_set_size(sub_obj, lv_pct(16), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_set_flex_grow(sub_obj, 1);
        lv_obj_add_event_cb(sub_obj, event_touch_bmin_cb, LV_EVENT_CLICKED, NULL);
        static int value = 10;
        lv_obj_set_user_data(sub_obj, &value);
        ui_bmin_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text_fmt(label, "B Min\n(%d)", 10);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *sub_obj = lv_obj_create(obj);
        lv_obj_set_pos(sub_obj, lv_pct(80), 0);
        lv_obj_set_size(sub_obj, lv_pct(16), lv_pct(100));
        lv_obj_set_style_radius(sub_obj, 0, 0);
        lv_obj_remove_flag(sub_obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_bg_color(sub_obj, lv_color_hex(0x2e2e2e), LV_STATE_CHECKED);
        lv_obj_set_style_border_side(sub_obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_outline_pad(sub_obj, 0, 0);
        lv_obj_add_flag(sub_obj, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_set_flex_grow(sub_obj, 1);
        lv_obj_add_event_cb(sub_obj, event_touch_bmax_cb, LV_EVENT_CLICKED, NULL);
        static int value = 80;
        lv_obj_set_user_data(sub_obj, &value);
        ui_bmax_btn = sub_obj;

        lv_obj_t *label = lv_label_create(sub_obj);
        lv_label_set_text_fmt(label, "B Max\n(%d)", 80);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }
}

static void _ui_bar_screen_init(void)
{
    lv_obj_t *scr = lv_obj_create(lv_screen_active());
    lv_obj_set_align(scr, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_y(scr, lv_pct(12));
    lv_obj_set_size(scr, lv_pct(15), lv_pct(58));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_0, 0);

    ui_bar_screen = scr;

    static lv_style_t style_bg;
    static lv_style_t style_indic;
    lv_style_init(&style_bg);
    lv_style_set_border_color(&style_bg, lv_color_make(91, 209, 215));
    lv_style_set_border_width(&style_bg, 2);
    lv_style_set_pad_all(&style_bg, 6); /*To make the indicator smaller*/
    lv_style_set_radius(&style_bg, 6);
    lv_style_set_bg_color(&style_bg, lv_color_hex(0x0));

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_color_make(91, 209, 215));
    lv_style_set_radius(&style_indic, 3);

    lv_obj_t * bar = lv_bar_create(scr);
    lv_obj_set_align(bar, LV_ALIGN_CENTER);
    lv_obj_set_size(bar, lv_pct(80), lv_pct(100));
    lv_obj_center(bar);
    lv_bar_set_range(bar, -128, 127);
    lv_obj_add_event_cb(bar, event_lab_bar_update_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(bar, event_lab_bar_update_cb, LV_EVENT_RELEASED, NULL);
    lv_bar_set_value(bar, 100, LV_ANIM_OFF);

    lv_obj_add_style(bar, &style_bg, 0);
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

    ui_bar = bar;
    {
        lv_obj_t *label = lv_label_create(bar);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_obj_set_y(label, lv_pct(10));
        lv_label_set_text_fmt(label, "None");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_obj_center(label);
    }

    {
        lv_obj_t *label = lv_label_create(bar);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_obj_set_y(label, lv_pct(10));
        lv_label_set_text_fmt(label, "%d", 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    }
}

static void _ui_color_box_screen_init(void)
{
    lv_obj_t *scr = lv_obj_create(lv_screen_active());
    lv_obj_set_align(scr, LV_ALIGN_TOP_LEFT);
    lv_obj_set_y(scr, lv_pct(12));
    lv_obj_set_size(scr, lv_pct(20), lv_pct(58));
    lv_obj_set_style_border_side(scr, LV_BORDER_SIDE_NONE, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0), 0);
    lv_obj_set_style_pad_hor(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);

    ui_color_box_screen = scr;

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_TOP_LEFT);
        lv_obj_set_y(obj, lv_pct(0));
        lv_obj_set_size(obj, lv_pct(100), lv_pct(10));
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), 0);
        lv_obj_set_style_pad_hor(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *label = lv_label_create(obj);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text_fmt(label, "L   A   B");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_TOP_LEFT);
        lv_obj_set_y(obj, lv_pct(0));
        lv_obj_set_size(obj, lv_pct(100), lv_pct(33));
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x1e1e1e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_pad_hor(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(obj, event_touch_color_btn_cb, LV_EVENT_CLICKED, NULL);
        static ui_lab_t lab;
        lv_obj_set_user_data(obj, &lab);

        ui_color_box[0] = obj;

        lv_obj_t *label = lv_label_create(obj);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text_fmt(label, "%.2d  %.2d  %.2d", 10, 20 ,30);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_TOP_LEFT);
        lv_obj_set_y(obj, lv_pct(33));
        lv_obj_set_size(obj, lv_pct(100), lv_pct(33));
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x4e4e4e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_pad_hor(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(obj, event_touch_color_btn_cb, LV_EVENT_CLICKED, NULL);
        static ui_lab_t lab;
        lv_obj_set_user_data(obj, &lab);

        ui_color_box[1] = obj;

        lv_obj_t *label = lv_label_create(obj);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text_fmt(label, "%.2d  %.2d  %.2d", 10, 20 ,30);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    }

    {
        lv_obj_t *obj = lv_obj_create(scr);
        lv_obj_set_align(obj, LV_ALIGN_TOP_LEFT);
        lv_obj_set_y(obj, lv_pct(66));
        lv_obj_set_size(obj, lv_pct(100), lv_pct(33));
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_radius(obj, 0, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x6e6e6e), 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0x2e2e2e), LV_STATE_PRESSED);
        lv_obj_set_style_pad_hor(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(obj, event_touch_color_btn_cb, LV_EVENT_CLICKED, NULL);
        static ui_lab_t lab;
        lv_obj_set_user_data(obj, &lab);

        ui_color_box[2] = obj;

        lv_obj_t *label = lv_label_create(obj);
        lv_obj_set_align(label, LV_ALIGN_CENTER);
        lv_label_set_text_fmt(label, "%.2d  %.2d  %.2d", 10, 20 ,30);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    }
}

static void _ui_draw_pointer_init(void)
{
    for (int i = 0; i < 3; i ++) {
        lv_obj_t *obj = lv_obj_create(lv_screen_active());
        lv_obj_set_align(obj, LV_ALIGN_TOP_LEFT);
        lv_obj_set_size(obj, 8, 8);
        lv_obj_set_style_radius(obj, 0, 90);
        lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_NONE, 0);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xff0000), 0);
        lv_obj_set_style_pad_hor(obj, 0, 0);
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

        ui_draw_pointer[i] = obj;
    }

}

void ui_all_screen_init(void)
{
#if 1
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
    lv_display_set_color_format(NULL, LV_COLOR_FORMAT_ARGB8888);
    lv_screen_load(lv_layer_top());
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
#else
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
#endif
    lv_obj_add_flag(lv_screen_active(), LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lv_screen_active(), event_active_screen_cb, LV_EVENT_CLICKED, NULL);

    _ui_upper_screen_init();
    _ui_lower_screen_init();
    _ui_lab_option_screen_init();
    _ui_bar_screen_init();
    _ui_color_box_screen_init();
    _ui_draw_pointer_init();
}