/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

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
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define MMF_VENC_CHN            (1)
namespace maix::video
{
#if CONFIG_BUILD_WITH_MAIXPY
    maix::image::Image *Video::NoneImage = new maix::image::Image();
    maix::image::Image *Encoder::NoneImage = new maix::image::Image();
    maix::Bytes *Encoder::NoneBytes = new maix::Bytes();
#else
    maix::image::Image *Video::NoneImage = NULL;
    maix::image::Image *Encoder::NoneImage = NULL;
    maix::Bytes *Encoder::NoneBytes = NULL;
#endif
    static void custom_log_callback(void* ptr, int level, const char* fmt, va_list vargs) {
        if (level > AV_LOG_ERROR) {
            return;
        }
    }

    static video::VideoType _get_video_type(const char *filename, video::VideoType type) {
        video::VideoType video_type = type;
        const char *suffix = strrchr(filename, '.');
        if (!suffix) {
            return type;
        }

        if (!strcmp(suffix, ".h264")) {
            video_type = video::VIDEO_H264;
        } else if (!strcmp(suffix, ".h265")) {
            video_type = video::VIDEO_H264;
        } else if (!strcmp(suffix, ".mp4")) {
            switch (type) {
            case video::VIDEO_H264:         // fall through
            case video::VIDEO_H264_MP4:     // fall through
            case video::VIDEO_H264_FLV:
                video_type = video::VIDEO_H264_MP4;
                break;
            case video::VIDEO_H265:         // fall through
            case video::VIDEO_H265_MP4:
                video_type = video::VIDEO_H265_MP4;
                break;
            default:
                err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
                break;
            }
        } else if (!strcmp(suffix, ".flv")) {
            switch (type) {
            case video::VIDEO_H264:         // fall through
            case video::VIDEO_H264_FLV:
                video_type = video::VIDEO_H264_FLV;
                break;
            default:
                err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
                break;
            }
        } else {
            video_type = type;
        }
        return video_type;
    }

    static PAYLOAD_TYPE_E _video_type_to_mmf(VideoType video_type) {
        PAYLOAD_TYPE_E payload_type = PT_H264;
        switch (video_type) {
            case VIDEO_H264:
            case VIDEO_H264_MP4:
            case VIDEO_H264_FLV:
                payload_type = PT_H264;
                break;
            case VIDEO_H265:
            case VIDEO_H265_MP4:
                payload_type = PT_H265;
                break;
            default:
                err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
        }
        return payload_type;
    }

    static enum AVCodecID _video_type_to_ffmpeg(VideoType video_type) {
        enum AVCodecID codec_id = AV_CODEC_ID_NONE;
        switch (video_type) {
            case VIDEO_H264:
            case VIDEO_H264_MP4:
            case VIDEO_H264_FLV:
                codec_id = AV_CODEC_ID_H264;
               break;
            case VIDEO_H265:
            case VIDEO_H265_MP4:
                codec_id = AV_CODEC_ID_HEVC;
                break;
            default:
                err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
        }
        return codec_id;
    }

    static enum AVPixelFormat _image_format_to_ffmpeg(image::Format format) {
        enum AVPixelFormat codec_fmt = AV_PIX_FMT_NONE;
        switch (format) {
            case image::Format::FMT_YVU420SP:
                codec_fmt = AV_PIX_FMT_NV21;
                break;
            default:
                err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
        }
        return codec_fmt;
    }

    static image::Image* _image_from_mmf_vi(int ch, void *data, int width, int height, int format) {
        image::Format capture_format = (image::Format)mmf_invert_format_to_maix(format);
        bool need_align = (width % mmf_vi_aligned_width(ch) == 0) ? false : true;   // Width need align only
        image::Image *out = NULL;
        switch (capture_format) {
            case image::Format::FMT_BGR888: // fall through
            case image::Format::FMT_RGB888:
            {
                out = new image::Image(width, height, capture_format);
                err::check_null_raise(out, "Failed to create image!");
                uint8_t * image_data = (uint8_t *)out->data();
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
                out = new image::Image(width, height, capture_format);
                err::check_null_raise(out, "Failed to create image!");
                uint8_t * image_data = (uint8_t *)out->data();
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
                out = NULL;
                break;
            }
        }
        return out;
    }

    double timebase_to_us(std::vector<int> timebase, uint64_t value) {
        return value * 1000000 / ((double)timebase[1] / timebase[0]);
    }

    double timebase_to_ms(std::vector<int> timebase, uint64_t value) {
        return value * 1000 / ((double)timebase[1] / timebase[0]);
    }

    uint64_t ms_to_pts(std::vector<int> timebase, uint64_t ms) {
        return ms * ((double)timebase[1] / timebase[0]) / 1000;
    }

    typedef struct {
        AVFormatContext *outputFormatContext;
        AVStream *outputStream;
        AVPacket *pPacket;
        bool find_sps_pps;
        bool copy_sps_pps_per_iframe;
        uint64_t frame_index;
        uint64_t video_frame_last_ms;
        uint64_t last_encode_ms;
        AVPacket *video_last_packet;
        std::list<AVPacket *> *video_packet_list;
        video::VideoType video_type;
        int venc_ch;
        PAYLOAD_TYPE_E venc_type;

        AVStream *audio_stream = NULL;
        AVCodecContext *audio_codec_ctx = NULL;
        AVCodec *audio_codec = NULL;
        AVFrame *audio_frame = NULL;
        SwrContext *swr_ctx = NULL;
        AVPacket *audio_packet = NULL;
        uint64_t audio_last_pts = 0;
        std::list<Bytes *> *pcm_list;
        int audio_sample_rate;
        int audio_channels;
        int audio_bitrate;
        enum AVSampleFormat audio_format;
    } encoder_param_t;

