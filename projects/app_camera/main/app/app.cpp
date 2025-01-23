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
#include "sophgo_middleware.hpp"
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

#define MMF_VENC_CHN            (1)
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
    void *loop_last_frame;

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

    int timelapse_s;
    bool audio_en;
} priv;

static void __find_fp5510(bool& flag, int id=-1, int slave_addr=-1, int freq=-1)
{
    using maix::ext_dev::fp5510::FP5510;
    try {
        if (id == -1 && slave_addr == -1 && freq == -1) {
            FP5510{};
        } else {
            FP5510(id, slave_addr, freq);
        }
    } catch (...) {
        flag = false;
        return;
    }
    flag = true;
    log::info("Found FP5510");
}

static bool fp5510_exist(int id=-1, int slave_addr=-1, int freq=-1)
{
    static bool cached = false;
    static std::once_flag flag;
    std::call_once(flag, __find_fp5510, cached, id, slave_addr, freq);
    return cached;
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
    priv.camera_resolution_w = 2560;
    priv.camera_resolution_h = 1440;
    priv.resolution_index = 0;  // 0: 2560x1440; 1: 1920x1080; 2: 1280x720; 3: 640x480
    priv.encoder_bitrate = 3 * 1000 * 1000;
    priv.camera_fps = 30;
    priv.audio_en = true;


    /* app ex init */
    tmc2209_init();
    TMC2209_EXIST_DO(
        log::info("found device: tmc2209");
        hp_shot_init();
        snap_init();
    )
    return 0;
}

static int _mmf_set_exp_mode(int ch, int mode)
{
    if (mode == 0) {
        if (priv.camera) {
            priv.camera->exp_mode(0);
        }
    }

	CVI_U32 ret;
	ISP_EXPOSURE_ATTR_S stExpAttr;

	memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));

	ret = CVI_ISP_GetExposureAttr(ch, &stExpAttr);
	if (ret != 0) {
		printf("CVI_ISP_GetExposureAttr failed, ret: %#x.\r\n", ret);
		return -1;
	}

	if (stExpAttr.enOpType == mode) {
		return 0;
	}

	stExpAttr.u8DebugMode = 0;
	if (mode == 0) {
		stExpAttr.bByPass = 0;
		stExpAttr.enOpType = OP_TYPE_AUTO;
		stExpAttr.stManual.enExpTimeOpType = OP_TYPE_AUTO;
		stExpAttr.stManual.enISONumOpType = OP_TYPE_AUTO;
		stExpAttr.stManual.enAGainOpType = OP_TYPE_AUTO;
		stExpAttr.stManual.enDGainOpType = OP_TYPE_AUTO;
		stExpAttr.stManual.enISPDGainOpType = OP_TYPE_AUTO;
	} else if (mode == 1) {
		stExpAttr.bByPass = 0;
		stExpAttr.enOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enISONumOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enGainType = AE_TYPE_ISO;
	}

	ret = CVI_ISP_SetExposureAttr(ch, &stExpAttr);
	if (ret != 0) {
		printf("CVI_ISP_SetExposureAttr failed, ret: %#x.\r\n", ret);
		return -1;
	}

	return 0;
}

static void _mmf_set_exptime_and_iso(int ch, int exp_time, int iso)
{
    mmf_set_exptime_and_iso(ch, exp_time, iso);
    if (priv.camera) {
        priv.camera->exposure(exp_time);
    }
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

static int _mmf_vi_frame_pop(int ch, void **frame_info,  mmf_frame_info_t *frame_info_mmap, int block_ms) {
    if (frame_info == NULL || frame_info_mmap == NULL) {
        printf("invalid param\n");
        return -1;
    }

    int ret = -1;
    VIDEO_FRAME_INFO_S *frame = (VIDEO_FRAME_INFO_S *)malloc(sizeof(VIDEO_FRAME_INFO_S));
    memset(frame, 0, sizeof(VIDEO_FRAME_INFO_S));
    if ((ret = CVI_VPSS_GetChnFrame(0, ch, frame, (CVI_S32)block_ms)) == 0) {
        int image_size = frame->stVFrame.u32Length[0]
                        + frame->stVFrame.u32Length[1]
                        + frame->stVFrame.u32Length[2];
        CVI_VOID *vir_addr;
        vir_addr = CVI_SYS_MmapCache(frame->stVFrame.u64PhyAddr[0], image_size);
        CVI_SYS_IonInvalidateCache(frame->stVFrame.u64PhyAddr[0], vir_addr, image_size);

        frame->stVFrame.pu8VirAddr[0] = (CVI_U8 *)vir_addr;		// save virtual address for munmap
        frame_info_mmap->data = vir_addr;
        frame_info_mmap->len = image_size;
        frame_info_mmap->w = frame->stVFrame.u32Width;
        frame_info_mmap->h = frame->stVFrame.u32Height;
        frame_info_mmap->fmt = frame->stVFrame.enPixelFormat;
    } else {
        free(frame);
        frame = NULL;
    }

    if (frame_info) {
        *frame_info = frame;
    }
    return ret;
}

static void _mmf_vi_frame_free(int ch, void **frame_info)
{
    if (!frame_info || !*frame_info) {
        return;
    }

    VIDEO_FRAME_INFO_S *frame = (VIDEO_FRAME_INFO_S *)*frame_info;
    int image_size = frame->stVFrame.u32Length[0]
                        + frame->stVFrame.u32Length[1]
                        + frame->stVFrame.u32Length[2];
    CVI_SYS_Munmap(frame->stVFrame.pu8VirAddr[0], image_size);
    if (CVI_VPSS_ReleaseChnFrame(0, ch, frame) != 0) {
        SAMPLE_PRT("CVI_VI_ReleaseChnFrame NG\n");
    }

    free(*frame_info);
    *frame_info = NULL;
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
    mmf_deinit_v2(true);

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
            _mmf_vi_frame_free(priv.camera->get_channel(), &priv.loop_last_frame);
            priv.loop_last_frame = NULL;
        }
        delete priv.camera;
        priv.camera = NULL;
    }

    app_deinit();
    return 0;
}

