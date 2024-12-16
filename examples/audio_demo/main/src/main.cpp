
// #include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_audio.hpp"
#include "alsa/asoundlib.h"

using namespace maix;
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

int decode_video(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return -1;
    }

    const char *input_filename = argv[2];
    AVFormatContext *format_ctx = NULL;
    AVCodecContext *codec_ctx = NULL;
    AVCodec *codec = NULL;
    AVPacket *packet = NULL;
    AVFrame *frame = NULL;
    SwrContext *swr_ctx = NULL;
    int audio_stream_index = -1;

    // av_register_all();

    // Open the input file
    if (avformat_open_input(&format_ctx, input_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        return -1;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }

    // Find the first audio stream
    // for (int i = 0; i < format_ctx->nb_streams; i++) {
    //     if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
    //         audio_stream_index = i;
    //         break;
    //     }
    // }
    audio_stream_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);

    if (audio_stream_index == -1) {
        fprintf(stderr, "Could not find audio stream in the input file\n");
        return -1;
    }

    // Get codec parameters and find the decoder
    AVCodecParameters *codecpar = format_ctx->streams[audio_stream_index]->codecpar;
    codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    // Allocate codec context and set parameters
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        return -1;
    }

    if (avcodec_parameters_to_context(codec_ctx, codecpar) < 0) {
        fprintf(stderr, "Could not copy codec parameters to codec context\n");
        return -1;
    }

    // Open codec
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return -1;
    }

    // Get PCM details from codec context
    int sample_rate = codec_ctx->sample_rate;
    enum AVSampleFormat sample_fmt = codec_ctx->sample_fmt;
    int channels = codec_ctx->channels;

    // Print PCM information
    printf("PCM Sample Rate: %d\n", sample_rate);
    printf("PCM Sample Format: %s\n", av_get_sample_fmt_name(sample_fmt));
    printf("PCM Channels: %d\n", channels);

    // Prepare to read packets and decode
    packet = av_packet_alloc();
    frame = av_frame_alloc();
    if (!packet || !frame) {
        fprintf(stderr, "Could not allocate packet or frame\n");
        return -1;
    }

    // Set up software resampler to convert to PCM
    swr_ctx = swr_alloc();
    av_opt_set_int(swr_ctx, "in_channel_layout", codec_ctx->channel_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", codec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", codec_ctx->sample_fmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout", codec_ctx->channel_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    swr_init(swr_ctx);

    audio::Player p = audio::Player("", 48000, audio::FMT_S16_LE, 2);

    // Read and decode audio frames
    while (av_read_frame(format_ctx, packet) >= 0 && !app::need_exit()) {
        if (packet->stream_index == audio_stream_index) {
            if (avcodec_send_packet(codec_ctx, packet) >= 0) {
                while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
                    uint8_t *output;
                    int out_samples = av_rescale_rnd(
                        swr_get_delay(swr_ctx, codec_ctx->sample_rate) + frame->nb_samples,
                        codec_ctx->sample_rate,
                        codec_ctx->sample_rate,
                        AV_ROUND_UP
                    );

                    av_samples_alloc(&output, NULL, codec_ctx->channels, out_samples, AV_SAMPLE_FMT_S16, 0);

                    int converted_samples = swr_convert(
                        swr_ctx,
                        &output, out_samples,
                        (const uint8_t **)frame->data, frame->nb_samples
                    );

                    if (converted_samples > 0) {
                        // Process the converted PCM data in `output` (e.g., write to file or buffer)
                        // fwrite(output, 1, converted_samples * codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16), stdout);
                        log::info("data:%p size:%d sample_rate:%d channel:%d", output,
                        converted_samples * codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16),
                        codec_ctx->sample_rate, codec_ctx->channels);
uint64_t t = time::ticks_ms();
                        Bytes b(output, converted_samples * codec_ctx->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
                        p.play(&b);
                        log::info("use %lld ms", time::ticks_ms() - t);
                    }

                    av_freep(&output);
                }
            }
        }
        av_packet_unref(packet);
    }

    // Clean up
    swr_free(&swr_ctx);
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    printf("Audio data extracted and decoded to PCM successfully.\n");
    return 0;
}

