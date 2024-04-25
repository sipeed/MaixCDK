#include "maix_util.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

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
}