static bool _stack_save = false;
// static float _stack_dis = 0;
// static float _stack_cla = 0;
static char _stack_buf[128];
static int _stack_pic_cnt = 0;

void auto_focus_main(const std::function<float(bool)>& get_clarity, bool& auto_focus_start)
{
    constexpr bool enable_filt = true;
    static int filt_cnt = 0;
    [[maybe_unused]]constexpr float fast_up_len = 4;
    [[maybe_unused]]constexpr float fast_down_len = 4;
    [[maybe_unused]]constexpr float nomal_up_len = 1;
    [[maybe_unused]]constexpr float nomal_down_len = 1;
    [[maybe_unused]]constexpr float fast_total_len = fast_up_len + fast_down_len;
    [[maybe_unused]]constexpr float nomal_total_len = nomal_up_len + nomal_down_len;
    static float focus_curr_pos = 0;
    static int focus_mode = 0;
    static float max_cla = std::numeric_limits<float>::min();
    static float max_cla_pos_with_focus_curr_pos = 0;
    static int _cnt = 0;
    [[maybe_unused]]int fast_steps = g_tmc2209_status.steps * 5;
    [[maybe_unused]]int nomal_steps = g_tmc2209_status.steps;
    float cla = 0;

    maix::log::info("focus_mode=%d", focus_mode);

    switch (focus_mode) {
    case 0: {
        /* skip fast mode */
        focus_mode = 5;
        break;
        /* step0: init 2209 --> 1mm mode */
        /* only support 1mmx1 */
        tmc2209_init_with(__high_speed_1mmx1);
        focus_mode = 1;
        break;
    }
    case 1: {
        /* step1: fast up, len<fast_up_len> */
        if constexpr (!enable_filt) {
            for (int i = 0; i < fast_up_len; i+=g_tmc2209_status.step_len) {
                g_slide->move(g_tmc2209_status.step_len);
            }
            focus_mode = 2;
        } else {
            constexpr int filt_num = 10;
            float filt_step_len = g_tmc2209_status.step_len / filt_num;
            int cnt_max = fast_up_len / filt_step_len;
            g_slide->move(filt_step_len);
            filt_cnt++;
            if (filt_cnt >= cnt_max) {
                focus_mode = 2;
                filt_cnt = 0;
            }
        }
        break;
    }
    case 2: {
        /* step2: reserve */
        focus_mode = 3;
        break;
    }
    case 3: {
        /* step3: fast scan */
        if constexpr (!enable_filt) {
            cla = get_clarity(false);
            maix::log::info("fast focus mode, cla: %f", cla);
            max_cla_pos_with_focus_curr_pos += g_tmc2209_status.step_len;
            if (cla > max_cla) {
                max_cla = cla;
                max_cla_pos_with_focus_curr_pos = 0;
                _cnt = 0;
            }
            // else if (cla < max_cla-10.0) {
            //     focus_mode = 4;
            // }
            // for (int i = 0; i < __low_speed_22um4x1.steps; ++i) {
            //     g_slide->move(-__low_speed_22um4x1.step_len);
            // }
            g_slide->move(-g_tmc2209_status.step_len);
            // time::sleep_ms(1);
            _cnt++;
            // focus_curr_pos += __low_speed_22um4x1.step_len * __low_speed_22um4x1.steps;
            focus_curr_pos += g_tmc2209_status.step_len;
            if (focus_curr_pos > fast_total_len || almost_eq(fast_total_len, focus_curr_pos, nomal_up_len)) {
                focus_mode = 4;
            }
        } else {
            constexpr int filt_num = 10;
            float filt_step_len = g_tmc2209_status.step_len / filt_num;
            // int cnt_max = fast_total_len / filt_step_len;

            if (filt_cnt % filt_num == 0) {
                cla = get_clarity(false);
                maix::log::info("fast focus mode, cla: %f", cla);
                if (filt_cnt != 0) {
                    max_cla_pos_with_focus_curr_pos += g_tmc2209_status.step_len;
                    if (cla > max_cla) {
                        max_cla = cla;
                        max_cla_pos_with_focus_curr_pos = 0;
                        _cnt = 0;
                    }
                    _cnt++;
                    focus_curr_pos += g_tmc2209_status.step_len;
                    if (focus_curr_pos > fast_total_len ||
                        almost_eq(fast_total_len, focus_curr_pos, nomal_up_len) ||
                        cla <= max_cla*0.75) {
                        focus_mode = 4;
                    }
                }
            }
            if (focus_mode == 4) {
                maix::log::info("fast focus mode finish, max cla:%f, need return:%f",
                    max_cla, max_cla_pos_with_focus_curr_pos);
                filt_cnt = 0;
                break;
            }
            g_slide->move(-filt_step_len);
            filt_cnt++;
        }
        break;
    }
    case 4: {
        /* step4: backtrack */
        if constexpr (!enable_filt) {
            maix::log::info("fast focus mode finish, max cla:%f, need return:%f", max_cla, max_cla_pos_with_focus_curr_pos);
            g_slide->move(max_cla_pos_with_focus_curr_pos);
            focus_mode = 5; /* skip nomal mode */
            // focus_mode = 100; /* skip nomal mode */
        } else {
            constexpr float filt_step_len = 0.1;
            // int filt_num = static_cast<int>(g_tmc2209_status.step_len/filt_step_len);
            int cnt_max = max_cla_pos_with_focus_curr_pos / filt_step_len;
            g_slide->move(filt_step_len);
            filt_cnt++;
            if (filt_cnt >= cnt_max) {
                filt_cnt = 0;
                focus_mode = 5;
            }
        }
        break;
    }
    case 5: {
        /* step5: init 2209 --> 22um4x1 mode */
        tmc2209_init_with(__low_speed_22um4x1);
        max_cla_pos_with_focus_curr_pos = 0;
        // max_cla = std::numeric_limits<float>::min();
        focus_curr_pos = 0;
        focus_mode = 6;
        break;
    }
    case 6: {
        /* step6: nomal up, len<nomal_up_len> */
        if constexpr (!enable_filt) {
            for (float i = 0; i < nomal_up_len; i+=__low_speed_22um4x1.step_len) {
                g_slide->move(__low_speed_22um4x1.step_len);
            }
            focus_mode = 7;
        } else {
            auto __step_len = __low_speed_22um4x1.step_len;
            g_slide->move(__step_len);
            filt_cnt++;
            if (almost_eq(filt_cnt, nomal_up_len/__step_len) || filt_cnt > nomal_up_len/__step_len) {
                filt_cnt = 0;
                focus_mode = 7;
            }
        }
        break;
    }
    case 7: {
        /* step7: nomal scan */
        constexpr int skip_frame_max = 2;
        static int skip_frame = 0;
        if (skip_frame < skip_frame_max) {
            skip_frame++;
            break;
        }
        cla = get_clarity(false);
        maix::log::info("nomal focus mode, cla: %f", cla);
        max_cla_pos_with_focus_curr_pos += __low_speed_22um4x1.step_len;
        if (cla > max_cla) {
            max_cla = cla;
            max_cla_pos_with_focus_curr_pos = 0;
            _cnt = 0;
        }
        // else if (cla < max_cla-10.0) {
        //     focus_mode = 8;
        // }
        g_slide->move(-__low_speed_22um4x1.step_len);
        time::sleep_ms(1);
        _cnt++;
        focus_curr_pos += __low_speed_22um4x1.step_len;
        if (focus_curr_pos > nomal_total_len ||
            almost_eq(nomal_total_len, focus_curr_pos) ||
            cla <= max_cla*0.75) {
            focus_mode = 8;
        }
        skip_frame = 0;
        break;
    }
    case 8:{
        /* step8: backtract */
        maix::log::info("nomal focus mode finish, max cla:%f, need return:%f", max_cla, max_cla_pos_with_focus_curr_pos);
        g_slide->move(max_cla_pos_with_focus_curr_pos);
        focus_mode = 9;
        break;
    }
    default: {
        /* finish: reset data and exit auto focus */
        tmc2209_init_with(__low_speed_2um8x4);
        focus_curr_pos = 0;
        focus_mode = 0;
        max_cla = std::numeric_limits<float>::min();
        max_cla_pos_with_focus_curr_pos = 0;
        auto_focus_start = false;
        g_auto_focus = std::nullopt;
        break;
    }
    };
}

