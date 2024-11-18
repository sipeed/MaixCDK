/**
 * @file maix_display_fb.hpp
 * @brief Maix display framebuffer implementation
 * @author 916BGAI
 * @license Apache 2.0 Sipeed Ltd
 * @update date 2024-11-13 Create by 916BGAI
 */

#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include "maix_display_base.hpp"
#include "maix_image.hpp"
#include "maix_pwm.hpp"
#include "opencv2/opencv.hpp"

#if __riscv_vector
#include <riscv_vector.h>
#endif

namespace maix::display
{

    static int invert_rgb888_to_rgb565(unsigned char *src, int srcw, int srch, unsigned char *dst)
    {
        if (!src || !dst)
            return -1;

        uint16_t *out = (uint16_t *)dst;

#if __riscv_vector
        for (int y = 0; y < srch; ++y) {
            unsigned char *src_row = src + y * srcw * 3;
            uint16_t *out_row = out + y * srcw;

            int x = 0;
            size_t vl;
            for (; (vl = vsetvl_e8m1(srcw - x)) > 0; x += vl) {
                vuint8m1_t vecR = vlse8_v_u8m1(src_row + x * 3 + 0, 3, vl);
                vuint8m1_t vecG = vlse8_v_u8m1(src_row + x * 3 + 1, 3, vl);
                vuint8m1_t vecB = vlse8_v_u8m1(src_row + x * 3 + 2, 3, vl);

                vuint16m2_t vecR5 = vsrl_vx_u16m2(vwcvtu_x_x_v_u16m2(vecR, vl), 3, vl);
                vuint16m2_t vecG6 = vsrl_vx_u16m2(vwcvtu_x_x_v_u16m2(vecG, vl), 2, vl);
                vuint16m2_t vecB5 = vsrl_vx_u16m2(vwcvtu_x_x_v_u16m2(vecB, vl), 3, vl);

                vuint16m2_t pixel = vor_vv_u16m2(vor_vv_u16m2(vsll_vx_u16m2(vecR5, 11, vl), vsll_vx_u16m2(vecG6, 5, vl), vl), vecB5, vl);

                vse16_v_u16m2(out_row + x, pixel, vl);
            }
        }
#else
        for (int y = 0; y < srch; ++y) {
            unsigned char *src_row = src + y * srcw * 3;
            uint16_t *out_row = out + y * srcw;

            for (int x = 0; x < srcw; ++x) {
                unsigned char R = src_row[x * 3 + 0];
                unsigned char G = src_row[x * 3 + 1];
                unsigned char B = src_row[x * 3 + 2];

                uint16_t pixel = ((R >> 3) << 11) |
                                 ((G >> 2) << 5)  |
                                 (B >> 3);

                out_row[x] = pixel;
            }
        }
#endif
        return 0;
    }

    static int invert_yvu420sp_to_rgb565(unsigned char *src, int srcw, int srch, unsigned char *dst)
    {
        if (!src || !dst)
            return -1;

        cv::Mat yvu_img(srch + srch / 2, srcw, CV_8UC1, src);
        cv::Mat rgb_img;
        cv::cvtColor(yvu_img, rgb_img, cv::COLOR_YUV2RGB_NV21);

        uint16_t *out = (uint16_t *)dst;

#if __riscv_vector
        for (int y = 0; y < srch; ++y) {
            unsigned char *src_row = rgb_img.data + y * srcw * 3;
            uint16_t *out_row = out + y * srcw;

            int x = 0;
            size_t vl;
            for (; (vl = vsetvl_e8m1(srcw - x)) > 0; x += vl) {
                vuint8m1_t vecR = vlse8_v_u8m1(src_row + x * 3 + 0, 3, vl);
                vuint8m1_t vecG = vlse8_v_u8m1(src_row + x * 3 + 1, 3, vl);
                vuint8m1_t vecB = vlse8_v_u8m1(src_row + x * 3 + 2, 3, vl);

                vuint16m2_t vecR5 = vsrl_vx_u16m2(vwcvtu_x_x_v_u16m2(vecR, vl), 3, vl);
                vuint16m2_t vecG6 = vsrl_vx_u16m2(vwcvtu_x_x_v_u16m2(vecG, vl), 2, vl);
                vuint16m2_t vecB5 = vsrl_vx_u16m2(vwcvtu_x_x_v_u16m2(vecB, vl), 3, vl);

                vuint16m2_t pixel = vor_vv_u16m2(vor_vv_u16m2(vsll_vx_u16m2(vecR5, 11, vl), vsll_vx_u16m2(vecG6, 5, vl), vl), vecB5, vl);

                vse16_v_u16m2(out_row + x, pixel, vl);
            }
        }
#else
        for (int y = 0; y < srch; ++y) {
            unsigned char *src_row = rgb_img.data + y * srcw * 3;
            uint16_t *out_row = out + y * srcw;

            for (int x = 0; x < srcw; ++x) {
                unsigned char R = src_row[x * 3 + 0];
                unsigned char G = src_row[x * 3 + 1];
                unsigned char B = src_row[x * 3 + 2];

                uint16_t pixel = ((R >> 3) << 11) |
                                 ((G >> 2) << 5)  |
                                 (B >> 3);

                out_row[x] = pixel;
            }
        }
#endif
        return 0;
    }

