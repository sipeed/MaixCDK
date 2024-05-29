#include "maix_util.hpp"
#include "maix_app.hpp"
#include "signal.h"

namespace maix::sys
{
    static void signal_handle(int signal)
    {
        const char *signal_msg = NULL;
        switch (signal) {
        case SIGINT:
            maix::app::set_exit_flag(true);
            raise(SIGINT);
        break;
        default: signal_msg = "UNKNOWN"; break;
        }
    }

    void register_default_signal_handle() {
        signal(SIGILL, signal_handle);
    }
}
