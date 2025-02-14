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

    typedef struct {
        int dev;
        int i2c_addr;
        bool raw;
        double fps;
        int exptime_max;    // unit:us
        int exptime_min;
    } camera_priv_t;

    Camera::Camera(int width, int height, image::Format format, const char *device, double fps, int buff_num, bool open, bool raw)
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
        _last_read_us = time::ticks_us();
        _invert_flip = false;
        _invert_mirror = false;
        _is_opened = false;

        camera_priv_t *priv = (camera_priv_t *)malloc(sizeof(camera_priv_t));
        err::check_null_raise(priv, "camera_priv_t malloc error");
        memset(priv, 0, sizeof(camera_priv_t));
        priv->dev = 0;
        priv->raw = raw;
        priv->fps = fps;
        _param = priv;


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

        if (_param) {
            free(_param);
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

    static bool _get_board_config_path(char *path, int path_size)
    {
        if (fs::exists("/boot/board")) {
            snprintf(path, path_size, "/boot/board");
            return true;
        }
        return false;
    }

    static int _get_mclk_id(void) {
        char path[64];
        int mclk_id = 0;

        err::check_bool_raise(_get_board_config_path(path, sizeof(path)), "Can't find board config file");

        std::string mclk_id_str;
        auto device_configs = sys::device_configs();
        auto it = device_configs.find("cam_mclk");
        if (it != device_configs.end()) {
            mclk_id_str = it->second;
        }
        if (!mclk_id_str.empty()) {
            mclk_id = atoi(mclk_id_str.c_str());
        } else {
            std::string board_id = sys::device_id();
            if (board_id == "maixcam_pro") {
                mclk_id = 0;
            } else {
                mclk_id = 1;
            }
        }
        return mclk_id;
    }

    static std::vector<int> _get_lane_id_from_board_file() {
        std::vector<int> lane_id;
        auto device_configs = sys::device_configs();
        auto it = device_configs.find("lane_id");
        if (it != device_configs.end()) {
            auto lane_id_str = it->second;
            std::string item;
            std::stringstream ss(lane_id_str);
            while (std::getline(ss, item, ',')) {
                lane_id.push_back(std::stoi(item));
            }
        }

        return lane_id;
    }

    static std::vector<int> _get_pn_swap_from_board_file() {
        std::vector<int> pn_swap;
        auto device_configs = sys::device_configs();
        auto it = device_configs.find("pn_swap");
        if (it != device_configs.end()) {
            auto pn_swap_str = it->second;
            std::string item;
            std::stringstream ss(pn_swap_str);
            while (std::getline(ss, item, ',')) {
                pn_swap.push_back(std::stoi(item));
            }
        }

        return pn_swap;
    }

    static std::vector<bool> _get_cam_flip_mirror(void) {
        char path[64];
        bool flip = 0, mirror = 0;
        bool flip_is_found = false, mirror_is_found = false;

        err::check_bool_raise(_get_board_config_path(path, sizeof(path)), "Can't find board config file");

        std::string flip_str;
        std::string mirror_str;
        auto device_configs = sys::device_configs();
        auto it = device_configs.find("cam_flip");
        if (it != device_configs.end()) {
            flip_str = it->second;
            flip_is_found = true;
        }
        auto it2 = device_configs.find("cam_mirror");
        if (it2 != device_configs.end()) {
            mirror_str = it2->second;
            mirror_is_found = true;
        }

        std::string board_id = sys::device_id();
        // log::info("cam flip=%s, cam mirror=%s", flip_str, mirror_str);
        if (flip_is_found && !flip_str.empty()) {
            flip = atoi(flip_str.c_str());
        } else {
            if (board_id == "maixcam_pro") {
                flip = true;
            } else {
                flip = false;
            }
        }

        if (mirror_is_found && !mirror_str.empty()) {
            mirror = atoi(mirror_str.c_str());
        } else {
            std::string board_id = sys::device_id();
            if (board_id == "maixcam_pro") {
                mirror = true;
            } else {
                mirror = false;
            }
        }

        return {flip, mirror};
    }

    static std::pair<bool, std::string> _get_sensor_name(void)
    {
        char name[30];
        peripheral::i2c::I2C i2c_obj(4, peripheral::i2c::Mode::MASTER);

        std::vector<int> addr_list = i2c_obj.scan();
        for (size_t i = 0; i < addr_list.size(); i++) {
            // log::info("i2c4 addr: 0x%02x", addr_list[i]);
            switch (addr_list[i]) {
                case 0x29:
                    // log::info("find gcore_gc4653, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "gcore_gc4653");
                    return {true, name};
                case 0x30:
                    // log::info("find sms_sc035gs, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "sms_sc035gs");
                    return {true, name};
                case 0x2b:
                    // log::info("find lt6911, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "lt6911");
                    return {true, name};
                case 0x36:
                    // log::info("find ov_os04a10, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "ov_os04a10");
                    return {true, name};
                case 0x37:
                    // log::info("find ov_os04a10, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "gcore_gc02m1");
                    return {true, name};
                case 0x48:// fall through
                case 0x3c:
                    // log::info("find ov_ov2685, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "ov_ov2685");
                    return {true, name};
                default: break;
            }
        }

        // log::info("sensor address not found , use gcore_gc4653\n" );
        snprintf(name, sizeof(name), "gcore_gc4653");
        return {false, name};
    }

    std::string get_device_name()
    {
        std::string device_name;
        std::pair<bool, std::string> res = _get_sensor_name();
        if (res.first == false) {
            device_name = "";
            return device_name;
        } else {
            device_name = res.second;
        }
        return device_name;
    }


    static int _is_module_in_use(const char *module_name) {
        FILE *fp;
        char buffer[256];

        fp = fopen("/proc/modules", "r");
        if (fp == NULL) {
            perror("fopen");
            return -1;
        }

        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            char mod_name[256];
            int usage_count;

            sscanf(buffer, "%255s %*s %d", mod_name, &usage_count);

            if (strcmp(mod_name, module_name) == 0) {
                fclose(fp);
                return usage_count > 0;
            }
        }

        fclose(fp);
        return 0;
    }

    err::Err Camera::open(int width, int height, image::Format format, double fps, int buff_num)
    {
        // int width_tmp = (width == -1) ? _width : width;
        // int height_tmp = (height == -1) ? _height : height;
        // image::Format format_tmp = (format == image::FMT_INVALID) ? _format : format;
        // double fps_tmp = (fps == -1) ? _fps : fps;
        // int buff_num_tmp =( buff_num == -1) ? _buff_num : buff_num;
        // camera_priv_t *priv = (camera_priv_t *)_param;

        // check format
        // err::check_bool_raise(_check_format(format_tmp), "Format not support");

        _is_opened = true;
        return err::ERR_NONE;
    }

    void Camera::close()
    {
        if (this->is_closed())
            return;
    }

    camera::Camera *Camera::add_channel(int width, int height, image::Format format, double fps, int buff_num, bool open)
    {
        err::check_bool_raise(_check_format(format), "Format not support");

        int width_tmp = (width == -1) ? _width : width;
        int height_tmp = (height == -1) ? _height : height;
        image::Format format_tmp = (format == image::Format::FMT_INVALID) ? _format : format;
        double fps_tmp = (fps == -1) ? _fps : fps;
        int buff_num_tmp = buff_num == -1 ? _buff_num : buff_num;

        Camera *cam = new Camera(width_tmp, height_tmp, format_tmp, _device.c_str(), fps_tmp, buff_num_tmp, true);
        return cam;
    }

    bool Camera::is_opened()
    {
        return _is_opened;
    }

    image::Image *Camera::read(void *buff, size_t buff_size, bool block, int block_ms)
    {
        if (!this->is_opened()) {
            err::Err e = open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "open camera failed");
        }
        return nullptr;
    }

    // static image::Format _get_raw_format_with_size(int w, int h, int total_size, BAYER_FORMAT_E bayer_format) {
    //     image::Format format = image::FMT_INVALID;
    //     int size = w * h;
    //     if (total_size == size * 0.75) {
    //         switch (bayer_format) {
    //         case BAYER_FORMAT_BG:
    //             format = image::FMT_BGGR6;
    //             break;
    //         case BAYER_FORMAT_GB:
    //             format = image::FMT_GBRG6;
    //             break;
    //         case BAYER_FORMAT_GR:
    //             format = image::FMT_GRBG6;
    //             break;
    //         case BAYER_FORMAT_RG:
    //             format = image::FMT_RGGB6;
    //             break;
    //         default:
    //             return image::FMT_INVALID;
    //         }
    //     } else if (total_size == size * 1) {
    //         switch (bayer_format) {
    //         case BAYER_FORMAT_BG:
    //             format = image::FMT_BGGR8;
    //             break;
    //         case BAYER_FORMAT_GB:
    //             format = image::FMT_GBRG8;
    //             break;
    //         case BAYER_FORMAT_GR:
    //             format = image::FMT_GRBG8;
    //             break;
    //         case BAYER_FORMAT_RG:
    //             format = image::FMT_RGGB8;
    //             break;
    //         default:
    //             return image::FMT_INVALID;
    //         }
    //     } else if (total_size == size * 1.25) {
    //         switch (bayer_format) {
    //         case BAYER_FORMAT_BG:
    //             format = image::FMT_BGGR10;
    //             break;
    //         case BAYER_FORMAT_GB:
    //             format = image::FMT_GBRG10;
    //             break;
    //         case BAYER_FORMAT_GR:
    //             format = image::FMT_GRBG10;
    //             break;
    //         case BAYER_FORMAT_RG:
    //             format = image::FMT_RGGB10;
    //             break;
    //         default:
    //             return image::FMT_INVALID;
    //         }
    //     } else if (total_size == size * 1.5) {
    //         switch (bayer_format) {
    //         case BAYER_FORMAT_BG:
    //             format = image::FMT_BGGR12;
    //             break;
    //         case BAYER_FORMAT_GB:
    //             format = image::FMT_GBRG12;
    //             break;
    //         case BAYER_FORMAT_GR:
    //             format = image::FMT_GRBG12;
    //             break;
    //         case BAYER_FORMAT_RG:
    //             format = image::FMT_RGGB12;
    //             break;
    //         default:
    //             return image::FMT_INVALID;
    //         }
    //     } else {
    //         return image::FMT_INVALID;
    //     }

    //     return format;
    // }

    image::Image *Camera::read_raw() {
        if (!this->is_opened()) {
            err::Err e = open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "open camera failed");
        }

        camera_priv_t *priv = (camera_priv_t *)this->_param;
        if (!priv->raw) {
            err::check_raise(err::ERR_NOT_READY, "you need to enable the raw parameter when constructing the Camera object.");
        }

        return nullptr;
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

        return err::ERR_NONE;
    }

    static void _config_extern_register_of_os04a10(int exptime_ms)
    {
        char cmd[128];
        if (exptime_ms >= 1 * 1000) {
            double scale = 1.587;       // (0x0c68 - 0x0635) / 1000
            double exp_time_ms = exptime_ms;
            uint64_t reg_val = (uint16_t)((exp_time_ms - 1000) * scale) + 0x0635;
            reg_val = reg_val > 0xffff ? 0xffff : reg_val;
            snprintf(cmd, sizeof(cmd), "i2ctransfer -fy 4 w4@0x36 0x38 0x0c %#.2x %#.2x", (uint8_t)((reg_val >> 8) & 0xff), (uint8_t)(reg_val & 0xff));
            system(cmd);
        } else {
            uint64_t reg_val = 0x05cc;
            snprintf(cmd, sizeof(cmd), "i2ctransfer -fy 4 w4@0x36 0x38 0x0c %#.2x %#.2x", (uint8_t)((reg_val >> 8) & 0xff), (uint8_t)(reg_val & 0xff));
            system(cmd);
        }
    }

    err::Err Camera::set_fps(double fps) {

        return err::ERR_NONE;
    }

    int Camera::exposure(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        return 0;
    }

    int Camera::gain(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        return 0;
    }

    int Camera::hmirror(int value) {

        return 0;
    }

    int Camera::vflip(int value) {

        return 0;
    }

    int Camera::luma(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }
        return 0;
    }

    int Camera::constrast(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }
        return 0;
    }

    int Camera::saturation(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }
        return 0;
    }

    int Camera::awb_mode(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }
        return 0;
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

        return value;
    }

    err::Err Camera::set_windowing(std::vector<int> roi) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        return err::ERR_NONE;
    }

    std::vector<int> Camera::get_sensor_size() {
        camera_priv_t *priv = (camera_priv_t *)_param;

        return {640, 480};
    }

    err::Err Camera::write_reg(int addr, int data, int bit_width)
    {
        camera_priv_t *priv = (camera_priv_t *)_param;
        (void)bit_width;
        peripheral::i2c::I2C i2c_obj(4, peripheral::i2c::Mode::MASTER);
        uint8_t temp[1];
        temp[0] = (uint8_t)data;
        i2c_obj.writeto_mem(priv->i2c_addr, addr, temp, sizeof(temp), 16);
        return err::ERR_NONE;
    }

    int Camera::read_reg(int addr, int bit_width)
    {
        camera_priv_t *priv = (camera_priv_t *)_param;
        (void)bit_width;
        peripheral::i2c::I2C i2c_obj(4, peripheral::i2c::Mode::MASTER);
        Bytes *data = i2c_obj.readfrom_mem(priv->i2c_addr, addr, 1, 16);
        int out = -1;log::info("addr:%#x", priv->i2c_addr);
        if (data) {
            if (data->size() > 0) {
                out = data->data[0];
            }
            delete data;
        }

        return out;
    }
}

