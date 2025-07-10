#include "lvgl.h"
#include "app.hpp"
#include "stdio.h"
#include "unistd.h"
#include "maix_basic.hpp"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_app.hpp"
#include "maix_pinmap.hpp"
#include "maix_gpio.hpp"
#include "maix_fs.hpp"
#include "maix_vision.hpp"
#include "maix_ffmpeg.hpp"
#include "region.hpp"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "app_ex.hpp"
#include "maix_fp5510.hpp"
#include "focus.hpp"

#include <numeric>
#include <future>
#include <filesystem>
#include <ctime>

#include <mutex>
#include <thread>

using namespace maix;
using namespace maix::peripheral;
#define ALIGN(v, a) (((v)+(a)-1)&~((a)-1))

static struct {
    uint32_t cam_start_snap_flag : 1;
    uint32_t cam_snap_flag : 1;
    uint32_t video_prepare_is_ok : 1;
    uint32_t video_start_flag : 1;
    uint32_t video_stop_flag : 1;

    uint32_t sensor_ae_mode : 1;    // 0,auto; 1,manual
    uint32_t sensor_awb_mode : 1;   // 0,auto; 1,manual

    uint32_t capture_raw_enable : 1;

    uint32_t sensor_shutter_value;  // us
    int sensor_iso_value;           // 100~800
    int sensor_ev;                  // -400~400

    uint8_t cam_snap_delay_s;
    int video_start_ms;
    std::string video_mp4_path;

    uint64_t loop_last_ms;
    uint64_t last_update_region_ms;
    pipeline::Frame *loop_last_frame;

    bool camera_resolution_update_flag;
    int camera_resolution_w;
    int camera_resolution_h;
    int camera_fps;
    int resolution_index;
    camera::Camera *camera;
    display::Display *disp;
    display::Display *other_disp;
    touchscreen::TouchScreen *touchscreen;
    video::Encoder *encoder;
    gpio::GPIO *light;
    ffmpeg::FFmpegPacker *ffmpeg_packer;
    audio::Recorder *audio_recorder;
    Region *region;
    int encoder_bitrate;

    bool show_timestamp_enable;

    uint64_t last_read_pcm_ms;
    uint64_t last_read_cam_ms;
    uint64_t last_push_venc_ms;
    uint64_t video_pts;
    uint64_t audio_pts;
    bool found_pps_frame;
    Bytes *pps;

    bool found_sps_frame;
    Bytes *sps;

    int timelapse_s;
    bool audio_en;

    ui_camera_config_t ui_camera_cfg;
} priv;

static ui_camera_resolution_config_t default_resulution_configs[UI_CAMERA_RESOLUTION_MAX_NUM] = {
    {false, 3840, 2160, "8.2MP(16:9)"},
    {false, 2560, 1440, "3.7MP(16:9)"},
    {false, 1920, 1080, "2MP(16:9)"},
    {false, 1280, 720, "0.9MP(16:9)"},
    {false, 640, 480, "0.3MP(4:3)"},
};

static void __config_camera_resolution_list(ui_camera_config_t *camera_config, int max_w, int max_h) {
    memcpy(camera_config->resulution_configs, default_resulution_configs, sizeof(default_resulution_configs));
    for (int i = 0; i < UI_CAMERA_RESOLUTION_MAX_NUM; i++) {
        if (camera_config->resulution_configs[i].w == 0
            || camera_config->resulution_configs[i].h == 0) {
            continue;
        }
        if (camera_config->resulution_configs[i].w <= max_w || camera_config->resulution_configs[i].h <= max_h) {
            camera_config->resulution_configs[i].enable = true;
        } else {
            camera_config->resulution_configs[i].enable = false;
        }
    }
}

static int __get_camera_resolution_index(ui_camera_config_t *camera_config, int w, int h)
{
    int index = -1;
    for (int i = 0; i < UI_CAMERA_RESOLUTION_MAX_NUM; i++) {
        if (!camera_config->resulution_configs[i].enable) {
            continue;
        }

        index ++;
        if (camera_config->resulution_configs[i].w == w && camera_config->resulution_configs[i].h == h) {
            break;
        }
    }

    if (index < 0) {
        log::error("Camera resolution not supported, please check the camera resolution(%dx%d)", w, h);
        err::check_raise(err::ERR_RUNTIME, "Camera resolution not supported");
    }
    return index;
}

