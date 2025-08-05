
#include "maix_basic.hpp"
#include "main.h"
#include "maix_fp5510.hpp"

using namespace maix;
using namespace maix::ext_dev;

void helper()
{
    log::info("cmd:\r\n"
    "./maix_fp5510 <value> (the range of value is [0, 1023])");
}

int _main(int argc, char* argv[])
{
    if (argc < 2) {
        helper();
        return 0;
    }

    int pos = atoi(argv[1]);
    auto dev = fp5510::FP5510();

    log::info(" set fp5510 position: %d", pos);
    dev.set_pos(pos);
    log::info(" get fp5510 position: %d", dev.get_pos());

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


