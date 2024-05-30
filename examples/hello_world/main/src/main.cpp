
#include "maix_basic.hpp"
#include "main.h"

using namespace maix;

void hello()
{
    log::info("Hello World!");
    log::info("git version: %d.%d.%d-%d-%s%s",
        BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_MICRO, BUILD_VERSION_DEV,
        BUILD_GIT_COMMIT_ID, BUILD_GIT_IS_DIRTY ? "-dirty" : "");
    log::info("%s version", DEBUG ? "Debug" : "Release");
    log::info("var from componet -> basic's `maix::app::app_id`: %s", app::app_id().c_str());
}

int _main(int argc, char* argv[])
{
    uint64_t t = time::time_s();
    log::info("Program start");

    hello();

    // Run until app want to exit, for example app::switch_app API will set exit flag.
    // And you can also call app::set_exit_flag(true) to mark exit.
    while(!app::need_exit())
    {
        log::info("%d", time::time_s());
        time::sleep(1);

        if(time::time_s() - t > 10)
            app::set_exit_flag(true);
    }
    log::info("Program exit");

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


