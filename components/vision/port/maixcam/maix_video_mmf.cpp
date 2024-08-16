/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include <stdint.h>
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_video.hpp"
#include "sophgo_middleware.hpp"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#define MMF_VENC_CHN            (1)
namespace maix::video
{
#if CONFIG_BUILD_WITH_MAIXPY
    maix::image::Image *Video::NoneImage = new maix::image::Image();
    maix::image::Image *Encoder::NoneImage = new maix::image::Image();
#else
    maix::image::Image *Video::NoneImage = NULL;
    maix::image::Image *Encoder::NoneImage = NULL;
#endif

    Encoder::Encoder(int width, int height, image::Format format, VideoType type, int framerate, int gop, int bitrate, int time_base, bool capture) {
        _width = width;
        _height = height;
        _format = format;
        _type = type;
        _framerate = framerate;
        _gop = gop;
        _bitrate = bitrate;
        _time_base = time_base;
        _need_capture = capture;
        _capture_image = NULL;
        _camera = NULL;
        _bind_camera = NULL;
        _start_encode_ms = 0;
        _encode_started = false;

        switch (_type) {
        case VIDEO_H265_CBR:
        {
            if (0 != mmf_init_v2(true)) {
                err::check_raise(err::ERR_RUNTIME, "init mmf failed!");
            }

            if (0 != mmf_enc_h265_init(MMF_VENC_CHN, _width, _height)) {
                mmf_deinit_v2(false);
                err::check_raise(err::ERR_RUNTIME, "init mmf enc failed!");
            }
            break;
        }
        case VIDEO_H264_CBR:
        {
            mmf_venc_cfg_t cfg = {
                .type = 2,  //1, h265, 2, h264
                .w = _width,
                .h = _height,
                .fmt = mmf_invert_format_to_mmf(_format),
                .jpg_quality = 0,       // unused
                .gop = _gop,
                .intput_fps = _framerate,
                .output_fps = _framerate,
                .bitrate = _bitrate / 1000,
            };

            if (0 != mmf_init_v2(true)) {
                err::check_raise(err::ERR_RUNTIME, "init mmf failed!");
            }

            if (0 != mmf_add_venc_channel_v2(MMF_VENC_CHN, &cfg)) {
                mmf_deinit_v2(false);
                err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!");
            }
            break;
        }
        default:
            std::string err_str = "Encoder not support type: " + std::to_string(_type);
            err::check_raise(err::ERR_RUNTIME, err_str);
        }
    }

    Encoder::~Encoder() {
        switch (_type) {
        case VIDEO_H265_CBR:
        {
            mmf_enc_h265_deinit(MMF_VENC_CHN);
            break;
        }
        case VIDEO_H264_CBR:
        {
            mmf_del_venc_channel(MMF_VENC_CHN);
            break;
        }
        default:
            std::string err_str = "Encoder not support type: " + std::to_string(_type);
            err::check_raise(err::ERR_RUNTIME, err_str);
        }

        if (_capture_image && _capture_image->data()) {
            delete _capture_image;
            _capture_image = nullptr;
        }
    }

    err::Err Encoder::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NONE;
        if (camera->format() != image::Format::FMT_YVU420SP) {
            err::check_raise(err::ERR_RUNTIME, "bind camera failed! support FMT_YVU420SP only!\r\n");
            return err::ERR_RUNTIME;
        }

