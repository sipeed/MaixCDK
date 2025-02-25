/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include <stdint.h>
#include "maix_basic.hpp"
#include "maix_err.hpp"
#include "maix_audio.hpp"
#include <sys/wait.h>
#include "tinyalsa/pcm.h"
#include "tinyalsa/mixer.h"

using namespace maix;

namespace maix::audio
{
    typedef struct {
        int file_size;  // pcm + 44
        int channel;
        int sample_rate;
        int sample_bit;
        int bitrate;
        int data_size;  // size of pcm
    } wav_header_t;

    static int _create_wav_header(wav_header_t *header, uint8_t *data, size_t size)
    {
        if (size < 44) return -1;

        int cnt = 0;
        data[cnt ++] = 'R';
        data[cnt ++] = 'I';
        data[cnt ++] = 'F';
        data[cnt ++] = 'F';

        data[cnt ++] = (uint8_t)((header->file_size - 8) & 0xff);
        data[cnt ++] = (uint8_t)(((header->file_size - 8) >> 8) & 0xff);
        data[cnt ++] = (uint8_t)(((header->file_size - 8) >> 16) & 0xff);
        data[cnt ++] = (uint8_t)(((header->file_size - 8) >> 24) & 0xff);

        data[cnt ++] = 'W';
        data[cnt ++] = 'A';
        data[cnt ++] = 'V';
        data[cnt ++] = 'E';

        data[cnt ++] = 'f';
        data[cnt ++] = 'm';
        data[cnt ++] = 't';
        data[cnt ++] = ' ';

        data[cnt ++] = 16;
        data[cnt ++] = 0;
        data[cnt ++] = 0;
        data[cnt ++] = 0;

        data[cnt ++] = 1;
        data[cnt ++] = 0;

        data[cnt ++] = (uint8_t)header->channel;
        data[cnt ++] = 0;

        data[cnt ++] = (uint8_t)((header->sample_rate) & 0xff);
        data[cnt ++] = (uint8_t)(((header->sample_rate) >> 8) & 0xff);
        data[cnt ++] = (uint8_t)(((header->sample_rate) >> 16) & 0xff);
        data[cnt ++] = (uint8_t)(((header->sample_rate) >> 24) & 0xff);

        data[cnt ++] = (uint8_t)((header->bitrate) & 0xff);
        data[cnt ++] = (uint8_t)(((header->bitrate) >> 8) & 0xff);
        data[cnt ++] = (uint8_t)(((header->bitrate) >> 16) & 0xff);
        data[cnt ++] = (uint8_t)(((header->bitrate) >> 24) & 0xff);

        data[cnt ++] = (uint8_t)(header->channel * header->sample_bit / 8);
        data[cnt ++] = 0;

        data[cnt ++] = (uint8_t)header->sample_bit;
        data[cnt ++] = 0;

        data[cnt ++] = 'd';
        data[cnt ++] = 'a';
        data[cnt ++] = 't';
        data[cnt ++] = 'a';

        data[cnt ++] = (uint8_t)((header->data_size) & 0xff);
        data[cnt ++] = (uint8_t)(((header->data_size) >> 8) & 0xff);
        data[cnt ++] = (uint8_t)(((header->data_size) >> 16) & 0xff);
        data[cnt ++] = (uint8_t)(((header->data_size) >> 24) & 0xff);

        return 0;
    }

