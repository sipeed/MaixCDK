#include "maix_util.hpp"

namespace maix::util
{
    void disable_kernel_debug() {

    }

    void enable_kernel_debug() {

    }

    void register_exit_function(void (*process)(void)) {
        (void)process;
    }

    void do_exit_function() {
    }

    void register_atexit() {
        atexit(do_exit_function);
    }
}

