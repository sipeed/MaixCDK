#include "maix_basic.hpp"

#if PLATFORM_MAIXCAM2

#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "ax_isp_3a_api.h"
#include "ax_vin_api.h"

#include <cstdlib>
#include <string>

using namespace maix;

static void print_vin_status(int channel)
{
    AX_VIN_DEV_STATUS_T dev_status = {};
    AX_VIN_PIPE_STATUS_T pipe_status = {};
    AX_VIN_CHN_STATUS_T chn_status = {};

    int dev_ret = AX_VIN_QueryDevStatus(0, &dev_status);
    int pipe_ret = AX_VIN_QueryPipeStatus(0, &pipe_status);
    int chn_ret = AX_VIN_QueryChnStatus(0, static_cast<AX_VIN_CHN_ID_E>(channel), &chn_status);

    if (dev_ret == 0) {
        log::info("VIN dev: enabled=%d fps=%.2f seq=%llu lost=%u vb_fail=%u",
                  dev_status.bEnable, dev_status.fFrameRate,
                  static_cast<unsigned long long>(dev_status.nFrameSeqNum),
                  dev_status.nLostFrameCnt, dev_status.nVbFailCnt);
    } else {
        log::error("AX_VIN_QueryDevStatus failed: 0x%x", dev_ret);
    }

    if (pipe_ret == 0) {
        log::info("VIN pipe: enabled=%d fps=%.2f seq=%llu lost=%u vb_fail=%u",
                  pipe_status.bEnable, pipe_status.fFrameRate,
                  static_cast<unsigned long long>(pipe_status.nFrameSeqNum),
                  pipe_status.nLostFrameCnt, pipe_status.nVbFailCnt);
    } else {
        log::error("AX_VIN_QueryPipeStatus failed: 0x%x", pipe_ret);
    }

    if (chn_ret == 0) {
        log::info("VIN ch%d: enabled=%d fps=%.2f seq=%llu lost=%u vb_fail=%u",
                  channel, chn_status.bEnable, chn_status.fFrameRate,
                  static_cast<unsigned long long>(chn_status.nFrameSeqNum),
                  chn_status.nLostFrameCnt, chn_status.nVbFailCnt);
    } else {
        log::error("AX_VIN_QueryChnStatus failed: 0x%x", chn_ret);
    }
}

static void print_ae_limits()
{
    AX_ISP_IQ_EXP_HW_LIMIT_T limits = {};
    int ret = AX_ISP_IQ_GetAeHwLimit(0, &limits);
    if (ret != 0) {
        log::error("AX_ISP_IQ_GetAeHwLimit failed: 0x%x", ret);
        return;
    }

    log::info("AE limits: shutter=%u..%u us slow_shutter=%u..%u us",
              limits.tSnsShutterLimit.nMin, limits.tSnsShutterLimit.nMax,
              limits.tSnsSlowShutterModeShutterLimit.nMin,
              limits.tSnsSlowShutterModeShutterLimit.nMax);
    log::info("AE gain limits: again=%.2f..%.2fx dgain=%.2f..%.2fx isp=%.2f..%.2fx total=%.2f..%.2fx",
              limits.tSnsAgainLimit.nMin / 1024.0, limits.tSnsAgainLimit.nMax / 1024.0,
              limits.tSnsDgainLimit.nMin / 1024.0, limits.tSnsDgainLimit.nMax / 1024.0,
              limits.tIspDgainLimit.nMin / 1024.0, limits.tIspDgainLimit.nMax / 1024.0,
              limits.nTotalGainMin / 1024.0, limits.nTotalGainMax / 1024.0);
}

