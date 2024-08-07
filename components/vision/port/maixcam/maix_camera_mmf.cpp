/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_camera.hpp"
#include "maix_basic.hpp"
#include "maix_i2c.hpp"
#include <dirent.h>
#include "sophgo_middleware.hpp"

#define MMF_SENSOR_NAME "MMF_SENSOR_NAME"
#define MAIX_SENSOR_FPS "MAIX_SENSOR_FPS"

namespace maix::camera
{
    static bool set_regs_flag = false;

    std::vector<std::string> list_devices()
    {
        log::warn("This device is not driven using device files!");
        return std::vector<std::string>();
    }

    void set_regs_enable(bool enable) {
        log::warn("This operation is not supported!");
    }

    err::Err Camera::show_colorbar(bool enable)
    {
        // only set variable now
        // should control camera to show colorbar
        _show_colorbar = enable;
        return err::ERR_NONE;
    }

    static void generate_colorbar(image::Image &img)
    {
        int width = img.width();
        int height = img.height();
        int step = width / 8;
        int color_step = 255 / 7;
        int color = 0;
        uint8_t colors[8][3] = {
            {255, 255, 255},
            {255, 0, 0},
            {255, 127, 0},
            {255, 255, 0},
            {0, 255, 0},
            {0, 0, 255},
            {143, 0, 255},
            {0, 0, 0},
        };
        for (int i = 0; i < 8; i++)
        {
            image::Color _color(colors[i][0], colors[i][1], colors[i][2], 0, image::FMT_RGB888);
            img.draw_rect(i * step, 0, step, height, _color, -1);
            color += color_step;
        }
    }

