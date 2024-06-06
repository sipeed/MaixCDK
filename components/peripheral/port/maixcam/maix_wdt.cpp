/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_wdt.hpp"
#include "maix_log.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

#define DEV_PATH "/dev/watchdog"

namespace maix::peripheral::wdt
{
    static int _wdt_init(int feed_time)
    {
        int fd = -1;

        fd = open(DEV_PATH, O_RDWR);
        if (fd < 0) {
            log::error("open %s fialed\r\n", DEV_PATH);
            return -1;
        }

        if (ioctl(fd, WDIOC_SETTIMEOUT, &feed_time) < 0) {
            log::error("watchdog set timeout error\n");
            close(fd);
            return -1;
        }

        // if (ioctl(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD) < 0) {
        //     log::error("watchdog enable error\n");perror("watchdog enable error");
        //     close(fd);
        //     return -1;
        // }

        if (close(fd) < 0) {
            log::error("close %s failed\n", DEV_PATH);
            return -1;
        }
        return 0;
    }

    WDT::WDT(int id, int feed_ms)
    {
        int feed_s;
        if (id != 0) {
            log::error("wdt id %d is not supported, you should set id = 0\r\n", id);
            return;
        }

        feed_s = feed_ms / 1000;
        if (0 < _wdt_init(feed_s)) {
            log::error("wdt init failed\r\n");
            return;
        }

        log::debug("set wdt feed time to %d s\r\n", feed_s);
    }

    WDT::~WDT()
    {
    }

    int WDT::feed()
    {
        printf("WDT::feed()\r\n");
        int res;
        int fd = -1;

        fd = open(DEV_PATH, O_RDWR);
        if (fd < 0)
        {
            log::error("open %s failed\n", DEV_PATH);
            return -1;
        }

        res = ioctl(fd, WDIOC_KEEPALIVE, 0);
        if (res < 0)
        {
            log::error("watchdog feed error\n");
            close(fd);
            return -1;
        }

        res = close(fd);
        if (res < 0)
        {
            log::error("close %s failed\n", DEV_PATH);
            return -1;   
        }
        return 0;
    }

    int WDT::stop()
    {
        log::warn("this operation is not supported\r\n");
        return 0;
    }

    int WDT::restart()
    {
        log::warn("this operation is not supported\r\n");
        return 0;
    }
}; // namespace maix

