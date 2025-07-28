
// #include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_audio.hpp"
#include "maix_image.hpp"
#include <stdio.h>

using namespace maix;

#if PLATFORM_MAIXCAM || PLATFORM_MAIXCAM2
// FIXME: only for maixcam/maixcam2, this is a ugly example
// comment for linux platform for now to make CI pass

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#endif // PLATFORM_MAIXCAM || PLATFORM_MAIXCAM2

#include <stdio.h>
#include <stdlib.h>

static void helper(void)
{
    auto img = image::Image(320, 240);
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
#if PLATFORM_MAIXCAM || PLATFORM_MAIXCAM2
// FIXME: only for maixcam/maixcam2, this is a ugly example
// comment for linux platform for now to make CI pass

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
        // r.period_size(1920);
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
                    if (data) {
                        log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                        time::sleep_ms(100);
                        delete data;
                    } else {
                        log::info("Record %ld ms, used %lld ms", record_ms, time::ticks_ms() - t);
                    }
                } else {
                    auto data = r.record(record_ms);
                    if (data) {
                        err::check_bool_raise(data->data_len == (size_t)record_bytes, "Record bytes error");
                        log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                        delete data;
                    } else {
                        log::info("Record %ld ms, used %lld ms", record_ms, time::ticks_ms() - t);
                    }
                }
            } else {
                if (record_ms < 0) {
                    auto data = r.record(record_ms);
                    if (data) {
                        log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                        time::sleep_ms(100);
                    } else {
                        log::info("Record %ld ms, used %lld ms", record_ms, time::ticks_ms() - t);
                    }
                } else {
                    auto data = r.record(record_ms);
                    if (data) {
                        log::info("Record %ld ms, bytes:%d, used %lld ms", record_ms, data->size(), time::ticks_ms() - t);
                        time::sleep_ms(record_ms);
                    } else {
                        log::info("Record %ld ms, used %lld ms", record_ms, time::ticks_ms() - t);
                    }
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
    default:
        helper();
        return 0;
    }

#endif // PLATFORM_MAIXCAM || PLATFORM_MAIXCAM2
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
