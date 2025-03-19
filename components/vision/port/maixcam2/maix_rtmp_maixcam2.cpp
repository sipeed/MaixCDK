#include "maix_rtmp.hpp"

#include "maix_video.hpp"
#include "maix_basic.hpp"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <list>

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

using namespace maix;

class RTMPClient
{
    typedef struct {
        std::string url;
        AVFormatContext *format_context = nullptr;
        bool has_video = true;
        AVStream *video_stream = nullptr;
        enum AVCodecID video_codec_id;
        int video_width;
        int video_height;
        enum AVPixelFormat vidoe_pixel_format;
        uint32_t video_bitrate;
        AVRational video_timebase;
        uint8_t *video_sps_pps = nullptr;
        size_t video_sps_pps_size;
        size_t video_frame_count = 0;
        size_t video_last_pts = 0;

        bool has_audio = false;
        AVStream *audio_stream = nullptr;
        AVCodec *audio_codec = nullptr;
        AVFrame *audio_frame = nullptr;
        AVCodecContext *audio_codec_ctx = NULL;
        SwrContext *audio_swr_ctx = nullptr;
        std::list<std::pair<size_t, Bytes *>> *pcm_list;
        int audio_sample_rate;
        int audio_channels;
        int audio_bitrate;
        enum AVSampleFormat audio_format;
    } ffmpeg_param_t;

