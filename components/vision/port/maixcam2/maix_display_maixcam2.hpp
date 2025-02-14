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
#include "maix_pwm.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include "maix_fs.hpp"

using namespace maix::peripheral;

namespace maix::display
{
    enum class PanelType {
        NORMAL,
        LT9611,
        UNKNOWN,
    };
    inline static PanelType __g_panel_type = PanelType::UNKNOWN;

    __attribute__((unused)) static int _get_vo_max_size(int *width, int *height, int rotate)
    {
        int w = 0, h = 0;

        if (rotate) {
            *width = h;
            *height = w;
        } else {
            *width = w;
            *height = h;
        }
        return 0;
    }


    static void _get_disp_configs(bool &flip, bool &mirror, float &max_backlight) {
        std::string flip_str;
        bool flip_is_found = false;

        auto device_configs = sys::device_configs();
        auto it = device_configs.find("disp_flip");
        if (it != device_configs.end()) {
            flip_str = it->second;
            flip_is_found = true;
        }
        auto it2 = device_configs.find("disp_mirror");
        if (it2 != device_configs.end()) {
            if (it2->second.size() > 0)
                mirror = atoi(it2->second.c_str());
        }
        auto it3 = device_configs.find("disp_max_backlight");
        if (it3 != device_configs.end()) {
            if (it3->second.size() > 0)
                max_backlight = atof(it3->second.c_str());
        }

        if (flip_is_found && flip_str.size() > 0) {
            flip = atoi(flip_str.c_str());
        } else {
            std::string board_id = sys::device_id();
            if (board_id == "maixcam_pro") {
                flip = true;
            }
        }

        // log::info("disp config flip: %d, mirror: %d, max_backlight: %.1f", flip, mirror, max_backlight);
    }


    class DisplayAx final : public DisplayBase
    {
    public:
        DisplayAx(const string &device, int width, int height, image::Format format)
        {
            err::check_bool_raise(!_get_vo_max_size(&_max_width, &_max_height, 1), "get vo max size failed");
            width = width <= 0 ? _max_width : width;
            height = height <= 0 ? _max_height : height;
            this->_width = width > _max_width ? _max_width : width;
            this->_height = height > _max_height ? _max_height : height;
            this->_format = format;
            this->_opened = false;
            this->_format = format;
            this->_invert_flip = false;
            this->_invert_mirror = false;
            this->_layer = 0;       // layer 0 means vedio layer
            err::check_bool_raise(_format == image::FMT_RGB888
                                || _format == image::FMT_YVU420SP
                                || _format == image::FMT_BGRA8888, "Format not support");

            int pwm_id = 10;
            _bl_pwm = new pwm::PWM(pwm_id, 100000, 20);
        }

        DisplayAx(int layer, int width, int height, image::Format format)
        {
            err::check_bool_raise(!_get_vo_max_size(&_max_width, &_max_height, 1), "get vo max size failed");
            width = width <= 0 ? _max_width : width;
            height = height <= 0 ? _max_height : height;
            this->_width = width > _max_width ? _max_width : width;
            this->_height = height > _max_height ? _max_height : height;
            this->_format = format;
            this->_opened = false;
            this->_format = format;
            this->_invert_flip = false;
            this->_invert_mirror = false;
            this->_max_backlight = 50.0;
            this->_layer = layer;       // layer 0 means vedio layer
                                        // layer 1 means osd layer
            err::check_bool_raise(_format == image::FMT_BGRA8888, "Format not support");

            int pwm_id = 10;
            _bl_pwm = new pwm::PWM(pwm_id, 100000, 20);
        }

        ~DisplayAx()
        {
            if(_bl_pwm && this->_layer == 0)    // _layer = 0, means video layer
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

            this->_opened = true;
            return err::ERR_NONE;
        }

        err::Err close()
        {
            if (!this->_opened)
                return err::ERR_NONE;

            this->_opened = false;
            return err::ERR_NONE;
        }

        display::DisplayAx *add_channel(int width, int height, image::Format format)
        {
            int new_width = 0;
            int new_height = 0;
            image::Format new_format = format;
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

            _format = new_format;
            DisplayAx *disp = new DisplayAx(1, new_width, new_height, new_format);
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

            return err::ERR_NONE;
        }

        void set_backlight(float value)
        {
            _bl_pwm->duty(value * _max_backlight / 100.0);
            _bl_pwm->disable();
            if(value == 0)
                return;
            _bl_pwm->enable();
        }

        float get_backlight()
        {
            return _bl_pwm->duty() / _max_backlight * 100;
        }

        int get_ch_nums()
        {
            return 2;
        }

        err::Err set_hmirror(bool en) {
            en = _invert_mirror ? !en : en;

            bool need_open = false;
            if (this->_opened) {
                this->close();
                need_open = true;
            }

            if (need_open) {
                err::check_raise(this->open(this->_width, this->_height, this->_format), "Open failed");
            }
            return err::ERR_NONE;
        }

        err::Err set_vflip(bool en) {
            en = _invert_flip ? !en : en;

            bool need_open = false;
            if (this->_opened) {
                this->close();
                need_open = true;
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
        bool _invert_flip;
        bool _invert_mirror;
        float _max_backlight;
        pwm::PWM *_bl_pwm;
    };
}
