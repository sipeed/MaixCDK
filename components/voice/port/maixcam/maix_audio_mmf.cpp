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
#include "alsa/asoundlib.h"
#include <sys/wait.h>

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
        if (size < 44) return -1;

        int cnt = 0;
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

        int audio_format = (uint32_t)data[cnt ++]
                        | ((uint32_t)data[cnt ++] << 8);
        if (audio_format != 1) {
            printf("audio format is not pcm!\r\n");
            return -3;
        }

        header->channel = (uint32_t)data[cnt ++]
                        | ((uint32_t)data[cnt ++] << 8);
        header->sample_rate = (uint32_t)data[cnt ++]
                            | ((uint32_t)data[cnt ++] << 8)
                            | ((uint32_t)data[cnt ++] << 16)
                            | ((uint32_t)data[cnt ++] << 24);
        header->bitrate = (uint32_t)data[cnt ++]
                            | ((uint32_t)data[cnt ++] << 8)
                            | ((uint32_t)data[cnt ++] << 16)
                            | ((uint32_t)data[cnt ++] << 24);
        cnt += 2;
        header->sample_bit = (uint32_t)data[cnt ++]
                            | ((uint32_t)data[cnt ++] << 8);
        cnt += 4;
        header->data_size = (uint32_t)data[cnt ++]
                            | ((uint32_t)data[cnt ++] << 8)
                            | ((uint32_t)data[cnt ++] << 16)
                            | ((uint32_t)data[cnt ++] << 24);
        return 0;
    }

    static int _alsa_capture_init(  snd_pcm_t **handle, snd_pcm_hw_params_t **hw_params, int *period_size,
                                    snd_pcm_format_t format, unsigned int sample_rate, unsigned int channels)
    {
        int err = 0;

        if (handle) *handle = NULL;
        if (hw_params) *hw_params = NULL;

        if ((err = snd_pcm_open(handle, "hw:0,0", SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0) {
            printf("Cannot open audio device hw:0,0 (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if((err = snd_pcm_hw_params_malloc(hw_params)) < 0) {
            printf("hw params malloc failed (%s)\n",snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_any(*handle, *hw_params)) < 0) {
            printf("Can't set hardware parameters (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_access(*handle, *hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
            printf("Can't set access type (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_format(*handle, *hw_params, format)) < 0) {
            printf("Can't set format (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_rate_near(*handle, *hw_params, &sample_rate, 0)) < 0) {
            printf("Can't set sample rate (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_channels(*handle, *hw_params, channels)) < 0) {
            printf("Can't set channel count (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params(*handle, *hw_params)) < 0) {
            printf("Can't set hardware parameters (%s)\n", snd_strerror(err));
            goto _exit;
        }

        snd_pcm_uframes_t period_value;
        if ((err = snd_pcm_hw_params_get_period_size(*hw_params, &period_value, 0)) < 0) {
            printf("Can't get period size (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if (period_size) *period_size = (int)period_value;

        snd_pcm_hw_params_free(*hw_params);
        *hw_params = NULL;

        if((err = snd_pcm_prepare(*handle)) < 0) {
            printf("not perpare (%s)\n",snd_strerror(err));
            goto _exit;
        }

        return err;
    _exit:
        if (*hw_params) snd_pcm_hw_params_free(*hw_params);
        if (*handle) snd_pcm_close(*handle);
        return err;
    }

    static int _alsa_capture_deinit(snd_pcm_t *handle)
    {
        if (handle) {
            return snd_pcm_close(handle);
        }
        return 0;
    }

    static void *_alsa_prepare_buffer(snd_pcm_format_t format, unsigned int channels, int period_size, snd_pcm_uframes_t  *buffer_size)
    {
        int buffer_frames = period_size;
        int frame_byte = snd_pcm_format_width(format) / 8;
        snd_pcm_uframes_t new_buffer_size = buffer_frames * frame_byte * channels;
        void *buffer = malloc(new_buffer_size);
        if (!buffer) {
            printf("Buffer allocation failed.\n");
            return NULL;
        }

        if (buffer_size) *buffer_size = new_buffer_size;
        return buffer;
    }

    static int _alsa_capture_pop(snd_pcm_t *handle, snd_pcm_format_t format, unsigned int channels, int period_size, void *buffer, snd_pcm_uframes_t buffer_size)
    {
        int len = 0;
        int frame_size = period_size;
        int frame_byte = snd_pcm_format_width(format) / 8;
        if (buffer_size < frame_byte * channels * frame_size) {
            printf("Bad buffer size, input %ld, need %d\r\n", buffer_size, frame_byte * channels * frame_size);
            return 0;
        }

        if ((len = (int)snd_pcm_readi(handle, buffer, frame_size)) < 0) {
            return len;
        }

        return len * frame_byte * channels;
    }

    static int _alsa_player_init(  snd_pcm_t **handle, snd_pcm_hw_params_t **hw_params, int *period_size,
                                    snd_pcm_format_t format, unsigned int sample_rate, unsigned int channels)
    {
        int err = 0;

        if (handle) *handle = NULL;
        if (hw_params) *hw_params = NULL;

        if ((err = snd_pcm_open(handle, "hw:1,0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0) {
            printf("Cannot open audio device hw:1,0 (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if((err = snd_pcm_hw_params_malloc(hw_params)) < 0) {
            printf("hw params malloc failed (%s)\n",snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_any(*handle, *hw_params)) < 0) {
            printf("Can't set hardware parameters (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_access(*handle, *hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
            printf("Can't set access type (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_format(*handle, *hw_params, format)) < 0) {
            printf("Can't set format (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_rate_near(*handle, *hw_params, &sample_rate, 0)) < 0) {
            printf("Can't set sample rate (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params_set_channels(*handle, *hw_params, channels)) < 0) {
            printf("Can't set channel count (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if ((err = snd_pcm_hw_params(*handle, *hw_params)) < 0) {
            printf("Can't set hardware parameters (%s)\n", snd_strerror(err));
            goto _exit;
        }

        snd_pcm_uframes_t period_value;
        if ((err = snd_pcm_hw_params_get_period_size(*hw_params, &period_value, 0)) < 0) {
            printf("Can't get period size (%s)\n", snd_strerror(err));
            goto _exit;
        }

        if (period_size) *period_size = (int)period_value;

        snd_pcm_hw_params_free(*hw_params);
        *hw_params = NULL;

        if((err = snd_pcm_prepare(*handle)) < 0) {
            printf("not perpare (%s)\n",snd_strerror(err));
            goto _exit;
        }

        return err;
    _exit:
        if (*hw_params) snd_pcm_hw_params_free(*hw_params);
        if (*handle) snd_pcm_close(*handle);
        return err;
    }

    static int _alsa_player_deinit(snd_pcm_t *handle)
    {
        if (handle) {
            return snd_pcm_close(handle);
        }
        return 0;
    }

    static int _alsa_player_push(snd_pcm_t *handle, snd_pcm_format_t format, unsigned int channels, void *buffer, snd_pcm_uframes_t buffer_size)
    {
        int len = 0;
        int frame_byte = snd_pcm_format_width(format) / 8;
        int write_count = buffer_size / frame_byte / channels;
        if (snd_pcm_wait(handle, 1000) < 0) {
            printf("alsa pcm wait timeout!\r\n");
            return 0;
        }

        if ((len = (int)snd_pcm_writei(handle, buffer, write_count)) < 0) {
            return len;
        }

        return len * frame_byte * channels;
    }

    static bool first_init = true;
    static void _trigger_segmentation_fault(void) {
        pid_t pid;
        int status;
        pid = fork();
        if (pid == -1) {
            perror("pcm fix bug failed!\r\n");
            return;
        } else if (pid == 0) {
            char buffer[1024];
            snd_pcm_t *handle;
            snd_pcm_hw_params_t *hwparams;
            int period_size;
            _alsa_capture_init(&handle, &hwparams, &period_size, SND_PCM_FORMAT_S16_LE, 48000, 1);
            _alsa_capture_pop(handle, SND_PCM_FORMAT_S16_LE, 1, period_size, buffer, sizeof(buffer));
            _alsa_capture_deinit(handle);
            exit(EXIT_SUCCESS);
        } else {
            waitpid(pid, &status, 0);
        }
    }

    static void _fix_segmentation_fault_bug(void) {
        if (!first_init) {
            return;
        }

        for (int i = 0; i < 3; i ++) {
            _trigger_segmentation_fault();
        }

        first_init = false;
    }

    static snd_pcm_format_t _alsa_format_from_maix(audio::Format format) {
        switch (format)
        {
        case audio::Format::FMT_S8: return SND_PCM_FORMAT_S8;
        case audio::Format::FMT_U8: return SND_PCM_FORMAT_U8;
        case audio::Format::FMT_S16_LE: return SND_PCM_FORMAT_S16_LE;
        case audio::Format::FMT_S32_LE: return SND_PCM_FORMAT_S32_LE;
        case audio::Format::FMT_S16_BE: return SND_PCM_FORMAT_S16_BE;
        case audio::Format::FMT_S32_BE: return SND_PCM_FORMAT_S32_BE;
        case audio::Format::FMT_U16_LE: return SND_PCM_FORMAT_U16_LE;
        case audio::Format::FMT_U32_LE: return SND_PCM_FORMAT_U32_LE;
        case audio::Format::FMT_U16_BE: return SND_PCM_FORMAT_U16_BE;
        case audio::Format::FMT_U32_BE: return SND_PCM_FORMAT_U32_BE;
        default: return SND_PCM_FORMAT_UNKNOWN;
        }
        return SND_PCM_FORMAT_UNKNOWN;
    }

    Recorder::Recorder(std::string path, int sample_rate, audio::Format format, int channel) {
        _path = path;
        _sample_rate = sample_rate;
        _format = format;
        _channel = channel;

        if (fs::splitext(_path) != ".wav"
            && fs::splitext(_path) != ".pcm") {
            err::check_raise(err::ERR_RUNTIME, "Only files with the `.pcm` and `.wav` extensions are supported.");
        }

        // alsa init
        snd_pcm_t *handle = NULL;
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_format_t format_p = _alsa_format_from_maix(format);
        _fix_segmentation_fault_bug();
        err::check_bool_raise(0 <= _alsa_capture_init(&handle, &hwparams, &_period_size, format_p, sample_rate, channel), "capture init failed");
        _handle = handle;
        snd_pcm_uframes_t buffer_size = 0;
        _buffer = _alsa_prepare_buffer(format_p, channel, _period_size, &buffer_size);
        err::check_null_raise(_buffer, "record buffer init failed!");
        _buffer_size = (size_t)buffer_size;

        // file init
        _file = NULL;
    }

    Recorder::~Recorder() {
        if (_handle) {
            snd_pcm_drain((snd_pcm_t *)_handle);
            err::check_bool_raise(0 <= _alsa_capture_deinit((snd_pcm_t *)_handle));
            _handle = NULL;
        }

        if (_file) {
            fclose(_file);
            _file = NULL;
        }

        if (_buffer) {
            free(_buffer);
            _buffer_size = 0;
        }
    }

    int Recorder::volume(int value) {
        char buffer[512];
        value = value < 0 ? 0 : value;

        if (value != -1) {
            snprintf(buffer, sizeof(buffer), "amixer -Dhw:0 cset name='ADC Capture Volume' %d &> /dev/zero", value);
            system(buffer);
        }

        FILE *fp;
        fp = popen("amixer -Dhw:0 cget name='ADC Capture Volume'", "r");
        if (fp == NULL) {
            return -1;
        }

        int read_value = -1;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, ": values=")) {
                sscanf(buffer, "  : values=%d,%*d", &read_value);
            }
        }
        pclose(fp);

        return read_value;
    }

    static int format_to_sample_bit(audio::Format format)
    {
        switch (format) {
        case audio::Format::FMT_NONE: return 0;
        case audio::Format::FMT_S8: return 8;
        case audio::Format::FMT_S16_LE: return 16;
        case audio::Format::FMT_S32_LE: return 32;
        case audio::Format::FMT_S16_BE: return 16;
        case audio::Format::FMT_S32_BE: return 32;
        case audio::Format::FMT_U8: return 8;
        case audio::Format::FMT_U16_LE: return 16;
        case audio::Format::FMT_U32_LE: return 32;
        case audio::Format::FMT_U16_BE: return 16;
        case audio::Format::FMT_U32_BE: return 32;
        default: return 0;
        }
        return 0;
    }

    static audio::Format wav_sample_bit_to_format(int sample_bit)
    {
        switch (sample_bit) {
            case 8: return audio::Format::FMT_U8;
            case 16: return audio::Format::FMT_S16_LE;
            case 32: return audio::Format::FMT_S32_LE;
            default: return audio::Format::FMT_NONE;
        }

        return audio::Format::FMT_NONE;
    }

    maix::Bytes *Recorder::record(int record_ms) {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;

        if (_file == NULL && _path.size() > 0) {
            _file = fopen(_path.c_str(), "w+");
            err::check_null_raise(_file, "Open file failed!");

            if (fs::splitext(_path) == ".wav") {
                wav_header_t header = {
                    .file_size = 44,
                    .channel = _channel,
                    .sample_rate = _sample_rate,
                    .sample_bit = format_to_sample_bit(_format),
                    .bitrate = _channel * _sample_rate * format_to_sample_bit(_format) / 8,
                    .data_size = 0,
                };

                uint8_t buffer[44];
                if (0 != _create_wav_header(&header, buffer, sizeof(buffer))) {
                    err::check_raise(err::ERR_RUNTIME, "create wav failed!");
                }

                if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), _file)) {
                    err::check_raise(err::ERR_RUNTIME, "write wav header failed!");
                }
            }
        }

        if (record_ms > 0) {
            uint64_t start_ms = time::time_ms();
            if (_path.size() <= 0) {
                log::error("If you pass in the record_ms parameter, you must also set the correct path in audio::Audio()\r\n");
                return new Bytes();
            }

            while (time::time_ms() - start_ms <= (uint64_t)record_ms) {
                len = 0;
                while (len >= 0) {
                    len = _alsa_capture_pop(handle, format, channel, _period_size, buffer, buffer_size);
                    if (len > 0) {
                        if (_file)
                            fwrite(buffer, 1, len, _file);
                    }
                }
                time::sleep_ms(10);
            }

            return new Bytes();
        } else {
            int add_len = 4096;
            int valid_len = 0;
            int remain_len = add_len;
            std::vector<uint8_t> data(add_len);

            while (len >= 0) {
                len = _alsa_capture_pop(handle, format, channel, _period_size, buffer, buffer_size);
                if (len > 0) {
                    if (remain_len < len) {
                        data.resize(data.size() + add_len);
                        remain_len = remain_len + add_len;
                    }

                    if (remain_len >= len) {
                        memcpy(&data[valid_len], buffer, len);
                        remain_len -= len;
                        valid_len += len;
                    }

                    if (_file)
                        fwrite(buffer, len, 1, _file);
                }
            }

            if (valid_len > 0) {
                return new Bytes(&data[0], valid_len, true, true);
            }
            return new Bytes();
        }

        return new Bytes();
    }

    err::Err Recorder::finish() {
        if (_file) {
            if (fs::splitext(_path) == ".wav") {
                int file_size = ftell(_file);
                int pcm_size = file_size - 44;
                char buffer[4];
                buffer[0] = (uint8_t)((file_size - 8) & 0xff);
                buffer[1] = (uint8_t)(((file_size - 8) >> 8) & 0xff);
                buffer[2] = (uint8_t)(((file_size - 8) >> 16) & 0xff);
                buffer[3] = (uint8_t)(((file_size - 8) >> 24) & 0xff);

                fseek(_file, 4, SEEK_SET);
                if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), _file)) {
                    err::check_raise(err::ERR_RUNTIME, "write wav file size failed!");
                }

                buffer[0] = (uint8_t)((pcm_size) & 0xff);
                buffer[1] = (uint8_t)(((pcm_size) >> 8) & 0xff);
                buffer[2] = (uint8_t)(((pcm_size) >> 16) & 0xff);
                buffer[3] = (uint8_t)(((pcm_size) >> 24) & 0xff);
                fseek(_file, 40, SEEK_SET);
                if (sizeof(buffer) != fwrite(buffer, 1, sizeof(buffer), _file)) {
                    err::check_raise(err::ERR_RUNTIME, "write wav data size failed!");
                }
            }

            fflush(_file);
            fclose(_file);
            _file = NULL;
        }

        return err::ERR_NONE;
    }
#if CONFIG_BUILD_WITH_MAIXPY
    maix::Bytes *Player::NoneBytes = new maix::Bytes();
#else
    maix::Bytes *Player::NoneBytes = NULL;
#endif
    Player::Player(std::string path, int sample_rate, audio::Format format, int channel) {
        _path = path;
        _sample_rate = sample_rate;
        _format = format;
        _channel = channel;
        _period_size = 0;
        _file = NULL;

        if (fs::splitext(_path) != ".wav"
            && fs::splitext(_path) != ".pcm") {
            err::check_raise(err::ERR_RUNTIME, "Only files with the `.pcm` and `.wav` extensions are supported.");
        }

        if (_file == NULL && _path.size() > 0) {
            _file = fopen(_path.c_str(), "rb+");
            err::check_null_raise(_file, "Open file failed!");

            if (fs::splitext(_path) == ".wav") {
                uint8_t buffer[44];
                wav_header_t header = {0};
                if (sizeof(buffer) != fread(buffer, 1, sizeof(buffer), _file)) {
                    err::check_raise(err::ERR_RUNTIME, "read wav header failed!");
                }

                if (0 != _read_wav_header(&header, buffer, sizeof(buffer))) {
                    err::check_raise(err::ERR_RUNTIME, "parse wav header failed!");
                }

                _sample_rate = header.sample_rate;
                _channel = header.channel;
                _format = wav_sample_bit_to_format(header.sample_bit);
                // printf("sample rate:%d channel:%d format:%d\r\n", _sample_rate, _channel, _format);
            }
        }

        // alsa init
        snd_pcm_t *handle = NULL;
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_format_t format_p = _alsa_format_from_maix(format);
        _fix_segmentation_fault_bug();
        err::check_bool_raise(0 <= _alsa_player_init(&handle, &hwparams, &_period_size, format_p, sample_rate, channel), "capture init failed");
        _handle = handle;
        snd_pcm_uframes_t buffer_size = 0;
        _buffer = _alsa_prepare_buffer(format_p, channel, _period_size, &buffer_size);
        err::check_null_raise(_buffer, "player buffer init failed!");
        _buffer_size = (size_t)buffer_size;
    }

    Player::~Player() {
        if (_handle) {
            snd_pcm_drain((snd_pcm_t *)_handle);
            err::check_bool_raise(0 <= _alsa_player_deinit((snd_pcm_t *)_handle));
            _handle = NULL;
        }

        if (_file) {
            fclose(_file);
            _file = NULL;
        }

        if (_buffer) {
            free(_buffer);
            _buffer_size = 0;
        }
    }

    int Player::volume(int value) {
        err::check_raise(err::ERR_NOT_IMPL, "Not support now\r\n");
        return -1;
    }

    err::Err Player::play(maix::Bytes *data) {
        err::Err ret = err::ERR_NONE;
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;

        if (!data || !data->data) {
            if (_file == NULL && _path.size() > 0) {
                _file = fopen(_path.c_str(), "rb+");
                err::check_null_raise(_file, "Open file failed!");
            }

            if (fs::splitext(_path) == ".wav") {
                fseek(_file, 44, SEEK_SET);
            }

            int read_len = 0;
            while ((read_len = fread(buffer, 1, buffer_size, _file)) > 0) {
                len = _alsa_player_push(handle, format, channel, buffer, read_len);
                if (len < 0) {
                    log::error("play failed, %s\r\n", snd_strerror(len));
                    ret = err::ERR_RUNTIME;
                    break;
                } else if (len != read_len) {
                    log::error("play data length is incorrect, write %d bytes, returns %d bytes\r\n", read_len, len);
                    ret = err::ERR_RUNTIME;
                    break;
                }
            }
        } else {
            len = _alsa_player_push(handle, format, channel, data->data, data->data_len);
            if (len < 0) {
                log::error("play failed, %s\r\n", snd_strerror(len));
                ret = err::ERR_RUNTIME;
            } else if ((size_t)len != data->data_len) {
                log::error("play data length is incorrect, write %d bytes, returns %d bytes\r\n", data->data_len, len);
                ret = err::ERR_RUNTIME;
            }
        }

        return ret;
    }
} // namespace maix::audio