    Encoder::Encoder(std::string path, int width, int height, image::Format format, VideoType type, int framerate, int gop, int bitrate, int time_base, bool capture, bool block) {
        _path = path;
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
        _block = block;

        err::check_bool_raise(format == image::Format::FMT_YVU420SP, "Encoder only support FMT_YVU420SP format!");
        video::VideoType video_type = _get_video_type(path.c_str(), type);
        PAYLOAD_TYPE_E venc_type = _video_type_to_mmf(type);

        if (_path.size() == 0) {
            switch (_type) {
            case VIDEO_H264:
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
            case VIDEO_H265:
            {
                mmf_venc_cfg_t cfg = {
                    .type = 1,  //1, h265, 2, h264
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
        } else {
            // ffmpeg init
            av_log_set_callback(custom_log_callback);
            _param = (encoder_param_t *)malloc(sizeof(encoder_param_t));
            err::check_null_raise(_param, "malloc failed!");
            memset(_param, 0, sizeof(encoder_param_t));

            AVFormatContext *outputFormatContext = NULL;
            if (0 != avformat_alloc_output_context2(&outputFormatContext, NULL, NULL, _path.c_str())) {
                log::error("Count not open file: %s", _path.c_str());
                err::check_raise(err::ERR_RUNTIME, "Could not open file");
            }

            /* video init */
            AVStream *outputStream = avformat_new_stream(outputFormatContext, NULL);
            err::check_null_raise(outputStream, "create new stream failed");

            outputStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
            outputStream->codecpar->codec_id = _video_type_to_ffmpeg(video_type);
            outputStream->codecpar->width = _width;
            outputStream->codecpar->height = _height;
            outputStream->codecpar->format = _image_format_to_ffmpeg(_format);
            outputStream->time_base = (AVRational){1, _framerate};
            outputStream->codecpar->bit_rate = _bitrate;

            if (!(outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
                if (avio_open(&outputFormatContext->pb, _path.c_str(), AVIO_FLAG_WRITE) < 0) {
                    log::error("Count not open file: %s", _path.c_str());
                    err::check_raise(err::ERR_RUNTIME, "Could not open file");
                }
            }

            AVPacket *pPacket = av_packet_alloc();
            err::check_null_raise(pPacket, "malloc failed!");

            // mmf venc init
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
            if (venc_type == PT_H265) {
                cfg.type = 1;
            } else if (venc_type == PT_H264) {
                cfg.type = 2;
            }

            if (0 != mmf_init_v2(true)) {
                err::check_raise(err::ERR_RUNTIME, "init mmf failed!");
            }

            if (0 != mmf_add_venc_channel_v2(MMF_VENC_CHN, &cfg)) {
                mmf_deinit_v2(false);
                err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!");
            }

            /* audio init */
            AVStream *audio_stream = NULL;
            AVCodecContext *audio_codec_ctx = NULL;
            AVCodec *audio_codec = NULL;
            AVFrame *audio_frame = NULL;
            SwrContext *swr_ctx = NULL;
            AVPacket *audio_packet = NULL;

            audio_packet = av_packet_alloc();
            err::check_null_raise(audio_packet, "av_packet_alloc");
            audio_packet->data = NULL;
            audio_packet->size = 0;

            int sample_rate = 48000;
            int channels = 1;
            int bitrate = 128000;
            enum AVSampleFormat format = AV_SAMPLE_FMT_S16;
            err::check_null_raise(audio_codec = avcodec_find_encoder(AV_CODEC_ID_AAC), "Could not find aac encoder");
            err::check_null_raise(audio_stream = avformat_new_stream(outputFormatContext, NULL), "Could not allocate stream");
            err::check_null_raise(audio_codec_ctx = avcodec_alloc_context3(audio_codec), "Could not allocate audio codec context");
            audio_codec_ctx->codec_id = AV_CODEC_ID_AAC;
            audio_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
            audio_codec_ctx->sample_rate = sample_rate;
            audio_codec_ctx->channels = channels;
            audio_codec_ctx->channel_layout = av_get_default_channel_layout(audio_codec_ctx->channels);
            audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;  // AAC编码需要浮点格式
            audio_codec_ctx->time_base = (AVRational){1, sample_rate};
            audio_codec_ctx->bit_rate = bitrate;
            audio_stream->time_base = audio_codec_ctx->time_base;
            err::check_bool_raise(avcodec_parameters_from_context(audio_stream->codecpar, audio_codec_ctx) >= 0, "avcodec_parameters_to_context");
            err::check_bool_raise(avcodec_open2(audio_codec_ctx, audio_codec, NULL) >= 0, "audio_codec open failed");

            swr_ctx = swr_alloc();
            av_opt_set_int(swr_ctx, "in_channel_layout", audio_codec_ctx->channel_layout, 0);
            av_opt_set_int(swr_ctx, "out_channel_layout", audio_codec_ctx->channel_layout, 0);
            av_opt_set_int(swr_ctx, "in_sample_rate", audio_codec_ctx->sample_rate, 0);
            av_opt_set_int(swr_ctx, "out_sample_rate", audio_codec_ctx->sample_rate, 0);
            av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", format, 0);
            av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
            swr_init(swr_ctx);

            int frame_size = audio_codec_ctx->frame_size;
            audio_frame = av_frame_alloc();
            audio_frame->nb_samples = frame_size;
            audio_frame->channel_layout = audio_codec_ctx->channel_layout;
            audio_frame->format = AV_SAMPLE_FMT_FLTP;
            audio_frame->sample_rate = audio_codec_ctx->sample_rate;
            av_frame_get_buffer(audio_frame, 0);

            err::check_bool_raise(avformat_write_header(outputFormatContext, NULL) >= 0, "avformat_write_header failed!");

            /* param init */
            encoder_param_t *param = (encoder_param_t *)_param;
            param->outputFormatContext = outputFormatContext;
            param->outputStream = outputStream;

            // video init
            param->pPacket = pPacket;
            param->video_type = video_type;
            param->venc_ch = MMF_VENC_CHN;
            param->venc_type = venc_type;
            param->find_sps_pps = false;
            param->frame_index = 0;
            param->last_encode_ms = time::ticks_ms();
            param->video_packet_list = new std::list<AVPacket *>;
            switch (video_type) {
                case VIDEO_H264:
                    param->copy_sps_pps_per_iframe = true;
                    break;
                case VIDEO_H264_MP4:
                    param->copy_sps_pps_per_iframe = false;
                    break;
                case VIDEO_H264_FLV:
                    param->copy_sps_pps_per_iframe = false;
                break;
                case VIDEO_H265:
                    param->copy_sps_pps_per_iframe = true;
                    break;
                case VIDEO_H265_MP4:
                    param->copy_sps_pps_per_iframe = false;
                    break;
                default:
                    err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
            }

            // audio init
            param->audio_stream = audio_stream;
            param->audio_codec_ctx = audio_codec_ctx;
            param->audio_codec = audio_codec;
            param->audio_frame = audio_frame;
            param->swr_ctx = swr_ctx;
            param->audio_packet = audio_packet;
            param->audio_last_pts = 0;
            param->pcm_list = new std::list<Bytes *>;
            param->audio_sample_rate = 48000;
            param->audio_channels = 1;
            param->audio_bitrate = 128000;
            param->audio_format = AV_SAMPLE_FMT_S16;
        }
    }

    Encoder::~Encoder() {
        if (_path.size() == 0) {
            switch (_type) {
            case VIDEO_H264:
            {
                mmf_del_venc_channel(MMF_VENC_CHN);
                mmf_deinit_v2(false);
                break;
            }
            case VIDEO_H265:
            {
                mmf_del_venc_channel(MMF_VENC_CHN);
                mmf_deinit_v2(false);
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
        } else {
            encoder_param_t *param = (encoder_param_t *)_param;
            if (param) {
                mmf_del_venc_channel(MMF_VENC_CHN);
                mmf_deinit_v2(false);
                av_write_trailer(param->outputFormatContext);

                for (auto it = param->pcm_list->begin(); it != param->pcm_list->end(); ++it) {
                    Bytes *pcm = *it;
                    delete pcm;
                    it = param->pcm_list->erase(it);
                }
                delete param->pcm_list;
                av_frame_free(&param->audio_frame);
                swr_free(&param->swr_ctx);
                avcodec_free_context(&param->audio_codec_ctx);

                avformat_close_input(&param->outputFormatContext);
                if (param->outputFormatContext && !(param->outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
                    avio_closep(&param->outputFormatContext->pb);
                }
                avformat_free_context(param->outputFormatContext);
                av_packet_unref(param->pPacket);
                av_packet_free(&param->pPacket);

                free(_param);
                _param = NULL;
            }

            if (_capture_image && _capture_image->data()) {
                delete _capture_image;
                _capture_image = nullptr;
            }

            system("sync");
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

    video::Frame *Encoder::encode(image::Image *img, Bytes *pcm) {
        uint8_t *stream_buffer = NULL;
        int stream_size = 0;
        uint64_t pts = 0, dts = 0;
        if (_path.size() == 0) {
            uint64_t curr_ms = time::ticks_ms();
            uint64_t diff_ms = 0;
            if (!_encode_started) {
                _encode_started = true;
                _start_encode_ms = curr_ms;
            }
            diff_ms = curr_ms - _start_encode_ms;

            switch (_type) {
            case VIDEO_H264:
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
            case VIDEO_H265:
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
        } else {
            encoder_param_t *param = (encoder_param_t *)_param;
            bool use_input_img = false;

            // you need bind camera or pass in an image
            err::check_bool_raise(_bind_camera || (img && img->data()), "You need bind a camera or pass in an image!");

            // check use image or not
            if (img && img->data() != NULL && (img->width() != _width || img->height() != _height)) {
                log::error("image is not match!\r\n"\
                            "the width of image need:%d input:%d\r\n"
                            "the height of image need:%d input:%d\r\n"
                            "the format of image need:%s input:%s",
                            _width, img->width(), _height, img->height(),
                            image::fmt_names[_format].c_str(), image::fmt_names[img->format()].c_str());
                err::check_raise(err::ERR_RUNTIME, "image is not match!");
            } else if (img && img->data() != NULL) {
                use_input_img = true;
            } else {
                use_input_img = false;
            }

            switch (_type) {
            case VIDEO_H264:
            {
                if (_block) {
                    if (use_input_img) {
                        while ((time::ticks_ms() - param->last_encode_ms) * _framerate < 1000) {
                            time::sleep_us(500);
                        }

                        param->last_encode_ms = time::ticks_ms();
                        if (mmf_venc_push(MMF_VENC_CHN, (uint8_t *)img->data(), img->width(), img->height(), mmf_invert_format_to_mmf(img->format()))) {
                            log::error("mmf_venc_push failed\n");
                            goto _exit;
                        }
                    } else {
                        int vi_ch = _camera->get_channel();
                        void *data;
                        int data_size, width, height, format;
                        if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                            log::error("read camera image failed!\r\n");
                            goto _exit;
                        }

                        while ((time::ticks_ms() - param->last_encode_ms) * _framerate < 1000) {
                            time::sleep_us(500);
                        }
                        param->last_encode_ms = time::ticks_ms();

                        if (mmf_venc_push(MMF_VENC_CHN, (uint8_t *)data, width, height, format)) {
                            log::warn("mmf_venc_push failed\n");
                            mmf_del_venc_channel(MMF_VENC_CHN);
                            goto _exit;
                        }

                        if (_need_capture) {
                            if (_capture_image && _capture_image->data()) {
                                delete _capture_image;
                                _capture_image = NULL;
                            }
                            _capture_image = _image_from_mmf_vi(vi_ch, data, width, height, format);
                            err::check_null_raise(_capture_image, "capture image failed!");
                        }

                        mmf_vi_frame_free(vi_ch);
                    }
                }

                mmf_stream_t stream = {0};
                if (mmf_venc_pop(MMF_VENC_CHN, &stream)) {
                    log::error("mmf_venc_pop failed\n");
                    mmf_venc_free(MMF_VENC_CHN);
                    mmf_del_venc_channel(MMF_VENC_CHN);
                    goto _exit;
                }

                for (int i = 0; i < stream.count; i ++) {
                    // printf("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];
                }

                if (stream_size != 0) {
                    bool is_first_frame = false;
                    if (!param->find_sps_pps && stream.count > 2) {
                        is_first_frame = true;
                        param->find_sps_pps = true;
                        param->video_frame_last_ms = time::ticks_ms();
                    }

                    if (param->find_sps_pps) {
                        stream_buffer = (uint8_t *)malloc(stream_size);
                        if (!stream_buffer) {
                            log::error("malloc failed!\r\n");
                            mmf_venc_free(MMF_VENC_CHN);
                            mmf_del_venc_channel(MMF_VENC_CHN);
                            goto _exit;
                        }

                        if (stream.count > 0) {
                            int copy_length = 0;
                            for (int i = 0; i < stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                                copy_length += stream.data_size[i];
                            }

                            auto frame_buffer = (uint8_t *)av_malloc(stream_size);
                            if (!frame_buffer) {
                                log::error("malloc failed!\r\n");
                                free(stream_buffer);
                                stream_buffer = NULL;
                                mmf_venc_free(MMF_VENC_CHN);
                                mmf_del_venc_channel(MMF_VENC_CHN);
                                goto _exit;
                            }
                            memcpy(frame_buffer, stream_buffer, stream_size);

                            auto curr_ms = time::ticks_ms();
                            if (is_first_frame) {
                                param->pPacket->stream_index = param->outputStream->index;
                                param->pPacket->duration = -1;
                                param->pPacket->pts = 0;
                                param->pPacket->dts = 0;
                                param->pPacket->data = frame_buffer;
                                param->pPacket->size = copy_length;
                            } else {
                                std::vector<int> timebase = {param->outputStream->time_base.num, param->outputStream->time_base.den};
                                AVPacket *new_packet = av_packet_alloc();
                                err::check_null_raise(new_packet, "malloc failed!");
                                new_packet->stream_index = param->outputStream->index;
                                new_packet->duration = -1;
                                new_packet->pts = ms_to_pts(timebase, curr_ms - param->video_frame_last_ms);
                                new_packet->dts = ms_to_pts(timebase, curr_ms - param->video_frame_last_ms);
                                new_packet->data = frame_buffer;
                                new_packet->size = copy_length;

                                param->pPacket->duration = new_packet->pts - param->pPacket->pts;
                                auto last_frame_buffer = param->pPacket->data;

                                // log::info("[VIDEO] pts:%d duration:%d time:%.2f", param->pPacket->pts, param->pPacket->duration, timebase_to_ms(timebase, param->pPacket->pts) / 1000);
                                err::check_bool_raise(av_interleaved_write_frame(param->outputFormatContext, param->pPacket) >= 0, "av_interleaved_write_frame failed!");
                                if (last_frame_buffer) {
                                    av_free(last_frame_buffer);
                                }
                                av_packet_unref(param->pPacket);
                                av_packet_free(&param->pPacket);
                                param->pPacket = new_packet;
                            }
                        } else {
                            free(stream_buffer);
                            stream_buffer = NULL;
                            stream_size = 0;
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

                // audio process
                if (pcm && pcm->data_len > 0) {
                    // uint64_t t = time::ticks_ms();
                    std::list<Bytes *> *pcm_list = param->pcm_list;
                    AVFrame *audio_frame = param->audio_frame;
                    AVStream *audio_stream = param->audio_stream;
                    AVCodecContext *audio_codec_ctx = param->audio_codec_ctx;
                    SwrContext *swr_ctx = param->swr_ctx;
                    AVFormatContext *outputFormatContext = param->outputFormatContext;
                    AVPacket *audio_packet = param->audio_packet;
                    size_t buffer_size = av_samples_get_buffer_size(NULL, 1, audio_frame->nb_samples, param->audio_format, 1);
                    size_t pcm_remain_len = pcm->data_len;
                    // fill last pcm to buffer_size
                    Bytes *last_pcm = pcm_list->back();
                    if (last_pcm && last_pcm->data_len < buffer_size) {
                        int temp_size = pcm_remain_len + last_pcm->data_len >= buffer_size ? buffer_size : pcm_remain_len + last_pcm->data_len;
                        uint8_t *temp = (uint8_t *)malloc(temp_size);
                        err::check_null_raise(temp, "malloc failed!");
                        memcpy(temp, last_pcm->data, last_pcm->data_len);
                        if (pcm_remain_len + last_pcm->data_len < buffer_size) {
                            memcpy(temp + last_pcm->data_len, pcm->data, pcm_remain_len);
                            pcm_remain_len = 0;
                        } else {
                            memcpy(temp + last_pcm->data_len, pcm->data, buffer_size - last_pcm->data_len);
                            pcm_remain_len -= (buffer_size - last_pcm->data_len);
                        }

                        Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                        pcm_list->pop_back();
                        delete last_pcm;
                        pcm_list->push_back(new_pcm);
                    }

                    // fill other pcm
                    while (pcm_remain_len > 0) {
                        int temp_size = pcm_remain_len >= buffer_size ? buffer_size : pcm_remain_len;
                        uint8_t *temp = (uint8_t *)malloc(temp_size);
                        err::check_null_raise(temp, "malloc failed!");
                        memcpy(temp, pcm->data + pcm->data_len - pcm_remain_len, temp_size);
                        pcm_remain_len -= temp_size;

                        Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                        pcm_list->push_back(new_pcm);
                    }

                    // audio process
                    while (pcm_list->size() > 0) {
                        Bytes *pcm = pcm_list->front();
                        if (pcm) {
                            if (pcm->data_len == buffer_size) {
                                // save to mp4
                                // log::info("pcm data:%p size:%d list_size:%d", pcm->data, pcm->data_len, pcm_list->size());
                                const uint8_t *in[] = {pcm->data};
                                uint8_t *out[] = {audio_frame->data[0]};
                                swr_convert(swr_ctx, out, audio_codec_ctx->frame_size, in, audio_codec_ctx->frame_size);
                                audio_frame->pts = param->audio_last_pts;
                                if (avcodec_send_frame(audio_codec_ctx, audio_frame) < 0) {
                                    printf("Error sending audio_frame to encoder.\n");
                                    break;
                                }

                                while (avcodec_receive_packet(audio_codec_ctx, audio_packet) == 0) {
                                    audio_packet->stream_index = audio_stream->index;
                                    audio_packet->pts = param->audio_last_pts;
                                    audio_packet->dts = audio_packet->pts;
                                    audio_packet->duration = audio_codec_ctx->frame_size;
                                    param->audio_last_pts = audio_packet->pts + audio_packet->duration;

                                    // std::vector<int> timebase = {audio_stream->time_base.num, audio_stream->time_base.den};
                                    // log::info("[AUDIO] pts:%d duration:%d timebase:%d/%d time:%.2f", audio_packet->pts, audio_packet->duration, audio_stream->time_base.num, audio_stream->time_base.den, timebase_to_ms(timebase, audio_packet->pts) / 1000);
                                    av_interleaved_write_frame(outputFormatContext, audio_packet);
                                    av_packet_unref(audio_packet);
                                }
                                pcm_list->pop_front();
                                delete pcm;
                            } else {
                                break;
                            }
                        } else {
                            err::check_raise(err::ERR_RUNTIME, "pcm data error");
                        }
                    }
                    // log::info("pcm process time:%d", time::ticks_ms() - t);
                }

                if (!_block) {
                    if (use_input_img) {
                        while ((time::ticks_ms() - param->last_encode_ms) * _framerate < 1000) {
                            time::sleep_us(500);
                        }

                        param->last_encode_ms = time::ticks_ms();
                        if (mmf_venc_push(MMF_VENC_CHN, (uint8_t *)img->data(), img->width(), img->height(), mmf_invert_format_to_mmf(img->format()))) {
                            log::error("mmf_venc_push failed\n");
                            goto _exit;
                        }
                    } else {
                        int vi_ch = _camera->get_channel();
                        void *data;
                        int data_size, width, height, format;
                        if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                            log::error("read camera image failed!\r\n");
                            goto _exit;
                        }

                        while ((time::ticks_ms() - param->last_encode_ms) * _framerate < 1000) {
                            time::sleep_us(500);
                        }
                        param->last_encode_ms = time::ticks_ms();

                        if (mmf_venc_push(MMF_VENC_CHN, (uint8_t *)data, width, height, format)) {
                            log::warn("mmf_venc_push failed\n");
                            mmf_del_venc_channel(MMF_VENC_CHN);
                            goto _exit;
                        }

                        if (_need_capture) {
                            if (_capture_image && _capture_image->data()) {
                                delete _capture_image;
                                _capture_image = NULL;
                            }
                            _capture_image = _image_from_mmf_vi(vi_ch, data, width, height, format);
                            err::check_null_raise(_capture_image, "capture image failed!");
                        }

                        mmf_vi_frame_free(vi_ch);
                    }
                }
                break;
            }
            default:
                err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
                break;
            }
        }
_exit:
            video::Frame *frame = new video::Frame(stream_buffer, stream_size, pts, dts, 0, true, false);
            return frame;
    }

    typedef enum {
        VIDEO_FORMAT_NONE,
        VIDEO_FORMAT_H264,
        VIDEO_FORMAT_H264_MP4,
        VIDEO_FORMAT_H264_FLV,
        VIDEO_FORMAT_MAX
    } video_format_e;

    typedef struct {
        AVFormatContext *pFormatContext;

        // video
        bool find_video;
        AVPacket *pPacket;
        AVBSFContext * bsfc;
        AVCodecContext *codec_ctx;
        int video_stream_index;
        video_format_e video_format;
        int vdec_ch;
        PAYLOAD_TYPE_E vdec_type;
        std::list<video::Context *> *ctx_list;
        uint64_t next_pts;

        // audio
        bool find_audio;
        int audio_stream_index;
        int sample_rate;
        int channels;
        enum AVSampleFormat sample_fmt;
        int resample_sample_rate;
        int resample_channels;
        enum AVSampleFormat resample_sample_format;
        AVCodecContext *audio_codec_ctx = NULL;
        // AVPacket *audio_packet = NULL;
        AVFrame *audio_frame = NULL;
        SwrContext *swr_ctx = NULL;
    } decoder_param_t;

    static video_format_e _get_video_format(const char *filename, PAYLOAD_TYPE_E type) {
        video_format_e format = VIDEO_FORMAT_NONE;
        const char *suffix = strrchr(filename, '.');
        err::check_null_raise((void *)suffix, "Try a file format with a suffix, e.g. video.h264/video.mp4/video.flv");

        if (!strcmp(suffix, ".h264")) {
            format = VIDEO_FORMAT_H264;
        } else if (!strcmp(suffix, ".mp4")) {
            if (type == PT_H264) {
                format = VIDEO_FORMAT_H264_MP4;
            }
        } else if (!strcmp(suffix, ".flv")) {
            if (type == PT_H264) {
                format = VIDEO_FORMAT_H264_FLV;
            }
        } else {
            err::check_raise(err::ERR_RUNTIME, "Currently only support avc/avc-mp4/avc-flv format!");
        }

        err::check_bool_raise(format != VIDEO_FORMAT_NONE, "Not found a valid video format!");
        return format;
    }


    static int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)
    {
        int ret = avformat_match_stream_specifier(s, st, spec);
        if (ret < 0)
            av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
        return ret;
    }

    static AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                    AVFormatContext *s, AVStream *st, const AVCodec *codec)
    {
        AVDictionary    *ret = NULL;
        AVDictionaryEntry *t = NULL;
        int            flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
                                        : AV_OPT_FLAG_DECODING_PARAM;
        char          prefix = 0;
        const AVClass    *cc = avcodec_get_class();

        if (!codec)
            codec            = s->oformat ? avcodec_find_encoder(codec_id)
                                        : avcodec_find_decoder(codec_id);

        switch (st->codecpar->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            prefix  = 'v';
            flags  |= AV_OPT_FLAG_VIDEO_PARAM;
            break;
        case AVMEDIA_TYPE_AUDIO:
            prefix  = 'a';
            flags  |= AV_OPT_FLAG_AUDIO_PARAM;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            prefix  = 's';
            flags  |= AV_OPT_FLAG_SUBTITLE_PARAM;
            break;
        default:
            break;
        }

        while ((t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX))) {
            const AVClass *priv_class;
            char *p = strchr(t->key, ':');

            /* check stream specification in opt name */
            if (p)
                switch (check_stream_specifier(s, st, p + 1)) {
                case  1: *p = 0; break;
                case  0:         continue;
                default:         return NULL;
                }

            if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
                !codec ||
                ((priv_class = codec->priv_class) &&
                av_opt_find(&priv_class, t->key, NULL, flags,
                            AV_OPT_SEARCH_FAKE_OBJ)))
                av_dict_set(&ret, t->key, t->value, 0);
            else if (t->key[0] == prefix &&
                    av_opt_find(&cc, t->key + 1, NULL, flags,
                                AV_OPT_SEARCH_FAKE_OBJ))
                av_dict_set(&ret, t->key + 1, t->value, 0);

            if (p)
                *p = ':';
        }
        return ret;
    }

    static AVDictionary **setup_find_stream_info_opts(AVFormatContext *s,
                                            AVDictionary *codec_opts)
    {
        unsigned int i;
        AVDictionary **opts;

        if (!s->nb_streams)
            return NULL;
        opts = (AVDictionary **)av_mallocz_array(s->nb_streams, sizeof(*opts));
        if (!opts) {
            av_log(NULL, AV_LOG_ERROR,
                "Could not alloc memory for stream options.\n");
            return NULL;
        }
        for (i = 0; i < s->nb_streams; i++)
            opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codecpar->codec_id,
                                        s, s->streams[i], NULL);
        return opts;
    }
#if 0
    static int _mmf_vdec_push(int ch, VDEC_STREAM_S *stStream)
    {
        CVI_S32 s32Ret = CVI_SUCCESS;
        s32Ret = CVI_VDEC_SendStream(ch, stStream, -1);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VDEC_SendStream chn[%d] failed with %#x!\n", ch, s32Ret);
            return -1;
        }
        return 0;
    }

    static int _mmf_vdec_pop(int ch, VIDEO_FRAME_INFO_S *frame)
    {
        CVI_S32 s32Ret = CVI_SUCCESS;
        int fd = CVI_VDEC_GetFd(ch);
        if (fd < 0) {
            printf("CVI_VENC_GetFd failed with %d\n", fd);
            return -1;
        }

        fd_set readFds;
        struct timeval timeoutVal;
        FD_ZERO(&readFds);
        FD_SET(fd, &readFds);
        timeoutVal.tv_sec = 0;
        timeoutVal.tv_usec = 80*1000;
        s32Ret = select(fd + 1, &readFds, NULL, NULL, &timeoutVal);
        if (s32Ret < 0) {
            if (errno == EINTR) {
                printf("VdecChn(%d) select failed!\n", ch);
                return -1;
            }
        } else if (s32Ret == 0) {
            printf("VdecChn(%d) select timeout!\n", ch);
            return -1;
        }

        s32Ret = CVI_VDEC_GetFrame(ch, frame, 1000);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VDEC_GetFrame failed with %#x(%d)\n", s32Ret, s32Ret);
            return s32Ret;
        }

        return s32Ret;
    }

    static int _mmf_vdec_free(int ch, VIDEO_FRAME_INFO_S *pstFrameInfo)
    {
        CVI_S32 s32Ret = CVI_SUCCESS;
        s32Ret = CVI_VDEC_ReleaseFrame(ch, pstFrameInfo);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VDEC_ReleaseFrame failed with %#x\n", s32Ret);
            return s32Ret;
        }

        return s32Ret;
    }
#endif
    static audio::Format _audio_format_from_alsa(enum AVSampleFormat format) {
        switch (format) {
            case AV_SAMPLE_FMT_NONE: return audio::FMT_NONE;
            case AV_SAMPLE_FMT_U8: return audio::FMT_S8;
            case AV_SAMPLE_FMT_S16: return audio::FMT_S16_LE;
            case AV_SAMPLE_FMT_S32: return audio::FMT_S32_LE;
            default: {
                log::error("Not support format %s", av_get_sample_fmt_name(format));
                err::check_raise(err::ERR_NOT_IMPL);
            }
        }
        return audio::FMT_NONE;
    }

    static enum AVSampleFormat _audio_format_to_alsa(audio::Format format) {
        switch (format) {
            case audio::FMT_NONE: return AV_SAMPLE_FMT_NONE;
            case audio::FMT_S8: return AV_SAMPLE_FMT_U8;
            case audio::FMT_S16_LE: return AV_SAMPLE_FMT_S16;
            case audio::FMT_S32_LE: return AV_SAMPLE_FMT_S32;
            default: {
                err::check_raise(err::ERR_NOT_IMPL);
            }
        }
        return AV_SAMPLE_FMT_NONE;
    }

    Decoder::Decoder(std::string path, image::Format format) {
        av_log_set_callback(custom_log_callback);
        err::check_bool_raise(format == image::Format::FMT_YVU420SP || format == image::Format::FMT_GRAYSCALE, "Decoder only support FMT_GRAYSCALE or FMT_YVU420SP format!");
        _path = path;
        _format_out = format;
        _param = (decoder_param_t *)malloc(sizeof(decoder_param_t));
        memset(_param, 0, sizeof(decoder_param_t));

        PAYLOAD_TYPE_E vdec_type = PT_H264;
        video_format_e video_format = VIDEO_FORMAT_H264;
        err::check_null_raise(_param, "malloc failed!");
        AVCodec *codec = NULL;
        AVFormatContext *pFormatContext = avformat_alloc_context();
        err::check_null_raise(pFormatContext, "malloc failed!");
        err::check_bool_raise(!avformat_open_input(&pFormatContext, _path.c_str(), NULL, NULL), "Could not open file");
        pFormatContext->max_analyze_duration = 5000;    // reduce analyze time

        // Find stream infomation
        AVDictionary *codec_opts = NULL;
        AVDictionary **opts = setup_find_stream_info_opts(pFormatContext, codec_opts);
        int orig_nb_streams = pFormatContext->nb_streams;
        err::check_bool_raise(!avformat_find_stream_info(pFormatContext, opts), "Could not find stream information");
        if (orig_nb_streams) {
            for (int i = 0; i < orig_nb_streams; i++) {
                if (opts[i]) {
                    av_dict_free(&opts[i]);
                }
            }
            av_freep(&opts);
        }
        _bitrate = pFormatContext->bit_rate;

        AVPacket *pPacket = av_packet_alloc();
        err::check_null_raise(pPacket, "malloc failed!");

        /* video process */
        // Find video stream
        int ch = -1;
        AVCodecContext *video_codec_ctx = NULL;
        AVCodecParameters *video_codec_params = NULL;
        AVBSFContext * bsfc = NULL;
        int video_stream_index = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
        _has_video = video_stream_index < 0 ? false : true;
        if (_has_video) {
            // Find the number of b frame
            video_codec_ctx = avcodec_alloc_context3(codec);
            err::check_null_raise(video_codec_ctx, "Could not allocate a decoding context");
            avcodec_parameters_to_context(video_codec_ctx, pFormatContext->streams[video_stream_index]->codecpar);

            // Check encode type
            video_codec_params = pFormatContext->streams[video_stream_index]->codecpar;
            err::check_bool_raise(video_codec_params->codec_id == AV_CODEC_ID_H264, "Only support h264 encode video format!");
            vdec_type = PT_H264;
            video_format = _get_video_format(_path.c_str(), vdec_type);


            // Get video width/height
            _width = video_codec_params->width;
            _height = video_codec_params->height;
            _timebase.push_back(pFormatContext->streams[video_stream_index]->time_base.num);
            _timebase.push_back(pFormatContext->streams[video_stream_index]->time_base.den);
            AVStream *video_stream = pFormatContext->streams[video_stream_index];
            AVRational frame_rate = av_guess_frame_rate(pFormatContext, video_stream, NULL);
            _fps = av_q2d(frame_rate);
            if (_width % 32 != 0) {
                log::error("Width need align to 32, current width: %d", _width);
                avformat_close_input(&pFormatContext);
                free(_param);
                err::check_raise(err::ERR_RUNTIME, "Width need align to 32");
            }

            ch = mmf_vdec_unused_channel();
            err::check_bool_raise(ch >= 0, "No unused channel of vdec");
            VDEC_CHN_ATTR_S vdec_chn_attr;
            vdec_chn_attr.enType = vdec_type;
            vdec_chn_attr.enMode = VIDEO_MODE_FRAME;
            vdec_chn_attr.u32PicWidth = _width;
            vdec_chn_attr.u32PicHeight = _height;
            vdec_chn_attr.u32FrameBufCnt = 3;
            vdec_chn_attr.u32StreamBufSize = _width * _height;
            err::check_bool_raise(!mmf_add_vdec_channel_v2(ch, mmf_invert_format_to_mmf(image::Format::FMT_YVU420SP), 8, &vdec_chn_attr), "mmf_add_vdec_channel_v2 failed");

            switch (video_format) {
                case VIDEO_FORMAT_H264:
                    bsfc = NULL;
                    break;
                case VIDEO_FORMAT_H264_FLV: {
                    const AVBitStreamFilter * filter = av_bsf_get_by_name("h264_mp4toannexb");
                    err::check_bool_raise(!av_bsf_alloc(filter, &bsfc), "av_bsf_alloc failed");
                    avcodec_parameters_copy(bsfc->par_in, pFormatContext->streams[video_stream_index]->codecpar);
                    av_bsf_init(bsfc);
                    break;
                }
                case VIDEO_FORMAT_H264_MP4: {
                    const AVBitStreamFilter * filter = av_bsf_get_by_name("h264_mp4toannexb");
                    err::check_bool_raise(!av_bsf_alloc(filter, &bsfc), "av_bsf_alloc failed");
                    avcodec_parameters_copy(bsfc->par_in, pFormatContext->streams[video_stream_index]->codecpar);
                    av_bsf_init(bsfc);
                    break;
                }
                default: {
                    err::check_raise(err::ERR_RUNTIME, "Unknown video format");
                    break;
                }
            }
        }

        /* audio process */
        AVCodec *audio_codec = NULL;
        AVCodecContext *audio_codec_ctx = NULL;
        AVFrame *audio_frame = NULL;
        SwrContext *swr_ctx = NULL;
        int resample_channels = 2;
        int resample_sample_rate = 48000;
        enum AVSampleFormat resample_format = AV_SAMPLE_FMT_S16;
        int audio_stream_index = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
        _has_audio = audio_stream_index < 0 ? false : true;
        if (_has_audio) {
            AVCodecParameters *audio_codecpar = pFormatContext->streams[audio_stream_index]->codecpar;
            err::check_null_raise(audio_codecpar, "Could not find audio codec parameters");
            err::check_null_raise(audio_codec = avcodec_find_decoder(audio_codecpar->codec_id), "Could not find audio codec");
            err::check_null_raise(audio_codec_ctx = avcodec_alloc_context3(audio_codec), "Could not allocate audio codec context");
            err::check_bool_raise(avcodec_parameters_to_context(audio_codec_ctx, audio_codecpar) >= 0, "Could not copy audio codec parameters to decoder context");
            err::check_bool_raise(avcodec_open2(audio_codec_ctx, codec, NULL) >= 0, "Could not open audio codec");
            err::check_null_raise(audio_frame = av_frame_alloc(), "Could not allocate audio frame");
            err::check_null_raise(swr_ctx = swr_alloc(), "Could not allocate resampler context");
            av_opt_set_int(swr_ctx, "in_channel_layout", audio_codec_ctx->channel_layout, 0);
            av_opt_set_int(swr_ctx, "in_sample_rate", audio_codec_ctx->sample_rate, 0);
            av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_codec_ctx->sample_fmt, 0);
            resample_channels = audio_codec_ctx->channels;
            av_opt_set_int(swr_ctx, "out_channel_layout", audio_codec_ctx->channel_layout, 0);
            av_opt_set_int(swr_ctx, "out_sample_rate", resample_sample_rate, 0);
            av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", resample_format, 0);
            swr_init(swr_ctx);
        }

        /* init param */
        // video
        decoder_param_t *param = (decoder_param_t *)_param;
        param->pFormatContext = pFormatContext;
        param->pPacket = pPacket;
        param->bsfc = bsfc;
        param->codec_ctx = video_codec_ctx;
        param->video_stream_index = video_stream_index;
        param->video_format = video_format;
        param->vdec_ch = ch;
        param->vdec_type = vdec_type;
        param->ctx_list = new std::list<video::Context *>();
        param->next_pts = 0;

        // audio
        if (_has_audio) {
            _audio_channels = resample_channels;
            _audio_format = _audio_format_from_alsa(resample_format);
            _audio_sample_rate = resample_sample_rate;
            param->audio_stream_index = audio_stream_index;
            param->sample_rate = audio_codec_ctx->sample_rate;
            param->channels = audio_codec_ctx->channels;
            param->resample_channels = resample_channels;
            param->resample_sample_rate = resample_sample_rate;
            param->resample_sample_format = resample_format;
            param->sample_fmt = audio_codec_ctx->sample_fmt;
            param->audio_codec_ctx = audio_codec_ctx;
            param->audio_frame = audio_frame;
            param->swr_ctx = swr_ctx;
        }
    }

    static int _release_video_ctx_list(std::list<video::Context *> *list)
    {
        if (list == NULL) return -1;

        std::list<video::Context *>::iterator iter;
        for(iter=list->begin();iter!=list->end();iter++) {
            video::Context *ctx = *iter;
            delete ctx;
            iter = list->erase(iter);
        }

        return 0;
    }

    Decoder::~Decoder() {
        decoder_param_t *param = (decoder_param_t *)_param;
        if (param) {
            // release video resource
            err::check_bool_raise(!_release_video_ctx_list(param->ctx_list), "release ctx list failed!");
            delete param->ctx_list;
            param->ctx_list = NULL;

            if (param->vdec_ch >= 0) {
                mmf_del_vdec_channel(param->vdec_ch);
            }

            av_packet_free(&param->pPacket);
            avcodec_free_context(&param->codec_ctx);
            avformat_close_input(&param->pFormatContext);

            if (param->bsfc) {
                av_bsf_free(&param->bsfc);
            }

            // release audio resource
            if (param->swr_ctx) {
                swr_free(&param->swr_ctx);
            }
            if (param->audio_frame) {
                av_frame_free(&param->audio_frame);
            }
            if (param->audio_codec_ctx) {
                avcodec_free_context(&param->audio_codec_ctx);
            }

            free(_param);
            _param = NULL;
        }
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

            if (frame->stVFrame.u32Stride[0] != (CVI_U32)width) {
                for (int h = 0; h < height; h ++) {
                    memcpy(buffer + h * width, frame->stVFrame.pu8VirAddr[0] + h * frame->stVFrame.u32Stride[0], width);
                }
            } else {
                memcpy(buffer, frame->stVFrame.pu8VirAddr[0], width * height);
            }
            break;
        case image::Format::FMT_YVU420SP:
            if (format != img->format()) {
                log::error("camera read: format not support, need %d, but %d", img->format(), format);
                goto _error;
            }

            if (frame->stVFrame.u32Stride[0] != (CVI_U32)width) {
                for (int h = 0; h < height; h ++) {
                    memcpy(buffer + h * width, frame->stVFrame.pu8VirAddr[0] + h * frame->stVFrame.u32Stride[0], width);
                }

                for (int h = 0; h < height / 2; h ++) {
                    memcpy(buffer + width * height + h * width, frame->stVFrame.pu8VirAddr[1] + h * frame->stVFrame.u32Stride[1], width);
                }
            } else {
                memcpy(buffer, frame->stVFrame.pu8VirAddr[0], width * height);
                memcpy(buffer + width * height, frame->stVFrame.pu8VirAddr[1], width * height / 2);
            }
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

    typedef struct {
        AVPacket *pPacket;
        uint8_t *data;
        uint8_t *end;
    } find_frame_iterator_t;

    static int _create_find_frame_iterator(find_frame_iterator_t *handler, AVPacket *pPacket)
    {
        if (handler == NULL || pPacket == NULL) return -1;

        memset(handler, 0, sizeof(find_frame_iterator_t));
        handler->pPacket = pPacket;
        handler->data = pPacket->data;
        handler->end = handler->data + pPacket->size;
        return 0;
    }

    // return 0: success, -1: failed
    static int _find_frame_iterator(find_frame_iterator_t *handler, uint8_t *type, uint8_t **data, size_t *data_size)
    {
        if (handler == NULL || type == NULL || data == NULL || data_size == NULL) return -1;
        bool found_start = false;
        uint8_t *start = NULL;
        uint8_t nal_type = 0;
        while (handler->data < handler->end) {
            if (handler->data + 3 < handler->end && handler->data[0] == 0 && handler->data[1] == 0 && handler->data[2] == 1) {
                if (!found_start) {
                    start = handler->data;
                    handler->data += 3;
                    nal_type = handler->data[0] & 0x1F;
                    found_start = true;
                } else {
                    break;
                }
            } else if (handler->data + 4 < handler->end && handler->data[0] == 0 && handler->data[1] == 0 && handler->data[2] == 0 && handler->data[3] == 1) {
                if (!found_start) {
                    start = handler->data;
                    handler->data += 4;
                    nal_type = handler->data[0] & 0x1F;
                    found_start = true;
                } else {
                    break;
                }
            }
            handler->data++;
        }

        if (found_start) {
            *type = nal_type;
            *data = start;
            *data_size = handler->data - start;
            return 0;
        }

        return -1;
    }

    video::Context *Decoder::decode_video(bool block) {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVPacket *pPacket = param->pPacket;
        AVFormatContext *pFormatContext = param->pFormatContext;
        AVBSFContext * bsfc = param->bsfc;
        int video_stream_index = param->video_stream_index;
        image::Image *img = NULL;
        video::Context *context = NULL;
        uint64_t last_pts = 0;
        uint64_t curr_pts = param->next_pts;
_retry:
        while (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == video_stream_index) {
                last_pts = _last_pts;
                _last_pts = pPacket->pts;

                int64_t packet_duration = pPacket->duration;
                switch (param->video_format) {
                    case VIDEO_FORMAT_H264:
                        break;
                    case VIDEO_FORMAT_H264_FLV:
                        err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        break;
                    case VIDEO_FORMAT_H264_MP4:
                        err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        break;
                    default:
                        err::check_raise(err::ERR_RUNTIME, "Unknown video format");
                        break;
                }

                // log::info("packet data:%p data size:%ld", pPacket->data, pPacket->size);

                find_frame_iterator_t frame_iterator;
                err::check_bool_raise(!_create_find_frame_iterator(&frame_iterator, pPacket), "create find frame iterator failed");
                uint8_t nal_type = 0, *frame = NULL;
                size_t frame_size = 0;
                uint8_t *sps = NULL, *pps = NULL, *i_or_pb = NULL;
                size_t sps_size = 0, pps_size = 0, i_or_pb_size = 0;
                while (0 == _find_frame_iterator(&frame_iterator, &nal_type, &frame, &frame_size)) {
                    switch (nal_type) {
                        case 1:
                            // log::info("NAL Unit Type: %u (P/B-Frame) frame:%p size:%ld", nal_type, frame, frame_size);
                            i_or_pb = frame;
                            i_or_pb_size = frame_size;
                            break;
                        case 5:
                            // log::info("NAL Unit Type: %u (I-Frame) frame:%p size:%ld", nal_type, frame, frame_size);
                            i_or_pb = frame;
                            i_or_pb_size = frame_size;
                            break;
                        case 7:
                            // log::info("NAL Unit Type: %u (SPS) frame:%p size:%ld", nal_type, frame, frame_size);
                            sps = frame;
                            sps_size = frame_size;
                            break;
                        case 8:
                            // log::info("NAL Unit Type: %u (PPS) frame:%p size:%ld", nal_type, frame, frame_size);
                            pps = frame;
                            pps_size = frame_size;
                            break;
                        default:
                            // log::info("NAL Unit Type: %u (Other) frame:%p size:%ld", nal_type, frame, frame_size);
                            break;
                    }

                    // for (int i = 0; i < frame_size; i ++) {
                    //     if (i > 64) {
                    //         break;
                    //     }

                    //     printf("%#x ", frame[i]);
                    //     if (i % 16 == 15) {
                    //         printf("\n");
                    //     }
                    // }
                    // printf("\n");
                }

                if (i_or_pb_size > 0) {
                    uint8_t *decode_data = NULL;
                    size_t decode_data_size = 0;
                    decode_data_size = sps_size + pps_size + i_or_pb_size;
                    decode_data = (uint8_t *)malloc(decode_data_size);
                    err::check_null_raise(decode_data, "malloc failed");
                    if (sps_size) {
                        memcpy(decode_data, sps, sps_size);
                    }
                    if (pps_size) {
                        memcpy(decode_data + sps_size, pps, pps_size);
                    }
                    memcpy(decode_data + sps_size + pps_size, i_or_pb, i_or_pb_size);

                    // log::info("process sps_size:%d pps_size:%d i_or_pb_size:%d decode_data_size:%d", sps_size, pps_size, i_or_pb_size, decode_data_size);
                    // for (int i = 0; i < decode_data_size; i ++) {
                    //     printf("%#x ", decode_data[i]);
                    //     if (i % 16 == 15) {
                    //         printf("\n");
                    //     }
                    // }
                    // printf("\n");
                    VDEC_STREAM_S stStream = {0};
                    stStream.pu8Addr = (CVI_U8 *)decode_data;
                    stStream.u32Len = decode_data_size;
                    stStream.u64PTS = pPacket->pts;
                    stStream.bEndOfFrame = CVI_TRUE;
                    stStream.bEndOfStream = CVI_FALSE;
                    stStream.bDisplay = 1;

                    VIDEO_FRAME_INFO_S frame = {0};
                    video::MediaType media_type = MEDIA_TYPE_VIDEO;
                    if (block) {
                        err::check_bool_raise(!mmf_vdec_push_v2(param->vdec_ch, &stStream));
                        err::check_bool_raise(!mmf_vdec_pop_v2(param->vdec_ch, &frame));
                        img = _mmf_frame_to_image(&frame, _format_out);
                        err::check_bool_raise(!mmf_vdec_free(param->vdec_ch));
                        media_type = MEDIA_TYPE_VIDEO;
                    } else {
                        err::check_bool_raise(!mmf_vdec_pop_v2(param->vdec_ch, &frame));
                        if (frame.stVFrame.u32Width) {  // if width!=0, we think this frame is valid.
                            img = _mmf_frame_to_image(&frame, _format_out);
                            err::check_bool_raise(!mmf_vdec_free(param->vdec_ch));
                            media_type = MEDIA_TYPE_VIDEO;
                        } else {
                            media_type = MEDIA_TYPE_UNKNOWN;
                        }
                        err::check_bool_raise(!mmf_vdec_push_v2(param->vdec_ch, &stStream));
                    }

                    std::vector<int> timebase = {(int)pFormatContext->streams[video_stream_index]->time_base.num,
                                                (int)pFormatContext->streams[video_stream_index]->time_base.den};
                    context = new video::Context(media_type, timebase);
                    context->set_image(img, packet_duration, frame.stVFrame.u64PTS, last_pts);
                    av_packet_unref(pPacket);

                    free(decode_data);
                    break;
                }
            }
            av_packet_unref(pPacket);
        }

        if (param->video_format != VIDEO_FORMAT_H264) {
            if (context && context->media_type() == video::MEDIA_TYPE_VIDEO) {
                video::Context *ctx = context;
                std::list<video::Context *> *ctx_list = param->ctx_list;
                video::Context *play_ctx = NULL;
                if (curr_pts == ctx->pts()) {
                    play_ctx = ctx;
                } else {
                    ctx_list->push_back(ctx);
                    std::list<video::Context *>::iterator iter;
                    for(iter=ctx_list->begin();iter!=ctx_list->end();iter++) {
                        video::Context *ctx = *iter;
                        if (curr_pts == ctx->pts()) {
                            play_ctx = ctx;
                            iter = ctx_list->erase(iter);
                            break;
                        }
                    }
                }
                context = play_ctx;
                if (context == NULL) {
                    goto _retry;
                }
            }
        }

        if (context) {
            param->next_pts += context->duration();
        }
        return context;
    }

    video::Context *Decoder::decode_audio() {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVPacket *pPacket = param->pPacket;
        AVFormatContext *pFormatContext = param->pFormatContext;
        int audio_stream_index = param->audio_stream_index;
        AVCodecContext *audio_codec_ctx = param->audio_codec_ctx;
        AVFrame *audio_frame = param->audio_frame;
        int resample_channels = param->resample_channels;
        int resample_sample_rate = param->resample_sample_rate;
        enum AVSampleFormat resample_format = param->resample_sample_format;
        SwrContext *swr_ctx = param->swr_ctx;
        video::Context *context = NULL;
        while (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == audio_stream_index) {
                if (avcodec_send_packet(audio_codec_ctx, pPacket) >= 0) {
                    while (avcodec_receive_frame(audio_codec_ctx, audio_frame) >= 0) {
                        uint8_t *output;
                        int out_samples = av_rescale_rnd(
                            swr_get_delay(swr_ctx, audio_codec_ctx->sample_rate) + audio_frame->nb_samples,
                            audio_codec_ctx->sample_rate,
                            audio_codec_ctx->sample_rate,
                            AV_ROUND_UP
                        );

                        av_samples_alloc(&output, NULL, audio_codec_ctx->channels, out_samples, AV_SAMPLE_FMT_S16, 0);

                        int converted_samples = swr_convert(
                            swr_ctx,
                            &output, out_samples,
                            (const uint8_t **)audio_frame->data, audio_frame->nb_samples
                        );

                        video::MediaType media_type = MEDIA_TYPE_AUDIO;
                        if (converted_samples > 0) {
                            media_type = MEDIA_TYPE_AUDIO;
                        } else {
                            media_type = MEDIA_TYPE_UNKNOWN;
                        }

                        std::vector<int> timebase = {1, resample_sample_rate};
                        context = new video::Context(media_type, timebase, resample_sample_rate, _audio_format_from_alsa(resample_format), resample_channels);
                        Bytes data(output, converted_samples * audio_codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
                        context->set_pcm(&data);

                        av_freep(&output);
                    }
                }
                break;
            }
            av_packet_unref(pPacket);
        }

        return context;
    }

    video::Context *Decoder::decode(bool block) {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVPacket *pPacket = param->pPacket;
        AVFormatContext *pFormatContext = param->pFormatContext;
        AVBSFContext * bsfc = param->bsfc;
        int video_stream_index = param->video_stream_index;
        int audio_stream_index = param->audio_stream_index;
        AVCodecContext *audio_codec_ctx = param->audio_codec_ctx;
        AVFrame *audio_frame = param->audio_frame;
        int resample_channels = param->resample_channels;
        int resample_sample_rate = param->resample_sample_rate;
        enum AVSampleFormat resample_format = param->resample_sample_format;
        SwrContext *swr_ctx = param->swr_ctx;
        image::Image *img = NULL;
        video::Context *context = NULL;
        uint64_t last_pts = 0;
        bool is_video = false;
        bool is_audio = false;

        while (av_read_frame(pFormatContext, pPacket) >= 0) {
            // log::info("[READ] audio/video:%d pts:%d pts_ms:%.2f ms",
            // pPacket->stream_index, pPacket->pts, pPacket->pts / ((float)pFormatContext->streams[pPacket->stream_index]->time_base.den/pFormatContext->streams[pPacket->stream_index]->time_base.num));
            if (pPacket->stream_index == video_stream_index) {
                last_pts = _last_pts;
                _last_pts = pPacket->pts;
                is_video = true;

                int64_t packet_duration = pPacket->duration;
                switch (param->video_format) {
                    case VIDEO_FORMAT_H264:
                        break;
                    case VIDEO_FORMAT_H264_FLV:
                        err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        break;
                    case VIDEO_FORMAT_H264_MP4:
                        err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        break;
                    default:
                        err::check_raise(err::ERR_RUNTIME, "Unknown video format");
                        break;
                }

                VDEC_STREAM_S stStream = {0};
                stStream.pu8Addr = (CVI_U8 *)pPacket->data;
                stStream.u32Len = pPacket->size;
                stStream.u64PTS = pPacket->pts;
                stStream.bEndOfFrame = CVI_TRUE;
                stStream.bEndOfStream = CVI_FALSE;
                stStream.bDisplay = 1;

                VIDEO_FRAME_INFO_S frame = {0};
                video::MediaType media_type = MEDIA_TYPE_VIDEO;
                if (block) {
                    err::check_bool_raise(!mmf_vdec_push_v2(param->vdec_ch, &stStream));
                    err::check_bool_raise(!mmf_vdec_pop_v2(param->vdec_ch, &frame));
                    img = _mmf_frame_to_image(&frame, _format_out);
                    err::check_bool_raise(!mmf_vdec_free(param->vdec_ch));
                    media_type = MEDIA_TYPE_VIDEO;
                } else {
                    err::check_bool_raise(!mmf_vdec_pop_v2(param->vdec_ch, &frame));
                    if (frame.stVFrame.u32Width) {  // if width!=0, we think this frame is valid.
                        img = _mmf_frame_to_image(&frame, _format_out);
                        err::check_bool_raise(!mmf_vdec_free(param->vdec_ch));
                        media_type = MEDIA_TYPE_VIDEO;
                    } else {
                        media_type = MEDIA_TYPE_UNKNOWN;
                    }
                    err::check_bool_raise(!mmf_vdec_push_v2(param->vdec_ch, &stStream));
                }

                std::vector<int> timebase = {(int)pFormatContext->streams[video_stream_index]->time_base.num,
                                            (int)pFormatContext->streams[video_stream_index]->time_base.den};
                context = new video::Context(media_type, timebase);
                context->set_image(img, packet_duration, frame.stVFrame.u64PTS, last_pts);
                av_packet_unref(pPacket);
                break;
            } else if (pPacket->stream_index == audio_stream_index) {
                is_audio = true;
                if (avcodec_send_packet(audio_codec_ctx, pPacket) >= 0) {
                    while (avcodec_receive_frame(audio_codec_ctx, audio_frame) >= 0) {
                        uint8_t *output;
                        int out_samples = av_rescale_rnd(
                            swr_get_delay(swr_ctx, audio_codec_ctx->sample_rate) + audio_frame->nb_samples,
                            audio_codec_ctx->sample_rate,
                            audio_codec_ctx->sample_rate,
                            AV_ROUND_UP
                        );

                        av_samples_alloc(&output, NULL, audio_codec_ctx->channels, out_samples, AV_SAMPLE_FMT_S16, 0);

                        int converted_samples = swr_convert(
                            swr_ctx,
                            &output, out_samples,
                            (const uint8_t **)audio_frame->data, audio_frame->nb_samples
                        );

                        video::MediaType media_type = MEDIA_TYPE_AUDIO;
                        if (converted_samples > 0) {
                            // Process the converted PCM data in `output` (e.g., write to file or buffer)
                            // fwrite(output, 1, converted_samples * codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16), stdout);

                            media_type = MEDIA_TYPE_AUDIO;
                        } else {
                            media_type = MEDIA_TYPE_UNKNOWN;
                        }

                        std::vector<int> timebase = {(int)pFormatContext->streams[audio_stream_index]->time_base.num,
                                                    (int)pFormatContext->streams[audio_stream_index]->time_base.den};
                        // context = new video::Context(MEDIA_TYPE_UNKNOWN, timebase);
                        context = new video::Context(media_type, timebase, resample_sample_rate, _audio_format_from_alsa(resample_format), resample_channels);
                        Bytes data(output, converted_samples * audio_codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
                        context->set_pcm(&data, pPacket->duration, pPacket->pts);
                        // log::info("data:%p size:%d sample_rate:%d channel:%d timebase:%d/%d, duration:%d pts:%d", output,
                        //         converted_samples * audio_codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16),
                        //         audio_codec_ctx->sample_rate, audio_codec_ctx->channels,
                        //         timebase[0], timebase[1], pPacket->duration, audio_frame->pts);

                        av_freep(&output);
                    }
                }
                break;
            }
            av_packet_unref(pPacket);
        }

        if (is_video) {
            if (context) {
                param->next_pts += context->duration();
            }
            return context;
        } else if (is_audio) {
            return context;
        } else {
            return NULL;
        }
    }

    video::Context *Decoder::unpack() {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVPacket *pPacket = param->pPacket;
        AVFormatContext *pFormatContext = param->pFormatContext;
        AVBSFContext * bsfc = param->bsfc;
        int video_stream_index = param->video_stream_index;
        int audio_stream_index = param->audio_stream_index;
        AVCodecContext *audio_codec_ctx = param->audio_codec_ctx;
        AVFrame *audio_frame = param->audio_frame;
        int resample_channels = param->resample_channels;
        int resample_sample_rate = param->resample_sample_rate;
        enum AVSampleFormat resample_format = param->resample_sample_format;
        SwrContext *swr_ctx = param->swr_ctx;
        video::Context *context = NULL;
        uint64_t last_pts = 0;
        bool is_video = false;
        bool is_audio = false;

        while (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == video_stream_index) {
                last_pts = _last_pts;
                _last_pts = pPacket->pts;
                is_video = true;

                int64_t packet_duration = pPacket->duration;
                switch (param->video_format) {
                    case VIDEO_FORMAT_H264:
                        break;
                    case VIDEO_FORMAT_H264_FLV:
                        err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        break;
                    case VIDEO_FORMAT_H264_MP4:
                        err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                        break;
                    default:
                        err::check_raise(err::ERR_RUNTIME, "Unknown video format");
                        break;
                }

                video::MediaType media_type = MEDIA_TYPE_VIDEO;
                std::vector<int> timebase = {(int)pFormatContext->streams[video_stream_index]->time_base.num,
                                            (int)pFormatContext->streams[video_stream_index]->time_base.den};
                context = new video::Context(media_type, timebase);
                context->set_raw_data(pPacket->data, pPacket->size, packet_duration, pPacket->pts, last_pts, true);
                av_packet_unref(pPacket);
                break;
            } else if (pPacket->stream_index == audio_stream_index) {
                is_audio = true;
                if (avcodec_send_packet(audio_codec_ctx, pPacket) >= 0) {
                    while (avcodec_receive_frame(audio_codec_ctx, audio_frame) >= 0) {
                        uint8_t *output;
                        int out_samples = av_rescale_rnd(
                            swr_get_delay(swr_ctx, audio_codec_ctx->sample_rate) + audio_frame->nb_samples,
                            audio_codec_ctx->sample_rate,
                            audio_codec_ctx->sample_rate,
                            AV_ROUND_UP
                        );

                        av_samples_alloc(&output, NULL, audio_codec_ctx->channels, out_samples, AV_SAMPLE_FMT_S16, 0);

                        int converted_samples = swr_convert(
                            swr_ctx,
                            &output, out_samples,
                            (const uint8_t **)audio_frame->data, audio_frame->nb_samples
                        );

                        video::MediaType media_type = MEDIA_TYPE_AUDIO;
                        if (converted_samples > 0) {
                            // Process the converted PCM data in `output` (e.g., write to file or buffer)
                            // fwrite(output, 1, converted_samples * codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16), stdout);

                            media_type = MEDIA_TYPE_AUDIO;
                        } else {
                            media_type = MEDIA_TYPE_UNKNOWN;
                        }

                        std::vector<int> timebase = {(int)pFormatContext->streams[audio_stream_index]->time_base.num,
                                                    (int)pFormatContext->streams[audio_stream_index]->time_base.den};
                        // context = new video::Context(MEDIA_TYPE_UNKNOWN, timebase);
                        context = new video::Context(media_type, timebase, resample_sample_rate, _audio_format_from_alsa(resample_format), resample_channels);
                        Bytes data(output, converted_samples * audio_codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
                        context->set_pcm(&data, pPacket->duration, pPacket->pts);
                        // log::info("data:%p size:%d sample_rate:%d channel:%d timebase:%d/%d, duration:%d pts:%d", output,
                        //         converted_samples * audio_codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16),
                        //         audio_codec_ctx->sample_rate, audio_codec_ctx->channels,
                        //         timebase[0], timebase[1], pPacket->duration, audio_frame->pts);

                        av_freep(&output);
                    }
                }
                break;
            }
            av_packet_unref(pPacket);
        }

        if (is_video) {
            if (context) {
                param->next_pts += context->duration();
            }
            return context;
        } else if (is_audio) {
            return context;
        } else {
            return NULL;
        }
    }

    double Decoder::seek(double time) {
#if 0
        decoder_param_t *param = (decoder_param_t *)_param;
        AVFormatContext *pFormatContext = param->pFormatContext;
        video_format_e video_format = param->video_format;
        AVPacket *pPacket = param->pPacket;
        int video_stream_index = param->video_stream_index;

        if (time >= 0) {
            int64_t seek_target = av_rescale_q(time * AV_TIME_BASE, AV_TIME_BASE_Q, pFormatContext->streams[video_stream_index]->time_base);
            if (video_format != VIDEO_FORMAT_H264_FLV && video_format != VIDEO_FORMAT_H264_MP4) {
                return 0;
            }log::info("seek target:%d timebase:%d/%d", seek_target, pFormatContext->streams[video_stream_index]->time_base.den, pFormatContext->streams[video_stream_index]->time_base.num);
            int ret = av_seek_frame(pFormatContext, video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                avformat_close_input(&param->pFormatContext);
                log::error("av_seek_frame failed, ret:%d", ret);
                return 0;
            }

            while (av_read_frame(pFormatContext, pPacket) >= 0) {
                if (pPacket->stream_index == video_stream_index) {
                    param->next_pts = pPacket->pts;
                    av_packet_unref(pPacket);
                    break;
                }
                av_packet_unref(pPacket);
            }

            ret = av_seek_frame(pFormatContext, video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                avformat_close_input(&param->pFormatContext);
                log::error("av_seek_frame failed, ret:%d", ret);
                return 0;
            }
        } else {
            time = param->next_pts * av_q2d(pFormatContext->streams[video_stream_index]->time_base);
        }
        return time;
#else
        decoder_param_t *param = (decoder_param_t *)_param;
        AVFormatContext *pFormatContext = param->pFormatContext;
        video_format_e video_format = param->video_format;
        AVPacket *pPacket = param->pPacket;
        AVBSFContext * bsfc = param->bsfc;
        int video_stream_index = param->video_stream_index;

        if (time >= 0) {
            int64_t seek_target = av_rescale_q(time * AV_TIME_BASE, AV_TIME_BASE_Q, pFormatContext->streams[video_stream_index]->time_base);
            if (video_format != VIDEO_FORMAT_H264_FLV && video_format != VIDEO_FORMAT_H264_MP4) {
                return 0;
            }
            int ret = av_seek_frame(pFormatContext, video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                avformat_close_input(&param->pFormatContext);
                log::error("av_seek_frame failed, ret:%d", ret);
                return 0;
            }

            bool found_i_sps_frame = false;
            while (av_read_frame(pFormatContext, pPacket) >= 0) {
                if (pPacket->stream_index == video_stream_index) {
                    switch (param->video_format) {
                        case VIDEO_FORMAT_H264:
                            break;
                        case VIDEO_FORMAT_H264_FLV:
                            err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                            err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                            break;
                        case VIDEO_FORMAT_H264_MP4:
                            err::check_bool_raise(!av_bsf_send_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                            err::check_bool_raise(!av_bsf_receive_packet(bsfc, pPacket), "av_bsf_send_packet failed");
                            break;
                        default:
                            err::check_raise(err::ERR_RUNTIME, "Unknown video format");
                            break;
                    }

                    find_frame_iterator_t frame_iterator;
                    err::check_bool_raise(!_create_find_frame_iterator(&frame_iterator, pPacket), "create find frame iterator failed");
                    uint8_t nal_type = 0, *frame = NULL;
                    size_t frame_size = 0;
                    while (0 == _find_frame_iterator(&frame_iterator, &nal_type, &frame, &frame_size)) {
                        if (nal_type == 5 || nal_type == 7) { // 1:P/B  5:I  7:SPS 8:PPS
                            found_i_sps_frame = true;
                        }
                    }
                    param->next_pts = pPacket->pts;
                    av_packet_unref(pPacket);
                    if (found_i_sps_frame) {
                        break;
                    }
                }
                av_packet_unref(pPacket);
            }

            if (!found_i_sps_frame) {
                return -1;
            }
            seek_target = param->next_pts;

            ret = av_seek_frame(pFormatContext, video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                avformat_close_input(&param->pFormatContext);
                log::error("av_seek_frame failed, ret:%d", ret);
                return 0;
            }

            err::check_bool_raise(!_release_video_ctx_list(param->ctx_list), "release ctx list failed!");
        } else {
            time = param->next_pts * av_q2d(pFormatContext->streams[video_stream_index]->time_base);
        }
        return time;
#endif
    }

    double Decoder::duration() {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVFormatContext *pFormatContext = param->pFormatContext;
        video_format_e video_format = param->video_format;
        if (video_format != VIDEO_FORMAT_H264_FLV && video_format != VIDEO_FORMAT_H264_MP4) {
            return 0;
        }
        int64_t duration = pFormatContext->duration;
        double duration_in_seconds = (double)duration / AV_TIME_BASE;
        return duration_in_seconds;
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

    typedef enum {
        VIDEO_RECORDER_IDLE = 0,
        VIDEO_RECORDER_RECORD,           // record and display
        VIDEO_RECORDER_UNKNOWN,
        VIDEO_RECORDER_DISPLAY_ONLY,
    } video_recoder_state_t;

    class rect_info {
    public:
        int id;
        int x;
        int y;
        int w;
        int h;
        image::Color color = image::Color(255);
        int thickness;
        bool show = false;
    };

    typedef struct {
        VideoRecorder *obj;
        pthread_mutex_t lock;
        video_recoder_state_t state;
        std::string path;
        bool snapshot_en;
        std::vector<int> snapshot_res;
        image::Format snapshot_fmt;
        image::Image *snapshot_img;
        int64_t seek_start_ms;
        int64_t seek_ms;

        encoder_param_t packager;
        pthread_t thread;
        bool thread_exit_flag;

        struct {
            int ch;
            int fps;
            int bitrate;
            std::vector<int> resolution;
        } venc;

        struct {
            camera::Camera *obj;
            camera::Camera *obj2;
        } camera;

        struct {
            display::Display *obj;
            int fit;
        } display;

        struct {
            audio::Recorder *obj;
            bool mute;
        } audio;

        struct {
            void *obj;
            void *gcsv;
            std::string gcsv_path;
            pthread_t imu_thread;
        } imu;

        std::vector<rect_info> rect;
    } video_recoder_param_t;

    static void _video_recoder_config_default(video_recoder_param_t *param)
    {
        param->venc.ch = MMF_VENC_CHN;
        param->venc.bitrate = 3000000;
        param->venc.fps = 30;
        param->seek_ms = 0;
        param->snapshot_en = false;
        param->snapshot_fmt = image::Format::FMT_YVU420SP;
        param->state = VIDEO_RECORDER_IDLE;
        param->thread_exit_flag = false;
        param->rect.clear();
        param->rect.resize(16);
    }

    VideoRecorder::VideoRecorder(bool open)
    {
        video_recoder_param_t *param = (video_recoder_param_t *)calloc(1, sizeof(video_recoder_param_t));
        err::check_null_raise(param, "malloc param failed");

        _is_opened = false;
        _param = param;

        if (open) {
            this->open();
        }
    }

    VideoRecorder::~VideoRecorder()
    {
        close();

        if (_param) {
            free(_param);
            _param = nullptr;
        }
    }


    static void nv21_fill_rect(uint8_t *yuv, int width, int height, int x1, int y1, int rect_w, int rect_h, uint8_t y_value, uint8_t u_value, uint8_t v_value)
    {
        for (int h = y1; h < y1 + rect_h; h++) {
            for (int w = x1; w < x1 + rect_w; w++) {
                int index = h * width + w;
                yuv[index] = y_value;
            }
        }

        for (int h = y1; h < y1 + rect_h; h+=2) {
            for (int w = x1; w < x1 + rect_w; w+=2) {
                int uv_index = width * height + (h / 2) * width + w;
                yuv[uv_index] = v_value;
                yuv[uv_index + 1] = u_value;
            }
        }
    }

    static void nv21_draw_rectangle(uint8_t *yuv, int width, int height, int x, int y, int rect_width, int rect_height, uint8_t y_value, uint8_t u_value, uint8_t v_value, int thickness) {

        int tmp_x = 0, tmp_y = 0, tmp_rect_width = 0, tmp_rect_height = 0;
        x = (x % 2 == 0) ? x : x - 1;
        x = x < 0 ? 0 : x;
        x = x >= width ? width - 2 : x;
        y = (y % 2 == 0) ? y : y - 1;
        y = y < 0 ? 0 : y;
        y = y >= height ? height - 2 : y;
        rect_width = (rect_width % 2 == 0) ? rect_width : rect_width - 1;
        rect_width = (rect_width + x) < width ? rect_width : width - x;
        rect_width = rect_width < 0 ? 0 : rect_width;
        rect_height = (rect_height % 2 == 0) ? rect_height : rect_height - 1;
        rect_height = (rect_height + y) < height ? rect_height : height - y;
        rect_height = rect_height < 0 ? 0 : rect_height;

        if (thickness < 0) {
            tmp_x = x;
            tmp_y = y;
            tmp_rect_width = rect_width;
            tmp_rect_height = rect_height;
            nv21_fill_rect(yuv, width, height, x, y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// upper
        } else {
            thickness = (thickness % 2 == 0) ? thickness : thickness + 1;
            thickness = thickness < 0 ? 0 : thickness;

            tmp_x = x;
            tmp_y = y;
            tmp_rect_width = rect_width;
            tmp_rect_height = thickness;
            nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// upper

            tmp_x = x;
            tmp_y = (y + rect_height - thickness) >= 0 ? (y + rect_height - thickness) : 0;
            tmp_rect_width = rect_width;
            tmp_rect_height = thickness;
            nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// lower

            tmp_x = x;
            tmp_y = (y + thickness) < height ? (y + thickness) : height - thickness;
            tmp_rect_width = thickness;
            tmp_rect_height = rect_height - thickness * 2;
            nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// left


            tmp_x = (x + rect_width - thickness) < width ? (x + rect_width - thickness) : width - thickness;
            tmp_y = (y + thickness) < height ? (y + thickness) : height - thickness;
            tmp_rect_width = thickness;
            tmp_rect_height = rect_height - thickness * 2;
            nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// right
        }
    }

    static void _draw_rect_on_nv21(uint8_t *yuv, int width, int height, video_recoder_param_t *param)
    {
        for (size_t i = 0; i < param->rect.size(); i++) {
            if (!param->rect[i].show) {
                continue;
            }

            int x = param->rect[i].x;
            int y = param->rect[i].y;
            int w = param->rect[i].w;
            int h = param->rect[i].h;
            int thickness = param->rect[i].thickness;

            int r = param->rect[i].color.r;
            int g = param->rect[i].color.g;
            int b = param->rect[i].color.b;
            int y_value = (int)(0.299 * r + 0.587 * g + 0.114 * b);
            int u_value = (int)(-0.14713 * r - 0.28886 * g + 0.436 * b) + 128;
            int v_value = (int)(0.615 * r - 0.51499 * g - 0.10001 * b) + 128;
            nv21_draw_rectangle(yuv, width, height, x, y, w, h, y_value, u_value, v_value, thickness);
        }
    }

    static void *record_thread_handle(void *par)
    {
        video_recoder_param_t *param = (video_recoder_param_t *)par;
        VideoRecorder *me = param->obj;
        // uint64_t last_loop_ms = time::ticks_ms();
        while (1) {
            // log::info("[%s] last loop use:%lld ms", __func__, time::ticks_ms() - last_loop_ms);
            // last_loop_ms = time::ticks_ms();
            video_recoder_state_t state = VIDEO_RECORDER_UNKNOWN;
            me->lock();
            if (param->thread_exit_flag) {
                me->unlock();
                break;
            }
            state = param->state;

            switch (state) {
            case VIDEO_RECORDER_IDLE:
            {
                // log::info("VIDEO RECORDER IDLE");
                camera::Camera *cam = param->camera.obj;
                camera::Camera *cam2 = param->camera.obj2;
                display::Display *disp = param->display.obj;
                int disp_fit = param->display.fit;

                if (cam && disp) {
                    int vi_ch = 0;
                    void *cam_frame;
                    mmf_frame_info_t cam_frame_info;
                    (void)cam_frame_info;
                    if (cam) {
                        vi_ch = cam->get_channel();

                        // camera read
                        if (!mmf_vi_frame_pop2(vi_ch, &cam_frame, &cam_frame_info)) {
                            if (cam_frame_info.fmt == PIXEL_FORMAT_NV21) {
                                _draw_rect_on_nv21((uint8_t *)cam_frame_info.data, cam_frame_info.w, cam_frame_info.h, param);
                            }
                            if (disp) {
                                mmf_vo_frame_push2(0, 0, disp_fit, cam_frame);
                            }
                            mmf_vi_frame_free2(vi_ch, &cam_frame);
                        }
                    }

                    if (cam2 && param->snapshot_en) {
                        if (param->snapshot_img) {
                            delete param->snapshot_img;
                            param->snapshot_img = NULL;
                        }
                        try {
                            param->snapshot_img = cam2->read();
                        } catch (const std::exception &e) {
                            param->snapshot_img = NULL;
                        }
                    }

                    time::sleep_ms(5);
                }
                break;
            }
            case VIDEO_RECORDER_RECORD:
            {
                // log::info("VIDEO RECORDER RECORD");
                camera::Camera *cam = param->camera.obj;
                display::Display *disp = param->display.obj;
                audio::Recorder *pcm_recorder = param->audio.obj;
                int disp_fit = param->display.fit;
                err::check_bool_raise(cam, "You need bind a camera or pass in an image!");

                bool found_cam_frame = false;
                bool found_pcm = false;
                Bytes *pcm = NULL;
                bool found_venc_stream = false;
                int vi_ch = cam->get_channel();
                int venc_ch = param->venc.ch;
                void *cam_frame;
                mmf_frame_info_t cam_frame_info;

                // read pcm
                if (pcm_recorder) {
                    // uint64_t t = time::ticks_ms();
                    std::list<Bytes *> *pcm_list = param->packager.pcm_list;
                    AVFrame *audio_frame = param->packager.audio_frame;
                    AVStream *audio_stream = param->packager.audio_stream;
                    AVCodecContext *audio_codec_ctx = param->packager.audio_codec_ctx;
                    SwrContext *swr_ctx = param->packager.swr_ctx;
                    AVFormatContext *outputFormatContext = param->packager.outputFormatContext;
                    AVPacket *audio_packet = param->packager.audio_packet;
                    int pcm_size = audio_frame->sample_rate * audio_frame->channels * av_get_bytes_per_sample(param->packager.audio_format) / param->venc.fps;

                    auto remain_frame_count = pcm_recorder->get_remaining_frames();
                    auto bytes_per_frame = pcm_recorder->frame_size();
                    auto remain_frame_bytes = remain_frame_count * bytes_per_frame;
                    pcm_size = (pcm_size + 1023) & ~1023;
                    if (pcm_size > remain_frame_bytes) {
                        pcm_size = remain_frame_bytes;
                    }
                    pcm = pcm_recorder->record_bytes(pcm_size);
                    // log::info("[%s][%d] pcm read(%d) use %lld ms", __func__, __LINE__, pcm_size, time::ticks_ms() - t);
                    if (pcm) {
                        if (pcm->size() > 0) {
                            found_pcm = true;

                            // audio process
                            if (found_pcm) {
                                // uint64_t t = time::ticks_ms();
                                size_t buffer_size = av_samples_get_buffer_size(NULL, audio_frame->channels, audio_frame->nb_samples, param->packager.audio_format, 1);
                                size_t pcm_remain_len = pcm->data_len;
                                // fill last pcm to buffer_size
                                Bytes *last_pcm = pcm_list->back();
                                if (last_pcm && last_pcm->data_len < buffer_size) {
                                    int temp_size = pcm_remain_len + last_pcm->data_len >= buffer_size ? buffer_size : pcm_remain_len + last_pcm->data_len;
                                    uint8_t *temp = (uint8_t *)malloc(temp_size);
                                    err::check_null_raise(temp, "malloc failed!");
                                    memcpy(temp, last_pcm->data, last_pcm->data_len);
                                    if (pcm_remain_len + last_pcm->data_len < buffer_size) {
                                        memcpy(temp + last_pcm->data_len, pcm->data, pcm_remain_len);
                                        pcm_remain_len = 0;
                                    } else {
                                        memcpy(temp + last_pcm->data_len, pcm->data, buffer_size - last_pcm->data_len);
                                        pcm_remain_len -= (buffer_size - last_pcm->data_len);
                                    }

                                    Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                                    pcm_list->pop_back();
                                    delete last_pcm;
                                    pcm_list->push_back(new_pcm);
                                }

                                // fill other pcm
                                while (pcm_remain_len > 0) {
                                    int temp_size = pcm_remain_len >= buffer_size ? buffer_size : pcm_remain_len;
                                    uint8_t *temp = (uint8_t *)malloc(temp_size);
                                    err::check_null_raise(temp, "malloc failed!");
                                    memcpy(temp, pcm->data + pcm->data_len - pcm_remain_len, temp_size);
                                    pcm_remain_len -= temp_size;

                                    Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                                    pcm_list->push_back(new_pcm);
                                }

                                // audio process
                                while (pcm_list->size() > 0) {
                                    Bytes *pcm = pcm_list->front();
                                    if (pcm) {
                                        if (pcm->data_len == buffer_size) {
                                            // save to mp4
                                            // log::info("pcm data:%p size:%d list_size:%d", pcm->data, pcm->data_len, pcm_list->size());
                                            const uint8_t *in[] = {pcm->data};
                                            uint8_t *out[] = {audio_frame->data[0]};
                                            swr_convert(swr_ctx, out, audio_codec_ctx->frame_size, in, audio_codec_ctx->frame_size);
                                            audio_frame->pts = param->packager.audio_last_pts;
                                            if (avcodec_send_frame(audio_codec_ctx, audio_frame) < 0) {
                                                printf("Error sending audio_frame to encoder.\n");
                                                break;
                                            }

                                            while (avcodec_receive_packet(audio_codec_ctx, audio_packet) == 0) {
                                                audio_packet->stream_index = audio_stream->index;
                                                audio_packet->pts = param->packager.audio_last_pts;
                                                audio_packet->dts = audio_packet->pts;
                                                audio_packet->duration = audio_codec_ctx->frame_size;
                                                param->packager.audio_last_pts = audio_packet->pts + audio_packet->duration;

                                                // std::vector<int> timebase = {audio_stream->time_base.num, audio_stream->time_base.den};
                                                // log::info("[AUDIO] pts:%d duration:%d timebase:%d/%d time:%.2f", audio_packet->pts, audio_packet->duration, audio_stream->time_base.num, audio_stream->time_base.den, timebase_to_ms(timebase, audio_packet->pts) / 1000);
                                                av_interleaved_write_frame(outputFormatContext, audio_packet);
                                                av_packet_unref(audio_packet);
                                            }
                                            pcm_list->pop_front();
                                            delete pcm;
                                        } else {
                                            break;
                                        }
                                    } else {
                                        err::check_raise(err::ERR_RUNTIME, "pcm data error");
                                    }
                                }
                                // log::info("pcm process time:%d", time::ticks_ms() - t);
                            }
                        } else {
                            delete pcm;
                        }
                    }
                }

                // camera read
                if (mmf_vi_frame_pop2(vi_ch, &cam_frame, &cam_frame_info)) {
                    log::error("read camera image failed!\r\n");
                    found_cam_frame = false;
                }
                found_cam_frame = true;

                // draw rect
                if (found_cam_frame) {
                    if (cam_frame_info.fmt == PIXEL_FORMAT_NV21) {
                        _draw_rect_on_nv21((uint8_t *)cam_frame_info.data, cam_frame_info.w, cam_frame_info.h, param);
                    }
                }

                // pop last result of venc
                mmf_stream_t venc_stream;
                if (0 != mmf_venc_pop(venc_ch, &venc_stream)) {
                    found_venc_stream = false;
                }
                found_venc_stream = true;

                // process result of venc
                if (found_venc_stream) {
                    int stream_size = 0;
                    uint8_t *stream_buffer = NULL;
                    encoder_param_t *packager = &param->packager;
                    for (int i = 0; i < venc_stream.count; i ++) {
                        // printf("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                        stream_size += venc_stream.data_size[i];
                    }

                    if (stream_size != 0) {
                        if (!packager->find_sps_pps && venc_stream.count > 2) {
                            packager->find_sps_pps = true;
                        }

                        if (packager->find_sps_pps) {
                            stream_buffer = (uint8_t *)malloc(stream_size);
                            err::check_null_raise(stream_buffer, "recorder malloc failed");
                            int copy_length = 0;
                            for (int i = 0; i < venc_stream.count; i ++) {
                                memcpy(stream_buffer + copy_length, venc_stream.data[i], venc_stream.data_size[i]);
                                copy_length += venc_stream.data_size[i];
                            }

                            if (venc_stream.count > 0) {
                                if (packager->video_packet_list->size() == 0) { // first
                                    packager->video_frame_last_ms = time::ticks_ms();
                                    AVPacket *packet = av_packet_alloc();
                                    err::check_null_raise(packet, "malloc failed!");
                                    packet->stream_index = packager->outputStream->index;
                                    packet->duration = 0;   // determined by the next packet
                                    packet->pts = 0;
                                    packet->dts = 0;
                                    packet->data = stream_buffer;
                                    packet->size = copy_length;
                                    packager->video_packet_list->push_back(packet);
                                    param->seek_ms = 0;
                                } else {
                                    uint64_t curr_ms = time::ticks_ms();
                                    uint64_t video_diff_ms = curr_ms - packager->video_frame_last_ms;
                                    packager->video_frame_last_ms = curr_ms;
                                    AVPacket *last_packet = packager->video_packet_list->back();
                                    AVPacket *packet = av_packet_alloc();
                                    err::check_null_raise(packet, "malloc failed!");
                                    last_packet->duration = (packager->outputStream->time_base.den * video_diff_ms) / (packager->outputStream->time_base.num * 1000);
                                    packet->stream_index = packager->outputStream->index;
                                    packet->pts = last_packet->pts + last_packet->duration;
                                    packet->dts = last_packet->pts + last_packet->duration;
                                    packet->data = stream_buffer;
                                    packet->size = copy_length;
                                    packager->video_packet_list->push_back(packet);
                                    param->seek_ms = timebase_to_ms({packager->outputStream->time_base.num, packager->outputStream->time_base.den}, packet->pts);
                                }
                            }

                            if (packager->video_packet_list->size() > 1) {
                                for (size_t i = 0; i < packager->video_packet_list->size() - 1; i ++) {
                                    AVPacket *packet = packager->video_packet_list->front();
                                    uint8_t *packet_data = packet->data;
                                    err::check_bool_raise(av_interleaved_write_frame(packager->outputFormatContext, packet) >= 0, "av_interleaved_write_frame failed!");
                                    if (packet_data) {
                                        free(packet_data);
                                    }
                                    av_packet_unref(packet);
                                    av_packet_free(&packet);
                                    packager->video_packet_list->pop_front();
                                }
                            }
                        }
                    }

                    if (0 != mmf_venc_free(venc_ch)) {
                        log::error("mmf venc free error!");
                    }
                }

                // push vi to venc
                if (0 != mmf_venc_push2(venc_ch, cam_frame)) {
                    log::error("mmf venc push error!");
                }

                // push image to vo
                if (found_cam_frame && disp) {
                    mmf_vo_frame_push2(0, 0, disp_fit, cam_frame);
                }

                if (found_cam_frame) {
                    mmf_vi_frame_free2(vi_ch, &cam_frame);
                }
                break;
            }
            default:
                sleep(1);
                log::info("VIDEO RECORDER UNKNOWED");
                break;
            }
            me->unlock();
        }

        return NULL;
    }

    err::Err VideoRecorder::open()
    {
        if (_is_opened)
            return err::ERR_NONE;

        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        _video_recoder_config_default(param);

        err::check_bool_raise(0 == pthread_mutex_init(&param->lock, NULL), "pthread mutex init failed!");
        pthread_mutex_unlock(&param->lock);

        // init venc
        if (0 != mmf_init_v2(true)) {
            err::check_raise(err::ERR_RUNTIME, "init mmf failed!");
        }

        param->obj = this;
        param->thread_exit_flag = false;
        param->state = VIDEO_RECORDER_IDLE;
        err::check_bool_raise(!pthread_create((pthread_t *)&param->thread, NULL, record_thread_handle, param), "pthread create failed!");
        _is_opened = true;
        return err::ERR_NONE;
    }

    err::Err VideoRecorder::close()
    {
        if (!_is_opened)
            return err::ERR_NONE;

        reset();

        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        param->thread_exit_flag = true;
        if (param->camera.obj2) {
            delete param->camera.obj2;
            param->camera.obj2 = NULL;
        }
        unlock();

        pthread_join(param->thread, NULL);
        mmf_deinit_v2(false);

        err::check_bool_raise(!pthread_mutex_destroy(&param->lock), "pthread mutex destroy failed!");
        _is_opened = false;
        return err::ERR_NONE;
    }

    err::Err VideoRecorder::lock(int64_t timeout)
    {
        video_recoder_param_t *param = (video_recoder_param_t *)_param;

        int res = -1;
        if (timeout < 0) {
            while (0 != (res = pthread_mutex_lock(&param->lock)));
        } else {
            uint64_t start_ms = time::ticks_ms();
            while (0 != (res = pthread_mutex_lock(&param->lock))) {
                if ((int64_t)(time::ticks_ms() - start_ms) >= timeout) {
                    break;
                } else {
                    time::sleep_ms(1);
                }
            }
        }

        return res == 0 ? err::ERR_NONE : err::ERR_TIMEOUT;
    }

    err::Err VideoRecorder::unlock()
    {
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        int res = pthread_mutex_unlock(&param->lock);
        return res == 0 ? err::ERR_NONE : err::ERR_RUNTIME;
    }

    err::Err VideoRecorder::bind_display(display::Display *display, image::Fit fit)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        param->display.obj = display;
        int new_fit = 0;
        switch (fit) {
        case image::Fit::FIT_FILL: new_fit = 0; break;
        case image::Fit::FIT_CONTAIN: new_fit = 1; break;
        case image::Fit::FIT_COVER: new_fit = 2; break;
        default: new_fit = 2; break;
        }
        param->display.fit = new_fit;
        unlock();
        return param->display.obj ? err::ERR_NONE : err::ERR_ARGS;
    }

    err::Err VideoRecorder::bind_camera(camera::Camera *camera)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        param->camera.obj = camera;

        if (camera) {
            err::check_bool_raise(camera->format() == image::FMT_YVU420SP, "camera format must be FMTYVU420SP");
        }

        unlock();
        return param->camera.obj ? err::ERR_NONE : err::ERR_ARGS;
    }

    err::Err VideoRecorder::bind_audio(audio::Recorder *audio)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        param->audio.obj = audio;
        unlock();
        return param->audio.obj ? err::ERR_NONE : err::ERR_ARGS;
    }

    err::Err VideoRecorder::bind_imu(void *imu)
    {
        err::check_raise(err::Err::ERR_NOT_IMPL, "Not supported");
        // lock();
        // video_recoder_param_t *param = (video_recoder_param_t *)_param;
        // param->imu.obj = imu;
        // param->imu.gcsv = new ext_dev::imu::Gcsv();
        // err::check_null_raise(param->imu.gcsv, "create gcsv failed!");
        // unlock();
        // return param->imu.obj ? err::ERR_NONE : err::ERR_ARGS;
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::reset()
    {
        record_finish();

        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        _video_recoder_config_default(param);
        unlock();
        return err::ERR_NONE;
    }

    err::Err VideoRecorder::config_path(std::string path)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        if (param->state != VIDEO_RECORDER_IDLE) {
            unlock();
            return err::ERR_BUSY;
        }

        param->path = path;
        unlock();
        return err::ERR_NONE;
    }

    std::string VideoRecorder::get_path()
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        std::string path = param->path;
        unlock();
        return path;
    }

    err::Err VideoRecorder::config_snapshot(bool enable, std::vector<int> resolution, image::Format format)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        if (param->state != VIDEO_RECORDER_IDLE) {
            unlock();
            return err::ERR_BUSY;
        }

        camera::Camera *cam = param->camera.obj;
        if (!cam) {
            log::error("You must use the bind_camera interface to bind a Camera object.");
            return err::ERR_RUNTIME;
        }

        if (enable) {
            if (resolution.size() < 2) {
                param->snapshot_res = {cam->width(), cam->height()};
            } else {
                param->snapshot_res = resolution;
            }
            param->snapshot_fmt = format;

            if (param->snapshot_en) {
                if (param->camera.obj2) {
                    delete param->camera.obj2;
                    param->camera.obj2 = NULL;
                }
            }
            param->camera.obj2 = cam->add_channel(param->snapshot_res[0], param->snapshot_res[1], param->snapshot_fmt);
            err::check_null_raise(param->camera.obj2, "camera add channel failed!");

            param->snapshot_en = true;
        } else {
            if (param->camera.obj2) {
                delete param->camera.obj2;
                param->camera.obj2 = NULL;
            }

            if (param->snapshot_img) {
                delete param->snapshot_img;
                param->snapshot_img = NULL;
            }
            param->snapshot_en = false;
        }

        unlock();
        return err::ERR_NONE;
    }

    err::Err VideoRecorder::config_resolution(std::vector<int> resolution)
    {
        if (resolution.size() < 2) return err::ERR_ARGS;

        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        if (param->state != VIDEO_RECORDER_IDLE) {
            unlock();
            return err::ERR_BUSY;
        }

        camera::Camera *cam = param->camera.obj;
        if (!cam) {
            log::error("You must use the bind_camera interface to bind a Camera object.");
            return err::ERR_RUNTIME;
        }

        cam->set_resolution(resolution[0], resolution[1]);

        param->venc.resolution = resolution;
        unlock();
        return err::ERR_NONE;
    }

    std::vector<int> VideoRecorder::get_resolution()
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        std::vector<int> resolution;
        if (param) {
            resolution = param->venc.resolution;
            if (resolution.size() == 0) {
                if (param->camera.obj) {
                    resolution = {param->camera.obj->width(), param->camera.obj->height()};
                }
            }

            err::check_bool_raise(resolution.size() == 2, "You need config resolution!");
        }
        unlock();

        return resolution;
    }

