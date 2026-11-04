/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_camera.hpp"
#include "maix_pipeline.hpp"
#include "maix_basic.hpp"
#include "maix_i2c.hpp"
#include <dirent.h>
#include <dlfcn.h>
#include <chrono>
#include "ax_middleware.hpp"

using namespace maix;
using namespace maix::middleware::maixcam2;
#define ALIGN_UP_2(value) ((value + 0x1) & (~0x1))
#define ALIGN_UP_16(value) ((value + 0xF) & (~0xF))

namespace maix::camera
{
    static bool set_regs_flag = false;
    static std::string __device_name = "";
    static std::vector<int> __sensor_size;
    static bool __invert_flip = false;
    static bool __invert_mirror = false;
    constexpr int kOs04a10BinnedMaxWidth = 1344;
    constexpr int kOs04a10BinnedMaxHeight = 760;
    constexpr int kOs04a10HighFpsCropMaxWidth = 640;
    constexpr int kOs04a10HighFpsCropMaxHeight = 360;

    static AX_S32 __set_os04a10_sensor_crop(ISP_PIPE_ID pipe, AX_S32 x, AX_S32 y,
                                             AX_S32 width, AX_S32 height, AX_F32 fps,
                                             bool require_api = true)
    {
        using set_crop_fn = AX_S32 (*)(ISP_PIPE_ID, AX_U32, AX_U32, AX_U32, AX_U32, AX_F32);
        static void *sensor_handle = nullptr;
        set_crop_fn set_crop = reinterpret_cast<set_crop_fn>(dlsym(RTLD_DEFAULT, "os04a10_set_crop"));
        if (!set_crop) {
            if (!sensor_handle) {
                sensor_handle = dlopen("/opt/lib/libsns_os04a10.so", RTLD_LAZY | RTLD_GLOBAL);
            }
            if (sensor_handle) {
                set_crop = reinterpret_cast<set_crop_fn>(dlsym(sensor_handle, "os04a10_set_crop"));
            }
        }
        if (!set_crop) {
            if (require_api) {
                log::error("OS04A10 sensor crop API is unavailable: %s", dlerror());
                return -1;
            }
            /* Old system sensor libraries do not implement the optional crop
             * API. They have no process-global crop state to clear. */
            return AX_SUCCESS;
        }
        return set_crop(pipe, (AX_U32)x, (AX_U32)y, (AX_U32)width, (AX_U32)height, fps);
    }

    static int __os04a10_max_fps_for_roi(int width, int height)
    {
        return width <= kOs04a10HighFpsCropMaxWidth && height <= kOs04a10HighFpsCropMaxHeight ? 360 :
               width <= kOs04a10BinnedMaxWidth && height <= kOs04a10BinnedMaxHeight ? 180 : 60;
    }

    /* Check timing before tearing down an active OS04A10 pipeline.  The
     * sensor mode is selected at stream-on, so changing FPS requires a
     * close/open; rejecting an impossible request here keeps the old stream
     * running. */
    static err::Err __validate_os04a10_fps(const std::vector<int> &windowing, double fps)
    {
        if (fps <= 0) {
            return err::ERR_NONE; // selector-auto is the documented 60 fps mode
        }
        const int requested_fps = static_cast<int>(fps);
        if (requested_fps <= 0 || static_cast<double>(requested_fps) != fps) {
            log::error("OS04A10 FPS must be a positive integer (or 0 for automatic 60 fps), requested %.3f", fps);
            return err::ERR_ARGS;
        }

        const bool has_roi = windowing.size() == 4;
        const int max_fps = has_roi ? __os04a10_max_fps_for_roi(windowing[2], windowing[3]) : 180;
        if (requested_fps > max_fps) {
            if (!has_roi) {
                log::error("OS04A10 full-FOV 1344x760 binned mode supports up to 180 fps; "
                           "set_windowing({x, y, 640, 360}) before requesting %d fps", requested_fps);
            } else {
                log::error("OS04A10 %dx%d sensor crop supports up to %d fps, requested %d",
                           windowing[2], windowing[3], max_fps, requested_fps);
            }
            return err::ERR_NOT_IMPL;
        }
        return err::ERR_NONE;
    }

