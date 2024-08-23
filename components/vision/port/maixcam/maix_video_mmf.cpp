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
#include <libavutil/opt.h>
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

    typedef enum {
        VIDEO_FORMAT_NONE,
        VIDEO_FORMAT_H264,
        VIDEO_FORMAT_H264_MP4,
        VIDEO_FORMAT_H264_FLV,
        VIDEO_FORMAT_MAX
    } video_format_e;

    typedef struct {
        AVFormatContext *pFormatContext;
        AVPacket *pPacket;
        AVBSFContext * bsfc;
        int video_stream_index;
        video_format_e video_format;

        int vdec_ch;
        PAYLOAD_TYPE_E vdec_type;
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

    static void custom_log_callback(void* ptr, int level, const char* fmt, va_list vargs) {
        if (level > AV_LOG_ERROR) {
            return;
        }
    }

    Decoder::Decoder(std::string path, image::Format format) {
        av_log_set_callback(custom_log_callback);
        err::check_bool_raise(format == image::Format::FMT_YVU420SP, "Decoder only support FMT_YVU420SP format!");
        _path = path;
        _format_out = format;
        _param = (decoder_param_t *)malloc(sizeof(decoder_param_t));
        memset(_param, 0, sizeof(decoder_param_t));

        PAYLOAD_TYPE_E vdec_type = PT_H264;
        video_format_e video_format = VIDEO_FORMAT_H264;
        err::check_null_raise(_param, "malloc failed!");
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

        // Find video stream
        int video_stream_index = -1;
        for (int i = 0; i < (int)pFormatContext->nb_streams; i++) {
            if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_stream_index = i;
                break;
            }
        }
        err::check_bool_raise(video_stream_index != -1, "Could not find video stream");

        // Check encode type
        AVCodecParameters *codec_params = pFormatContext->streams[video_stream_index]->codecpar;
        err::check_bool_raise(codec_params->codec_id == AV_CODEC_ID_H264, "Only support h264 encode video format!");
        vdec_type = PT_H264;
        video_format = _get_video_format(_path.c_str(), vdec_type);

        // Get video width/height
        _width = codec_params->width;
        _height = codec_params->height;
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
        vdec_chn_attr.enType = vdec_type;
        vdec_chn_attr.enMode = VIDEO_MODE_FRAME;
        vdec_chn_attr.u32PicWidth = _width;
        vdec_chn_attr.u32PicHeight = _height;
        vdec_chn_attr.u32FrameBufCnt = 3;
        vdec_chn_attr.u32StreamBufSize = _width * _height;
        err::check_bool_raise(!mmf_add_vdec_channel_v2(ch, mmf_invert_format_to_mmf(format), 4, &vdec_chn_attr), "mmf_add_vdec_channel_v2 failed");

        AVBSFContext * bsfc;
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

        decoder_param_t *param = (decoder_param_t *)_param;
        param->pFormatContext = pFormatContext;
        param->pPacket = pPacket;
        param->bsfc = bsfc;
        param->video_stream_index = video_stream_index;
        param->video_format = video_format;
        param->vdec_ch = ch;
        param->vdec_type = vdec_type;
    }

    Decoder::~Decoder() {
        decoder_param_t *param = (decoder_param_t *)_param;
        if (param) {
            if (param->vdec_ch >= 0) {
                mmf_del_vdec_channel(param->vdec_ch);
            }

            av_packet_free(&param->pPacket);
            avformat_close_input(&param->pFormatContext);

            if (param->bsfc) {
                av_bsf_free(&param->bsfc);
            }
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
        AVBSFContext * bsfc = param->bsfc;
        int video_stream_index = param->video_stream_index;
        image::Image *img = NULL;
        while (av_read_frame(pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == video_stream_index) {
                _last_pts = pPacket->pts;

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
                err::check_bool_raise(!mmf_vdec_push(param->vdec_ch, pPacket->data, pPacket->size, 0, 0));
                VIDEO_FRAME_INFO_S frame;
                err::check_bool_raise(!mmf_vdec_pop_v2(param->vdec_ch, &frame));
                img = _mmf_frame_to_image(&frame, _format_out);
                err::check_bool_raise(!mmf_vdec_free(param->vdec_ch));
                av_packet_unref(pPacket);
                break;
            }
            av_packet_unref(pPacket);
        }
        return img;
    }

    err::Err Decoder::seek(double time) {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVFormatContext *pFormatContext = param->pFormatContext;
        video_format_e video_format = param->video_format;
        int video_stream_index = param->video_stream_index;
        int64_t seek_target = av_rescale_q(time * AV_TIME_BASE, AV_TIME_BASE_Q, pFormatContext->streams[video_stream_index]->time_base);
        if (video_format != VIDEO_FORMAT_H264_FLV && video_format != VIDEO_FORMAT_H264_MP4) {
            return err::ERR_RUNTIME;
        }
        int ret = av_seek_frame(pFormatContext, video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            avformat_close_input(&param->pFormatContext);
            log::error("av_seek_frame failed, ret:%d", ret);
            return err::ERR_RUNTIME;
        }

        return err::ERR_NONE;
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

    double Decoder::last_pts() {
        decoder_param_t *param = (decoder_param_t *)_param;
        AVFormatContext *pFormatContext = param->pFormatContext;
        int video_stream_index = param->video_stream_index;
        double last_pts = _last_pts * av_q2d(pFormatContext->streams[video_stream_index]->time_base);
        return last_pts;
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
