/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.9.4: Add framework, create this file.
 */


#include "maix_hid.hpp"
#include "maix_basic.hpp"
#include "cstdio"
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#define DEV_PATH "/dev/hidg%d"

namespace maix::peripheral::hid
{
    Hid::Hid(hid::DeviceType device_type, bool open)
    {

    }

    Hid::~Hid()
    {
    }

    err::Err Hid::open() {
        return err::ERR_NOT_IMPL;
    }

    err::Err Hid::close() {
        return err::ERR_NOT_IMPL;
    }

    err::Err Hid::write(std::vector<int> &data) {
        return err::ERR_NOT_IMPL;
    }
};