void shot_tp_main(void)
{
    TMC2209_EXIST_DO(
        constexpr uint64_t snap_check_time = 1000; /* 1s限制,防止误触一直拍摄/录像 */
        static auto prev_snap_check_timepoint = time::ticks_ms();
        auto curr_snap_check_timepoint = time::ticks_ms();
        if (curr_snap_check_timepoint-prev_snap_check_timepoint>=snap_check_time && snap_check(1)) {
            prev_snap_check_timepoint = curr_snap_check_timepoint;
            hp_shot_trigger();
            // priv.cam_snap_flag = true;
            if (g_camera_mode == 0) {
                touch_start_pic();
            } else if (g_camera_mode == 1) {
                touch_start_video(4598);
            }
        }
    )
}

void slide_run_main(int& pic_cnt)
{
    TMC2209_EXIST_DO(
        static TMC2209Status prev_tmc2209_status{
            .run = false, // ignore
            .step_len = 0.0,
            .steps = 0,
            .up = true, // ignore
        };

        if (g_tmc2209_status.run) {
            if (almost_eq(prev_tmc2209_status.step_len, g_tmc2209_status.step_len)
                || prev_tmc2209_status.steps != g_tmc2209_status.steps) {
                tmc2209_init_with(g_tmc2209_status);
                prev_tmc2209_status.step_len = g_tmc2209_status.step_len;
                prev_tmc2209_status.steps = g_tmc2209_status.steps;
            }
            float step_len = g_tmc2209_status.up ? g_tmc2209_status.step_len : -g_tmc2209_status.step_len;
            maix::log::info("step_len %d: %f", g_tmc2209_status.up?1:0, step_len);
            for (int i = 0; i < g_tmc2209_status.steps; ++i) {
                g_slide->move(step_len);
                if (g_ui_stack_status.is_set_start_point && !g_ui_stack_status.is_set_end_point) {
                    g_ui_stack_status.move_len += step_len;
                }
            }
            g_tmc2209_status.run = false;
        }

        if (g_camera_mode == 0) {
            static bool __can_move = true;
            static std::future<void> __fut = std::async([](){});
            if (g_ui_stack_status.need_reset) {
                maix::log::info("reset start");

                g_ui_stack_status.reset_len = -g_ui_stack_status.move_len;
                // g_ui_stack_status.reset_len = std::fabs(g_ui_stack_status.reset_len);
                // g_ui_stack_status.reset_len = (g_ui_stack_status.move_len>=0) ?
                //     -g_ui_stack_status.reset_len : g_ui_stack_status.reset_len;

                float _fast_len = floor(g_ui_stack_status.reset_len);
                float _nom_len = g_ui_stack_status.reset_len - static_cast<int>(g_ui_stack_status.reset_len);
                int _nom_steps = static_cast<int>(_nom_len / 0.0028);

                tmc2209_init_with(__high_speed_1mmx1);
                for (int i = 0; i < std::fabs(_fast_len); ++i) {
                    g_slide->move((g_ui_stack_status.reset_len>=0)?1:-1);
                }
                tmc2209_init_with(__low_speed_2um8x4);
                for (int i = 0; i < std::fabs(_nom_steps); ++i) {
                    g_slide->move((g_ui_stack_status.reset_len>=0)?0.0224:-0.0224);
                }

                g_ui_stack_status.reset_len = 4000;
                g_ui_stack_status.run = 2;

                ui_stack_status_reset();
                g_ui_stack_status.need_reset = 0;
            }
            do {if (g_ui_stack_status.run == 2) {
                // auto rt = time::ticks_ms();
                // if (rt - _stack_lt < static_cast<decltype(rt)>(g_ui_stack_status.wait_time_ms)) {
                //     break;
                // }
                // _stack_lt = rt;
                // maix::log::info("stack cap start, %d", g_ui_stack_status.shot_steps_cnt);
                if (!__can_move) {
                    break;
                }
                tmc2209_init_with(__low_speed_2um8x4);
                if (pic_cnt >= g_ui_stack_status.shot_number) {
                    pic_cnt = 0;
                    g_ui_stack_status.run = 0;
                    if (g_ui_stack_status.reset_at_end_mode)
                        g_ui_stack_status.need_reset = 1;
                    update_stack_start_btn();
                    reset_stack_start_btn();
                }
                float _a = std::fabs(g_ui_stack_status.move_len / 0.0028 / g_ui_stack_status.shot_number);
                float step_len = g_ui_stack_status.move_len>=0 ? __low_speed_2um8x4.step_len : -__low_speed_2um8x4.step_len;
                // maix::log::info("steps cnt: %f, step: %f", _a, step_len);
                for (int i = 0; i < _a; ++i) {
                    g_slide->move(step_len);
                }
                g_ui_stack_status.reset_len += g_ui_stack_status.shot_steps_cnt*0.0028;
                __fut.get();
                __fut = std::move(std::async(std::launch::async, [&](){
                    // maix::log::info("need wait %d ms, sleep start", g_ui_stack_status.wait_time_ms);
                    // auto _slt = time::ticks_ms();
                    // maix::log::info("sleep lt: %llu", _slt);
                    time::sleep_ms(g_ui_stack_status.wait_time_ms);
                    // auto _srt = time::ticks_ms();
                    // maix::log::info("sleep rt: %llu, %llu", _srt, _srt-_slt);
                    // time::sleep_ms(5);
                    touch_start_pic();
                    pic_cnt++;
                    __can_move = true;
                }));
                __can_move = false;
            }} while(0);
            if (g_ui_stack_status.run == 1) {
                if (!almost_eq(g_ui_stack_status.reset_len, 4000)) {
                    maix::log::info("reset start point and start");
                    g_ui_stack_status.reset_len = -g_ui_stack_status.move_len;

                    float _fast_len = floor(g_ui_stack_status.reset_len);
                    float _nom_len = g_ui_stack_status.reset_len - static_cast<int>(g_ui_stack_status.reset_len);
                    int _nom_steps = std::abs(static_cast<int>(_nom_len / 0.0028));

                    // maix::log::info("fast len: %f, nom len: %f, nom step: %d", _fast_len, _nom_len, _nom_steps);

                    tmc2209_init_with(__high_speed_1mmx1);
                    for (int i = 0; i < std::fabs(floor(_fast_len)); ++i) {
                        g_slide->move((g_ui_stack_status.reset_len>=0)?1:-1);
                    }
                    tmc2209_init_with(__low_speed_2um8x4);
                    for (int i = 0; i < _nom_steps; ++i) {
                        g_slide->move((g_ui_stack_status.reset_len>=0)?0.0224:-0.0224);
                    }
                }
                g_ui_stack_status.reset_len = 0;
                g_ui_stack_status.run = 2;
                __can_move = true;

                memset(_stack_buf, 0x00, std::size(_stack_buf));
                std::time_t now = std::time(nullptr);
                std::tm* local_time = std::localtime(&now);
                int hour = local_time->tm_hour;
                int minute = local_time->tm_min;
                int second = local_time->tm_sec;
                snprintf(_stack_buf, std::size(_stack_buf), "%d_%d_%d", hour, minute, second);
                _stack_pic_cnt = 0;
            }
        } else {
            if (g_ui_stack_status.run) {
                ui_stack_status_reset();
                g_ui_stack_status.run = 0;
            }
        }
    )
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

    static int pic_cnt = 0;

    slide_run_main(pic_cnt);
    shot_tp_main();

    void *frame = NULL;
    bool found_camera_frame = false;
    mmf_frame_info_t f;
    if (priv.camera->width() % 64 != 0) {
        printf("camera width must be multiple of 64!\r\n");
        return -1;
    }

    if (priv.camera->format() != image::Format::FMT_YVU420SP) {
        printf("camera format must be YVU420SP!\r\n");
        return -1;
    }

    int ch = priv.camera->get_channel();
    int res = _mmf_vi_frame_pop(ch, &frame, &f, 40);
    if (res == 0 && frame != NULL) {
        found_camera_frame = true;
    }

    if (found_camera_frame) {
        if (priv.loop_last_frame) {
            _mmf_vi_frame_free(ch, &priv.loop_last_frame);
            priv.loop_last_frame = NULL;
        }

        // Push frame to encoder
        int enc_ch = 1;

        static std::unique_ptr<ImageClarityEvaluationMethod> icem = std::make_unique<EngeryOfGradient>();
        static int focus_x = 0;
        static int focus_y = 0;
        static int focus_x_map = 0;
        static int focus_y_map = 0;
        static MAIX_FOCUS_NAMESPACE::PointMapper focus_point_mapper{640, 480, f.w, f.h};
        static bool auto_focus_start = false;

        auto get_clarity = [&](bool save=false){
            // auto lt = maix::time::ticks_ms();
            _stack_save = save;
            if (save) {
                // memset(_stack_buf, 0x00, std::size(_stack_buf));
                // _stack_dis = std::fabs(g_ui_stack_status.reset_len);
                // _stack_cla = ret_;
                // snprintf(_stack_buf, std::size(_stack_buf), "dis_%0.4f_cla_%0.2f.png", _stack_dis, _stack_cla);
                // maix::log::info("dis = %0.5f, clarity=%0.2f", _stack_dis, _stack_cla);
                return 0.0f;
            }

            cv::Mat cv_img;
            std::unique_ptr<image::Image> grayi;
            {
                image::Format fmt = image::Format::FMT_YVU420SP;
                std::unique_ptr<image::Image> img = std::make_unique<image::Image>(f.w, f.h, fmt, (uint8_t *)f.data, f.len, true);
                int crop_w = (f.w == 640) ? 320 : 640;
                int crop_h = (f.h == 480) ? 240 : 480;
                int start_x = std::max(0, focus_x - crop_w / 2);
                int start_y = std::max(0, focus_y - crop_h / 2);
                if (start_x + crop_w > img->width()) {
                    start_x = img->width() - crop_w;
                }
                if (start_y + crop_h > img->height()) {
                    start_y = img->height() - crop_h;
                }
                /* Unsupport */
                // std::unique_ptr<image::Image> cropi = std::unique_ptr<image::Image>(img->crop(start_x, start_y, crop_w, crop_h));
                // grayi = std::unique_ptr<image::Image>(cropi->to_format(image::Format::FMT_GRAYSCALE));
                std::unique_ptr<image::Image> __tmpi = std::unique_ptr<image::Image>(img->to_format(image::Format::FMT_GRAYSCALE));
                grayi = std::unique_ptr<image::Image>(__tmpi->crop(start_x, start_y, crop_w, crop_h));
                cv_img = cv::Mat(grayi->height(), grayi->width(), CV_8UC1, grayi->data());
            }
            // maix::log::info("get a cvMat");
            auto ret_ = icem->clarity(cv_img, focus_x, focus_y);

            // maix::log::info("cla used: %llu", maix::time::ticks_ms()-lt);
            return ret_;
        };

        /* auto focus */
        TMC2209_EXIST_DO(
            do {
                if (!auto_focus_start && !g_ui_stack_status.run) {
                    if (g_auto_focus == std::nullopt) break;
                    focus_x = g_auto_focus.value().x;
                    focus_y = g_auto_focus.value().y;
                    g_auto_focus = std::nullopt;
                    auto_focus_start = true;
                }

                if (auto_focus_start) {
                    auto_focus_main(get_clarity, auto_focus_start);
                }

            } while (0);
        ) else if (fp5510_exist()) {
            do {
#define FP5510_HILLCLIMBING 1
#define FP5510_FULLSCAN 2
#define FP5510_HILLCLIMBING_FUTURE 3
#define FP5510_HILLCLIMBING_FUTURE_MINI 4
#define FP5510_HILLCLIMBING_FUTURE_MINI_YVUCROP 5
#define FP5510_ONLY_SET_POS_MIN_MAX 6

#define FP5510_FOCUS_TYPE FP5510_HILLCLIMBING_FUTURE_MINI_YVUCROP

                using namespace MAIX_FOCUS_NAMESPACE;
                static maix::ext_dev::fp5510::FP5510 fp5510{};
                static int fp5510_focus_w = 0;
                static int fp5510_focus_h = 0;

#if FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING
                static Box<AutoFocusHillClimbing> fp5510_focus_impl{nullptr};
#elif FP5510_FOCUS_TYPE == FP5510_FULLSCAN
                static Box<FullScan> full_scan_impl{nullptr};
                static int full_scan_cnt = 0;
#elif FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE
                static Box<AutoFocusHCFuture> fp5510_focus_impl{nullptr};
#elif FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE_MINI || FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE_MINI_YVUCROP
                static Box<AutoFocusHCFutureMini> fp5510_focus_impl{nullptr};
#elif FP5510_FOCUS_TYPE == FP5510_ONLY_SET_POS_MIN_MAX
                static int fp5510_pos = 0;
#endif
                /* dbg */
                static uint64_t dbg_focus_total_ltime;
                if (!auto_focus_start) {
                    if (g_auto_focus == std::nullopt) break;
                    bool impl_need_reset = false;
                    auto point = g_auto_focus.value();
                    if (focus_x != point.x || focus_y != point.y) {
                        focus_x = point.x;
                        focus_y = point.y;
#if FP5510_FOCUS_TYPE != FP5510_FULLSCAN
                        std::tie(focus_x_map, focus_y_map) =
                            focus_point_mapper.map_s2d(focus_x, focus_y);
#else
                        std::tie(focus_x_map, focus_y_map) =
                            focus_point_mapper.map_s2d(320, 240);
#endif
                        impl_need_reset = true;
                    }
                    if (f.w <= 640 && f.h <= 480 &&
                        fp5510_focus_w != f.w /2 &&
                        fp5510_focus_h != f.h / 2) {
                        fp5510_focus_w = f.w / 2;
                        fp5510_focus_h = f.h / 2;
                        impl_need_reset = true;
                    // } else if (fp5510_focus_w != 640 && fp5510_focus_h != 480) {
                    //     fp5510_focus_w = 640;
                    //     fp5510_focus_h = 480;
                    //     impl_need_reset = true;
                    // }
                    } else if (fp5510_focus_w != 320 && fp5510_focus_h != 240) {
                        fp5510_focus_w = 320;
                        fp5510_focus_h = 240;
                        impl_need_reset = true;
                    }
                    if (impl_need_reset) {
                        maix::log::info("reset fp5510 focus impl");
#if FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING
                        fp5510_focus_impl.release();
                        fp5510_focus_impl = std::make_unique<AutoFocusHillClimbing>(
                            std::make_unique<CVMatCreater>(
                                std::make_unique<ImageCropper>(
                                    focus_x_map, focus_y_map, fp5510_focus_w, fp5510_focus_h,
                                    std::make_unique<ImageGrayCreater>()
                                )
                            ),
                            std::make_unique<CALaplace>(),
                            [&](int pos) {
                                pos = std::max(0, pos);
                                fp5510.set_pos(pos);
                            }, 0, 1023, -1, -1, 1
                        );
#elif FP5510_FOCUS_TYPE == FP5510_FULLSCAN
                        maix::log::info("full scan %dx%d (%d,%d)",
                            fp5510_focus_w, fp5510_focus_h, focus_x_map, focus_y_map);
                        full_scan_impl.release();
                        full_scan_impl = std::make_unique<FullScan>(
                            std::make_unique<CVMatCreater>(
                                std::make_unique<ImageGrayCreater>(
                                    std::make_unique<ImageYVU420SPNV21Cropper>(
                                        focus_x_map, focus_y_map, fp5510_focus_w, fp5510_focus_h
                                    )
                                )
                            ),
                            std::make_unique<CALaplace>(),
                            [&](int pos) {
                                pos = std::max(0, pos);
                                fp5510.set_pos(pos);
                            }, 0, 1023, 1, 1
                        );
                        full_scan_impl->set_save_path("/root/focus_full_scan");
                        full_scan_cnt = 0;
#elif FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE
                        fp5510_focus_impl.release();
                        fp5510_focus_impl = std::make_unique<AutoFocusHCFuture>(
                            std::make_unique<CVMatCreater>(
                                std::make_unique<ImageGrayCreater>(
                                    std::make_unique<ImageYVU420SPNV21Cropper>(
                                        focus_x_map, focus_y_map, fp5510_focus_w, fp5510_focus_h
                                    )
                                )
                            ),
                            std::make_unique<CALaplace>(),
                            [&](int pos) {
                                pos = std::max(0, pos);
                                fp5510.set_pos(pos);
                            }, 0, 1023, -1, -1, 2
                        );
#elif FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE_MINI
                        fp5510_focus_impl.release();
                        fp5510_focus_impl = std::make_unique<AutoFocusHCFutureMini>(
                            std::make_unique<CVMatCreater>(
                                std::make_unique<ImageCropper>(
                                    focus_x_map, focus_y_map, fp5510_focus_w, fp5510_focus_h,
                                    std::make_unique<ImageGrayCreater>()
                                )
                            ),
                            std::make_unique<CALaplace>(),
                            [&](int pos) {
                                pos = std::max(0, pos);
                                fp5510.set_pos(pos);
                            }, 0, 1023, -1, -1, 2
                        );
#elif FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE_MINI_YVUCROP
                        fp5510_focus_impl.release();
                        fp5510_focus_impl = std::make_unique<AutoFocusHCFutureMini>(
                            std::make_unique<CVMatCreater>(
                                std::make_unique<ImageGrayCreater>(
                                    std::make_unique<ImageYVU420SPNV21Cropper>(
                                        focus_x_map, focus_y_map, fp5510_focus_w, fp5510_focus_h
                                    )
                                )
                            ),
                            std::make_unique<CALaplace>(),
                            [&](int pos) {
                                pos = std::max(0, pos);
                                fp5510.set_pos(pos);
                            }, 0, 1023, -1, -1, 2
                        );
#endif
                    }
                    g_auto_focus = std::nullopt;
                    auto_focus_start = true;
                    dbg_focus_total_ltime = time::ticks_ms();
                }
                if (auto_focus_start) {
                    // auto _create_oimg_lt = time::ticks_ms();
                    image::Format fmt = image::Format::FMT_YVU420SP;
                    ArcBox<image::Image> img = ArcBox<image::Image>(
                            new image::Image(f.w, f.h, fmt, (uint8_t *)f.data, f.len, false)
                    );
                    // maix::log::info("create_oimg used %d", time::ticks_ms()-_create_oimg_lt);

                    // auto dbg_focus_lt = time::ticks_ms();
#if FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING
                    if (fp5510_focus_impl->focus(img) == AutoFocusHillClimbing::AFMode::STOP) {
                        auto_focus_start = false;
                        log::info("FOCUS total used %llu ms", time::ticks_ms()-dbg_focus_total_ltime);
                    }
#elif FP5510_FOCUS_TYPE == FP5510_FULLSCAN
                    full_scan_cnt++;
                    if (full_scan_impl->scan(img) == FullScan::Mode::STOP
                        /* || full_scan_cnt >= 100 */ ) {
                        auto_focus_start = false;
                        full_scan_impl->save("/root/focus_full_scan/info.txt");
                    }
#elif FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE
                    if (fp5510_focus_impl->focus(img) == AutoFocusHCFuture::Mode::STOP) {
                        auto_focus_start = false;
                        log::info("FOCUS total used %llu ms", time::ticks_ms()-dbg_focus_total_ltime);
                    }
#elif FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE_MINI || FP5510_FOCUS_TYPE == FP5510_HILLCLIMBING_FUTURE_MINI_YVUCROP
                    if (fp5510_focus_impl->focus(img) == AutoFocusHCFutureMini::Mode::STOP) {
                        auto_focus_start = false;
                        log::info("FOCUS total used %llu ms", time::ticks_ms()-dbg_focus_total_ltime);
                    }
#elif FP5510_FOCUS_TYPE == FP5510_ONLY_SET_POS_MIN_MAX
                    fp5510.set_pos(fp5510_pos);
                    maix::log::info("now set pos: %d", fp5510_pos);
                    fp5510_pos = fp5510_pos ? 0 : 1023;
                    auto_focus_start = false;
#endif
                    // log::info("focus 1 frame used %d", time::ticks_ms()-dbg_focus_lt);
                }
            } while(0);
        }

        // Snap picture
        if (priv.cam_snap_flag) {
            priv.cam_snap_flag = false;

            image::Format fmt = image::Format::FMT_YVU420SP;
            image::Image *img = new image::Image(f.w, f.h, fmt, (uint8_t *)f.data, f.len, true);
            if (!img) {
                printf("create image failed!\r\n");
                _mmf_vi_frame_free(ch, &frame);
                return -1;
            }

            focus_x = f.w/2;
            focus_y = f.h/2;
            get_clarity(true);
            // get_clarity(false);

            _capture_image(*priv.camera, img);
            delete img;
        }

        bool found_venc_stream = false;

        // Pop stream from encoder
        mmf_stream_t venc_stream = {0};
        if (0 == mmf_venc_pop(enc_ch, &venc_stream)) {
            // for (int i = 0; i < venc_stream.count; i++) {
            //     printf("venc stream[%d]: data:%p size:%d\r\n", i, venc_stream.data[i], venc_stream.data_size[i]);
            // }

            if (venc_stream.count > 0) {
                found_venc_stream = true;
            }
        }

        if (priv.ffmpeg_packer && priv.ffmpeg_packer->is_opened()) {
            double temp_us = priv.ffmpeg_packer->video_pts_to_us(priv.video_pts);
            priv.audio_pts = priv.ffmpeg_packer->audio_us_to_pts(temp_us);
        }

        if (found_venc_stream) {
            if (priv.ffmpeg_packer) {
                if (!priv.ffmpeg_packer->is_opened()) {
                    if (venc_stream.count > 1) {
                        int sps_pps_size = venc_stream.data_size[0] + venc_stream.data_size[1];
                        uint8_t *sps_pps = (uint8_t *)malloc(sps_pps_size);
                        if (sps_pps) {
                            memcpy(sps_pps, venc_stream.data[0], venc_stream.data_size[0]);
                            memcpy(sps_pps + venc_stream.data_size[0], venc_stream.data[1], venc_stream.data_size[1]);

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
            }

            if (priv.ffmpeg_packer->is_opened()) {
                uint8_t *data = NULL;
                int data_size = 0;
                if (venc_stream.count == 1) {
                    data = venc_stream.data[0];
                    data_size = venc_stream.data_size[0];
                } else if (venc_stream.count > 1) {
                    data = venc_stream.data[2];
                    data_size = venc_stream.data_size[2];
                }

                if (data_size) {
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
                    if (err::ERR_NONE != priv.ffmpeg_packer->push(data, data_size, priv.video_pts)) {
                        log::error("ffmpeg push failed!");
                    }
                }
            }
            mmf_venc_free(enc_ch);
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
                mmf_venc_push2(enc_ch, frame);
            } else {
                if (timelapse_record_is_auto()) {
                    mmf_venc_push2(enc_ch, frame);
                } else {
                    if (time::ticks_ms() - priv.last_push_venc_ms > (uint64_t)priv.timelapse_s * 1000) {
                        mmf_venc_push2(enc_ch, frame);
                        priv.last_push_venc_ms = time::ticks_ms();
                    }
                }
            }

            ui_set_record_time(record_time);
        }
        priv.loop_last_frame = frame;
    } else {
        frame = priv.loop_last_frame;
    }

    // Push frame to vo
    mmf_vo_frame_push2(0, 0, 2, frame);

    // Run ui rocess, must run after disp.show
    lv_timer_handler();

    app_loop(*priv.camera, *priv.disp, priv.other_disp);

    if (priv.show_timestamp_enable) {
        uint64_t curr_ms = time::ticks_ms();
        if (curr_ms - priv.last_update_region_ms > 1000) {
            if (priv.region) {
                auto img = priv.region->get_canvas();
                auto datetime = time::now();
                auto str1 = datetime->strftime("%Y/%m/%d %H:%M:%S");
                delete datetime;
                img->draw_string(0, 0, str1, image::COLOR_WHITE);
                priv.region->update_canvas();
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
    ui_all_screen_init();
    ui_camera_config_t ui_camera_cfg;
    ui_camera_config_read(&ui_camera_cfg);
    // auto exptime_range = cam.get_exposure_time_range();
    // ui_camera_cfg.exposure_time_max = exptime_range[0];             // 0.0024fps
    // ui_camera_cfg.exposure_time_min = exptime_range[1];             // 240fps
    ui_camera_config_update(&ui_camera_cfg);
    _ui_update_new_image_from_maix_path();

    usleep(1000 * 1000); // wait sensor init
    uint32_t exposure_time = 0, iso_num = 0;
    mmf_get_exptime_and_iso(0, &exposure_time, &iso_num);
    priv.sensor_shutter_value = exposure_time;
    priv.sensor_iso_value = iso_num;
    priv.sensor_ae_mode = mmf_get_exp_mode(0);
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
                _mmf_set_exp_mode(0, 0);
                priv.sensor_ae_mode = 0;
            }
        } else {
            if (priv.sensor_ae_mode == 0) {
                uint32_t exposure_time = 0, iso_num = 0;
                _mmf_set_exp_mode(0, 1);
                mmf_get_exptime_and_iso(0, &exposure_time, &iso_num);
                priv.sensor_shutter_value = exposure_time;
                priv.sensor_iso_value = iso_num;
                priv.sensor_ae_mode = 1;
            }

            double shutter_value;
            ui_get_shutter_value(&shutter_value);
            printf("Shutter setting: %f s\n", shutter_value);
            if (shutter_value != 0) {
                _mmf_set_exptime_and_iso(0, shutter_value, priv.sensor_iso_value);
            }
            priv.sensor_shutter_value = (uint32_t)shutter_value;
        }
    }

    if (ui_get_iso_setting_flag()) {
        if (ui_get_iso_auto_flag()) {
            printf("ISO setting: Auto\n");
            if (priv.sensor_ae_mode != 0) {
                _mmf_set_exp_mode(0, OP_TYPE_AUTO);
                priv.sensor_ae_mode = 0;
            }
        } else {
            if (priv.sensor_ae_mode == 0) {
                uint32_t exposure_time = 0, iso_num = 0;
                _mmf_set_exp_mode(0, 1);
                mmf_get_exptime_and_iso(0, &exposure_time, &iso_num);
                priv.sensor_shutter_value = exposure_time;
                priv.sensor_iso_value = iso_num;
                priv.sensor_ae_mode = 1;
            }
            int iso_value;
            ui_get_iso_value(&iso_value);
            printf("ISO setting: %d\n", iso_value);
            _mmf_set_exptime_and_iso(0, priv.sensor_shutter_value, iso_value);
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
            maix::image::Image *jpg_img = img->to_format(maix::image::FMT_JPEG);
            if (jpg_img) {
                save_buff_to_file((char *)picture_save_path.c_str(), (uint8_t *)jpg_img->data(), jpg_img->data_size());
                delete jpg_img;

                image::Image *load_img = image::load((char *)picture_save_path.c_str());
                maix::image::Image *thumbnail_img = load_img->resize(128, 128, image::Fit::FIT_COVER);
                thumbnail_img->save(thumbnail_path.c_str());
                if (_stack_save) {
                    _stack_pic_cnt++;
                    std::filesystem::path stack_save_path = picture_path;
                    stack_save_path = stack_save_path/std::string(_stack_buf);
                    if (!std::filesystem::exists(stack_save_path)) {
                        std::filesystem::create_directory(stack_save_path);
                        maix::log::info("create stack save path: %s", stack_save_path.c_str());
                    }
                    stack_save_path = stack_save_path/std::to_string(_stack_pic_cnt).append(".jpg");
                    try {
                        std::filesystem::copy_file(picture_save_path, stack_save_path, std::filesystem::copy_options::overwrite_existing);
                    } catch (...) {}
                }
                delete thumbnail_img;
                delete load_img;

                system("sync");

                printf("update small and big img\n");
                maix::image::Image *new_img = maix::image::load((char *)picture_save_path.c_str(), maix::image::FMT_RGB888);
                if (!g_ui_stack_status.run)
                    ui_anim_run_save_img();
                _ui_update_pic_img(new_img);
                delete new_img;
            } else {
                printf("encode nv21 to jpg failed!\n");
            }
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

    uint32_t exposure_time = 0, iso_num = 0;
    mmf_get_exptime_and_iso(0, &exposure_time, &iso_num);
    // log::info("exp:%d iso:%d", exposure_time, iso_num);
    // if (ui_get_shutter_auto_flag()) {
    //     ui_set_shutter_value((double)exposure_time);
    // }
    // if (ui_get_iso_auto_flag()) {
    //     ui_set_iso_value(iso_num);
    // }

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