    static int _read_wav_header(wav_header_t *header, uint8_t *data, size_t size)
    {
        int cnt = 0;
        if (size < 44) return -1;

        if (data[cnt ++] != 'R' || data[cnt ++] != 'I' || data[cnt ++] != 'F' || data[cnt ++] != 'F')
        {
            printf("RIFF not found in wav header!\r\n");
            return -1;
        }

        cnt += 4; // jump file size

        if (data[cnt ++] != 'W' || data[cnt ++] != 'A' || data[cnt ++] != 'V' || data[cnt ++] != 'E')
        {
            printf("WAVE not found in wav header!\r\n");
            return -2;
        }

        cnt += 4; // jump fmt
        cnt += 4;

        int audio_format = (uint32_t)data[cnt]
                        | ((uint32_t)data[cnt + 1] << 8);
        cnt += 2;
        if (audio_format != 1) {
            printf("audio format is not pcm!\r\n");
            return -3;
        }

        header->channel = (uint32_t)data[cnt]
                        | ((uint32_t)data[cnt + 1] << 8);
        cnt += 2;
        header->sample_rate = (uint32_t)data[cnt]
                            | ((uint32_t)data[cnt + 1] << 8)
                            | ((uint32_t)data[cnt + 2] << 16)
                            | ((uint32_t)data[cnt + 3] << 24);
        cnt += 4;
        header->bitrate = (uint32_t)data[cnt]
                            | ((uint32_t)data[cnt + 1] << 8)
                            | ((uint32_t)data[cnt + 2] << 16)
                            | ((uint32_t)data[cnt + 3] << 24);
        cnt += 6;
        header->sample_bit = (uint32_t)data[cnt]
                            | ((uint32_t)data[cnt + 1] << 8);
        cnt += 6;
        header->data_size = (uint32_t)data[cnt]
                            | ((uint32_t)data[cnt + 1] << 8)
                            | ((uint32_t)data[cnt + 2] << 16)
                            | ((uint32_t)data[cnt + 3] << 24);
        return 0;
    }

    typedef struct {
        uint32_t card;
        uint32_t device;
        std::string path = "";
        struct pcm *pcm = nullptr;
        FILE *file = nullptr;
        wav_header_t wav_header;
        bool block;
    } audio_param_t;

    enum pcm_format to_tinyalsa_format(audio::Format format) {
        switch (format) {
        case FMT_S8:
            return PCM_FORMAT_S8;
        case FMT_S16_LE:
            return PCM_FORMAT_S16_LE;
        case FMT_S32_LE:
            return PCM_FORMAT_S32_LE;
        case FMT_S16_BE:
            return PCM_FORMAT_S16_BE;
        case FMT_S32_BE:
            return PCM_FORMAT_S32_BE;
        default:
            std::string error_msg = "not support audio format(" + std::to_string(format) + ")";
            err::check_raise(err::ERR_ARGS, error_msg);
        }

        return PCM_FORMAT_INVALID;
    }


    static audio::Format _audio_format_from_tinyalsa(enum pcm_format format) {
        switch (format)
        {
        case PCM_FORMAT_S8: return audio::Format::FMT_S8;
        case PCM_FORMAT_S16_LE: return audio::Format::FMT_S16_LE;
        case PCM_FORMAT_S32_LE: return audio::Format::FMT_S32_LE;
        case PCM_FORMAT_S16_BE: return audio::Format::FMT_S16_BE;
        case PCM_FORMAT_S32_BE: return audio::Format::FMT_S32_BE;
        default: return audio::Format::FMT_NONE;
        }
    }

    static enum pcm_format wav_sample_bit_to_tinyalsa_format(int sample_bit)
    {
        switch (sample_bit) {
            case 8: return PCM_FORMAT_S8;
            case 16: return PCM_FORMAT_S16_LE;
            case 32: return PCM_FORMAT_S32_LE;
            default:
            {
                std::string error_msg = "not support sample bit(" + std::to_string(sample_bit) + ")";
                err::check_raise(err::ERR_ARGS, error_msg);
            }
        }
        return PCM_FORMAT_INVALID;
    }

