
#include "maix_lvgl.hpp"
#include "monitor.h"
#include "mouse.h"
#include "keyboard.h"
#include "lvgl.h"
#include "maix_basic.hpp"

// extern "C" {
// #include "cursor_48.c"
// #include "cursor_96.c"
// }
LV_IMAGE_DECLARE(cursor_96);
LV_IMAGE_DECLARE(cursor_48);

namespace maix
{
    display::Display *maix_display = nullptr;
    image::Image *maix_image = nullptr;
    touchscreen::TouchScreen *maix_touchscreen = nullptr;
    static thread::Thread *tick_th = nullptr;
    static volatile bool tick_th_exit = false;
    static volatile bool tick_th_exit_done = false;
    static lv_color_t *buf1_1 = nullptr;
    static lv_color_t *buf1_2 = nullptr;



    static inline uint32_t get_time_ms()
    {
        return time::ticks_ms();
    }

    void lvgl_init(display::Display *display, touchscreen::TouchScreen *touchscreen)
    {
        assert(LV_COLOR_DEPTH == 32);

        if (!display)
        {
            throw std::runtime_error("lvgl_init display is null");
        }
        // if (!touchscreen)
        // {
        //     throw std::runtime_error("lvgl_init touchscreen is null");
        // }

        int buf_size_byte = display->width() * 100 * 4;
        if (buf1_1)
        {
            throw std::runtime_error("lvgl_init already init");
        }
        buf1_1 = (lv_color_t *)malloc(buf_size_byte);
        if (!buf1_1)
        {
            throw std::runtime_error("lvgl_init malloc failed");
        }
        buf1_2 = (lv_color_t *)malloc(buf_size_byte);
        if (!buf1_2)
        {
            free(buf1_1);
            buf1_1 = nullptr;
            throw std::runtime_error("lvgl_init malloc failed");
        }
        maix_image = new maix::image::Image(display->width(), display->height(), image::FMT_BGRA8888);
        maix_display = display;
        maix_touchscreen = touchscreen;
#ifdef PLATFORM_LINUX
        assert(maix::image::fmt_size[maix_display->format()] == LV_COLOR_DEPTH / 8);
#endif
        // LVGL global init
        lv_init();

        monitor_init(display->width(), display->height());

        /* Tick init.
         * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
         * how much time were elapsed Create an SDL thread to do this*/
        // tick_th = new thread::Thread(tick_thread, nullptr);
        // tick_th->detach();
        lv_tick_set_cb(get_time_ms);

        /*Create a display buffer*/

        /*Create a display*/
        lv_display_t *disp_drv = lv_display_create(display->width(), display->height());
        lv_display_set_buffers(disp_drv, buf1_1, buf1_2, buf_size_byte, LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_display_set_flush_cb(disp_drv, monitor_flush);
        // disp_drv->antialiasing = 1;

        lv_theme_t *th = lv_theme_default_init(disp_drv, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
        lv_disp_set_theme(disp_drv, th);

        lv_group_t *g = lv_group_create();
        lv_group_set_default(g);

        /* Add the mouse as input device
         * Use the 'mouse' driver which reads the PC's mouse*/
#if CONFIG_LVGL_USE_MOUSE
        lv_indev_t * indev = lv_indev_create();
        MouseInputDevice type = mouse_init(indev);
        lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev, mouse_read);
        // lv_indev_set_mode(indev, LV_INDEV_MODE_EVENT);
        if (type == MouseInputDevice::USB_MOUSE) {
            static lv_obj_t *cursor_rect = lv_img_create(lv_scr_act());
            int w;
            monitor_rect(&w, nullptr);
            if (w > 640) {
                lv_img_set_src(cursor_rect, &cursor_96);
                maix::log::info("use cursor size 48x48");
            } else {
                lv_img_set_src(cursor_rect, &cursor_48);
                maix::log::info("use cursor size 96x96");
            }
            // w = static_cast<int>((20.0/640)*w);
            // lv_obj_set_size(cursor_rect, w, w);
            lv_obj_set_style_bg_color(cursor_rect, lv_color_hex(0xFF0000), 0);
            lv_indev_set_cursor(indev, cursor_rect);
        }
#endif

#if USE_KEYBOARD
        keyboard_init();
        static lv_indev_drv_t indev_drv_2;
        lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
        indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
        indev_drv_2.read_cb = keyboard_read;
        lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
        lv_indev_set_group(kb_indev, g);
#endif
    }

    void lvgl_destroy()
    {
        if (maix_image)
            delete maix_image;
        if (tick_th)
        {
            tick_th_exit_done = true;
            tick_th_exit = true;
            while (!tick_th_exit_done)
            {
                thread::sleep_ms(1);
            }
            delete tick_th;
            tick_th = nullptr;
        }
        maix_image = nullptr;
        maix_display = nullptr;
        maix_touchscreen = nullptr;
        lv_deinit();
        if (buf1_1)
        {
            free(buf1_1);
            buf1_1 = nullptr;
        }
        if (buf1_2)
        {
            free(buf1_2);
            buf1_2 = nullptr;
        }
    }

}
