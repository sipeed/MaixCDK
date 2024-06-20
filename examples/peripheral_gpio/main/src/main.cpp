#include "maix_basic.hpp"
#include "maix_gpio.hpp"
#include "maix_pinmap.hpp"
#include "main.h"

using namespace maix;
using namespace maix::peripheral;

int _main(int argc, char* argv[])
{
    log::info("Program start");

    log::info("Toggle_led");
    pinmap::set_pin_function("A26", "GPIOA26");
    gpio::GPIO gpio_14("GPIOA26", gpio::Mode::OUT, gpio::Pull::PULL_NONE);
    // Run until app want to exit, for example app::switch_app API will set exit flag.
    // And you can also call app::set_exit_flag(true) to mark exit.
    while(!app::need_exit())
    {
        time::sleep(1);
        gpio_14.toggle();
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
