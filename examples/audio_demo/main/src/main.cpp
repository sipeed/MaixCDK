
// #include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_audio.hpp"
#include "alsa/asoundlib.h"

using namespace maix;

static void helper(void)
{
    log::info(
    "==================================\r\n"
    "Please input command:\r\n"
    "0 [path] [record_ms] [sample_rate] [channel] [format]: record and save to path, ./audio_demo 0 output.pcm 3\r\n"
    "1 [path] [sample_rate] [channel] [format]: record and save to path, ./audio_demo 1 output.pcm 48000 1 2\r\n"
    "2 [volumn]: set recorder volume, ./audio_demo 2 12\r\n"
    "3 [path] [sample_rate] [channel] [format]: playback path, ./audio_demo 3 output.pcm\r\n"
    "4 [path] [sample_rate] [channel] [format]: playback path, ./audio_demo 4 output.pcm 48000 1 2\r\n"
    "\r\n"
    "Note: format = 2, means FMT_S16_LE\r\n"
    "==================================\r\n");
}

int _main(int argc, char* argv[])
{
    int cmd = 0;
    if (argc > 1) {
        cmd = atoi(argv[1]);
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
        audio::Recorder r = audio::Recorder(path, sample_rate, format, channel);

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

        while (!app::need_exit()) {
            time::sleep_ms(1000);
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
