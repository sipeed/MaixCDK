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
    Region::Region(int x, int y, int width, int height, image::Format format, camera::Camera *camera)
    {
        if (format != image::Format::FMT_BGRA8888) {
            err::check_raise(err::ERR_RUNTIME, "region support FMT_BGRA8888 only!");
        }

        if (camera == NULL) {
            err::check_raise(err::ERR_RUNTIME, "region bind a NULL camera!");
        }

        int rgn_id = mmf_get_region_unused_channel();
        if (rgn_id < 0) {
            err::check_raise(err::ERR_RUNTIME, "no more region id!");
        }

        int flip = true;
        int mirror = true;
        int x2 = flip ? camera->width() - width - x : x;
        int y2 = mirror ? camera->height() - height - y : y;

        int vi_vpss = 0;
        int vi_vpss_chn = camera->get_channel();
        if (0 != mmf_add_region_channel(rgn_id, 0, 6, vi_vpss, vi_vpss_chn, x2, y2, width, height, mmf_invert_format_to_mmf(format))) {
            err::check_raise(err::ERR_RUNTIME, "mmf_add_region_channel failed!");
        }

        this->_id = rgn_id;
        this->_width = width;
        this->_height = height;
        this->_x = x;
        this->_y = y;
        this->_format = format;
        this->_camera = camera;
        this->_flip = flip;
        this->_mirror = mirror;
    }

    Region::~Region() {
        if (mmf_del_region_channel(this->_id) < 0) {
            err::check_raise(err::ERR_RUNTIME, "mmf_del_region_unused_channel failed!");
        }
    }

    image::Image *Region::get_canvas() {
        void *data;
        if (0 != mmf_region_get_canvas(this->_id, &data, NULL, NULL, NULL)) {
            err::check_raise(err::ERR_RUNTIME, "mmf_region_get_canvas failed!");
        }

        image::Image *img = NULL;
        switch (this->_format) {
        case image::Format::FMT_BGRA8888:
            img = new image::Image(this->_width, this->_height, this->_format, (uint8_t *)data, this->_width * this->_height * 4, false);
            if (img == NULL) {
                mmf_del_region_channel(this->_id);
                err::check_raise(err::ERR_RUNTIME, "malloc failed!");
            }
            memset(img->data(), 0, img->data_size());
        break;
        default:err::check_raise(err::ERR_RUNTIME, "region format not support!");break;
        }

        this->_image = img;

        return img;
    }

    err::Err Region::update_canvas() {
        image::Image *img = this->_image;
        if (img->format() == image::Format::FMT_BGRA8888) {
            uint32_t *data_u32 = (uint32_t *)img->data();
            int width = img->width();
            int height = img->height();

            if (this->_flip) {
                for (int h = 0; h < height; h ++) {
                    for (int w = 0; w < width / 2; w ++) {
                        int left_idx = h * width + w;
                        int right_idx = h * width + (width - 1 - w);
                        uint32_t tmp = data_u32[left_idx];
                        data_u32[left_idx] = data_u32[right_idx];
                        data_u32[right_idx] = tmp;
                    }
                }
            }

            if (this->_mirror) {
                for (int h = 0; h < height / 2; h ++) {
                    for (int w = 0; w < width; w ++) {
                        int left_idx = h * width + w;
                        int right_idx = (height - 1 - h) * width + w;
                        uint32_t tmp = data_u32[left_idx];
                        data_u32[left_idx] = data_u32[right_idx];
                        data_u32[right_idx] = tmp;
                    }
                }
            }
        } else {
            log::error("support FMT_BGRA888 only!\r\n");
            return err::ERR_RUNTIME;
        }

        if (0 != mmf_region_update_canvas(this->_id)) {
            log::error("mmf_region_update_canvas failed!\r\n");
            return err::ERR_RUNTIME;
        }
        return err::ERR_NONE;
    }

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

        for (auto &region : this->_region_list) {
            delete region;
        }
    }

    static void _camera_push_thread(void *args) {
        Rtsp *rtsp = (Rtsp *)args;
        void *data;
        int data_size, width, height, format;
        int vi_ch = 0, enc_ch = 1;
        while (rtsp->rtsp_is_start()) {
            mmf_h265_stream_t stream;
            if (!mmf_enc_h265_pop(enc_ch, &stream)) {
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

                if (mmf_enc_h265_free(enc_ch)) {
                    log::warn("mmf_enc_h265_free failed\n");
                    continue;
                }
            }

            if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                continue;
            }

            if (mmf_enc_h265_push(enc_ch, (uint8_t *)data, width, height, format)) {
                log::warn("mmf_enc_h265_push failed\n");
                continue;
            }

            mmf_vi_frame_free(vi_ch);
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

    err::Err Rtsp::write(video::Frame &stream) {
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

    rtsp::Region *Rtsp::add_region(int x, int y, int width, int height, image::Format format) {
        if (format != image::Format::FMT_BGRA8888) {
            log::error("region support FMT_BGRA8888 only!\r\n");
            return NULL;
        }

        if (!this->_bind_camera) {
            log::error("You must use bind camera firstly!\r\n");
            return NULL;
        }

        rtsp::Region *region = new rtsp::Region(x, y, width, height, format, this->_camera);
        if (region) {
            this->_region_list.push_back(region);
            this->_region_update_flag.push_back(false);
        }
        return region;
    }

    err::Err Rtsp::update_region(rtsp::Region &region) {
        return region.update_canvas();
    }
}
