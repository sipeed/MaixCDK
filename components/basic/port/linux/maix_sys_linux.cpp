#include "maix_util.hpp"
#include "maix_app.hpp"
#include "signal.h"

namespace maix::sys
{
    static void signal_handle(int signal)
    {
        switch (signal) {
        case SIGINT:
            maix::app::set_exit_flag(true);
#if CONFIG_BUILD_WITH_MAIXPY
            raise(SIGINT);
#endif
        break;
        default: break;
        }
    }

    void register_default_signal_handle() {
        signal(SIGINT, signal_handle);
    }
}
