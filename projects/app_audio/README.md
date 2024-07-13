### 录音机软件（含播放）
#### 功能介绍

该软件主要包含录音和播放两大功能。
（1）进入APP后，会将已经存在的wav文件加载以列表的方式进行显示，列表的排序方式按照文件的时间先后顺序排列，点击文件后会弹出窗口选择是播放该文件还是删除该文件，选择播放文件将会跳转到音频播放界面，删除文件按钮将会将文件删除并从列表移除该文件。
（2）录音上能够控制录音的暂停与继续、开始与结束，并在录音的过程中可以调整录音的音量大小,录音保存的文件为开始录音的时间，音频格式为wav文件并保存在/maixapp/share/wav文件夹下面（如果没有该目录会自动创建目录）。
（3）播放上能够控制音频的暂停和继续、播放的退出与重复播放，并且在播放过程中可以调整播放的音量大小，在播放过程中播放的进度条会显示播放进度。
（4）对于音量的修改是记忆的，如果你上次进入app修改的音量值，这次进入会保持上次进入的音量值，点击喇叭会显示音量调节滑块，再次点击即不显示。

app地址：https://maixhub.com/app/38
#### 需要使用的话的注意事项
如果您需要在我基础上简单修改的话，需要把maixcdk的音频这个库贴我下面的，再编译，活着您修改maixcdk，添加我下面的一些函数和参数修改，因为为了结合进度条播放，我需要阻塞方式的播放，就修改了缓冲区大小和播放方式，以及加入暂停和停止

