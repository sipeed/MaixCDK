
#include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_display.hpp"
#include "maix_video.hpp"
#include "maix_camera.hpp"

using namespace maix;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

static int h264_to_mp4(int argc, char *argv[]) {
    const char *input_filename = "output.h264";
    const char *output_filename = "output2.mp4";

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
            fprintf(stderr, "Failed to copy codec parameters\n");
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
    }

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
    while (av_read_frame(input_format_ctx, &packet) >= 0) {
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
        packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        packet.pts = count * packet.duration;
        packet.dts = count * packet.duration;
        packet.pos = -1;

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
            video::Frame *frame = e.encode(img);
            // printf("frame data:%p size:%ld pts:%ld dts:%ld\r\n",
            //     frame->data(), frame->size(), frame->get_pts(), frame->get_dts());
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
        e.bind_camera(&cam);

        while(!app::need_exit()) {
            video::Frame *frame = e.encode();
            image::Image *img = e.capture();
            printf("frame data:%p size:%ld pts:%ld dts:%ld\r\n",
                frame->data(), frame->size(), frame->get_pts(), frame->get_dts());

            disp.show(*img);
            delete frame;
            delete img;
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
                video::Frame *frame = e.encode(img);
                printf("frame data:%p size:%ld pts:%ld dts:%ld\r\n",
                    frame->data(), frame->size(), frame->get_pts(), frame->get_dts());
                delete frame;
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