    Camera::Camera(int width, int height, image::Format format, const char *device, int fps, int buff_num, bool open)
    {
        err::Err e;
        err::check_bool_raise(_check_format(format), "Format not support");

        _width = (width == -1) ? 640 : width;
        _height = (height == -1) ? 480 : height;
        _format = format;
        _buff_num = buff_num;
        _show_colorbar = false;
        _open_set_regs = set_regs_flag;
        _device =  "";
        _is_opened = false;
        _last_read_us = time::ticks_us();


        if (format == image::Format::FMT_RGB888 && _width * _height * 3 > 640 * 640 * 3) {
            log::warn("Note that we do not recommend using large resolution RGB888 images, which can take up a lot of memory!\r\n");
        }

        // config fps
        if (fps == -1 && _width <= 1280 && _height <= 720) {
            _fps = 60;
        } else if (fps == -1) {
            _fps = 30;
        } else {
            _fps = fps;
        }

        if ((_width > 1280 || _height > 720) && _fps > 30) {
            log::warn("Current fps is too high, will be be updated to 30fps! Currently only supported up to 720p 60fps or 1440p 30fps.\r\n");
            _fps = 30;
        } else if (_width <= 1280 && _height <= 720 && _fps > 60 && _fps != 80)  {
            log::warn("Currently only supports fixed 30,60 and 80fps in 720p configuration, current configuration will be updated to 80fps.\r\n");
            _fps = 80;
        } else if (_width <= 1280 && _height <= 720 && _fps < 80 && _fps > 30 && _fps != 60) {
            log::warn("Currently only supports fixed 30,60 and 80fps in 720p configuration, current configuration will be updated to 60fps.\r\n");
            _fps = 60;
        }

        // open camera
        if (open) {
            e = this->open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "camera open failed");
        }
    }

    Camera::~Camera()
    {
        if (this->is_opened()) {
            this->close();
        }
    }

    int Camera::get_ch_nums()
    {
        return 2;
    }

    int Camera::get_channel()
    {
        return this->_ch;
    }

    bool Camera::_check_format(image::Format format) {
        if (format == image::FMT_RGB888 || format == image::FMT_BGR888
        || format == image::FMT_RGBA8888 || format == image::FMT_BGRA8888
        || format == image::FMT_YVU420SP || format == image::FMT_GRAYSCALE) {
            return true;
        } else {
            return false;
        }
    }

    static char* _get_sensor_name(void)
    {
        static char name[30];
        peripheral::i2c::I2C i2c_obj(4, peripheral::i2c::Mode::MASTER);
        std::vector<int> addr_list = i2c_obj.scan();
        for (size_t i = 0; i < addr_list.size(); i++) {
            printf("i2c4 addr: 0x%02x\n", addr_list[i]);
            switch (addr_list[i]) {
                case 0x30:
                    printf("found sms_sc035gs, addr 0x30\n" );
                    snprintf(name, sizeof(name), "sms_sc035gs");
                    return name;
                case 0x29: // fall through
                default:
                    printf("found gcore_gc4653, addr 0x29\n" );
                    snprintf(name, sizeof(name), "gcore_gc4653");
                    return name;
            }
        }

        log::info("sensor address not found , use gcore_gc4653\n" );
        snprintf(name, sizeof(name), "gcore_gc4653");
        return name;
    }

    static void _config_sensor_env(int fps)
    {
        char *env_value = getenv(MMF_SENSOR_NAME);
        if (!env_value) {
            char *sensor_name = _get_sensor_name();
            err::check_null_raise(sensor_name, "sensor name not found!");
            setenv(MMF_SENSOR_NAME, sensor_name, 0);
        } else {
            log::info("Found MMF_SENSOR_NAME=%s", env_value);
        }

        env_value = getenv(MAIX_SENSOR_FPS);
        if (!env_value) {
            char new_value[10];
            snprintf(new_value, sizeof(new_value), "%d", fps);
            setenv(MAIX_SENSOR_FPS, new_value, 0);
        } else {
            log::info("Found MMF_SENSOR_FPS=%s", env_value);
        }
    }

    err::Err Camera::open(int width, int height, image::Format format, int fps, int buff_num)
    {
        int width_tmp = (width == -1) ? _width : width;
        int height_tmp = (height == -1) ? _height : height;
        image::Format format_tmp = (format == image::FMT_INVALID) ? _format : format;
        int fps_tmp = (fps == -1) ? _fps : fps;
        int buff_num_tmp =( buff_num == -1) ? _buff_num : buff_num;

        // check format
        err::check_bool_raise(_check_format(format_tmp), "Format not support");

        // check if we need update camera params
        if (this->is_opened()) {
            this->close();  // Get new param, close and reopen
        }

        _width = width_tmp;
        _height = height_tmp;
        _fps = fps_tmp;
        _buff_num = buff_num_tmp;
        _format = format_tmp;
        _format_impl = _format;

        // _format_impl is used to config mmf
        switch (_format) {
        case image::Format::FMT_RGB888: // fall through
        case image::Format::FMT_YVU420SP:
            break;
        case image::Format::FMT_BGR888:
            _format_impl = image::Format::FMT_RGB888;
            break;
        case image::Format::FMT_RGBA8888:
            _format_impl = image::Format::FMT_RGB888;
            break;
        case image::Format::FMT_BGRA8888:
            _format_impl = image::Format::FMT_RGB888;
            break;
        case image::Format::FMT_GRAYSCALE:
            _format_impl = image::Format::FMT_YVU420SP;
            break;
        default:
            err::check_raise(err::ERR_ARGS, "Format not support");
        }

        // config sensor env
        _config_sensor_env(_fps);

        // mmf init
        mmf_sys_cfg_t sys_cfg = {0};
        if (_width <= 1280 && _height <= 720 && _fps > 30) {
            sys_cfg.vb_pool[0].size = 1280 * 720 * 3 / 2;
            sys_cfg.vb_pool[0].count = 3;
            sys_cfg.vb_pool[0].map = 2;
            sys_cfg.max_pool_cnt = 1;
        } else {
            sys_cfg.vb_pool[0].size = 2560 * 1440 * 3 / 2;
            sys_cfg.vb_pool[0].count = 2;
            sys_cfg.vb_pool[0].map = 3;
            sys_cfg.max_pool_cnt = 1;
        }
        mmf_pre_config_sys(&sys_cfg);
        err::check_bool_raise(!mmf_init(), "mmf init failed");

        mmf_vi_cfg_t cfg = {0};
        cfg.w = _width;
        cfg.h = _height;
        cfg.fmt = mmf_invert_format_to_mmf(_format_impl);
        cfg.depth = _buff_num;
        cfg.fps = _fps;
        if (0 != mmf_vi_init2(&cfg)) {
            mmf_deinit();
            err::check_raise(err::ERR_RUNTIME, "mmf vi init failed");
        }

        err::check_bool_raise((_ch = mmf_get_vi_unused_channel()) >= 0, "mmf get vi channel failed");
        if (0 != mmf_add_vi_channel(_ch, _width, _height, mmf_invert_format_to_mmf(_format_impl))) {
            mmf_vi_deinit();
            mmf_deinit();
            err::check_raise(err::ERR_RUNTIME, "mmf add vi channel failed");
        }

        _is_opened = true;
        return err::ERR_NONE;
    }

    void Camera::close()
    {
        if (this->is_closed())
            return;

        if (mmf_vi_chn_is_open(this->_ch) == true) {
            if (0 != mmf_del_vi_channel(this->_ch)) {
                log::error("mmf del vi channel failed");
            }
        }

        mmf_deinit();
    }

    camera::Camera *Camera::add_channel(int width, int height, image::Format format, int fps, int buff_num, bool open)
    {
        err::check_bool_raise(_check_format(format), "Format not support");

        int width_tmp = (width == -1) ? _width : width;
        int height_tmp = (height == -1) ? _height : height;
        image::Format format_tmp = (format == image::Format::FMT_INVALID) ? _format : format;
        int fps_tmp = (fps == -1) ? _fps : fps;
        int buff_num_tmp = buff_num == -1 ? _buff_num : buff_num;

        Camera *cam = new Camera(width_tmp, height_tmp, format_tmp, _device.c_str(), fps_tmp, buff_num_tmp, true);
        return cam;
    }

    bool Camera::is_opened()
    {
        return _is_opened;
    }

    static image::Image *_mmf_read(int ch, int width_out, int height_out, image::Format format_out, void *buff = NULL, size_t buff_size = 0)
    {
        image::Image *img = NULL;
        uint8_t *image_data = NULL;
        image::Format pop_fmt;

        void *buffer = NULL;
        int buffer_len = 0, width = 0, height = 0, format = 0;
        if (0 == mmf_vi_frame_pop(ch, &buffer, &buffer_len, &width, &height, &format)) {
            int need_align = (width_out % mmf_vi_aligned_width(ch) == 0) ? false : true;
            if (buffer == NULL) {
                mmf_vi_frame_free(ch);
                return NULL;
            }

            if(buff) {
                img = new image::Image(width_out, height_out, format_out, (uint8_t*)buff, buff_size, false);
                if (buff_size < width * height * image::fmt_size[format_out]) {
                    log::error("camera read: buff size not enough, need %d, but %d", width * height * image::fmt_size[format_out], buff_size);
                    goto _error;
                }
            } else {
                img = new image::Image(width_out, height_out, format_out);
            }

            if (!img) {
                log::error("camera read: new image failed");
                goto _error;
            }
            image_data = (uint8_t *)img->data();
            pop_fmt = (image::Format)mmf_invert_format_to_maix(format);
            switch (img->format()) {
                case image::Format::FMT_GRAYSCALE:
                    if (pop_fmt != image::Format::FMT_YVU420SP) {
                        log::error("camera read: format not support, need %d, but %d", image::Format::FMT_YVU420SP, pop_fmt);
                        goto _error;
                    }
                    if (need_align) {
                        for (int h = 0; h < height_out; h ++) {
                            memcpy((uint8_t *)image_data + h * width_out, (uint8_t *)buffer + h * width, width_out);
                        }
                    } else {
                        memcpy(image_data, buffer, width_out * height_out);
                    }
                    break;
                case image::Format::FMT_RGB888:
                    if (pop_fmt != img->format()) {
                        log::error("camera read: format not support, need %d, but %d", img->format(), pop_fmt);
                        goto _error;
                    }
                    if (need_align) {
                        for (int h = 0; h < height_out; h++) {
                            memcpy((uint8_t *)image_data + h * width_out * 3, (uint8_t *)buffer + h * width * 3, width_out * 3);
                        }
                    } else {
                        memcpy(image_data, buffer, width_out * height_out * 3);
                    }
                    break;
                case image::Format::FMT_YVU420SP:
                    if (pop_fmt != img->format()) {
                        log::error("camera read: format not support, need %d, but %d", img->format(), pop_fmt);
                        goto _error;
                    }
                    if (need_align) {
                        for (int h = 0; h < height_out * 3 / 2; h ++) {
                            memcpy((uint8_t *)image_data + h * width_out, (uint8_t *)buffer + h * width, width_out);
                        }
                    } else {
                        memcpy(image_data, buffer, width_out * height_out * 3 / 2);
                    }
                    break;
                case image::Format::FMT_BGR888:
                {
                    if (pop_fmt != image::Format::FMT_RGB888) {
                        log::error("camera read: format not support, need %d, but %d", image::Format::FMT_RGB888, pop_fmt);
                        goto _error;
                    }

                    int bgr888_cnt = 0;
                    uint8_t *bgr888 = (uint8_t *)image_data;
                    uint8_t *rgb888 = (uint8_t *)buffer;
                    int line_size = width * 3;
                    for (int h = 0; h < height_out; h ++) {
                        for (int w = 0; w < width_out; w ++) {
                            uint8_t *tmp = rgb888 + h * line_size + w * 3;
                            bgr888[bgr888_cnt ++] = tmp[2];
                            bgr888[bgr888_cnt ++] = tmp[1];
                            bgr888[bgr888_cnt ++] = tmp[0];
                        }
                    }
                    break;
                }
                case image::Format::FMT_RGBA8888:
                {
                    if (pop_fmt != image::Format::FMT_RGB888) {
                        log::error("camera read: format not support, need %d, but %d", image::Format::FMT_RGB888, pop_fmt);
                        goto _error;
                    }

                    int rgba8888_cnt = 0;
                    uint8_t *rgba8888 = (uint8_t *)image_data;
                    uint8_t *rgb888 = (uint8_t *)buffer;
                    int line_size = width * 3;
                    for (int h = 0; h < height_out; h ++) {
                        for (int w = 0; w < width_out; w ++) {
                            uint8_t *tmp = rgb888 + h * line_size + w * 3;
                            rgba8888[rgba8888_cnt ++] = tmp[0];
                            rgba8888[rgba8888_cnt ++] = tmp[1];
                            rgba8888[rgba8888_cnt ++] = tmp[2];
                            rgba8888[rgba8888_cnt ++] = 0xff;
                        }
                    }
                    break;
                }
                case image::Format::FMT_BGRA8888:
                {
                    if (pop_fmt != image::Format::FMT_RGB888) {
                        log::error("camera read: format not support, need %d, but %d", image::Format::FMT_RGB888, pop_fmt);
                        goto _error;
                    }

                    int rgba8888_cnt = 0;
                    uint8_t *rgba8888 = (uint8_t *)image_data;
                    uint8_t *rgb888 = (uint8_t *)buffer;
                    int line_size = width * 3;
                    for (int h = 0; h < height_out; h ++) {
                        for (int w = 0; w < width_out; w ++) {
                            uint8_t *tmp = rgb888 + h * line_size + w * 3;
                            rgba8888[rgba8888_cnt ++] = tmp[2];
                            rgba8888[rgba8888_cnt ++] = tmp[1];
                            rgba8888[rgba8888_cnt ++] = tmp[0];
                            rgba8888[rgba8888_cnt ++] = 0xff;
                        }
                    }
                    break;
                }
                default:
                    printf("Read failed, unknown format:%d\n", img->format());
                    delete img;
                    mmf_vi_frame_free(ch);
                    return NULL;
            }
            mmf_vi_frame_free(ch);
            return img;
_error:
            if (img) {
                delete img;
                img = NULL;
            }
            mmf_vi_frame_free(ch);
            return NULL;
        }

        return img;
    } // read

    image::Image *Camera::read(void *buff, size_t buff_size, bool block)
    {
        if (!this->is_opened()) {
            err::Err e = open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "open camera failed");
        }

        if (_show_colorbar) {
            image::Image *img = new image::Image(_width, _height);
            generate_colorbar(*img);
            err::check_null_raise(img, "camera read failed");
            return img;
        } else {
            image::Image *img = _mmf_read(_ch, _width, _height, _format, buff, buff_size);
            err::check_null_raise(img, "camera read failed");

            // FIXME: delete me and fix driver bug
            uint64_t wait_us = 1000000 / _fps;
            while (time::ticks_us() - _last_read_us < wait_us) {
                time::sleep_us(50);
            }
            _last_read_us = time::ticks_us();
            return img;
        }
    }

    void Camera::clear_buff()
    {
        log::warn("This operation is not supported!");
    }

    void Camera::skip_frames(int num)
    {
        for(int i = 0; i < num; i++)
        {
            image::Image *img = this->read();
            delete img;
        }
    }

    err::Err Camera::set_resolution(int width, int height)
    {
        err::Err e;
        if (this->is_opened()) {
            this->close();
        }

        _width = width;
        _height = height;
        e = this->open(_width, _height, _format, _fps, _buff_num);
        err::check_raise(e, "camera open failed");
        return err::ERR_NONE;
    }

    int Camera::exposure(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        uint32_t out;
        if (value == -1) {
            mmf_get_exptime(_ch, &out);
        } else {
            mmf_set_exptime(_ch, value);
            out = value;
        }
        return out;
    }

    int Camera::gain(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        uint32_t out;
        if (value == -1) {
            mmf_get_again(_ch, &out);
        } else {
            mmf_set_again(_ch, value);
            out = value;
        }
        return out;
    }

    int Camera::hmirror(int value) {
        bool out;
        if (value == -1) {
            mmf_get_vi_hmirror(_ch, &out);
        } else {
            bool need_open = false;
            if (this->is_opened()) {
                this->close();
                need_open = true;
            }

            mmf_set_vi_hmirror(_ch, value);

            if (need_open) {
                err::check_raise(this->open(_width, _height, _format, _fps, _buff_num), "Open failed");
            }
            out = value;
        }

        return out;
    }

    int Camera::vflip(int value) {
        bool out;
        if (value == -1) {
            mmf_get_vi_vflip(_ch, &out);
        } else {
            bool need_open = false;
            if (this->is_opened()) {
                this->close();
                need_open = true;
            }

            mmf_set_vi_vflip(_ch, value);

            if (need_open) {
                err::check_raise(this->open(_width, _height, _format, _fps, _buff_num), "Open failed");
            }
            out = value;
        }
        return out;
    }

    int Camera::luma(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        uint32_t out;
        if (value == -1) {
            mmf_get_luma(_ch, &out);
        } else {
            mmf_set_luma(_ch, value);
            out = value;
        }
        return out;
    }

    int Camera::constrast(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        uint32_t out;
        if (value == -1) {
            mmf_get_constrast(_ch, &out);
        } else {
            mmf_set_constrast(_ch, value);
            out = value;
        }
        return out;
    }

    int Camera::saturation(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        uint32_t out;
        if (value == -1) {
            mmf_get_saturation(_ch, &out);
        } else {
            mmf_set_saturation(_ch, value);
            out = value;
        }
        return out;
    }

    int Camera::awb_mode(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        uint32_t out;
        if (value == -1) {
            out = mmf_get_wb_mode(_ch);
        } else {
            mmf_set_wb_mode(_ch, value);
            out = value;
        }

        err::check_bool_raise(out >= 0, "set white balance failed");
        return out;
    }

    int Camera::set_awb(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        if (value == 0) {
            value = 1;
        } else if (value > 0) {
            value = 0;
        }

        return this->awb_mode(value) == 0 ? 1 : 0;
    }

    int Camera::exp_mode(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        uint32_t out;
        if (value == -1) {
            out = mmf_get_exp_mode(_ch);
        } else {
            mmf_set_exp_mode(_ch, value);
            out = value;
        }

        err::check_bool_raise(out >= 0, "set exposure failed");
        return out;
    }

    err::Err Camera::set_windowing(std::vector<int> roi) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        int max_height, max_width;
        char log_msg[100];
        int x = 0, y = 0, w = 0, h = 0;

        err::check_bool_raise(!mmf_vi_get_max_size(&max_width, &max_height), "get max size of camera failed");

        if (roi.size() == 4) {
            x = roi[0], y = roi[1], w = roi[2], h = roi[3];
        } else if (roi.size() == 2) {
            w = roi[0], h = roi[1];
            x = (max_width - w) / 2;
            y = (max_height - h) / 2;
        } else {
            err::check_raise(err::ERR_RUNTIME, "roi size must be 4 or 2");
        }

        snprintf(log_msg, sizeof(log_msg), "Width must be a multiple of 64.");
        err::check_bool_raise(w % 64 == 0, std::string(log_msg));
        snprintf(log_msg, sizeof(log_msg), "the coordinate x range needs to be [0,%d].", max_width - 1);
        err::check_bool_raise(x >= 0 || x < max_width, std::string(log_msg));
        snprintf(log_msg, sizeof(log_msg), "the coordinate y range needs to be [0,%d].", max_height - 1);
        err::check_bool_raise(y >= 0 || y < max_height, std::string(log_msg));
        snprintf(log_msg, sizeof(log_msg), "the row of the window is larger than the maximum, try x=%d, w=%d.", x, max_width - x);
        err::check_bool_raise(x + w <= max_width, std::string(log_msg));
        snprintf(log_msg, sizeof(log_msg), "the column of the window is larger than the maximum, try y=%d, h=%d.", y, max_height - y);
        err::check_bool_raise(y + h <= max_height, std::string(log_msg));

        bool is_vflip = false, is_hmirror = false;
        mmf_get_vi_hmirror(_ch, &is_hmirror);
        mmf_get_vi_vflip(_ch, &is_vflip);
        if (!is_vflip) {
            y = max_height - y - h;
        }
        if (!is_hmirror) {
            x = max_width - x - w;
        }
        err::check_bool_raise(!mmf_vi_channel_set_windowing(_ch,  x, y, w, h), "set windowing failed.");
        return err::ERR_NONE;
    }
}