maix_audio_mmf.cpp文件
```
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
#include <atomic>
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
    static int _alsa_capture_init(  snd_pcm_t **handle, snd_pcm_hw_params_t **hw_params,
                                    snd_pcm_format_t format, unsigned int sample_rate, unsigned int channels)
    {
        int err = 0;
        ///
        // snd_pcm_uframes_t frames_set = 1024; // 设定缓冲区大小为 1024 帧
        if (handle) *handle = NULL;
        if (hw_params) *hw_params = NULL;
        if ((err = snd_pcm_open(handle, "hw:0,0", SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0) {
        // if ((err = snd_pcm_open(handle, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        // if ((err = snd_pcm_open(handle, "hw:1,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
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
        // //new
        // if ((err = snd_pcm_hw_params_set_buffer_size_near(*handle, *hw_params, &frames_set)) < 0) {
        //     printf("Can't set frames  (%s)\n", snd_strerror(err));
        //     goto _exit;
        // }
        //
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
        ///
        snd_pcm_uframes_t frames_set = 1024; // 设定缓冲区大小为 1024 帧
        ////
        if (handle) *handle = NULL;
        if (hw_params) *hw_params = NULL;
        // if ((err = snd_pcm_open(handle, "hw:1,0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0) 
        if ((err = snd_pcm_open(handle, "hw:1,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
        {
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
        //new
        if ((err = snd_pcm_hw_params_set_buffer_size_near(*handle, *hw_params, &frames_set)) < 0) {
            printf("Can't set frames  (%s)\n", snd_strerror(err));
            goto _exit;
        }
        //
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
        // int frame_size = _alsa_get_frame_size();
        int frame_byte = snd_pcm_format_width(format) / 8;
        // int write_count = buffer_size / frame_byte / channels;
        int write_count = buffer_size  / frame_byte / channels;
        if (snd_pcm_wait(handle, 1000) < 0) {
            printf("alsa pcm wait timeout!\r\n");
            return 0;
        }
        if ((len = (int)snd_pcm_writei(handle, buffer, write_count)) < 0) 
        {
            snd_pcm_prepare(handle);
            return len;
            // return len * frame_byte * channels;
        }
        return len * frame_byte * channels;
    }
    // static int _alsa_player_push(snd_pcm_t *handle, snd_pcm_format_t format, unsigned int channels, void *buffer, snd_pcm_uframes_t buffer_size)
    // {
    //     int len = 0;
    //     int frame_byte = snd_pcm_format_width(format) / 8;
    //     int write_count = buffer_size / frame_byte / channels;
    //     if (snd_pcm_wait(handle, 1000) < 0) {
    //         printf("alsa pcm wait timeout!\r\n");
    //         return 0;
    //     }
    //     if ((len = (int)snd_pcm_writei(handle, buffer, write_count)) < 0) {
    //         return len;
    //     }
    //     return len * frame_byte * channels;
    // }
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
        // value = value < 0 ? 0 : value;
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
    maix::Bytes *Recorder::record_pcm_to_file(std::string path_record_pcm_file,int record_ms) 
    {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;
        FILE *record_pcm_file=fopen(path_record_pcm_file.c_str(), "w+");
        if (record_ms > 0) 
        {
            snd_pcm_drop(handle);
            // 重新初始化 PCM 设备
            snd_pcm_prepare(handle);
            uint64_t start_ms = time::time_ms();
            while (time::time_ms() - start_ms <= (uint64_t)record_ms) 
            {
                len = 0;
                while (len >= 0) {
                    len = _alsa_capture_pop(handle, format, channel, buffer, buffer_size);
                    if (len > 0) {
                        if (record_pcm_file)
                            fwrite(buffer, 1, len, record_pcm_file);
                    }
                }
                time::sleep_ms(10);
            }
            fclose(record_pcm_file);
            return new Bytes();
        } 
        fclose(record_pcm_file);
        return new Bytes();
    }
    maix::Bytes *Recorder::record_pcm_to_file_keep(std::string path_record_pcm_file,std::atomic<bool>& flag_record) 
    {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;
        FILE *record_pcm_file=fopen(path_record_pcm_file.c_str(), "w+");
        if (flag_record.load()) 
        {
            // snd_pcm_drop(handle);
            // 重新初始化 PCM 设备
            snd_pcm_prepare(handle);
            while (flag_record.load()) 
            {
                len = 0;
                while (len >= 0) {
                    len = _alsa_capture_pop(handle, format, channel, buffer, buffer_size);
                    if (len > 0) {
                        if (record_pcm_file)
                        {
                            fwrite(buffer, 1, len, record_pcm_file);
                        }
                    }
                }
                time::sleep_ms(10);
            }
            snd_pcm_drop(handle);
            fclose(record_pcm_file);
            return new Bytes();
        } 
        fclose(record_pcm_file);
        
        return new Bytes();
    }
    maix::Bytes *Recorder::record_pcm_to_file_keep_pause(std::string path_record_pcm_file,std::atomic<bool>& flag_record,std::atomic<bool>& flag_record_pause) 
    {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;
        FILE *record_pcm_file=fopen(path_record_pcm_file.c_str(), "w+");
        wav_header_t header = {
                    .file_size = 44,
                    .channel = _channel,
                    .sample_rate = _sample_rate,
                    .sample_bit = format_to_sample_bit(_format),
                    .bitrate = _channel * _sample_rate * format_to_sample_bit(_format) / 8,
                    .data_size = 0,
        }; 
        uint8_t buffer_wav[44];
        if (0 != _create_wav_header(&header, buffer_wav, sizeof(buffer_wav))) {
            err::check_raise(err::ERR_RUNTIME, "create wav failed!");
        }
        if (sizeof(buffer_wav) != fwrite(buffer_wav, 1, sizeof(buffer_wav), record_pcm_file)) {
            err::check_raise(err::ERR_RUNTIME, "write wav header failed!");
        }
        if (flag_record.load()) 
        {
            snd_pcm_prepare(handle);
            while (flag_record.load()) 
            {
                if(flag_record_pause.load())
                {
                    snd_pcm_drop(handle);
                    while (flag_record_pause.load())
                    {
                        if (!flag_record.load()) 
                        {
                            break;
                        }
                    }
                    snd_pcm_prepare(handle);
                }
                len = 0;
                while (len >= 0) {
                    len = _alsa_capture_pop(handle, format, channel, buffer, buffer_size);
                    if (len > 0) {
                        if (record_pcm_file)
                        {
                            fwrite(buffer, 1, len, record_pcm_file);
                        }
                    }
                }
                time::sleep_ms(10);
            }
            snd_pcm_drop(handle);
            int file_size = ftell(record_pcm_file);
            int pcm_size = file_size - 44;
            char buffer_set[4];
            buffer_set[0] = (uint8_t)((file_size - 8) & 0xff);
            buffer_set[1] = (uint8_t)(((file_size - 8) >> 8) & 0xff);
            buffer_set[2] = (uint8_t)(((file_size - 8) >> 16) & 0xff);
            buffer_set[3] = (uint8_t)(((file_size - 8) >> 24) & 0xff);
            fseek(record_pcm_file, 4, SEEK_SET);
            if (sizeof(buffer_set) != fwrite(buffer_set, 1, sizeof(buffer_set), record_pcm_file)) {
                err::check_raise(err::ERR_RUNTIME, "write wav file size failed!");
            }
            buffer_set[0] = (uint8_t)((pcm_size) & 0xff);
            buffer_set[1] = (uint8_t)(((pcm_size) >> 8) & 0xff);
            buffer_set[2] = (uint8_t)(((pcm_size) >> 16) & 0xff);
            buffer_set[3] = (uint8_t)(((pcm_size) >> 24) & 0xff);
            fseek(record_pcm_file, 40, SEEK_SET);
            if (sizeof(buffer_set) != fwrite(buffer_set, 1, sizeof(buffer_set), record_pcm_file)) {
                err::check_raise(err::ERR_RUNTIME, "write wav data size failed!");
            }
            fflush(record_pcm_file);
            fclose(record_pcm_file);
            return new Bytes();
        } 
        fclose(record_pcm_file);
        
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
    Player::Player(std::string path, int sample_rate, audio::Format format, int channel) 
    {
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
    err::Err Player::finish() {
        if (_file) {
            fclose(_file);
            _file = NULL;
        }
        return err::ERR_NONE;
    }
    int Player::volume(int value) {
        char buffer[512];
        // value = value < 0 ? 0 : value;
        if (value != -1) {
            snprintf(buffer, sizeof(buffer), "amixer -Dhw:1 cset name='DAC Playback Volume' %d &> /dev/zero",value);
            system(buffer);
        }
        FILE *fp;
        fp = popen("amixer -Dhw:1 cget name='DAC Playback Volume'", "r");
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
    err::Err Player::play_pcm_file(std::string path_pcm) {
        err::Err ret = err::ERR_NONE;
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size=512;//256*format/8
        // snd_pcm_uframes_t buffer_size = (snd_pcm_uframes_t)_buffer_size;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;
        FILE * debug_wxw=fopen("/root/debug_test.txt", "a+");
        FILE * file_pcm_file=fopen(path_pcm.c_str(), "rb+");
        // snd_pcm_drain(handle);    // 确保上一次播放已完成
        snd_pcm_drop(handle);
        snd_pcm_prepare(handle);  // 准备设备
        int read_len = 0;
        while ((read_len = fread(buffer, 1, buffer_size, file_pcm_file)) > 0) {
            len = _alsa_player_push(handle, format, channel, buffer, read_len);
            fprintf(debug_wxw,"read size %d bytes, write size %d bytes\r\n",read_len, len);
            if (len < 0) {
                log::error("play failed, %s\r\n", snd_strerror(len));
                fprintf(debug_wxw,"play failed, %s\r\n",snd_strerror(len));
                // snd_pcm_prepare(handle);
                ret = err::ERR_RUNTIME;
                break;
            } 
            else if (len != read_len) {
                log::error("play data length is incorrect, write %d bytes, \r\n", read_len, len);
                fprintf(debug_wxw,"len != read_len size buff %d bytes, size buffsize %d bytes\r\n",sizeof(buffer), buffer_size);
                fprintf(debug_wxw,"len != read_len play data length is incorrect, write %d bytes, returns %d bytes\r\n",read_len, len);
                // snd_pcm_prepare(handle);
                ret = err::ERR_RUNTIME;
                break;
            }
            // time::sleep_ms(10);
        }
        snd_pcm_drain(handle);    // 确保上一次播放已完成
        fclose(debug_wxw);
        fclose(file_pcm_file);
        
        return ret;
    }
    err::Err Player::play_pcm_file_flag_bool(std::string path_pcm,bool& flag_play,bool&  flag_pause) 
    {
        err::Err ret = err::ERR_NONE;
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size=512;//256*format/8
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;
        FILE * file_pcm_file=fopen(path_pcm.c_str(), "rb+");
        FILE * debug_wxw=fopen("/root/debug_test.txt", "w+");
        // snd_pcm_drain(handle);    // 确保上一次播放已完成
        
        snd_pcm_prepare(handle);  // 准备设备
        int read_len = 0;
        while ((read_len = fread(buffer, 1, buffer_size, file_pcm_file)) > 0&&flag_play) 
        {
            snd_pcm_sframes_t avail;
            avail = snd_pcm_avail_update(handle);
            fprintf(debug_wxw," 当前写入缓冲区帧数：%ld\r\n",avail);
            len = _alsa_player_push(handle, format, channel, buffer, read_len);
            if (len < 0) {
                log::error("play failed, %s\r\n", snd_strerror(len));
                fprintf(debug_wxw,"play failed, %s\r\n",snd_strerror(len));
                ret = err::ERR_RUNTIME;
                break;
            } 
            else if (len != read_len) {
                log::error("play data length is incorrect, write %d bytes, \r\n", read_len, len);
                fprintf(debug_wxw,"len != read_len size buff %d bytes, size buffsize %d bytes\r\n",sizeof(buffer), buffer_size);
                fprintf(debug_wxw,"len != read_len play data length is incorrect, write %d bytes, returns %d bytes\r\n",read_len, len);
                ret = err::ERR_RUNTIME;
                break;
            }
            // if(!flag_play)
            // {
            //     snd_pcm_drop(handle);
            //     break;
            // }
            if(flag_pause)
            {
                fprintf(debug_wxw," state1 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                // snd_pcm_drain(handle); 
                snd_pcm_pause(handle, 1);
                fprintf(debug_wxw," state2 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                // snd_pcm_drop(handle);
                while (flag_pause) 
                {
                    
                    // snd_pcm_prepare(handle);  // 准备设备
                    if(!flag_play)
                    {
                        // snd_pcm_drop(handle);
                        break;
                    }
                }
                fprintf(debug_wxw," state3 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
                int err1 = snd_pcm_pause(handle, 0);
                fprintf(debug_wxw," state4 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
                // int err1 = snd_pcm_prepare(handle);
                // time::sleep_ms(10);
                // snd_pcm_start(handle);
                fprintf(debug_wxw," eee %d\r\n",err1);
                fprintf(debug_wxw," www %s\r\n",snd_strerror(err1));
                // snd_pcm_drain(handle); 
                // snd_pcm_prepare(handle);
                // snd_pcm_resume(handle);
                // snd_pcm_pause(handle, 0);
                // snd_pcm_prepare(handle);
                
            }  
        }
        snd_pcm_drop(handle);
        // snd_pcm_drain(handle);    // 确保上一次播放已完成
        fclose(debug_wxw);
        fclose(file_pcm_file);
        
        return ret;
    }
    err::Err Player::play_pcm_file_flag(std::string path_pcm,std::atomic<bool>& flag_play,std::atomic<bool>& flag_pause) 
    {
        err::Err ret = err::ERR_NONE;
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        void *buffer = _buffer;
        snd_pcm_uframes_t buffer_size=512;//256*format/8
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        int len = 0;
        FILE * file_pcm_file=fopen(path_pcm.c_str(), "rb+");
        FILE * debug_wxw=fopen("/root/debug_test.txt", "w+");
        // snd_pcm_drain(handle);    // 确保上一次播放已完成
        
        snd_pcm_prepare(handle);  // 准备设备
        int read_len = 0;
        // while (flag_play.load())
        // {
        //     if (!flag_play.load()) 
        //     {
        //         fprintf(debug_wxw," file over %s\r\n","1");
        //         snd_pcm_drop(handle);
        //         break;
        //     }
        //     read_len = fread(buffer, 1, buffer_size, file_pcm_file);
        //     if (read_len > 0) 
        //     {
        //         // 处理音频数据
        //         len = _alsa_player_push(handle, format, channel, buffer, read_len);
        //         if (len < 0) {
        //             // 处理错误情况
        //             log::error("play failed, %s\r\n", snd_strerror(len));
        //             fprintf(debug_wxw,"play failed, %s\r\n",snd_strerror(len));
        //             ret = err::ERR_RUNTIME;
        //             break;
        //         } 
        //         else if (len != read_len) {
        //             // 处理数据长度不正确的情况
        //             log::error("play data length is incorrect, write %d bytes, \r\n", read_len, len);
        //             fprintf(debug_wxw,"len != read_len size buff %d bytes, size buffsize %d bytes\r\n",sizeof(buffer), buffer_size);
        //             fprintf(debug_wxw,"len != read_len play data length is incorrect, write %d bytes, returns %d bytes\r\n",read_len, len);
        //             ret = err::ERR_RUNTIME;
        //             break;
        //         }
        //     }
        //     else if (read_len == 0) 
        //     {
        //         fprintf(debug_wxw," file over %s\r\n","2");
        //         // 文件已经读取完毕，退出循环
        //         break;
        //     } 
        //     if(flag_pause.load())
        //     {
        //         fprintf(debug_wxw," state1 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
        //         // snd_pcm_drain(handle); 
        //         snd_pcm_pause(handle, 1);
        //         fprintf(debug_wxw," state2 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
        //         // snd_pcm_drop(handle);
        //         while (flag_pause.load()) 
        //         {
                    
        //             // snd_pcm_prepare(handle);  // 准备设备
        //             if(!flag_play.load())
        //             {
        //                 snd_pcm_drop(handle);
        //                 break;
        //             }
        //         }
        //         fprintf(debug_wxw," state3 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
        //         int err1 = snd_pcm_pause(handle, 0);
        //         fprintf(debug_wxw," state4 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
        //         // int err1 = snd_pcm_prepare(handle);
        //         // time::sleep_ms(10);
        //         // snd_pcm_start(handle);
        //         fprintf(debug_wxw," eee %d\r\n",err1);
        //         fprintf(debug_wxw," www %s\r\n",snd_strerror(err1));
        //         // snd_pcm_drain(handle); 
        //         // snd_pcm_prepare(handle);
        //         // snd_pcm_resume(handle);
        //         // snd_pcm_pause(handle, 0);
        //         // snd_pcm_prepare(handle);
                
        //     }   
        // }
        
        
        while ((read_len = fread(buffer, 1, buffer_size, file_pcm_file)) > 0&&flag_play.load()) 
        {
            snd_pcm_sframes_t avail;
            avail = snd_pcm_avail_update(handle);
            fprintf(debug_wxw," 当前写入缓冲区帧数：%ld\r\n",avail);
            if(!flag_play.load())
            {
                snd_pcm_drop(handle);
                break;
            }
            if(flag_pause.load())
            {
                fprintf(debug_wxw," state1 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                // snd_pcm_drain(handle); 
                snd_pcm_pause(handle, 1);
                fprintf(debug_wxw," state2 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                // snd_pcm_drop(handle);
                while (flag_pause.load()) 
                {
                    fprintf(debug_wxw," wwww \r\n");
                    // snd_pcm_prepare(handle);  // 准备设备
                    if(!flag_play.load())
                    {
                        snd_pcm_drop(handle);
                        break;
                    }
                }
                fprintf(debug_wxw," state3 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
                int err1 = snd_pcm_pause(handle, 0);
                fprintf(debug_wxw," state4 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
                // int err1 = snd_pcm_prepare(handle);
                // time::sleep_ms(10);
                // snd_pcm_start(handle);
                fprintf(debug_wxw," eee %d\r\n",err1);
                fprintf(debug_wxw," www %s\r\n",snd_strerror(err1));
                // snd_pcm_drain(handle); 
                // snd_pcm_prepare(handle);
                // snd_pcm_resume(handle);
                // snd_pcm_pause(handle, 0);
                // snd_pcm_prepare(handle);
                
            }  
            len = _alsa_player_push(handle, format, channel, buffer, read_len);
            if (len < 0) {
                log::error("play failed, %s\r\n", snd_strerror(len));
                fprintf(debug_wxw,"play failed, %s\r\n",snd_strerror(len));
                ret = err::ERR_RUNTIME;
                break;
            } 
            else if (len != read_len) {
                log::error("play data length is incorrect, write %d bytes, \r\n", read_len, len);
                fprintf(debug_wxw,"len != read_len size buff %d bytes, size buffsize %d bytes\r\n",sizeof(buffer), buffer_size);
                fprintf(debug_wxw,"len != read_len play data length is incorrect, write %d bytes, returns %d bytes\r\n",read_len, len);
                ret = err::ERR_RUNTIME;
                break;
            }
            if(!flag_play.load())
            {
                snd_pcm_drop(handle);
                break;
            }
            if(flag_pause.load())
            {
                fprintf(debug_wxw," state1 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                // snd_pcm_drain(handle); 
                snd_pcm_pause(handle, 1);
                fprintf(debug_wxw," state2 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                // snd_pcm_drop(handle);
                while (flag_pause.load()) 
                {
                    
                    // snd_pcm_prepare(handle);  // 准备设备
                    if(!flag_play.load())
                    {
                        snd_pcm_drop(handle);
                        break;
                    }
                }
                fprintf(debug_wxw," state3 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
                int err1 = snd_pcm_pause(handle, 0);
                fprintf(debug_wxw," state4 %s\r\n",snd_pcm_state_name(snd_pcm_state(handle)));
                
                // int err1 = snd_pcm_prepare(handle);
                // time::sleep_ms(10);
                // snd_pcm_start(handle);
                fprintf(debug_wxw," eee %d\r\n",err1);
                fprintf(debug_wxw," www %s\r\n",snd_strerror(err1));
                // snd_pcm_drain(handle); 
                // snd_pcm_prepare(handle);
                // snd_pcm_resume(handle);
                // snd_pcm_pause(handle, 0);
                // snd_pcm_prepare(handle);
                
            }  
        }
        snd_pcm_drop(handle);
        // snd_pcm_drain(handle);    // 确保上一次播放已完成
        fclose(debug_wxw);
        fclose(file_pcm_file);
        
        return ret;
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
    void Player::audio_prepare() 
    {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        snd_pcm_prepare(handle);  // 准备设备
    }
    void Player::audio_drain() 
    {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        snd_pcm_drain(handle);  // 准备设备
    }
    void Player::audio_drop() 
    {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        snd_pcm_drop(handle);  // 准备设备
    }
    int Player::audio_pause(int enable) 
    {
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        int ret=snd_pcm_pause(handle,enable);  // 准备设备
        return ret;
    }
    err::Err Player::play_data(maix::Bytes *data) 
    {
        err::Err ret = err::ERR_NONE;
        snd_pcm_t *handle = (snd_pcm_t *)_handle;
        snd_pcm_format_t format = _alsa_format_from_maix(_format);
        unsigned int channel = _channel;
        
        int len = 0;
        if (!data || !data->data) 
        {
            
        } 
        else 
        {
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
```