        this->_camera = camera;
        this->_bind_camera = true;
        return err;
    }

    video::Frame *Encoder::encode(image::Image *img) {
        uint8_t *stream_buffer = NULL;
        int stream_size = 0;

        uint64_t pts = 0, dts = 0;
        uint64_t curr_ms = time::ticks_ms();
        uint64_t diff_ms = 0;
        if (!_encode_started) {
            _encode_started = true;
            _start_encode_ms = curr_ms;
        }
        diff_ms = curr_ms - _start_encode_ms;

        switch (_type) {
        case VIDEO_H264_CBR:
        {
            if (img && img->data() != NULL) {  // encode from image
                if (img->data_size() > 2560 * 1440 * 3 / 2) {
                    log::error("image is too large!\r\n");
                    goto _exit;
                }

                mmf_venc_cfg_t cfg = {0};
                if (0 != mmf_venc_get_cfg(MMF_VENC_CHN, &cfg)) {
                    err::check_raise(err::ERR_RUNTIME, "get venc config failed!\r\n");
                }

                dts = get_dts(diff_ms);
                pts = get_pts(diff_ms);

                int img_w = img->width();
                int img_h = img->height();
                image::Format img_fmt = img->format();
                if (img_w != cfg.w
                    || img->height() != cfg.h
                    || img->format() != mmf_invert_format_to_maix(cfg.fmt)) {
                    log::warn("image size or format is incorrect, try to reinit venc!\r\n");
                    mmf_del_venc_channel(MMF_VENC_CHN);
                    cfg.w = img_w;
                    cfg.h = img_h;
                    cfg.fmt = mmf_invert_format_to_mmf(img_fmt);
                    if (0 != mmf_add_venc_channel_v2(MMF_VENC_CHN, &cfg)) {
                        err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!\r\n");
                    }
                    _width = img_w;
                    _height = img_h;
                    _format = img_fmt;
                }

                if (mmf_venc_push(MMF_VENC_CHN, (uint8_t *)img->data(), img->width(), img->height(), mmf_invert_format_to_mmf(img->format()))) {
                    log::error("mmf_venc_push failed\n");
                    goto _exit;
                }

                mmf_stream_t stream = {0};
                if (mmf_venc_pop(MMF_VENC_CHN, &stream)) {
                    log::error("mmf_enc_h265_pull failed\n");
                    mmf_venc_free(MMF_VENC_CHN);
                    goto _exit;
                }

                for (int i = 0; i < stream.count; i ++) {
                    // printf("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];
                }

                if (stream_size != 0) {
                    stream_buffer = (uint8_t *)malloc(stream_size);
                    if (!stream_buffer) {
                        log::error("malloc failed!\r\n");
                        mmf_venc_free(MMF_VENC_CHN);
                        goto _exit;
                    } else {
                        if (stream.count > 1) {
                            int copy_length = 0;
                            for (int i = 0; i < stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                copy_length += stream.data_size[i];
                            }
                        } else if (stream.count == 1) {
                            memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                        }
                    }
                }

                if (mmf_venc_free(MMF_VENC_CHN)) {
                    printf("mmf_venc_free failed\n");
                    free(stream_buffer);
                    stream_buffer = NULL;
                    goto _exit;
                }
            } else { // encode from camera
                if (!this->_bind_camera) {
                    goto _exit;
                }

                int vi_ch = _camera->get_channel();
                void *data;
                int data_size, width, height, format;
                do {
                    mmf_stream_t stream = {0};
                    if (mmf_venc_pop(MMF_VENC_CHN, &stream)) {
                        log::error("mmf_venc_pop failed\n");
                        mmf_venc_free(MMF_VENC_CHN);
                        mmf_del_venc_channel(MMF_VENC_CHN);
                        goto _exit;
                    }

                    for (int i = 0; i < stream.count; i ++) {
                        stream_size += stream.data_size[i];
                    }

                    if (stream_size != 0) {
                        stream_buffer = (uint8_t *)malloc(stream_size);
                        if (!stream_buffer) {
                            log::error("malloc failed!\r\n");
                            mmf_venc_free(MMF_VENC_CHN);
                            mmf_del_venc_channel(MMF_VENC_CHN);
                            goto _exit;
                        } else {
                            if (stream.count > 1) {
                                int copy_length = 0;
                                for (int i = 0; i < stream.count; i ++) {
                                    memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                    copy_length += stream.data_size[i];
                                }
                            } else if (stream.count == 1) {
                                memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                            }
                        }
                    }

                    if (mmf_venc_free(MMF_VENC_CHN)) {
                        printf("mmf_venc_free failed\n");
                        free(stream_buffer);
                        stream_buffer = NULL;
                        mmf_del_venc_channel(MMF_VENC_CHN);
                        goto _exit;
                    }

                    if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                        log::error("read camera image failed!\r\n");
                        goto _exit;
                    }

                    dts = get_dts(diff_ms);
                    pts = get_pts(diff_ms);

                    if (data_size > 2560 * 1440 * 3 / 2) {
                        log::error("image is too large!\r\n");
                        goto _exit;
                    }

                    if (_need_capture) {
                        if (_capture_image && _capture_image->data()) {
                            delete _capture_image;
                            _capture_image = NULL;
                        }

                        image::Format capture_format = (image::Format)mmf_invert_format_to_maix(format);
                        bool need_align = (width % mmf_vi_aligned_width(vi_ch) == 0) ? false : true;   // Width need align only
                        switch (capture_format) {
                            case image::Format::FMT_BGR888: // fall through
                            case image::Format::FMT_RGB888:
                            {
                                _capture_image = new image::Image(width, height, capture_format);
                                uint8_t * image_data = (uint8_t *)_capture_image->data();
                                if (need_align) {
                                    for (int h = 0; h < height; h++) {
                                        memcpy((uint8_t *)image_data + h * width * 3, (uint8_t *)data + h * width * 3, width * 3);
                                    }
                                } else {
                                    memcpy(image_data, data, width * height * 3);
                                }
                            }
                                break;
                            case image::Format::FMT_YVU420SP:
                            {
                                _capture_image = new image::Image(width, height, capture_format);
                                uint8_t * image_data = (uint8_t *)_capture_image->data();
                                if (need_align) {
                                    for (int h = 0; h < height * 3 / 2; h ++) {
                                        memcpy((uint8_t *)image_data + h * width, (uint8_t *)data + h * width, width);
                                    }
                                } else {
                                    memcpy(image_data, data, width * height * 3 / 2);
                                }
                                break;
                            }
                            default:
                            {
                                _capture_image = NULL;
                                break;
                            }
                        }
                    }

                    mmf_venc_cfg_t cfg = {0};
                    if (0 != mmf_venc_get_cfg(MMF_VENC_CHN, &cfg)) {
                        err::check_raise(err::ERR_RUNTIME, "get venc config failed!\r\n");
                    }

                    int img_w = width;
                    int img_h = height;
                    int mmf_fmt = format;
                    if (img_w != cfg.w
                        || img_h != cfg.h
                        || mmf_fmt != cfg.fmt) {
                        log::warn("image size or format is incorrect, try to reinit venc!\r\n");
                        mmf_del_venc_channel(MMF_VENC_CHN);
                        cfg.w = img_w;
                        cfg.h = img_h;
                        cfg.fmt = mmf_invert_format_to_mmf(mmf_fmt);
                        if (0 != mmf_add_venc_channel_v2(MMF_VENC_CHN, &cfg)) {
                            err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!\r\n");
                        }
                        _width = img_w;
                        _height = img_h;
                        _format = (image::Format)mmf_invert_format_to_maix(mmf_fmt);
                    }

                    if (mmf_venc_push(MMF_VENC_CHN, (uint8_t *)data, width, height, format)) {
                        log::warn("mmf_venc_push failed\n");
                        mmf_del_venc_channel(MMF_VENC_CHN);
                        goto _exit;
                    }

                    mmf_vi_frame_free(vi_ch);
                } while (stream_size == 0);
            }
            break;
        }
        case VIDEO_H265_CBR:
        {
            if (img && img->data() != NULL) {  // encode from image
                if (img->data_size() > 2560 * 1440 * 3 / 2) {
                    log::error("image is too large!\r\n");
                    goto _exit;
                }

                dts = get_dts(diff_ms);
                pts = get_pts(diff_ms);

                if (mmf_enc_h265_push(MMF_VENC_CHN, (uint8_t *)img->data(), img->width(), img->height(), mmf_invert_format_to_mmf(img->format()))) {
                    log::error("mmf_enc_h265_push failed\n");
                    goto _exit;
                }

                mmf_h265_stream_t stream = {0};
                if (mmf_enc_h265_pop(MMF_VENC_CHN, &stream)) {
                    log::error("mmf_enc_h265_pull failed\n");
                    mmf_enc_h265_free(MMF_VENC_CHN);
                    goto _exit;
                }

                for (int i = 0; i < stream.count; i ++) {
                    // printf("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];
                }

                if (stream_size != 0) {
                    stream_buffer = (uint8_t *)malloc(stream_size);
                    if (!stream_buffer) {
                        log::error("malloc failed!\r\n");
                        mmf_enc_h265_free(MMF_VENC_CHN);
                        goto _exit;
                    } else {
                        if (stream.count > 1) {
                            int copy_length = 0;
                            for (int i = 0; i < stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                copy_length += stream.data_size[i];
                            }
                        } else if (stream.count == 1) {
                            memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                        }
                    }
                }

                if (mmf_enc_h265_free(MMF_VENC_CHN)) {
                    printf("mmf_enc_h265_free failed\n");
                    free(stream_buffer);
                    stream_buffer = NULL;
                    goto _exit;
                }
            } else { // encode from camera
                if (!this->_bind_camera) {
                    log::warn("You need use bind_camera() function to bind the camera!\r\n");
                    goto _exit;
                }

                int vi_ch = _camera->get_channel();
                void *data;
                int data_size, width, height, format;

                do {
                    mmf_h265_stream_t stream = {0};
                    if (mmf_enc_h265_pop(MMF_VENC_CHN, &stream)) {
                        log::error("mmf_enc_h265_pop failed\n");
                        mmf_enc_h265_free(MMF_VENC_CHN);
                        mmf_enc_h265_deinit(MMF_VENC_CHN);
                        goto _exit;
                    }

                    for (int i = 0; i < stream.count; i ++) {
                        stream_size += stream.data_size[i];
                    }

                    if (stream_size != 0) {
                        stream_buffer = (uint8_t *)malloc(stream_size);
                        if (!stream_buffer) {
                            log::error("malloc failed!\r\n");
                            mmf_enc_h265_free(MMF_VENC_CHN);
                            mmf_enc_h265_deinit(MMF_VENC_CHN);
                            goto _exit;
                        } else {
                            if (stream.count > 1) {
                                int copy_length = 0;
                                for (int i = 0; i < stream.count; i ++) {
                                    memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                    copy_length += stream.data_size[i];
                                }
                            } else if (stream.count == 1) {
                                memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                            }
                        }
                    }

                    if (mmf_enc_h265_free(MMF_VENC_CHN)) {
                        printf("mmf_enc_h265_free failed\n");
                        free(stream_buffer);
                        stream_buffer = NULL;
                        mmf_enc_h265_deinit(MMF_VENC_CHN);
                        goto _exit;
                    }

                    if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                        log::error("read camera image failed!\r\n");
                        goto _exit;
                    }

                    dts = get_dts(diff_ms);
                    pts = get_pts(diff_ms);

                    if (data_size > 2560 * 1440 * 3 / 2) {
                        log::error("image is too large!\r\n");
                        goto _exit;
                    }

                    if (_need_capture) {
                        if (_capture_image && _capture_image->data()) {
                            delete _capture_image;
                            _capture_image = NULL;
                        }

                        image::Format capture_format = (image::Format)mmf_invert_format_to_maix(format);
                        bool need_align = (width % mmf_vi_aligned_width(vi_ch) == 0) ? false : true;   // Width need align only
                        switch (capture_format) {
                            case image::Format::FMT_BGR888: // fall through
                            case image::Format::FMT_RGB888:
                            {
                                _capture_image = new image::Image(width, height, capture_format);
                                uint8_t * image_data = (uint8_t *)_capture_image->data();
                                if (need_align) {
                                    for (int h = 0; h < height; h++) {
                                        memcpy((uint8_t *)image_data + h * width * 3, (uint8_t *)data + h * width * 3, width * 3);
                                    }
                                } else {
                                    memcpy(image_data, data, width * height * 3);
                                }
                            }
                                break;
                            case image::Format::FMT_YVU420SP:
                            {
                                _capture_image = new image::Image(width, height, capture_format);
                                uint8_t * image_data = (uint8_t *)_capture_image->data();
                                if (need_align) {
                                    for (int h = 0; h < height * 3 / 2; h ++) {
                                        memcpy((uint8_t *)image_data + h * width, (uint8_t *)data + h * width, width);
                                    }
                                } else {
                                    memcpy(image_data, data, width * height * 3 / 2);
                                }
                                break;
                            }
                            default:
                            {
                                _capture_image = NULL;
                                break;
                            }
                        }
                    }

                    if (mmf_enc_h265_push(MMF_VENC_CHN, (uint8_t *)data, width, height, format)) {
                        log::warn("mmf_enc_h265_push failed\n");
                        mmf_enc_h265_deinit(MMF_VENC_CHN);
                        goto _exit;
                    }

                    mmf_vi_frame_free(vi_ch);
                } while (stream_size == 0);
            }
            break;
        }
        default:
            std::string err_str = "Encoder not support type: " + std::to_string(_type);
            err::check_raise(err::ERR_RUNTIME, err_str);
        }