    static int invert_gray_to_rgb565(unsigned char *src, int srcw, int srch, unsigned char *dst)
    {
        if (!src || !dst)
            return -1;

        uint16_t *out = (uint16_t *)dst;

#if __riscv_vector
        for (int y = 0; y < srch; ++y) {
            unsigned char *src_row = src + y * srcw;
            uint16_t *out_row = out + y * srcw;

            int x = 0;
            size_t vl;
            for (; (vl = vsetvl_e8m1(srcw - x)) > 0; x += vl) {
                vuint8m1_t vecGray = vle8_v_u8m1(src_row + x, vl);
                vuint16m2_t vecGray16 = vwcvtu_x_x_v_u16m2(vecGray, vl);

                vuint16m2_t vecR5 = vsrl_vx_u16m2(vecGray16, 3, vl);
                vuint16m2_t vecG6 = vsrl_vx_u16m2(vecGray16, 2, vl);
                vuint16m2_t vecB5 = vsrl_vx_u16m2(vecGray16, 3, vl);

                vuint16m2_t pixel = vor_vv_u16m2(vor_vv_u16m2(vsll_vx_u16m2(vecR5, 11, vl), vsll_vx_u16m2(vecG6, 5, vl), vl), vecB5, vl);

                vse16_v_u16m2(out_row + x, pixel, vl);
            }
        }
#else
        for (int y = 0; y < srch; ++y) {
            unsigned char *src_row = src + y * srcw;
            uint16_t *out_row = out + y * srcw;

            for (int x = 0; x < srcw; ++x) {
                unsigned char gray = src_row[x];

                uint16_t pixel = ((gray >> 3) << 11) |
                                 ((gray >> 2) << 5)  |
                                 (gray >> 3);

                out_row[x] = pixel;
            }
        }
#endif
        return 0;
    }

    class FB_Display final : public DisplayBase
    {
    public:
        FB_Display(const string &device, int width, int height, image::Format format)
        {
            this->_width = width;
            this->_height = height;
            this->_format = format;
            this->_device = device;
            this->_opened = false;

            err::check_bool_raise(_format == image::FMT_RGB888
                                || _format == image::FMT_YVU420SP
                                || _format == image::FMT_BGRA8888, "Format not support");
#ifdef PLATFORM_MAIXCAM
            int pwm_id = 10;
            _bl_pwm = new pwm::PWM(pwm_id, 100000, 20);
#endif
        }

        ~FB_Display()
        {
            close();
#ifdef PLATFORM_MAIXCAM
            if(_bl_pwm)
            {
                delete _bl_pwm;
            }
#endif
        }

        int width()
        {
            return _width;
        }

        int height()
        {
            return _height;
        }

        std::vector<int> size()
        {
            return {_width, _height};
        }

        image::Format format()
        {
            switch (_bpp) {
                case 16: return image::FMT_RGB565;
                case 18: return image::FMT_RGB565;
                case 24: return image::FMT_BGR888;
                case 32: return image::FMT_BGRA8888;
                default: return image::FMT_RGB888;
            }
        }

