/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_rtsp.hpp"
#include "maix_err.hpp"
#include "rtsp_server.h"
#include "maix_basic.hpp"
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
        this->_region_max_number = 16;
        for (int i = 0; i < this->_region_max_number; i ++) {
            this->_region_list.push_back(NULL);
            this->_region_type_list.push_back(0);
            this->_region_used_list.push_back(false);
        }

        char *new_ip = NULL;
        if (this->_ip.size() != 0) {
            new_ip = (char *)this->_ip.c_str();
        }
        this->_timestamp = 0;
        this->_last_ms = 0;
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
            rtsp->update_timestamp();
            uint64_t timestamp = rtsp->get_timestamp();

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
                        rtsp_send_h265_data(timestamp, stream_buffer, copy_length);
                        free(stream_buffer);
                    } else {
                        log::warn("malloc failed!\r\n");
                    }
                } else if (stream.count == 1) {
                    rtsp_send_h265_data(timestamp, (uint8_t *)stream.data[0], stream.data_size[0]);
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

    err::Err Rtsp::write(video::Frame &frame) {
        err::Err err = err::ERR_NONE;

        if (frame.type() != video::VideoType::VIDEO_ENC_H265_CBR) {
            log::warn("You passed in an unsupported type!\r\n");
            return err::ERR_RUNTIME;
        }

        void *data;
        int data_len = 0;
        if (err::ERR_NONE != frame.get(&data, &data_len) || data_len == 0) {
            return err::ERR_NONE;
        }

        this->update_timestamp();
        uint64_t timestamp = this->get_timestamp();
        rtsp_send_h265_data(timestamp, (uint8_t *)data, data_len);

        return err;
    }

    std::string Rtsp::get_url() {
        std::string real_ip = std::string(rtsp_get_server_ip());
        std::string real_port = std::to_string(rtsp_get_server_port());
        return "rtsp://" + real_ip + ":" + real_port + "/live";
    }

    std::vector<std::string> Rtsp::get_urls()
    {
        return rtsp_get_server_urls();
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

        // Find unused idx
        int unused_idx = -1;
        for (int i = 0; i < this->_region_max_number; i ++) {
            if (this->_region_used_list[i] == false) {
                unused_idx = i;
                break;
            }
        }
        err::check_bool_raise(unused_idx != -1, "Unused region not found");

        // Create region
        rtsp::Region *region = new rtsp::Region(x, y, width, height, format, this->_camera);
        err::check_null_raise(region, "Create region failed!");
        this->_region_list[unused_idx] = region;
        this->_region_used_list[unused_idx] = true;
        this->_region_type_list[unused_idx] = 0;

        return region;
    }

    err::Err Rtsp::update_region(rtsp::Region &region) {
        return region.update_canvas();
    }

    err::Err Rtsp::del_region(rtsp::Region *region) {
        err::check_null_raise(region, "The region object is NULL");

        for (int i = 0; i < this->_region_max_number; i ++) {
            if (this->_region_list[i] == region) {
                this->_region_list[i] = NULL;
                this->_region_used_list[i] = false;
                this->_region_type_list[i] = 0;
                delete region;
                return err::ERR_NONE;
            }
        }

        return err::ERR_NONE;
    }

    err::Err Rtsp::draw_rect(int id, int x, int y, int width, int height, image::Color color, int thickness)
    {
        // Check id
        if (id < 0 || id > 3) {
            log::error("region id is invalid! range is [0, 3");
            err::check_raise(err::ERR_RUNTIME, "invalid parameter");
        }

        if (x < 0) {
            width = width + x < 0 ? 0 : width + x;
            x = 0;
        }

        if (y < 0) {
            height = height + y < 0 ? 0 : height + y;
            y = 0;
        }

        if (x > this->_camera->width()) {
            x = 0;
            width = 0;
        }

        if (y > this->_camera->height()) {
            y = 0;
            height = 0;
        }

        if (x + width > this->_camera->width()) {
            width = this->_camera->width() - x;
        }

        if (y + height > this->_camera->height()) {
            height = this->_camera->height() - y;
        }

        // Check if the region [id, id + 4) is used for other functions
        for (size_t i = id; i < this->_region_used_list.size() && (int)i < id + 4; i ++) {
            if (_region_used_list[i] == true && _region_type_list[i] != 2) {
                log::error("In areas %d - %d, %d is used for other functions(%d)", id, id + 4, i, _region_type_list[i]);
                err::check_raise(err::ERR_RUNTIME, "invalid parameter");
            }
        }

        // Delete last region
        for (size_t i = id; i < this->_region_used_list.size() && (int)i < id + 4; i ++) {
            if (_region_used_list[i] == true && _region_type_list[i] == 2) {
                delete _region_list[i];
                _region_list[i] = NULL;
                _region_used_list[i] = false;
                _region_type_list[i] = -1;
            }
        }

        // Create upper region
        int upper_lower_height = 0;
        upper_lower_height = thickness > 0 ? thickness : height;
        upper_lower_height = upper_lower_height > height ? height : upper_lower_height;
        rtsp::Region *region_upper = add_region(x, y, width, upper_lower_height);
        err::check_null_raise(region_upper);

        // Create lower region
        rtsp::Region *region_lower = add_region(x, y + height - upper_lower_height, width, upper_lower_height);
        err::check_null_raise(region_upper);

        // Create left region
        int left_right_width = 0;
        left_right_width = thickness > 0 ? thickness : width;
        left_right_width = left_right_width > width ? width : left_right_width;
        rtsp::Region *region_left = add_region(x, y + upper_lower_height, left_right_width, height - 2 * upper_lower_height);
        err::check_null_raise(region_left);

        // Create right region
        rtsp::Region *region_right = add_region(x + width - left_right_width, y + upper_lower_height, left_right_width, height - 2 * upper_lower_height);
        err::check_null_raise(region_right);

        // Config region list
        _region_list[id] = region_upper;
        _region_list[id + 1] = region_lower;
        _region_list[id + 2] = region_left;
        _region_list[id + 3] = region_right;
        _region_used_list[id] = true;
        _region_used_list[id + 1] = true;
        _region_used_list[id + 2] = true;
        _region_used_list[id + 3] = true;
        _region_type_list[id] = 2;
        _region_type_list[id + 1] = 2;
        _region_type_list[id + 2] = 2;
        _region_type_list[id + 3] = 2;

        // Draw all of region
        uint32_t color_hex = color.hex();
        for (int i = id; i < id + 4; i ++) {
            rtsp::Region *region = _region_list[i];
            image::Image *img = region->get_canvas();
            err::check_null_raise(img, "Get canvas image failed!");
            uint32_t *data = (uint32_t *)img->data();
            int width = img->width();
            int height = img->height();
            for (int i = 0; i < height; i ++) {
                for (int j = 0; j < width; j ++) {
                    data[i * width + j] = color_hex;
                }
            }
            update_region(*region);
        }

        return err::ERR_NONE;
    }

    err::Err Rtsp::draw_string(int id, int x, int y, const char *str, image::Color color, int size, int thickness)
    {
        return err::ERR_NOT_IMPL;
    }
}
