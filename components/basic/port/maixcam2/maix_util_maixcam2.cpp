#include "maix_util.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

namespace maix::util
{
    void disable_kernel_debug() {
        char name[] = "/dev/tty1";
        int fd = open(name, O_RDONLY | O_WRONLY);
        if (fd < 0) {
            log::warn("open %s failed!\r\n", name);
            return;
        }
        if (0 < ioctl(fd, TIOCCONS)) {
            log::warn("ioctl(fd, TIOCCONS) failed!\r\n");
            return;
        }

        close(fd);

        system("echo 0 > /proc/sys/kernel/printk");
    }

    void enable_kernel_debug() {
        char name[] = "/dev/console";
        int fd = open(name, O_RDONLY | O_WRONLY);
        if (fd < 0) {
            log::warn("open %s failed!\r\n", name);
            return;
        }
        if (0 < ioctl(fd, TIOCCONS)) {
            log::warn("ioctl(fd, TIOCCONS) failed!\r\n");
            return;
        }

        close(fd);

        system("echo 9 > /proc/sys/kernel/printk");
    }

    static std::vector<void(*)()> *exit_function_list;

    void register_exit_function(void (*process)(void)) {
        if (exit_function_list == nullptr) {
            exit_function_list = new std::vector<void(*)()>;
        }
        exit_function_list->push_back(process);
    }

    void do_exit_function() {
        if (exit_function_list) {
            for (auto& func : *exit_function_list) {
                func();
            }
        }
    }

    void register_atexit() {
        atexit(do_exit_function);
    }
}