static void print_ae_status(double image_luma)
{
    AX_ISP_IQ_AE_STATUS_T status = {};
    AX_ISP_IQ_AE_PARAM_T param = {};
    int ret = AX_ISP_IQ_GetAeStatus(0, &status);
    if (ret != 0) {
        log::error("AX_ISP_IQ_GetAeStatus failed: 0x%x", ret);
        return;
    }

    const AX_ISP_IQ_EXP_SETTING_T &exp = status.tExpStatus;
    const AX_ISP_IQ_AE_ALG_STATUS_T &alg = status.tAlgStatus;
    ret = AX_ISP_IQ_GetAeParam(0, &param);
    if (ret == 0) {
        const AX_ISP_IQ_AE_ALG_CONFIG_T &config = param.tAeAlgAuto;
        log::info("AE mode: %s manual_shutter=%u us manual_again=%.2fx manual_total=%.2fx",
                  param.nEnable ? "auto" : "manual", param.tExpManual.nShutter,
                  param.tExpManual.nAGain / 1024.0, param.tExpManual.nSysTotalGain / 1024.0);
        log::info("AE config: strategy=%u compensation=%u shutter=%u..%u us again=%.2f..%.2fx dgain=%.2f..%.2fx isp=%.2f..%.2fx sys=%.2f..%.2fx",
                  config.nStrategyMode, config.nCompensationMode,
                  config.nMinShutter, config.nMaxShutter,
                  config.nMinUserTotalAgain / 1024.0, config.nMaxUserTotalAgain / 1024.0,
                  config.nMinUserDgain / 1024.0, config.nMaxUserDgain / 1024.0,
                  config.nMinIspGain / 1024.0, config.nMaxIspGain / 1024.0,
                  config.nMinUserSysGain / 1024.0, config.nMaxUserSysGain / 1024.0);
    }
    log::info("AE actual: shutter=%u us again=%.2fx dgain=%.2fx isp=%.2fx total=%.2fx fps=%.2f stable=%u",
              exp.nShutter, exp.nAGain / 1024.0, exp.nDgain / 1024.0,
              exp.nIspGain / 1024.0, exp.nSysTotalGain / 1024.0,
              alg.nFps / 1024.0, alg.nAeStable);
    log::info("AE luma: mean=%.1f weighted=%.1f target=%.1f sampled_image=%.1f",
              alg.nMeanLuma / 1024.0, alg.nWeightedMeanLuma / 1024.0,
              alg.nFrameTarget / 1024.0, image_luma);
}

static double measure_luma(image::Image &img)
{
    const uint8_t *data = static_cast<const uint8_t *>(img.data());
    uint64_t sum = 0;
    uint64_t count = 0;

    if (img.format() == image::Format::FMT_YVU420SP) {
        int pixels = img.width() * img.height();
        for (int i = 0; i < pixels; i += 16) {
            sum += data[i];
            ++count;
        }
    } else if (img.format() == image::Format::FMT_RGB888) {
        int pixels = img.width() * img.height();
        for (int i = 0; i < pixels; i += 16) {
            int offset = i * 3;
            sum += (77 * data[offset] + 150 * data[offset + 1] + 29 * data[offset + 2]) >> 8;
            ++count;
        }
    }

    return count ? static_cast<double>(sum) / count : -1.0;
}