    Recorder::Recorder(std::string path, int sample_rate, audio::Format format, int channel, bool block) {
        if (path.size() > 0) {
            if (fs::splitext(path)[1] != ".wav"
                && fs::splitext(path)[1] != ".pcm") {
                err::check_raise(err::ERR_RUNTIME, "Only files with the `.pcm` and `.wav` extensions are supported.");
            }
        }

        system("arecord -q -r 16000 -f S16_LE -c 1 -s 1024 /dev/null");

        auto tinyalsa_format = to_tinyalsa_format(format);
        audio_param_t *param = new audio_param_t();
        err::check_null_raise(param, "malloc failed");
        struct pcm_config config = {
            .channels = (uint32_t)channel,
            .rate = (uint32_t)sample_rate,
            .period_size = 1024,
            .period_count = 40,
            .format = tinyalsa_format,
            .start_threshold = 1024,
            .stop_threshold = 1024 * 40
        };
        param->file = nullptr;
        param->card = 0;
        param->device = 0;
        param->path = path;
        param->block = block;
        uint32_t pcm_flag = PCM_IN | (!param->block ? PCM_NONBLOCK : 0);
#ifdef PLATFORM_MAIXCAM
        // Fix segment error when start pcm with channel=2 for the first time.
        if (channel != 1 || sample_rate != 16000 || tinyalsa_format != PCM_FORMAT_S16_LE) {
            config.channels = 1;
            config.rate = 16000;
            config.format = PCM_FORMAT_S16_LE;
            param->pcm = pcm_open(param->card, param->device, pcm_flag, &config);
            if (param->pcm == NULL) {
                err::check_null_raise(param->pcm, "failed to allocate memory for PCM");
            } else if (!pcm_is_ready(param->pcm)){
                pcm_close(param->pcm);
                err::check_raise(err::ERR_RUNTIME, "failed to open PCM");
            }
            pcm_prepare(param->pcm);
            pcm_start(param->pcm);
            pcm_close(param->pcm);
            config.channels = (uint32_t)channel;
            config.rate = (uint32_t)sample_rate;
            config.format = tinyalsa_format;
        }
#endif
        param->pcm = pcm_open(param->card, param->device, pcm_flag, &config);
        if (param->pcm == NULL) {
            err::check_null_raise(param->pcm, "failed to allocate memory for PCM");
        } else if (!pcm_is_ready(param->pcm)){
            pcm_close(param->pcm);
            err::check_raise(err::ERR_RUNTIME, "failed to open PCM");
        }
        if (param->block) {
            pcm_prepare(param->pcm);
        }

        {
            int retry_count = 5;
            while (retry_count && !app::need_exit()) {
                pcm_start(param->pcm);
                if (!block) {
                    time::sleep_ms(100);
                }
                auto read_frame_count = 128;
                auto buffer = (uint8_t *)malloc(pcm_frames_to_bytes(param->pcm, read_frame_count));
                auto len = 0;
                if (buffer) {
                    unsigned int avail;
                    struct timespec tstamp;
                    err::check_bool_raise(!pcm_get_htimestamp(param->pcm, &avail, &tstamp), "Get htimestamp failed");
                    len = pcm_readi(param->pcm, buffer, read_frame_count);
                    free(buffer);
                }
                pcm_drain(param->pcm);
                pcm_stop(param->pcm);
                pcm_prepare(param->pcm);
                retry_count --;
                if (len > 0) {
                    break;
                }
            }
        }

        _param = param;
    }

    Recorder::~Recorder() {
        audio_param_t *param = (audio_param_t *)_param;
        if (param) {
            if (param->pcm) {
                pcm_close(param->pcm);
                param->pcm = nullptr;
            }

            if (param->file) {
                fclose(param->file);
                param->file = nullptr;
            }

            delete param;
            _param = nullptr;
        }
    }