        err::Err open(int width, int height, image::Format format)
        {
            if (_opened) {
                return err::ERR_NONE;
            }

            struct fb_var_screeninfo vinfo;
            struct fb_fix_screeninfo finfo;
            _fbfd = ::open(_device.c_str(), O_RDWR);
            if (_fbfd == -1) {
                log::error("Error opening %s", _device.c_str());
                return err::ERR_RUNTIME;
            }
            if (ioctl(_fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
                log::error("Error reading fixed information from %s", _device.c_str());
                ::close(_fbfd);
                return err::ERR_IO;
            }
            if (ioctl(_fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
                log::error("Error reading variable information from %s", _device.c_str());
                ::close(_fbfd);
                return err::ERR_IO;
            }

            _width = vinfo.xres;
            _height = vinfo.yres;
            _xres_virtual = vinfo.xres_virtual;
            _yres_virtual = vinfo.yres_virtual;
            _bpp = vinfo.bits_per_pixel;
            _line_length = finfo.line_length;
            _screensize = vinfo.yres_virtual * finfo.line_length;

            if (_bpp != 16 && _bpp != 18 && _bpp != 24 && _bpp != 32) {
                log::error("Not support bpp: %d", _bpp);
                ::close(_fbfd);
                return err::ERR_ARGS;
            }

            _fbp = (unsigned char *)mmap(0, _screensize, PROT_READ | PROT_WRITE, MAP_SHARED, _fbfd, 0);
            if (_fbp == MAP_FAILED) {
                log::error("Error mapping framebuffer to memory");
                ::close(_fbfd);
                return err::ERR_NO_MEM;
            }

            _opened = true;
            return err::ERR_NONE;
        }

        err::Err close()
        {
            if (!this->_opened)
                return err::ERR_NONE;

            memset(_fbp, 0, _screensize);
            munmap(_fbp, _screensize);
            ::close(_fbfd);
            _opened = false;
            return err::ERR_NONE;
        }

        display::FB_Display *add_channel(int width, int height, image::Format format)
        {
            return NULL;
        }

        bool is_opened()
        {
            return _opened;
        }

        err::Err show(image::Image &img, image::Fit fit)
        {
            image::Image *img_ptr = &img;
            image::Image *fit_img_ptr = nullptr;
            image::Format format = img_ptr->format();

            if (img.width() != _width || img.height() != _height) {
                if (format == image::FMT_YVU420SP) {
                    fit_img_ptr = img_ptr->resize(_width, _height, image::FIT_FILL);
                } else {
                    switch (fit) {
                        case image::Fit::FIT_FILL: fit_img_ptr = img_ptr->resize(_width, _height, image::FIT_FILL); break;
                        case image::Fit::FIT_CONTAIN: fit_img_ptr = img_ptr->resize(_width, _height, image::FIT_CONTAIN); break;
                        case image::Fit::FIT_COVER: fit_img_ptr = img_ptr->resize(_width, _height, image::FIT_COVER); break;
                        default: fit_img_ptr = img_ptr->resize(_width, _height, image::FIT_FILL); break;
                    }
                }
                img_ptr = fit_img_ptr;
            }

            if (format == image::FMT_BGRA8888)
            {
                if (_bpp == 16 || _bpp == 18) {
                    image::Image *bgra8888_img = nullptr;
                    bgra8888_img = img_ptr->to_format(image::Format::FMT_RGB888);
                    invert_rgb888_to_rgb565((uint8_t *)bgra8888_img->data(), bgra8888_img->width(), bgra8888_img->height(), _fbp);
                    delete bgra8888_img;
                } else if (_bpp == 24) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGR888, _fbp, _screensize, false);
                    img->draw_image(0, 0, *img_ptr);
                    delete img;
                } else if (_bpp == 32) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGRA8888, _fbp, _screensize, false);
                    img->draw_image(0, 0, *img_ptr);
                    delete img;
                }
            }
            else if (format == image::FMT_RGB888)
            {
                if (_bpp == 16 || _bpp == 18) {
                    invert_rgb888_to_rgb565((uint8_t *)img_ptr->data(), img_ptr->width(), img_ptr->height(), _fbp);
                } else if (_bpp == 24) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGR888, _fbp, _screensize, false);
                    img->draw_image(0, 0, *img_ptr);
                    delete img;
                } else if (_bpp == 32) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGRA8888, _fbp, _screensize, false);
                    img->draw_image(0, 0, *img_ptr);
                    delete img;
                }
            }
            else if (format == image::FMT_GRAYSCALE)
            {
                if (_bpp == 16 || _bpp == 18) {
                    invert_gray_to_rgb565((uint8_t *)img_ptr->data(), img_ptr->width(), img_ptr->height(), _fbp);
                } else if (_bpp == 24) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGR888, _fbp, _screensize, false);
                    img->draw_image(0, 0, *img_ptr);
                    delete img;
                } else if (_bpp == 32) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGRA8888, _fbp, _screensize, false);
                    img->draw_image(0, 0, *img_ptr);
                    delete img;
                }
            }
            else if (format == image::FMT_YVU420SP)
            {
                if (_bpp == 16 || _bpp == 18) {
                    invert_yvu420sp_to_rgb565((uint8_t *)img_ptr->data(), img_ptr->width(), img_ptr->height(), _fbp);
                } else if (_bpp == 24) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGR888, _fbp, _screensize, false);
                    cv::Mat cv_yvu_img(img_ptr->height() + img_ptr->height() / 2, img_ptr->width(), CV_8UC1, img_ptr->data());
                    cv::Mat cv_rgb_img;
                    cv::cvtColor(cv_yvu_img, cv_rgb_img, cv::COLOR_YUV2RGB_NV21);
                    image::Image *rgb_img = new image::Image(_width, _height, image::Format::FMT_RGB888, cv_rgb_img.data, -1, false);
                    img->draw_image(0, 0, *rgb_img);
                    delete img;
                    delete rgb_img;
                } else if (_bpp == 32) {
                    image::Image *img = new image::Image(_line_length / (_bpp / 8), _yres_virtual, image::Format::FMT_BGRA8888, _fbp, _screensize, false);
                    cv::Mat cv_yvu_img(img_ptr->height() + img_ptr->height() / 2, img_ptr->width(), CV_8UC1, img_ptr->data());
                    cv::Mat cv_rgb_img;
                    cv::cvtColor(cv_yvu_img, cv_rgb_img, cv::COLOR_YUV2RGB_NV21);
                    image::Image *rgb_img = new image::Image(_width, _height, image::Format::FMT_RGB888, cv_rgb_img.data, -1, false);
                    img->draw_image(0, 0, *rgb_img);
                    delete img;
                    delete rgb_img;
                }
            }
            else
            {
                log::error("not support format: %d\n", format);
                return err::ERR_ARGS;
            }

            if (fit_img_ptr != nullptr) {
                delete fit_img_ptr;
                fit_img_ptr = nullptr;
            }

            return err::ERR_NONE;
        }

        void set_backlight(float value)
        {
#ifdef PLATFORM_MAIXCAM
            float max_duty = 50;
            _bl_pwm->duty(value * max_duty / 100.0);
            _bl_pwm->disable();
            if(value == 0)
                return;
            _bl_pwm->enable();
#else
            return;
#endif
        }

        float get_backlight()
        {
#ifdef PLATFORM_MAIXCAM
            float max_duty = 50;
            return _bl_pwm->duty() / max_duty * 100;
#else
            return 0.0;
#endif
        }

        int get_ch_nums()
        {
            return 1;
        }

        /**
         * Set display mirror
         * @param en enable/disable mirror
         */
        virtual err::Err set_hmirror(bool en)
        {
            return err::Err::ERR_NOT_IMPL;
        }

        /**
         * Set display flip
         * @param en enable/disable flip
         */
        virtual err::Err set_vflip(bool en)
        {
            return err::Err::ERR_NOT_IMPL;
        }

    private:
        int _width;
        int _height;
        image::Format _format;
        std::string _device = "";
        bool _opened;
        int _fbfd = 0;
        unsigned char *_fbp = 0;
        unsigned int _xres_virtual;
        unsigned int _yres_virtual;
        unsigned int _line_length;
        long int _screensize = 0;
        int _bpp;
#ifdef PLATFORM_MAIXCAM
        pwm::PWM *_bl_pwm;
#endif
    };
}