
#include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_display.hpp"
#include "maix_video.hpp"
#include "maix_camera.hpp"
#include "list"
using namespace maix;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
}

#include "maix_qmi8658.hpp"
#include "pthread.h"
#include "gcsv.h"

static double timebase_to_ms(std::vector<int> timebase, uint64_t value) {
    return value * 1000 / ((double)timebase[1] / timebase[0]);
}

static int h264_to_mp4(int argc, char *argv[]) {
    const char *input_filename = "output.h264";
    const char *output_filename = "output2.mp4";

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

    AVFormatContext *input_format_ctx = NULL;
    AVFormatContext *output_format_ctx = NULL;
    AVPacket packet;
    int ret;

    // 初始化 FFmpeg 库
    // av_register_all();

    // 打开输入文件
    if ((ret = avformat_open_input(&input_format_ctx, input_filename, NULL, NULL)) < 0) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        return -1;
    }

    // 查找输入文件流信息
    if ((ret = avformat_find_stream_info(input_format_ctx, NULL)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information\n");
        return -1;
    }

    // 打开输出文件
    avformat_alloc_output_context2(&output_format_ctx, NULL, "mp4", output_filename);
    if (!output_format_ctx) {
        fprintf(stderr, "Could not create output context\n");
        return -1;
    }

    // 查找视频流并复制流到输出
    for (int i = 0; i < (int)input_format_ctx->nb_streams; i++) {
        AVStream *in_stream = input_format_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(output_format_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            return -1;
        }

        // 复制编码器参数
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy audio_codec parameters\n");
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    enum AVSampleFormat format = AV_SAMPLE_FMT_S16;
    int sample_rate = 48000;
    int channels = 1;
    int bitrate = 128000;
    {
        err::check_null_raise(audio_codec = avcodec_find_encoder(AV_CODEC_ID_AAC), "Could not find aac encoder");
        err::check_null_raise(audio_stream = avformat_new_stream(output_format_ctx, NULL), "Could not allocate stream");
        err::check_null_raise(audio_codec_ctx = avcodec_alloc_context3(audio_codec), "Could not allocate audio codec context");
        audio_codec_ctx->codec_id = AV_CODEC_ID_AAC;
        audio_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_codec_ctx->sample_rate = sample_rate;
        audio_codec_ctx->channels = channels;
        audio_codec_ctx->channel_layout = av_get_default_channel_layout(audio_codec_ctx->channels);
        audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;  // AAC编码需要浮点格式
        audio_codec_ctx->bit_rate = bitrate;
        audio_stream->time_base = (AVRational){1, sample_rate};
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
    }

    int frame_size = audio_codec_ctx->frame_size;
    size_t buffer_size = av_samples_get_buffer_size(NULL, 1, frame_size, format, 1);
    audio_frame = av_frame_alloc();
    audio_frame->nb_samples = frame_size;
    audio_frame->channel_layout = audio_codec_ctx->channel_layout;
    audio_frame->format = AV_SAMPLE_FMT_FLTP;
    audio_frame->sample_rate = audio_codec_ctx->sample_rate;
    av_frame_get_buffer(audio_frame, 0);

    int last_pts = 0;

    // 打开输出文件
    if (!(output_format_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&output_format_ctx->pb, output_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'\n", output_filename);
            return -1;
        }
    }

    // 写入文件头
    ret = avformat_write_header(output_format_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        return -1;
    }

    int count = 0;
    // 读取输入数据包并写入输出文件

    audio::Recorder *r = new audio::Recorder();
    Bytes *pcm = NULL;
    std::list<Bytes *> *pcm_list = new std::list<Bytes *>;

    while (av_read_frame(input_format_ctx, &packet) >= 0 && !app::need_exit()) {
{
        uint64_t t = time::ticks_ms();
        pcm = r->record();
        if (pcm) {
            if (pcm->data_len > 0) {
                log::info("record use %lld ms", time::ticks_ms() - t);
                t = time::ticks_ms();
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
                            swr_convert(swr_ctx, out, frame_size, in, frame_size);
                            audio_frame->pts = last_pts;
                            if (avcodec_send_frame(audio_codec_ctx, audio_frame) < 0) {
                                printf("Error sending audio_frame to encoder.\n");
                                break;
                            }

                            while (avcodec_receive_packet(audio_codec_ctx, audio_packet) == 0) {
                                audio_packet->stream_index = audio_stream->index;
                                audio_packet->pts = last_pts;
                                audio_packet->dts = audio_packet->pts;
                                audio_packet->duration = audio_codec_ctx->frame_size;
                                last_pts = audio_packet->pts + audio_packet->duration;
                                // log::info("audio_frame->pts:%d packet: pts:%d dts:%d duration:%d", audio_frame->pts, audio_packet->pts, audio_packet->dts, audio_packet->duration);

                                std::vector<int> timebase = {audio_stream->time_base.num, audio_stream->time_base.den};
                                log::info("[AUDIO] id:%d pts:%d dts:%d duration:%d timebase:%d/%d time:%.2f", audio_packet->stream_index, audio_packet->pts, audio_packet->dts, audio_packet->duration, audio_stream->time_base.num, audio_stream->time_base.den, timebase_to_ms(timebase, audio_packet->pts) / 1000);
                                av_interleaved_write_frame(output_format_ctx, audio_packet);
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
                // log::info("save to mp4 use %lld ms", time::ticks_ms() - t);
            }
            delete pcm;
        }
        time::sleep_ms(33);
}

        AVStream *in_stream = input_format_ctx->streams[packet.stream_index];
        AVStream *out_stream = output_format_ctx->streams[packet.stream_index];
        count ++;
        uint8_t nal_unit_type = packet.data[4] & 0x1F;
        switch (nal_unit_type) {
            case 1:
                printf("NAL Unit Type: %u (P/B-Frame) count:%d size:%d\n", nal_unit_type, count, packet.size);
                break;
            case 5:
                printf("NAL Unit Type: %u (I-Frame) count:%d size:%d\n", nal_unit_type, count, packet.size);
                break;
            case 7:
                printf("NAL Unit Type: %u (SPS) count:%d size:%d\n", nal_unit_type, count, packet.size);
                // found_i_sps_frame = true;
                break;
            case 8:
                printf("NAL Unit Type: %u (PPS) count:%d size:%d\n", nal_unit_type, count, packet.size);
                break;
            default:
                printf("NAL Unit Type: %u (Other) count:%d size:%d\n", nal_unit_type, count, packet.size);
                break;
        }

        // 将时间戳从输入流转换为输出流
        packet.stream_index = out_stream->index;
        packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        packet.pts = count * packet.duration;
        packet.dts = count * packet.duration;
        packet.pos = -1;

        std::vector<int> timebase = {out_stream->time_base.num, out_stream->time_base.den};
        log::info("[VIDEO] id:%d pts:%d dts:%d duration:%d time:%.2f", packet.stream_index, packet.pts, packet.dts, packet.duration, timebase_to_ms(timebase, packet.pts) / 1000);
        // 写入包到输出文件
        ret = av_interleaved_write_frame(output_format_ctx, &packet);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }

        av_packet_unref(&packet);
    }

    // 写入文件尾
    av_write_trailer(output_format_ctx);

    // 释放资源
    avformat_close_input(&input_format_ctx);

    if (output_format_ctx && !(output_format_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&output_format_ctx->pb);

    avformat_free_context(output_format_ctx);

    return 0;
}

static void *_imu_thread_process(void *arg) {
    #define _M_PI     (3.14159265358979323846f)
    int *imu_is_ready = (int *)arg;
    // gcsv init
    gcsv_handle_t gcsv_handle;
    gcsv_header_t gcsv_header = {
        .tscale = 0.001,    //unit: ms
        .gscale = ((double)1024 * _M_PI / 180) / 32768,
        .ascale = ((double)16 / 32768),
        .mscale = 0.0,
    };
    strncpy(gcsv_header.version, "1.3", sizeof(gcsv_header.version));
    strncpy(gcsv_header.id, "test", sizeof(gcsv_header.id));
    strncpy(gcsv_header.orientation, "YxZ", sizeof(gcsv_header.orientation));
    gcsv_init(&gcsv_handle, (char *)"/root/output.gcsv", &gcsv_header);

    // qmi init
    ext_dev::qmi8658::QMI8658 qmi8658(4, 0x6B, 400000,
                    maix::ext_dev::imu::Mode::DUAL,
                    maix::ext_dev::imu::AccScale::ACC_SCALE_16G,
                    maix::ext_dev::imu::AccOdr::ACC_ODR_8000,
                    maix::ext_dev::imu::GyroScale::GYRO_SCALE_1024DPS,
                    maix::ext_dev::imu::GyroOdr::GYRO_ODR_8000);
    time::sleep_ms(1000);
    // bias init
    float gyro_x_bias = 0, gyro_y_bias = 0, gyro_z_bias = 0;
    float acc_x_bias = 0, acc_y_bias = 0, acc_z_bias = 0;
#if 1
    {
        float acc_total[3] = {0}, gyro_total[3] = {0};
        int max_count = 100;
        for (int i = 0; i < max_count; i ++) {
            auto data = qmi8658.read();
            err::check_bool_raise(data.size(), "qmi8658 read failed!");
            acc_total[0] += data[0];
            acc_total[1] += data[1];
            acc_total[2] += data[2];
            gyro_total[0] += data[3];
            gyro_total[1] += data[4];
            gyro_total[2] += data[5];
        }
        acc_x_bias = acc_total[0] / max_count;
        acc_y_bias = acc_total[1] / max_count - 1;
        acc_z_bias = acc_total[2] / max_count;
        gyro_x_bias = gyro_total[0] / max_count;
        gyro_y_bias = gyro_total[1] / max_count;
        gyro_z_bias = gyro_total[2] / max_count;
        log::info("acc bias: %f %f %f", acc_x_bias, acc_y_bias, acc_z_bias);
        log::info("gyro bias: %f %f %f", gyro_x_bias, gyro_y_bias, gyro_z_bias);
    }
#else
    acc_x_bias = -0.010146;
    acc_y_bias = 0.028223;
    acc_z_bias = -0.012349;
    gyro_x_bias = 0.055321;
    gyro_y_bias = -0.031989;
    gyro_z_bias = -0.016962;
#endif

    *imu_is_ready = true;
    uint64_t first_read_ms = time::ticks_ms();
    log::info("imu first read ms:%d", first_read_ms);
    while (!app::need_exit()) {
        // read gyro
        {
            // uint64_t t = time::time_ms();
            float acc[3], gyro[3];
            auto data = qmi8658.read();
            err::check_bool_raise(data.size(), "qmi8658 read failed!");
            acc[0] = data[0] - acc_x_bias;
            acc[1] = data[1] - acc_y_bias;
            acc[2] = data[2] - acc_z_bias;
            gyro[0] = (data[3] - gyro_x_bias) * M_PI / 180;
            gyro[1] = (data[4] - gyro_y_bias) * M_PI / 180;
            gyro[2] = (data[5] - gyro_z_bias) * M_PI / 180;

            // printf("acc: %f %f %f\n", acc[0], acc[1], acc[2]);
            // printf("gyro: %f %f %f\n", gyro[0], gyro[1], gyro[2]);

            gcsv_info_t gcsv_info;
            memset(&gcsv_info, 0, sizeof(gcsv_info));
            gcsv_info.t = time::ticks_ms() - first_read_ms;
            gcsv_info.gyro.x = gyro[0] / gcsv_handle.header.gscale;
            gcsv_info.gyro.y = gyro[1] / gcsv_handle.header.gscale;
            gcsv_info.gyro.z = gyro[2] / gcsv_handle.header.gscale;
            gcsv_info.acc.x = acc[0] / gcsv_handle.header.ascale;
            gcsv_info.acc.y = acc[1] / gcsv_handle.header.ascale;
            gcsv_info.acc.z = acc[2] / gcsv_handle.header.ascale;
            gcsv_write(&gcsv_handle, &gcsv_info);

            // log::info("imu use %lld ms", time::time_ms() - t);
            time::sleep_us(100);
        }
    }
    return NULL;
}

static void helper(void)
{
    log::info(
    "==================================\r\n"
    "Please input command:\r\n"
    "0 <path> <width> <height> <format> <video_type> <fps> <gop> <bitrate> <time_base> <capture>: encode without bind\r\n"
    "1 <path> <width> <height> <format> <video_type> <fps> <gop> <bitrate> <time_base> <capture>: encode with bind\r\n"
    "2 <path> <delay_s> <width> <height> <fps>: time-lapse record\r\n"
    "note:\r\n"
    "format=%d, NV21\r\n"
    "type=%d, VIDEO_H264; type=%d, VIDEO_H265"
    "\r\n"
    "Example: ./encode_demo 0 output.mp4\r\n"
    "Example: ./encode_demo 0 output.mp4 640 480 8 30 50 3000000 1000 1\r\n"
    "==================================\r\n", image::FMT_YVU420SP, video::VIDEO_H264, video::VIDEO_H265);
}

int _main(int argc, char* argv[])
{
    image::Image img = image::Image();  // do nothing
    // return main_test();
    int cmd = 0;
    if (argc > 1) {
        if (!strcmp(argv[1], "-h")) {
            helper();
            return 0;
        } else {
            cmd = atoi(argv[1]);
        }
    } else {
        helper();
        return 0;
    }

    switch (cmd) {
    case 0:
    {
        std::string path = "/root/output.mp4";
        int width = 640;
        int height = 480;
        image::Format format = image::Format::FMT_YVU420SP;
        video::VideoType type = video::VIDEO_H264;
        audio::Recorder r = audio::Recorder();
        int framerate = 30;
        int gop = 50;
        int bitrate = 3000 * 1000;
        int time_base = 1000;
        bool capture = true;
        bool block = true;
        if (argc > 2) path = argv[2];
        if (argc > 3) width = atoi(argv[3]);
        if (argc > 4) height = atoi(argv[4]);
        if (argc > 5) format = (image::Format)atoi(argv[5]);
        if (argc > 6) type = (video::VideoType)atoi(argv[6]);
        if (argc > 7) framerate = atoi(argv[7]);
        if (argc > 8) gop = atoi(argv[8]);
        if (argc > 9) bitrate = atoi(argv[9]);
        if (argc > 10) time_base = atoi(argv[10]);
        if (argc > 11) capture = atoi(argv[11]) == 0 ? false : true;
        if (argc > 12) block = atoi(argv[12]) == 0 ? false : true;
        log::info("path:%s width:%d height:%d format:%d type:%d fps:%d gop:%d bitrate:%d time_base:%d capture:%d\r\n",
            path.c_str(), width, height, format, type, framerate, gop, bitrate, time_base, capture);
        video::Encoder e = video::Encoder(path, width, height, format, type, framerate, gop, bitrate, time_base, capture, block);
        camera::Camera cam = camera::Camera(width, height, format);
        display::Display disp = display::Display();

        while(!app::need_exit()) {
            image::Image *img = cam.read();
            Bytes *pcm = r.record();
            video::Frame *frame = e.encode(img, pcm);
            delete pcm;
            delete frame;

            disp.show(*img);
            delete img;
        }
        break;
    }
    case 1:
    {
        std::string path = "/root/output.mp4";
        int width = 640;
        int height = 480;
        image::Format format = image::Format::FMT_YVU420SP;
        video::VideoType type = video::VIDEO_H264;
        int framerate = 30;
        int gop = 50;
        int bitrate = 3000 * 1000;
        int time_base = 1000;
        bool capture = true;
        if (argc > 2) path = argv[2];
        if (argc > 3) width = atoi(argv[3]);
        if (argc > 4) height = atoi(argv[4]);
        if (argc > 5) framerate = atoi(argv[5]);
        video::Encoder e = video::Encoder(path, width, height, format, type, framerate, gop, bitrate, time_base, capture);
        camera::Camera cam = camera::Camera(width, height, format);
        display::Display disp = display::Display();
        audio::Recorder r = audio::Recorder();
        e.bind_camera(&cam);

        while(!app::need_exit()) {
            Bytes *pcm = r.record();
            video::Frame *frame = e.encode(NULL, pcm);
            image::Image *img = e.capture();
            disp.show(*img);
            delete frame;
            delete img;
            delete pcm;
        }
        break;
    }
    case 2:
    {
        std::string path = "/root/output.mp4";
        int width = 640;
        int height = 480;
        image::Format format = image::Format::FMT_YVU420SP;
        video::VideoType type = video::VIDEO_H264;
        int framerate = 30;
        int gop = 50;
        int bitrate = 3000 * 1000;
        int time_base = 1000;
        bool capture = true;
        bool block = true;
        int delay_s = 1 * 1000;
        if (argc > 2) path = argv[2];
        if (argc > 3) delay_s = atoi(argv[3]);
        if (argc > 4) width = atoi(argv[4]);
        if (argc > 5) height = atoi(argv[5]);
        if (argc > 6) framerate = atoi(argv[6]);

        log::info("path:%s width:%d height:%d format:%d type:%d fps:%d gop:%d bitrate:%d time_base:%d capture:%d\r\n",
            path.c_str(), width, height, format, type, framerate, gop, bitrate, time_base, capture);
        video::Encoder e = video::Encoder(path, width, height, format, type, framerate, gop, bitrate, time_base, capture, block);
        camera::Camera cam = camera::Camera(width, height, format);
        display::Display disp = display::Display();

        uint64_t delay_record = delay_s * 1000;
        uint64_t last_ms = time::ticks_ms();
        while(!app::need_exit()) {
            image::Image *img = cam.read();

            if (time::ticks_ms() - last_ms >= delay_record) {
                last_ms = time::ticks_ms();
                video::Frame *audio_frame = e.encode(img);
                printf("audio_frame data:%p size:%ld pts:%ld dts:%ld\r\n",
                    audio_frame->data(), audio_frame->size(), audio_frame->get_pts(), audio_frame->get_dts());
                delete audio_frame;
                continue;
            }

            disp.show(*img);
            delete img;
        }
        break;
    }
    case 3:
        h264_to_mp4(argc, argv);
        break;
    case 4:
    {
        std::string path = "/root/output.mp4";
        int width = 640;
        int height = 480;
        image::Format format = image::Format::FMT_YVU420SP;
        video::VideoType type = video::VIDEO_H264;
        audio::Recorder r = audio::Recorder();
        int framerate = 60;
        int gop = 50;
        int bitrate = 3000 * 1000;
        int time_base = 1000;
        bool capture = true;
        bool block = true;
        if (argc > 2) path = argv[2];
        if (argc > 3) width = atoi(argv[3]);
        if (argc > 4) height = atoi(argv[4]);
        if (argc > 5) format = (image::Format)atoi(argv[5]);
        if (argc > 6) type = (video::VideoType)atoi(argv[6]);
        if (argc > 7) framerate = atoi(argv[7]);
        if (argc > 8) gop = atoi(argv[8]);
        if (argc > 9) bitrate = atoi(argv[9]);
        if (argc > 10) time_base = atoi(argv[10]);
        if (argc > 11) capture = atoi(argv[11]) == 0 ? false : true;
        if (argc > 12) block = atoi(argv[12]) == 0 ? false : true;
        log::info("path:%s width:%d height:%d format:%d type:%d fps:%d gop:%d bitrate:%d time_base:%d capture:%d\r\n",
            path.c_str(), width, height, format, type, framerate, gop, bitrate, time_base, capture);
        video::Encoder e = video::Encoder(path, width, height, format, type, framerate, gop, bitrate, time_base, capture, block);
        camera::Camera cam = camera::Camera(width, height, format, NULL, framerate);
        display::Display disp = display::Display();

        pthread_t imu_thread;
        int imu_is_ready = false;
        err::check_bool_raise(pthread_create(&imu_thread, NULL, _imu_thread_process, &imu_is_ready) == 0, "create thread failed!");
        while (!imu_is_ready) {time::sleep_ms(10);}

        uint64_t start_time = time::ticks_ms();
        uint64_t last_loop_time = 0;
        log::info("camera first read ms:%d", start_time);
        while(!app::need_exit()) {
            // uint64_t t = time::ticks_ms();
            image::Image *img = cam.read();
            // log::info("camera read use %lld ms", time::ticks_ms() - t);

            // t = time::ticks_ms();
            video::Frame *frame = e.encode(img);
            // log::info("encode use %lld ms", time::ticks_ms() - t);

            // t = time::ticks_ms();
            disp.show(*img);
            // log::info("show use %lld ms", time::ticks_ms() - t);

            // t = time::ticks_ms();
            // delete pcm;
            delete frame;
            delete img;
            // log::info("free use %lld ms", time::ticks_ms() - t);

            while ((time::ticks_ms() - last_loop_time) * framerate < 1000) {
                time::sleep_us(500);
            }
            last_loop_time = time::ticks_ms();

            log::info("loop use %lld ms", time::ticks_ms() - start_time, time::ticks_ms());
            start_time = time::ticks_ms();
        }
        break;
    }
    default:
        helper();
        return 0;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
