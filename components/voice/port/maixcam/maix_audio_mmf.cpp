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

    static int _alsa_capture_init(  snd_pcm_t **handle, snd_pcm_hw_params_t **hw_params, int *period_size,
                                    snd_pcm_format_t format, unsigned int sample_rate, unsigned int channels, snd_pcm_uframes_t buffer_size)
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

        if ((err = snd_pcm_hw_params_set_buffer_size_near(*handle, *hw_params, &buffer_size)) < 0) {
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
            return -1;
        }

        if ((len = (int)snd_pcm_readi(handle, buffer, frame_size)) < 0) {
            if (len == -EPIPE) {
                snd_pcm_prepare(handle);
            }
            return len;
        }

        return len * frame_byte * channels;
    }

    static int _alsa_player_init(  snd_pcm_t **handle, snd_pcm_hw_params_t **hw_params, int *period_size,
                                    snd_pcm_format_t format, unsigned int sample_rate, unsigned int channels, snd_pcm_uframes_t buffer_size)
    {
        int err = 0;

        if (handle) *handle = NULL;
        if (hw_params) *hw_params = NULL;

        if ((err = snd_pcm_open(handle, "hw:1,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
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

        if ((err = snd_pcm_hw_params_set_buffer_size_near(*handle, *hw_params, &buffer_size)) < 0) {
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
        int ret = 0;
        int retry_count = 2;
_retry:
        if ((ret = snd_pcm_wait(handle, 5000)) < 0) {
            // printf(" alsa pcm wait timeout! retry_count:%d ret:%d(%s)\r\n", retry_count, ret, snd_strerror(errno));
            snd_pcm_prepare(handle);
            if (retry_count > 0) {
                retry_count--;
                goto _retry;
            }
            return 0;
        }

        int remain_len = write_count;
        uint64_t timeout_ms = 1000;
        uint64_t last_ms = time::ticks_ms();
        uint8_t *buffer2 = (uint8_t *)buffer;
        while (remain_len > 0) {
            // log::info("======[%s][%d] total:%d remain_len:%d idle:%d", __func__, __LINE__, write_count, remain_len, snd_pcm_avail_update(handle));
            if ((len = (int)snd_pcm_writei(handle, buffer2, remain_len)) < 0) {
                snd_pcm_prepare(handle);
                return len;
            }

            remain_len -= len;
            buffer2 += len * frame_byte * channels;

            if (len > 0) {
                last_ms = time::ticks_ms();
            }

            if (time::ticks_ms() - last_ms > timeout_ms) {
                log::warn("write pcm timeout! write:%d remain %d", len * frame_byte * channels, remain_len * frame_byte * channels);
                return (len - remain_len) * frame_byte * channels;
            }
        }

        return len * frame_byte * channels;
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

        if (path.size() > 0) {
            if (fs::splitext(_path)[1] != ".wav"
                && fs::splitext(_path)[1] != ".pcm") {
                err::check_raise(err::ERR_RUNTIME, "Only files with the `.pcm` and `.wav` extensions are supported.");
            }
        }

        // alsa init
        snd_pcm_t *handle = NULL;
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_format_t format_p = _alsa_format_from_maix(format);
        err::check_bool_raise(0 <= _alsa_capture_init(&handle, &hwparams, &_period_size, format_p, sample_rate, channel, 24000), "capture init failed");
        _handle = handle;
        snd_pcm_uframes_t buffer_size = 0;
        _buffer = _alsa_prepare_buffer(format_p, channel, _period_size, &buffer_size);
        err::check_null_raise(_buffer, "record buffer init failed!");
        _buffer_size = (size_t)buffer_size;

        char buffer[512];
        snprintf(buffer, sizeof(buffer), "amixer -Dhw:0 cset name='ADC Capture Volume' %d &> /dev/zero", 12);
        system(buffer);
        snprintf(buffer, sizeof(buffer), "amixer sset 'ADC Capture Mute' off &> /dev/zero");
        system(buffer);

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

    bool Recorder::mute(int data) {
        char buffer[512];
        bool is_muted = false;
        if (data < 0) {
            // get
            FILE *fp;
            fp = popen("amixer sget 'ADC Capture Mute'", "r");
            if (fp == NULL) {
                return -1;
            }

            char temp[30];
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                if (strstr(buffer, "Front Left: Playback")) {
                    sscanf(buffer, "  Front Left: Playback %s", temp);
                    break;
                }
            }

            if (!strncmp(temp, "[off]", sizeof(temp))) {
                is_muted = false;
            } else {
                is_muted = true;
            }
            pclose(fp);
        } else if (data == 0) {
            snprintf(buffer, sizeof(buffer), "amixer sset 'ADC Capture Mute' off &> /dev/zero");
            system(buffer);
            is_muted = false;
        } else {
            snprintf(buffer, sizeof(buffer), "amixer sset 'ADC Capture Mute' on &> /dev/zero");
            system(buffer);
            is_muted = true;
        }
        return is_muted;
    }

    int Recorder::volume(int value) {
        char buffer[512];
        value = value > 100 ? 100 : value;

        // set
        if (value != -1) {
            value = (double)value * 24 / 100;
            snprintf(buffer, sizeof(buffer), "amixer -Dhw:0 cset name='ADC Capture Volume' %d &> /dev/zero", value);
            system(buffer);
        }

        // get
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

        read_value = read_value * 100 / 24;

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

    void Recorder::reset(bool start) {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        snd_pcm_prepare(handle);
        if (start) {
            snd_pcm_start(handle);
        }
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

            if (fs::splitext(_path)[1] == ".wav") {
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

        auto state = snd_pcm_state(handle);
        if (state != SND_PCM_STATE_RUNNING) {
            snd_pcm_start(handle);
        }

        if (record_ms > 0) {
            auto bytes_per_frame = this->frame_size();
            auto period_size = bytes_per_frame * _period_size;
            auto read_remain_size = record_ms * _sample_rate * bytes_per_frame / 1000;
            auto period_ms = period_size * 1000 / _sample_rate / bytes_per_frame;

            if (read_remain_size < period_size) {
                auto err_msg = "the record time must be greater than " + std::to_string(period_ms) + " ms";
                err::check_raise(err::ERR_ARGS, err_msg);
            }

            auto out_bytes = new Bytes(nullptr, read_remain_size);
            err::check_null_raise(out_bytes, "Create new bytes failed!");
            while (read_remain_size > 0 && !app::need_exit()) {
                if (this->remain_size() < period_size) {
                    time::sleep_ms(1);
                    continue;
                }

                auto buffer_size = read_remain_size;
                auto buffer = out_bytes->data + out_bytes->data_len - read_remain_size;

                auto len = (int)snd_pcm_readi(handle, buffer, buffer_size / bytes_per_frame);
                if (len == -EPIPE) {
                    snd_pcm_prepare(handle);
                } else if (len < 0) {
                    log::error("pcm read error: %s", snd_strerror(len));
                    return new Bytes();
                } else {
                    read_remain_size -= len * bytes_per_frame;
                }
                // log::info("pcm read total %d, remain %d len %d", out_bytes->data_len, read_remain_size, len * bytes_per_frame);
            }

            if (_file)
                fwrite(out_bytes->data, 1, out_bytes->data_len, _file);

            return out_bytes;
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

    int Recorder::frame_size() {
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        int format_width = snd_pcm_format_width(format) / 8;
        return format_width * _channel;
    }

    int Recorder::remain_size() {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        return snd_pcm_avail_update(handle);
    }

    maix::Bytes *Recorder::record_bytes(int record_size) {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;

        if (_file == NULL && _path.size() > 0) {
            _file = fopen(_path.c_str(), "w+");
            err::check_null_raise(_file, "Open file failed!");

            if (fs::splitext(_path)[1] == ".wav") {
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

        {
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

                    if (valid_len >= record_size) {
                        break;
                    }
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
            if (fs::splitext(_path)[1] == ".wav") {
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

        if (path.size() > 0) {
            if (fs::splitext(_path)[1] != ".wav"
                && fs::splitext(_path)[1] != ".pcm") {
                err::check_raise(err::ERR_RUNTIME, "Only files with the `.pcm` and `.wav` extensions are supported.");
            }
        }

        if (_file == NULL && _path.size() > 0) {
            _file = fopen(_path.c_str(), "rb+");
            err::check_null_raise(_file, "Open file failed!");

            if (fs::splitext(_path)[1] == ".wav") {
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
        err::check_bool_raise(0 <= _alsa_player_init(&handle, &hwparams, &_period_size, format_p, sample_rate, channel, 24000), "capture init failed");
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
        char buffer[512];
        value = value > 100 ? 100 : value;
        value = value < 0 ? -1 : value;
        // set
        if (value != -1) {
            value = 100 - value;
            snprintf(buffer, sizeof(buffer), "amixer -c 1 set 'DAC' %d%% &> /dev/zero", value);
            system(buffer);
        }

        // get
        FILE *fp;
        fp = popen("amixer -c 1 get 'DAC'", "r");
        if (fp == NULL) {
            return -1;
        }

        int read_value = -1;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "  Front Right: Playback")) {
                sscanf(buffer, "  Front Right: Playback %*d [%d%%]", &read_value);
            }
        }
        pclose(fp);

        read_value = ((100 - read_value)) * 2;

        return read_value;
    }

    err::Err Player::play(maix::Bytes *data) {
        err::Err ret = err::ERR_NONE;
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;

        if (!data || !data->data || !data->size()) {
            if (_file == NULL && _path.size() > 0) {
                _file = fopen(_path.c_str(), "rb+");
                err::check_null_raise(_file, "Open file failed!");
            }

            if (fs::splitext(_path)[1] == ".wav") {
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

    int Player::frame_size() {
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        int format_width = snd_pcm_format_width(format) / 8;
        return format_width * _channel;
    }
} // namespace maix::audio