_exit:
        video::Frame *frame = new video::Frame(stream_buffer, stream_size, pts, dts, 0, true, false);
        return frame;
    }

    typedef struct {
        AVFormatContext *pFormatContext;
        AVPacket *pPacket;
        int video_stream_index;

        int vdec_ch;
    } decoder_param_t;

    static PAYLOAD_TYPE_E _get_entype(const char *filename) {
        PAYLOAD_TYPE_E enType;
        const char *suffix = strrchr(filename, '.');
        err::check_null_raise((void *)suffix, "Try a file format with a suffix, e.g. video.h264");

        if (!strcmp(suffix, ".h264")) {
            enType = PT_H264;
        } else {
            err::check_raise(err::ERR_RUNTIME, "Currently only support h264 video format!");
        }
        return enType;
    }

    Decoder::Decoder(std::string path, image::Format format) {
        err::check_bool_raise(format == image::Format::FMT_YVU420SP, "Decoder only support FMT_YVU420SP format!");
        _path = path;
        _format_out = format;
        _param = (decoder_param_t *)malloc(sizeof(decoder_param_t));
        err::check_null_raise(_param, "malloc failed!");
        AVFormatContext *pFormatContext = avformat_alloc_context();
        err::check_null_raise(pFormatContext, "malloc failed!");
        err::check_bool_raise(!avformat_open_input(&pFormatContext, _path.c_str(), NULL, NULL), "Could not open file");
        pFormatContext->max_analyze_duration = 5000;    // reduce analyze time
        err::check_bool_raise(!avformat_find_stream_info(pFormatContext, NULL), "Could not find stream information");
        int video_stream_index = -1;
        for (int i = 0; i < (int)pFormatContext->nb_streams; i++) {
            if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_stream_index = i;
                break;
            }
        }
        err::check_bool_raise(video_stream_index != -1, "Could not find video stream");
        AVCodecParameters *codecpar = pFormatContext->streams[video_stream_index]->codecpar;
        _width = codecpar->width;
        _height = codecpar->height;
        AVStream *video_stream = pFormatContext->streams[video_stream_index];
        AVRational frame_rate = av_guess_frame_rate(pFormatContext, video_stream, NULL);
        _fps = av_q2d(frame_rate);
        if (_width % 32 != 0) {
            log::error("Width need align to 32, current width: %d", _width);
            avformat_close_input(&pFormatContext);
            free(_param);
            err::check_raise(err::ERR_RUNTIME, "Width need align to 32");
        }
        AVPacket *pPacket = av_packet_alloc();
        err::check_null_raise(pPacket, "malloc failed!");

        int ch = mmf_vdec_unused_channel();
        err::check_bool_raise(ch >= 0, "No unused channel of vdec");
        VDEC_CHN_ATTR_S vdec_chn_attr;
        vdec_chn_attr.enType = _get_entype(_path.c_str());
        vdec_chn_attr.enMode = VIDEO_MODE_FRAME;
        vdec_chn_attr.u32PicWidth = _width;
        vdec_chn_attr.u32PicHeight = _height;
        vdec_chn_attr.u32FrameBufCnt = 3;
        vdec_chn_attr.u32StreamBufSize = _width * _height;
        err::check_bool_raise(!mmf_add_vdec_channel_v2(ch, mmf_invert_format_to_mmf(format), 4, &vdec_chn_attr), "mmf_add_vdec_channel_v2 failed");

        decoder_param_t *param = (decoder_param_t *)_param;
        param->pFormatContext = pFormatContext;
        param->pPacket = pPacket;
        param->video_stream_index = video_stream_index;
        param->vdec_ch = ch;
    }

    Decoder::~Decoder() {
        decoder_param_t *param = (decoder_param_t *)_param;
        if (param) {
            if (param->vdec_ch >= 0) {
                mmf_del_vdec_channel(param->vdec_ch);
            }

            av_packet_free(&param->pPacket);
            avformat_close_input(&param->pFormatContext);
            free(_param);
            _param = NULL;
        }
        avformat_network_deinit();
    }

    static image::Image *_mmf_frame_to_image(VIDEO_FRAME_INFO_S *frame, image::Format format_out)
    {
        int width = frame->stVFrame.u32Width;
        int height = frame->stVFrame.u32Height;
        image::Format format = (image::Format )mmf_invert_format_to_maix(frame->stVFrame.enPixelFormat);
        image::Image *img = new image::Image(width, height, format_out);
        err::check_null_raise(img, "new image failed");
        uint8_t *buffer = (uint8_t *)img->data();
        switch (img->format()) {
        case image::Format::FMT_GRAYSCALE:
            if (format != image::Format::FMT_YVU420SP) {
                log::error("camera read: format not support, need %d, but %d", image::Format::FMT_YVU420SP, format);
                goto _error;
            }
            memcpy(buffer, frame->stVFrame.pu8VirAddr[0], width * height);
            break;
        case image::Format::FMT_YVU420SP:
            if (format != img->format()) {
                log::error("camera read: format not support, need %d, but %d", img->format(), format);
                goto _error;
            }
            memcpy(buffer, frame->stVFrame.pu8VirAddr[0], width * height);
            memcpy(buffer + width * height, frame->stVFrame.pu8VirAddr[1], width * height / 2);
            break;
        default:
            log::error("Read failed, unknown format:%d", img->format());
            delete img;
            err::check_raise(err::ERR_RUNTIME, "Invert frame failed, unknown format");
        }

        return img;
_error:
        if (img) delete img;
        err::check_raise(err::ERR_RUNTIME, "Invert frame failed");
    }

    image::Image *Decoder::decode_video() {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVPacket *pPacket = param->pPacket;
        AVFormatContext *pFormatContext = param->pFormatContext;
        int video_stream_index = param->video_stream_index;
        image::Image *img = NULL;
        while (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == video_stream_index) {
                err::check_bool_raise(!mmf_vdec_push(param->vdec_ch, pPacket->data, pPacket->size, 0, 0));
                VIDEO_FRAME_INFO_S frame;
                err::check_bool_raise(!mmf_vdec_pop_v2(param->vdec_ch, &frame));
                img = _mmf_frame_to_image(&frame, _format_out);
                err::check_bool_raise(!mmf_vdec_free(param->vdec_ch));
                break;
            }
            av_packet_unref(pPacket);
        }
        return img;
    }

    err::Err Decoder::seek(uint64_t timestamp) {
        err::check_raise(err::ERR_NOT_IMPL, "seek not impl");
        decoder_param_t *param = (decoder_param_t *)_param;
        AVFormatContext *pFormatContext = param->pFormatContext;
        int video_stream_index = param->video_stream_index;
        int ret = av_seek_frame(pFormatContext, video_stream_index, 0, AVSEEK_FLAG_FRAME);
        if (ret < 0) {
            avformat_close_input(&param->pFormatContext);
            log::error("av_seek_frame failed, ret:%d", ret);
            return err::ERR_RUNTIME;
        }

        return err::ERR_NONE;
    }

    Video::Video(std::string path, int width, int height, image::Format format, int time_base, int framerate, bool capture, bool open)
    {
        this->_pre_path = path;
        this->_video_type = VIDEO_NONE;
        this->_bind_camera = false;
        this->_is_recording = false;
        this->_camera = NULL;
        this->_fd = -1;
        this->_time_base = time_base;
        this->_framerate = framerate;
        this->_need_auto_config = true;
        this->_pre_width = width;
        this->_pre_height = height;
        this->_last_pts = 0;
        this->_capture_image = nullptr;
        this->_need_capture = capture;
        this->_is_opened = false;

        if (open) {
            err::check_bool_raise(err::ERR_NONE == this->open(), "Video open failed!\r\n");
        }
    }

    Video::~Video() {
        if (this->_is_opened) {
            this->close();
        }
    }

    err::Err Video::open(std::string path, double fps)
    {
        if (this->_is_opened) {
            return err::ERR_NONE;
        }

        if (path == std::string()) {
            this->_path = this->_pre_path;
        } else {
            this->_path = path;
        }

        if (fps == 30.0) {
            this->_fps = this->_pre_fps;
        } else {
            this->_fps = fps;
        }

        if (0 != mmf_enc_h265_init(MMF_VENC_CHN, _pre_width, _pre_height)) {
            return err::ERR_RUNTIME;
        }

        this->_is_opened = true;
        return err::ERR_NONE;
    }

    void Video::close()
    {
        if (this->_is_opened) {
            mmf_enc_h265_deinit(MMF_VENC_CHN);
        }

        if (_capture_image && _capture_image->data()) {
            delete _capture_image;
            _capture_image = nullptr;
        }
        this->_is_opened = false;
    }

    err::Err Video::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NONE;

        if (camera->format() != image::Format::FMT_YVU420SP) {
            err::check_raise(err::ERR_RUNTIME, "bind camera failed! support FMT_YVU420SP only!\r\n");
            return err::ERR_RUNTIME;
        }

        this->_camera = camera;
        this->_bind_camera = true;
        return err;
    }

    static video::VideoType get_video_type(std::string &path, bool encode)
    {
        video::VideoType video_type;
        std::string ext;
        size_t pos = path.rfind('.');
        if (pos != std::string::npos) {
            ext = path.substr(pos);
        }

        if (ext.find(".h265") != std::string::npos) {
            if (encode) {
                video_type = VIDEO_ENC_H265_CBR;
            } else {
                video_type = VIDEO_DEC_H265_CBR;
            }
        } else if (ext.find(".mp4") != std::string::npos) {
            if (encode) {
                video_type = VIDEO_ENC_MP4_CBR;
            } else {
                video_type = VIDEO_DEC_MP4_CBR;
            }
        } else {
            log::error("Video not support %s!\r\n", ext.c_str());
            video_type = VIDEO_NONE;
        }

        return video_type;
    }

    video::Packet *Video::encode(image::Image *img) {
        uint8_t *stream_buffer = NULL;
        int stream_size = 0;

        if (_need_auto_config) {
            _video_type = get_video_type(_path, true);
            err::check_bool_raise(_video_type != VIDEO_NONE, "Can't parse video type!");
            if (_video_type == VIDEO_ENC_MP4_CBR) {
                system("rm _encode_video_tmp.h265 &> /dev/zero");
                _tmp_path = "_encode_video_tmp.h265";
            }
            _need_auto_config = false;
        }

        if (img && img->data() != NULL) {  // encode from image
            if (img->data_size() > 2560 * 1440 * 3 / 2) {
                log::error("image is too large!\r\n");
                goto _exit;
            }

            switch (_video_type) {
            case VIDEO_ENC_H265_CBR:
            {
                if (mmf_enc_h265_push(MMF_VENC_CHN, (uint8_t *)img->data(), img->width(), img->height(), mmf_invert_format_to_mmf(img->format()))) {
                    log::error("mmf_enc_h265_push failed\n");
                    goto _exit;
                }

                mmf_h265_stream_t stream = {0};
                if (mmf_enc_h265_pop(MMF_VENC_CHN, &stream)) {
                    log::error("mmf_enc_h265_pull failed\n");
                    mmf_enc_h265_free(MMF_VENC_CHN);
                    goto _exit;
                }

                for (int i = 0; i < stream.count; i ++) {
                    // printf("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];
                }

                if (stream_size != 0) {
                    stream_buffer = (uint8_t *)malloc(stream_size);
                    if (!stream_buffer) {
                        log::error("malloc failed!\r\n");
                        mmf_enc_h265_free(MMF_VENC_CHN);
                        goto _exit;
                    } else {
                        if (stream.count > 1) {
                            int copy_length = 0;
                            for (int i = 0; i < stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                copy_length += stream.data_size[i];
                            }
                        } else if (stream.count == 1) {
                            memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                        }
                    }
                }

                if (mmf_enc_h265_free(MMF_VENC_CHN)) {
                    printf("mmf_enc_h265_free failed\n");
                    free(stream_buffer);
                    stream_buffer = NULL;
                    goto _exit;
                }

                if (_path.size() > 0) {
                    if (_fd < 0) {
                        _fd = ::open((char *)_path.c_str(), O_WRONLY | O_CREAT, 0777);
                        if (_fd < 0) {
                            log::error("Open %s failed!\r\n", (char *)_path.c_str());
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }

                    if (_fd > 2) {
                        int res = 0;
                        if ((res = write(_fd, stream_buffer, stream_size)) < 0) {
                            log::error("Write failed, res = %d\r\n", res);
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }
                }
            }
            break;
            case VIDEO_ENC_MP4_CBR:
            {
                if (mmf_enc_h265_push(MMF_VENC_CHN, (uint8_t *)img->data(), img->width(), img->height(), mmf_invert_format_to_mmf(img->format()))) {
                    log::error("mmf_enc_h265_push failed\n");
                    goto _exit;
                }

                mmf_h265_stream_t stream = {0};
                if (mmf_enc_h265_pop(MMF_VENC_CHN, &stream)) {
                    log::error("mmf_enc_h265_pull failed\n");
                    mmf_enc_h265_free(MMF_VENC_CHN);
                    goto _exit;
                }

                for (int i = 0; i < stream.count; i ++) {
                    // printf("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];
                }

                if (stream_size != 0) {
                    stream_buffer = (uint8_t *)malloc(stream_size);
                    if (!stream_buffer) {
                        log::error("malloc failed!\r\n");
                        mmf_enc_h265_free(MMF_VENC_CHN);
                        goto _exit;
                    } else {
                        if (stream.count > 1) {
                            int copy_length = 0;
                            for (int i = 0; i < stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                copy_length += stream.data_size[i];
                            }
                        } else if (stream.count == 1) {
                            memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                        }
                    }
                }

                if (mmf_enc_h265_free(MMF_VENC_CHN)) {
                    printf("mmf_enc_h265_free failed\n");
                    free(stream_buffer);
                    stream_buffer = NULL;
                    goto _exit;
                }

                if (_tmp_path.size() > 0) {
                    if (_fd < 0) {
                        _fd = ::open((char *)_tmp_path.c_str(), O_WRONLY | O_CREAT, 0777);
                        if (_fd < 0) {
                            log::error("Open %s failed!\r\n", (char *)_tmp_path.c_str());
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }

                    if (_fd > 2) {
                        int res = 0;
                        if ((res = write(_fd, stream_buffer, stream_size)) < 0) {
                            log::error("Write failed, res = %d\r\n", res);
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }
                }
            }
            break;
            default:err::check_raise(err::ERR_RUNTIME, "Unknown video type");
            }
        } else { // encode from camera
            if (!this->_bind_camera) {
                goto _exit;
            }

            int vi_ch = _camera->get_channel();
            void *data;
            int data_size, width, height, format;
            switch (_video_type) {
            case VIDEO_ENC_H265_CBR:
            {
_retry_enc_h265:
                mmf_h265_stream_t stream = {0};
                if (mmf_enc_h265_pop(MMF_VENC_CHN, &stream)) {
                    log::error("mmf_enc_h265_pop failed\n");
                    mmf_enc_h265_free(MMF_VENC_CHN);
                    mmf_enc_h265_deinit(MMF_VENC_CHN);
                    goto _exit;
                }

                for (int i = 0; i < stream.count; i ++) {
                    stream_size += stream.data_size[i];
                }

                if (stream_size != 0) {
                    stream_buffer = (uint8_t *)malloc(stream_size);
                    if (!stream_buffer) {
                        log::error("malloc failed!\r\n");
                        mmf_enc_h265_free(MMF_VENC_CHN);
                        mmf_enc_h265_deinit(MMF_VENC_CHN);
                        goto _exit;
                    } else {
                        if (stream.count > 1) {
                            int copy_length = 0;
                            for (int i = 0; i < stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                copy_length += stream.data_size[i];
                            }
                        } else if (stream.count == 1) {
                            memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                        }
                    }
                }

                if (mmf_enc_h265_free(MMF_VENC_CHN)) {
                    printf("mmf_enc_h265_free failed\n");
                    free(stream_buffer);
                    stream_buffer = NULL;
                    mmf_enc_h265_deinit(MMF_VENC_CHN);
                    goto _exit;
                }

                if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                    log::error("read camera image failed!\r\n");
                    goto _exit;
                }

                if (data_size > 2560 * 1440 * 3 / 2) {
                    log::error("image is too large!\r\n");
                    goto _exit;
                }

                if (_need_capture) {
                    if (_capture_image && _capture_image->data()) {
                        delete _capture_image;
                        _capture_image = NULL;
                    }

                    image::Format capture_format = (image::Format)mmf_invert_format_to_maix(format);
                    bool need_align = (width % mmf_vi_aligned_width(vi_ch) == 0) ? false : true;   // Width need align only
                    switch (capture_format) {
                        case image::Format::FMT_BGR888: // fall through
                        case image::Format::FMT_RGB888:
                        {
                            _capture_image = new image::Image(width, height, capture_format);
                            uint8_t * image_data = (uint8_t *)_capture_image->data();
                            if (need_align) {
                                for (int h = 0; h < height; h++) {
                                    memcpy((uint8_t *)image_data + h * width * 3, (uint8_t *)data + h * width * 3, width * 3);
                                }
                            } else {
                                memcpy(image_data, data, width * height * 3);
                            }
                        }
                            break;
                        case image::Format::FMT_YVU420SP:
                        {
                            _capture_image = new image::Image(width, height, capture_format);
                            uint8_t * image_data = (uint8_t *)_capture_image->data();
                            if (need_align) {
                                for (int h = 0; h < height * 3 / 2; h ++) {
                                    memcpy((uint8_t *)image_data + h * width, (uint8_t *)data + h * width, width);
                                }
                            } else {
                                memcpy(image_data, data, width * height * 3 / 2);
                            }
                            break;
                        }
                        default:
                        {
                            _capture_image = NULL;
                            break;
                        }
                    }
                }

                if (mmf_enc_h265_push(MMF_VENC_CHN, (uint8_t *)data, width, height, format)) {
                    log::warn("mmf_enc_h265_push failed\n");
                    mmf_enc_h265_deinit(MMF_VENC_CHN);
                    goto _exit;
                }

                mmf_vi_frame_free(vi_ch);

                if (stream_size == 0) {
                    goto _retry_enc_h265;
                }

                if (_path.size() > 0) {
                    if (_fd < 0) {
                        _fd = ::open((char *)_path.c_str(), O_WRONLY | O_CREAT, 0777);
                        if (_fd < 0) {
                            log::error("Open %s failed!\r\n", (char *)_path.c_str());
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }

                    if (_fd > 2) {
                        int res = 0;
                        if ((res = write(_fd, stream_buffer, stream_size)) < 0) {
                            log::error("Write failed, res = %d\r\n", res);
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }
                }
            }
            break;
            case VIDEO_ENC_MP4_CBR:
            {
_retry_enc_mp4:
                mmf_h265_stream_t stream = {0};
                if (mmf_enc_h265_pop(MMF_VENC_CHN, &stream)) {
                    log::error("mmf_enc_h265_pop failed\n");
                    mmf_enc_h265_free(MMF_VENC_CHN);
                    mmf_enc_h265_deinit(MMF_VENC_CHN);
                    goto _exit;
                }

                for (int i = 0; i < stream.count; i ++) {
                    stream_size += stream.data_size[i];
                }

                if (stream_size != 0) {
                    stream_buffer = (uint8_t *)malloc(stream_size);
                    if (!stream_buffer) {
                        log::error("malloc failed!\r\n");
                        mmf_enc_h265_free(MMF_VENC_CHN);
                        mmf_enc_h265_deinit(MMF_VENC_CHN);
                        goto _exit;
                    } else {
                        if (stream.count > 1) {
                            int copy_length = 0;
                            for (int i = 0; i < stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                copy_length += stream.data_size[i];
                            }
                        } else if (stream.count == 1) {
                            memcpy(stream_buffer, stream.data[0], stream.data_size[0]);
                        }
                    }
                }

                if (mmf_enc_h265_free(MMF_VENC_CHN)) {
                    printf("mmf_enc_h265_free failed\n");
                    free(stream_buffer);
                    stream_buffer = NULL;
                    mmf_enc_h265_deinit(MMF_VENC_CHN);
                    goto _exit;
                }

                if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                    log::error("read camera image failed!\r\n");
                    goto _exit;
                }

                if (data_size > 2560 * 1440 * 3 / 2) {
                    log::error("image is too large!\r\n");
                    goto _exit;
                }

                if (mmf_enc_h265_push(MMF_VENC_CHN, (uint8_t *)data, width, height, format)) {
                    log::warn("mmf_enc_h265_push failed\n");
                    mmf_enc_h265_deinit(MMF_VENC_CHN);
                    goto _exit;
                }

                mmf_vi_frame_free(vi_ch);

                if (stream_size == 0) {
                    goto _retry_enc_mp4;
                }

                if (_tmp_path.size() > 0) {
                    if (_fd < 0) {
                        _fd = ::open((char *)_tmp_path.c_str(), O_WRONLY | O_CREAT, 0777);
                        if (_fd < 0) {
                            log::error("Open %s failed!\r\n", (char *)_tmp_path.c_str());
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }

                    if (_fd > 2) {
                        int res = 0;
                        if ((res = write(_fd, stream_buffer, stream_size)) < 0) {
                            log::error("Write failed, res = %d\r\n", res);
                            free(stream_buffer);
                            stream_buffer = NULL;
                            goto _exit;
                        }
                    }
                }
            }
            break;
            default:err::check_raise(err::ERR_RUNTIME, "Unknown video type");
            }
        }
_exit:
        video::Packet *packet = new video::Packet(stream_buffer, stream_size);
        return packet;
    }

    image::Image *Video::decode(video::Frame *frame) {
        return NULL;
    }

    err::Err Video::finish() {
        if (this->_fd > 2) {
            ::close(this->_fd);

            switch (this->_video_type) {
            case VIDEO_ENC_H265_CBR:
                // do nothing
                break;
            case VIDEO_ENC_MP4_CBR:
            {
                char cmd[128];
                snprintf(cmd, sizeof(cmd), "ffmpeg -loglevel quiet -i %s -c:v copy -c:a copy %s -y", this->_tmp_path.c_str(), this->_path.c_str());
                system(cmd);

                snprintf(cmd, sizeof(cmd), "rm %s", this->_tmp_path.c_str());
                system(cmd);
            }
                break;
            default:
                log::error("%s not support\r\n", this->_video_type);
            }
            system("sync");
        }
        return err::ERR_NONE;
    }
} // namespace maix::video
