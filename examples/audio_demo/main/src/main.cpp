
// #include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_audio.hpp"

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

#include <tinyalsa/pcm.h>
#include <tinyalsa/mixer.h>
#include <stdio.h>
#include <stdlib.h>

static size_t read_frames(void **frames)
{
    unsigned int card = 0;
    unsigned int device = 0;
    int flags = PCM_IN;

    const struct pcm_config config = {
        .channels = 1,
        .rate = 16000,
        .period_size = 1024,
        .period_count = 2,
        .format = PCM_FORMAT_S16_LE,
        .start_threshold = 1024,
        .stop_threshold = 1024 * 2,
        .silence_threshold = 2 * 1024,
    };

    struct pcm *pcm = pcm_open(card, device, flags, &config);
    if (pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for PCM\n");
        return 0;
    } else if (!pcm_is_ready(pcm)){
        pcm_close(pcm);
        fprintf(stderr, "failed to open PCM\n");
        return 0;
    }

    unsigned int frame_size = pcm_frames_to_bytes(pcm, 1);
    unsigned int frames_per_sec = pcm_get_rate(pcm);

    *frames = malloc(frame_size * frames_per_sec);
    if (*frames == NULL) {
        fprintf(stderr, "failed to allocate frames\n");
        pcm_close(pcm);
        return 0;
    }

    int read_count = pcm_readi(pcm, *frames, frames_per_sec);

    size_t byte_count = pcm_frames_to_bytes(pcm, read_count);

    pcm_close(pcm);

    return byte_count;
}

static int write_file(const void *frames, size_t size)
{
    FILE *output_file = fopen("audio.raw", "wb");
    if (output_file == NULL) {
        perror("failed to open 'audio.raw' for writing");
        return EXIT_FAILURE;
    }
    fwrite(frames, 1, size, output_file);
    fclose(output_file);
    return 0;
}

static void test_tiny_alsa_record() {
    void *frames = NULL;
    size_t size = 0;
    log::info("read frames");
    size = read_frames(&frames);
    if (size == 0) {
        return;
    }
    log::info("write frames to files");
    if (write_file(frames, size) < 0) {
        free(frames);
        return;
    }

    free(frames);
}


static long int file_size(FILE * file)
{
    if (fseek(file, 0, 0) < 0) {
        return -1;
    }
    long int file_size = ftell(file);
    if (fseek(file, 0, 0) < 0) {
        return -1;
    }
    return file_size;
}

static size_t read_file(void ** frames){

    FILE * input_file = fopen("audio.raw", "rb");
    if (input_file == NULL) {
        perror("failed to open 'audio.raw' for writing");
        return 0;
    }

    long int size = file_size(input_file);
    if (size < 0) {
        perror("failed to get file size of 'audio.raw'");
        fclose(input_file);
        return 0;
    }

    *frames = malloc(size);
    if (*frames == NULL) {
        fprintf(stderr, "failed to allocate frames\n");
        fclose(input_file);
        return 0;
    }

    size = fread(*frames, 1, size, input_file);

    fclose(input_file);

    return size;
}

static int write_frames(const void * frames, size_t byte_count){

    unsigned int card = 1;
    unsigned int device = 0;
    int flags = PCM_OUT;

    const struct pcm_config config = {
        .channels = 1,
        .rate = 16000,
        .period_size = 1024,
        .period_count = 2,
        .format = PCM_FORMAT_S16_LE,
        .start_threshold = 1024,
        .stop_threshold = 1024 * 2,
        .silence_threshold = 1024 * 2,
    };

    struct pcm * pcm = pcm_open(card, device, flags, &config);
    if (pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for PCM\n");
        return -1;
    } else if (!pcm_is_ready(pcm)){
        pcm_close(pcm);
        fprintf(stderr, "failed to open PCM\n");
        return -1;
    }

    unsigned int frame_count = pcm_bytes_to_frames(pcm, byte_count);

    int err = pcm_writei(pcm, frames, frame_count);
    if (err < 0) {
      printf("error: %s\n", pcm_get_error(pcm));
    }

    pcm_close(pcm);

    return 0;
}

static void test_tiny_alsa_playback() {
    void *frames;
    size_t size;

    size = read_file(&frames);
    if (size == 0) {
        return;
    }

    if (write_frames(frames, size) < 0) {
        return;
    }

    free(frames);
}


static void print_enum(struct mixer_ctl *ctl)
{
    unsigned int num_enums;
    unsigned int i;
    unsigned int value;
    const char *string;

    num_enums = mixer_ctl_get_num_enums(ctl);
    value = mixer_ctl_get_value(ctl, 0);

    for (i = 0; i < num_enums; i++) {
        string = mixer_ctl_get_enum_string(ctl, i);
        printf("%s%s, ", value == i ? "> " : "", string);
    }
}