maix_audio.hpp文件

```
/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */
#pragma once
#include "maix_basic.hpp"
#include <memory>
/**
 * @brief maix.audio module
 * @maixpy maix.audio
*/
namespace maix::audio
{
    /**
     * Audio type
     * @maixpy maix.audio.Format
     */
    enum Format
    {
        FMT_NONE = 0,       // format invalid
        FMT_S8,             // unsigned 8 bits
        FMT_S16_LE,         // signed 16 bits, little endian
        FMT_S32_LE,         // signed 32 bits, little endian
        FMT_S16_BE,         // signed 16 bits, big endian
        FMT_S32_BE,         // signed 32 bits, big endian
        FMT_U8,             // unsigned 8 bits
        FMT_U16_LE,         // unsigned 16 bits, little endian
        FMT_U32_LE,         // unsigned 32 bits, little endian
        FMT_U16_BE,         // unsigned 16 bits, big endian
        FMT_U32_BE,         // unsigned 32 bits, big endian
    };
    /**
     * Recorder class
     * @maixpy maix.audio.Recorder
     */
    class Recorder
    {
        std::string _path;
        int _sample_rate;
        int _channel;
        audio::Format _format;
        void *_handle;
        void *_buffer;
        size_t _buffer_size;
        FILE *_file;
    public:
        /**
         * @brief Construct a new Recorder object
         * @param path record path. the path determines the location where you save the file, if path is none, the audio module will not save file.
         * @param sample_rate record sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format record sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel record sample channel, default is 1, means 1 channel sampling at the same time
         * @maixpy maix.audio.Recorder.__init__
         * @maixcdk maix.audio.Recorder.Recorder
         */
        Recorder(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1);
        ~Recorder();
        /**
         * Set/Get record volume
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.Recorder.volume
        */
        int volume(int value = -1);
        /**
         * Record, Read all cached data in buffer and return.
         * @param record_ms record time. unit: ms
         * @param sample_rate audio sample rate
         * @param format audio sample format
         * @param channel audio sample channel
         * @return pcm data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixpy maix.audio.Recorder.record
        */
        maix::Bytes *record(int record_ms = -1);
        /**
         * Record, Read all cached data in buffer and return.
         * @param path_record_pcm_file record pcm to file
         * @param record_ms record time. unit: ms
         * @return pcm data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixpy maix.audio.Recorder.record
        */
        maix::Bytes *record_pcm_to_file(std::string path_record_pcm_file,int record_ms=-1);
        /**
         * Record, Read all cached data in buffer and return.
         * @param path_record_pcm_file record pcm to file
         * @param flag_record record keep flag. 
         * @return pcm data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixpy maix.audio.Recorder.record
        */
        maix::Bytes * record_pcm_to_file_keep(std::string path_record_pcm_file,std::atomic<bool>& flag_record); 
        /**
         * Record, Read all cached data in buffer and return.
         * @param record_pcm_to_file_keep_pause record pcm to file
         * @param flag_record record keep flag. 
         * @return pcm data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixpy maix.audio.Recorder.record
        */
        maix::Bytes * record_pcm_to_file_keep_pause(std::string path_record_pcm_file,std::atomic<bool>& flag_record,std::atomic<bool>& flag_record_pause); 
        /**
         * Finish the record, if you have passed in the path, this api will save the audio data to file.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Recorder.finish
        */
        err::Err finish();
        /**
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.Recorder.sample_rate
         */
        int sample_rate() {
            return _sample_rate;
        }
        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.Recorder.format
         */
        audio::Format format() {
            return _format;
        }
        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.Recorder.channel
         */
        int channel() {
            return _channel;
        }
    };
    /**
     * Player class
     * @maixpy maix.audio.Player
     */
    class Player
    {
        std::string _path;
        int _sample_rate;
        int _channel;
        audio::Format _format;
        void *_handle;
        void *_buffer;
        size_t _buffer_size;
        FILE *_file;
    public:
        static maix::Bytes *NoneBytes;
        /**
         * @brief Construct a new Player object
         * @param path player path. the path determines the location where you save the file, if path is none, the audio module will not save file.
         * @param sample_rate player sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format player sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel player sample channel, default is 1, means 1 channel sampling at the same time
         * @maixpy maix.audio.Player.__init__
         * @maixcdk maix.audio.Player.Player
         */
        Player(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1);
        ~Player();
        /**
         * Set/Get player volume(Not support now)
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.Player.volume
        */
        int volume(int value = -1);
        /**
         * Set/Get player volume(Not support now)
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.Player.volume
        */
        void audio_drop();
        /**
         * Set/Get player volume(Not support now)
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.Player.volume
        */
        void audio_drain();
        /**
         * Set/Get player volume(Not support now)
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.Player.volume
        */
        void audio_prepare();
        /**
         * Set/Get player volume(Not support now)
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.Player.volume
        */
        int audio_pause(int enable=1);
        /**
         * Play_pcm_file
         * @param path_pcm pcm file pateh
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Player.play
        */
        err::Err play_pcm_file(std::string path_pcm);
        /**
         * play_pcm_file_flag
         * @param path_pcm pcm file pateh
         * @param flag_play flag
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Player.play
        */
        err::Err play_pcm_file_flag_bool(std::string path_pcm,bool& flag_play,bool&  flag_pause); 
        /**
         * play_pcm_file_flag
         * @param path_pcm pcm file pateh
         * @param flag_play flag
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Player.play
        */
        err::Err play_pcm_file_flag(std::string path_pcm,std::atomic<bool>& flag_play,std::atomic<bool>& flag_pause);
        /**
         * Play_wxw
         * @param data audio data, must be raw data
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Player.play
        */
        err::Err play_wxw(maix::Bytes *data = maix::audio::Player::NoneBytes);
        /**
         * play_data
         * @param data audio data, must be raw data
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Player.play
        */
        err::Err play_data(maix::Bytes *data = maix::audio::Player::NoneBytes);
        /**
         * Play
         * @param data audio data, must be raw data
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Player.play
        */
        err::Err play(maix::Bytes *data = maix::audio::Player::NoneBytes);
        /**
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.Player.sample_rate
         */
        int sample_rate() {
            return _sample_rate;
        }
        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.Player.format
         */
        audio::Format format() {
            return _format;
        }
        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.Player.channel
         */
        int channel() {
            return _channel;
        }
        err::Err finish();
    };
}
```