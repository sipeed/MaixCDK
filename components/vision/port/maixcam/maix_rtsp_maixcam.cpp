/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_rtsp.hpp"
#include "maix_err.hpp"
#include "rtsp-server.hpp"
#include <dirent.h>
#include "sophgo_middleware.hpp"

namespace maix::rtsp
{
    Rtsp::Rtsp(std::string ip, int port, int fps, rtsp::RtspStreamType stream_type) {
        err::check_bool_raise(stream_type == rtsp::RtspStreamType::RTSP_STREAM_H265,
                            "support RTSP_STREAM_H265 only!");
        this->_ip = ip;
        this->_port = port;
        this->_fps = fps;
        this->_stream_type = stream_type;
        this->_bind_camera = false;
        this->_is_start = false;
        this->_thread = NULL;

        char *new_ip = NULL;
        if (this->_ip.size() != 0) {
            new_ip = (char *)this->_ip.c_str();
        }

        err::check_bool_raise(!rtsp_server_init(new_ip, this->_port), "Rtsp init failed!");
    }

    Rtsp::~Rtsp() {
        if (this->_is_start) {
            this->stop();
        }

        if (0 != rtsp_server_deinit()) {
            log::warn("rtsp deinit failed!\r\n");
        }
    }

    static void _camera_push_thread(void *args) {
        Rtsp *rtsp = (Rtsp *)args;
        void *data;
        int data_size, width, height, format;
        int vi_ch = 0, enc_ch = 0;
        while (rtsp->rtsp_is_start()) {
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
                    // log::info("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];
                }

                if (stream.count > 1) {
                    uint8_t *stream_buffer = (uint8_t *)malloc(stream_size);
                    if (stream_buffer) {
                        int copy_length = 0;
                        for (int i = 0; i < stream.count; i ++) {
                            memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                            copy_length += stream.data_size[i];
                        }
                        rtsp_send_h265_data(stream_buffer, copy_length);
                        free(stream_buffer);
                    } else {
                        log::warn("malloc failed!\r\n");
                    }
                } else if (stream.count == 1) {
                    rtsp_send_h265_data((uint8_t *)stream.data[0], stream.data_size[0]);
                }
            }

            if (mmf_enc_h265_free(enc_ch)) {
                log::warn("mmf_enc_h265_free failed\n");
                continue;
            }
        }
    }

    err::Err Rtsp::start() {
        err::Err err = err::ERR_NONE;

        if (0 != rtsp_server_start()) {
            log::error("rtsp start failed!\r\n");
            return err::ERR_RUNTIME;
        }

        if (this->_bind_camera) {
            this->_thread = new thread::Thread(_camera_push_thread, this);
            if (this->_thread == NULL) {
                log::error("create camera thread failed!\r\n");
                return err::ERR_RUNTIME;
            }
        }

        this->_is_start = true;

        return err;
    }

    err::Err Rtsp::stop() {
        err::Err err = err::ERR_NONE;

        this->_is_start = false;

        if (this->_bind_camera) {
            this->_thread->join();
        }

        if (0 != rtsp_server_stop()) {
            log::error("rtsp stop failed!\r\n");
            this->_is_start = true;
            return err::ERR_RUNTIME;
        }

        return err;
    }

    err::Err Rtsp::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NONE;

        if (camera->format() != image::Format::FMT_YVU420SP) {
            err::check_raise(err::ERR_RUNTIME, "bind camera failed! support FMT_YVU420SP only!\r\n");
            return err::ERR_RUNTIME;
        }

        this->_camera = camera;
        this->_bind_camera = true;
        return err;
    }

    err::Err Rtsp::write(video::VideoStream &stream) {
        err::Err err = err::ERR_NONE;

        if (stream.type != video::VideoType::VIDEO_ENC_H265_CBR) {
            log::warn("You passed in an unsupported type!\r\n");
            return err::ERR_RUNTIME;
        }


        rtsp_send_h265_data(stream.frame.get(), sizeof(stream.frame));

        return err;
    }

    std::string Rtsp::get_url() {
        std::string real_ip = std::string(rtsp_get_server_ip());
        std::string real_port = std::to_string(rtsp_get_server_port());
        return "rtsp://" + real_ip + ":" + real_port + "/live";
    }
}
