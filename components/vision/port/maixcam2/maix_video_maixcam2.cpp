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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "ax_middleware.hpp"

namespace maix::video
{
    using namespace maix::middleware;

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
            video_type = video::VIDEO_H265;
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

        unique_ptr<maixcam2::SYS> sys;
        unique_ptr<maixcam2::VENC> venc;
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

        _param = (encoder_param_t *)malloc(sizeof(encoder_param_t));
        encoder_param_t *param = (encoder_param_t *)_param;
        err::check_null_raise(_param, "malloc failed!");
        memset(_param, 0, sizeof(encoder_param_t));

        if (_path.size() != 0) {
            // ffmpeg init
            av_log_set_callback(custom_log_callback);
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

            // /* audio init */
            // AVStream *audio_stream = NULL;
            // AVCodecContext *audio_codec_ctx = NULL;
            // AVCodec *audio_codec = NULL;
            // AVFrame *audio_frame = NULL;
            // SwrContext *swr_ctx = NULL;
            // AVPacket *audio_packet = NULL;
            //
            // audio_packet = av_packet_alloc();
            // err::check_null_raise(audio_packet, "av_packet_alloc");
            // audio_packet->data = NULL;
            // audio_packet->size = 0;
            //
            // int sample_rate = 48000;
            // int channels = 1;
            // int bitrate = 128000;
            // enum AVSampleFormat format = AV_SAMPLE_FMT_S16;
            // err::check_null_raise(audio_codec = (AVCodec *)avcodec_find_encoder(AV_CODEC_ID_AAC), "Could not find aac encoder");
            // err::check_null_raise(audio_stream = avformat_new_stream(outputFormatContext, NULL), "Could not allocate stream");
            // err::check_null_raise(audio_codec_ctx = avcodec_alloc_context3(audio_codec), "Could not allocate audio codec context");
            // audio_codec_ctx->codec_id = AV_CODEC_ID_AAC;
            // audio_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
            // audio_codec_ctx->sample_rate = sample_rate;
            // audio_codec_ctx->channels = channels;
            // audio_codec_ctx->channel_layout = av_get_default_channel_layout(audio_codec_ctx->channels);
            // audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;  // AAC编码需要浮点格式
            // audio_codec_ctx->time_base = (AVRational){1, sample_rate};
            // audio_codec_ctx->bit_rate = bitrate;
            // audio_stream->time_base = audio_codec_ctx->time_base;
            // err::check_bool_raise(avcodec_parameters_from_context(audio_stream->codecpar, audio_codec_ctx) >= 0, "avcodec_parameters_to_context");
            // err::check_bool_raise(avcodec_open2(audio_codec_ctx, audio_codec, NULL) >= 0, "audio_codec open failed");
            //
            // swr_ctx = swr_alloc();
            // av_opt_set_int(swr_ctx, "in_channel_layout", audio_codec_ctx->channel_layout, 0);
            // av_opt_set_int(swr_ctx, "out_channel_layout", audio_codec_ctx->channel_layout, 0);
            // av_opt_set_int(swr_ctx, "in_sample_rate", audio_codec_ctx->sample_rate, 0);
            // av_opt_set_int(swr_ctx, "out_sample_rate", audio_codec_ctx->sample_rate, 0);
            // av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", format, 0);
            // av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
            // swr_init(swr_ctx);
            //
            // int frame_size = audio_codec_ctx->frame_size;
            // audio_frame = av_frame_alloc();
            // audio_frame->nb_samples = frame_size;
            // audio_frame->channel_layout = audio_codec_ctx->channel_layout;
            // audio_frame->format = AV_SAMPLE_FMT_FLTP;
            // audio_frame->sample_rate = audio_codec_ctx->sample_rate;
            // av_frame_get_buffer(audio_frame, 0);

            err::check_bool_raise(avformat_write_header(outputFormatContext, NULL) >= 0, "avformat_write_header failed!");

            param->outputFormatContext = outputFormatContext;
            param->outputStream = outputStream;
            param->pPacket = pPacket;

            // param->audio_stream = audio_stream;
            // param->audio_codec_ctx = audio_codec_ctx;
            // param->audio_codec = audio_codec;
            // param->audio_frame = audio_frame;
            // param->swr_ctx = swr_ctx;
            // param->audio_packet = audio_packet;
        }

        param->video_type = video_type;
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