static void print_control_values(struct mixer_ctl *control)
{
    enum mixer_ctl_type type;
    unsigned int num_values;
    unsigned int i;
    int min, max;
    int ret;
    char *buf = NULL;

    type = mixer_ctl_get_type(control);
    num_values = mixer_ctl_get_num_values(control);

    if ((type == MIXER_CTL_TYPE_BYTE) && (num_values > 0)) {
        buf = (char *)calloc(1, num_values);
        if (buf == NULL) {
            fprintf(stderr, "Failed to alloc mem for bytes %u\n", num_values);
            return;
        }

        ret = mixer_ctl_get_array(control, buf, num_values);
        if (ret < 0) {
            fprintf(stderr, "Failed to mixer_ctl_get_array\n");
            free(buf);
            return;
        }
    }

    for (i = 0; i < num_values; i++) {
        switch (type)
        {
        case MIXER_CTL_TYPE_INT:
            printf("%d", mixer_ctl_get_value(control, i));
            break;
        case MIXER_CTL_TYPE_BOOL:
            printf("%s", mixer_ctl_get_value(control, i) ? "On" : "Off");
            break;
        case MIXER_CTL_TYPE_ENUM:
            print_enum(control);
            break;
        case MIXER_CTL_TYPE_BYTE:
            printf("%02hhx", buf[i]);
            break;
        default:
            printf("unknown");
            break;
        };
        if ((i + 1) < num_values) {
           printf(", ");
        }
    }

    if (type == MIXER_CTL_TYPE_INT) {
        min = mixer_ctl_get_range_min(control);
        max = mixer_ctl_get_range_max(control);
        printf(" (range %d->%d)", min, max);
    }

    free(buf);
}
static void list_controls(struct mixer *mixer, int print_all)
{
    struct mixer_ctl *ctl;
    const char *name, *type;
    unsigned int num_ctls, num_values, device;
    unsigned int i;

    num_ctls = mixer_get_num_ctls(mixer);

    printf("Number of controls: %u\n", num_ctls);

    if (print_all)
        printf("ctl\ttype\tnum\t%-40s\tdevice\tvalue\n", "name");
    else
        printf("ctl\ttype\tnum\t%-40s\tdevice\n", "name");

    for (i = 0; i < num_ctls; i++) {
        ctl = mixer_get_ctl(mixer, i);

        name = mixer_ctl_get_name(ctl);
        type = mixer_ctl_get_type_string(ctl);
        num_values = mixer_ctl_get_num_values(ctl);
        device = mixer_ctl_get_device(ctl);
        printf("%u\t%s\t%u\t%-40s\t%u", i, type, num_values, name, device);
        if (print_all)
            print_control_values(ctl);
        printf("\n");
    }
}

static void test_tiny_alsa_mixer() {
    struct mixer *mixer = mixer_open(0);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return;
    }

    list_controls(mixer, 1);
    mixer_close(mixer);

    mixer = mixer_open(1);
    if (!mixer) {
        fprintf(stderr, "Failed to open mixer\n");
        return;
    }

    list_controls(mixer, 1);
    mixer_close(mixer);
}