static void helper(void)
{
    log::info(
    "==================================\r\n"
    "Please input command:\r\n"
    "0 [path] [record_ms] [sample_rate] [channel] [format]: record block, ./audio_demo 0 output.pcm 3\r\n"
    "1 [path] [sample_rate] [channel] [format]: record nonblock, ./audio_demo 1 output.pcm 48000 1 2\r\n"
    "2 [volumn]: set recorder volume, ./audio_demo 2 12\r\n"
    "3 [path] [sample_rate] [channel] [format]: playback block, ./audio_demo 3 output.pcm\r\n"
    "4 [path] [sample_rate] [channel] [format]: playback nonblock, ./audio_demo 4 output.pcm 48000 1 2\r\n"
    "5 [path] [sample_rate] [channel] [format]: record nonblock  and save with fopen, ./audio_demo 5 output.pcm 48000 1 2\r\n"
    "6 [path] [sample_rate] [channel] [format]: playback nonblock and load with fopen./audio_demo 6 output.pcm 48000 1 2\r\n"
    "\r\n"
    "Note: format = 2, means FMT_S16_LE\r\n"
    "==================================\r\n");
}

int _main(int argc, char* argv[])
{
    int cmd = 0;
    if (argc > 1) {
        if (!strcmp("-h", argv[1])) {
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
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1, record_ms = 3000;
        audio::Format format = audio::Format::FMT_S16_LE;
        if (argc > 2) path = argv[2];
        if (argc > 3) record_ms = atoi(argv[3]) * 1000;
        if (argc > 4) sample_rate = atoi(argv[4]);
        if (argc > 5) channel = atoi(argv[5]);
        if (argc > 6) {
            switch (atoi(argv[6])) {
            case 2:
                format = audio::Format::FMT_S16_LE;
                break;
            default:
                log::error("Only support format = 2(FMT_S16_LE)\r\n");
                break;
            }
        }

        log::info("Ready to record %ld ms, and save to %s\r\n", record_ms, path.c_str());
        audio::Recorder r = audio::Recorder(path, sample_rate, format, channel);
        r.record(record_ms);

        log::info("Record over!\r\n");
        while (!app::need_exit()) {
            time::sleep_ms(1000);
        }
        break;
    }
    case 1:
    {
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1;
        audio::Format format = audio::Format::FMT_S16_LE;
        if (argc > 2) path = argv[2];
        if (argc > 3) sample_rate = atoi(argv[3]);
        if (argc > 4) channel = atoi(argv[4]);
        if (argc > 5) {
            switch (atoi(argv[5])) {
            case 2:
                format = audio::Format::FMT_S16_LE;
                break;
            default:
                log::error("Only support format = 2(FMT_S16_LE)\r\n");
                break;
            }
        }

        log::info("Ready to record and save to %s\r\n", path.c_str());
        audio::Recorder r = audio::Recorder(nullptr, sample_rate, format, channel);

        while (!app::need_exit()) {
            Bytes *b = r.record(-1);
            log::info("read bytes %p len %ld\r\n", b->data, b->data_len);
            delete b;

            time::sleep_ms(200);
        }
        r.finish();

        break;
    }
    case 2:
    {
        int volume = 10;
        if (argc > 2) volume = atoi(argv[2]);

        log::info("Set record volume:%d\r\n", volume);
        audio::Recorder r = audio::Recorder();
        int new_volume = r.volume(volume);
        log::info("Get record volume:%d\r\n", new_volume);
        break;
    }
    case 3:
    {
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1;
        audio::Format format = audio::Format::FMT_S16_LE;
        if (argc > 2) path = argv[2];
        if (argc > 3) sample_rate = atoi(argv[3]);
        if (argc > 4) channel = atoi(argv[4]);
        if (argc > 5) {
            switch (atoi(argv[5])) {
            case 2:
                format = audio::Format::FMT_S16_LE;
                break;
            default:
                log::error("Only support format = 2(FMT_S16_LE)\r\n");
                break;
            }
        }

        log::info("Playback %s\r\n", path.c_str());
        audio::Player p = audio::Player(path, sample_rate, format, channel);

        p.play();
        log::info("Playback finish\r\n");
        while (!app::need_exit()) {
            time::sleep_ms(200);
        }
        break;
    }
    case 4:
    {
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1;
        audio::Format format = audio::Format::FMT_S16_LE;
        if (argc > 2) path = argv[2];
        if (argc > 3) sample_rate = atoi(argv[3]);
        if (argc > 4) channel = atoi(argv[4]);
        if (argc > 5) {
            switch (atoi(argv[5])) {
            case 2:
                format = audio::Format::FMT_S16_LE;
                break;
            default:
                log::error("Only support format = 2(FMT_S16_LE)\r\n");
                break;
            }
        }

        log::info("Playback %s\r\n", path.c_str());
        audio::Player p = audio::Player(path, sample_rate, format, channel);
        p.play();
        while (!app::need_exit()) {
            time::sleep_ms(1000);
        }
        break;
    }
    case 5:
    {
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1;
        audio::Format format = audio::Format::FMT_S16_LE;
        if (argc > 2) path = argv[2];
        if (argc > 3) sample_rate = atoi(argv[3]);
        if (argc > 4) channel = atoi(argv[4]);
        if (argc > 5) {
            switch (atoi(argv[5])) {
            case 2:
                format = audio::Format::FMT_S16_LE;
                break;
            default:
                log::error("Only support format = 2(FMT_S16_LE)\r\n");
                break;
            }
        }

        log::info("Ready to record and save to %s\r\n", path.c_str());
        audio::Recorder r = audio::Recorder("", sample_rate, format, channel);

        FILE *file;
        file = fopen(path.c_str(), "wb+");
        err::check_null_raise(file);

        while (!app::need_exit()) {
            Bytes *b = r.record(-1);
            log::info("read bytes %p len %ld\r\n", b->data, b->data_len);
            fwrite(b->data, 1, b->data_len, file);
            delete b;

            time::sleep_ms(200);
        }
        r.finish();
        fclose(file);

        break;
    }
    case 6:
    {
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1;
        audio::Format format = audio::Format::FMT_S16_LE;
        if (argc > 2) path = argv[2];
        if (argc > 3) sample_rate = atoi(argv[3]);
        if (argc > 4) channel = atoi(argv[4]);
        if (argc > 5) {
            switch (atoi(argv[5])) {
            case 2:
                format = audio::Format::FMT_S16_LE;
                break;
            default:
                log::error("Only support format = 2(FMT_S16_LE)\r\n");
                break;
            }
        }

        log::info("Playback %s\r\n", path.c_str());
        audio::Player p = audio::Player("", sample_rate, format, channel);
        FILE *file;
        file = fopen(path.c_str(), "rb+");
        err::check_null_raise(file);
        uint8_t buffer[4096];

        int read_len = 0;
        while ((read_len = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            Bytes data(buffer, read_len);
            err::check_raise(p.play(&data), "play failed!\r\n");
        }
        fclose(file);

        break;
    }
    case 7:
    {
        int volume = 10;
        if (argc > 2) volume = atoi(argv[2]);

        log::info("Set player volume:%d\r\n", volume);
        audio::Player r = audio::Player();
        int new_volume = r.volume(volume);
        log::info("Get player volume:%d\r\n", new_volume);
        break;
    }
    case 100:
    {
        decode_video(argc, argv);
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