    bool Recorder::mute(int data) {
        audio_param_t *param = (audio_param_t *)_param;
        struct mixer *mixer = mixer_open(param->card);
        if (!mixer) {
            err::check_raise(err::ERR_RUNTIME, "Open mixer failed");
        }

        auto ctl = mixer_get_ctl_by_name_and_device(mixer, "ADC Capture Mute", param->device);
        if (!ctl) {
            mixer_close(mixer);
            err::check_raise(err::ERR_RUNTIME, "Get mixer mute ctl failed");
        }

        auto num_values = mixer_ctl_get_num_values(ctl);
        auto is_muted = mixer_ctl_get_value(ctl, 0) ? true : false;

        if (data >= 0) {
            auto dist_mute = (data == 0) ? false : true;
            for (size_t i = 0 ; i < num_values ; i++) {
                auto res = mixer_ctl_set_value(ctl, i, dist_mute);
                if (res != 0) {
                    mixer_close(mixer);
                    err::check_raise(err::ERR_RUNTIME, "Set mixer mute ctl failed, res:" + std::to_string(res));
                }
            }
            is_muted = dist_mute;
        }

        mixer_close(mixer);
        return is_muted;
    }

    int Recorder::volume(int value) {
        audio_param_t *param = (audio_param_t *)_param;
        struct mixer *mixer = mixer_open(param->card);
        if (!mixer) {
            err::check_raise(err::ERR_RUNTIME, "Open mixer failed");
        }

        auto ctl = mixer_get_ctl_by_name_and_device(mixer, "ADC Capture Volume", param->device);
        if (!ctl) {
            mixer_close(mixer);
            err::check_raise(err::ERR_RUNTIME, "Get mixer volume ctl failed");
        }

        auto num_values = mixer_ctl_get_num_values(ctl);
        auto curr_volume = mixer_ctl_get_percent(ctl, 0);
        if (value >= 0) {
            auto dist_volume = value > 100 ? 100 : value;
            for (size_t i = 0 ; i < num_values ; i++) {
                auto res = mixer_ctl_set_percent(ctl, i, dist_volume);
                if (res != 0) {
                    mixer_close(mixer);
                    err::check_raise(err::ERR_RUNTIME, "Set mixer mute ctl failed, res:" + std::to_string(res));
                }
            }

            curr_volume = dist_volume;
        }

        mixer_close(mixer);
        return curr_volume;
    }

    void Recorder::reset(bool start) {
        audio_param_t *param = (audio_param_t *)_param;
        pcm_drain(param->pcm);
        pcm_stop(param->pcm);
        pcm_prepare(param->pcm);
        if (start) {
            pcm_start(param->pcm);
        }
    }

    maix::Bytes *Recorder::record(int record_ms) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;

