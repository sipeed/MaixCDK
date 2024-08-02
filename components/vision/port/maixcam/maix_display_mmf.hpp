/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#pragma once

#include "maix_display_base.hpp"
#include "maix_thread.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include <unistd.h>
#include "sophgo_middleware.hpp"
#include "maix_pwm.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
using namespace maix::peripheral;

namespace maix::display
{
    __attribute__((unused)) static int _get_vo_max_size(int *width, int *height, int rotate)
    {
        int w = 0, h = 0;

        char line[1024];
        char panel_value[256];

        char *panel_env = getenv("MMF_PANEL_NAME");
        if (panel_env) {
            log::info("Found panel env MMF_PANEL_NAME=%s\r\n", panel_env);
            strncpy(panel_value, panel_env, sizeof(panel_value));
        } else {
            FILE *file = fopen("/boot/uEnv.txt", "r");
            if (file == NULL) {
                perror("Error opening file");
                return 1;
            }

            while (fgets(line, sizeof(line), file)) {
                if (strncmp(line, "panel=", 6) == 0) {
                    strcpy(panel_value, line + 6);
                    panel_value[strcspn(panel_value, "\n")] = '\0';
                    break;
                }
            }

            fclose(file);
        }

        if (strlen(panel_value) > 0) {
            printf("panel= %s\n", panel_value);
        } else {
            printf("panel value not found\n");
        }

        if (!strcmp(panel_value, "zct2133v1")) {
            w = 800;
            h = 1280;
        } else if (!strcmp(panel_value, "mtd700920b")) {
            w = 800;
            h = 1280;
        } else if (!strcmp(panel_value, "st7701_dxq5d0019_V0")) {
            w = 480;
            h = 854;
        } else if (!strcmp(panel_value, "st7701_dxq5d0019b480854")) {
            w = 480;
            h = 854;
        } else if (!strcmp(panel_value, "st7701_d300fpc9307a")) {
            w = 480;
            h = 854;
        } else if (!strcmp(panel_value, "st7701_hd228001c31")) {
            w = 368;
            h = 552;
        } else if (!strcmp(panel_value, "MaixCam_Pro")) {
            w = 480;
            h = 640;
        } else {
            w = 368;
            h = 552;
        }

        if (rotate) {
            *width = h;
            *height = w;
        } else {
            *width = w;
            *height = h;
        }
        return 0;
    }

    class DisplayCviMmf final : public DisplayBase
    {
    public:
        DisplayCviMmf(const char *device, int width, int height, image::Format format)
        {
            err::check_bool_raise(!_get_vo_max_size(&_max_width, &_max_height, 1), "get vo max size failed");
            width = width <= 0 ? _max_width : width;
            height = height <= 0 ? _max_height : height;
            this->_width = width > _max_width ? _max_width : width;
            this->_height = height > _max_height ? _max_height : height;
            this->_format = format;
            this->_opened = false;
            this->_format = format;
            this->_layer = 0;       // layer 0 means vedio layer
            err::check_bool_raise(_format == image::FMT_RGB888
                                || _format == image::FMT_YVU420SP
                                || _format == image::FMT_BGRA8888, "Format not support");

            if (0 != mmf_init()) {
                err::check_raise(err::ERR_RUNTIME, "mmf init failed");
            }
            int pwm_id = 10;
            _bl_pwm = new pwm::PWM(pwm_id, 100000, 20);
        }

        DisplayCviMmf(int layer, int width, int height, image::Format format)
        {
            err::check_bool_raise(!_get_vo_max_size(&_max_width, &_max_height, 1), "get vo max size failed");
            width = width <= 0 ? _max_width : width;
            height = height <= 0 ? _max_height : height;
            this->_width = width > _max_width ? _max_width : width;
            this->_height = height > _max_height ? _max_height : height;
            this->_format = format;
            this->_opened = false;
            this->_format = format;
            this->_layer = layer;       // layer 0 means vedio layer
                                        // layer 1 means osd layer
            err::check_bool_raise(_format == image::FMT_BGRA8888, "Format not support");

            if (0 != mmf_init()) {
                err::check_raise(err::ERR_RUNTIME, "mmf init failed");
            }
            int pwm_id = 10;
            _bl_pwm = new pwm::PWM(pwm_id, 100000, 20);
        }

        ~DisplayCviMmf()
        {
            mmf_del_vo_channel(this->_layer, this->_ch);
            mmf_deinit();
            if(_bl_pwm)
            {
                delete _bl_pwm;
            }
        }

