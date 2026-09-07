#include "maix_basic.hpp"
#include "maix_camera.hpp"

#include <cstdlib>
#include <fstream>
#include <string>

using namespace maix;

static int run_test(int argc, char *argv[])
{
    const int x = argc > 1 ? std::strtol(argv[1], nullptr, 10) : 0;
    const int y = argc > 2 ? std::strtol(argv[2], nullptr, 10) : 4;
    const int crop_width = argc > 3 ? std::strtol(argv[3], nullptr, 10) : 1344;
    const int crop_height = argc > 4 ? std::strtol(argv[4], nullptr, 10) : 760;
    const int fps = argc > 5 ? std::strtol(argv[5], nullptr, 10) : -1;
    const int seconds = argc > 6 ? std::strtol(argv[6], nullptr, 10) : 10;
    const bool apply_after_open = argc > 7 && std::strtol(argv[7], nullptr, 10) != 0;
    const std::string snapshot_path = argc > 8 ? argv[8] : std::string();
    const int output_width = argc > 9 ? std::strtol(argv[9], nullptr, 10) : 640;
    const int output_height = argc > 10 ? std::strtol(argv[10], nullptr, 10) : 360;
    const std::string raw_path = argc > 11 ? argv[11] : std::string();

    // Enable the RAW path when requested so we can distinguish sensor/VI data
    // from an ISP-only rendering problem during high-speed crop validation.
    camera::Camera cam(output_width, output_height, image::FMT_YVU420SP, nullptr, fps, 3,
                       false, !raw_path.empty());
    err::Err ret = err::ERR_NONE;
    if (apply_after_open) {
        ret = cam.open();
        if (ret != err::ERR_NONE) {
            log::error("initial open camera failed: %s", err::to_str(ret).c_str());
            return -1;
        }
    }
    if (crop_width > 0 && crop_height > 0) {
        ret = cam.set_windowing({x, y, crop_width, crop_height});
        if (ret != err::ERR_NONE) {
            log::error("set sensor crop failed: %s", err::to_str(ret).c_str());
            return -1;
        }
    }
    if (!cam.is_opened()) {
        ret = cam.open();
        if (ret != err::ERR_NONE) {
            log::error("open camera failed: %s", err::to_str(ret).c_str());
            return -1;
        }
    }

    log::info("camera output=%dx%d fps=%.2f sensor_crop=[%d,%d,%d,%d]",
              cam.width(), cam.height(), cam.fps(), x, y, crop_width, crop_height);
    const std::vector<int> sensor_size = cam.get_sensor_size();
    log::info("active sensor size=%dx%d", sensor_size[0], sensor_size[1]);
    log::info("sensor window regs: start=%02x%02x,%02x%02x end=%02x%02x,%02x%02x output=%02x%02x,%02x%02x vts=%02x%02x",
              cam.read_reg(0x3800), cam.read_reg(0x3801), cam.read_reg(0x3802), cam.read_reg(0x3803),
              cam.read_reg(0x3804), cam.read_reg(0x3805), cam.read_reg(0x3806), cam.read_reg(0x3807),
              cam.read_reg(0x3808), cam.read_reg(0x3809), cam.read_reg(0x380a), cam.read_reg(0x380b),
              cam.read_reg(0x380e), cam.read_reg(0x380f));
    log::info("sensor timing/binning regs: pll0305=%02x pll0325=%02x inc=%02x/%02x/%02x/%02x format=%02x/%02x",
              cam.read_reg(0x0305), cam.read_reg(0x0325), cam.read_reg(0x3814), cam.read_reg(0x3815),
              cam.read_reg(0x3816), cam.read_reg(0x3817), cam.read_reg(0x3820), cam.read_reg(0x3821));

    uint64_t frames = 0;
    uint64_t failures = 0;
    bool snapshot_saved = false;
    // Do not charge pipeline startup latency to the steady-state frame rate.
    cam.skip_frames(30);
    if (!raw_path.empty()) {
        image::Image *raw = cam.read_raw();
        if (!raw) {
            log::error("read raw frame failed");
            return -1;
        }
        std::ofstream out(raw_path, std::ios::binary);
        if (!out) {
            log::error("open raw output failed: %s", raw_path.c_str());
            delete raw;
            return -1;
        }
        out.write(reinterpret_cast<const char *>(raw->data()), raw->data_size());
        log::info("saved raw frame to %s width=%d height=%d size=%d format=%s",
                  raw_path.c_str(), raw->width(), raw->height(), raw->data_size(),
                  image::fmt_names[raw->format()].c_str());
        delete raw;
    }
    const uint64_t start = time::ticks_ms();
    while (time::ticks_ms() - start < static_cast<uint64_t>(seconds) * 1000) {
        image::Image *img = cam.read(true, 1000);
        if (img) {
            ++frames;
            if (!snapshot_saved && !snapshot_path.empty()) {
                img->save(snapshot_path.c_str());
                snapshot_saved = true;
            }
            delete img;
        } else {
            ++failures;
        }
    }
    const uint64_t elapsed = time::ticks_ms() - start;
    log::info("result frames=%llu failures=%llu elapsed_ms=%llu measured_fps=%.2f",
              static_cast<unsigned long long>(frames), static_cast<unsigned long long>(failures),
              static_cast<unsigned long long>(elapsed), frames * 1000.0 / elapsed);
    return failures == 0 && frames > 0 ? 0 : -1;
}

int main(int argc, char *argv[])
{
    CATCH_EXCEPTION_RUN_RETURN(run_test, -1, argc, argv);
}
