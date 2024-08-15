/**
 * @file maix_display.hpp
 * @brief Maix display SDL implementation
 * @author neucrack@sipeed.com
 * @license Apache 2.0 Sipeed Ltd
 * @update date 2023-10-23 Create by neucrack
*/

#include "maix_display.hpp"
#include "maix_log.hpp"
#include "global_config.h"
#include "maix_image_trans.hpp"
#ifdef PLATFORM_LINUX
    #include "maix_display_sdl.hpp"
#endif
#ifdef PLATFORM_MAIXCAM
    #include "maix_display_mmf.hpp"
#endif

namespace maix::display
{

    static ImageTrans *img_trans = nullptr;

    std::vector<std::string> list_devices()
    {
        return {"0"};
    }

    Display::Display(int width, int height, image::Format format, const char *device, bool open)
    {
        _impl = NULL;

        // Select implementation by platform
#ifdef PLATFORM_LINUX
        _impl = new SDL_Display(device, width, height, format);
#endif
#ifdef PLATFORM_MAIXCAM
        _impl = new DisplayCviMmf(device, width, height, format);
#endif
        if (open) {
            err::Err e = this->open();
            err::check_raise(e, "display open failed");
        }

        char line[1024];
        char panel_value[256];

        char *panel_env = getenv("MMF_PANEL_NAME");
        if (panel_env) {
            log::info("Found panel env MMF_PANEL_NAME=%s\r\n", panel_env);
            strncpy(panel_value, panel_env, sizeof(panel_value));
        } else {
            FILE *file = fopen("/boot/uEnv.txt", "r");
            if (file == NULL) {
                perror("Error opening uEnv.txt");
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

        if (!strcmp(panel_value, "MaixCam_Pro")) {
            this->set_vflip(true);
        }
    }

    Display::Display(const char *device, DisplayBase *base, int width, int height, image::Format format, bool open)
    {
        err::Err e;
        _impl = base;

        if (open) {
            e = this->open();
            err::check_raise(e, "display open failed");
        }
    }

    Display::~Display()
    {
#ifdef PLATFORM_LINUX
        delete (SDL_Display *)_impl;
#endif
#ifdef PLATFORM_MAIXCAM
        delete (DisplayCviMmf *)_impl;
#endif
    }

    int Display::get_ch_nums()
    {
        return _impl->get_ch_nums();
    }

    err::Err Display::open(int width, int height, image::Format format)
    {
        if (_impl == NULL)
            return err::Err::ERR_RUNTIME;

        int width_tmp = (width == -1) ? this->width() : width;
        int height_tmp = (height == -1) ? this->height() : height;
        image::Format format_tmp = (format == image::FMT_INVALID) ? this->format() : format;

        if (this->is_opened()) {
            if (width == width_tmp && height == height_tmp && format == format_tmp) {
                return err::ERR_NONE;
            }
            this->close();  // Get new param, close and reopen
        }
        std::string bl_v_str = app::get_sys_config_kv("backlight", "value");
        float bl_v = 50;
        try
        {
            if(!bl_v_str.empty())
                bl_v = atof(bl_v_str.c_str());
        }
        catch(...)
        {
            bl_v = 50;
        }
        this->set_backlight(bl_v);

        if(!img_trans && maixvision_mode())
        {
            img_trans = new ImageTrans(maixvision_image_fmt());
        }
        return _impl->open(width_tmp, height_tmp, format_tmp);
    }

    err::Err Display::close()
    {
        return _impl->close();
    }

    display::Display *Display::add_channel(int width, int height, image::Format format, bool open)
    {
        int width_tmp = (width == -1) ? this->width() : width;
        int height_tmp = (height == -1) ? this->height() : height;
        image::Format format_tmp = (format == image::Format::FMT_INVALID) ? this->format() : format;

        Display *disp = NULL;
        if (_impl) {
            DisplayBase *disp_base = _impl->add_channel(width_tmp, height_tmp, format_tmp);
            err::check_bool_raise(disp_base, "Unable to add a new channel. Please check the maximum number of supported channels.");
            disp = new Display(this->device().c_str(), disp_base, width_tmp, height_tmp, format_tmp, open);
        }
        return disp;
    }

    bool Display::is_opened()
    {
        return _impl->is_opened();
    }

    int Display::width()
    {
        return _impl->width();
    }

    int Display::height()
    {
        return _impl->height();
    }

    std::vector<int> Display::size()
    {
        return _impl->size();
    }

    image::Format Display::format()
    {
        return _impl->format();
    }

    err::Err Display::show(image::Image &img, image::Fit fit)
    {
        err::Err e = err::ERR_NONE;

        if(img_trans)
            img_trans->send_image(img);

        if (!is_opened())
        {
            log::debug("display not opened, now auto open\n");
            e = open(this->width(), this->height(), this->format());
            if (e != err::ERR_NONE)
            {
                log::error("open display failed: %d\n", e);
                return e;
            }
        }

#ifdef PLATFORM_MAIXCAM
        maix::image::Format show_img_format = img.format();
        if (show_img_format != maix::image::Format::FMT_RGB888
        && show_img_format != maix::image::Format::FMT_YVU420SP
        && show_img_format != maix::image::Format::FMT_BGRA8888
        && show_img_format != maix::image::Format::FMT_GRAYSCALE) {
            image::Image *show_img = img.to_format(maix::image::Format::FMT_RGB888);
            if (show_img == NULL) {
                log::error("image format convert failed\n");
                return err::ERR_RUNTIME;
            }

            _impl->show(*show_img, fit);
            delete show_img;
        } else {
            _impl->show(img, fit);
        }
        return e;
#else
        image::Image *show_img = NULL;
        bool show_img_need_delete = false;
        if (fit == image::FIT_NONE)
        {
            // img is bigger than display size, crop it
            if (img.width() > _impl->width() || img.height() > _impl->height())
            {
                show_img = img.crop(0, 0, _impl->width(), _impl->height());
                show_img_need_delete = true;
            }
            else
            {
                show_img = &img;
            }
        }
        else if (img.width() != _impl->width() || img.height() != _impl->height())
        {
            show_img = img.resize(_impl->width(), _impl->height(), fit);
            show_img_need_delete = true;
        }
        else
        {
            show_img = &img;
        }

        if (img.format() != _impl->format()) {
            image::Image *impl_fmt_img = show_img->to_format(_impl->format());
            if (impl_fmt_img == NULL) {
                log::error("image format convert failed\n");
                return err::ERR_RUNTIME;
            }

            e = _impl->show(*impl_fmt_img);
            delete impl_fmt_img;
        } else {
            e = _impl->show(*show_img);
        }

        if (show_img_need_delete) {
            delete show_img;
        }
#endif
        return e;
    }

    void Display::set_backlight(float value)
    {
        if (value < 0)
            value = 0;
        if (value > 100)
            value = 100;
        _impl->set_backlight(value);
    }

    float Display::get_backlight()
    {
        return _impl->get_backlight();
    }

    err::Err Display::set_hmirror(bool en) {
        if (_impl == NULL)
            return err::ERR_NOT_INIT;

        return _impl->set_hmirror(en);
    }

    err::Err Display::set_vflip(bool en) {
        if (_impl == NULL)
            return err::ERR_NOT_INIT;

        return _impl->set_vflip(en);
    }

    void send_to_maixvision(image::Image &img)
    {
        if(img_trans)
            img_trans->send_image(img);
        else if(maixvision_mode())
        {
            img_trans = new ImageTrans(maixvision_image_fmt());
            img_trans->send_image(img);
        }

    }

} // namespace maix::display