        int width()
        {
            return this->_width;
        }

        int height()
        {
            return this->_height;
        }

        std::vector<int> size()
        {
            return {this->_width, this->_height};
        }

        image::Format format()
        {
            return this->_format;
        }

        err::Err open(int width, int height, image::Format format)
        {
            width = width > _max_width ? _max_width : width;
            height = height > _max_height ? _max_height : height;
            if(this->_opened)
            {
                return err::ERR_NONE;
            }

            int ch = mmf_get_vo_unused_channel(this->_layer);
            if (ch < 0) {
                log::error("mmf_get_vo_unused_channel failed\n");
                return err::ERR_RUNTIME;
            }

            if (0 != mmf_add_vo_channel_with_fit(this->_layer, ch, width, height, mmf_invert_format_to_mmf(format), 2)) {
                log::error("mmf_add_vo_channel_with_fit failed\n");
                return err::ERR_RUNTIME;
            }

            this->_ch = ch;
            this->_opened = true;
            return err::ERR_NONE;
        }

        err::Err close()
        {
            if (!this->_opened)
                return err::ERR_NONE;

            if (mmf_vo_channel_is_open(this->_layer, this->_ch) == true) {
                if (0 != mmf_del_vo_channel(this->_layer, this->_ch)) {
                    log::error("mmf del vo channel failed\n");
                    return err::ERR_RUNTIME;
                }
            }
            this->_opened = false;
            return err::ERR_NONE;
        }

        display::DisplayCviMmf *add_channel(int width, int height, image::Format format)
        {
            int new_width = 0;
            int new_height = 0;
            image::Format new_format;
            if (width == -1) {
                new_width = this->_width;
            } else {
                new_width = width > this->_width ? this->_width : width;
            }
            if (height == -1) {
                new_height = this->_height;
            } else {
                new_height = height > this->_height ? this->_height : height;
            }

            new_format = image::Format::FMT_BGRA8888;
            _format = new_format;
            DisplayCviMmf *disp = new DisplayCviMmf(1, new_width, new_height, new_format);
            return disp;
        }

        bool is_opened()
        {
            return this->_opened;
        }

