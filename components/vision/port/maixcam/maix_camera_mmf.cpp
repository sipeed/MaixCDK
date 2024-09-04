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
        _last_read_us = time::ticks_us();
        _is_opened = false;


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
        // TODO: supports dual sensor
        int res = CVI_MIPI_SetSensorReset(0, 0);
		if (res != CVI_SUCCESS) {
			log::error("sensor 0 unreset failed!\n");
			return NULL;
		}

        static char name[30];
        peripheral::i2c::I2C i2c_obj(4, peripheral::i2c::Mode::MASTER);
        std::vector<int> addr_list = i2c_obj.scan();
        for (size_t i = 0; i < addr_list.size(); i++) {
            log::info("i2c4 addr: 0x%02x", addr_list[i]);
            switch (addr_list[i]) {
                case 0x29:
                    log::info("found gcore_gc4653, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "gcore_gc4653");
                    return name;
                case 0x30:
                    log::info("found sms_sc035gs, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "sms_sc035gs");
                    return name;
                case 0x2b:
                    log::info("found lt6911, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "lt6911");
                    return name;
                case 0x36:
                    log::info("found ov_os04a10, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "ov_os04a10");
                    return name;
                case 0x48:// fall through
                case 0x3c:
                    log::info("found ov_ov2685, addr %#x", addr_list[i]);
                    snprintf(name, sizeof(name), "ov_ov2685");
                    return name;
                default: break;
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

    static int _mmf_vi_init(int width, int height, int fps)
    {
        SIZE_S stSize;
        PIC_SIZE_E enPicSize;
        SAMPLE_INI_CFG_S	stIniCfg;
        SAMPLE_VI_CONFIG_S	stViConfig;
        PIXEL_FORMAT_E vi_format;
        PIXEL_FORMAT_E vi_vpss_format;

        // config vi param
        char *sensor_name = getenv(MMF_SENSOR_NAME);
        err::check_null_raise(sensor_name, "sensor name not found!");
        err::check_bool_raise(!SAMPLE_COMM_VI_ParseIni(&stIniCfg), "sensor cfg parse complete!");
        err::check_bool_raise(stIniCfg.devNum != 0, "Not device found! devnum = 0");

        if (!strcmp(sensor_name, "sms_sc035gs")) {

            stIniCfg.enSnsType[0] = SMS_SC035GS_MIPI_480P_120FPS_12BIT;
            stIniCfg.as8PNSwap[0][0] = 0;
            stIniCfg.as8PNSwap[0][1] = 0;
            stIniCfg.as8PNSwap[0][2] = 0;
            stIniCfg.as8PNSwap[0][3] = 0;
            stIniCfg.as8PNSwap[0][4] = 0;
            vi_format = PIXEL_FORMAT_NV21;
            vi_vpss_format = PIXEL_FORMAT_YUV_400;
        } else if (!strcmp(sensor_name, "ov_ov2685")) {
            stIniCfg.enSnsType[0] = GCORE_OV2685_MIPI_1600x1200_30FPS_10BIT;
            stIniCfg.as8PNSwap[0][0] = 1;
            stIniCfg.as8PNSwap[0][1] = 1;
            stIniCfg.as8PNSwap[0][2] = 1;
            stIniCfg.as8PNSwap[0][3] = 0;
            stIniCfg.as8PNSwap[0][4] = 0;
            vi_format = PIXEL_FORMAT_NV21;
            vi_vpss_format = PIXEL_FORMAT_NV21;
            err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.ov2685"), "set config path failed!");
        } else if (!strcmp(sensor_name, "lt6911")) {
            stIniCfg.enSnsType[0] = LONTIUM_LT6911_2M_60FPS_8BIT;
            stIniCfg.as16LaneId[0][0] = 2;
            stIniCfg.as16LaneId[0][1] = 4;
            stIniCfg.as16LaneId[0][2] = 3;
            stIniCfg.as16LaneId[0][3] = 1;
            stIniCfg.as16LaneId[0][4] = 0;
            stIniCfg.as8PNSwap[0][0] = 0;
            stIniCfg.as8PNSwap[0][1] = 0;
            stIniCfg.as8PNSwap[0][2] = 0;
            stIniCfg.as8PNSwap[0][3] = 0;
            stIniCfg.as8PNSwap[0][4] = 0;
            vi_format = PIXEL_FORMAT_UYVY;
            vi_vpss_format = PIXEL_FORMAT_UYVY;
        } else if (!strcmp(sensor_name, "ov_os04a10")) {
            stIniCfg.enSnsType[0] = OV_OS04A10_MIPI_4M_1440P_30FPS_12BIT;
            stIniCfg.as16LaneId[0][0] = 2;
            stIniCfg.as16LaneId[0][1] = 1;
            stIniCfg.as16LaneId[0][2] = 3;
            stIniCfg.as16LaneId[0][3] = 0;
            stIniCfg.as16LaneId[0][4] = 4;
            stIniCfg.as8PNSwap[0][0] = 0;
            stIniCfg.as8PNSwap[0][1] = 0;
            stIniCfg.as8PNSwap[0][2] = 0;
            stIniCfg.as8PNSwap[0][3] = 0;
            stIniCfg.as8PNSwap[0][4] = 0;
            vi_format = PIXEL_FORMAT_NV21;
            vi_vpss_format = PIXEL_FORMAT_NV21;
            err::check_bool_raise(!CVI_BIN_SetBinName(WDR_MODE_NONE, "/mnt/cfg/param/cvi_sdr_bin.os04a10"), "set config path failed!");
        } else { // default is gcore_gc4653
            if (width <= 1280 && height <= 720 && fps > 30) {
                stIniCfg.enSnsType[0] = GCORE_GC4653_MIPI_720P_60FPS_10BIT;
            } else {
                stIniCfg.enSnsType[0] = GCORE_GC4653_MIPI_4M_30FPS_10BIT;
            }
            stIniCfg.as8PNSwap[0][0] = 0;
            stIniCfg.as8PNSwap[0][1] = 0;
            stIniCfg.as8PNSwap[0][2] = 0;
            stIniCfg.as8PNSwap[0][3] = 0;
            stIniCfg.as8PNSwap[0][4] = 0;
            vi_format = PIXEL_FORMAT_NV21;
            vi_vpss_format = PIXEL_FORMAT_NV21;
        }

        err::check_bool_raise(!mmf_init_v2(false), "mmf init failed");
        err::check_bool_raise(!SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig), "IniToViCfg failed!");
        err::check_bool_raise(!SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize), "GetSizeBySensor failed!");
        err::check_bool_raise(!SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize), "GetPicSize failed!");
        if (0 !=  mmf_vi_init_v2(stSize.u32Width, stSize.u32Height, vi_format, vi_vpss_format, fps, 3, &stViConfig)) {
            mmf_deinit_v2(false);
            err::check_raise(err::ERR_RUNTIME, "mmf vi init failed");
        }
        return  0;
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

        // check if we need update camera params
        if (this->is_opened()) {
            if (width == width_tmp && height == height_tmp && format == format_tmp && fps == fps_tmp && buff_num == buff_num_tmp) {
                return err::ERR_NONE;
            }
            this->close();
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
        err::check_bool_raise(!_mmf_vi_init(_width, _height, _fps), "mmf vi init failed");
        err::check_bool_raise((_ch = mmf_get_vi_unused_channel()) >= 0, "mmf get vi channel failed");
        if (0 != mmf_add_vi_channel_v2(_ch, _width, _height, mmf_invert_format_to_mmf(_format_impl), _fps, 2, -1, -1, 2, 3)) {
            mmf_vi_deinit();
            mmf_deinit_v2(false);
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

        // mmf_vi_deinit();

        mmf_deinit_v2(false);
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

