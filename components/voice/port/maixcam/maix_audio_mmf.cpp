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
    static int _alsa_capture_init(  snd_pcm_t **handle, snd_pcm_hw_params_t **hw_params,
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

    static int _alsa_get_frame_size(void)
    {
        return 512;
    }

    static void *_alsa_prepare_buffer(snd_pcm_format_t format, unsigned int channels, snd_pcm_uframes_t  *buffer_size)
    {
        int buffer_frames = _alsa_get_frame_size();
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

    static int _alsa_capture_pop(snd_pcm_t *handle, snd_pcm_format_t format, unsigned int channels, void *buffer, snd_pcm_uframes_t buffer_size)
    {
        int len = 0;
        int frame_size = _alsa_get_frame_size();
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

    static int _alsa_player_init(  snd_pcm_t **handle, snd_pcm_hw_params_t **hw_params,
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
            _alsa_capture_init(&handle, &hwparams, SND_PCM_FORMAT_S16_LE, 48000, 1);
            _alsa_capture_pop(handle, SND_PCM_FORMAT_S16_LE, 1, buffer, sizeof(buffer));
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

        // alsa init
        snd_pcm_t *handle = NULL;
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_format_t format_p = _alsa_format_from_maix(format);
        _fix_segmentation_fault_bug();
        err::check_bool_raise(0 <= _alsa_capture_init(&handle, &hwparams, format_p, sample_rate, channel), "capture init failed");
        _handle = handle;
        snd_pcm_uframes_t buffer_size = 0;
        _buffer = _alsa_prepare_buffer(format_p, channel, &buffer_size);
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
                    len = _alsa_capture_pop(handle, format, channel, buffer, buffer_size);
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
                len = _alsa_capture_pop(handle, format, channel, buffer, buffer_size);
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
            fclose(_file);
            _file = NULL;
        }

        return err::ERR_NONE;
    }

    maix::Bytes *Player::NoneBytes = new maix::Bytes();

    Player::Player(std::string path, int sample_rate, audio::Format format, int channel) {
        _path = path;
        _sample_rate = sample_rate;
        _format = format;
        _channel = channel;

        // alsa init
        snd_pcm_t *handle = NULL;
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_format_t format_p = _alsa_format_from_maix(format);
        _fix_segmentation_fault_bug();
        err::check_bool_raise(0 <= _alsa_player_init(&handle, &hwparams, format_p, sample_rate, channel), "capture init failed");
        _handle = handle;
        snd_pcm_uframes_t buffer_size = 0;
        _buffer = _alsa_prepare_buffer(format_p, channel, &buffer_size);
        err::check_null_raise(_buffer, "player buffer init failed!");
        _buffer_size = (size_t)buffer_size;

        // file init
        _file = NULL;
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