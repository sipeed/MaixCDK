
#include "maix_basic.hpp"
#include "main.h"

#include "maix_display.hpp"
#include "maix_tof100.hpp"
#include "maix_pinmap.hpp"

using namespace maix;
using namespace maix::display;
using namespace maix::ext_dev::tof100;
using namespace maix::peripheral::pinmap;
using namespace maix::ext_dev::cmap;

static const std::vector<std::pair<std::string, std::string>> pins = {
    {"A24", "SPI4_CS"},
    {"A23", "SPI4_MISO"},
    {"A25", "SPI4_MOSI"},
    {"A22", "SPI4_SCK"},
};

int _main(int argc, char* argv[])
{

    for (auto& i : pins) {
        if (set_pin_function(i.first, i.second) != maix::err::Err::ERR_NONE) {
            log::info("%s --> %s failed!", i.first.c_str(), i.second.c_str());
            return -1;
        }
    }

    auto disp = Display();

    auto tof = Tof100(4, Resolution::RES_100x100, Cmap::JET, 40, 1000);

    while (!app::need_exit()) {
        auto img = tof.image();
        if (img == nullptr)
            continue;
        disp.show(*img);
        delete img;

        auto fps = maix::time::fps();

        auto [minx, miny, mindis] = tof.min_dis_point();
        log::info("min (%d, %d) = %u", minx, miny, mindis);

        auto [maxx, maxy, maxdis] = tof.max_dis_point();
        log::info("max (%d, %d) = %u", maxx, maxy, maxdis);

        auto [cx, cy, cdis] = tof.center_point();
        log::info("center (%d, %d) = %u, fps: %0.2f", cx, cy, cdis, fps);

        // auto matrix = tof.matrix();

        // auto [minx_, miny_, mindis_] = tof.min_dis_point_from(matrix);
        // log::info("min (%d, %d) = %u", minx_, miny_, mindis_);

        // auto [maxx_, maxy_, maxdis_] = tof.max_dis_point_from(matrix);
        // log::info("max (%d, %d) = %u", maxx_, maxy_, maxdis_);

        // auto [cx_, cy_, cdis_] = tof.center_point_from(matrix);
        // log::info("center (%d, %d) = %u", cx_, cy_, cdis_);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