    ffmpeg_param_t _ffmpeg;
    bool _open;
public:
    static int ffmpeg_init(ffmpeg_param_t *ffmpeg)
    {
        avformat_network_init();
        AVFormatContext *format_context = NULL;
        avformat_alloc_output_context2(&format_context, NULL, "flv", ffmpeg->url.c_str());
        if (!format_context) {
			log::info("Can't create output context");
            return -1;
        }

        if (ffmpeg->has_video) {
            AVStream *video_stream = avformat_new_stream(format_context, NULL);
            if (!video_stream) {
				log::info("Can't create video stream");
                goto _free_format_context;
            }
            video_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
            video_stream->codecpar->codec_id = ffmpeg->video_codec_id;
            video_stream->codecpar->width = ffmpeg->video_width;
            video_stream->codecpar->height = ffmpeg->video_height;
            video_stream->codecpar->format = ffmpeg->vidoe_pixel_format;
            video_stream->codecpar->bit_rate = ffmpeg->video_bitrate;
            video_stream->codecpar->codec_tag = 0;
            video_stream->codecpar->extradata = ffmpeg->video_sps_pps;
            video_stream->codecpar->extradata_size = ffmpeg->video_sps_pps_size;
            video_stream->time_base = ffmpeg->video_timebase;
            if (video_stream->codecpar->extradata == NULL ||
                video_stream->codecpar->extradata_size == 0) {
				log::info("video sps pps is null, size is 0");
                goto _free_format_context;
            }

            ffmpeg->video_stream = video_stream;
        }

        if (ffmpeg->has_audio) {
            int sample_rate = 48000;
            int channels = 1;
            int bitrate = 128000;
            enum AVSampleFormat format = AV_SAMPLE_FMT_S16;

            AVStream *audio_stream = avformat_new_stream(format_context, NULL);
            if (!audio_stream) {
				log::info("Can't create audio stream");
                goto _free_format_context;
            }

            AVCodec *audio_codec = (AVCodec *)avcodec_find_encoder(AV_CODEC_ID_AAC);
            if (!audio_codec) {
				log::info("Can't find audio encoder");
                goto _free_format_context;
            }

            AVCodecContext *audio_codec_ctx = avcodec_alloc_context3(audio_codec);
            if (!audio_codec_ctx) {
				log::info("Can't alloc audio codec context");
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
            audio_stream->time_base = audio_codec_ctx->time_base;

            if (0 > avcodec_open2(audio_codec_ctx, audio_codec, NULL)) {
				log::info("Can't open audio codec");
                goto _free_audio_codec_ctx;
            }

            if (0 > avcodec_parameters_from_context(audio_stream->codecpar, audio_codec_ctx)) {
				log::info("Can't copy audio codec parameters");
                goto _free_audio_codec_ctx;
            }

            SwrContext *swr_ctx = swr_alloc();
            if (!swr_ctx) {
				log::info("Can't alloc swr context");
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
				log::info("Can't alloc audio frame");
                swr_free(&swr_ctx);
                swr_ctx = NULL;
                goto _free_swr_ctx;
            }
            audio_frame->nb_samples = audio_codec_ctx->frame_size;
            audio_frame->channel_layout = audio_codec_ctx->channel_layout;
            audio_frame->format = AV_SAMPLE_FMT_FLTP;
            audio_frame->sample_rate = audio_codec_ctx->sample_rate;
            av_frame_get_buffer(audio_frame, 0);

            ffmpeg->audio_stream = audio_stream;
            ffmpeg->audio_codec = audio_codec;
            ffmpeg->audio_codec_ctx = audio_codec_ctx;
            ffmpeg->audio_swr_ctx = swr_ctx;
            ffmpeg->audio_frame = audio_frame;
            ffmpeg->audio_sample_rate = sample_rate;
            ffmpeg->audio_channels = channels;
            ffmpeg->audio_bitrate = bitrate;
            ffmpeg->audio_format = format;
        }

        if (avio_open(&format_context->pb, ffmpeg->url.c_str(), AVIO_FLAG_WRITE) < 0) {
			log::info("rtmp connect failed!");
            goto _free_audio_frame;
        }

        if (avformat_write_header(format_context, NULL) < 0) {
			log::info("rtmp write header failed!");
            goto _close_io;
        }

        ffmpeg->format_context = format_context;
        return 0;
_close_io:
        if (ffmpeg->format_context && ffmpeg->format_context->pb) {
            avio_closep(&ffmpeg->format_context->pb);
			ffmpeg->format_context->pb = nullptr;
        }
_free_audio_frame:
        if (ffmpeg->audio_frame) {
            av_frame_free(&ffmpeg->audio_frame);
            ffmpeg->audio_frame = nullptr;
        }
_free_swr_ctx:
        if (ffmpeg->audio_swr_ctx) {
            swr_free(&ffmpeg->audio_swr_ctx);
            ffmpeg->audio_swr_ctx = nullptr;
        }
_free_audio_codec_ctx:
        if (ffmpeg->audio_codec_ctx) {
            avcodec_free_context(&ffmpeg->audio_codec_ctx);
            ffmpeg->audio_codec_ctx = nullptr;
        }
_free_format_context:
        if (ffmpeg->format_context) {
            avformat_free_context(ffmpeg->format_context);
			ffmpeg->format_context = nullptr;
        }

		avformat_network_deinit();
        return -1;
    }

    static int ffmpeg_deinit(ffmpeg_param_t *ffmpeg)
    {
        if (!ffmpeg) return 0;

        /* free sps pps in avformat_free_context */
        /*
            if (ffmpeg->video_sps_pps) {
                free(ffmpeg->video_sps_pps);
                ffmpeg->video_sps_pps = NULL;
                ffmpeg->video_sps_pps_size = 0;
            }
        */

        if (ffmpeg->audio_frame) {
            av_frame_free(&ffmpeg->audio_frame);
            ffmpeg->audio_frame = nullptr;
        }

        if (ffmpeg->audio_swr_ctx) {
            swr_free(&ffmpeg->audio_swr_ctx);
            ffmpeg->audio_swr_ctx = nullptr;
        }

        if (ffmpeg->audio_codec_ctx) {
            avcodec_free_context(&ffmpeg->audio_codec_ctx);
            ffmpeg->audio_codec_ctx = nullptr;
        }

        if (ffmpeg->format_context) {
            // av_write_trailer(ffmpeg->format_context);
            if (ffmpeg->format_context && ffmpeg->format_context->pb) {
                avio_closep(&ffmpeg->format_context->pb);
            }
            avformat_free_context(ffmpeg->format_context);
            ffmpeg->format_context = NULL;
        }

		avformat_network_deinit();
        return 0;
    }

    RTMPClient() {
        _ffmpeg.has_video = true;
        _ffmpeg.pcm_list = new std::list<std::pair<size_t, Bytes *>>;
        _ffmpeg.video_last_pts = 0;
        err::check_null_raise(_ffmpeg.pcm_list, "create pcm list failed!");
        _open = false;
    }

    ~RTMPClient() {
        if (_ffmpeg.pcm_list) {
            for (auto it = _ffmpeg.pcm_list->begin(); it != _ffmpeg.pcm_list->end(); ++it) {
                auto &item = *it;
                Bytes *pcm = item.second;
                delete pcm;
                it = _ffmpeg.pcm_list->erase(it);
            }
            delete _ffmpeg.pcm_list;
            _ffmpeg.pcm_list = nullptr;
        }

        if (_open) {
            close();
        }
    }

    err::Err open()
    {
        if (_open) return err::ERR_NONE;

        if (ffmpeg_init(&_ffmpeg) < 0) {
            return err::ERR_RUNTIME;
        }

        _open = true;
        return err::ERR_NONE;
    }

    err::Err close()
    {
        if (!_open) return err::ERR_NONE;

        if (ffmpeg_deinit(&_ffmpeg) < 0) {
            return err::ERR_RUNTIME;
        }

        _open = false;
        return err::ERR_NONE;
    }

    err::Err config(std::string cmd, int data)
    {
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;

        if (cmd == "has_video") {
            ffmpeg->has_video = data ? true : false;
            return err::ERR_NONE;
        } else if (cmd == "video_codec_id") {
            ffmpeg->video_codec_id = (AVCodecID)data;
            return err::ERR_NONE;
        } else if (cmd == "video_width") {
            ffmpeg->video_width = data;
            return err::ERR_NONE;
        } else if (cmd == "video_height") {
            ffmpeg->video_height = data;
            return err::ERR_NONE;
        } else if (cmd == "video_bitrate") {
            ffmpeg->video_bitrate = data;
            return err::ERR_NONE;
        } else if (cmd == "video_fps") {
            ffmpeg->video_timebase = (AVRational){1, data};
            return err::ERR_NONE;
        } else if (cmd == "video_pixel_format") {
            ffmpeg->vidoe_pixel_format = (AVPixelFormat)data;
            return err::ERR_NONE;
        } else if (cmd == "has_audio") {
            ffmpeg->has_audio = data ? true : false;
            return err::ERR_NONE;
        } else {
            return err::ERR_RUNTIME;
        }
        return err::ERR_RUNTIME;
    }

    err::Err config_url(std::string url) {
        _ffmpeg.url = url;
        return err::ERR_NONE;
    }

    err::Err config_sps_pps(uint8_t *sps_pps, int sps_pps_size)
    {
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;

        if (ffmpeg->video_sps_pps) {
            free(ffmpeg->video_sps_pps);
            ffmpeg->video_sps_pps = NULL;
        }
        ffmpeg->video_sps_pps = (uint8_t *)malloc(sps_pps_size);
        if (!ffmpeg->video_sps_pps) {
            return err::ERR_NO_MEM;
        }
        memcpy(ffmpeg->video_sps_pps, sps_pps, sps_pps_size);
        ffmpeg->video_sps_pps_size = sps_pps_size;
        return err::ERR_NONE;
    }

    err::Err push(uint8_t *frame, size_t frame_size, uint64_t pts, bool is_audio = false)
    {
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;
        if (!_open) {
            return err::ERR_NOT_OPEN;
        }

        if (!is_audio) {
            if (ffmpeg->has_video) {
                AVPacket *pkt = av_packet_alloc();
                if (!pkt) {
                    fprintf(stderr, "Can't malloc avpacket\r\n");
                    return err::ERR_RUNTIME;
                }

                pkt->data = frame;
                pkt->size = frame_size;
                pkt->stream_index = ffmpeg->video_stream->index;
                pkt->duration = pts - _ffmpeg.video_last_pts;
                _ffmpeg.video_last_pts = pts;
                pkt->pts = pkt->dts = pts;
                pkt->flags |= AV_PKT_FLAG_KEY;
                ffmpeg->video_frame_count ++;
                // log::info("[VIDEO] frame:%p frame_size:%d pts:%ld(%f s)", frame, frame_size, pkt->pts, this->timebase_to_ms(this->get_video_timebase(), pkt->pts) / 1000);
                if (av_interleaved_write_frame(ffmpeg->format_context, pkt) < 0) {
                    fprintf(stderr, "send frame failed!\r\n");
                    av_packet_free(&pkt);
                    return err::ERR_RUNTIME;
                }

                av_packet_unref(pkt);
                av_packet_free(&pkt);
            }
        } else {
            if (ffmpeg->has_audio) {
                AVPacket *pkt = av_packet_alloc();
                if (!pkt) {
                    fprintf(stderr, "Can't malloc avpacket\r\n");
                    return err::ERR_RUNTIME;
                }

                if (frame && frame_size > 0) {
                    auto pcm_list = ffmpeg->pcm_list;
                    AVFrame *audio_frame = ffmpeg->audio_frame;
                    AVStream *audio_stream = ffmpeg->audio_stream;
                    AVCodecContext *audio_codec_ctx = ffmpeg->audio_codec_ctx;
                    SwrContext *swr_ctx = ffmpeg->audio_swr_ctx;
                    AVFormatContext *outputFormatContext = ffmpeg->format_context;
                    AVPacket *audio_packet = pkt;
                    size_t buffer_size = av_samples_get_buffer_size(NULL, ffmpeg->audio_channels, audio_frame->nb_samples, ffmpeg->audio_format, 1);
                    size_t pcm_remain_len = frame_size;

                    // fill last pcm to buffer_size
                    size_t next_pts = pts;
                    if (!pcm_list->empty()) {
                        auto last_item = pcm_list->back();
                        Bytes *last_pcm = last_item.second;
                        if (last_pcm && last_pcm->data_len < buffer_size) {
                            int temp_size = pcm_remain_len + last_pcm->data_len >= buffer_size ? buffer_size : pcm_remain_len + last_pcm->data_len;
                            uint8_t *temp = (uint8_t *)malloc(temp_size);
                            err::check_null_raise(temp, "malloc failed!");
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
                        err::check_null_raise(temp, "malloc failed!");
                        memcpy(temp, frame + frame_size - pcm_remain_len, temp_size);
                        pcm_remain_len -= temp_size;

                        Bytes *new_pcm = new Bytes(temp, temp_size, true, false);
                        pcm_list->push_back(std::make_pair(next_pts, new_pcm));
                        next_pts += get_audio_pts_from_pcm_size(temp_size);
                    }

                    // for (auto it = _ffmpeg.pcm_list->begin(); it != _ffmpeg.pcm_list->end(); ++it) {
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

                av_packet_unref(pkt);
                av_packet_free(&pkt);
            }
        }

        return err::ERR_NONE;
    }

    void reset_pts()
    {
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;
        ffmpeg->video_frame_count = 0;
    }

    bool is_opened()
    {
        return _open;
    }

    int get_frame_size_per_second() {
        if (!_open)
            return 0;
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;
        return ffmpeg->audio_frame->sample_rate * ffmpeg->audio_frame->channels * av_get_bytes_per_sample(ffmpeg->audio_format);
    }

    std::vector<int> get_video_timebase() {
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;
        if (!_open || !ffmpeg->has_video)
            return std::vector<int>();
        return {ffmpeg->video_stream->time_base.num, ffmpeg->video_stream->time_base.den};
    }

    std::vector<int> get_audio_timebase() {
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;
        if (!_open || !ffmpeg->has_audio)
            return std::vector<int>();
        return {ffmpeg->audio_stream->time_base.num, ffmpeg->audio_stream->time_base.den};
    }

    double timebase_to_us(std::vector<int> timebase, uint64_t value) {
        if (timebase.size() < 2) return -1;
        return value * 1000000 / ((double)timebase[1] / timebase[0]);
    }

    double timebase_to_ms(std::vector<int> timebase, uint64_t value) {
        if (timebase.size() < 2) return -1;
        return value * 1000 / ((double)timebase[1] / timebase[0]);
    }

    uint64_t ms_to_pts(std::vector<int> timebase, uint64_t value) {
        return value * (timebase[1] / timebase[0]) / 1000;
    }

    uint64_t get_audio_pts_from_pcm_size(size_t pcm_length) {
        ffmpeg_param_t *ffmpeg = (ffmpeg_param_t *)&_ffmpeg;
        if (!_open || !ffmpeg->has_audio)
            return 0;
        return pcm_length * (ffmpeg->audio_stream->time_base.den / ffmpeg->audio_stream->time_base.num) / get_frame_size_per_second();
    }
};

namespace maix::rtmp {
	class PrivateParam {
	public:
		enum {
			RTMP_IDLE = 0,
			RTMP_STOP,
			RTMP_RUNNING,
		} status;
		RTMPClient *rtmp_client = nullptr;
		rtmp::Rtmp *rtmp;
		std::string url;
		int bitrate;
		camera::Camera **camera;
		audio::Recorder **recorder;
		video::Encoder **encoder;
		display::Display **display;
	};

	Rtmp::Rtmp(std::string host, int port, std::string app, std::string stream, int bitrate) {
		_host = host;
		_port = port;
		_app = app;
		_stream = stream;
		_bitrate = bitrate;

		RTMPClient *rtmp_client = new RTMPClient();
		PrivateParam *param = new PrivateParam();
		param->rtmp = this;
		param->rtmp_client = rtmp_client;
		param->status = PrivateParam::RTMP_IDLE;
		param->bitrate = bitrate;
		param->camera = &_camera;
		param->recorder = &_audio_recorder;
		param->encoder = &_video_encoder;
		param->display = &_display;
		/* rtmp://host:prot/app/stream */
		param->url = "rtmp://" + host + ":" + std::to_string(port) + "/" + app + "/" + stream;
		if (0 != pthread_mutex_init(&_lock, NULL)) {
			throw std::runtime_error("create lock failed!");
		}
		// log::info("url:%s", param->url.c_str());

		_param = param;
		_thread = nullptr;
		_camera = nullptr;
		_start = false;
		_capture_image = nullptr;
		_need_capture = false;
		_path = std::string();
	}

	Rtmp::~Rtmp() {
		if (_param) {
			PrivateParam *param = (PrivateParam *)_param;
			if (param->status != PrivateParam::RTMP_IDLE) {
				this->stop();
			}

			if (param->rtmp_client) {
				delete param->rtmp_client;
				param->rtmp_client = nullptr;
			}
			delete param;
			_param = nullptr;
		}

		pthread_mutex_destroy(&_lock);
	}

	// return 0 ok, other error
	int Rtmp::push_video(void *data, size_t data_size, uint32_t timestamp) {
		PrivateParam *param = (PrivateParam *)_param;
		if (param && param->rtmp_client) {
			auto pts = param->rtmp_client->ms_to_pts(param->rtmp_client->get_audio_timebase(), timestamp);
			param->rtmp_client->push((uint8_t *)data, data_size, pts);
			return 0;
		} else {
			return -1;
		}
	}

	// return 0 ok, other error
	int Rtmp::push_audio(void *data, size_t data_size, uint32_t timestamp) {
		PrivateParam *param = (PrivateParam *)_param;
		if (param && param->rtmp_client) {
			auto pts = param->rtmp_client->ms_to_pts(param->rtmp_client->get_audio_timebase(), timestamp);
			param->rtmp_client->push((uint8_t *)data, data_size, pts, true);
			return 0;
		} else {
			return -1;
		}
	}

	// return 0 ok, other error
	int Rtmp::push_script(void *data, size_t data_size, uint32_t timestamp) {
		err::check_raise(err::ERR_NOT_IMPL, "This interface is not supported");
		return -1;
	}

	err::Err Rtmp::lock(uint32_t time) {
		uint32_t count = 0;
		while (0 != pthread_mutex_trylock(&_lock)) {
			count ++;
			if (count >= time) {
				break;
			}
			time::sleep_ms(1);
		}

		if (count >= time)
			return err::ERR_BUSY;
		else
			return err::ERR_NONE;
	}

	err::Err Rtmp::unlock() {
		if (0 == pthread_mutex_unlock(&_lock)) {
			return err::ERR_NONE;
		}
		return err::ERR_RUNTIME;
	}

	static void _push_camera_thread(void *priv)
	{
		PrivateParam *param = (PrivateParam *)priv;
		camera::Camera *camera = *param->camera;
		audio::Recorder *audio_recorder = *param->recorder;

		param->status = PrivateParam::RTMP_IDLE;
	}

	err::Err Rtmp::start(std::string path) {
		lock(-1);
		err::Err ret = err::ERR_NONE;
		PrivateParam *param = (PrivateParam *)_param;
		auto video_encoder_width = 0;
		auto video_encoder_height = 0;
		auto video_encoder_format = 0;
		auto fps = 30;
        bool has_audio = false;
		RTMPClient *rtmp_client = nullptr;

		if (param == nullptr) {
			ret = err::ERR_RUNTIME;
			goto _error;
		}

		rtmp_client = (RTMPClient *)param->rtmp_client;
		if (param->status != PrivateParam::RTMP_IDLE) {
			ret = err::ERR_BUSY;
			goto _error;
		}

		if (_camera == nullptr) {
			log::info("You must use the bind_camera interface to bind a Camera object.");
			ret = err::ERR_NOT_READY;
			goto _error;
		}

		if (_audio_recorder == nullptr) {
			has_audio = false;
		} else {
            has_audio = true;
            _audio_recorder->reset(true);
        }

		video_encoder_width = _camera->width();
		video_encoder_height = _camera->height();
		video_encoder_format = _camera->format();
		_camera->set_fps(fps);
		// log::info("Create encoder, width:%d height:%d format:%s fps:%d bitrate:%d", video_encoder_width, video_encoder_height, image::fmt_names[video_encoder_format].c_str(), fps, param->bitrate);
		_video_encoder = new video::Encoder("", video_encoder_width, video_encoder_height, (image::Format)video_encoder_format, video::VIDEO_H264, fps, 50, param->bitrate);
		if (!_video_encoder) {
			log::info("Create video encoder failed!");
			ret = err::ERR_RUNTIME;
			goto _error;
		}

		err::check_raise(rtmp_client->config("has_audio", has_audio), "rtmp config failed!");
        err::check_raise(rtmp_client->config("video_codec_id", AV_CODEC_ID_H264), "rtmp config failed!");
        err::check_raise(rtmp_client->config("video_width", video_encoder_width), "rtmp config failed!");
        err::check_raise(rtmp_client->config("video_height", video_encoder_height), "rtmp config failed!");
        err::check_raise(rtmp_client->config("video_bitrate", _bitrate), "rtmp config failed!");
        err::check_raise(rtmp_client->config("video_fps", fps), "rtmp config failed!");
        err::check_raise(rtmp_client->config("video_pixel_format", AV_PIX_FMT_NV21), "rtmp config failed!");
		err::check_raise(rtmp_client->config_url(param->url), "rtmp config failed!");

		_thread = new thread::Thread(_push_camera_thread, param);
		if (this->_thread == NULL) {
			log::error("create camera thread failed!\r\n");
			ret = err::ERR_RUNTIME;
			goto _error;
		}

		unlock();
		return err::ERR_NONE;
_error:
		unlock();
		return ret;
	}

	err::Err Rtmp::stop() {
		PrivateParam *param = (PrivateParam *)_param;
		lock(-1);
		if (param->status != PrivateParam::RTMP_IDLE) {
			param->status = PrivateParam::RTMP_STOP;
		}

		unlock();

		if (_thread) {
			_thread->join();
			_thread = nullptr;
		}

		while (param->status != PrivateParam::RTMP_IDLE) {
			time::sleep_ms(100);
			log::info("wait rtmp thread exit..");
		}

        if (_video_encoder) {
            delete _video_encoder;
            _video_encoder = nullptr;
        }

        if (_audio_recorder) {
            _audio_recorder->reset(false);
        }

		return err::ERR_NONE;
	}
}