    /* A bad sensor timing/crop must not turn a command handler into an
     * unbounded wait.  This helper is intentionally used only by dynamic
     * OS04A10 restart paths; ordinary camera reads keep their existing
     * blocking semantics. */
    static bool __warmup_os04a10(Camera &camera, int frames)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        for (int i = 0; i < frames; ++i) {
            const auto now = std::chrono::steady_clock::now();
            if (now >= deadline) {
                log::error("OS04A10 warmup exceeded its 3 second deadline after %d/%d frames", i, frames);
                return false;
            }
            const int remaining_ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
            if (remaining_ms <= 0) {
                log::error("OS04A10 warmup exceeded its 3 second deadline after %d/%d frames", i, frames);
                return false;
            }
            const int timeout_ms = i == 0 ? (remaining_ms < 1000 ? remaining_ms : 1000) :
                                            (remaining_ms < 100 ? remaining_ms : 100);
            image::Image *img = nullptr;
            try {
                img = camera.read(true, timeout_ms);
            } catch (const err::Exception &) {
                log::error("OS04A10 did not produce warmup frame %d/%d", i + 1, frames);
                return false;
            }
            if (!img) {
                log::error("OS04A10 warmup frame %d/%d timed out", i + 1, frames);
                return false;
            }
            delete img;
        }
        return true;
    }

    static AX_S32 __config_os04d10_360p120_ae(AX_U8 pipe)
    {
        AX_ISP_IQ_AE_PARAM_T param = {};
        AX_S32 ret = AX_ISP_IQ_GetAeParam(pipe, &param);
        if (ret != AX_SUCCESS) {
            return ret;
        }

        param.nEnable = false;
        param.tExpManual.nShutter = 4000;
        param.tExpManual.nShortShutter = 250;
        param.tExpManual.nVsShutter = 4000;
        ret = AX_ISP_IQ_SetAeParam(pipe, &param);
        if (ret != AX_SUCCESS) {
            return ret;
        }

        ret = AX_ISP_IQ_GetAeParam(pipe, &param);
        if (ret != AX_SUCCESS) {
            return ret;
        }
        param.nEnable = false;
        param.tExpManual.nAGain = 1024;
        ret = AX_ISP_IQ_SetAeParam(pipe, &param);
        if (ret != AX_SUCCESS) {
            return ret;
        }

        ret = AX_ISP_IQ_GetAeParam(pipe, &param);
        if (ret != AX_SUCCESS) {
            return ret;
        }
        AX_ISP_IQ_AE_ALG_CONFIG_T &config = param.tAeAlgAuto;
        config.nCompensationMode = 0;
        config.nMaxIspGain = 1024;
        config.nMinIspGain = 1024;
        config.nMaxUserDgain = 1024;
        config.nMinUserDgain = 1024;
        config.nMaxUserTotalAgain = 15872;
        config.nMinUserTotalAgain = 1024;
        config.nMaxUserSysGain = 16384;
        config.nMinUserSysGain = 1024;
        config.nMaxShutter = 8000;
        config.nMinShutter = 361;
        config.nStrategyMode = 0;
        config.nAeRouteMode = 0;
        config.tAntiFlickerParam.nAntiFlickerMode = 0;
        config.tSlowShutterParam.nFrameRateMode = 0;
        param.nEnable = true;
        return AX_ISP_IQ_SetAeParam(pipe, &param);
    }

    std::vector<std::string> list_devices()
    {
        log::warn("This device is not driven using device files!");
        return std::vector<std::string>();
    }

    static void __get_global_info() {
        peripheral::i2c::I2C i2c_obj(0, peripheral::i2c::Mode::MASTER);

        std::vector<int> addr_list = i2c_obj.scan();
        for (size_t i = 0; i < addr_list.size(); i++) {
            switch (addr_list[i]) {
                case 0x29:  // gcore_gc4653
                    __device_name = "gcore_gc4653";
                    __sensor_size = {2560, 1440};
                    __invert_flip = false;
                    __invert_mirror = false;
                    break;
                case 0x30:  // sc850sl
                    __device_name = "smartsens_sc850sl";
                    __sensor_size = {3840, 2160};
                    __invert_flip = false;
                    __invert_mirror = false;
                    break;
                case 0x2b:  // lt6911
                    __device_name = "lt6911";
                    __sensor_size = {1920, 1080};
                    __invert_flip = false;
                    __invert_mirror = false;
                    break;
                case 0x36:  // ov_os04a10
                    __device_name = "ov_os04a10";
                    __sensor_size = {2688, 1520};
                    __invert_flip = true;
                    __invert_mirror = false;
                    break;
                case 0x37:  // gcore_gc02m1
                    __device_name = "gcore_gc02m1";
                    __sensor_size = {1600, 1200};
                    __invert_flip = false;
                    __invert_mirror = false;
                    break;
                case 0x48:// fall through
                case 0x3c:  // os04d10 ??ov_ov2685
                    __device_name = "ov_os04d10";
                    __sensor_size = {2560, 1440};
                    __invert_flip = true;
                    __invert_mirror = true;
                    break;
                default:
                    __device_name = "unknown";
                    __sensor_size = {0, 0};
                    break;
            }
        }
    }

    std::string get_device_name()
    {
        if (__device_name.size() == 0) {
            __get_global_info();
        }
        return __device_name;
    }

    void set_regs_enable(bool enable) {
        log::warn("This operation is not supported!");
    }


    static void __generate_colorbar(image::Image &img)
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

    bool Camera::_check_format(image::Format format) {
        if (format == image::FMT_RGB888 || format == image::FMT_BGR888
        || format == image::FMT_RGBA8888 || format == image::FMT_BGRA8888
        || format == image::FMT_YVU420SP || format == image::FMT_GRAYSCALE) {
            return true;
        } else {
            return false;
        }
    }

    static bool __check_board_config_path()
    {
        if (fs::exists("/boot/board")) {
            return true;
        }
        return false;
    }

    static void __get_cam_flip_mirror(int &new_flip, int &new_mirror) {
        bool flip = 0, mirror = 0;
        bool flip_is_found = false, mirror_is_found = false;

        if (__check_board_config_path()) {
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
            // log::info("cam flip=%s, cam mirror=%s", flip_str.c_str(), mirror_str.c_str());
            if (flip_is_found && !flip_str.empty()) {
                flip = atoi(flip_str.c_str());
            } else {
                if (board_id == "maixcam2") {
                    flip = true;
                } else {
                    flip = false;
                }
            }

            if (mirror_is_found && !mirror_str.empty()) {
                mirror = atoi(mirror_str.c_str());
            } else {
                std::string board_id = sys::device_id();
                if (board_id == "maixcam2") {
                    mirror = true;
                } else {
                    mirror = false;
                }
            }
        }

        new_flip = flip;
        new_mirror = mirror;
    }

    // static bool _get_board_config_path(char *path, int path_size)
    // {
    //     if (fs::exists("/boot/board")) {
    //         snprintf(path, path_size, "/boot/board");
    //         return true;
    //     }
    //     return false;
    // }

    // static int _get_mclk_id(void) {
    //     char path[64];
    //     int mclk_id = 0;

    //     err::check_bool_raise(_get_board_config_path(path, sizeof(path)), "Can't find board config file");

    //     std::string mclk_id_str;
    //     auto device_configs = sys::device_configs();
    //     auto it = device_configs.find("cam_mclk");
    //     if (it != device_configs.end()) {
    //         mclk_id_str = it->second;
    //     }
    //     if (!mclk_id_str.empty()) {
    //         mclk_id = atoi(mclk_id_str.c_str());
    //     } else {
    //         std::string board_id = sys::device_id();
    //         if (board_id == "maixcam_pro") {
    //             mclk_id = 0;
    //         } else {
    //             mclk_id = 1;
    //         }
    //     }
    //     return mclk_id;
    // }

    // static std::vector<int> _get_lane_id_from_board_file() {
    //     std::vector<int> lane_id;
    //     auto device_configs = sys::device_configs();
    //     auto it = device_configs.find("lane_id");
    //     if (it != device_configs.end()) {
    //         auto lane_id_str = it->second;
    //         std::string item;
    //         std::stringstream ss(lane_id_str);
    //         while (std::getline(ss, item, ',')) {
    //             lane_id.push_back(std::stoi(item));
    //         }
    //     }

    //     return lane_id;
    // }

    // static std::vector<int> _get_pn_swap_from_board_file() {
    //     std::vector<int> pn_swap;
    //     auto device_configs = sys::device_configs();
    //     auto it = device_configs.find("pn_swap");
    //     if (it != device_configs.end()) {
    //         auto pn_swap_str = it->second;
    //         std::string item;
    //         std::stringstream ss(pn_swap_str);
    //         while (std::getline(ss, item, ',')) {
    //             pn_swap.push_back(std::stoi(item));
    //         }
    //     }

    //     return pn_swap;
    // }

    // static std::vector<bool> _get_cam_flip_mirror(void) {
    //     char path[64];
    //     bool flip = 0, mirror = 0;
    //     bool flip_is_found = false, mirror_is_found = false;

    //     err::check_bool_raise(_get_board_config_path(path, sizeof(path)), "Can't find board config file");

    //     std::string flip_str;
    //     std::string mirror_str;
    //     auto device_configs = sys::device_configs();
    //     auto it = device_configs.find("cam_flip");
    //     if (it != device_configs.end()) {
    //         flip_str = it->second;
    //         flip_is_found = true;
    //     }
    //     auto it2 = device_configs.find("cam_mirror");
    //     if (it2 != device_configs.end()) {
    //         mirror_str = it2->second;
    //         mirror_is_found = true;
    //     }

    //     std::string board_id = sys::device_id();
    //     // log::info("cam flip=%s, cam mirror=%s", flip_str, mirror_str);
    //     if (flip_is_found && !flip_str.empty()) {
    //         flip = atoi(flip_str.c_str());
    //     } else {
    //         if (board_id == "maixcam_pro") {
    //             flip = true;
    //         } else {
    //             flip = false;
    //         }
    //     }

    //     if (mirror_is_found && !mirror_str.empty()) {
    //         mirror = atoi(mirror_str.c_str());
    //     } else {
    //         std::string board_id = sys::device_id();
    //         if (board_id == "maixcam_pro") {
    //             mirror = true;
    //         } else {
    //             mirror = false;
    //         }
    //     }

    //     return {flip, mirror};
    // }

    // static image::Format _get_raw_format_with_size(int w, int h, int total_size, AX_IMG_FORMAT_E bayer_format) {
    //     image::Format format = image::FMT_INVALID;
    //     // int size = w * h;
    //     // if (total_size == size * 0.75) {
    //     //     switch (bayer_format) {
    //     //     case BAYER_FORMAT_BG:
    //     //         format = image::FMT_BGGR6;
    //     //         break;
    //     //     case BAYER_FORMAT_GB:
    //     //         format = image::FMT_GBRG6;
    //     //         break;
    //     //     case BAYER_FORMAT_GR:
    //     //         format = image::FMT_GRBG6;
    //     //         break;
    //     //     case BAYER_FORMAT_RG:
    //     //         format = image::FMT_RGGB6;
    //     //         break;
    //     //     default:
    //     //         return image::FMT_INVALID;
    //     //     }
    //     // } else if (total_size == size * 1) {
    //     //     switch (bayer_format) {
    //     //     case BAYER_FORMAT_BG:
    //     //         format = image::FMT_BGGR8;
    //     //         break;
    //     //     case BAYER_FORMAT_GB:
    //     //         format = image::FMT_GBRG8;
    //     //         break;
    //     //     case BAYER_FORMAT_GR:
    //     //         format = image::FMT_GRBG8;
    //     //         break;
    //     //     case BAYER_FORMAT_RG:
    //     //         format = image::FMT_RGGB8;
    //     //         break;
    //     //     default:
    //     //         return image::FMT_INVALID;
    //     //     }
    //     // } else if (total_size == size * 1.25) {
    //     //     switch (bayer_format) {
    //     //     case BAYER_FORMAT_BG:
    //     //         format = image::FMT_BGGR10;
    //     //         break;
    //     //     case BAYER_FORMAT_GB:
    //     //         format = image::FMT_GBRG10;
    //     //         break;
    //     //     case BAYER_FORMAT_GR:
    //     //         format = image::FMT_GRBG10;
    //     //         break;
    //     //     case BAYER_FORMAT_RG:
    //     //         format = image::FMT_RGGB10;
    //     //         break;
    //     //     default:
    //     //         return image::FMT_INVALID;
    //     //     }
    //     // } else if (total_size == size * 1.5) {
    //     //     switch (bayer_format) {
    //     //     case BAYER_FORMAT_BG:
    //     //         format = image::FMT_BGGR12;
    //     //         break;
    //     //     case BAYER_FORMAT_GB:
    //     //         format = image::FMT_GBRG12;
    //     //         break;
    //     //     case BAYER_FORMAT_GR:
    //     //         format = image::FMT_GRBG12;
    //     //         break;
    //     //     case BAYER_FORMAT_RG:
    //     //         format = image::FMT_RGGB12;
    //     //         break;
    //     //     default:
    //     //         return image::FMT_INVALID;
    //     //     }
    //     // } else {
    //     //     return image::FMT_INVALID;
    //     // }

    //     return format;
    // }

    std::vector<int> get_sensor_size()
    {
        if (__sensor_size.size() == 0) {
            __get_global_info();
        }
        return __sensor_size;
    }

    typedef struct {
        int i2c_addr;
        bool raw;
        bool flip;
        bool mirror;
        /* Keep the constructor's "no output size supplied" state separate
         * from its public 640x480 default.  OS04A10 mode selection must not
         * infer a small sensor crop merely because its public output happens
         * to be 640x480. */
        bool output_size_auto;
        bool fps_auto;
        double requested_fps;
        int exptime_max;    // unit:us
        int exptime_min;

        SYS *ax_sys;
        VI *ax_vi;

        struct {
            int id;
            int w;
            int h;
            image::Format fmt;
            int fps;
            int depth;
            int mirror;
            int vflip;
            int fit;
        } chn;
    } camera_priv_t;

    Camera::Camera(int width, int height, image::Format format, const char *device, double fps, int buff_num, bool open, bool raw)
    {
        err::Err e;
        err::check_bool_raise(_check_format(format), "Format not support");

        _width = (width == -1) ? 640 : width;
        _height = (height == -1) ? 480 : height;
        _format = format;
        _buff_num = buff_num > 3 ? buff_num : 3;
        _device =  device ? std::string(device) : "";
        _fps = fps;

        _show_colorbar = false;
        _open_set_regs = set_regs_flag;

        _is_opened = false;

        camera_priv_t *priv = (camera_priv_t *)malloc(sizeof(camera_priv_t));
        err::check_null_raise(priv, "camera_priv_t malloc error");
        memset(priv, 0, sizeof(camera_priv_t));
        priv->raw = raw;
        priv->output_size_auto = width == -1 && height == -1;
        priv->fps_auto = fps <= 0;
        priv->requested_fps = fps;
        _param = priv;

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
            ::free(_param);
        }
    }

    int Camera::get_ch_nums()
    {
        return AX_IVPS_MAX_OUTCHN_NUM;
    }

    int Camera::get_channel()
    {
        camera_priv_t *priv = (camera_priv_t *)_param;
        return priv->chn.id;
    }

    err::Err Camera::open(int width, int height, image::Format format, double fps, int buff_num)
    {
        if ((width != -1 && width != _width)
        || (height != -1 && height != _height)
        || (format != image::FMT_INVALID && format != _format)
        || (fps != -1 && fps != _fps)
        || (buff_num != -1 && buff_num != _buff_num)) {
            this->close();
        }
        if (this->is_opened()) {
            return err::ERR_NONE;
        }
        err::Err err = err::ERR_NONE;
        int width_tmp = (width == -1) ? _width : width;
        int height_tmp = (height == -1) ? _height : height;
        image::Format format_tmp = (format == image::FMT_INVALID) ? _format : format;
        camera_priv_t *priv = (camera_priv_t *)_param;
        if (fps != -1) {
            /* Both 0 and negative constructor FPS values mean selector-auto
             * (60 fps for OS04A10). Record an explicit open(…, 0) in the
             * same way; otherwise a previous explicit FPS leaks through a
             * later auto reopen. */
            priv->fps_auto = fps <= 0;
            priv->requested_fps = fps;
        }
        double fps_tmp = (fps == -1) ? (priv->fps_auto ? -1 : _fps) :
                         (priv->fps_auto ? -1 : fps);
        _fps = fps_tmp;
        int buff_num_tmp =( buff_num == -1) ? _buff_num : buff_num;

        // check resolution
        if (format == image::FMT_RGB888) {
            err::check_bool_raise(width_tmp < 1920 && height_tmp < 1080, "The resolution of an image in RGB888 format must not exceed 1920x1080");
        }

        // check format
        err::check_bool_raise(_check_format(format_tmp), "Format not support");

        // init sys
        SYS *ax_sys = new SYS(priv->raw);
        err::check_null_raise(ax_sys, "ax sys malloc error");
        err = ax_sys->init();
        if (err::ERR_NONE != err) {
            log::info("Init ax sys failed");
            return err;
        }

        auto &mod_param = AxModuleParam::getInstance();
        mod_param.lock(AX_MOD_VI);
        auto vi_param = (ax_vi_mod_t *)mod_param.get_param(AX_MOD_VI);
        auto ax_cam = vi_param->cams[0];
        auto p_ax_cam = &ax_cam;
        mod_param.unlock(AX_MOD_VI);
        priv->i2c_addr = p_ax_cam->nI2cAddr;

        // config vi param
        COMMON_SYS_ARGS_T tCommonArgs = {0};
        COMMON_SYS_ARGS_T tPrivArgs = {0};
        SAMPLE_VIN_PARAM_T tVinParam = {
            .eSysCase = SAMPLE_VIN_SINGLE_SC450AI,
            .eSysMode = COMMON_VIN_SENSOR,
            .eHdrMode = AX_SNS_LINEAR_MODE,
            .eLoadRawNode = LOAD_RAW_IFE,
            .bAiispEnable = AX_FALSE,
            .statDeltaPtsFrmNum = 0,
        };

        if (_windowing.size() == 4) {
            tVinParam.bSensorCrop = AX_TRUE;
            tVinParam.nSensorCropX = _windowing[0];
            tVinParam.nSensorCropY = _windowing[1];
            tVinParam.nSensorCropW = _windowing[2];
            tVinParam.nSensorCropH = _windowing[3];
        }


        // init vi
        VI *ax_vi = new VI();
        if (ax_vi == NULL) {
            delete ax_sys;
            err::check_raise(err::ERR_RUNTIME, "construct VI failed");
        }
        auto get_sensor_res = ax_vi->get_sensor_name();
        if (!get_sensor_res.first) {
            log::error("get sensor name failed");
            return err::ERR_RUNTIME;
        }

        if (get_sensor_res.second == "os04a10" && !tVinParam.bSensorCrop) {
            /* Application output and sensor input are intentionally separate.
             * The only no-ROI request that selects native readout is an
             * explicit 2688x1520 output.  The constructor's unspecified size
             * (which publicly defaults to 640x480), every smaller explicit
             * output, and larger scaled output all retain the full-FOV 2x2
             * binned 1344x760 sensor image.  In particular, a small public
             * output must never implicitly turn into a 640x360 sensor crop:
             * only set_windowing() opts into a smaller sensor ROI. */
            const bool explicit_native_output = !priv->output_size_auto &&
                                                _width == 2688 && _height == 1520;
            const bool require_binned_input = !explicit_native_output ||
                                              (!priv->fps_auto && priv->requested_fps > 60);
            if (require_binned_input) {
                if (!priv->fps_auto && priv->requested_fps > 180) {
                    log::error("OS04A10 full-FOV 1344x760 binned mode supports up to 180 fps; "
                               "for 240 fps, call set_windowing() with a small ROI (recommended: 640x360) before open");
                    delete ax_vi;
                    delete ax_sys;
                    return err::ERR_NOT_IMPL;
                }
                if (explicit_native_output) {
                    log::info("OS04A10 2688x1520 at %.0f fps uses the full-FOV 1344x760 binned sensor input",
                              priv->requested_fps);
                }
                tVinParam.bSensorCrop = AX_TRUE;
                tVinParam.nSensorCropX = 0;
                tVinParam.nSensorCropY = 4;
                tVinParam.nSensorCropW = kOs04a10BinnedMaxWidth;
                tVinParam.nSensorCropH = kOs04a10BinnedMaxHeight;
            }
        }

        int tmp_w = _width, tmp_h = _height, tmp_fps = _fps;
        const int crop_w = tVinParam.bSensorCrop ? tVinParam.nSensorCropW : -1;
        const int crop_h = tVinParam.bSensorCrop ? tVinParam.nSensorCropH : -1;
        tVinParam.eSysCase = ax_vi->get_vi_case((char *)get_sensor_res.second.c_str(), tmp_w, tmp_h, tmp_fps,
                                                crop_w, crop_h);
        if (tVinParam.eSysCase == SAMPLE_VIN_NONE) {
            delete ax_vi;
            delete ax_sys;
            return err::ERR_NOT_IMPL;
        }
        tVinParam.nSensorWidth = tmp_w;
        tVinParam.nSensorHeight = tmp_h;
        tVinParam.nSensorFps = tmp_fps;
        _fps = tmp_fps;
        if (get_sensor_res.second == "os04a10") {
            const AX_S32 crop_x = tVinParam.bSensorCrop ? tVinParam.nSensorCropX : 0;
            const AX_S32 crop_y = tVinParam.bSensorCrop ? tVinParam.nSensorCropY : 0;
            const AX_S32 crop_width = tVinParam.bSensorCrop ? tVinParam.nSensorCropW : 0;
            const AX_S32 crop_height = tVinParam.bSensorCrop ? tVinParam.nSensorCropH : 0;
            if (__set_os04a10_sensor_crop(0, crop_x, crop_y, crop_width, crop_height,
                                           (AX_F32)tVinParam.nSensorFps,
                                           tVinParam.bSensorCrop == AX_TRUE) != AX_SUCCESS) {
                delete ax_vi;
                delete ax_sys;
                return err::ERR_NOT_IMPL;
            }
        }
        tVinParam.bAiispEnable = app::get_sys_config_kv("npu", "ai_isp", "1") == "1" ? AX_TRUE : AX_FALSE;
        if (tVinParam.bSensorCrop) {
            /* The shipped AI-ISP model is calibrated for the native OS04A10
             * frame geometry and cannot safely process arbitrary sensor ROIs. */
            tVinParam.bAiispEnable = AX_FALSE;
        }
        ax_vi->config_sample_case(&tVinParam, &tCommonArgs, &tPrivArgs);
        err = ax_vi->init();
        if (err != err::ERR_NONE) {
            delete ax_vi;
            delete ax_sys;
            err::check_raise(err, "Init ax vi failed");
        }

        if (priv->raw) {
            AX_VIN_DUMP_ATTR_T stDumpAttr;
            stDumpAttr.bEnable = AX_TRUE;
            stDumpAttr.nDepth = 1;
            AX_S32 axRet = AX_VIN_SetPipeDumpAttr(0, AX_VIN_PIPE_DUMP_NODE_IFE, AX_VIN_DUMP_QUEUE_TYPE_DEV, &stDumpAttr);
            if (0 != axRet) {
                log::error("AX_VIN_SetPipeDumpAttr failed, ret=0x%x\n", axRet);
            }
        }

        auto ch = ax_vi->get_unused_channel();
        if (ch == -1) {
            delete ax_vi;
            delete ax_sys;
            err::check_raise(err::ERR_RUNTIME, "Get unused channel failed");
        }

        int fit = 2;
        err = ax_vi->add_channel(ch, ALIGN_UP_16(width_tmp), ALIGN_UP_2(height_tmp), get_ax_fmt_from_maix(format_tmp), (int)fps_tmp, buff_num_tmp, false, false, fit);
        if (err != err::ERR_NONE) {
            delete ax_vi;
            delete ax_sys;
            err::check_raise(err, "Add channel failed");
        }

        if (tVinParam.eSysCase == SAMPLE_VIN_SINGLE_SC850SL_1080P60) {
            auto axRet = AX_ISP_SetTopsLevel(ch, AX_ISP_AI_TOPS_LEVEL4);
            if (0 != axRet) {
                log::error("[%d] AX_ISP_SetTopsLevel failed, ret=0x%x.", ch, axRet);
            }
        }

        __get_global_info();
        __get_cam_flip_mirror(priv->chn.vflip, priv->chn.mirror);
        priv->chn.vflip = __invert_flip ? !priv->chn.vflip : priv->chn.vflip;
        priv->chn.mirror = __invert_mirror ? !priv->chn.mirror : priv->chn.mirror;
        priv->ax_sys = ax_sys;
        priv->ax_vi = ax_vi;
        priv->chn.id = ch;
        priv->chn.w = width_tmp;
        priv->chn.h = height_tmp;
        priv->chn.fmt = format_tmp;
        priv->chn.fit = fit;
        _ch = ch;
        _is_opened = true;

        if (width_tmp % 16 != 0) {
            log::warn("Width %d is not aligned to 16, will be set to %d", width_tmp, ALIGN_UP_16(width_tmp));
        }
        if (height_tmp % 2 != 0) {
            log::warn("Height %d is not aligned to 2, will be set to %d", width_tmp, ALIGN_UP_2(height_tmp));
        }

        auto camera_nt = app::get_sys_config_kv("camera", "nt", "0") == "1" ? AX_TRUE : AX_FALSE;
        if (camera_nt) {
            auto camera_nt_stream_port = atoi(app::get_sys_config_kv("camera", "nt_stream_port", "6000").c_str());
            auto camera_nt_ctrl_port = atoi(app::get_sys_config_kv("camera", "nt_ctrl_port", "8082").c_str());
            auto axRet = COMMON_NT_Init(camera_nt_stream_port, camera_nt_ctrl_port);
            if (axRet) {
                printf("COMMON_NT_Init fail, ret:0x%x\r\n", axRet);
            }
        }

        this->vflip(priv->chn.vflip);
        this->hmirror(priv->chn.mirror);
        if (tVinParam.eSysCase == SAMPLE_VIN_SINGLE_OS04D10_360P120) {
            AX_S32 ret = __config_os04d10_360p120_ae(0);
            if (ret != AX_SUCCESS) {
                log::error("Configure OS04D10 360p120 AE failed, ret=0x%x", ret);
            }
        }
        return err::ERR_NONE;
    }

    void Camera::close()
    {
        err::Err ret = err::ERR_NONE;
        camera_priv_t *priv = (camera_priv_t *)_param;
        if (this->is_closed())
            return;

#if AX_NT_ENABLE
        COMMON_NT_DeInit();
#endif
        ret = priv->ax_vi->del_channel(priv->chn.id);
        if (ret != err::ERR_NONE) {
            log::error("vi del_channel failed, ret:%d", ret);
        }
        ret = priv->ax_vi->deinit();
        if (ret != err::ERR_NONE) {
            log::error("vi deinit failed, ret:%d", ret);
        }
        delete priv->ax_vi;
        delete priv->ax_sys;

        _is_opened = false;
    }

    camera::Camera *Camera::add_channel(int width, int height, image::Format format, double fps, int buff_num, bool open)
    {
        err::check_bool_raise(_check_format(format), "Format not support");

        int width_tmp = (width <= 0) ? _width : width;
        int height_tmp = (height <= 0) ? _height : height;
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
        auto *priv = (camera_priv_t *)_param;
        auto vi = priv->ax_vi;
        if (!this->is_opened()) {
            err::Err e = open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "open camera failed");
        }

        if (_show_colorbar) {
            image::Image *img = new image::Image(_width, _height);
            err::check_null_raise(img, "create colorbar image failed");
            __generate_colorbar(*img);
            return img;
        } else {
            if (!block) block_ms = 0;
            auto frame = vi->pop(priv->chn.id, block_ms);
            if (!block && frame == nullptr) {
                return nullptr;
            } else if (frame == nullptr) {
                err::check_raise(err::ERR_BUFF_EMPTY, "read camera failed");
            }

            auto pipeline_frame = pipeline::Frame(frame, true);
            auto img = pipeline_frame.to_image();
            return img;
        }
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

        constexpr AX_U8 pipe_id = 0;
        constexpr AX_VIN_PIPE_DUMP_NODE_E dump_node = AX_VIN_PIPE_DUMP_NODE_IFE;
        constexpr AX_SNS_HDR_FRAME_E sns_frame = AX_SNS_HDR_FRAME_L;
        AX_IMG_INFO_T img_info = {};
        AX_S32 ax_ret = AX_VIN_GetRawFrame(pipe_id, dump_node, sns_frame, &img_info, 5000);
        if (ax_ret != AX_SUCCESS) {
            if (ax_ret == AX_ERR_VIN_RES_EMPTY) {
                err::check_raise(err::ERR_BUFF_EMPTY, "Raw buffer empty");
            }
            log::error("AX_VIN_GetRawFrame failed, ret:0x%x", ax_ret);
            err::check_raise(err::ERR_RUNTIME, "AX_VIN_GetRawFrame failed");
        }

        bool frame_acquired = true;
        AX_VOID *mapped_addr = nullptr;
        const AX_VIDEO_FRAME_T &raw_frame = img_info.tFrameInfo.stVFrame;

        auto cleanup_raw_frame = [&]() {
            if (mapped_addr) {
                AX_S32 ret = AX_SYS_Munmap(mapped_addr, raw_frame.u32FrameSize);
                if (ret != AX_SUCCESS) {
                    log::error("AX_SYS_Munmap failed for RAW frame, ret:0x%x", ret);
                }
                mapped_addr = nullptr;
            }
            if (frame_acquired) {
                AX_S32 ret = AX_VIN_ReleaseRawFrame(pipe_id, dump_node, sns_frame, &img_info);
                if (ret != AX_SUCCESS) {
                    log::error("AX_VIN_ReleaseRawFrame failed, ret:0x%x", ret);
                }
                frame_acquired = false;
            }
        };

        if (raw_frame.u64PhyAddr[0] == 0 || raw_frame.u32FrameSize == 0 ||
            raw_frame.u32Width == 0 || raw_frame.u32Height == 0) {
            cleanup_raw_frame();
            err::check_raise(err::ERR_RUNTIME, "RAW frame descriptor is invalid");
        }

        // Always create and own a separate CPU mapping. The virtual address in
        // the VIN descriptor belongs to VIN and must remain untouched for release.
        mapped_addr = AX_SYS_MmapCache(raw_frame.u64PhyAddr[0], raw_frame.u32FrameSize);
        if (!mapped_addr) {
            cleanup_raw_frame();
            err::check_raise(err::ERR_RUNTIME, "AX_SYS_MmapCache failed for RAW frame");
        }

        ax_ret = AX_SYS_MinvalidateCache(raw_frame.u64PhyAddr[0], mapped_addr, raw_frame.u32FrameSize);
        if (ax_ret != AX_SUCCESS) {
            log::error("AX_SYS_MinvalidateCache failed for RAW frame, ret:0x%x", ax_ret);
            cleanup_raw_frame();
            err::check_raise(err::ERR_RUNTIME, "AX_SYS_MinvalidateCache failed for RAW frame");
        }

        image::Image *img = nullptr;
        try {
            img = new image::Image(raw_frame.u32Width, raw_frame.u32Height, image::FMT_RGGB10,
                                   static_cast<uint8_t *>(mapped_addr), raw_frame.u32FrameSize, true);
        } catch (...) {
            cleanup_raw_frame();
            throw;
        }

        cleanup_raw_frame();
        return img;
    }

    pipeline::Frame *Camera::pop(int block_ms) {
        auto *priv = (camera_priv_t *)_param;
        auto vi = priv->ax_vi;
        if (!this->is_opened()) {
            err::Err e = open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "open camera failed");
        }

        auto frame = vi->pop(priv->chn.id, block_ms);
        if (!frame)
            return nullptr;

        return new pipeline::Frame(frame, true);
    }

    err::Err Camera::show_colorbar(bool enable)
    {
        // only set variable now
        // should control camera to show colorbar
        _show_colorbar = enable;
        return err::ERR_NONE;
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
        err::Err ret = err::ERR_NONE;
        auto *priv = (camera_priv_t *)_param;
        auto vi = priv->ax_vi;

        ret = vi->del_channel(priv->chn.id);
        err::check_raise(ret, "del channel failed");
        ret = vi->add_channel(priv->chn.id, width, height, get_ax_fmt_from_maix(priv->chn.fmt), priv->chn.fps,
                            priv->chn.depth, false, false, priv->chn.fit);
        err::check_raise(ret, "del channel failed");
        return ret;
    }

    err::Err Camera::set_fps(double fps) {
        camera_priv_t *priv = (camera_priv_t *)_param;

        /* Do not change the established behaviour of other MaixCAM2
         * sensors: their set_fps implementation historically only adjusted
         * the exposure limit.  OS04A10 timing, however, is programmed during
         * VIN/sensor stream-on and must be reopened. */
        if (get_device_name() != "ov_os04a10") {
            if (fps > 0) {
                this->exposure(1000 / fps * 1000);
            }
            return err::ERR_NONE;
        }

        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        const err::Err validation = __validate_os04a10_fps(_windowing, fps);
        if (validation != err::ERR_NONE) {
            return validation;
        }

        const bool old_fps_auto = priv->fps_auto;
        const double old_requested_fps = priv->requested_fps;
        const image::Format old_format = _format;
        const int old_buff_num = _buff_num;
        const bool target_fps_auto = fps <= 0;
        /* open(..., -1) means "use the already recorded state", whereas 0
         * explicitly records selector-auto.  Use 0 for a public auto FPS
         * request so a previous explicit value cannot leak into this reopen. */
        const double target_open_fps = target_fps_auto ? 0 : fps;

        this->close();
        err::Err ret = this->open(_width, _height, old_format, target_open_fps, old_buff_num);
        if (ret != err::ERR_NONE) {
            log::error("OS04A10 failed to apply %.0f fps; restoring the previous stream", fps);
            const double restore_fps = old_fps_auto ? 0 : old_requested_fps;
            const err::Err restore_ret = this->open(_width, _height, old_format, restore_fps, old_buff_num);
            if (restore_ret != err::ERR_NONE) {
                log::error("OS04A10 failed to restore the previous stream after FPS reconfiguration");
            }
            return ret;
        }

        /* A fresh ISP instance needs a short AWB/AE settling window. */
        int warmup_frames = 30;
        if (_fps >= 360) {
            warmup_frames = 180;
        } else if (_fps >= 180) {
            warmup_frames = 90;
        }
        if (!__warmup_os04a10(*this, warmup_frames)) {
            log::error("OS04A10 %.0f fps restart produced no usable frames; restoring the previous stream", fps);
            this->close();
            const double restore_fps = old_fps_auto ? 0 : old_requested_fps;
            const err::Err restore_ret = this->open(_width, _height, old_format, restore_fps, old_buff_num);
            if (restore_ret != err::ERR_NONE) {
                log::error("OS04A10 failed to restore the previous stream after warmup timeout");
            }
            return err::ERR_RUNTIME;
        }
        return err::ERR_NONE;
    }

    int Camera::exposure(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        AX_S32 ax_res = AX_SUCCESS;
        AX_ISP_IQ_AE_PARAM_T tAeParam;
        int current_exposure_time = 0;

        ax_res = AX_ISP_IQ_GetAeParam(_ch, &tAeParam);
        if (ax_res != AX_SUCCESS) {
            log::info("get ae param failed");
            return err::ERR_RUNTIME;
        }

        if (value > 0) {
            tAeParam.nEnable = false;
            tAeParam.tExpManual.nShutter = value;
            tAeParam.tExpManual.nShortShutter = value / 16;
            tAeParam.tExpManual.nVsShutter = value;
            ax_res = AX_ISP_IQ_SetAeParam(_ch, &tAeParam);
            if (ax_res != AX_SUCCESS) {
                log::info("set ae param failed");
                return err::ERR_RUNTIME;
            }
        }

        ax_res = AX_ISP_IQ_GetAeParam(_ch, &tAeParam);
        if (ax_res != AX_SUCCESS) {
            log::info("get ae param failed");
            return err::ERR_RUNTIME;
        }
        current_exposure_time = tAeParam.tExpManual.nShutter;
        _fps = 1000000 / current_exposure_time;
        return current_exposure_time;
    }

    int Camera::gain(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        AX_S32 ax_res = AX_SUCCESS;
        AX_ISP_IQ_AE_PARAM_T tAeParam;
        int current_gain = 0;

        ax_res = AX_ISP_IQ_GetAeParam(_ch, &tAeParam);
        if (ax_res != AX_SUCCESS) {
            log::info("get ae param failed");
            return err::ERR_RUNTIME;
        }

        if (value > 0) {
            tAeParam.nEnable = false;
            // tAeParam.tExpManual.nSysTotalGain = value;
            // tAeParam.tExpManual.nIspGain = value;
            tAeParam.tExpManual.nAGain = value;
            ax_res = AX_ISP_IQ_SetAeParam(_ch, &tAeParam);
            if (ax_res != AX_SUCCESS) {
                log::info("set ae param failed");
                return err::ERR_RUNTIME;
            }
        }

        ax_res = AX_ISP_IQ_GetAeParam(_ch, &tAeParam);
        if (ax_res != AX_SUCCESS) {
            log::info("get ae param failed");
            return err::ERR_RUNTIME;
        }
        current_gain = tAeParam.tExpManual.nSysTotalGain;
        return current_gain;
    }

    int Camera::iso(int value) {
        int gain_vale = (double)value / 100.0 * 1024;
        return this->gain(gain_vale);
    }

    int Camera::hmirror(int value) {
        if (!this->is_opened()) {
            return -1;
        }

        auto *priv = (camera_priv_t *)_param;
        auto ax_vi = priv->ax_vi;
        peripheral::i2c::I2C i2c_obj(0, peripheral::i2c::Mode::MASTER);
        int mirror = false;
        switch (priv->i2c_addr) {
        case 0x30:
        {
            uint8_t reg_val = 0;
            uint16_t reg_addr = 0x3221;
            auto data = i2c_obj.readfrom_mem(priv->i2c_addr, reg_addr, 1, 16);
            if (!data) {
                return -1;
            }
            #define SC850SL_MIRROR_MASK (0x60)
            reg_val = data->data[0];
            mirror = (reg_val & SC850SL_MIRROR_MASK) ? true : false;
            if (value >= 0) {
                mirror = value > 0 ? true : false;
                if (mirror) {
                    reg_val = (reg_val & ~SC850SL_MIRROR_MASK) | SC850SL_MIRROR_MASK;
                } else {
                    reg_val = (reg_val & ~SC850SL_MIRROR_MASK);
                }
                uint8_t temp[1] = {reg_val};
                i2c_obj.writeto_mem(priv->i2c_addr, reg_addr, temp, sizeof(temp), 16);
            } else {

            }
        }
        break;
        case 0x36:
        {
            uint8_t reg_val = 0;
            uint16_t reg_addr = 0x3820;
            auto data = i2c_obj.readfrom_mem(priv->i2c_addr, reg_addr, 1, 16);
            if (!data) {
                return -1;
            }
            #define OS04A10_MIRROR_MASK (0x02)
            reg_val = data->data[0];
            mirror = (reg_val & OS04A10_MIRROR_MASK) ? true : false;
            if (value >= 0) {
                mirror = value > 0 ? true : false;
                if (mirror) {
                    reg_val = (reg_val & ~OS04A10_MIRROR_MASK) | OS04A10_MIRROR_MASK;
                } else {
                    reg_val = (reg_val & ~OS04A10_MIRROR_MASK);
                }
                uint8_t temp[1] = {reg_val};
                i2c_obj.writeto_mem(priv->i2c_addr, reg_addr, temp, sizeof(temp), 16);
            } else {

            }
        }
        break;
        case 0x3c:
        {
            mirror = ax_vi->set_and_get_mirror(priv->chn.id, value);
        }
        break;
        default:
        break;
        }

        priv->chn.mirror = mirror;
        return mirror;
    }

    int Camera::vflip(int value) {
        if (!this->is_opened()) {
            return -1;
        }

        auto *priv = (camera_priv_t *)_param;
        auto ax_vi = priv->ax_vi;
        peripheral::i2c::I2C i2c_obj(0, peripheral::i2c::Mode::MASTER);
        int flip = false;
        switch (priv->i2c_addr) {
        case 0x30:
        {
            uint8_t reg_val = 0;
            uint16_t reg_addr = 0x3221;
            auto data = i2c_obj.readfrom_mem(priv->i2c_addr, reg_addr, 1, 16);
            if (!data) {
                return -1;
            }
            #define SC850SL_FLIP_MASK (0x6)
            reg_val = data->data[0];
            flip = (reg_val & SC850SL_FLIP_MASK) ? true : false;
            if (value >= 0) {
                flip = value > 0 ? true : false;
                if (flip) {
                    reg_val = (reg_val & ~SC850SL_FLIP_MASK) | SC850SL_FLIP_MASK;
                } else {
                    reg_val = (reg_val & ~SC850SL_FLIP_MASK);
                }
                uint8_t temp[1] = {reg_val};
                i2c_obj.writeto_mem(priv->i2c_addr, reg_addr, temp, sizeof(temp), 16);
            } else {

            }
        }
        break;
        case 0x36:
        {
            uint8_t reg_val = 0;
            uint16_t reg_addr = 0x3820;
            auto data = i2c_obj.readfrom_mem(priv->i2c_addr, reg_addr, 1, 16);
            if (!data) {
                return -1;
            }
            #define OS04A10_FLIP_MASK (0x04)
            reg_val = data->data[0];
            flip = (reg_val & OS04A10_FLIP_MASK) ? true : false;
            if (value >= 0) {
                flip = value > 0 ? true : false;
                if (flip) {
                    reg_val = (reg_val & ~OS04A10_FLIP_MASK) | OS04A10_FLIP_MASK;
                } else {
                    reg_val = (reg_val & ~OS04A10_FLIP_MASK);
                }
                uint8_t temp[1] = {reg_val};
                i2c_obj.writeto_mem(priv->i2c_addr, reg_addr, temp, sizeof(temp), 16);
            } else {

            }
        }
        break;
        case 0x3c:
        {
            flip = ax_vi->set_and_get_flip(priv->chn.id, value);
        }
        default:
        break;
        }

        priv->chn.vflip = flip;
        return flip;
    }

    int Camera::luma(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        AX_S32 ax_res = AX_SUCCESS;
        AX_ISP_IQ_YCPROC_PARAM_T param;
        int current_value = 0;
        if (value >= 0) {
            ax_res = AX_ISP_IQ_GetYcprocParam(_ch, &param);
            if (ax_res != AX_SUCCESS) {
                log::error("AX_ISP_IQ_GetYcprocParam failed: %d", ax_res);
                return current_value;
            }

            param.nBrightness = ((double)value / 100) * 4096;
            param.nBrightness = param.nBrightness < 0 ? 0 : param.nBrightness;
            param.nBrightness = param.nBrightness > 4095 ? 4095 : param.nBrightness;
            ax_res = AX_ISP_IQ_SetYcprocParam(_ch, &param);
            if (ax_res != AX_SUCCESS) {
                log::error("AX_ISP_IQ_SetYcprocParam failed: %d", ax_res);
                return current_value;
            }
        }

        ax_res = AX_ISP_IQ_GetYcprocParam(_ch, &param);
        if (ax_res != AX_SUCCESS) {
            log::error("AX_ISP_IQ_GetYcprocParam failed: %d", ax_res);
            return current_value;
        }

        current_value = (float)param.nBrightness / 4096 * 100;
        return current_value;
    }

    int Camera::constrast(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        AX_S32 ax_res = AX_SUCCESS;
        AX_ISP_IQ_YCPROC_PARAM_T param;
        int current_value = 0;
        if (value >= 0) {
            ax_res = AX_ISP_IQ_GetYcprocParam(_ch, &param);
            if (ax_res != AX_SUCCESS) {
                log::error("AX_ISP_IQ_GetYcprocParam failed: %d", ax_res);
                return current_value;
            }

            param.nContrast = ((double)value / 100) * 8192 - 4096;
            param.nContrast = param.nContrast < -4096 ? -4096 : param.nContrast;
            param.nContrast = param.nContrast > 4095 ? 4095 : param.nContrast;
            ax_res = AX_ISP_IQ_SetYcprocParam(_ch, &param);
            if (ax_res != AX_SUCCESS) {
                log::error("AX_ISP_IQ_SetYcprocParam failed: %d", ax_res);
                return current_value;
            }
        }

        ax_res = AX_ISP_IQ_GetYcprocParam(_ch, &param);
        if (ax_res != AX_SUCCESS) {
            log::error("AX_ISP_IQ_GetYcprocParam failed: %d", ax_res);
            return current_value;
        }

        current_value = (float)(param.nContrast + 4096) / 8196 * 100;
        return current_value;
    }

    int Camera::saturation(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        AX_S32 ax_res = AX_SUCCESS;
        AX_ISP_IQ_YCPROC_PARAM_T param;
        int current_value = 0;
        if (value >= 0) {
            ax_res = AX_ISP_IQ_GetYcprocParam(_ch, &param);
            if (ax_res != AX_SUCCESS) {
                log::error("AX_ISP_IQ_GetYcprocParam failed: %d", ax_res);
                return current_value;
            }

            param.nSaturation = ((double)value / 100) * 65535;
            param.nSaturation = param.nSaturation < 0 ? 0 : param.nSaturation;
            param.nSaturation = param.nSaturation > 65535 ? 65535 : param.nSaturation;
            ax_res = AX_ISP_IQ_SetYcprocParam(_ch, &param);
            if (ax_res != AX_SUCCESS) {
                log::error("AX_ISP_IQ_SetYcprocParam failed: %d", ax_res);
                return current_value;
            }
        }

        ax_res = AX_ISP_IQ_GetYcprocParam(_ch, &param);
        if (ax_res != AX_SUCCESS) {
            log::error("AX_ISP_IQ_GetYcprocParam failed: %d", ax_res);
            return current_value;
        }

        current_value = (float)param.nSaturation / 65536 * 100;log::info("2param.nSaturation: %d", param.nSaturation);
        return current_value;
    }

    AwbMode Camera::awb_mode(AwbMode value) {
        if (!this->is_opened()) {
            return AwbMode::Invalid;
        }

        AX_S32 ax_res;
        AX_ISP_IQ_AWB_PARAM_T param;
        AwbMode current_mode = AwbMode::Invalid;

        if (value != AwbMode::Invalid) {
            ax_res = AX_ISP_IQ_GetAwbParam(_ch, &param);
            if (ax_res != 0) {
                log::error("AX_ISP_IQ_GetAwbParam failed: %d", ax_res);
                return current_mode;
            }

            param.nEnable = (value == AwbMode::Manual ? 0 : 1);
            ax_res = AX_ISP_IQ_SetAwbParam(_ch, &param);
            if (ax_res != 0) {
                log::error("AX_ISP_IQ_SetAwbParam failed: %d", ax_res);
                return current_mode;
            }
        }

        ax_res = AX_ISP_IQ_GetAwbParam(_ch, &param);
        if (ax_res != 0) {
            log::error("AX_ISP_IQ_GetAwbParam failed: %d", ax_res);
            return current_mode;
        }

        current_mode = param.nEnable > 0 ? AwbMode::Auto : AwbMode::Manual;
        return current_mode;
    }

    int Camera::set_awb(int value) {
        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }
        err::check_raise(err::ERR_NOT_IMPL, "set awb failed");
        return -1;
    }

    static uint16_t get_gain_float2u16(float gain) {
        uint16_t new_gain = 0;
        gain = gain > 1 ? 1 : gain;
        gain = gain < 0 ? 0 : gain;
        new_gain = (4095 - 256) * gain + 256;
        return new_gain;
    }

    static float get_gain_u162float(uint16_t gain) {
        float new_gain = 0;
        new_gain = (gain - 256) / (4095 - 256);
        return new_gain;
    }

    std::vector<float> Camera::set_wb_gain(std::vector<float> gains) {
        std::vector<float> new_gains;
        if (!this->is_opened()) {
            log::warn("Camera is not opened");
            return new_gains;
        }

        AX_S32 ax_res;
        AX_ISP_IQ_AWB_PARAM_T param;
        if (gains.size() >= 4) {
            ax_res = AX_ISP_IQ_GetAwbParam(_ch, &param);
            if (ax_res != 0) {
                log::error("AX_ISP_IQ_GetAwbParam failed: %d", ax_res);
                return new_gains;
            }

            param.nEnable = 0;  // 0,manual; 1,auto
            param.tManualParam.tGain.nGainR = get_gain_float2u16(gains[0]);
            param.tManualParam.tGain.nGainGr = get_gain_float2u16(gains[1]);
            param.tManualParam.tGain.nGainGb = get_gain_float2u16(gains[2]);
            param.tManualParam.tGain.nGainB = get_gain_float2u16(gains[3]);
            ax_res = AX_ISP_IQ_SetAwbParam(_ch, &param);
            if (ax_res != 0) {
                log::error("AX_ISP_IQ_SetAwbParam failed: %d", ax_res);
                return new_gains;
            }
        }

        // ax_res = AX_ISP_IQ_GetAwbParam(_ch, &param);
        // if (ax_res != 0) {
        //     log::error("AX_ISP_IQ_GetAwbParam failed: %d", ax_res);
        //     return new_gains;
        // }
        // log::info("get awb gains %d %d %d %d", param.tManualParam.tGain.nGainR, param.tManualParam.tGain.nGainGr, param.tManualParam.tGain.nGainGb, param.tManualParam.tGain.nGainB);


        AX_ISP_IQ_AWB_STATUS_T status;
        ax_res = AX_ISP_IQ_GetAwbStatus(_ch, &status);
        if (ax_res != 0) {
            log::error("AX_ISP_IQ_GetAwbStatus failed: %d", ax_res);
            return new_gains;
        }
        new_gains.push_back(get_gain_u162float(status.tGainStatus.nGainR));
        new_gains.push_back(get_gain_u162float(status.tGainStatus.nGainGr));
        new_gains.push_back(get_gain_u162float(status.tGainStatus.nGainGb));
        new_gains.push_back(get_gain_u162float(status.tGainStatus.nGainB));
        // log::info("awb gains:[%d, %d, %d, %d] colot temperature: %d", status.tGainStatus.nGainR, status.tGainStatus.nGainGr, status.tGainStatus.nGainGb, status.tGainStatus.nGainB, status.tAlgoStatus.nCct);

        return new_gains;
    }

    AeMode Camera::exp_mode(AeMode value) {
        if (!this->is_opened()) {
            return AeMode::Invalid;
        }

        AX_S32 ax_res;
        AX_ISP_IQ_AE_PARAM_T param;
        AeMode current_mode = AeMode::Invalid;
        if (value != AeMode::Invalid) {
            ax_res = AX_ISP_IQ_GetAeParam(_ch, &param);
            if (ax_res != 0) {
                log::error("AX_ISP_IQ_GetAeParam failed: %d", ax_res);
                return current_mode;
            }

            current_mode = value;
            param.nEnable = (value == AeMode::Manual ? 0 : 1);
            ax_res = AX_ISP_IQ_SetAeParam(_ch, &param);
            if (ax_res != 0) {
                log::error("AX_ISP_IQ_GetAeParam failed: %d", ax_res);
                return current_mode;
            }
        }

        ax_res = AX_ISP_IQ_GetAeParam(_ch, &param);
        if (ax_res != 0) {
            log::error("AX_ISP_IQ_GetAeParam failed: %d", ax_res);
            return current_mode;
        }

        current_mode = param.nEnable > 0 ? AeMode::Auto : AeMode::Manual;
        return current_mode;
    }

    err::Err Camera::set_windowing(std::vector<int> roi) {
        camera_priv_t *priv = (camera_priv_t *)_param;
        auto &mod_param = AxModuleParam::getInstance();
        int max_width = 2688;
        int max_height = 1520;
        bool os04a10 = false;
        if (this->is_opened()) {
            mod_param.lock(AX_MOD_VI);
            auto vi_param = (ax_vi_mod_t *)mod_param.get_param(AX_MOD_VI);
            auto ax_cam = vi_param->cams[0];
            mod_param.unlock(AX_MOD_VI);
            os04a10 = ax_cam.eSnsType == OMNIVISION_OS04A10;
            if (!os04a10) {
                max_width = ax_cam.tSnsAttr.nWidth;
                max_height = ax_cam.tSnsAttr.nHeight;
            }
        } else {
            os04a10 = get_device_name() == "ov_os04a10";
        }

        err::Err ret = err::ERR_NONE;
        char log_msg[100];
        int x = 0, y = 0, w = 0, h = 0;

        if (roi.size() == 4) {
            x = roi[0], y = roi[1], w = roi[2], h = roi[3];
        } else if (roi.size() == 2) {
            w = roi[0], h = roi[1];
            const bool binning = os04a10 && w <= kOs04a10BinnedMaxWidth && h <= kOs04a10BinnedMaxHeight;
            if (binning) {
                /* x/y are native-array coordinates; width/height are the
                 * post-binning output. Center the corresponding 2x2
                 * physical readout, including the 8-pixel output guard. */
                x = (2704 - 2 * (w + 8)) / 2;
                y = (1536 - (2 * h + 8)) / 2;
                x &= ~1;
                y &= ~1;
            } else {
                x = (max_width - w) / 2;
                y = (max_height - h) / 2;
            }
        } else {
            err::check_raise(err::ERR_RUNTIME, "roi size must be 4 or 2");
        }

        snprintf(log_msg, sizeof(log_msg), "Width must be a multiple of 2.");
        err::check_bool_raise(w % 2 == 0, std::string(log_msg));
        if (os04a10) {
            const bool binning = w <= kOs04a10BinnedMaxWidth && h <= kOs04a10BinnedMaxHeight;
            err::check_bool_raise(x % 2 == 0 && y % 2 == 0,
                                  "sensor crop x and y must be even");
            err::check_bool_raise(w >= 256 && w <= max_width && w % 16 == 0,
                                  "sensor crop width must be 256..2688 and a multiple of 16");
            err::check_bool_raise(h >= 20 && h <= max_height && h % 2 == 0,
                                  "sensor crop height must be 20..1520 and even");
            /* Reject this verified AX ISP failure class before stopping an
             * active stream. It is not an alignment requirement. */
            if (w < 1024 && h > 464) {
                throw err::Exception(err::ERR_ARGS,
                    "This OS04A10 crop does not work. Some offset, width, and height combinations are not supported. "
                    "For this size, use height <= 464 or width >= 1024.");
            }
            const int physical_w = binning ? 2 * (w + 8) : w + 16;
            const int physical_h = binning ? 2 * h + 8 : h + 16;
            err::check_bool_raise(x + physical_w <= 2704,
                                  "sensor crop physical readout exceeds 2704 pixels");
            err::check_bool_raise(y + physical_h <= 1536,
                                  "sensor crop physical readout exceeds 1536 lines");
        }
        snprintf(log_msg, sizeof(log_msg), "the coordinate x range needs to be [0,%d].", max_width - 1);
        err::check_bool_raise(x >= 0 && x < max_width, std::string(log_msg));
        snprintf(log_msg, sizeof(log_msg), "the coordinate y range needs to be [0,%d].", max_height - 1);
        err::check_bool_raise(y >= 0 && y < max_height, std::string(log_msg));
        snprintf(log_msg, sizeof(log_msg), "the row of the window is larger than the maximum, try x=%d, w=%d.", x, max_width - x);
        err::check_bool_raise(x + w <= max_width, std::string(log_msg));
        snprintf(log_msg, sizeof(log_msg), "the column of the window is larger than the maximum, try y=%d, h=%d.", y, max_height - y);
        err::check_bool_raise(y + h <= max_height, std::string(log_msg));

        if (os04a10) {
            const std::vector<int> old_windowing = _windowing;
            const double old_fps = priv->fps_auto ? -1 : priv->requested_fps;
            const err::Err validation = __validate_os04a10_fps({x, y, w, h}, old_fps);
            if (validation != err::ERR_NONE) {
                return validation;
            }
            _windowing = {x, y, w, h};
            if (!this->is_opened()) {
                return ret;
            }
            /* Sensor timing and VIN dimensions are established during open;
             * restart the pipeline so the new ROI is applied before stream-on. */
            auto old_format = _format;
            auto old_buff_num = _buff_num;
            this->close();
            /* Preserve an explicit caller FPS across the restart. Passing
             * -1 re-enters the documented automatic 60 fps mode, so only do
             * that for an originally automatic request. */
            ret = this->open(_width, _height, old_format, old_fps, old_buff_num);
            if (ret != err::ERR_NONE) {
                log::error("OS04A10 failed to apply the new crop; restoring the previous stream");
                _windowing = old_windowing;
                const err::Err restore_ret = this->open(_width, _height, old_format, old_fps, old_buff_num);
                if (restore_ret != err::ERR_NONE) {
                    log::error("OS04A10 failed to restore the previous stream after crop reconfiguration");
                }
                return ret;
            }
            {
                /* A fresh ISP instance needs a short AWB/AE settling window.
                 * Without discarding these first frames, a dynamic crop can
                 * briefly show magenta/green output while the statistics
                 * block converges. Keep the restart synchronous so the first
                 * frame returned to the caller is already usable. */
                int warmup_frames = 30;
                if (_fps >= 360) {
                    warmup_frames = 180;
                } else if (_fps >= 180) {
                    warmup_frames = 90;
                }
                if (!__warmup_os04a10(*this, warmup_frames)) {
                    log::error("This OS04A10 crop does not work. Some offset, width, and height combinations are not supported. "
                               "Try another crop area. Restoring the previous stream.");
                    this->close();
                    _windowing = old_windowing;
                    const err::Err restore_ret = this->open(_width, _height, old_format, old_fps, old_buff_num);
                    if (restore_ret != err::ERR_NONE) {
                        log::error("OS04A10 failed to restore the previous stream after crop warmup timeout");
                    }
                    throw err::Exception("This OS04A10 crop does not work. Some offset, width, and height combinations are not supported. "
                                         "Try another crop area.");
                }
            }
            return ret;
        }

        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        auto ax_vi = priv->ax_vi;
        bool is_vflip = priv->chn.vflip, is_hmirror = priv->chn.mirror;
        if (!is_vflip) {
            y = max_height - y - h;
        }
        if (!is_hmirror) {
            x = max_width - x - w;
        }
        err::check_bool_raise(!ax_vi->set_windowing(priv->chn.id,  x, y, w, h), "set windowing failed.");
        return ret;
    }

    std::vector<int> Camera::get_sensor_size() {
        auto &mod_param = AxModuleParam::getInstance();
        mod_param.lock(AX_MOD_VI);
        auto vi_param = (ax_vi_mod_t *)mod_param.get_param(AX_MOD_VI);
        auto ax_cam = vi_param->cams[0];
        mod_param.unlock(AX_MOD_VI);

        int max_width = ax_cam.tSnsAttr.nWidth;
        int max_height = ax_cam.tSnsAttr.nHeight;

        if (ax_cam.eSnsType == SMARTSENS_SC850SL) {
            max_width = 3840;
            max_height = 2160;
        } else if (ax_cam.eSnsType == OMNIVISION_OS04D10) {
            max_width = 2560;
            max_height = 1440;
        } else if (ax_cam.eSnsType == OMNIVISION_OS04A10) {
            max_width = 2688;
            max_height = 1520;
            if (_is_opened && ax_cam.tSnsAttr.nWidth > 0 && ax_cam.tSnsAttr.nHeight > 0) {
                max_width = ax_cam.tSnsAttr.nWidth;
                max_height = ax_cam.tSnsAttr.nHeight;
            }
        }
        return {max_width, max_height};
    }

    err::Err Camera::write_reg(int addr, int data, int bit_width)
    {
        camera_priv_t *priv = (camera_priv_t *)_param;
        (void)bit_width;
        peripheral::i2c::I2C i2c_obj(0, peripheral::i2c::Mode::MASTER);
        uint8_t temp[1];
        temp[0] = (uint8_t)data;
        i2c_obj.writeto_mem(priv->i2c_addr, addr, temp, sizeof(temp), 16);
        return err::ERR_NONE;
    }

    int Camera::read_reg(int addr, int bit_width)
    {
        camera_priv_t *priv = (camera_priv_t *)_param;
        (void)bit_width;
        peripheral::i2c::I2C i2c_obj(0, peripheral::i2c::Mode::MASTER);
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

    void *Camera::get_driver()
    {
        auto *priv = (camera_priv_t *)_param;
        auto ax_vi = priv->ax_vi;
        return ax_vi;
    }

    bool Camera::get_aiisp_workmode() {
        auto *priv = (camera_priv_t *)_param;
        AX_ISP_IQ_SCENE_PARAM_T IspSceneParam = {0};
        AX_S32 ret = 0;
        if (0 != (ret = AX_ISP_IQ_GetSceneParam(priv->chn.id, &IspSceneParam))) {
            log::error("AX_ISP_IQ_GetSceneParam failed: %d", ret);
            return false;
        }

        return IspSceneParam.tManualParam.nAiWorkMode ? true : false;
    }
}