        if (record_ms < 0) {
            return record_bytes(record_ms);
        } else {
            int record_size = record_ms * pcm_get_rate(pcm) / 1000;
            return record_bytes(pcm_frames_to_bytes(pcm, record_size));
        }
    }

    int Recorder::frame_size(int frame_count) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        frame_count = frame_count <= 0 ? 1 : frame_count;
        return pcm_frames_to_bytes(pcm, frame_count);
    }

    int Recorder::get_remaining_frames() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        unsigned int avail;
        struct timespec tstamp;

        err::check_bool_raise(!pcm_get_htimestamp(pcm, &avail, &tstamp), "Get htimestamp failed");

        return (int)avail;
    }

    int Recorder::period_size(int period_size) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;

        if (period_size >= 0) {
            const struct pcm_config *config = pcm_get_config(pcm);
            struct pcm_config new_config;
            memcpy(&new_config, config, sizeof(new_config));
            new_config.period_size = period_size;
            new_config.start_threshold = new_config.period_size;
            new_config.stop_threshold = new_config.period_size * new_config.period_count;
            auto res = pcm_set_config(pcm, &new_config);
            if (res) {
                log::error("pcm set period size config failed, res = %d: %s", res, pcm_get_error(pcm));
                err::check_bool_raise(!res, "Set audio config failed");
            }
        }

        const struct pcm_config *curr_config = pcm_get_config(pcm);
        return curr_config->period_size;
    }

    int Recorder::period_count(int period_count) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;

        if (period_count >= 0) {
            const struct pcm_config *config = pcm_get_config(pcm);
            struct pcm_config new_config;
            memcpy(&new_config, config, sizeof(new_config));
            new_config.period_count = period_count;
            new_config.stop_threshold = new_config.period_size * new_config.period_count;
            auto res = pcm_set_config(pcm, &new_config);
            if (res) {
                log::error("pcm set period count config failed, res = %d: %s", res, pcm_get_error(pcm));
                err::check_bool_raise(!res, "Set audio config failed");
            }
        }

        const struct pcm_config *curr_config = pcm_get_config(pcm);
        return curr_config->period_count;
    }


    maix::Bytes *Recorder::record_bytes(int record_size) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        const struct pcm_config *config = pcm_get_config(pcm);

        if (param->file == nullptr && param->path.size() > 0) {
            param->file = fopen(param->path.c_str(), "w+");
            err::check_null_raise(param->file, "Open file failed!");

            if (fs::splitext(param->path)[1] == ".wav") {
                wav_header_t header = {
                    .file_size = 44,
                    .channel = (int)config->channels,
                    .sample_rate = (int)config->rate,
                    .sample_bit = (int)pcm_format_to_bits(config->format),
                    .bitrate = (int)(config->channels * config->rate * pcm_format_to_bits(config->format) / 8),
                    .data_size = 0,
                };

                uint8_t buffer[44];
                if (0 != _create_wav_header(&header, buffer, sizeof(buffer))) {
                    err::check_raise(err::ERR_RUNTIME, "create wav failed!");
                }

                if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), param->file)) {
                    err::check_raise(err::ERR_RUNTIME, "write wav header failed!");
                }
            }
        }

        auto remaining_frames = get_remaining_frames();
        auto bytes_per_frame = pcm_frames_to_bytes(pcm, 1);
        auto remaining_bytes = remaining_frames * bytes_per_frame;
        if (record_size < 0) {
            record_size = remaining_bytes;
        } else {
            if (!param->block) {
                if ((uint32_t)record_size > remaining_bytes) {
                    record_size = remaining_bytes;
                }

                if (record_size == 0) {
                    return new Bytes();
                }
            }
        }

        auto remain_frame_count = pcm_bytes_to_frames(pcm, record_size);
        auto out_bytes = new Bytes(nullptr, record_size);
        err::check_null_raise(out_bytes, "Create new bytes failed!");
        while (remain_frame_count > 0 && !app::need_exit()) {
            auto buffer = out_bytes->data + out_bytes->data_len - remain_frame_count * bytes_per_frame;
            auto read_frame_count = remain_frame_count > 1024 ? 1024 : remain_frame_count;
            auto len = pcm_readi(pcm, buffer, read_frame_count);
            if (len == -EPIPE) {
                pcm_prepare(pcm);
            } else if (len < 0) {
                log::error("pcm read error(%d): %s", len, pcm_get_error(pcm));
                delete out_bytes;
                return new Bytes();
            } else {
                // log::info("pcm read total %d, need read:%d, actual read: %d, remain %d ", pcm_frames_to_bytes(pcm, out_bytes->data_len), remain_frame_count, len, remain_frame_count - len);
                remain_frame_count -= len;
            }
        }

        if (param->file)
            fwrite(out_bytes->data, 1, out_bytes->data_len, param->file);

        return out_bytes;
    }

    err::Err Recorder::finish() {
        audio_param_t *param = (audio_param_t *)_param;
        if (param->file) {
            if (fs::splitext(param->path)[1] == ".wav") {
                int file_size = ftell(param->file);
                int pcm_size = file_size - 44;
                char buffer[4];
                buffer[0] = (uint8_t)((file_size - 8) & 0xff);
                buffer[1] = (uint8_t)(((file_size - 8) >> 8) & 0xff);
                buffer[2] = (uint8_t)(((file_size - 8) >> 16) & 0xff);
                buffer[3] = (uint8_t)(((file_size - 8) >> 24) & 0xff);

                fseek(param->file, 4, 0);
                if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), param->file)) {
                    err::check_raise(err::ERR_RUNTIME, "write wav file size failed!");
                }

                buffer[0] = (uint8_t)((pcm_size) & 0xff);
                buffer[1] = (uint8_t)(((pcm_size) >> 8) & 0xff);
                buffer[2] = (uint8_t)(((pcm_size) >> 16) & 0xff);
                buffer[3] = (uint8_t)(((pcm_size) >> 24) & 0xff);
                fseek(param->file, 40, 0);
                if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), param->file)) {
                    err::check_raise(err::ERR_RUNTIME, "write wav data size failed!");
                }
            }

            fflush(param->file);
            fclose(param->file);
            param->file = NULL;
        }

        return err::ERR_NONE;
    }

    int Recorder::sample_rate() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        return pcm_get_rate(pcm);
    }

    audio::Format Recorder::format() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        return _audio_format_from_tinyalsa(pcm_get_format(pcm));
    }

    int Recorder::channel() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        return pcm_get_channels(pcm);
    }
