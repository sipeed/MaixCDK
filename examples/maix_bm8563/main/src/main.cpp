
#include "maix_basic.hpp"
#include "main.h"
#include "maix_bm8563.hpp"

using namespace maix;

int _main(int argc, char* argv[])
{
    auto bm8653 = ext_dev::bm8563::BM8563();

    // bm8653.systohc();

    // std::vector<int> t{2020, 12, 31, 23, 59, 45};
    // bm8653.datetime(t);

    while (!app::need_exit()) {
        time::sleep_ms(1000);
        auto rt = bm8653.datetime();
        if (rt.empty()) {
            maix::log::info("read error");
            continue;
        }
        log::info("bm8653: %d-%d-%d %d:%d:%d\n\n", rt[0], rt[1], rt[2], rt[3], rt[4], rt[5]);
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


