
#include "maix_basic.hpp"
#include "main.h"
#include "maix_touchscreen.hpp"
// #include "maix_comm.hpp"
#include "maix_key.hpp"


using namespace maix;

int _main(int argc, char* argv[])
{
    log::info("Program start");
    auto ts = touchscreen::TouchScreen();


    // Run until app want to exit, for example app::switch_app API will set exit flag.
    // And you can also call app::set_exit_flag(true) to mark exit.
    while(!app::need_exit())
    {
        auto info = ts.read();
        log::info("x: %d, y: %d, pressed: %d", info[0], info[1], info[2]);
        time::sleep_ms(10);
    }
    log::info("Program exit");

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // // support default maix communication protol commands
    // comm::add_default_comm_listener();

    default key action
    peripheral::key::add_default_listener();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