static void helper(void)
{
    log::info(
    "==================================\r\n"
    "Please input command:\r\n"
    "0 [path] [ms_per_record] [sample_rate] [channel] [format] [block]: record audio, ./audio_demo 0 output.pcm 30 16000 1 2 1\r\n"
    "1 [volumn]: set recorder volume(0 ~ 100), ./audio_demo 1 12\r\n"
    "3 [path] [sample_rate] [channel] [format]: playback block, ./audio_demo 3 output.pcm\r\n"
    "4 [path] [sample_rate] [channel] [format]: playback nonblock, ./audio_demo 4 output.pcm 48000 1 2\r\n"
    "5 [volumn]: set player volume(0 ~ 100), ./audio_demo 5 12\r\n"
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
        std::string path = "output.wav";
        int sample_rate = 48000, channel = 1, record_ms = 50;
        audio::Format format = audio::Format::FMT_S16_LE;
        bool block = true;
        if (argc > 2) path = argv[2];
        if (argc > 3) record_ms = atoi(argv[3]);
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
        if (argc > 7) {
            block = atoi(argv[7]) ? true : false;
        }

        log::info("Ready to record %ld ms, and save to %s\r\n", record_ms, path.c_str());
        audio::Recorder r = audio::Recorder(path, sample_rate, format, channel, block);
        r.period_size(1920);
        r.reset();
        err::check_bool_raise(r.sample_rate() == sample_rate);
        err::check_bool_raise(r.format() == format);
        err::check_bool_raise(r.channel() == channel);

        log::info("Record start\r\n");
        auto bytes_per_frame = r.frame_size();
        while (!app::need_exit()) {
            auto t = time::ticks_ms();
            auto record_bytes = record_ms * bytes_per_frame * r.sample_rate() / 1000;
            if (block) {
                if (record_ms < 0) {
                    record_bytes = record_ms;
                    auto data = r.record(record_ms);
                    log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                    time::sleep_ms(100);
                } else {
                    auto data = r.record(record_ms);
                    err::check_bool_raise(data->data_len == (size_t)record_bytes, "Record bytes error");
                    log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                }
            } else {
                if (record_ms < 0) {
                    record_bytes = record_ms;
                    auto data = r.record(record_ms);
                    log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                    time::sleep_ms(record_ms);
                } else {
                    auto data = r.record(record_ms);
                    log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                    time::sleep_ms(record_ms);
                }
            }
        }
        break;
    }
    case 1:
    {
        int volume = 10;
        if (argc > 2) volume = atoi(argv[2]);

        log::info("Set record volume:%d\r\n", volume);
        audio::Recorder r = audio::Recorder();
        log::info("sample rate:%d\n"
                "channel:%d\n"
                "format:%d\n", r.sample_rate(), r.channel(), r.format());

        int new_volume = r.volume(volume);
        log::info("Get record volume:%d\r\n", new_volume);
        break;
    }
    case 3:
    {
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1;
        audio::Format format = audio::Format::FMT_S16_LE;
        bool block = true;
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
        if (argc > 7) {
            block = atoi(argv[7]) ? true : false;
        }

        log::info("Playback %s\r\n", path.c_str());
        audio::Player p = audio::Player(path, sample_rate, format, channel, block);
        log::info("player sample rate:%d", p.sample_rate());
        log::info("player channel:%d", p.channel());
        log::info("player format:%d", p.format());
        if (!block) {
            log::info("Set period count:%d", p.period_count(20));
        }

        p.play();
        log::info("Playback finish\r\n");
        while (!app::need_exit()) {
            auto remain_frames = p.get_remaining_frames();
            log::info("remain_frames: %d\r\n", remain_frames);
            time::sleep_ms(100);
        }
        break;
    }
    case 4:
    {
        std::string path = "output.pcm";
        int sample_rate = 48000, channel = 1;
        audio::Format format = audio::Format::FMT_S16_LE;
        bool block = true;
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
        if (argc > 7) {
            block = atoi(argv[7]) ? true : false;
        }

        log::info("Playback %s\r\n", path.c_str());
        audio::Player p = audio::Player("", sample_rate, format, channel, block);
        err::check_bool_raise(p.sample_rate() == sample_rate);
        err::check_bool_raise(p.format() == format);
        err::check_bool_raise(p.channel() == channel);
        if (!block) {
            log::info("Set period count:%d", p.period_count(20));
        }

        FILE *file;
        file = fopen(path.c_str(), "rb+");
        err::check_null_raise(file);
        uint8_t buffer[4096];

        int read_len = 0;
        while ((read_len = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            Bytes data(buffer, read_len);
            auto t = time::ticks_ms();
            if (block) {
                err::check_raise(p.play(&data), "play failed!\r\n");
                log::info("Play bytes:%d, used %lld ms", data.size(), time::ticks_ms() - t);
            } else {
                auto bytes_per_frames = p.frame_size();
                while ((size_t)(p.get_remaining_frames() * bytes_per_frames) < data.size() && !app::need_exit()) {
                    time::sleep_ms(1);
                }

                err::check_raise(p.play(&data), "play failed!\r\n");
                log::info("Play bytes:%d, used %lld ms", data.size(), time::ticks_ms() - t);
            }
        }
        fclose(file);

        break;
    }
    case 5:
    {
        int volume = 10;
        if (argc > 2) volume = atoi(argv[2]);

        log::info("Set player volume:%d\r\n", volume);
        audio::Player p = audio::Player();
        log::info("sample rate:%d\n"
                "channel:%d\n"
                "format:%d\n", p.sample_rate(), p.channel(), p.format());

        int new_volume =p.volume(volume);
        log::info("Get player volume:%d\r\n", new_volume);
        break;
    }
    case 100:
    {
        decode_video(argc, argv);
        break;
    }
    case 101:
    {
        test_tiny_alsa_record();
        break;
    }
    case 102:
    {
        test_tiny_alsa_playback();
        break;
    }
    case 103:
    {
        test_tiny_alsa_mixer();
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
