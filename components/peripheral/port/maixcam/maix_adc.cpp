/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_adc.hpp"
#include "maix_log.hpp"
#include "cstdio"
#include <linux/input.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define DEV_PATH "/dev/input/event0"

namespace maix::peripheral::adc{    
    ADC::ADC(int pin, int resolution, float vref)
    {
        if (resolution == -1) {
            resolution = adc::RES_BIT_12;
        } else if (resolution != adc::RES_BIT_12) {
            log::warn("warning: ADC only support 12bit resolution, set to adc::RES_BIT_12\r\n");
            resolution = adc::RES_BIT_12;
        }
        if (resolution == adc::RES_BIT_8)
            _resolution = 256;
        else if (resolution == adc::RES_BIT_10)
            _resolution = 1024;
        else if (resolution == adc::RES_BIT_12)
            _resolution = 4096;
        else if (resolution == adc::RES_BIT_16)
            _resolution = 65536;
        else
            log::error("error: ADC resolution error\r\n");

        if (pin == -1) {
            log::warn("warning: ADC only support pin 0, set to 0\r\n");
            pin = 0;
        }
        _pin = pin;

        _vref = 180 * 1000;

        _fd = ::open(DEV_PATH, O_RDONLY | O_NONBLOCK);
        if (_fd < 0) {
            log::error("open %s failed\r\n", DEV_PATH);
            return;
        }
    }

    ADC::~ADC()
    {
        if (_fd > 2) {
            close(_fd);
        }
    }

    int ADC::read()
    {
        struct input_event data;
        int value = -1;
        if (sizeof(data) == ::read(_fd, &data, sizeof(data))) {
            if (data.type == EV_KEY) {
                log::warn("this is a key event? value: %d\r\n", data.value);
                return -1;
            } else if (data.type == EV_MSC) {
                value = data.value;
            }
        } else {
            return -1;
        }

        return value;
    }

    float ADC::read_vol()
    {
        struct input_event data;
        int value = -1;
        if (sizeof(data) == ::read(_fd, &data, sizeof(data))) {
            if (data.type == EV_KEY) {
                log::warn("this is a key event? value: %d\r\n", data.value);
                return -1;
            } else if (data.type == EV_MSC) {
                value = data.value;
            }
        } else {
            return -1;
        }

        return (value * _vref / _resolution);
    }
}