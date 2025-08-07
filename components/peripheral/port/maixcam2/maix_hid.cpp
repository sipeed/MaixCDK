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
    static int get_hid_device_idx(hid::DeviceType device_type)
    {
        int find_mouse = fs::exists("/boot/usb.mouse") ? 1 : 0;
        int find_touchpad = fs::exists("/boot/usb.touchpad") ? 1 : 0;
        int find_keyboard = fs::exists("/boot/usb.keyboard") ? 1 : 0;
        int id = -1;

        switch (device_type) {
        case DEVICE_KEYBOARD:
        {
            if (find_keyboard) {
                id = 0;
            }
            break;
        }
        case DEVICE_MOUSE:
        {
            if (find_keyboard && find_mouse) {
                id = 1;
            } else if (!find_keyboard && find_mouse) {
                id = 0;
            }
            break;
        }
        case DEVICE_TOUCHPAD:
        {
            if ((find_keyboard + find_mouse == 2) && find_touchpad) {
                id = 2;
            } else if ((find_keyboard + find_mouse == 1) && find_touchpad) {
                id = 1;
            } else if (!(find_keyboard + find_mouse) && find_touchpad) {
                id = 0;
            }
            break;
        }
        default: err::check_raise(err::ERR_ARGS, "device type not support");
        }
        err::check_bool_raise(id >= 0, "Can not find device idx");
        return id;
    }

    Hid::Hid(hid::DeviceType device_type, bool open)
    {
        _fd = -1;
        _device_type = device_type;
        _is_opened = false;

        if (open)
        {
            err::check_raise(this->open(), "open hid failed");
        }
    }

    Hid::~Hid()
    {
        this->close();
    }

    err::Err Hid::open() {
        if (_fd > 0) {
            return err::ERR_NONE;
        }

        char dev_path[256];
        int type_id = get_hid_device_idx(_device_type);
        snprintf(dev_path, sizeof(dev_path), DEV_PATH, type_id);

        // open hid devicce
        _fd = ::open(dev_path, O_RDWR | O_NONBLOCK | O_NOCTTY);
        err::check_bool_raise(_fd > 0, "open hid failed");
        _is_opened = true;
        return err::ERR_NONE;
    }

    err::Err Hid::close() {
        if (!this->is_opened())
            return err::ERR_NONE;

        if (_fd <= 0)
            return err::ERR_NONE;

        ::close(_fd);
        _fd = -1;
        _is_opened = false;
        return err::ERR_NONE;
    }

    err::Err Hid::write(std::vector<int> &data) {
        err::check_bool_raise(this->is_opened(), "hid device is not opened");

        switch (_device_type) {
            case DEVICE_KEYBOARD:
            {
                err::check_bool_raise(data.size() >= 8, "data too short, you need press at least 8 bytes for keyboard device");
                break;
            }
            case DEVICE_MOUSE:
            {
                err::check_bool_raise(data.size() >= 4, "data too short, you need press at least 4 bytes for keyboard device");
                break;
            }
            case DEVICE_TOUCHPAD:
            {
                err::check_bool_raise(data.size() >= 6, "data too short, you need press at least 6 bytes for keyboard device");
                uint16_t x = (data[1] & 0xff) | (((uint16_t)data[2] << 8) & 0xff00);
                uint16_t y = (data[3] & 0xff) | (((uint16_t)data[4] << 8) & 0xff00);
                err::check_bool_raise(x >= 0 && x <= 0x7fff, "x out of range, max range is [0, 0x7fff]");
                err::check_bool_raise(y >= 0 && y <= 0x7fff, "y out of range, max range is [0, 0x7fff]");
                break;
            }
            default:
                err::check_raise(err::ERR_ARGS, "device type not support");
        }

        std::vector<uint8_t> hid_data(data.size());
        for (size_t i = 0; i < data.size(); i ++) {
            hid_data[i] = data[i];
        }

        int remain_len = hid_data.size();
        uint8_t *buf = hid_data.data();
        while (remain_len > 0) {
            int write_len = remain_len > 6 ? 6 : remain_len;
            remain_len -= write_len;

            int len = -1;
            int retry_cnt = 0;
            while (retry_cnt ++ < 200) {
                len = ::write(_fd, buf, write_len);
                if (len == write_len) {
                    break;
                }
                time::sleep_ms(10);
            }

            if (len != write_len) {
                log::error("write hid failed, need write %d bytes, but write %d bytes", write_len, len);
                err::check_bool_raise(len == write_len, "write hid failed");
            }

            buf += write_len;
        }
        return err::ERR_NONE;
    }
};