        err::Err show(image::Image &img, image::Fit fit)
        {
            err::check_bool_raise((img.width() % 2 == 0 && img.height() % 2 == 0), "Image width and height must be a multiple of 2.");
            int format = img.format();

            int mmf_fit = 0;
            switch (fit) {
                case image::Fit::FIT_FILL: mmf_fit = 0; break;
                case image::Fit::FIT_CONTAIN: mmf_fit = 1; break;
                case image::Fit::FIT_COVER: mmf_fit = 2; break;
                default: mmf_fit = 0; break;
            }

            if (this->_layer == 0) {
                switch (format)
                {
                case image::FMT_GRAYSCALE:
                    if (0 != mmf_vo_frame_push_with_fit(this->_layer, this->_ch, img.data(), img.data_size(), img.width(), img.height(), mmf_invert_format_to_mmf(img.format()), mmf_fit)) {
                        log::error("mmf_vo_frame_push failed\n");
                        return err::ERR_RUNTIME;
                    }
                    break;
                case image::FMT_RGB888:
                    if (0 != mmf_vo_frame_push_with_fit(this->_layer, this->_ch, img.data(), img.data_size(), img.width(), img.height(), mmf_invert_format_to_mmf(img.format()), mmf_fit)) {
                        log::error("mmf_vo_frame_push failed\n");
                        return err::ERR_RUNTIME;
                    }
                    break;
                case image::FMT_YVU420SP:
                    if (0 != mmf_vo_frame_push_with_fit(this->_layer, this->_ch, img.data(), img.data_size(), img.width(), img.height(), mmf_invert_format_to_mmf(img.format()), mmf_fit)) {
                        log::error("mmf_vo_frame_push failed\n");
                        return err::ERR_RUNTIME;
                    }
                    break;
                case image::FMT_BGRA8888:
                {
                    int width = img.width(), height = img.height();
                    image::Image *rgb = new image::Image(width, height, image::FMT_RGB888);
                    uint8_t *src = (uint8_t *)img.data(), *dst = (uint8_t *)rgb->data();
                    for (int i = 0; i < height; i ++) {
                        for (int j = 0; j < width; j ++) {
                            dst[(i * width + j) * 3 + 0] = src[(i * width + j) * 4 + 2];
                            dst[(i * width + j) * 3 + 1] = src[(i * width + j) * 4 + 1];
                            dst[(i * width + j) * 3 + 2] = src[(i * width + j) * 4 + 0];
                        }
                    }
                    if (0 != mmf_vo_frame_push_with_fit(this->_layer, this->_ch, rgb->data(), rgb->data_size(), rgb->width(), rgb->height(), mmf_invert_format_to_mmf(rgb->format()), mmf_fit)) {
                        log::error("mmf_vo_frame_push failed\n");
                        return err::ERR_RUNTIME;
                    }
                    delete rgb;
                    break;
                }
                case image::FMT_RGBA8888:
                {
                    int width = img.width(), height = img.height();
                    image::Image *rgb = new image::Image(width, height, image::FMT_RGB888);
                    uint8_t *src = (uint8_t *)img.data(), *dst = (uint8_t *)rgb->data();
                    for (int i = 0; i < height; i ++) {
                        for (int j = 0; j < width; j ++) {
                            dst[(i * width + j) * 3 + 0] = src[(i * width + j) * 4 + 0];
                            dst[(i * width + j) * 3 + 1] = src[(i * width + j) * 4 + 1];
                            dst[(i * width + j) * 3 + 2] = src[(i * width + j) * 4 + 2];
                        }
                    }
                    if (0 != mmf_vo_frame_push_with_fit(this->_layer, this->_ch, rgb->data(), rgb->data_size(), rgb->width(), rgb->height(), mmf_invert_format_to_mmf(rgb->format()), mmf_fit)) {
                        log::error("mmf_vo_frame_push failed\n");
                        return err::ERR_RUNTIME;
                    }
                    delete rgb;
                    break;
                }
                default:
                    log::error("display layer 0 not support format: %d\n", format);
                    return err::ERR_ARGS;
                }
            } else if (this->_layer == 1) {
                image::Image *new_img = &img;
                if (format != image::FMT_BGRA8888) {
                    new_img = img.to_format(image::FMT_BGRA8888);
                    err::check_null_raise(new_img, "This image format is not supported, try image::Format::FMT_BGRA8888");
                }

                if (img.width() != _width || img.height() != _height) {
                    log::error("image size not match, you must pass in an image of size %dx%d", _width, _height);
                    err::check_raise(err::ERR_RUNTIME, "image size not match");
                }
                if (0 != mmf_vo_frame_push_with_fit(this->_layer, this->_ch, new_img->data(), new_img->data_size(), new_img->width(), new_img->height(), mmf_invert_format_to_mmf(new_img->format()), mmf_fit)) {
                    log::error("mmf_vo_frame_push failed\n");
                    return err::ERR_RUNTIME;
                }

                if (format != image::FMT_BGRA8888) {
                    delete new_img;
                }
            } else {
                log::error("not support layer: %d\n", this->_layer);
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }

        void set_backlight(float value)
        {
            float max_duty = 50;
            _bl_pwm->duty(value * max_duty / 100.0);
            _bl_pwm->disable();
            if(value == 0)
                return;
            _bl_pwm->enable();
        }

        float get_backlight()
        {
            float max_duty = 50;
            return _bl_pwm->duty() / max_duty * 100;
        }

        int get_ch_nums()
        {
            return 2;
        }

        err::Err set_hmirror(bool en) {
            bool need_open = false;
            if (this->_opened) {
                this->close();
                need_open = true;
            }

            if (this->_layer == 0) {
                mmf_set_vo_video_hmirror(this->_ch, en);
            } else {
                err::check_raise(err::ERR_RUNTIME, "Not support layer");
            }

            if (need_open) {
                err::check_raise(this->open(this->_width, this->_height, this->_format), "Open failed");
            }
            return err::ERR_NONE;
        }

        err::Err set_vflip(bool en) {
            bool need_open = false;
            if (this->_opened) {
                this->close();
                need_open = true;
            }

            if (this->_layer == 0) {
                mmf_set_vo_video_flip(this->_ch, en);
            } else {
                err::check_raise(err::ERR_RUNTIME, "Not support layer");
            }

            if (need_open) {
                err::check_raise(this->open(this->_width, this->_height, this->_format), "Open failed");
            }
            return err::ERR_NONE;
        }

    private:
        int _width;
        int _height;
        int _max_width;
        int _max_height;
        image::Format _format;
        int _layer;
        int _ch;
        bool _opened;
        pwm::PWM *_bl_pwm;
    };
}
