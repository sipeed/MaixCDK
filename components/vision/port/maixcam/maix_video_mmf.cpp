/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include <stdint.h>
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_video.hpp"
#include "sophgo_middleware.hpp"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace maix::video
{
    Encode::Encode(int width, int height, video::VideoType type) {
        this->_pre_width = width;
        this->_pre_height = height;
        this->_pre_video_type = type;
        this->_is_opened = false;
    }

    Encode::~Encode() {
        if (this->_is_opened) {
            this->close();
        }
    }

    err::Err Encode::open(int width, int height, video::VideoType type) {
        err::Err err = err::ERR_NONE;

        if (width == -1) {
            this->_width = this->_pre_width;
        } else {
            this->_width = width;
        }

        if (height == -1) {
            this->_height = this->_pre_height;
        } else {
            this->_height = height;
        }

        if (type == video::VideoType::VIDEO_ENC_H265_CBR) {
            this->_video_type = this->_pre_video_type;
        } else {
            this->_video_type = type;
        }

        if (this->_is_opened) {
            return err::ERR_NONE;
        }

        switch (this->_video_type) {
        case VIDEO_ENC_H265_CBR:
            if (mmf_enc_h265_init(0, this->_width, this->_height)) {
                log::error("open encode failed\r\n");
                return err::ERR_RUNTIME;
            }
        break;
        default:
            log::error("encode not support type %d\r\n", this->_video_type);
            return err::ERR_RUNTIME;
        }

        this->_is_opened = true;

        return err;
    }

    void Encode::close() {
        if (!this->_is_opened) {
            return;
        }

        switch (this->_video_type) {
        case VIDEO_ENC_H265_CBR:
            if (mmf_enc_h265_deinit(0)) {
                log::error("close encode failed\r\n");
                return;
            }
        break;
        default:
            log::error("encode not support type %d\r\n", this->_video_type);
            return;
        }
    }

    video::VideoStream Encode::encode(image::Image &img) {
        video::VideoStream stream;
        if (!this->_is_opened) {
            return stream;
        }

        switch (this->_video_type) {
        case VIDEO_ENC_H265_CBR:
            if (mmf_enc_h265_push(0, (uint8_t *)img.data(), img.width(), img.height(), mmf_invert_format_to_mmf(img.format()))) {
                log::error("encode push failed\r\n");
                return stream;
            }

            mmf_h265_stream_t mmf_stream;
            if (mmf_enc_h265_pop(0, &mmf_stream)) {
                log::error("encode pop failed\r\n");
                return stream;
            }

            if (mmf_enc_h265_free(0)) {
                log::error("encode free failed\r\n");
                return stream;
            }
        break;
        default:
            log::error("encode not support type %d\r\n", this->_video_type);
            return stream;
        }

        return stream;
    }

    Decode::Decode(int width, int height, video::VideoType type) {

    };

    Decode::~Decode() {

    }

    err::Err Decode::open(int width, int height, video::VideoType type) {
        return err::ERR_NOT_IMPL;
    }

    void Decode::close() {

    }

    image::Image *Decode::decode(video::VideoStream &stream) {
        return NULL;
    }

    Video::Video(std::string path, bool record, int interval, int width, int height, bool audio, int sample_rate, int channel, bool open)
    {
        this->_pre_path = path;
        this->_pre_record = record;
        this->_pre_interval = interval;
        this->_pre_width = width;
        this->_pre_height = height;
        this->_pre_audio = audio;
        this->_pre_sample_rate = sample_rate;
        this->_pre_channel = channel;
        this->_video_type = VIDEO_NONE;
        this->_bind_camera = false;
        this->_is_recording = false;
        this->_camera = NULL;
        this->_fd = -1;
        this->_is_opened = false;

        if (open) {
            err::check_bool_raise(err::ERR_NONE == this->open(), "Video open failed!\r\n");
        }
    }

    Video::~Video() {
        if (this->_is_opened) {
            this->close();
        }
    }

    err::Err Video::open(std::string path, bool record, int interval, int width, int height, bool audio, int sample_rate, int channel)
    {
        if (this->_is_opened) {
            return err::ERR_NONE;
        }

        if (path == std::string()) {
            this->_path = this->_pre_path;
        } else {
            this->_path = path;
        }

        if (record == false) {
            this->_record = this->_pre_record;
        } else {
            this->_record = record;
        }

        if (interval == 33333) {
            this->_interval = this->_pre_interval;
        } else {
            this->_interval = interval;
        }

        if (width == -1) {
            this->_width = this->_pre_width;
        } else {
            this->_width = width;
        }

        if (height == -1) {
            this->_height = this->_pre_height;
        } else {
            this->_height = height;
        }

        if (audio == false) {
            this->_audio = this->_pre_audio;
        } else {
            this->_audio = audio;
        }

        if (sample_rate == 44100) {
            this->_sample_rate = this->_pre_sample_rate;
        } else {
            this->_sample_rate = sample_rate;
        }

        if (channel == 1) {
            this->_channel = this->_pre_channel;
        } else {
            this->_channel = channel;
        }

        if (this->_path.size() == 0) {
            this->_path = "/root/output.mp4";
        }

        std::string name, ext;
        size_t pos = this->_path.rfind('.');
        if (pos != std::string::npos) {
            name = this->_path.substr(0, pos);
        }

        pos = this->_path.rfind('.');
        if (pos != std::string::npos) {
            ext = this->_path.substr(pos);
        }

        if (ext.find(".h265") != std::string::npos) {
            if (this->_record) {
                this->_video_type = VIDEO_ENC_H265_CBR;
            } else {
                this->_video_type = VIDEO_DEC_H265_CBR;
            }
        } else if (ext.find(".mp4") != std::string::npos) {
            if (this->_record) {
                this->_video_type = VIDEO_ENC_MP4_CBR;
                this->_tmp_path = "video_tmp.h265";
            } else {
                this->_video_type = VIDEO_DEC_MP4_CBR;
            }
        } else {
            log::error("Video not support %s!\r\n", ext.c_str());
            return err::ERR_RUNTIME;
        }

        switch (this->_video_type) {
        case VIDEO_ENC_H265_CBR:
            break;
        case VIDEO_ENC_MP4_CBR:
            break;
        default:
            log::error("Find not video type %d!\r\n", this->_video_type);
            return err::ERR_RUNTIME;
        }

        this->_is_opened = true;
        return err::ERR_NONE;
    }

    void Video::close()
    {
        if (this->_is_opened) {
            return;
        }

        this->_is_opened = false;
    }

    err::Err Video::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NONE;

        if (camera->format() != image::Format::FMT_YVU420SP) {
            err::check_raise(err::ERR_RUNTIME, "bind camera failed! support FMT_YVU420SP only!\r\n");
            return err::ERR_RUNTIME;
        }

        this->_camera = camera;
        this->_bind_camera = true;
        return err;
    }

    static void _camera_thread(void *args) {
        video::Video *video = (video::Video *)args;
        video->camera_thread();
    }

    void Video::camera_thread() {
        void *data;
        int data_size, width, height, format;
        int vi_ch = 0, enc_ch = 0;

        char *save_path = NULL;
        switch (this->_video_type) {
        case VIDEO_ENC_H265_CBR:
            save_path = (char *)this->_path.c_str();
            break;
        case VIDEO_ENC_MP4_CBR:
            save_path = (char *)this->_tmp_path.c_str();
            break;
        default:
            log::error("%s not support\r\n", this->_video_type);
            return;
        }

        this->_fd = ::open(save_path, O_WRONLY | O_CREAT, 0777);
        if (this->_fd < 0) {
            log::error("Open %s failed!\r\n", this->_path);
            this->_is_recording = false;
        }

        while (this->_is_recording) {
            if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                continue;
            }

            if (mmf_enc_h265_push(enc_ch, (uint8_t *)data, width, height, format)) {
                log::warn("mmf_enc_h265_push failed\n");
                continue;
            }

            mmf_vi_frame_free(vi_ch);

            mmf_h265_stream_t stream;
            if (mmf_enc_h265_pop(enc_ch, &stream)) {
                log::warn("mmf_enc_h265_pull failed\n");
                continue;
            }

            {
                int stream_size = 0;
                for (int i = 0; i < stream.count; i ++) {
                    log::info("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];

                    int res = 0;
                    if ((res = write(this->_fd, stream.data[i], stream.data_size[i])) < 0) {
                        log::error("Write failed, res = %d\r\n", res);
                    }
                }
            }

            if (mmf_enc_h265_free(enc_ch)) {
                log::warn("mmf_enc_h265_free failed\n");
                continue;
            }

            if (this->_record_start_ms != (uint64_t)-1 && time::time_ms() - this->_record_start_ms >= this->_record_ms) {
                log::info("curr ms:%ld last ms:%ld record ms:%ld\r\n", time::time_ms(), this->_record_start_ms, this->_record_ms);
                this->_is_recording = false;
            }
        }

        mmf_enc_h265_deinit(0);

        if (this->_fd > 2) {
            ::close(this->_fd);

            switch (this->_video_type) {
            case VIDEO_ENC_H265_CBR:
                break;
            case VIDEO_ENC_MP4_CBR:
            {
                char cmd[128];
                snprintf(cmd, sizeof(cmd), "ffmpeg -loglevel quiet -i %s -c:v copy -c:a copy %s -y", this->_tmp_path.c_str(), this->_path.c_str());
                system(cmd);

                snprintf(cmd, sizeof(cmd), "rm %s", this->_tmp_path.c_str());
                system(cmd);
            }
                break;
            default:
                log::error("%s not support\r\n", this->_video_type);
            }
            system("sync");
        }
    }

    err::Err Video::record_start(uint64_t record_time) {
        if (this->_is_recording) {
            return err::ERR_NONE;
        }

        switch (this->_video_type) {
        case VIDEO_ENC_H265_CBR: // fall through
        case VIDEO_ENC_MP4_CBR:
            if (this->_bind_camera) {
                this->_record_ms = record_time;
                this->_record_start_ms = time::time_ms();
                this->_is_recording = true;
                this->_thread = new thread::Thread(_camera_thread, (void *)this);
                if (this->_thread == NULL) {
                    log::error("create camera thread failed!\r\n");
                    this->_is_recording = false;
                    return err::ERR_RUNTIME;
                }
            }
            break;
        default:
            log::error("%s not support record_start\r\n", this->_video_type);
            return err::ERR_RUNTIME;
        }

        return err::ERR_NONE;
    }

    err::Err Video::record_finish() {
        if (!this->_is_recording) {
            return err::ERR_NONE;
        }

        this->_is_recording = false;

        if (this->_bind_camera) {
            this->_thread->join();
        }

        return err::ERR_NONE;
    }

    image::Image *Video::capture() {
        return NULL;
    }

    video::VideoStream Video::encode(image::Image &img)
    {
        video::VideoStream stream;
        return stream;
    }

    image::Image *Video::decode(video::VideoStream &stream)
    {
        return NULL;
    }
} // namespace maix::video