static int run_test(int argc, char *argv[])
{
    uint64_t duration_ms = 10000;
    if (argc > 1) {
        long seconds = std::strtol(argv[1], nullptr, 10);
        if (seconds > 0) {
            duration_ms = static_cast<uint64_t>(seconds) * 1000;
        }
    }

    bool rgb888 = argc > 2 && std::string(argv[2]) == "rgb";
    bool use_display = argc > 3 && std::string(argv[3]) == "display";
    int manual_exposure_us = argc > 4 ? std::strtol(argv[4], nullptr, 10) : 0;
    int manual_again = argc > 5 ? std::strtol(argv[5], nullptr, 10) : 0;
    image::Format format = rgb888 ? image::Format::FMT_RGB888 : image::Format::FMT_YVU420SP;
    log::info("Opening OS04D10 test mode: 640x360 %s @ 120 FPS, display=%s, AI-ISP disabled",
              rgb888 ? "RGB888" : "NV21", use_display ? "on" : "off");
    camera::Camera cam(640, 360, format, "", 120, 4, true, false);
    display::Display *disp = use_display ? new display::Display() : nullptr;
    log::info("Camera accepted: %dx%d @ %.2f FPS, channel=%d, buffers=%d",
              cam.width(), cam.height(), cam.fps(), cam.get_channel(), cam.buff_num());
    if (disp) {
        log::info("Display opened: %dx%d %s", disp->width(), disp->height(),
                  image::format_name(disp->format()).c_str());
    }
    if (manual_exposure_us > 0) {
        log::info("Set manual exposure: requested=%d us result=%d us",
                  manual_exposure_us, cam.exposure(manual_exposure_us));
    }
    if (manual_again > 0) {
        log::info("Set manual analog gain: requested=%d/1024 result_total=%d/1024",
                  manual_again, cam.gain(manual_again));
    }

    print_vin_status(cam.get_channel());
    print_ae_limits();

    uint64_t start_ms = time::ticks_ms();
    uint64_t next_report_ms = start_ms + 1000;
    uint64_t frame_count = 0;
    uint64_t timeout_count = 0;
    uint64_t display_count = 0;
    uint64_t display_fail_count = 0;
    uint64_t display_time_us = 0;
    double image_luma = -1.0;

    while (!app::need_exit() && time::ticks_ms() - start_ms < duration_ms) {
        image::Image *img = cam.read(true, 1000);
        if (img) {
            if (frame_count == 0) {
                log::info("First frame: %dx%d %s, data=%d bytes", img->width(), img->height(),
                          image::format_name(img->format()).c_str(), img->data_size());
            }
            if (frame_count % 30 == 0) {
                image_luma = measure_luma(*img);
            }
            if (disp) {
                uint64_t show_start_us = time::ticks_us();
                err::Err show_ret = disp->show(*img, image::FIT_CONTAIN);
                display_time_us += time::ticks_us() - show_start_us;
                if (show_ret == err::ERR_NONE) {
                    ++display_count;
                } else {
                    ++display_fail_count;
                }
            }
            ++frame_count;
            delete img;
        } else {
            ++timeout_count;
        }

        uint64_t now_ms = time::ticks_ms();
        if (now_ms >= next_report_ms) {
            double read_fps = frame_count * 1000.0 / (now_ms - start_ms);
            log::info("Read progress: frames=%llu timeouts=%llu average=%.2f FPS",
                      static_cast<unsigned long long>(frame_count),
                      static_cast<unsigned long long>(timeout_count), read_fps);
            if (disp) {
                log::info("Display progress: shown=%llu failed=%llu average_show=%.3f ms",
                          static_cast<unsigned long long>(display_count),
                          static_cast<unsigned long long>(display_fail_count),
                          display_count ? display_time_us / display_count / 1000.0 : 0.0);
            }
            print_vin_status(cam.get_channel());
            print_ae_status(image_luma);
            next_report_ms = now_ms + 1000;
        }
    }

    uint64_t elapsed_ms = time::ticks_ms() - start_ms;
    double read_fps = elapsed_ms ? frame_count * 1000.0 / elapsed_ms : 0.0;
    log::info("FINAL: frames=%llu elapsed=%llu ms timeouts=%llu measured_read_fps=%.2f",
              static_cast<unsigned long long>(frame_count),
              static_cast<unsigned long long>(elapsed_ms),
              static_cast<unsigned long long>(timeout_count), read_fps);
    if (disp) {
        log::info("DISPLAY_FINAL: shown=%llu failed=%llu measured_loop_fps=%.2f average_show=%.3f ms",
                  static_cast<unsigned long long>(display_count),
                  static_cast<unsigned long long>(display_fail_count),
                  elapsed_ms ? display_count * 1000.0 / elapsed_ms : 0.0,
                  display_count ? display_time_us / display_count / 1000.0 : 0.0);
        delete disp;
    }
    print_vin_status(cam.get_channel());
    print_ae_status(image_luma);
    return frame_count ? 0 : -1;
}

int main(int argc, char *argv[])
{
    sys::register_default_signal_handle();
    CATCH_EXCEPTION_RUN_RETURN(run_test, -1, argc, argv);
}

#else

int main()
{
    maix::log::warn("os04d10_120_test only supports MaixCAM2");
    return 0;
}

#endif
