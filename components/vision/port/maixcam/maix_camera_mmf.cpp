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

#define MMF_SENSOR_NAME "MMF_SENSOR_NAME"                           // Setting the sensor name will be used to select which driver to use
#define MAIX_SENSOR_FPS "MAIX_SENSOR_FPS"                           // Set the frame rate, whether it takes effect or not is determined by the driver
#define MMF_INIT_DO_NOT_RELOAD_KMOD "MMF_INIT_DO_NOT_RELOAD_KMOD"   // Disable reloading of kmod on mmf_init

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
        SAMPLE_SNS_TYPE_E sns_type;
        ISP_BAYER_FORMAT_E bayer_fmt;
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
        // TODO: supports dual sensor
        int res = CVI_MIPI_SetSensorReset(0, 0);
		if (res != CVI_SUCCESS) {
			log::error("sensor 0 unreset failed!\n");
			return {false, std::string()};
		}
        int retry_count = 0;
        char name[30];
        peripheral::i2c::I2C i2c_obj(4, peripheral::i2c::Mode::MASTER);

_retry:
        if (retry_count < 1) {
            int mclk_id = _get_mclk_id();
            if (mclk_id == 0) {
                system("devmem 0x0300116C 32 0x5"); // MIPI RX 4N PINMUX MCLK0
                system("devmem 0x0300118C 32 0x3"); // MIPI RX 0N PINMUX MIPI RX 0N
            } else if (mclk_id == 1) {
                system("devmem 0x0300116C 32 0x3"); // MIPI RX 4N PINMUX MIPI RX 4N
                system("devmem 0x0300118C 32 0x5"); // MIPI RX 0N PINMUX MCLK1
            } else {
                system("devmem 0x0300116C 32 0x3"); // MIPI RX 4N PINMUX MIPI RX 4N
                system("devmem 0x0300118C 32 0x3"); // MIPI RX 0N PINMUX MCLK1
            }
            retry_count ++;
            goto _retry;
        }

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

    static void _config_sensor_env(double fps)
    {
        char *env_value = getenv(MMF_SENSOR_NAME);
        if (!env_value) {
            std::pair<bool, std::string> res = _get_sensor_name();
            char *sensor_name = (char *)res.second.c_str();
            err::check_null_raise(sensor_name, "sensor name not found!");
            setenv(MMF_SENSOR_NAME, sensor_name, 0);
        } else {
            log::info("Find MMF_SENSOR_NAME=%s", env_value);
        }

        env_value = getenv(MAIX_SENSOR_FPS);
        if (!env_value) {
            char new_value[10];
            snprintf(new_value, sizeof(new_value), "%d", (int)fps);      // only gc4653 used
            setenv(MAIX_SENSOR_FPS, new_value, 0);
        } else {
            log::info("Find MMF_SENSOR_FPS=%s", env_value);
        }
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

    static int reinit_soph_vb(void)
    {
        // if (access("/mnt/system/ko/soph_mipi_tx.ko", F_OK) != -1) {
        // 	system("mv /mnt/system/ko/soph_mipi_tx.ko /mnt/system/ko/soph_mipi_tx.ko.bak");
        // 	system("reboot");
        // }

        system("rmmod soph_ive soph_vc_driver soph_rgn soph_dwa soph_vo soph_vpss soph_vi soph_snsr_i2c soph_mipi_rx soph_fast_image soph_rtos_cmdqu soph_base");
        // system("insmod /mnt/system/ko/soph_sys.ko");
        system("insmod /mnt/system/ko/soph_base.ko");
        system("insmod /mnt/system/ko/soph_rtos_cmdqu.ko");
        system("insmod /mnt/system/ko/soph_fast_image.ko");
        system("insmod /mnt/system/ko/soph_mipi_rx.ko");
        system("insmod /mnt/system/ko/soph_snsr_i2c.ko");
        system("insmod /mnt/system/ko/soph_vi.ko");
        system("insmod /mnt/system/ko/soph_vpss.ko");
        system("insmod /mnt/system/ko/soph_vo.ko");
        system("insmod /mnt/system/ko/soph_dwa.ko");
        system("insmod /mnt/system/ko/soph_rgn.ko");
        system("insmod /mnt/system/ko/soph_vc_driver.ko");
        system("insmod /mnt/system/ko/soph_ive.ko");

        return 0;
    }

    static int _get_vb_pool_cnt(void)
    {
        FILE *file;
        char line[1024];
        int poolIdCount = 0;

        file = fopen("/proc/cvitek/vb", "r");
        if (file == NULL) {
            perror("can not open /proc/cvitek/vb");
            return 0;
        }

        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "PoolId(") != NULL) {
                poolIdCount++;
            }
        }

        fclose(file);
        return poolIdCount;
    }

    static int _mmf_vi_init(const char *board_id, int width, int height, double fps, camera_priv_t *priv)
    {
        SIZE_S stSize;
        PIC_SIZE_E enPicSize;
        SAMPLE_INI_CFG_S	stIniCfg;
        SAMPLE_VI_CONFIG_S	stViConfig;
        PIXEL_FORMAT_E vi_format = PIXEL_FORMAT_NV21;
        PIXEL_FORMAT_E vi_vpss_format = PIXEL_FORMAT_NV21;

        // config vi param
        char *sensor_name = getenv(MMF_SENSOR_NAME);
        err::check_null_raise(sensor_name, "sensor name not found!");
        err::check_bool_raise(!SAMPLE_COMM_VI_ParseIni(&stIniCfg), "sensor cfg parse complete!");
        err::check_bool_raise(stIniCfg.devNum != 0, "Not device found! devnum = 0");

        struct {
            SAMPLE_SNS_TYPE_E sns_type = GCORE_GC4653_MIPI_4M_30FPS_10BIT;
            std::vector<int> lane_id = std::vector<int>(5);
            std::vector<int> pn_swap = std::vector<int>(5);
            int mclk_en = 1;
            int mclk = 1;
            int i2c_addr;
            uint32_t exptime_max;      // unit:us
            uint32_t exptime_min;
        } sensor_cfg;

        if (!strcmp(board_id, "maixcam_pro")) {
            sensor_cfg.mclk = 0;
            if (!strcmp(sensor_name, "sms_sc035gs")) {
                sensor_cfg.sns_type = SMS_SC035GS_MIPI_480P_120FPS_12BIT;
                sensor_cfg.lane_id = {0, 1, -1, -1, -1};
                sensor_cfg.pn_swap = {1, 1, 0, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x30;
                sensor_cfg.exptime_max = 800000;
                sensor_cfg.exptime_min = 8333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_YUV_400;
            } else if (!strcmp(sensor_name, "ov_ov2685")) {
                sensor_cfg.sns_type = GCORE_OV2685_MIPI_1600x1200_30FPS_10BIT;
                sensor_cfg.lane_id = {0, 1, 2, -1, -1};
                sensor_cfg.pn_swap = {0, 0, 0, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x3c;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
                err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.ov2685"), "set config path failed!");
            } else if (!strcmp(sensor_name, "lt6911")) {
                sensor_cfg.sns_type = LONTIUM_LT6911_2M_60FPS_8BIT;
                sensor_cfg.lane_id = {2, 0, 1, 3, 4};
                sensor_cfg.pn_swap = {1, 1, 1, 1, 1};
                sensor_cfg.mclk_en = 0;
                sensor_cfg.i2c_addr = 0x2b;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_UYVY;
                vi_vpss_format = PIXEL_FORMAT_UYVY;
            } else if (!strcmp(sensor_name, "ov_os04a10")) {
                if (width <= 1280 && height <= 720 && fps >= 80) {
                    sensor_cfg.sns_type = OV_OS04A10_MIPI_4M_720P90_12BIT;
                } else {
                    sensor_cfg.sns_type = OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT;
                }
                sensor_cfg.lane_id = {2, 3, 1, 4, 0};
                sensor_cfg.pn_swap = {1, 1, 1, 1, 1};
                sensor_cfg.mclk_en = 0;
                sensor_cfg.i2c_addr = 0x36;
                sensor_cfg.exptime_max = 40000000;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
                err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.os04a10"), "set config path failed!");
            } else if (!strcmp(sensor_name, "gcore_gc02m1")) {
                sensor_cfg.sns_type = GCORE_GC02M1_MIPI_2M_30FPS_10BIT;
                sensor_cfg.lane_id = {0, 1, -1, -1, -1};
                sensor_cfg.pn_swap = {1, 1, 0, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x37;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
                err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.gc02m1"), "set config path failed!");
            } else { // default is gcore_gc4653
                if (width <= 1280 && height <= 720 && fps > 30) {
                    sensor_cfg.sns_type = GCORE_GC4653_MIPI_720P_60FPS_10BIT;
                } else {
                    sensor_cfg.sns_type = GCORE_GC4653_MIPI_4M_30FPS_10BIT;
                }
                sensor_cfg.lane_id = {0, 1, 2, -1, -1};
                sensor_cfg.pn_swap = {1, 1, 1, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x29;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
            }
        } else if (!strcmp(board_id, "maixcam")) {
            sensor_cfg.mclk = 1;
            if (!strcmp(sensor_name, "sms_sc035gs")) {
                sensor_cfg.sns_type = SMS_SC035GS_MIPI_480P_120FPS_12BIT;
                sensor_cfg.lane_id = {4, 3, -1, -1, -1};
                sensor_cfg.pn_swap = {0, 0, 0, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x30;
                sensor_cfg.exptime_max = 800000;
                sensor_cfg.exptime_min = 8333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_YUV_400;
            } else if (!strcmp(sensor_name, "ov_ov2685")) {
                sensor_cfg.sns_type = GCORE_OV2685_MIPI_1600x1200_30FPS_10BIT;
                sensor_cfg.lane_id = {4, 3, 2, -1, -1};
                sensor_cfg.pn_swap = {1, 1, 1, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x3c;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
                err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.ov2685"), "set config path failed!");
            } else if (!strcmp(sensor_name, "lt6911")) {
                sensor_cfg.sns_type = LONTIUM_LT6911_2M_60FPS_8BIT;
                sensor_cfg.lane_id = {2, 4, 3, 1, 0};
                sensor_cfg.pn_swap = {0, 0, 0, 0, 0};
                sensor_cfg.mclk_en = 0;
                sensor_cfg.i2c_addr = 0x2b;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_UYVY;
                vi_vpss_format = PIXEL_FORMAT_UYVY;
            } else if (!strcmp(sensor_name, "ov_os04a10")) {
                if (width <= 1280 && height <= 720 && fps >= 80) {
                    sensor_cfg.sns_type = OV_OS04A10_MIPI_4M_720P90_12BIT;
                } else {
                    sensor_cfg.sns_type = OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT;
                }
                sensor_cfg.lane_id = {2, 1, 3, 0, 4};
                sensor_cfg.pn_swap = {0, 0, 0, 0, 0};
                sensor_cfg.mclk_en = 0;
                sensor_cfg.i2c_addr = 0x36;
                sensor_cfg.exptime_max = 40000000;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
                err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.os04a10"), "set config path failed!");
            } else if (!strcmp(sensor_name, "gcore_gc02m1")) {
                sensor_cfg.sns_type = GCORE_GC02M1_MIPI_2M_30FPS_10BIT;
                sensor_cfg.lane_id = {4, 3, -1, -1, -1};
                sensor_cfg.pn_swap = {0, 0, 0, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x37;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
                err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.gc02m1"), "set config path failed!");
            } else { // default is gcore_gc4653
                if (width <= 1280 && height <= 720 && fps > 30) {
                    sensor_cfg.sns_type = GCORE_GC4653_MIPI_720P_60FPS_10BIT;
                } else {
                    sensor_cfg.sns_type = GCORE_GC4653_MIPI_4M_30FPS_10BIT;
                }
                sensor_cfg.lane_id = {4, 3, 2, -1, -1};
                sensor_cfg.pn_swap = {0, 0, 0, 0, 0};
                sensor_cfg.mclk_en = 1;
                sensor_cfg.i2c_addr = 0x29;
                sensor_cfg.exptime_max = 363636;
                sensor_cfg.exptime_min = 33333;
                vi_format = PIXEL_FORMAT_NV21;
                vi_vpss_format = PIXEL_FORMAT_NV21;
            }
        } else {
            err::check_raise(err::ERR_RUNTIME, "unknown board name!");
        }

        auto board_config_lane_id = _get_lane_id_from_board_file();
        if (board_config_lane_id.size() == 5) {
            sensor_cfg.lane_id = board_config_lane_id;
        }

        auto board_config_pn_swap = _get_pn_swap_from_board_file();
        if (board_config_pn_swap.size() == 5) {
            sensor_cfg.pn_swap = board_config_pn_swap;
        }

        stIniCfg.enSnsType[0] = sensor_cfg.sns_type;
        stIniCfg.stMclkAttr[0].bMclkEn = sensor_cfg.mclk_en;
        stIniCfg.stMclkAttr[0].u8Mclk = sensor_cfg.mclk;
        for (int i = 0; i < 5; i++) {
            stIniCfg.as16LaneId[0][i] = sensor_cfg.lane_id[i];
        }
        for (int i = 0; i < 5; i++) {
            stIniCfg.as8PNSwap[0][i] = sensor_cfg.pn_swap[i];
        }

        ISP_PUB_ATTR_S stPubAttr;
        memset(&stPubAttr, 0, sizeof(ISP_PUB_ATTR_S));
        SAMPLE_COMM_SNS_GetIspAttrBySns(sensor_cfg.sns_type, &stPubAttr);
        priv->sns_type = sensor_cfg.sns_type;
        priv->bayer_fmt = stPubAttr.enBayer;
        priv->i2c_addr = sensor_cfg.i2c_addr;
        priv->exptime_max = sensor_cfg.exptime_max;
        priv->exptime_min = sensor_cfg.exptime_min;

        err::check_bool_raise(!mmf_init_v2(false), "mmf init failed");
        err::check_bool_raise(!SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig), "IniToViCfg failed!");
        if (priv->raw) {
            stViConfig.astViInfo[0].stChnInfo.enCompressMode = COMPRESS_MODE_NONE;
        }
        err::check_bool_raise(!SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize), "GetSizeBySensor failed!");
        err::check_bool_raise(!SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize), "GetPicSize failed!");

        if (0 !=  mmf_vi_init_v2(stSize.u32Width, stSize.u32Height, vi_format, vi_vpss_format, fps, 3, &stViConfig)) {
            mmf_deinit_v2(false);
            err::check_raise(err::ERR_RUNTIME, "mmf vi init failed");
        }

        ISP_EXPOSURE_ATTR_S stExpAttr;
        memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));
        err::check_bool_raise(CVI_SUCCESS == CVI_ISP_GetExposureAttr(0, &stExpAttr), "GetExposureAttr failed!");
        stExpAttr.stAuto.stExpTimeRange.u32Max = 1 * 1000 * 1000;
        err::check_bool_raise(CVI_SUCCESS == CVI_ISP_SetExposureAttr(0, &stExpAttr), "SetExposureAttr failed!");
        return  0;
    }

    err::Err Camera::open(int width, int height, image::Format format, double fps, int buff_num)
    {
        int width_tmp = (width == -1) ? _width : width;
        int height_tmp = (height == -1) ? _height : height;
        image::Format format_tmp = (format == image::FMT_INVALID) ? _format : format;
        double fps_tmp = (fps == -1) ? _fps : fps;
        int buff_num_tmp =( buff_num == -1) ? _buff_num : buff_num;
        camera_priv_t *priv = (camera_priv_t *)_param;

        // check format
        err::check_bool_raise(_check_format(format_tmp), "Format not support");

        // release old pool
        if (!mmf_is_init()) {
            int old_pool_cnt = _get_vb_pool_cnt();
            if (old_pool_cnt > 0) {
                if (_is_module_in_use("soph_vi") == 0) {
                    reinit_soph_vb();
                }
            }
            setenv("MMF_INIT_DO_NOT_RELOAD_KMOD", "1", 0);
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
        auto flip_and_mirror = _get_cam_flip_mirror();
        _invert_flip = flip_and_mirror[0];
        _invert_mirror = flip_and_mirror[1];
        _config_sensor_env(_fps);

        const char *board_id = sys::device_id().c_str();
        if (!strcmp(getenv(MMF_SENSOR_NAME), "sms_sc035gs")) {
            _fps = priv->fps;
            if (_fps == -1 && _width <= 640 && _height <= 480) {
                _fps = 90;
            } else if (_fps == -1) {
                _fps = 60;
            }
            if ((_width > 640 || _height > 480) && _fps > 60) {
                log::warn("Current fps is too high, will be be updated to 60fps! Currently only supported up to 480p 180fps.\r\n");
                _fps = 60;
            } else if (_width <= 640 && _height <= 480 && _fps > 180)  {
                log::warn("Current fps is too high, will be be updated to 180fps! Currently only supported up to 480p 180fps.\r\n");
                _fps = 180;
            }
            char new_value[10];
            snprintf(new_value, sizeof(new_value), "%d", (int)_fps);
            setenv(MAIX_SENSOR_FPS, new_value, 0);
        } else if (!strcmp(getenv(MMF_SENSOR_NAME), "ov_os04a10")) {
            if (_width <= 1280 && _height <= 720 && priv->fps >= 80) {
                priv->fps = 90;
                _fps = 90;
            }
        }

        int pool_num = 3;
        if (priv->sns_type == SMS_SC035GS_MIPI_480P_120FPS_12BIT) {
            pool_num = 4;
        }

        if (is_opened()) {
            if (0 != mmf_del_vi_channel(_ch)) {
                mmf_vi_deinit();
                mmf_deinit_v2(false);
                err::check_raise(err::ERR_RUNTIME, "mmf add vi channel failed");
            }
            err::check_bool_raise((_ch = mmf_get_vi_unused_channel()) >= 0, "mmf get vi channel failed");
            if (0 != mmf_add_vi_channel_v2(_ch, _width, _height, mmf_invert_format_to_mmf(_format_impl), _fps, 2, -1, -1, 2, pool_num)) {
                mmf_vi_deinit();
                mmf_deinit_v2(false);
                err::check_raise(err::ERR_RUNTIME, "mmf add vi channel failed");
            }
        } else {
            // mmf init
            err::check_bool_raise(!_mmf_vi_init(board_id, _width, _height, _fps, priv), "mmf vi init failed");
            err::check_bool_raise((_ch = mmf_get_vi_unused_channel()) >= 0, "mmf get vi channel failed");
            mmf_set_vi_vflip(_ch, _invert_flip);
            mmf_set_vi_hmirror(_ch, _invert_mirror);

            if (0 != mmf_add_vi_channel_v2(_ch, _width, _height, mmf_invert_format_to_mmf(_format_impl), _fps, 2, -1, -1, 2, pool_num)) {
                mmf_vi_deinit();
                mmf_deinit_v2(false);
                err::check_raise(err::ERR_RUNTIME, "mmf add vi channel failed");
            }
        }

        // wait camera is ready
        VIDEO_FRAME_INFO_S frame;
        CVI_U32 s32Ret;
        if ((s32Ret = CVI_VPSS_GetChnFrame(0, _ch, &frame, 3000 + (CVI_S32)(1000.0 / _fps * 3))) != CVI_SUCCESS) {
            SAMPLE_PRT("vi get frame timeout: 0x%x !\n", s32Ret);
            return err::ERR_RUNTIME;
        }
        CVI_VPSS_ReleaseChnFrame(0, _ch, &frame);

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

        // bool vi_need_deinit = true;
        // for (int i = 0; i < this->get_ch_nums(); i ++) {
        //     if (mmf_vi_chn_is_open(i)) {
        //         vi_need_deinit = false;
        //     }
        // }

        // if (vi_need_deinit) {
        //     mmf_vi_deinit();
        // }

        mmf_deinit_v2(false);
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

    static int _mmf_vi_frame_pop(int ch, void **frame_info,  mmf_frame_info_t *frame_info_mmap, int block_ms) {
        if (frame_info == NULL || frame_info_mmap == NULL) {
            printf("invalid param\n");
            return -1;
        }

        int ret = -1;
        VIDEO_FRAME_INFO_S *frame = (VIDEO_FRAME_INFO_S *)malloc(sizeof(VIDEO_FRAME_INFO_S));
        memset(frame, 0, sizeof(VIDEO_FRAME_INFO_S));
        if ((ret = CVI_VPSS_GetChnFrame(0, ch, frame, (CVI_S32)block_ms)) == 0) {
            int image_size = frame->stVFrame.u32Length[0]
                            + frame->stVFrame.u32Length[1]
                            + frame->stVFrame.u32Length[2];
            CVI_VOID *vir_addr;
            vir_addr = CVI_SYS_MmapCache(frame->stVFrame.u64PhyAddr[0], image_size);
            CVI_SYS_IonInvalidateCache(frame->stVFrame.u64PhyAddr[0], vir_addr, image_size);

            frame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)vir_addr;		// save virtual address for munmap
            frame_info_mmap->data = vir_addr;
            frame_info_mmap->len = image_size;
            frame_info_mmap->w = frame->stVFrame.u32Width;
            frame_info_mmap->h = frame->stVFrame.u32Height;
            frame_info_mmap->fmt = frame->stVFrame.enPixelFormat;
        } else {
            free(frame);
            frame = NULL;
        }

        if (frame_info) {
            *frame_info = frame;
        }
        return ret;
    }

    static void _mmf_vi_frame_free(int ch, void **frame_info)
    {
        if (!frame_info || !*frame_info) {
            return;
        }

        VIDEO_FRAME_INFO_S *frame = (VIDEO_FRAME_INFO_S *)*frame_info;
        int image_size = frame->stVFrame.u32Length[0]
                            + frame->stVFrame.u32Length[1]
                            + frame->stVFrame.u32Length[2];
        CVI_SYS_Munmap(frame->stVFrame.pu8VirAddr[0], image_size);
        if (CVI_VPSS_ReleaseChnFrame(0, ch, frame) != 0) {
            SAMPLE_PRT("CVI_VI_ReleaseChnFrame NG\n");
        }

        free(*frame_info);
        *frame_info = NULL;
    }

    static image::Image *_mmf_read(int ch, int width_out, int height_out, image::Format format_out, void *buff = NULL, size_t buff_size = 0, int block_ms = 1000)
    {
        image::Image *img = NULL;
        uint8_t *image_data = NULL;
        image::Format pop_fmt;

        void *pframe = NULL;
        mmf_frame_info_t frame_info;


        void *buffer = NULL;
        int width = 0, height = 0, format = 0;
        if (0 == _mmf_vi_frame_pop(ch, &pframe, &frame_info, block_ms)) {
            VIDEO_FRAME_INFO_S *frame = (VIDEO_FRAME_INFO_S *)pframe;
            buffer = frame->stVFrame.pu8VirAddr[0];
            width = frame->stVFrame.u32Width;
            height = frame->stVFrame.u32Height;
            format = frame->stVFrame.enPixelFormat;
            int need_align = (width_out % mmf_vi_aligned_width(ch) == 0) ? false : true;
            if (buffer == NULL) {
                _mmf_vi_frame_free(ch, &pframe);
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
                    _mmf_vi_frame_free(ch, &pframe);
                    return NULL;
            }
            _mmf_vi_frame_free(ch, &pframe);
            return img;
_error:
            if (img) {
                delete img;
                img = NULL;
            }
            _mmf_vi_frame_free(ch, &pframe);
            return NULL;
        } else {
            log::error("camera read timeout");
        }

        return img;
    } // read

    image::Image *Camera::read(void *buff, size_t buff_size, bool block, int block_ms)
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
            int read_block_ms = block_ms < 0 ? (1000.0 / _fps * 3) : block_ms;
            image::Image *img = _mmf_read(_ch, _width, _height, _format, buff, buff_size, read_block_ms);
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

    static image::Format _get_raw_format_with_size(int w, int h, int total_size, BAYER_FORMAT_E bayer_format) {
        image::Format format = image::FMT_INVALID;
        int size = w * h;
        if (total_size == size * 0.75) {
            switch (bayer_format) {
            case BAYER_FORMAT_BG:
                format = image::FMT_BGGR6;
                break;
            case BAYER_FORMAT_GB:
                format = image::FMT_GBRG6;
                break;
            case BAYER_FORMAT_GR:
                format = image::FMT_GRBG6;
                break;
            case BAYER_FORMAT_RG:
                format = image::FMT_RGGB6;
                break;
            default:
                return image::FMT_INVALID;
            }
        } else if (total_size == size * 1) {
            switch (bayer_format) {
            case BAYER_FORMAT_BG:
                format = image::FMT_BGGR8;
                break;
            case BAYER_FORMAT_GB:
                format = image::FMT_GBRG8;
                break;
            case BAYER_FORMAT_GR:
                format = image::FMT_GRBG8;
                break;
            case BAYER_FORMAT_RG:
                format = image::FMT_RGGB8;
                break;
            default:
                return image::FMT_INVALID;
            }
        } else if (total_size == size * 1.25) {
            switch (bayer_format) {
            case BAYER_FORMAT_BG:
                format = image::FMT_BGGR10;
                break;
            case BAYER_FORMAT_GB:
                format = image::FMT_GBRG10;
                break;
            case BAYER_FORMAT_GR:
                format = image::FMT_GRBG10;
                break;
            case BAYER_FORMAT_RG:
                format = image::FMT_RGGB10;
                break;
            default:
                return image::FMT_INVALID;
            }
        } else if (total_size == size * 1.5) {
            switch (bayer_format) {
            case BAYER_FORMAT_BG:
                format = image::FMT_BGGR12;
                break;
            case BAYER_FORMAT_GB:
                format = image::FMT_GBRG12;
                break;
            case BAYER_FORMAT_GR:
                format = image::FMT_GRBG12;
                break;
            case BAYER_FORMAT_RG:
                format = image::FMT_RGGB12;
                break;
            default:
                return image::FMT_INVALID;
            }
        } else {
            return image::FMT_INVALID;
        }

        return format;
    }

    image::Image *Camera::read_raw() {
        if (!this->is_opened()) {
            err::Err e = open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "open camera failed");
        }

        camera_priv_t *priv = (camera_priv_t *)this->_param;
        if (!priv->raw) {
            err::check_raise(err::ERR_NOT_READY, "you need to enable the raw parameter when constructing the Camera object.");
        }

        VI_DUMP_ATTR_S attr;
        memset(&attr, 0, sizeof(VI_DUMP_ATTR_S));
        CVI_VI_GetPipeDumpAttr(0, &attr);
        // log::info("Get enable:%d dumptype:%d", attr.bEnable, attr.enDumpType);
        if (attr.bEnable != 1 || attr.enDumpType != VI_DUMP_TYPE_RAW) {
            memset(&attr, 0, sizeof(VI_DUMP_ATTR_S));
            attr.bEnable = 1;
            attr.enDumpType = VI_DUMP_TYPE_RAW;
            CVI_VI_SetPipeDumpAttr(0, &attr);

            memset(&attr, 0, sizeof(VI_DUMP_ATTR_S));
            CVI_VI_GetPipeDumpAttr(0, &attr);
            err::check_bool_raise(attr.bEnable == 1, "Set dump enable failed");
            err::check_bool_raise(attr.enDumpType == VI_DUMP_TYPE_RAW, "Set dump type failed");
        }
        image::Image *img = NULL;
        image::Format format = image::FMT_INVALID;
        VIDEO_FRAME_INFO_S stVideoFrame;
        memset(&stVideoFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
        if (0 == CVI_VI_GetPipeFrame(0, &stVideoFrame, 2000)) {
            stVideoFrame.stVFrame.pu8VirAddr[0] = (CVI_U8 *)CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0],
                                                                        stVideoFrame.stVFrame.u32Length[0]);
            log::info("bayer format:%d pixel format:%d length:%d width:%d height:%d\r\n", stVideoFrame.stVFrame.enBayerFormat, stVideoFrame.stVFrame.enPixelFormat, stVideoFrame.stVFrame.u32Length[0], stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height);
			CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[0],
								stVideoFrame.stVFrame.pu8VirAddr[0],
								stVideoFrame.stVFrame.u32Length[0]);
            format = _get_raw_format_with_size(stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height, stVideoFrame.stVFrame.u32Length[0], stVideoFrame.stVFrame.enBayerFormat);
            if (format == image::FMT_INVALID) {
                log::error("camera read: unsupport pixel/bayer format");
                CVI_VI_ReleasePipeFrame(0, &stVideoFrame);
                return NULL;
            }
            img = new image::Image(stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height, format);
            if (!img) {
                log::error("camera read: new image failed");
                CVI_VI_ReleasePipeFrame(0, &stVideoFrame);
                return NULL;
            }

            memcpy(img->data(), (const void *)stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
            CVI_SYS_Munmap((void *)stVideoFrame.stVFrame.pu8VirAddr[0], stVideoFrame.stVFrame.u32Length[0]);
            CVI_VI_ReleasePipeFrame(0, &stVideoFrame);
        }
        return img;
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
        if (0 != mmf_del_vi_channel(this->_ch)) {
            log::error("mmf del vi channel failed");
        }

        _width = width;
        _height = height;
        if (0 != mmf_add_vi_channel_v2(_ch, _width, _height, mmf_invert_format_to_mmf(_format_impl), _fps, 2, -1, -1, 2, 3)) {
            err::check_raise(err::ERR_RUNTIME, "mmf add vi channel failed");
        }
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
        double new_fps = fps;
        camera_priv_t *priv = (camera_priv_t *)this->_param;
        switch (priv->sns_type) {
        case GCORE_GC4653_MIPI_720P_60FPS_10BIT:
            new_fps = fps / 2;
        break;
        case OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT:
        case OV_OS04A10_MIPI_4M_720P90_12BIT:
        {
            int exp_time_ms = (int)(1000.0 / fps);
            _config_extern_register_of_os04a10(exp_time_ms);
        }
        break;
        case SMS_SC035GS_MIPI_480P_120FPS_12BIT:
            err::check_raise(err::ERR_NOT_IMPL, "SC035GS set_fps not support");
        break;
        default:break;
        }
        ISP_PUB_ATTR_S stPubAttr;
        memset(&stPubAttr, 0, sizeof(stPubAttr));
        err::check_bool_raise(0 == CVI_ISP_GetPubAttr(0, &stPubAttr), "CVI_ISP_GetPubAttr failed");
        stPubAttr.f32FrameRate = new_fps;
        err::check_bool_raise(0 == CVI_ISP_SetPubAttr(0, &stPubAttr), "CVI_ISP_SetPubAttr failed");
        _fps = fps;
        return err::ERR_NONE;
    }

    int Camera::exposure(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }
        camera_priv_t *priv = (camera_priv_t *)this->_param;
        uint32_t out;
        if (value == -1) {
            mmf_get_exptime(_ch, &out);
        } else {
            switch (priv->sns_type) {
            case OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT:
            case OV_OS04A10_MIPI_4M_720P90_12BIT:
                mmf_set_exptime(_ch, value);
                _config_extern_register_of_os04a10(value / 1000);
            break;
            default:
                mmf_set_exptime(_ch, value);
            break;
            }
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
            mmf_get_iso_num(_ch, &out);
            out = (double)out * 1024 / 100;
        } else {
            int iso_num = value * 100 / 1024;
            mmf_set_iso_num(_ch, iso_num);
            out = value;
        }
        return out;
    }

    int Camera::hmirror(int value) {
        bool out;
        if (value == -1) {
            mmf_get_vi_hmirror(_ch, &out);
        } else {
            value = _invert_mirror ? !value : value;
            if (this->is_opened()) {
                VPSS_CHN_ATTR_S chn_attr = {0};
                int s32Ret = CVI_VPSS_GetChnAttr(0, _ch, &chn_attr);
                if (s32Ret != CVI_SUCCESS) {
                    SAMPLE_PRT("CVI_VPSS_GetChnAttr failed with %#x\n", s32Ret);
                    return CVI_FAILURE;
                }
                chn_attr.bMirror = !value;
                s32Ret = CVI_VPSS_SetChnAttr(0, _ch, &chn_attr);
                if (s32Ret != CVI_SUCCESS) {
                    SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                    return CVI_FAILURE;
                }
            }

            mmf_set_vi_hmirror(_ch, value);
            out = value;
        }

        return out;
    }

    int Camera::vflip(int value) {
        bool out;
        if (value == -1) {
            mmf_get_vi_vflip(_ch, &out);
        } else {
            value = _invert_flip ? !value : value;
            if (this->is_opened()) {
                VPSS_CHN_ATTR_S chn_attr = {0};
                int s32Ret = CVI_VPSS_GetChnAttr(0, _ch, &chn_attr);
                if (s32Ret != CVI_SUCCESS) {
                    SAMPLE_PRT("CVI_VPSS_GetChnAttr failed with %#x\n", s32Ret);
                    return CVI_FAILURE;
                }
                chn_attr.bFlip = !value;
                s32Ret = CVI_VPSS_SetChnAttr(0, _ch, &chn_attr);
                if (s32Ret != CVI_SUCCESS) {
                    SAMPLE_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                    return CVI_FAILURE;
                }
            }
            mmf_set_vi_vflip(_ch, value);
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

    static int _mmf_set_exp_mode(int ch, int mode)
    {
        CVI_U32 ret;
        ISP_EXPOSURE_ATTR_S stExpAttr;

        memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));

        ret = CVI_ISP_GetExposureAttr(ch, &stExpAttr);
        if (ret != 0) {
            printf("CVI_ISP_GetExposureAttr failed, ret: %#x.\r\n", ret);
            return -1;
        }

        if (stExpAttr.enOpType == mode) {
            return 0;
        }

        stExpAttr.u8DebugMode = 0;
        if (mode == 0) {
            stExpAttr.bByPass = 0;
            stExpAttr.enOpType = OP_TYPE_AUTO;
            stExpAttr.stManual.enExpTimeOpType = OP_TYPE_AUTO;
            stExpAttr.stManual.enISONumOpType = OP_TYPE_AUTO;
            stExpAttr.stManual.enAGainOpType = OP_TYPE_AUTO;
            stExpAttr.stManual.enDGainOpType = OP_TYPE_AUTO;
            stExpAttr.stManual.enISPDGainOpType = OP_TYPE_AUTO;
        } else if (mode == 1) {
            stExpAttr.bByPass = 0;
            stExpAttr.enOpType = OP_TYPE_MANUAL;
            stExpAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
            stExpAttr.stManual.enISONumOpType = OP_TYPE_MANUAL;
            stExpAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
            stExpAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
            stExpAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
            stExpAttr.stManual.enGainType = AE_TYPE_ISO;
        }

        ret = CVI_ISP_SetExposureAttr(ch, &stExpAttr);
        if (ret != 0) {
            printf("CVI_ISP_SetExposureAttr failed, ret: %#x.\r\n", ret);
            return -1;
        }

        return 0;
    }

    int Camera::exp_mode(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }
        camera_priv_t *priv = (camera_priv_t *)this->_param;
        uint32_t out;
        if (value == -1) {
            out = mmf_get_exp_mode(_ch);
        } else {
            switch (priv->sns_type) {
            case OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT:
            case OV_OS04A10_MIPI_4M_720P90_12BIT:
                if (value == 0) {
                    _config_extern_register_of_os04a10(0);  // revert exposure time register of os04a10
                }
            break;
            default:
            break;
            }
            _mmf_set_exp_mode(_ch, value);
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

    std::vector<int> Camera::get_sensor_size() {
        camera_priv_t *priv = (camera_priv_t *)_param;
        PIC_SIZE_E enPicSize;
        SIZE_S stSize;
        SAMPLE_COMM_VI_GetSizeBySensor(priv->sns_type, &enPicSize);
        SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

        return {(int)stSize.u32Width, (int)stSize.u32Height};
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

