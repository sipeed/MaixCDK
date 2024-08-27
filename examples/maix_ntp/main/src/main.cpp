
#include "maix_basic.hpp"
#include "main.h"
#include "maix_ntp.hpp"

using namespace maix;

int _main(int argc, char* argv[])
{
    // auto t = maix::ext_dev::ntp::time_with_config("./ntp_config.yaml");
    auto t = maix::ext_dev::ntp::sync_sys_time("ntp.tencent.com");
    if (t.empty()) return -1;

    maix::log::info("\tNTP response : [ %04d-%02d-%02d %02d:%02d:%02d ]\n",
                       t[0],
                       t[1],
                       t[2],
                       t[3],
                       t[4],
                       t[5]);


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


