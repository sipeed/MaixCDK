
#include "maix_basic.hpp"
#include "main.h"

#include "maix_mlx90640.hpp"
#include "maix_display.hpp"

using namespace maix;
using namespace maix::ext_dev::mlx90640;

int _main(int argc, char* argv[])
{
    auto disp = display::Display();

    auto mlx = MLX90640Celsius(5, FPS::FPS_32, maix::ext_dev::cmap::Cmap::IRONBOW, 5, 50);

    while (true) {
        auto img = mlx.image();
        disp.show(*img);
        delete img;

        auto [max_x, max_y, max] = mlx.max_temp_point();
        log::info("max temp %0.2f in (%d, %d)", max, max_x, max_y);
        auto fps = maix::time::fps();
        log::info("fps: %0.2f", fps);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    // sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