    err::Err VideoRecorder::config_fps(int fps)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        if (param->state != VIDEO_RECORDER_IDLE) {
            unlock();
            return err::ERR_BUSY;
        }

        camera::Camera *cam = param->camera.obj;
        if (!cam) {
            log::error("You must use the bind_camera interface to bind a Camera object.");
            return err::ERR_RUNTIME;
        }

        cam->set_fps(fps);

        param->venc.fps = fps;
        unlock();
        return err::ERR_NONE;
    }

    int VideoRecorder::get_fps()
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        int fps = param->venc.fps;
        unlock();
        return fps;
    }

    err::Err VideoRecorder::config_bitrate(int bitrate)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        if (param->state != VIDEO_RECORDER_IDLE) {
            unlock();
            return err::ERR_BUSY;
        }

        param->venc.bitrate = bitrate;
        unlock();
        return err::ERR_NONE;
    }

    int VideoRecorder::get_bitrate()
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        int bitrate = param->venc.bitrate;
        unlock();
        return bitrate;
    }

    int VideoRecorder::mute(int data)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        int current_mute = 0;
        if (param->audio.obj) {
            if (data >= 0) {
                current_mute = param->audio.obj->mute(data);
            } else {
                current_mute = param->audio.obj->mute();
            }
        }
        unlock();

        return current_mute;
    }

    int VideoRecorder::volume(int data)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        int current_volume = 0;
        if (data >= 0) {
            data = data >= 100 ? 100 : data;
            param->audio.obj->volume(data);
        } else {
            current_volume = param->audio.obj->volume();
        }
        unlock();

        return current_volume;
    }

    int64_t VideoRecorder::seek()
    {
        // Only read param, so don't lock
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        return param->seek_ms;
    }

    // static void *_imu_thread_process(void *arg) {
    //     video_recoder_param_t *param = (video_recoder_param_t *)arg;
    //     ext_dev::imu::IMU *imu = param->imu.obj;
    //     ext_dev::imu::Gcsv *gcsv = param->imu.gcsv;
    //     uint64_t first_write_ms = time::ticks_ms();
    //     while (!app::need_exit()) {
    //         if (param->state == VIDEO_RECORDER_IDLE) {
    //             break;
    //         }

    //         if (imu && gcsv) {
    //             auto res = imu->read();
    //             // log::info("------------------------");
    //             // printf("acc x:  %f\t", res[0]);
    //             // printf("acc y:  %f\t", res[1]);
    //             // printf("acc z:  %f\n", res[2]);
    //             // printf("gyro x: %f\t", res[3]);
    //             // printf("gyro y: %f\t", res[4]);
    //             // printf("gyro z: %f\n", res[5]);
    //             // printf("temp:   %f\n", res[6]);
    //             // log::info("------------------------\n");

    //             double t = (double)(time::ticks_ms() - first_write_ms);
    //             std::vector<double> g = {res[3] * M_PI / 180, res[4] * M_PI / 180, res[5] * M_PI / 180};
    //             std::vector<double> a = {res[0], res[1], res[2]};
    //             gcsv->write(t, g, a);
    //         }

    //         time::sleep_ms(1);
    //     }
    //     return NULL;
    // }

    err::Err VideoRecorder::record_start()
    {
        auto resolution = this->get_resolution();

        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        int width = resolution[0];
        int height = resolution[1];
        image::Format format = image::FMT_YVU420SP;
        int framerate = param->venc.fps;
        int bitrate = param->venc.bitrate;
        std::string path = param->path;
        VideoType type = VIDEO_H264;
        video::VideoType video_type = _get_video_type(param->path.c_str(), type);
        PAYLOAD_TYPE_E venc_type = _video_type_to_mmf(type);

        // ffmpeg init
        av_log_set_callback(custom_log_callback);


        AVFormatContext *outputFormatContext = NULL;
        if (0 != avformat_alloc_output_context2(&outputFormatContext, NULL, NULL, path.c_str())) {
            log::error("Count not open file: %s", path.c_str());
            err::check_raise(err::ERR_RUNTIME, "Could not open file");
        }

        /* video init */
        AVStream *outputStream = avformat_new_stream(outputFormatContext, NULL);
        err::check_null_raise(outputStream, "create new stream failed");

        outputStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        outputStream->codecpar->codec_id = _video_type_to_ffmpeg(video_type);
        outputStream->codecpar->width = width;
        outputStream->codecpar->height = height;
        outputStream->codecpar->format = _image_format_to_ffmpeg(format);
        outputStream->time_base = (AVRational){1, framerate};
        outputStream->codecpar->bit_rate = bitrate;

        if (!(outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&outputFormatContext->pb, path.c_str(), AVIO_FLAG_WRITE) < 0) {
                log::error("Count not open file: %s", path.c_str());
                err::check_raise(err::ERR_RUNTIME, "Could not open file");
            }
        }

        AVPacket *pPacket = av_packet_alloc();
        err::check_null_raise(pPacket, "malloc failed!");

        mmf_venc_cfg_t cfg = {
            .type = 2,  //1, h265, 2, h264
            .w = width,
            .h = height,
            .fmt = mmf_invert_format_to_mmf(image::FMT_YVU420SP),
            .jpg_quality = 0,       // unused
            .gop = 50,
            .intput_fps = framerate,
            .output_fps = framerate,
            .bitrate = bitrate / 1000,
        };

        if (venc_type == PT_H265) {
            cfg.type = 1;
        } else if (venc_type == PT_H264) {
            cfg.type = 2;
        }

        if (0 != mmf_add_venc_channel_v2(MMF_VENC_CHN, &cfg)) {
            mmf_deinit_v2(false);
            err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!");
        }

        /* audio init */
        AVStream *audio_stream = NULL;
        AVCodecContext *audio_codec_ctx = NULL;
        AVCodec *audio_codec = NULL;
        AVFrame *audio_frame = NULL;
        SwrContext *swr_ctx = NULL;
        AVPacket *audio_packet = NULL;
        int audio_sample_rate = 48000;
        int audio_channels = 1;
        int audio_bitrate = 128000;
        enum AVSampleFormat audio_format = AV_SAMPLE_FMT_S16;
        audio::Recorder *audio_recorder = param->audio.obj;
        if (audio_recorder) {
            audio_sample_rate = audio_recorder->sample_rate();
            audio_channels = audio_recorder->channel();
            audio_format = _audio_format_to_alsa(audio_recorder->format());
        }
        audio_packet = av_packet_alloc();
        err::check_null_raise(audio_packet, "av_packet_alloc");
        audio_packet->data = NULL;
        audio_packet->size = 0;

        err::check_null_raise(audio_codec = avcodec_find_encoder(AV_CODEC_ID_AAC), "Could not find aac encoder");
        err::check_null_raise(audio_stream = avformat_new_stream(outputFormatContext, NULL), "Could not allocate stream");
        err::check_null_raise(audio_codec_ctx = avcodec_alloc_context3(audio_codec), "Could not allocate audio codec context");
        audio_codec_ctx->codec_id = AV_CODEC_ID_AAC;
        audio_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_codec_ctx->sample_rate = audio_sample_rate;
        audio_codec_ctx->channels = audio_channels;
        audio_codec_ctx->channel_layout = av_get_default_channel_layout(audio_codec_ctx->channels);
        audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;  // AAC编码需要浮点格式
        audio_codec_ctx->time_base = (AVRational){1, audio_sample_rate};
        audio_codec_ctx->bit_rate = bitrate;
        audio_stream->time_base = audio_codec_ctx->time_base;
        err::check_bool_raise(avcodec_parameters_from_context(audio_stream->codecpar, audio_codec_ctx) >= 0, "avcodec_parameters_to_context");
        err::check_bool_raise(avcodec_open2(audio_codec_ctx, audio_codec, NULL) >= 0, "audio_codec open failed");

        swr_ctx = swr_alloc();
        av_opt_set_int(swr_ctx, "in_channel_layout", audio_codec_ctx->channel_layout, 0);
        av_opt_set_int(swr_ctx, "out_channel_layout", audio_codec_ctx->channel_layout, 0);
        av_opt_set_int(swr_ctx, "in_sample_rate", audio_codec_ctx->sample_rate, 0);
        av_opt_set_int(swr_ctx, "out_sample_rate", audio_codec_ctx->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_format, 0);
        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
        swr_init(swr_ctx);

        int frame_size = audio_codec_ctx->frame_size;
        audio_frame = av_frame_alloc();
        audio_frame->nb_samples = frame_size;
        audio_frame->channel_layout = audio_codec_ctx->channel_layout;
        audio_frame->format = AV_SAMPLE_FMT_FLTP;
        audio_frame->sample_rate = audio_codec_ctx->sample_rate;
        av_frame_get_buffer(audio_frame, 0);

        err::check_bool_raise(avformat_write_header(outputFormatContext, NULL) >= 0, "avformat_write_header failed!");

        /* param init */
        encoder_param_t *packager = (encoder_param_t *)&param->packager;
        packager->outputFormatContext = outputFormatContext;
        packager->outputStream = outputStream;

        // video init
        packager->pPacket = pPacket;
        packager->video_type = video_type;
        packager->venc_ch = MMF_VENC_CHN;
        packager->venc_type = venc_type;
        packager->find_sps_pps = false;
        packager->frame_index = 0;
        packager->last_encode_ms = time::ticks_ms();
        switch (video_type) {
            case VIDEO_H264:
                packager->copy_sps_pps_per_iframe = true;
                break;
            case VIDEO_H264_MP4:
                packager->copy_sps_pps_per_iframe = false;
                break;
            case VIDEO_H264_FLV:
                packager->copy_sps_pps_per_iframe = false;
            break;
            case VIDEO_H265:
                packager->copy_sps_pps_per_iframe = true;
                break;
            case VIDEO_H265_MP4:
                packager->copy_sps_pps_per_iframe = false;
                break;
            default:
                err::check_raise(err::ERR_RUNTIME, "Unsupported video type!");
        }
        packager->video_packet_list = new std::list<AVPacket *>;

        // audio init
        packager->audio_stream = audio_stream;
        packager->audio_codec_ctx = audio_codec_ctx;
        packager->audio_codec = audio_codec;
        packager->audio_frame = audio_frame;
        packager->swr_ctx = swr_ctx;
        packager->audio_packet = audio_packet;
        packager->audio_last_pts = 0;
        packager->pcm_list = new std::list<Bytes *>;
        packager->audio_sample_rate = audio_sample_rate;
        packager->audio_channels = audio_channels;
        packager->audio_bitrate = audio_bitrate;
        packager->audio_format = audio_format;

        param->state = VIDEO_RECORDER_RECORD;
        // // imu thread init
        // if (param->imu.gcsv) {
        //     double t_scale = 0.001;
        //     double g_scale = ((double)1024 * M_PI / 180) / 32768;
        //     double a_scale = ((double)16 / 32768);
        //     std::string::size_type pos = param->path.find('.');
        //     if (pos != std::string::npos) {
        //         param->imu.gcsv_path = param->path.substr(0, pos) + ".gcsv";
        //     } else {
        //         param->imu.gcsv_path = param->path + ".gcsv";
        //     }
        //     param->imu.gcsv->open(param->imu.gcsv_path, t_scale, g_scale, a_scale);
        //     // struct sched_param thread_param;
		// 	// pthread_attr_t thread_attr;
		// 	// pthread_attr_init(&thread_attr);
		// 	// pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
		// 	// pthread_attr_setschedparam(&thread_attr, &thread_param);
		// 	// pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
        //     err::check_bool_raise(!pthread_create((pthread_t *)&param->imu.imu_thread, NULL, _imu_thread_process, param), "imu thread create failed!");
        // }
        unlock();

        return err::ERR_NONE;
    }

    image::Image *VideoRecorder::snapshot()
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        image::Image *new_image = NULL;
        if (param->snapshot_img) {
            new_image = param->snapshot_img->copy();
            delete param->snapshot_img;
            param->snapshot_img = NULL;
        }
        unlock();
        return new_image;
    }

    err::Err VideoRecorder::record_finish()
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        encoder_param_t *packager = &param->packager;

        if (param->state == VIDEO_RECORDER_IDLE) {
            unlock();
            return err::ERR_NONE;
        }
        param->state = VIDEO_RECORDER_IDLE;

        // if (param->imu.gcsv) {
        //     pthread_join(param->imu.imu_thread, NULL);
        //     param->imu.gcsv->close();
        // }

        mmf_del_venc_channel(param->venc.ch);
        av_write_trailer(packager->outputFormatContext);

        for (auto it = packager->video_packet_list->begin(); it != packager->video_packet_list->end(); ++it) {
            AVPacket *packet = *it;
            if (packet) {
                if (packet->data) {
                    free(packet->data);
                    packet->data = nullptr;
                }
                av_packet_unref(packet);
                av_packet_free(&packet);
            }
            it = packager->video_packet_list->erase(it);
        }
        delete packager->video_packet_list;

        for (auto it = packager->pcm_list->begin(); it != packager->pcm_list->end(); ++it) {
            Bytes *pcm = *it;
            delete pcm;
            it = packager->pcm_list->erase(it);
        }
        delete packager->pcm_list;

        av_frame_free(&packager->audio_frame);
        swr_free(&packager->swr_ctx);
        avcodec_free_context(&packager->audio_codec_ctx);

        avformat_close_input(&packager->outputFormatContext);
        if (packager->outputFormatContext && !(packager->outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&packager->outputFormatContext->pb);
        }
        avformat_free_context(packager->outputFormatContext);
        av_packet_unref(packager->pPacket);
        av_packet_free(&packager->pPacket);
        unlock();

        return err::ERR_NONE;
    }

    err::Err VideoRecorder::draw_rect(int id, int x, int y, int w, int h, image::Color color, int thickness, bool hidden)
    {
        lock();
        video_recoder_param_t *param = (video_recoder_param_t *)_param;
        if (id < 0 || id >= (int)param->rect.size()) {
            log::error("draw_rect id %d out of range", id);
            return err::ERR_ARGS;
        }

        if (!param->camera.obj || param->camera.obj->format() != image::Format::FMT_YVU420SP) {
            log::error("Drawing the box is only effective in NV21 format. Ensure that the camera's image format is set to image::Format::FMT_YVU420SP");
            return err::ERR_ARGS;
        }

        param->rect[id].id = id;
        param->rect[id].x = x;
        param->rect[id].y = y;
        param->rect[id].w = w;
        param->rect[id].h = h;
        param->rect[id].color = color;
        param->rect[id].thickness = thickness;
        param->rect[id].show = !hidden;

        unlock();
        return err::ERR_NOT_IMPL;
    }
} // namespace maix::video