        ax_venc_param_t cfg = {0};
        switch (video_type) {
            case VIDEO_H264:
            case VIDEO_H264_MP4:
            case VIDEO_H264_FLV:
                cfg.w = width;
                cfg.h = height;
                cfg.fmt = maixcam2::get_ax_fmt_from_maix(format);
                cfg.type = AX_VENC_TYPE_H264;
                cfg.h264.bitrate = bitrate / 1000;
                cfg.h264.input_fps = framerate;
                cfg.h264.output_fps = framerate;
                cfg.h264.gop = gop;
                cfg.h264.intra_qp_delta = -2;
                cfg.h264.de_breath_qp_delta = -2;
                cfg.h264.min_qp = 10;
                cfg.h264.max_qp = 51;
                cfg.h264.min_iqp = 10;
                cfg.h264.max_iqp = 51;
                cfg.h264.min_iprop = 10;
                cfg.h264.max_iprop = 40;
                cfg.h264.first_frame_start_qp = -1;
                break;
            case VIDEO_H265:
            case VIDEO_H265_MP4:
                cfg.w = width;
                cfg.h = height;
                cfg.fmt = maixcam2::get_ax_fmt_from_maix(format);
                cfg.type = AX_VENC_TYPE_H265;
                cfg.h265.bitrate = bitrate / 1000;
                cfg.h265.input_fps = framerate;
                cfg.h265.output_fps = framerate;
                cfg.h265.gop = gop;
                cfg.h265.intra_qp_delta = -2;
                cfg.h264.de_breath_qp_delta = -2;
                cfg.h265.min_qp = 10;
                cfg.h265.max_qp = 51;
                cfg.h265.min_iqp = 10;
                cfg.h265.max_iqp = 51;
                cfg.h265.min_iprop = 30;
                cfg.h265.max_iprop = 40;
                cfg.h265.first_frame_start_qp = -1;
                cfg.h265.qp_delta_rgn = 10;
                cfg.h265.qp_map_type = AX_VENC_QPMAP_QP_DISABLE;
                cfg.h265.qp_map_blk_type = AX_VENC_QPMAP_BLOCK_DISABLE;
                cfg.h265.qp_map_block_unit = AX_VENC_QPMAP_BLOCK_UNIT_64x64;
                cfg.h265.ctb_rc_mode = AX_VENC_RC_CTBRC_DISABLE;
                break;
            default:
                err::check_raise(err::ERR_RUNTIME, "unsupport stream type!");
        }
        param->sys = make_unique<maixcam2::SYS>(false);
        param->venc = make_unique<maixcam2::VENC>(&cfg);

