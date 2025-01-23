/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_rtsp.hpp"
#include "maix_err.hpp"
#include "maix_basic.hpp"
#include "maix_audio.hpp"
#include "maix_video.hpp"
#include "maix_ffmpeg.hpp"
#include <dirent.h>
#include "sophgo_middleware.hpp"
#include "maix_rtsp_server.hpp"
#include <sys/types.h>
#include <ifaddrs.h>

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
        auto configs = sys::device_configs(true);
        for (auto &item : configs) {
            log::info("device:%s value:%s", item.first.c_str(), item.second.c_str());
        }
        auto mirror_string = configs.find("cam_flip");
        auto flip_string = configs.find("cam_mirror");
        if (mirror_string != configs.end()) {
            mirror = !atoi(mirror_string->second.c_str());
        }

        if (flip_string != configs.end()) {
            flip = !atoi(flip_string->second.c_str());
        }

        int x2 = flip ? camera->width() - width - x : x;
        int y2 = mirror ? camera->height() - height - y : y;

        int vi_vpss = 0;
        int vi_vpss_chn = camera->get_channel();
        if (0 != mmf_add_region_channel_v2(rgn_id, 0, 6, vi_vpss, vi_vpss_chn, x2, y2, width, height, mmf_invert_format_to_mmf(format))) {
            err::check_raise(err::ERR_RUNTIME, "mmf_add_region_channel_v2 failed!");
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

    enum RtspStatus{
        RTSP_IDLE = 0,
        RTSP_RUNNING,
        RTSP_STOP,
    };

    typedef struct {
        MaixRtspServer *rtsp_server;
        enum RtspStatus status;
        int *clients;
        camera::Camera *camera;
        video::Encoder *encoder;
        audio::Recorder *audio_recorder;
        ffmpeg::FFmpegPacker *ffmpeg_packer;
        bool bind_camera;
        bool bind_audio_recorder;
        int encoder_bitrate;
        int fps;
        int audio_bitrate;
    } rtsp_param_t;

    Rtsp::Rtsp(std::string ip, int port, int fps, rtsp::RtspStreamType stream_type, int bitrate) {
        rtsp_param_t *param = (rtsp_param_t *)malloc(sizeof(rtsp_param_t));
        err::check_null_raise(param, "malloc failed!");
        memset(param, 0, sizeof(rtsp_param_t));

        this->_ip = ip;
        this->_port = port;
        this->_fps = fps;
        this->_stream_type = stream_type;
        this->_is_start = false;
        this->_thread = NULL;
        this->_param = param;
        this->_region_max_number = 16;
        for (int i = 0; i < this->_region_max_number; i ++) {
            this->_region_list.push_back(NULL);
            this->_region_type_list.push_back(0);
            this->_region_used_list.push_back(false);
        }

        if (_ip.size() == 0) {
            _ip = "0.0.0.0";
        }

        this->_timestamp = 0;
        this->_last_ms = 0;

        param->status = RTSP_IDLE;
        param->camera = nullptr;
        param->audio_recorder = nullptr;
        param->bind_camera = false;
        param->bind_audio_recorder = false;
        param->encoder_bitrate = bitrate;
        param->fps = fps;
        param->audio_bitrate = 128000;
    }

    Rtsp::~Rtsp() {
        rtsp_param_t *param = (rtsp_param_t *)_param;
        if (param) {
            if (param->status != RTSP_IDLE) {
                this->stop();
            }

            if (param->encoder) {
                delete param->encoder;
                param->encoder = nullptr;
            }

            if (param->rtsp_server) {
                delete param->rtsp_server;
                param->rtsp_server = nullptr;
            }

            free(_param);
            _param = nullptr;
        }

        for (auto &region : this->_region_list) {
            delete region;
        }
    }
#if 0
    static void dump_type(uint8_t *m_buf, int bytes_read)
    {
        int i = 0, start_code = 0;
        for (i=0; i<bytes_read-5; i++) {
            if(m_buf[i] == 0 && m_buf[i+1] == 0 && m_buf[i+2] == 1) {
                start_code = 3;

                if (start_code > 0) {
                    printf("[%d] nalu type:%d\r\n", i, m_buf[i+start_code]&0x1F);
                }

                i += start_code;
                continue;
            }
            else if(m_buf[i] == 0 && m_buf[i+1] == 0 && m_buf[i+2] == 0 && m_buf[i+3] == 1) {
                start_code = 4;

                if (start_code > 0) {
                    printf("[%d] nalu type:%d\r\n", i, m_buf[i+start_code]&0x1F);
                }

                i += start_code;
                continue;
            }
            else  {
                continue;
            }
        }
    }
#endif

    static uint64_t get_video_timestamp() {
        return time::ticks_ms() * 90;
    }

    static uint64_t get_audio_timestamp() {
        return get_video_timestamp();
    }

    static void _camera_push_thread(void *args) {
        rtsp_param_t *param = (rtsp_param_t *)args;
        MaixRtspServer *rtsp_server = param->rtsp_server;
        int vi_ch = param->camera->get_channel();
        int enc_ch = 1;
        size_t video_pts = 0, audio_pts = 0;
        uint64_t last_ms = time::ticks_ms();
        bool found_first_pps_sps = false;
        uint64_t last_read_pcm_ms = time::ticks_ms();

        typedef struct {
            MaixRtspServer *rtsp_server;
            bool is_i_or_b;
        } temp_param_t;

        while (param->status == RTSP_RUNNING && !app::need_exit()) {
            void *frame = NULL;
            bool found_camera_frame = false;
            bool found_venc_stream = false;
            bool found_audio_data = false;
            mmf_frame_info_t f_info;
            mmf_stream_t venc_stream = {0};

            while (((time::ticks_ms() - last_ms) * 1000) < ((uint64_t)(1000000 / param->fps) - 1000)) {
                time::sleep_ms(1);
            }

            // log::info(" loop use %lld ms", time::ticks_ms() - last_ms);
            last_ms = time::ticks_ms();

            Bytes *pcm = nullptr;
            if (param->bind_audio_recorder) {
                int frame_size_per_second = param->ffmpeg_packer->get_audio_frame_size_per_second();
                uint64_t loop_ms = time::ticks_ms() - last_read_pcm_ms;
                last_read_pcm_ms = time::ticks_ms();
                int read_pcm_size = frame_size_per_second * loop_ms * 1.5 / 1000;

                auto remain_frame_count = param->audio_recorder->get_remaining_frames();
                auto bytes_per_frame = param->audio_recorder->frame_size();
                auto remain_frame_bytes = remain_frame_count * bytes_per_frame;
                read_pcm_size = (read_pcm_size + 1023) & ~1023;
                if (read_pcm_size > remain_frame_bytes) {
                    read_pcm_size = remain_frame_bytes;
                }
                pcm = param->audio_recorder->record_bytes(read_pcm_size);
            }

            if (pcm) {
                found_audio_data = true;
            }

            if (0 == mmf_vi_frame_pop2(vi_ch, &frame, &f_info) && frame) {
                found_camera_frame = true;
            }

            if (0 == mmf_venc_pop(enc_ch, &venc_stream) && venc_stream.count > 0) {
                found_venc_stream = true;
            }

            if (rtsp_server->get_clients() > 0) {
                if (found_venc_stream) {
                    if (!found_first_pps_sps) {
                        if (venc_stream.count > 1) {
                            found_first_pps_sps = true;
                            video_pts = 0;
                        }
                    }

                    if (found_first_pps_sps) {
                        int venc_output_data_size = 0;
                        uint8_t *venc_output_data = nullptr;
                        for (int i = 0; i < venc_stream.count; i ++) {
                            // printf("[%d] stream.data:%p stream.len:%d\n", i, venc_stream.data[i], venc_stream.data_size[i]);
                            venc_output_data_size += venc_stream.data_size[i];
                        }

                        venc_output_data = (uint8_t *)malloc(venc_output_data_size);
                        if (venc_output_data) {
                            int curr_data_size = 0;
                            for (int i = 0; i < venc_stream.count; i ++) {
                                memcpy(venc_output_data + curr_data_size, venc_stream.data[i], venc_stream.data_size[i]);
                                curr_data_size += venc_stream.data_size[i];
                            }


                            auto _callback = [](void *data, size_t data_size, size_t pts, void *args) {
                                temp_param_t *param = (temp_param_t *)args;
                                param->rtsp_server->video_frame_push((uint8_t *)data, data_size, pts, param->is_i_or_b);
                            };

                            temp_param_t temp_param;
                            temp_param.rtsp_server = rtsp_server;
                            temp_param.is_i_or_b = venc_stream.count > 1 ? true : false;

                            video_pts = get_video_timestamp();
                            param->ffmpeg_packer->push2(venc_output_data, venc_output_data_size, video_pts, false, _callback, &temp_param);
                            free(venc_output_data);
                        }
                    }
                }

                if (found_audio_data) {
                    if (pcm && pcm->data_len > 0) {
                        auto _callback = [](void *data, size_t data_size, size_t pts, void *args) {
                            temp_param_t *param = (temp_param_t *)args;
                            param->rtsp_server->audio_frame_push((uint8_t *)data, data_size, pts);
                        };
                        temp_param_t temp_param;
                        temp_param.rtsp_server = rtsp_server;

                        audio_pts = get_audio_timestamp();
                        param->ffmpeg_packer->push2(pcm->data, pcm->data_len, audio_pts, true, _callback, &temp_param);
                    }
                }
            } else {
                found_first_pps_sps = false;
            }

            if (found_venc_stream) {
                if (0 != mmf_venc_free(enc_ch)) {
                    log::error("mmf_venc_free failed!\r\n");
                }
            }

            if (found_camera_frame) {
                if (0 != mmf_venc_push2(enc_ch, frame)) {
                    log::error("mmf_venc_push2 failed!\r\n");
                }

                mmf_vi_frame_free2(vi_ch, &frame);
            }

            if (found_audio_data) {
                delete pcm;
            }
        }

        param->status = RTSP_IDLE;
    }

    err::Err Rtsp::start() {
        err::Err err = err::ERR_NONE;
        rtsp_param_t *param = (rtsp_param_t *)_param;
        if (!param) {
            return err::ERR_RUNTIME;
        }

        if (param->status != RTSP_IDLE) {
            return err::ERR_BUSY;
        }

        // check camera
        if (!param->bind_camera || !param->camera) {
            log::error("You need bind a camera!");
            return err::ERR_RUNTIME;
        }

        if (param->camera->width() % 32 != 0) {
            log::error("camera width must be multiple of 32!\r\n");
            return err::ERR_RUNTIME;
        }

        // create rtsp server
        MaixRtspServerBuilder rtsp_builder = MaixRtspServerBuilder()
                                    .set_ip(_ip)
                                    .set_port(this->_port)
                                    .set_session_name("live");
        if (param->bind_audio_recorder && param->audio_recorder) {
            rtsp_builder = rtsp_builder.set_audio(true)
                                        .set_audio_channels(param->audio_recorder->channel())
                                        .set_audio_sample_rate(param->audio_recorder->sample_rate());
        }
        param->rtsp_server = rtsp_builder.build();

        // create encoder
        if (param->encoder) {
            delete param->encoder;
            param->encoder = nullptr;
        }
        param->encoder = new video::Encoder("", param->camera->width(), param->camera->height(), image::Format::FMT_YVU420SP, video::VIDEO_H264, param->fps, 50, param->encoder_bitrate);
        err::check_null_raise(param->encoder, "Create video encoder failed!");

        // create frame package
        param->ffmpeg_packer = new ffmpeg::FFmpegPacker();
        err::check_null_raise(param->ffmpeg_packer, "ffmpeg packer init failed");
        err::check_bool_raise(!param->ffmpeg_packer->config2("context_format_name", "flv"), "rtmp config failed!");
        if (param->bind_audio_recorder && param->audio_recorder) {
            if (param->audio_recorder->format() != audio::FMT_S16_LE) {
                log::error("Only support audio::FMT_S16_LE format!");
                return err::ERR_RUNTIME;
            }
            err::check_bool_raise(!param->ffmpeg_packer->config("has_audio", true), "rtmp config failed!");
            err::check_bool_raise(!param->ffmpeg_packer->config("audio_sample_rate", param->audio_recorder->sample_rate()), "rtmp config failed!");
            err::check_bool_raise(!param->ffmpeg_packer->config("audio_channels", param->audio_recorder->channel()), "rtmp config failed!");
            err::check_bool_raise(!param->ffmpeg_packer->config("audio_bitrate", param->audio_bitrate), "rtmp config failed!");
            err::check_bool_raise(!param->ffmpeg_packer->config("audio_format", AV_SAMPLE_FMT_S16), "rtmp config failed!");
        }
        param->ffmpeg_packer->open();

        // start audio
        if (param->bind_audio_recorder) {
            param->audio_recorder->reset(true);
        }

        // create runtime thread
        param->status = RTSP_RUNNING;
        _thread = new thread::Thread(_camera_push_thread, param);
        if (_thread == NULL) {
            log::error("create camera thread failed!\r\n");
            return err::ERR_RUNTIME;
        }
        return err;
    }

    err::Err Rtsp::stop() {
        err::Err err = err::ERR_NONE;
        rtsp_param_t *param = (rtsp_param_t *)_param;
        if (param->status != RTSP_RUNNING) {
            return err::ERR_NONE;
        }

        param->status = RTSP_STOP;
        if (_thread) {
            _thread->join();
            _thread = nullptr;
        }

		while (param->status != RTSP_IDLE) {
			time::sleep_ms(100);
			log::info("wait rtsp thread exit..");
		}

        // stop audio
        if (param->bind_audio_recorder) {
            param->audio_recorder->reset(false);
        }

        if (param->ffmpeg_packer) {
            delete param->ffmpeg_packer;
            param->ffmpeg_packer = nullptr;
        }

        if (param->encoder) {
            delete param->encoder;
            param->encoder = nullptr;
        }

        if (param->rtsp_server) {
            delete param->rtsp_server;
            param->rtsp_server = nullptr;
        }

        return err;
    }

    err::Err Rtsp::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NONE;
        rtsp_param_t *param = (rtsp_param_t *)_param;
        if (!param) {
            return err::ERR_RUNTIME;
        }

        if (camera->format() != image::Format::FMT_YVU420SP) {
            err::check_raise(err::ERR_RUNTIME, "bind camera failed! support FMT_YVU420SP only!\r\n");
            return err::ERR_RUNTIME;
        }

        param->camera = camera;
        param->bind_camera = true;
        return err;
    }

    err::Err Rtsp::bind_audio_recorder(audio::Recorder *recorder) {
        err::Err err = err::ERR_NONE;
        rtsp_param_t *param = (rtsp_param_t *)_param;
        if (!param) {
            return err::ERR_RUNTIME;
        }

        param->audio_recorder = recorder;
        param->bind_audio_recorder = true;
        return err;
    }

    err::Err Rtsp::write(video::Frame &frame) {
        err::Err err = err::ERR_NONE;
        err::check_raise(err::ERR_NOT_IMPL, "write frame not impl!");
        return err;
    }

    camera::Camera *Rtsp::to_camera() {
        rtsp_param_t *param = (rtsp_param_t *)_param;
        err::check_null_raise(param->camera, "camera is null!");
        return param->camera;
    }

    std::string Rtsp::get_url() {
        return "rtsp://" + _ip + ":" + std::to_string(_port) + "/live";
    }


    static int get_ip(char *hw, char ip[16])
    {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
            perror("getifaddrs");
            return -1;
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) {
                continue;
            }

            family = ifa->ifa_addr->sa_family;

            if (family == AF_INET) {
                s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    return -1;
                }

                if (!strcmp(ifa->ifa_name, hw)) {
                    strncpy(ip, host, 16);
                    freeifaddrs(ifaddr);
                    return 0;
                }
            }
        }

        freeifaddrs(ifaddr);
        return -1;
    }

    static std::vector<std::string> rtsp_get_server_urls(std::string ip, int port)
    {
        char new_ip[16] = {0};

        std::vector<std::string> ip_list;

        if (!strcmp("0.0.0.0", ip.c_str())) {
            if (!get_ip((char *)"eth0", new_ip)) {
                ip_list.push_back("rtsp://" + std::string(new_ip) + ":" + std::to_string(port) + "/live");
            }
            if (!get_ip((char *)"usb0", new_ip)) {
                ip_list.push_back("rtsp://" + std::string(new_ip) + ":" + std::to_string(port) + "/live");
            }
            if (!get_ip((char *)"wlan0", new_ip)) {
                ip_list.push_back("rtsp://" + std::string(new_ip) + ":" + std::to_string(port) + "/live");
            }
        } else {
            ip_list.push_back("rtsp://" + ip + ":" + std::to_string(port) + "/live");
        }

        return ip_list;
    }

    std::vector<std::string> Rtsp::get_urls()
    {
        return rtsp_get_server_urls(_ip, _port);
    }

    rtsp::Region *Rtsp::add_region(int x, int y, int width, int height, image::Format format) {
        rtsp_param_t *param = (rtsp_param_t *)_param;
        if (!param) {
            return nullptr;
        }

        if (format != image::Format::FMT_BGRA8888) {
            log::error("region support FMT_BGRA8888 only!\r\n");
            return NULL;
        }

        if (!param->bind_camera) {
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
        rtsp::Region *region = new rtsp::Region(x, y, width, height, format, param->camera);
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
        rtsp_param_t *param = (rtsp_param_t *)_param;
        if (!param) {
            return err::ERR_RUNTIME;
        }

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

        if (x > param->camera->width()) {
            x = 0;
            width = 0;
        }

        if (y > param->camera->height()) {
            y = 0;
            height = 0;
        }

        if (x + width > param->camera->width()) {
            width = param->camera->width() - x;
        }

        if (y + height > param->camera->height()) {
            height = param->camera->height() - y;
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
