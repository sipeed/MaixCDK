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
#include <memory>
#include "ax_middleware.hpp"

using namespace maix::peripheral;
using namespace maix::middleware;
namespace maix::display
{
    enum class PanelType {
        NORMAL,
        LT9611,
        UNKNOWN,
    };
    // inline static PanelType __g_panel_type = PanelType::UNKNOWN;

    __attribute__((unused)) static int _get_vo_max_size(int *width, int *height, int rotate)
    {
        int w = 480, h = 640;

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
            if (board_id == "maixcam2") {
                flip = true;
            }
        }

        // log::info("disp config flip: %d, mirror: %d, max_backlight: %.1f", flip, mirror, max_backlight);
    }


    class DisplayAx final : public DisplayBase
    {
        std::unique_ptr<maixcam2::SYS> __sys;
        std::unique_ptr<maixcam2::VO> __vo;

        static void __config_vo_param(maixcam2::ax_vo_param_t *new_param, int width, int height, image::Format format, int rotate) {
            memset(new_param, 0, sizeof(maixcam2::ax_vo_param_t));
            AX_VO_SYNC_INFO_T sync_info = {.u16Vact = 640, .u16Vbb = 30, .u16Vfb = 30, .u16Hact = 480, .u16Hbb = 30, .u16Hfb = 30, .u16Hpw = 40, .u16Vpw = 11, .u32Pclk = 24750, .bIdv = AX_TRUE, .bIhs = AX_FALSE, .bIvs = AX_TRUE};
            SAMPLE_VO_CONFIG_S vo_cfg = {
                .u32VDevNr = 1,
                .stVoDev = {{
                    .u32VoDev = 0,
                    .enMode = AX_VO_MODE_OFFLINE,
                    .enVoIntfType = AX_VO_INTF_DSI,
                    .enIntfSync = AX_VO_OUTPUT_USER,
                    .enVoOutfmt = AX_VO_OUT_FMT_UNUSED,
                    .u32SyncIndex = 2,
                    .setCsc = AX_FALSE,
                    .bWbcEn = AX_FALSE,
                }},
                .stVoLayer = {
                    {
                        .bindVoDev = {SAMPLE_VO_DEV_MAX, SAMPLE_VO_DEV_MAX},
                        .enChnFrmFmt = AX_FORMAT_YUV420_SEMIPLANAR,
                    },
                    {
                        .bindVoDev = {SAMPLE_VO_DEV_MAX, SAMPLE_VO_DEV_MAX},
                        .enChnFrmFmt = AX_FORMAT_YUV420_SEMIPLANAR,
                    },
                },
            };
            vo_cfg.stGraphicLayer[0].u32FbNum = 1;
            vo_cfg.stGraphicLayer[0].stFbConf[0].u32Index = 0;
            vo_cfg.stGraphicLayer[0].stFbConf[0].u32Fmt = AX_FORMAT_ARGB8888;
            if (rotate) {
                vo_cfg.stVoLayer[0].stVoLayerAttr.stImageSize.u32Width = height;
                vo_cfg.stVoLayer[0].stVoLayerAttr.stImageSize.u32Height = width;
                vo_cfg.stGraphicLayer[0].stFbConf[0].u32ResoW = height;
                vo_cfg.stGraphicLayer[0].stFbConf[0].u32ResoH = width;
            } else {
                vo_cfg.stVoLayer[0].stVoLayerAttr.stImageSize.u32Width = height;
                vo_cfg.stVoLayer[0].stVoLayerAttr.stImageSize.u32Height = width;
                vo_cfg.stGraphicLayer[0].stFbConf[0].u32ResoW = width;
                vo_cfg.stGraphicLayer[0].stFbConf[0].u32ResoH = height;
            }
            vo_cfg.stVoLayer[0].stVoLayerAttr.enPixFmt = AX_FORMAT_YUV420_SEMIPLANAR;
            vo_cfg.stVoLayer[0].stVoLayerAttr.u32DispatchMode = 1;
            vo_cfg.stVoLayer[0].stVoLayerAttr.f32FrmRate = 60.0;
            vo_cfg.stVoLayer[0].u32ChnNr = 1;
            memcpy(&new_param->vo_cfg, &vo_cfg, sizeof(vo_cfg));
            memcpy(&new_param->sync_info, &sync_info, sizeof(sync_info));
        }
    public:
        DisplayAx(const string &device, int width, int height, image::Format format)
        {
            bool rotate = 1;
            err::check_bool_raise(!_get_vo_max_size(&_max_width, &_max_height, rotate), "get vo max size failed");
            width = width <= 0 ? _max_width : width;
            height = height <= 0 ? _max_height : height;
            this->_width = width > _max_width ? _max_width : width;
            this->_height = height > _max_height ? _max_height : height;
            this->_format = format;
            this->_opened = false;
            this->_format = format;
            this->_invert_flip = false;
            this->_invert_mirror = false;
            this->_pool_id = AX_INVALID_POOLID;
            this->_layer = 0;       // layer 0 means vedio layer
            err::check_bool_raise(_format == image::FMT_RGB888
                                || _format == image::FMT_YVU420SP
                                || _format == image::FMT_YUV420SP
                                || _format == image::FMT_BGRA8888, "Format not support");

            _get_disp_configs(this->_invert_flip, this->_invert_mirror, _max_backlight);

            __sys = std::make_unique<maixcam2::SYS>();
            err::check_bool_raise(__sys != nullptr, "display construct sys failed");
            err::check_bool_raise(__sys->init() == err::ERR_NONE, "display init sys failed");

            __vo = std::make_unique<maixcam2::VO>();
            err::check_bool_raise(__vo != nullptr, "VO init failed");

            maixcam2::ax_vo_param_t vo_param;
            __config_vo_param(&vo_param, width, height, format, rotate);
            err::check_bool_raise(__vo->init(&vo_param) == err::ERR_NONE, "VO init failed");
            _bl_pwm = nullptr;
            int pwm_id = 5;
            _bl_pwm = new pwm::PWM(pwm_id, 10000, 50);
        }

        DisplayAx(int layer, int width, int height, image::Format format)
        {
            bool rotate = 1;
            err::check_bool_raise(!_get_vo_max_size(&_max_width, &_max_height, rotate), "get vo max size failed");
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
            this->_pool_id = AX_INVALID_POOLID;
            this->_layer = layer;       // layer 0 means vedio layer
                                        // layer 1 means osd layer
            err::check_bool_raise(_format == image::FMT_BGRA8888, "Format not support");

            _get_disp_configs(this->_invert_flip, this->_invert_mirror, _max_backlight);

            __sys = std::make_unique<maixcam2::SYS>();
            err::check_bool_raise(__sys != nullptr, "display construct sys failed");
            err::check_bool_raise(__sys->init() == err::ERR_NONE, "display init sys failed");

            __vo = std::make_unique<maixcam2::VO>();
            err::check_bool_raise(__vo != nullptr, "VO init failed");

            maixcam2::ax_vo_param_t vo_param;
            __config_vo_param(&vo_param, width, height, format, rotate);
            err::check_bool_raise(__vo->init(&vo_param) == err::ERR_NONE, "VO init failed");
            _bl_pwm = nullptr;
            int pwm_id = 5;
            _bl_pwm = new pwm::PWM(pwm_id, 10000, 50);
        }

        ~DisplayAx()
        {
            __vo->del_channel(this->_layer, this->_ch);
            __vo->deinit();
            __vo = nullptr;
            __sys = nullptr;

            if (this->_pool_id != (int)AX_INVALID_POOLID) {
                AX_POOL_DestroyPool(this->_pool_id);
                this->_pool_id = AX_INVALID_POOLID;
            }

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
            err::Err ret = err::ERR_NONE;
            width = (width < 0 || width > _max_width) ? _max_width : width;
            height = (height < 0 || height > _max_height) ? _max_height : height;
            if(this->_opened)
            {
                return err::ERR_NONE;
            }

            int ch = __vo->get_unused_channel(this->_layer);
            if (ch < 0) {
                log::error("vo get unused channel failed\n");
                return err::ERR_RUNTIME;
            }

            maixcam2::ax_vo_channel_param_t param ={
                .width = width,
                .height = height,
                .format_in = maixcam2::get_ax_fmt_from_maix(format),
                .format_out = maixcam2::get_ax_fmt_from_maix(image::FMT_YVU420SP),
                .fps = 60,
                .depth = 0,
                .mirror = _invert_mirror,
                .vflip = _invert_flip,
                .fit = 0,
                .rotate = 90,
                .pool_num_in = -1,
                .pool_num_out = -1,
            };
            if (err::ERR_NONE != (ret = __vo->add_channel(this->_layer, ch, &param))) {
                log::error("vo add channel failed, ret:%d", ret);
                return err::ERR_RUNTIME;
            }

            if (this->_layer == 0) {
                AX_POOL_CONFIG_T stPoolCfg = {0};
                stPoolCfg.MetaSize = 512;
                if (width * height * image::fmt_size[format] >= 2560 * 1440 * 3 / 2) {
                    stPoolCfg.BlkCnt = 3;
                } else {
                    stPoolCfg.BlkCnt = 2;
                }
                stPoolCfg.BlkSize = width * height * image::fmt_size[format];
                stPoolCfg.CacheMode = AX_POOL_CACHE_MODE_NONCACHE;
                strcpy((char *)stPoolCfg.PartitionName, "anonymous");
                AX_POOL pool_id = AX_POOL_CreatePool(&stPoolCfg);
                if (pool_id == AX_INVALID_POOLID) {
                    log::info("AX_POOL_CreatePool failed, u32BlkCnt = %d, u64BlkSize = 0x%llx, u64MetaSize = 0x%llx\n", stPoolCfg.BlkCnt,
                        stPoolCfg.BlkSize, stPoolCfg.MetaSize);
                    return err::ERR_RUNTIME;
                }
                this->_pool_id = pool_id;
                this->_pool_size = stPoolCfg.BlkSize;
                this->_pool_cnt = stPoolCfg.BlkCnt;
            }

            this->_ch = ch;
            this->_opened = true;
            return err::ERR_NONE;
        }

        err::Err close()
        {
            if (!this->_opened)
                return err::ERR_NONE;

            __vo->del_channel(this->_layer, this->_ch);

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
            // int mmf_fit = 0;
            // switch (fit) {
            //     case image::Fit::FIT_FILL: mmf_fit = 0; break;
            //     case image::Fit::FIT_CONTAIN: mmf_fit = 1; break;
            //     case image::Fit::FIT_COVER: mmf_fit = 2; break;
            //     default: mmf_fit = 0; break;
            // }

            if (this->_layer == 0) {
                if (img.data_size() > this->_pool_size) {
                    AX_POOL_DestroyPool(this->_pool_id);

                    AX_POOL_CONFIG_T stPoolCfg = {0};
                    stPoolCfg.MetaSize = 512;
                    stPoolCfg.BlkCnt = this->_pool_cnt;
                    stPoolCfg.BlkSize = img.data_size();
                    if (img.data_size() >= 2560 * 1440 * 3 / 2) {
                        stPoolCfg.BlkCnt = 3;
                    } else {
                        stPoolCfg.BlkCnt = 2;
                    }
                    stPoolCfg.CacheMode = AX_POOL_CACHE_MODE_NONCACHE;
                    strcpy((char *)stPoolCfg.PartitionName, "anonymous");
                    AX_POOL pool_id = AX_POOL_CreatePool(&stPoolCfg);
                    if (pool_id == AX_INVALID_POOLID) {
                        log::info("AX_POOL_CreatePool failed, u32BlkCnt = %d, u64BlkSize = 0x%llx, u64MetaSize = 0x%llx\n", stPoolCfg.BlkCnt,
                            stPoolCfg.BlkSize, stPoolCfg.MetaSize);
                        return err::ERR_RUNTIME;
                    }
                    this->_pool_id = pool_id;
                    this->_pool_size = stPoolCfg.BlkSize;
                }

                maixcam2::Frame *frame = new maixcam2::Frame(_pool_id, img.width(), img.height(), img.data(), img.data_size(), maixcam2::get_ax_fmt_from_maix(img.format()));
                if (!frame) {
                    log::error("Failed to create frame");
                    return err::Err(err::ERR_RUNTIME);
                }

                switch (format)
                {
                case image::FMT_GRAYSCALE:  // fall through
                case image::FMT_RGB888:
                case image::FMT_YVU420SP:
                case image::FMT_YUV420SP:
                    if (0 != __vo->push(this->_layer, this->_ch, frame)) {
                        log::error("mmf_vo_frame_push failed\n");
                        delete frame;
                        frame = nullptr;
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
                    if (0 != __vo->push(this->_layer, this->_ch, frame)) {
                        log::error("mmf_vo_frame_push failed\n");
                        delete frame;
                        frame = nullptr;
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
                    if (0 != __vo->push(this->_layer, this->_ch, frame)) {
                        log::error("mmf_vo_frame_push failed\n");
                        delete frame;
                        frame = nullptr;
                        return err::ERR_RUNTIME;
                    }
                    delete rgb;
                    break;
                }
                default:
                    log::error("display layer 0 not support format: %d\n", format);
                    delete frame;
                    frame = nullptr;
                    return err::ERR_ARGS;
                }

                if (frame) {
                    delete frame;
                    frame = nullptr;
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

                maixcam2::Frame *frame = new  maixcam2::Frame(img.width(), img.height(), img.data(), img.data_size(), maixcam2::get_ax_fmt_from_maix(img.format()));
                if (!frame) {
                    log::error("Failed to create frame");
                    return err::Err(err::ERR_RUNTIME);
                }
                if (0 != __vo->push(this->_layer, this->_ch, frame)) {
                    log::error("mmf_vo_frame_push failed\n");
                    delete frame;
                    frame = nullptr;
                    return err::ERR_RUNTIME;
                }

                if (frame) {
                    delete frame;
                    frame = nullptr;
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

        err::Err push(pipeline::Frame *frame, image::Fit fit) {
            if (!frame) return err::ERR_ARGS;
            if (0 != __vo->push(this->_layer, this->_ch, (maixcam2::Frame *)frame->frame())) {
                log::error("mmf_vo_frame_push failed\n");
                delete frame;
                frame = nullptr;
                return err::ERR_RUNTIME;
            }
            return err::ERR_NONE;
        }

        void set_backlight(float value)
        {
            if (_bl_pwm) {
                _bl_pwm->duty(value * _max_backlight / 100.0);
                _bl_pwm->disable();
                if(value == 0)
                    return;
                _bl_pwm->enable();
            }
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
            _invert_mirror = en;

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
            _invert_flip = en;

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
        int _pool_id;
        int _pool_size;
        int _pool_cnt;
        pwm::PWM *_bl_pwm;
    };
}
