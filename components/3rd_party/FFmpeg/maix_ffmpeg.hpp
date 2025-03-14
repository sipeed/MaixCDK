#ifndef __MAIX_FFMPEG_HPP
#define __MAIX_FFMPEG_HPP

extern "C" {
#include <unistd.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}
#include <list>
#include <string>
#include <vector>



namespace maix::ffmpeg {
using namespace std;
class FFmpegPacker {
    class Bytes
    {
    public:
        Bytes(uint8_t *data, uint32_t len, bool auto_detele = false, bool copy = true)
        {
            this->data = data;
            this->data_len = len;
            this->buff_len = len;
            this->_is_alloc = auto_detele;
            if(len > 0)
            {
                if(data && copy)
                {
                    this->data = new uint8_t[this->buff_len];
                    this->_is_alloc = true;
                    memcpy(this->data, data, this->buff_len);
                }
                else if (!data && this->buff_len > 0)
                {
                    this->data = new uint8_t[this->buff_len];
                    this->_is_alloc = true;
                }
            }
        }

        Bytes()
        {
            this->data = NULL;
            this->buff_len = 0;
            this->data_len = 0;
            this->_is_alloc = false;
        }

        ~Bytes()
        {
            if (_is_alloc && data)
            {
                delete[] data;
            }
        }

        Bytes &operator=(const Bytes &other)
        {
            if (this != &other)
            {
                if (_is_alloc && data)
                {
                    delete[] data;
                }
                // alloc new buffer and copy
                this->data = new uint8_t[other.buff_len];
                this->buff_len = other.buff_len;
                this->data_len = other.data_len;
                this->_is_alloc = true;
                memcpy(this->data, other.data, this->buff_len);
            }
            return *this;
        }

        uint8_t at(int index) const
        {
            if (index < 0 || index >= (int)this->data_len)
            {
                return 0;
            }
            return this->data[index];
        }

        uint8_t operator[](int index) const
        {
            if (index < 0 || index >= (int)this->data_len)
            {
                return 0;
            }
            return this->data[index];
        }

        size_t size() const
        {
            return this->data_len;
        }

        uint8_t *begin()
        {
            return this->data;
        }

        uint8_t *end()
        {
            return this->data + this->data_len;
        }

        uint8_t *data;
        size_t buff_len;
        size_t data_len;
    private:
        bool _is_alloc;
    };

    bool _open = false;
    size_t _last_pts;
    int _stream_index;
    std::string _path;
    std::string _context_format_name;
    AVFormatContext *_format_context = nullptr;

    /* video parameter */
    bool _has_video = false;
    AVStream *_video_stream = nullptr;
    enum AVCodecID _video_codec_id;
    int _video_width;
    int _video_height;
    enum AVPixelFormat _vidoe_pixel_format;
    uint32_t _video_bitrate;
    AVRational _video_timebase;
    uint8_t *_video_sps_pps = nullptr;
    size_t _video_sps_pps_size = 0;
    size_t _video_last_pts;

    /* audio parameter */
    bool _has_audio = false;
    AVStream *_audio_stream = nullptr;
    AVCodecContext *_audio_codec_ctx;
    SwrContext *_audio_swr_ctx;
    AVCodec *_audio_codec = nullptr;
    AVFrame *_audio_frame;
    int _audio_sample_rate = 48000;
    int _audio_channels = 1;
    int _audio_bitrate = 128000;
    enum AVSampleFormat _audio_format = AV_SAMPLE_FMT_S16;
    std::list<std::pair<size_t, Bytes *>> *_pcm_list;
public:
    // pack video(h264) and audio(acc), then save to dist path
    FFmpegPacker() {

    }

    ~FFmpegPacker() {
        if (_open) {
            this->close();
        }
    }
    int config(std::string cmd, int data)
    {
        if (cmd == "has_video") {
            _has_video = data ? true : false;
            return 0;
        } else if (cmd == "video_codec_id") {
            _video_codec_id = (AVCodecID)data;
            return 0;
        } else if (cmd == "video_width") {
            _video_width = data;
            return 0;
        } else if (cmd == "video_height") {
            _video_height = data;
            return 0;
        } else if (cmd == "video_bitrate") {
            _video_bitrate = data;
            return 0;
        } else if (cmd == "video_fps") {
            _video_timebase = (AVRational){1, data};
            return 0;
        } else if (cmd == "video_pixel_format") {
            _vidoe_pixel_format = (AVPixelFormat)data;
            return 0;
        } else if (cmd == "has_audio") {
            _has_audio = data ? true : false;
            return 0;
        } else if (cmd == "audio_sample_rate") {
            _audio_sample_rate = data;
            return 0;
        } else if (cmd == "audio_channels") {
            _audio_channels = data;
            return 0;
        } else if (cmd == "audio_bitrate") {
            _audio_bitrate = data;
            return 0;
        } else if (cmd == "audio_format") {
            _audio_format = (enum AVSampleFormat)data;
            return 0;
        } else {
            return -1;
        }
        return -1;
    }

    int config2(std::string cmd, std::string str) {
        if (cmd == "path") {
            _path = str;
            return 0;
        } else if (cmd == "context_format_name") {
            _context_format_name = str;
            return 0;
        } else {
            return -1;
        }

        return -1;
    }

    int config_sps_pps(uint8_t *sps_pps, int sps_pps_size)
    {
        if (_video_sps_pps) {
            free(_video_sps_pps);
            _video_sps_pps = NULL;
        }
        _video_sps_pps = (uint8_t *)malloc(sps_pps_size);
        if (!_video_sps_pps) {
            return -1;
        }
        memcpy(_video_sps_pps, sps_pps, sps_pps_size);
        _video_sps_pps_size = sps_pps_size;
        return 0;
    }

    bool is_opened() {return _open;};

    int open() {
        if (_open) return 0;

        AVFormatContext *format_context = NULL;
        const char *context_format_name;
        if (_context_format_name.size() == 0) {
            context_format_name = NULL;
        } else {
            context_format_name = _context_format_name.c_str();
        }
        avformat_alloc_output_context2(&format_context, NULL, context_format_name, _path.c_str());
        if (!format_context) {
			printf("Can't create output context\r\n");
            return -1;
        }

        if (_has_video) {
            AVStream *video_stream = avformat_new_stream(format_context, NULL);
            if (!video_stream) {
				printf("Can't create video stream\r\n");
                goto _free_format_context;
            }
            video_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
            video_stream->codecpar->codec_id = _video_codec_id;
            video_stream->codecpar->width = _video_width;
            video_stream->codecpar->height = _video_height;
            video_stream->codecpar->format = _vidoe_pixel_format;
            video_stream->codecpar->bit_rate = _video_bitrate;
            video_stream->codecpar->codec_tag = 0;
            video_stream->codecpar->extradata = _video_sps_pps;
            video_stream->codecpar->extradata_size = _video_sps_pps_size;
            video_stream->time_base = _video_timebase;
            if (video_stream->codecpar->extradata == NULL ||
                video_stream->codecpar->extradata_size == 0) {
				printf("video sps pps is null, size is 0\r\n");
                goto _free_format_context;
            }

            _video_sps_pps = nullptr;
            _video_sps_pps_size = 0;
            _video_stream = video_stream;
        }

        if (_has_audio) {
            int sample_rate = _audio_sample_rate;
            int channels = _audio_channels;
            int bitrate = _audio_bitrate;
            enum AVSampleFormat format = _audio_format;

            AVStream *audio_stream = avformat_new_stream(format_context, NULL);
            if (!audio_stream) {
				printf("Can't create audio stream\r\n");
                goto _free_format_context;
            }

            AVCodec *audio_codec = (AVCodec *)avcodec_find_encoder(AV_CODEC_ID_AAC);
            if (!audio_codec) {
				printf("Can't find audio encoder\r\n");
                goto _free_format_context;
            }

            AVCodecContext *audio_codec_ctx = avcodec_alloc_context3(audio_codec);
            if (!audio_codec_ctx) {
				printf("Can't alloc audio codec context\r\n");
                goto _free_format_context;
            }
            audio_codec_ctx->codec_id = AV_CODEC_ID_AAC;
            audio_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
            audio_codec_ctx->sample_rate = sample_rate;
            audio_codec_ctx->channels = channels;
            audio_codec_ctx->channel_layout = av_get_default_channel_layout(audio_codec_ctx->channels);
            audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
            audio_codec_ctx->time_base = (AVRational){1, sample_rate};
            audio_codec_ctx->bit_rate = bitrate;
            audio_codec_ctx->profile = FF_PROFILE_AAC_LOW;
            audio_stream->time_base = audio_codec_ctx->time_base;

            if (0 > avcodec_open2(audio_codec_ctx, audio_codec, NULL)) {
				printf("Can't open audio codec\r\n");
                goto _free_audio_codec_ctx;
            }

            if (0 > avcodec_parameters_from_context(audio_stream->codecpar, audio_codec_ctx)) {
				printf("Can't copy audio codec parameters\r\n");
                goto _free_audio_codec_ctx;
            }

            SwrContext *swr_ctx = swr_alloc();
            if (!swr_ctx) {
				printf("Can't alloc swr context\r\n");
                goto _free_audio_codec_ctx;
            }

            av_opt_set_int(swr_ctx, "in_channel_layout", audio_codec_ctx->channel_layout, 0);
            av_opt_set_int(swr_ctx, "out_channel_layout", audio_codec_ctx->channel_layout, 0);
            av_opt_set_int(swr_ctx, "in_sample_rate", audio_codec_ctx->sample_rate, 0);
            av_opt_set_int(swr_ctx, "out_sample_rate", audio_codec_ctx->sample_rate, 0);
            av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", format, 0);
            av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
            swr_init(swr_ctx);

            AVFrame *audio_frame = av_frame_alloc();
            if (!audio_frame) {
				printf("Can't alloc audio frame\r\n");
                swr_free(&swr_ctx);
                swr_ctx = NULL;
                goto _free_swr_ctx;
            }
            audio_frame->nb_samples = audio_codec_ctx->frame_size;
            audio_frame->channel_layout = audio_codec_ctx->channel_layout;
            audio_frame->format = AV_SAMPLE_FMT_FLTP;
            audio_frame->sample_rate = audio_codec_ctx->sample_rate;
            av_frame_get_buffer(audio_frame, 0);

            _audio_stream = audio_stream;
            _audio_codec = audio_codec;
            _audio_codec_ctx = audio_codec_ctx;
            _audio_swr_ctx = swr_ctx;
            _audio_frame = audio_frame;
            _audio_sample_rate = sample_rate;
            _audio_channels = channels;
            _audio_bitrate = bitrate;
            _audio_format = format;
        }

        if (_path.length() > 0) {
            if (avio_open(&format_context->pb, _path.c_str(), AVIO_FLAG_WRITE) < 0) {
                printf("avio open failed!\r\n");
                goto _free_audio_frame;
            }

            if (avformat_write_header(format_context, NULL) < 0) {
                printf("avformat write header failed!\r\n");
                goto _close_io;
            }
        }

        _format_context = format_context;
        _open = true;
        _video_last_pts = 0;
        _pcm_list = new std::list<std::pair<size_t, Bytes *>>;
        return 0;
_close_io:
        if (_format_context && _format_context->pb) {
            avio_closep(&_format_context->pb);
			_format_context->pb = nullptr;
        }
_free_audio_frame:
        if (_audio_frame) {
            av_frame_free(&_audio_frame);
            _audio_frame = nullptr;
        }
_free_swr_ctx:
        if (_audio_swr_ctx) {
            swr_free(&_audio_swr_ctx);
            _audio_swr_ctx = nullptr;
        }
_free_audio_codec_ctx:
        if (_audio_codec_ctx) {
            avcodec_free_context(&_audio_codec_ctx);
            _audio_codec_ctx = nullptr;
        }
_free_format_context:
        if (_format_context) {
            avformat_free_context(_format_context);
			_format_context = nullptr;
        }
        return -1;
    }

    void reset() {
        if (_pcm_list) {
            for (auto it = _pcm_list->begin(); it != _pcm_list->end(); ++it) {
                auto &item = *it;
                Bytes *pcm = item.second;
                delete pcm;
                it = _pcm_list->erase(it);
            }
        }
    }

    void close() {
        if (!_open) return;

        if (_pcm_list) {
            for (auto it = _pcm_list->begin(); it != _pcm_list->end(); ++it) {
                auto &item = *it;
                Bytes *pcm = item.second;
                delete pcm;
                it = _pcm_list->erase(it);
            }
            delete _pcm_list;
            _pcm_list = nullptr;
        }

        if (_audio_frame) {
            av_frame_free(&_audio_frame);
            _audio_frame = nullptr;
        }

        if (_audio_swr_ctx) {
            swr_free(&_audio_swr_ctx);
            _audio_swr_ctx = nullptr;
        }

        if (_audio_codec_ctx) {
            avcodec_free_context(&_audio_codec_ctx);
            _audio_codec_ctx = nullptr;
        }

        if (_format_context) {
            if (_path.length() > 0) {
                av_write_trailer(_format_context);
            }

            // av_write_trailer(_format_context);
            if (_format_context && _format_context->pb) {
                avio_closep(&_format_context->pb);
            }
            avformat_free_context(_format_context);
            _format_context = NULL;
        }

        _open = false;
        _video_last_pts = 0;
    }

    std::vector<unsigned char> pack_pcm(uint8_t *frame, size_t frame_size) {
        std::vector<unsigned char> output_data;
        if (!_open) {
            return output_data;
        }

        if (_has_audio) {
            AVPacket *pkt = av_packet_alloc();
            if (!pkt) {
                fprintf(stderr, "Can't malloc avpacket\r\n");
                return output_data;
            }

            if (frame && frame_size > 0) {
                AVFrame *audio_frame = _audio_frame;
                AVCodecContext *audio_codec_ctx = _audio_codec_ctx;
                SwrContext *swr_ctx = _audio_swr_ctx;
                AVPacket *audio_packet = pkt;

                const uint8_t *in[] = {frame};
                uint8_t *out[] = {audio_frame->data[0]};
                swr_convert(swr_ctx, out, audio_codec_ctx->frame_size, in, audio_codec_ctx->frame_size);
                if (avcodec_send_frame(audio_codec_ctx, audio_frame) < 0) {
                    printf("Error send audio_frame to encoder.\n");
                    return output_data;
                }

                if (avcodec_receive_packet(audio_codec_ctx, audio_packet) != 0) {
                    printf("Error receive audio_frame to encoder.\n");
                    return output_data;
                }

                output_data.resize(audio_packet->size);
                memcpy(output_data.data(), audio_packet->data, audio_packet->size);
                av_packet_unref(audio_packet);
            }

            av_packet_unref(pkt);
            av_packet_free(&pkt);
        }

        return output_data;
    }

    int push(uint8_t *frame, size_t frame_size, uint64_t pts, bool is_audio = false)
    {
        if (!_open) {
            return -1;
        }

        if (!is_audio) {
            if (_has_video) {
                AVPacket *pkt = av_packet_alloc();
                if (!pkt) {
                    fprintf(stderr, "Can't malloc avpacket\r\n");
                    return -1;
                }

                pkt->data = frame;
                pkt->size = frame_size;
                pkt->stream_index = _video_stream->index;
                pkt->duration = pts - _video_last_pts;
                _video_last_pts = pts;
                pkt->pts = pkt->dts = pts;
                pkt->flags |= AV_PKT_FLAG_KEY;
                // log::info("[VIDEO] frame:%p frame_size:%d pts:%ld(%f s)", frame, frame_size, pkt->pts, this->video_pts_to_us(pkt->pts) / 1000);
                if (av_interleaved_write_frame(_format_context, pkt) < 0) {
                    fprintf(stderr, "send frame failed!\r\n");
                    av_packet_unref(pkt);
                    av_packet_free(&pkt);
                    return -1;
                }

                av_packet_unref(pkt);
                av_packet_free(&pkt);
            }
        } else {
            if (_has_audio) {
                AVPacket *pkt = av_packet_alloc();
                if (!pkt) {
                    fprintf(stderr, "Can't malloc avpacket\r\n");
                    return -1;
                }

                if (frame && frame_size > 0) {
                    auto pcm_list = _pcm_list;
                    AVFrame *audio_frame = _audio_frame;
                    AVStream *audio_stream = _audio_stream;
                    AVCodecContext *audio_codec_ctx = _audio_codec_ctx;
                    SwrContext *swr_ctx = _audio_swr_ctx;
                    AVFormatContext *outputFormatContext = _format_context;
                    AVPacket *audio_packet = pkt;
                    size_t buffer_size = av_samples_get_buffer_size(NULL, _audio_channels, audio_frame->nb_samples, _audio_format, 1);
                    size_t pcm_remain_len = frame_size;

                    // fill last pcm to buffer_size
                    size_t next_pts = pts;
                    if (!pcm_list->empty()) {
                        auto last_item = pcm_list->back();
                        Bytes *last_pcm = last_item.second;
                        if (last_pcm && last_pcm->data_len < buffer_size) {
                            int temp_size = pcm_remain_len + last_pcm->data_len >= buffer_size ? buffer_size : pcm_remain_len + last_pcm->data_len;
                            uint8_t *temp = (uint8_t *)malloc(temp_size);
                            if (!temp) {
                                fprintf(stderr, "malloc failed!\r\n");
                                return -1;
                            }
                            memcpy(temp, last_pcm->data, last_pcm->data_len);
                            if (pcm_remain_len + last_pcm->data_len < buffer_size) {
                                memcpy(temp + last_pcm->data_len, frame, pcm_remain_len);
                                pcm_remain_len = 0;
                            } else {
                                memcpy(temp + last_pcm->data_len, frame, buffer_size - last_pcm->data_len);
                                pcm_remain_len -= (buffer_size - last_pcm->data_len);
                            }

                            Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                            pcm_list->pop_back();
                            delete last_pcm;

                            size_t new_pts = last_item.first;
                            next_pts = new_pts + get_audio_pts_from_pcm_size(new_pcm->data_len);
                            pcm_list->push_back(std::make_pair(new_pts, new_pcm));
                        }
                    }

                    // fill other pcm
                    while (pcm_remain_len > 0) {
                        int temp_size = pcm_remain_len >= buffer_size ? buffer_size : pcm_remain_len;
                        uint8_t *temp = (uint8_t *)malloc(temp_size);
                        if (!temp) {
                            fprintf(stderr, "malloc failed!\r\n");
                            return -1;
                        }
                        memcpy(temp, frame + frame_size - pcm_remain_len, temp_size);
                        pcm_remain_len -= temp_size;

                        Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                        pcm_list->push_back(std::make_pair(next_pts, new_pcm));
                        next_pts += get_audio_pts_from_pcm_size(temp_size);
                    }

                    // for (auto it = _pcm_list->begin(); it != _pcm_list->end(); ++it) {
                    //     auto &item = *it;
                    //     log::info("PTS:%d PCM:%p PCM_SIZE:%d", item.first, item.second->data, item.second->data_len);
                    // }

                    // audio process
                    while (pcm_list->size() > 0) {
                        auto item = pcm_list->front();
                        auto next_pts = item.first;
                        Bytes *pcm = item.second;
                        if (pcm) {
                            if (pcm->data_len == buffer_size) {
                                const uint8_t *in[] = {pcm->data};
                                uint8_t *out[] = {audio_frame->data[0]};
                                swr_convert(swr_ctx, out, audio_codec_ctx->frame_size, in, audio_codec_ctx->frame_size);
                                audio_frame->pts = next_pts;
                                if (avcodec_send_frame(audio_codec_ctx, audio_frame) < 0) {
                                    printf("Error sending audio_frame to encoder.\n");
                                    break;
                                }

                                while (avcodec_receive_packet(audio_codec_ctx, audio_packet) == 0) {
                                    audio_packet->stream_index = audio_stream->index;
                                    audio_packet->pts = audio_packet->dts = next_pts;
                                    audio_packet->duration = get_audio_pts_from_pcm_size(pcm->data_len);

                                    // log::info("[AUIDIO] frame:%p frame_size:%d pts:%ld(%f s)", pcm->data, pcm->data_len, pkt->pts, this->audio_pts_to_us(pkt->pts) / 1000);
                                    av_interleaved_write_frame(outputFormatContext, audio_packet);
                                    av_packet_unref(audio_packet);
                                }
                                pcm_list->pop_front();
                                delete pcm;
                            } else {
                                break;
                            }
                        } else {
                            fprintf(stderr, "pcm data is nullptr..\r\n");
                        }
                    }
                }

                av_packet_unref(pkt);
                av_packet_free(&pkt);
            }
        }

        return 0;
    }

    void add_adts_header(uint8_t *adts_header, int aac_frame_length, int profile, int sample_rate_index, int channel_config) {
        // ADTS 固定头部的前 5 字节
        adts_header[0] = 0xFF;                           // Syncword 0xFFF, 高8位
        adts_header[1] = 0xF1;                           // Syncword低4位+ID=0+Layer=0+protection_absent=1
        adts_header[2] = ((profile - 1) << 6)            // Profile（2位）：AAC-LC 为 1（profile - 1 后为 0）
                        | (sample_rate_index << 2)       // Sampling frequency index（4位）
                        | (channel_config >> 2);         // Channel configuration（高2位）
        adts_header[3] = ((channel_config & 0x3) << 6)   // Channel configuration（低2位）
                        | ((aac_frame_length + 7) >> 11); // Frame length（高3位）
        adts_header[4] = ((aac_frame_length + 7) >> 3) & 0xFF; // Frame length（中间 8 位）
        adts_header[5] = (((aac_frame_length + 7) & 0x7) << 5) // Frame length（低 3 位）
                        | 0x1F;                         // Buffer fullness（高5位，固定值0x7FF）
        adts_header[6] = 0xFC;                          // Buffer fullness（低2位，固定值0x7FF）+ number_of_raw_data_blocks（2位，固定0）
    }

    static int get_sample_rate_index(int sample_rate)
    {
        switch (sample_rate) {
        case 96000: return 0;
        case 88200: return 1;
        case 64000: return 2;
        case 48000: return 3;
        case 44100: return 4;
        case 32000: return 5;
        case 24000: return 6;
        case 22050: return 7;
        case 16000: return 8;
        case 12000: return 9;
        case 11025: return 10;
        case 8000: return 11;
        case 7350: return 12;
        default: return 3;
        }
    }

    int push2(uint8_t *frame, size_t frame_size, uint64_t pts, bool is_audio, void (*callback)(void*, size_t, size_t, void *args), void *args)
    {
        if (!_open) {
            return -1;
        }

        if (!is_audio) {
            // log::info("[VIDEO] frame:%p frame_size:%d pts:%ld(%f s)", frame, frame_size, pts, this->video_pts_to_us(pts) / 1000);
            if (callback) {
                callback(frame, frame_size, pts, args);
            }
        } else {
            if (_has_audio) {
                AVPacket *pkt = av_packet_alloc();
                if (!pkt) {
                    fprintf(stderr, "Can't malloc avpacket\r\n");
                    return -1;
                }

                if (frame && frame_size > 0) {
                    auto pcm_list = _pcm_list;
                    AVFrame *audio_frame = _audio_frame;
                    AVStream *audio_stream = _audio_stream;
                    AVCodecContext *audio_codec_ctx = _audio_codec_ctx;
                    SwrContext *swr_ctx = _audio_swr_ctx;
                    AVPacket *audio_packet = pkt;
                    size_t buffer_size = av_samples_get_buffer_size(NULL, _audio_channels, audio_frame->nb_samples, _audio_format, 1);
                    size_t pcm_remain_len = frame_size;

                    // fill last pcm to buffer_size
                    size_t next_pts = pts;
                    if (!pcm_list->empty()) {
                        auto last_item = pcm_list->back();
                        Bytes *last_pcm = last_item.second;
                        if (last_pcm && last_pcm->data_len < buffer_size) {
                            int temp_size = pcm_remain_len + last_pcm->data_len >= buffer_size ? buffer_size : pcm_remain_len + last_pcm->data_len;
                            uint8_t *temp = (uint8_t *)malloc(temp_size);
                            if (!temp) {
                                fprintf(stderr, "malloc failed!\r\n");
                                return -1;
                            }
                            memcpy(temp, last_pcm->data, last_pcm->data_len);
                            if (pcm_remain_len + last_pcm->data_len < buffer_size) {
                                memcpy(temp + last_pcm->data_len, frame, pcm_remain_len);
                                pcm_remain_len = 0;
                            } else {
                                memcpy(temp + last_pcm->data_len, frame, buffer_size - last_pcm->data_len);
                                pcm_remain_len -= (buffer_size - last_pcm->data_len);
                            }

                            Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                            pcm_list->pop_back();
                            delete last_pcm;

                            size_t new_pts = last_item.first;
                            next_pts = new_pts + get_audio_pts_from_pcm_size(new_pcm->data_len);
                            pcm_list->push_back(std::make_pair(new_pts, new_pcm));
                        }
                    }

                    // fill other pcm
                    while (pcm_remain_len > 0) {
                        int temp_size = pcm_remain_len >= buffer_size ? buffer_size : pcm_remain_len;
                        uint8_t *temp = (uint8_t *)malloc(temp_size);
                        if (!temp) {
                            fprintf(stderr, "malloc failed!\r\n");
                            return -1;
                        }
                        memcpy(temp, frame + frame_size - pcm_remain_len, temp_size);
                        pcm_remain_len -= temp_size;

                        Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                        pcm_list->push_back(std::make_pair(next_pts, new_pcm));
                        next_pts += get_audio_pts_from_pcm_size(temp_size);
                    }

                    // for (auto it = _pcm_list->begin(); it != _pcm_list->end(); ++it) {
                    //     auto &item = *it;
                    //     log::info("PTS:%d PCM:%p PCM_SIZE:%d", item.first, item.second->data, item.second->data_len);
                    // }

                    // audio process
                    while (pcm_list->size() > 0) {
                        auto item = pcm_list->front();
                        auto next_pts = item.first;
                        Bytes *pcm = item.second;
                        if (pcm) {
                            if (pcm->data_len == buffer_size) {
                                const uint8_t *in[] = {pcm->data};
                                uint8_t *out[] = {audio_frame->data[0]};
                                swr_convert(swr_ctx, out, audio_codec_ctx->frame_size, in, audio_codec_ctx->frame_size);
                                audio_frame->pts = next_pts;
                                if (avcodec_send_frame(audio_codec_ctx, audio_frame) < 0) {
                                    printf("Error sending audio_frame to encoder.\n");
                                    break;
                                }

                                while (avcodec_receive_packet(audio_codec_ctx, audio_packet) == 0) {
                                    audio_packet->stream_index = audio_stream->index;
                                    audio_packet->pts = audio_packet->dts = next_pts;
                                    audio_packet->duration = get_audio_pts_from_pcm_size(pcm->data_len);

                                    // log::info("[AUIDIO] frame:%p frame_size:%d pts:%ld(%f s)", pcm->data, pcm->data_len, pkt->pts, this->audio_pts_to_us(pkt->pts) / 1000);
                                    if (callback) {
                                        int new_size = audio_packet->size + 7;
                                        uint8_t *new_data = (uint8_t *)malloc(new_size);
                                        if (new_data) {
                                            int profile = 2;
                                            int sample_rate_index = get_sample_rate_index(_audio_sample_rate);
                                            add_adts_header(new_data, audio_frame->nb_samples, profile, sample_rate_index, _audio_channels);
                                            memcpy(new_data + 7, audio_packet->data, audio_packet->size);
                                            callback(new_data, new_size, pts, args);
                                            free(new_data);
                                        }
                                    }
                                    av_packet_unref(audio_packet);
                                }
                                pcm_list->pop_front();
                                delete pcm;
                            } else {
                                break;
                            }
                        } else {
                            fprintf(stderr, "pcm data is nullptr..\r\n");
                        }
                    }
                }

                av_packet_unref(pkt);
                av_packet_free(&pkt);
            }
        }

        return 0;
    }
    uint64_t get_audio_pts_from_pcm_size(size_t pcm_length) {
        if (!_open || !_has_audio)
            return 0;
        uint64_t frame_size_per_second = _audio_frame->sample_rate * _audio_frame->channels * av_get_bytes_per_sample(_audio_format);
        return pcm_length * (_audio_stream->time_base.den / _audio_stream->time_base.num) / frame_size_per_second;
    }

    double video_pts_to_us(uint64_t pts) {
        if (!_open || !_has_video)
            return 0;
        return (double)pts / ( _video_stream->time_base.den /_video_stream->time_base.num) * 1000000;
    }

    double audio_pts_to_us(uint64_t pts) {
        if (!_open || !_has_video)
            return 0;
        return (double)pts / ( _audio_stream->time_base.den /_audio_stream->time_base.num) * 1000000;
    }

    uint64_t video_us_to_pts(uint64_t us) {
        if (!_open || !_has_video) {
            return 0;
        }

        return us * (_video_stream->time_base.den / _video_stream->time_base.num) / 1000000;
    }

    uint64_t audio_us_to_pts(uint64_t us) {
        if (!_open || !_has_audio) {
            return 0;
        }

        return us * (_audio_stream->time_base.den / _audio_stream->time_base.num) / 1000000;
    }

    int get_audio_frame_size_per_second() {
        if (!_open || !_has_audio) {
            return 0;
        }

        return _audio_frame->sample_rate * _audio_frame->channels * av_get_bytes_per_sample(_audio_format);
    }
};
}

#endif // __MAIX_FFMPEG_HPP