        // audio init
        param->audio_last_pts = 0;
        param->pcm_list = new std::list<Bytes *>;
        param->audio_sample_rate = 48000;
        param->audio_channels = 1;
        param->audio_bitrate = 128000;
        param->audio_format = AV_SAMPLE_FMT_S16;
    }

    Encoder::~Encoder() {
        encoder_param_t *param = (encoder_param_t *)_param;
        param->venc = nullptr;      // release venc
        param->sys = nullptr;
        if (_path.size() == 0) {
            if (_capture_image && _capture_image->data()) {
                delete _capture_image;
                _capture_image = nullptr;
            }
        } else {
            if (param) {
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
        auto err = err::ERR_NONE;
        auto param = (encoder_param_t *)_param;
        auto need_save = false;
        auto curr_ms = time::ticks_ms();
        video::Frame *frame = nullptr;

        // Check if need save frame to file
        if (_path.size() == 0) {
            need_save = false;
        } else {
            need_save = true;
        }

        // Pop last frame
        auto pop_frame = param->venc->pop(30);
        if (pop_frame) {
            AX_VENC_STREAM_T stream;
            if (err::ERR_NONE != pop_frame->get_venc_stream(&stream)) {
                log::error("get venc stream failed!");
                delete pop_frame;
                return nullptr;
            }

            // for (size_t i = 0; i < stream.stPack.u32NaluNum; i ++) {
            //     log::info("[%d] type:%d oft:%d size:%d", i, stream.stPack.stNaluInfo[i].unNaluType, stream.stPack.stNaluInfo[i].u32NaluOffset, stream.stPack.stNaluInfo[i].u32NaluLength);
            // }

            if (need_save) {
                bool is_first_frame = false;
                if (!param->find_sps_pps && stream.stPack.u32NaluNum > 1) {
                    is_first_frame = true;
                    param->find_sps_pps = true;
                    param->video_frame_last_ms = curr_ms;
                }

                if (param->find_sps_pps) {
                    auto frame_buffer = (uint8_t *)av_malloc(pop_frame->len);
                    if (!frame_buffer) {
                        log::error("av malock failed!");
                        delete pop_frame;
                        return nullptr;
                    }

                    memcpy(frame_buffer, (uint8_t *)pop_frame->data, pop_frame->len);

                    if (is_first_frame) {
                        param->pPacket->stream_index = param->outputStream->index;
                        param->pPacket->duration = -1;
                        param->pPacket->pts = 0;
                        param->pPacket->dts = 0;
                        param->pPacket->data = frame_buffer;
                        param->pPacket->size = pop_frame->len;
                    } else {
                        std::vector<int> timebase = {param->outputStream->time_base.num, param->outputStream->time_base.den};
                        AVPacket *new_packet = av_packet_alloc();
                        err::check_null_raise(new_packet, "malloc failed!");
                        new_packet->stream_index = param->outputStream->index;
                        new_packet->duration = -1;
                        new_packet->pts = ms_to_pts(timebase, curr_ms - param->video_frame_last_ms);
                        new_packet->dts = new_packet->pts;
                        new_packet->data = frame_buffer;
                        new_packet->size = pop_frame->len;

                        param->pPacket->duration = new_packet->pts - param->pPacket->pts;
                        auto last_frame_buffer = param->pPacket->data;

                        log::info("[VIDEO] pts:%d duration:%d(%d ms) time:%.2f", param->pPacket->pts, param->pPacket->duration, curr_ms - param->video_frame_last_ms, timebase_to_ms(timebase, param->pPacket->pts) / 1000);
                        if (av_interleaved_write_frame(param->outputFormatContext, param->pPacket) < 0) {
                            log::error("av_interleaved_write_frame failed!");
                            delete pop_frame;
                            return nullptr;
                        }
                        if (last_frame_buffer) {
                            av_free(last_frame_buffer);
                        }
                        av_packet_unref(param->pPacket);
                        av_packet_free(&param->pPacket);
                        param->pPacket = new_packet;
                    }
                }
            }

            frame = new video::Frame((uint8_t *)pop_frame->data, pop_frame->len, 0, 0, 0, true, true);
            delete pop_frame;
        }

        auto new_frame = maixcam2::Frame(-1, img->width(), img->height(), img->data(), img->data_size(), AX_FORMAT_YUV420_SEMIPLANAR_VU);
        err = param->venc->push(&new_frame, 1000);
        if (err != err::ERR_NONE) {
            log::error("encode failed! err:%d", err);
            return nullptr;
        }

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
        // AVBSFContext * bsfc;
        AVCodecContext *codec_ctx;
        int video_stream_index;
        video_format_e video_format;
        int vdec_ch;
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

    // static video_format_e _get_video_format(const char *filename, PAYLOAD_TYPE_E type) {
    //     video_format_e format = VIDEO_FORMAT_NONE;
    //     const char *suffix = strrchr(filename, '.');
    //     err::check_null_raise((void *)suffix, "Try a file format with a suffix, e.g. video.h264/video.mp4/video.flv");

    //     if (!strcmp(suffix, ".h264")) {
    //         format = VIDEO_FORMAT_H264;
    //     } else if (!strcmp(suffix, ".mp4")) {
    //         // if (type == PT_H264) {
    //         //     format = VIDEO_FORMAT_H264_MP4;
    //         // }
    //     } else if (!strcmp(suffix, ".flv")) {
    //         // if (type == PT_H264) {
    //         //     format = VIDEO_FORMAT_H264_FLV;
    //         // }
    //     } else {
    //         err::check_raise(err::ERR_RUNTIME, "Currently only support avc/avc-mp4/avc-flv format!");
    //     }

    //     err::check_bool_raise(format != VIDEO_FORMAT_NONE, "Not found a valid video format!");
    //     return format;
    // }


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
        return nullptr;
    }

    video::Context *Decoder::decode_audio() {
        return nullptr;
    }

    video::Context *Decoder::decode(bool block) {
        return nullptr;
    }

    video::Context *Decoder::unpack() {
        return nullptr;
    }

    double Decoder::seek(double time) {
        return 0;
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

        this->_is_opened = true;
        return err::ERR_NONE;
    }

    void Video::close()
    {

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
        return nullptr;
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
        return NULL;
    }

    err::Err VideoRecorder::open()
    {

        return err::ERR_NONE;
    }

    err::Err VideoRecorder::close()
    {

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