static void set_audio_enable(bool en)
{
    priv.audio_en = en;
    if (priv.ffmpeg_packer) {
        err::check_bool_raise(!priv.ffmpeg_packer->config("has_audio", priv.audio_en), "rtmp config failed!");
    }
}

static void timelapse_record_init(int second) {
    priv.timelapse_s = second;
    priv.last_push_venc_ms = time::ticks_ms();
}

static bool timelapse_record_is_enable() {
    return priv.timelapse_s == 0 ? false : true;
}

static bool timelapse_record_is_auto() {
    return priv.timelapse_s < 0 ? true : false;
}

static void _capture_image(maix::camera::Camera &camera, maix::image::Image *img);

static int save_buff_to_file(char *filename, uint8_t *filebuf, uint32_t filebuf_len)
{
    int fd = -1;
    fd = open(filename, O_WRONLY | O_CREAT, 0777);
    if (fd <= 2) {
        printf("Open filed, fd = %d\r\n", fd);
        return -1;
    }

    int res = 0;
    if ((res = write(fd, filebuf, filebuf_len)) < 0) {
        printf("Write failed");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int app_pre_init(void)
{
    auto sensor_size = camera::get_sensor_size();
    __config_camera_resolution_list(&priv.ui_camera_cfg, sensor_size[0], sensor_size[1]);
    auto index = __get_camera_resolution_index(&priv.ui_camera_cfg, sensor_size[0], sensor_size[1]);
    priv.camera_resolution_w = sensor_size[0];
    priv.camera_resolution_h = sensor_size[1];
    priv.resolution_index = index;  // 0: 2560x1440; 1: 1920x1080; 2: 1280x720; 3: 640x480
    priv.encoder_bitrate = 3 * 1000 * 1000;
    priv.camera_fps = 30;
    priv.audio_en = true;
    return 0;
}


static void _ui_update_pic_img(maix::image::Image *img)
{
    printf("update small img\n");
    maix::image::Image *resize_img = NULL, *bgra_img = NULL;
    resize_img = img->resize(48, 48, maix::image::Fit::FIT_CONTAIN);
    if (resize_img) {
        bgra_img = resize_img->to_format(maix::image::FMT_BGRA8888);
        if (bgra_img) {
            ui_update_small_img(bgra_img->data(), bgra_img->data_size());
            delete bgra_img;
        }
        delete resize_img;
    }

    printf("update big img\n");
    int width = 552;
    int height = 368;
    if (priv.disp) {
        width = priv.disp->width();
        height = priv.disp->height();
    }
    resize_img = img->resize(width, height, maix::image::Fit::FIT_CONTAIN);
    if (resize_img) {
        bgra_img = resize_img->to_format(maix::image::FMT_BGRA8888);
        if (bgra_img) {
            ui_update_big_img(bgra_img->data(), bgra_img->data_size());
            delete bgra_img;
        }
        delete resize_img;
    }
}

static void _ui_update_new_image_from_maix_path(void)
{
    std::string pic_path = maix::app::get_picture_path();
    printf("pic_path: %s\n", pic_path.c_str());
}

static int _get_encode_bitrate_by_camera_resolution(int w, int h) {
    if (w == 640 && h == 480) {
        return 1 * 1000 * 1000;
    } else if (w == 1280 && h == 720) {
        return 2 * 1000 * 1000;
    } else if (w == 1920 && h == 1080) {
        return 4 * 1000 * 1000;
    } else if (w == 2560 && h == 1440) {
        return 6 * 1000 * 1000;
    } else if (w == 3840 && h == 2160) {
        return 10 * 1000 * 1000;
    } else {
        return 3 * 1000 * 1000;
    }
}

static void trim(std::string &str) {
    str.erase(0, str.find_first_not_of(" \t\n\r"));
    str.erase(str.find_last_not_of(" \t\n\r") + 1);
}

int app_base_init(void)
{
    // FIXME: camera can't switch to other sensor config online.
    // mmf_deinit_v2(true);

    // init camera
    priv.camera = new camera::Camera(priv.camera_resolution_w, priv.camera_resolution_h, image::Format::FMT_YVU420SP, NULL, priv.camera_fps, 3, true, priv.capture_raw_enable);
    err::check_bool_raise(priv.camera->is_opened(), "camera open failed");

    // // init region
    // auto string_size = image::string_size("2024/09/20 10:23:33");
    // auto region_w = 200;
    // auto region_h = string_size.height();
    // auto region_x = 10;
    // auto region_y = priv.camera->height() - region_h - 10;
    // priv.region = new Region(region_x, region_y, region_w, region_h, image::FMT_BGRA8888, priv.camera);
    // err::check_null_raise(priv.region, "region open failed");

    // init display
    priv.disp = new display::Display();
    priv.other_disp = priv.disp->add_channel();  // This object(other_disp) is depend on disp, so we must keep disp.show() running.
    err::check_bool_raise(priv.disp->is_opened(), "display open failed");

    // init encoder
    priv.encoder_bitrate = _get_encode_bitrate_by_camera_resolution(priv.camera_resolution_w, priv.camera_resolution_h);
    priv.encoder = new video::Encoder("", priv.camera_resolution_w, priv.camera_resolution_h, image::Format::FMT_YVU420SP, video::VideoType::VIDEO_H264, priv.camera_fps, 50, priv.encoder_bitrate);

    // touch screen
    priv.touchscreen = new touchscreen::TouchScreen();
    err::check_bool_raise(priv.touchscreen->is_opened(), "touchscreen open failed");

    // init light
    auto device_configs = sys::device_configs();
    auto light_io = std::string("B3");          // default use GPIOB3
    auto it = device_configs.find("cam_light_io");
    if (it != device_configs.end()) {
        auto new_io = it->second;
        trim(new_io);
        if (new_io.size() > 0) {
            light_io = new_io;
        }
    }
    log::info("light_io: %s", light_io.c_str());
    pinmap::set_pin_function(light_io, "GPIO" + light_io);
    priv.light = new gpio::GPIO(light_io, gpio::Mode::OUT);
    err::check_null_raise(priv.light, "light gpio open failed");
    priv.light->low();

    // init audio
    priv.audio_recorder = new audio::Recorder();
    err::check_null_raise(priv.audio_recorder, "audio recorder init failed!");

    // init ffmpeg packer
    priv.ffmpeg_packer = new ffmpeg::FFmpegPacker();
    err::check_null_raise(priv.ffmpeg_packer, "ffmpeg packer init failed");
    err::check_bool_raise(!priv.ffmpeg_packer->config("has_video", true), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("video_codec_id", AV_CODEC_ID_H264), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("video_width", priv.camera_resolution_w), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("video_height", priv.camera_resolution_h), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("video_bitrate", priv.encoder_bitrate), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("video_fps", priv.camera_fps), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("video_pixel_format", AV_PIX_FMT_NV21), "rtmp config failed!");

    err::check_bool_raise(!priv.ffmpeg_packer->config("has_audio", priv.audio_en), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("audio_sample_rate", 48000), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("audio_channels", 1), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("audio_bitrate", 128000), "rtmp config failed!");
    err::check_bool_raise(!priv.ffmpeg_packer->config("audio_format", AV_SAMPLE_FMT_S16), "rtmp config failed!");

    // init gui
    maix::lvgl_init(priv.other_disp, priv.touchscreen);
    app_init(*priv.camera);

    priv.loop_last_ms = time::ticks_ms();
    priv.found_pps_frame = false;
    priv.found_sps_frame = false;
    return 0;
}

int app_base_deinit(void)
{
    maix::lvgl_destroy();

    if (priv.ffmpeg_packer) {
        delete priv.ffmpeg_packer;
        priv.ffmpeg_packer = NULL;
    }

    if (priv.audio_recorder) {
        delete priv.audio_recorder;
        priv.audio_recorder = NULL;
    }

    if (priv.light) {
        priv.light->low();
        delete priv.light;
        priv.light = NULL;
    }

    if (priv.touchscreen) {
        delete priv.touchscreen;
        priv.touchscreen = NULL;
    }

    if (priv.encoder) {
        delete priv.encoder;
        priv.encoder = NULL;
    }

    if (priv.other_disp) {
        delete priv.other_disp;
        priv.other_disp = NULL;
    }

    if (priv.disp) {
        delete priv.disp;
        priv.disp = NULL;
    }

    if (priv.region) {
        delete priv.region;
        priv.region = nullptr;
    }

    if (priv.camera) {
        if (priv.loop_last_frame) {
            delete priv.loop_last_frame;
            priv.loop_last_frame = NULL;
        }
        delete priv.camera;
        priv.camera = NULL;
    }

    app_deinit();
    return 0;
}

// static void show_loop_used_time()
// {
//     static uint64_t ltime = time::ticks_ms();
//     auto rtime = time::ticks_ms();
//     log::info("loop used %llu ms", rtime - ltime);
//     ltime = rtime;
// }

int app_base_loop(void)
{
    // auto fps = time::fps();
    // log::info("curr fps: %0.2f", fps);

    // show_loop_used_time();

    bool found_camera_frame = false;
    if (priv.camera->width() % 64 != 0) {
        printf("camera width must be multiple of 64!\r\n");
        return -1;
    }

    if (priv.camera->format() != image::Format::FMT_YVU420SP) {
        printf("camera format must be YVU420SP!\r\n");
        return -1;
    }

    auto cam_frame = priv.camera->pop(40);
    if (cam_frame != NULL) {
        found_camera_frame = true;
    }

    if (found_camera_frame) {
        auto cam_frame_width = cam_frame->width();
        auto cam_frame_height = cam_frame->height();
        auto cam_frame_format = cam_frame->format();

        if (priv.loop_last_frame) {
            delete priv.loop_last_frame;
            priv.loop_last_frame = NULL;
        }

        // Push frame to encoder

        // Snap picture
        if (priv.cam_snap_flag) {
            priv.cam_snap_flag = false;

            uint8_t *data = (uint8_t *)cam_frame->virtual_address(0);
            int data_size = image::fmt_size[cam_frame_format] * cam_frame_width * cam_frame_height;
            image::Image *img = new image::Image(cam_frame_width, cam_frame_height, cam_frame_format, (uint8_t *)data, data_size, true);
            if (!img) {
                printf("create image failed!\r\n");
                delete cam_frame;
                return -1;
            }

            _capture_image(*priv.camera, img);
            delete img;
        }

        bool found_venc_stream = false;

        // Pop stream from encoder
        auto venc_stream = priv.encoder->pop(0);
        if (venc_stream) {
            found_venc_stream = true;
        }

        if (priv.ffmpeg_packer && priv.ffmpeg_packer->is_opened()) {
            double temp_us = priv.ffmpeg_packer->video_pts_to_us(priv.video_pts);
            priv.audio_pts = priv.ffmpeg_packer->audio_us_to_pts(temp_us);
        }

        if (found_venc_stream) {
            if (priv.video_start_flag && priv.ffmpeg_packer && !priv.ffmpeg_packer->is_opened()) {
                // found sps and pps
                if (!priv.found_pps_frame && venc_stream->has_pps_frame()) {
                    priv.found_pps_frame = true;
                    priv.pps = venc_stream->get_pps_frame();
                }

                if (!priv.found_sps_frame && venc_stream->has_sps_frame()) {
                    priv.found_sps_frame = true;
                    priv.sps = venc_stream->get_sps_frame();
                }

                // ffmpeg packer open
                if (priv.found_pps_frame && priv.found_sps_frame) {
                    int sps_pps_size = priv.pps->data_len + priv.sps->data_len;
                    uint8_t *sps_pps = (uint8_t *)malloc(sps_pps_size);
                    if (sps_pps) {
                        memcpy(sps_pps, priv.sps->data, priv.sps->data_len);
                        memcpy(sps_pps + priv.sps->data_len, priv.pps->data, priv.pps->data_len);

                        if (0 == priv.ffmpeg_packer->config_sps_pps(sps_pps, sps_pps_size)) {
                            while (0 != priv.ffmpeg_packer->open() && !app::need_exit()) {
                                time::sleep_ms(500);
                                log::info("Can't open ffmpeg, retry again..");
                            }

                            if (priv.audio_recorder) {
                                priv.audio_recorder->reset();
                            }

                            priv.last_read_pcm_ms = 0;
                            priv.last_read_cam_ms = 0;
                            priv.video_pts = 0;
                            priv.audio_pts = 0;
                        }
                        free(sps_pps);
                    }
                }
            }

            if (priv.ffmpeg_packer->is_opened()) {
                Bytes *new_frame = NULL;
                if (venc_stream->has_i_frame()) {
                    new_frame = venc_stream->get_i_frame();
                } else if (venc_stream->has_p_frame()) {
                    new_frame = venc_stream->get_p_frame();
                }

                if (new_frame) {
                    if (priv.last_read_cam_ms == 0) {
                        priv.video_pts = 0;
                        priv.last_read_cam_ms = time::ticks_ms();
                    } else {
                        if (!timelapse_record_is_enable()) {
                            priv.video_pts += priv.ffmpeg_packer->video_us_to_pts((time::ticks_ms() - priv.last_read_cam_ms) * 1000);
                        } else {
                            priv.video_pts += priv.ffmpeg_packer->video_us_to_pts(1000000 / priv.camera_fps);
                        }
                        priv.last_read_cam_ms = time::ticks_ms();
                    }
                    // log::info("[VIDEO] pts:%d  pts %f s", priv.video_pts, priv.ffmpeg_packer->video_pts_to_us(priv.video_pts) / 1000000);
                    if (err::ERR_NONE != priv.ffmpeg_packer->push(new_frame->data, new_frame->data_len, priv.video_pts)) {
                        log::error("ffmpeg push failed!");
                    }

                    delete new_frame;
                }
            }
            delete venc_stream;
            venc_stream = nullptr;
        }

        if (priv.audio_en && priv.ffmpeg_packer->is_opened()) {
            int frame_size_per_second = priv.ffmpeg_packer->get_audio_frame_size_per_second();
            uint64_t loop_ms = 0;
            int read_pcm_size = 0;
            if (priv.last_read_pcm_ms == 0) {
                loop_ms = 30;
                read_pcm_size = frame_size_per_second * loop_ms * 1.5 / 1000;
                priv.audio_pts = 0;
                priv.last_read_pcm_ms = time::ticks_ms();
            } else {
                loop_ms = time::ticks_ms() - priv.last_read_pcm_ms;
                priv.last_read_pcm_ms = time::ticks_ms();

                read_pcm_size = frame_size_per_second * loop_ms * 1.5 / 1000;
                priv.audio_pts += priv.ffmpeg_packer->audio_us_to_pts(loop_ms * 1000);
            }

            auto remain_frame_count = priv.audio_recorder->get_remaining_frames();
            auto bytes_per_frame = priv.audio_recorder->frame_size();
            auto remain_frame_bytes = remain_frame_count * bytes_per_frame;
            read_pcm_size = (read_pcm_size + 1023) & ~1023;
            if (read_pcm_size > remain_frame_bytes) {
                read_pcm_size = remain_frame_bytes;
            }

            Bytes *pcm_data = priv.audio_recorder->record_bytes(read_pcm_size);
            if (pcm_data) {
                if (pcm_data->data_len > 0) {
                    // log::info("[AUDIO] pts:%d  pts %f s", priv.audio_pts, priv.ffmpeg_packer->audio_pts_to_us(priv.audio_pts) / 1000000);
                    if (err::ERR_NONE != priv.ffmpeg_packer->push(pcm_data->data, pcm_data->data_len, priv.audio_pts, true)) {
                        log::error("ffmpeg push failed!");
                    }
                }
                delete pcm_data;
            }
        }

        if (priv.video_start_flag && priv.video_prepare_is_ok) {
            uint64_t record_time = time::ticks_ms() - priv.video_start_ms;

            if (!timelapse_record_is_enable()) {
                priv.encoder->push(cam_frame);
            } else {
                if (timelapse_record_is_auto()) {
                    priv.encoder->push(cam_frame);
                } else {
                    if (time::ticks_ms() - priv.last_push_venc_ms > (uint64_t)priv.timelapse_s * 1000) {
                        priv.encoder->push(cam_frame);
                        priv.last_push_venc_ms = time::ticks_ms();
                    }
                }
            }

            ui_set_record_time(record_time);
        }
        priv.loop_last_frame = cam_frame;
    } else {
        cam_frame = priv.loop_last_frame;
    }

    // Push frame to vo
    priv.disp->push(cam_frame, image::FIT_COVER);

    // Run ui rocess, must run after disp.show
    lv_timer_handler();

    app_loop(*priv.camera, *priv.disp, priv.other_disp);

    if (priv.show_timestamp_enable) {
        uint64_t curr_ms = time::ticks_ms();
        if (curr_ms - priv.last_update_region_ms > 1000) {
            if (priv.region) {
                auto img = priv.region->get_canvas();
                if (img) {
                    auto datetime = time::now();
                    auto str1 = datetime->strftime("%Y/%m/%d %H:%M:%S");
                    delete datetime;
                    img->draw_string(0, 0, str1, image::COLOR_WHITE);
                    priv.region->update_canvas();
                }
                // log::info("use:%lld str:%s", time::ticks_ms() - curr_ms, str1.c_str());
            }
            priv.last_update_region_ms = curr_ms;
        }
    }

    // printf("loop time: %ld ms\n", time::ticks_ms() - priv.loop_last_ms);
    // priv.loop_last_ms = time::ticks_ms();
    return 0;
}

int app_init(camera::Camera &cam)
{
    ui_camera_config_read(&priv.ui_camera_cfg);
    auto sensor_size = cam.get_sensor_size();
    __config_camera_resolution_list(&priv.ui_camera_cfg, sensor_size[0], sensor_size[1]);
    ui_camera_config_update(&priv.ui_camera_cfg);
    ui_all_screen_init();
    _ui_update_new_image_from_maix_path();

    usleep(1000 * 1000); // wait sensor init
    uint32_t exposure_time = 0, iso_num = 0;
    exposure_time = priv.camera->exposure();
    iso_num = priv.camera->iso(iso_num);;
    priv.sensor_shutter_value = exposure_time;
    priv.sensor_iso_value = iso_num;
    ui_set_shutter_value((double)exposure_time);
    ui_set_iso_value(iso_num);
    ui_set_select_option(priv.resolution_index);
    ui_set_bitrate(priv.encoder_bitrate, false);
    ui_set_timelapse_s(priv.timelapse_s, false);
    if (priv.capture_raw_enable) {
        ui_click_raw_button();
    }
    return 0;
}

static int app_config_param(void)
{
    if (ui_get_cam_snap_flag()) {
        printf("Take a photo\n");
        priv.cam_start_snap_flag = true;

        if (priv.cam_snap_delay_s > 0) {
            ui_anim_photo_delay_start(priv.cam_snap_delay_s);
        }
    }

    if (ui_get_cam_video_start_flag()) {
        printf("Start video\n");
        priv.video_prepare_is_ok = false;
        priv.video_start_flag = true;
        priv.video_stop_flag = false;
        priv.video_start_ms = time::ticks_ms();
        ui_set_record_time(0);
    }

    if (ui_get_cam_video_stop_flag()) {
        printf("Stop video\n");
        priv.video_prepare_is_ok = false;
        priv.video_start_flag = false;
        priv.video_stop_flag = true;
        priv.video_start_ms = 0;
        ui_set_record_time(0);
    }

    if (ui_get_cam_video_try_stop_flag()) {
        printf("Try to stop video\n");
        priv.video_prepare_is_ok = false;
        priv.video_start_flag = false;
        priv.video_stop_flag = true;
        priv.video_start_ms = 0;
        ui_set_record_time(0);
    }

    if (ui_get_view_photo_flag()) {
        printf("View photo\n");
    }

    if (ui_get_exit_flag()) {
        printf("Exit\n");
        app::set_exit_flag(true);
    }

    if (ui_get_delay_setting_flag()) {
        int delay_ms;
        ui_get_photo_delay(&delay_ms);
        printf("Delay setting: %d ms\n", delay_ms);
        priv.cam_snap_delay_s = delay_ms / 1000;
    }

    if (ui_get_resolution_setting_flag()) {
        int w, h;
        ui_get_resulution(&w, &h);
        priv.resolution_index = ui_get_resolution_setting_idx();
        printf("Resolution setting: %d x %d\n", w, h);

        priv.camera_resolution_update_flag = 1;
        priv.camera_resolution_w = w;
        priv.camera_resolution_h = h;
    }

    if (ui_get_shutter_setting_flag()) {
        if (ui_get_shutter_auto_flag()) {
            printf("Shutter setting: Auto\n");
            if (priv.sensor_ae_mode != 0) {
                priv.camera->exp_mode(0);
                priv.sensor_ae_mode = 0;
            }
        } else {
            if (priv.sensor_ae_mode == 0) {
                uint32_t exposure_time = 0, iso_num = 0;
                priv.camera->exp_mode(1);
                priv.sensor_shutter_value = exposure_time;
                priv.sensor_iso_value = iso_num;
                priv.sensor_ae_mode = 1;
            }

            double shutter_value;
            ui_get_shutter_value(&shutter_value);
            printf("Shutter setting: %f s\n", shutter_value);
            if (shutter_value != 0) {
                priv.camera->exposure(shutter_value);
            }
            priv.sensor_shutter_value = (uint32_t)shutter_value;
        }
    }

    if (ui_get_iso_setting_flag()) {
        if (ui_get_iso_auto_flag()) {
            printf("ISO setting: Auto\n");
            if (priv.sensor_ae_mode != 0) {
                priv.sensor_ae_mode = 0;
                priv.camera->exp_mode(0);
            }
        } else {
            if (priv.sensor_ae_mode == 0) {
                uint32_t exposure_time = 0, iso_num = 0;
                priv.sensor_shutter_value = exposure_time;
                priv.sensor_iso_value = iso_num;
                priv.sensor_ae_mode = 1;
                priv.camera->exp_mode(1);
            }
            int iso_value;
            ui_get_iso_value(&iso_value);
            printf("ISO setting: %d\n", iso_value);
            priv.camera->iso(iso_value);
            priv.sensor_iso_value = iso_value;
        }
    }

    if (ui_get_focus_btn_update_flag()) {
        if (ui_get_focus_btn_touched()) {
            int width = ALIGN(priv.disp->width(), 64);
            int height = ALIGN(priv.disp->height(), 64);
            std::vector<int> sensor_size = priv.camera->get_sensor_size();
            int windowing_x = ALIGN((sensor_size[0] - width) / 2, 64);
            int windowing_y = ALIGN((sensor_size[1] - height) / 2, 64);
            priv.camera->set_windowing({windowing_x, windowing_y, width, height});
            log::info("camera set windowing, %d, %d, %d, %d", windowing_x, windowing_y, width, height);
        } else {
            std::vector<int> sensor_size = priv.camera->get_sensor_size();
            priv.camera->set_windowing({0, 0, sensor_size[0], sensor_size[1]});
            log::info("camera set windowing, %d, %d, %d, %d", 0, 0, sensor_size[0], sensor_size[1]);
        }
    }

    if (ui_get_raw_btn_update_flag()) {
        if (ui_get_raw_btn_touched()) {
            log::info("camera enable capture raw");
            priv.capture_raw_enable = 1;
        } else {
            log::info("camera disable capture raw");
            priv.capture_raw_enable = 0;
        }
        app_base_deinit();
        app_base_init();
    }

    if (ui_get_light_btn_update_flag()) {
        if (ui_get_light_btn_touched()) {
            log::info("light on");
            if (priv.light) {
                priv.light->high();
            }
        } else {
            log::info("light off");
            if (priv.light) {
                priv.light->low();
            }
        }
    }


    if (ui_get_ev_setting_flag()) {
        if (ui_get_ev_auto_flag()) {
            printf("EV setting: Auto\n");
        } else {
            double ev_value;
            ui_get_ev_value(&ev_value);
            printf("EV setting: %f\n", ev_value);
        }
    }

    if (ui_get_wb_setting_flag()) {
        if (ui_get_wb_auto_flag()) {
            printf("WB setting: Auto\n");
        } else {
            int wb_value;
            ui_get_wb_value(&wb_value);
            printf("WB setting: %d\n", wb_value);
        }
    }

    if (ui_get_bitrate_update_flag()) {
        if (priv.video_start_flag || priv.video_prepare_is_ok) {
            ui_set_bitrate(priv.encoder_bitrate, false);
            log::warn("video is busy!");
        } else {
            priv.encoder_bitrate = ui_get_bitrate();
            printf("Bitrate changed to %d\n", priv.encoder_bitrate);
            if (priv.encoder) {
                delete priv.encoder;
                priv.encoder = nullptr;
                priv.encoder = new video::Encoder("", priv.camera_resolution_w, priv.camera_resolution_h, image::Format::FMT_YVU420SP, video::VideoType::VIDEO_H264, 30, 50, priv.encoder_bitrate);
            }
        }
    }

    if (ui_get_timestamp_btn_update_flag()) {
        priv.show_timestamp_enable = ui_get_timestamp_btn_touched();
        if (priv.region) {
            delete priv.region;
            priv.region = nullptr;
        }

        if (priv.show_timestamp_enable) {
            // init region
            auto string_size = image::string_size("2024/09/20 10:23:33");
            auto region_w = 200;
            auto region_h = string_size.height();
            auto region_x = 10;
            auto region_y = priv.camera->height() - region_h - 10;
            priv.region = new Region(region_x, region_y, region_w, region_h, image::FMT_BGRA8888, priv.camera);
            err::check_null_raise(priv.region, "region open failed");
        }
    }

    if (ui_get_timelapse_update_flag()) {
        int timelapse_s = ui_get_timelapse_s();
        timelapse_record_init(timelapse_s);
        if (timelapse_s == 0) {
            set_audio_enable(true);
        } else {
            set_audio_enable(false);
        }
        log::info("timelapse_s update: %d s", timelapse_s);
    }

    return 0;
}

static void _capture_image(maix::camera::Camera &camera, maix::image::Image *img)
{
    char *date = ui_get_sys_date();
    if (date) {
        string picture_root_path = maix::app::get_picture_path();
        string picture_date(date);
        string picture_path = picture_root_path + "/" + picture_date;
        printf("picture_path path:%s\n", picture_path.c_str());
        if (!fs::exists(picture_path)) {
            fs::mkdir(picture_path);
        }
        std::vector<std::string> *file_list = fs::listdir(picture_path);
        if (file_list) {
            auto it = std::find(file_list->begin(), file_list->end(), ".thumbnail");
            if (it != file_list->end()) {
                file_list->erase(it);
            }
        }

        printf("file_list_cnt:%ld\n", file_list->size());
        string picture_save_path = picture_path + "/" + std::to_string(file_list->size()) +".jpg";
        std::string thumbnail_path = picture_path + "/.thumbnail/" + std::to_string(file_list->size()) +".jpg";
        printf("picture_path path:%s  picture_save_path:%s\n", picture_path.c_str(), picture_save_path.c_str());

        if (img) {
            if (err::ERR_NONE != img->save(picture_save_path.c_str())) {
                log::error("save picture error");
                return;
            }

            if (!fs::exists(picture_save_path)) {
                log::error("picture %s not exists", picture_save_path.c_str());
                return;
            }
            image::Image *load_img = image::load((char *)picture_save_path.c_str());
            if (load_img) {
                auto small_img = load_img->resize(priv.disp->width(), priv.disp->height(), image::Fit::FIT_CONTAIN);
                if (small_img) {
                    _ui_update_pic_img(small_img);

                    auto thumbnail_img = small_img->resize(128, 128, image::Fit::FIT_COVER);
                    if (thumbnail_img) {
                        thumbnail_img->save(thumbnail_path.c_str());
                        delete thumbnail_img;
                    }
                    delete small_img;
                }
                ui_anim_run_save_img();
                delete load_img;
            }
            system("sync");

            printf("update small and big img\n");
        } else {
            printf("Found not image..\n");
        }

        if (priv.capture_raw_enable) {
            printf("save raw photo\n");
            image::Image *raw = camera.read_raw();
            if (raw) {
                string raw_save_path = picture_path + "/" + std::to_string(file_list->size()) + "_" + image::fmt_names[raw->format()] + ".raw";
                log::info("save raw to %s", raw_save_path.c_str());
                save_buff_to_file((char *)raw_save_path.c_str(), (uint8_t *)raw->data(), raw->data_size());
                delete raw;
                system("sync");
            }
        }
        free(file_list);
        free(date);
    } else {
        printf("get date failed!\n");
    }
}

int app_loop(maix::camera::Camera &camera, maix::display::Display &disp, maix::display::Display *disp2)
{
    app_config_param();

    if (priv.cam_start_snap_flag) {
        if (priv.cam_snap_delay_s == 0 || (priv.cam_snap_delay_s > 0 && ui_get_photo_delay_anim_stop_flag())) {
            priv.cam_start_snap_flag = false;
            priv.cam_snap_flag = true;
        }
    }
    if (priv.video_start_flag && !priv.video_prepare_is_ok) {
        printf("Prepare record video\n");
        char *date = ui_get_sys_date();
        if (date) {
            string video_root_path = maix::app::get_video_path();
            string video_date(date);
            string video_path = video_root_path + "/" + video_date;
            printf("video_path path:%s\n", video_path.c_str());
            if (!fs::exists(video_path)) {
                fs::mkdir(video_path);
            }
            std::vector<std::string> *file_list = fs::listdir(video_path);
            string video_mp4_path = video_path + "/" + std::to_string(file_list->size()) +".mp4";
            free(file_list);
            free(date);

            if (priv.ffmpeg_packer) {
                priv.ffmpeg_packer->config2("path", video_mp4_path);
            }

            priv.video_mp4_path = video_mp4_path;
            printf("video_save_path:%s\n", priv.video_mp4_path.c_str());
        } else {
            printf("get date failed!\n");
            priv.video_mp4_path = "";
        }

        priv.video_prepare_is_ok = true;
    }

    if (priv.video_stop_flag) {
        printf("Stop video\n");
        if (priv.ffmpeg_packer) {
            priv.ffmpeg_packer->close();
        }
        system("sync");

        priv.video_stop_flag = false;
        priv.video_prepare_is_ok = false;
        priv.video_start_flag = false;
        priv.video_start_ms = 0;
        priv.found_pps_frame = false;
        priv.found_sps_frame = false;
    }

    if (priv.camera_resolution_update_flag) {
        priv.camera_resolution_update_flag = false;
        app_base_deinit();
        app_base_init();
    }
    return 0;
}

int app_deinit(void)
{
    return 0;
}