#if CONFIG_BUILD_WITH_MAIXPY
    maix::Bytes *Player::NoneBytes = new maix::Bytes();
#else
    maix::Bytes *Player::NoneBytes = NULL;
#endif

    Player::Player(std::string path, int sample_rate, audio::Format format, int channel, bool block) {
        if (path.size() > 0) {
            if (fs::splitext(path)[1] != ".wav"
                && fs::splitext(path)[1] != ".pcm") {
                err::check_raise(err::ERR_RUNTIME, "Only files with the `.pcm` and `.wav` extensions are supported.");
            }
        }
        auto tinyalsa_format = to_tinyalsa_format(format);
        FILE *new_file = nullptr;
        wav_header_t wav_header = {0};
        if (new_file == NULL && path.size() > 0) {
            new_file = fopen(path.c_str(), "rb+");
            err::check_null_raise(new_file, "Open file failed!");

            if (fs::splitext(path)[1] == ".wav") {
                uint8_t buffer[44];

                if (sizeof(buffer) != fread(buffer, 1, sizeof(buffer), new_file)) {
                    err::check_raise(err::ERR_RUNTIME, "read wav header failed!");
                }

                if (0 != _read_wav_header(&wav_header, buffer, sizeof(buffer))) {
                    err::check_raise(err::ERR_RUNTIME, "parse wav header failed!");
                }

                sample_rate = wav_header.sample_rate;
                channel = wav_header.channel;
                tinyalsa_format = wav_sample_bit_to_tinyalsa_format(wav_header.sample_bit);
                // printf("sample rate:%d channel:%d format:%d\r\n", _sample_rate, _channel, _format);
            }
        }

        audio_param_t *param = new audio_param_t();
        err::check_null_raise(param, "malloc failed");
        struct pcm_config config = {
            .channels = (uint32_t)channel,
            .rate = (uint32_t)sample_rate,
            .period_size = 1024,
            .period_count = 40,
            .format = tinyalsa_format,
            .start_threshold = 1024,
            .stop_threshold = 1024 * 40
        };

        memcpy(&param->wav_header, &wav_header, sizeof(wav_header));
        param->file = new_file;
        param->card = 1;
        param->device = 0;
        param->path = path;
        param->block = block;
        uint32_t pcm_flag = PCM_OUT | (!param->block ? PCM_NONBLOCK : 0);

#ifdef PLATFORM_MAIXCAM
        // Fix segment error when start pcm with samplerate=44100 for the first time.
        if (channel != 1 || sample_rate != 16000 || tinyalsa_format != PCM_FORMAT_S16_LE) {
            config.channels = 1;
            config.rate = 16000;
            config.format = PCM_FORMAT_S16_LE;
            param->pcm = pcm_open(param->card, param->device, pcm_flag, &config);
            if (param->pcm == NULL) {
                err::check_null_raise(param->pcm, "failed to allocate memory for PCM");
            } else if (!pcm_is_ready(param->pcm)){
                pcm_close(param->pcm);
                err::check_raise(err::ERR_RUNTIME, "failed to open PCM");
            }
            pcm_prepare(param->pcm);
            pcm_start(param->pcm);
            uint8_t *data = new uint8_t[320];
            pcm_writei(param->pcm, data, 160);
            delete[] data;
            pcm_drain(param->pcm);
            pcm_close(param->pcm);
            config.channels = (uint32_t)channel;
            config.rate = (uint32_t)sample_rate;
            config.format = tinyalsa_format;
        }
#endif
        param->pcm = pcm_open(param->card, param->device, pcm_flag, &config);
        if (param->pcm == NULL) {
            err::check_null_raise(param->pcm, "failed to allocate memory for PCM");
        } else if (!pcm_is_ready(param->pcm)){
            pcm_close(param->pcm);
            err::check_raise(err::ERR_RUNTIME, "failed to open PCM");
        }
        _param = param;
    }

    Player::~Player() {
        audio_param_t *param = (audio_param_t *)_param;
        if (param) {
            if (param->pcm) {
                pcm_close(param->pcm);
                param->pcm = nullptr;
            }

            if (param->file) {
                fclose(param->file);
                param->file = nullptr;
            }

            delete param;
            _param = nullptr;
        }
    }

    int Player::volume(int value) {
        audio_param_t *param = (audio_param_t *)_param;
        struct mixer *mixer = mixer_open(param->card);
        if (!mixer) {
            err::check_raise(err::ERR_RUNTIME, "Open mixer failed");
        }

        auto ctl = mixer_get_ctl_by_name_and_device(mixer, "DAC Playback Volume", param->device);
        if (!ctl) {
            mixer_close(mixer);
            err::check_raise(err::ERR_RUNTIME, "Get mixer volume ctl failed");
        }

        auto num_values = mixer_ctl_get_num_values(ctl);
        auto curr_volume = mixer_ctl_get_percent(ctl, 0);
        curr_volume = 100 - curr_volume;
        if (value >= 0) {
            auto dist_volume = value > 100 ? 100 : value;
            dist_volume = 100 - dist_volume;
            for (size_t i = 0 ; i < num_values ; i++) {
                auto res = mixer_ctl_set_percent(ctl, i, dist_volume);
                if (res != 0) {
                    mixer_close(mixer);
                    err::check_raise(err::ERR_RUNTIME, "Set mixer mute ctl failed, res:" + std::to_string(res));
                }
            }

            curr_volume = 100 - dist_volume;
        }

        mixer_close(mixer);
        return curr_volume;
    }

    err::Err Player::play(maix::Bytes *data) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        err::Err ret = err::ERR_NONE;

        auto bytes_per_frame = pcm_frames_to_bytes(pcm, 1);
        if (!data || !data->data || !data->size()) {
            if (param->file == NULL && param->path.size() > 0) {
                param->file = fopen(param->path.c_str(), "rb+");
                err::check_null_raise(param->file, "Open file failed!");
            }

            if (fs::splitext(param->path)[1] == ".wav") {
                fseek(param->file, 44, 0);
            }

            int read_len = 0;
            uint32_t buffer_size = 4096;
            uint8_t *buffer = new uint8_t[buffer_size];
            while ((read_len = fread(buffer, 1, buffer_size, param->file)) > 0 && !app::need_exit()) {
                auto remain_frame_count = pcm_bytes_to_frames(pcm, read_len);
                auto buffer2 = buffer + read_len - remain_frame_count * bytes_per_frame;
                while (remain_frame_count > 0 && !app::need_exit()) {
                    if (!param->block) {
                        auto remain_frames = get_remaining_frames();
                        if ((uint32_t)remain_frames < remain_frame_count) {
                            time::sleep_ms(1);
                            continue;
                        }
                    }

                    auto len = pcm_writei(pcm, buffer2, remain_frame_count);
                    if (len == -EPIPE) {
                        pcm_prepare(pcm);
                        break;
                    } else if (len < 0) {
                        log::error("pcm write error(%d): %s", len, pcm_get_error(pcm));
                        return err::ERR_RUNTIME;
                    } else {
                        // log::info("pcm write total %d, write %d, remain %d ", pcm_frames_to_bytes(pcm, remain_frame_count), len, remain_frame_count);
                        remain_frame_count -= len;
                    }
                }
            }
        } else {
            auto remain_frame_count = pcm_bytes_to_frames(pcm, data->data_len);
            auto buffer2 = data->data + data->data_len - remain_frame_count * bytes_per_frame;
            while (remain_frame_count > 0 && !app::need_exit()) {
                if (!param->block) {
                    auto remain_frames = get_remaining_frames();
                    if ((uint32_t)remain_frames < remain_frame_count) {
                        time::sleep_ms(100);
                        continue;
                    }
                }

                auto len = pcm_writei(pcm, buffer2, remain_frame_count);
                if (len == -EPIPE) {
                    pcm_prepare(pcm);
                    break;
                } else if (len < 0) {
                    log::error("pcm write error(%d): %s", len, pcm_get_error(pcm));
                    return err::ERR_RUNTIME;
                } else {
                    // log::info("pcm write total %d, need write:%d, actual write: %d, remain %d ", pcm_frames_to_bytes(pcm, remain_frame_count), remain_frame_count, len, remain_frame_count - len);
                    remain_frame_count -= len;
                }
            }
        }

        return ret;
    }

    int Player::frame_size(int frame_count) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        frame_count = frame_count <= 0 ? 1 : frame_count;
        return pcm_frames_to_bytes(pcm, frame_count);
    }

    int Player::get_remaining_frames() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        unsigned int avail;
        struct timespec tstamp;

        err::check_bool_raise(!pcm_get_htimestamp(pcm, &avail, &tstamp), "Get htimestamp failed");
        return (int)avail;
    }

    int Player::period_size(int period_size) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;

        if (period_size >= 0) {
            const struct pcm_config *config = pcm_get_config(pcm);
            struct pcm_config new_config;
            memcpy(&new_config, config, sizeof(new_config));
            new_config.period_size = period_size;
            new_config.start_threshold = new_config.period_size;
            new_config.stop_threshold = new_config.period_size * new_config.period_count;
            err::check_bool_raise(!pcm_set_config(pcm, &new_config), "Set audio config failed");
        }

        const struct pcm_config *curr_config = pcm_get_config(pcm);
        return curr_config->period_size;
    }

    int Player::period_count(int period_count) {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;

        if (period_count >= 0) {
            const struct pcm_config *config = pcm_get_config(pcm);
            struct pcm_config new_config;
            memcpy(&new_config, config, sizeof(new_config));
            new_config.period_count = period_count;
            new_config.stop_threshold = new_config.period_size * new_config.period_count;
            err::check_bool_raise(!pcm_set_config(pcm, &new_config), "Set audio config failed");
        }

        const struct pcm_config *curr_config = pcm_get_config(pcm);
        return curr_config->period_count;
    }

    void Player::reset(bool start) {
        audio_param_t *param = (audio_param_t *)_param;
        pcm_drain(param->pcm);
        pcm_stop(param->pcm);
        pcm_prepare(param->pcm);
        if (start) {
            pcm_start(param->pcm);
        }
    }

    int Player::sample_rate() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        return pcm_get_rate(pcm);
    }

    audio::Format Player::format() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        return _audio_format_from_tinyalsa(pcm_get_format(pcm));
    }

    int Player::channel() {
        audio_param_t *param = (audio_param_t *)_param;
        struct pcm *pcm = param->pcm;
        return pcm_get_channels(pcm);
    }
} // namespace maix::audio