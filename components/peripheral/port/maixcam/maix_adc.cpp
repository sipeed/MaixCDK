/**
 * @author spieed
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
#include <cstdlib>

#define DEV_PATH "/sys/class/cvi-saradc/cvi-saradc0/device/cv_saradc"

namespace maix::peripheral::adc{   

    ADC::ADC(int pin, int resolution, float vref) : _vref(vref)
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

        if (vref < 0) 
            _vref = 180 * 1000;

        const char adc_channel[] = "1";

        _fd = ::open(DEV_PATH, O_RDWR | O_NONBLOCK);
        if (_fd < 0) {
            log::error("open %s failed\r\n", DEV_PATH);
            return;
        }

        int ret = ::write(_fd, adc_channel, sizeof(adc_channel));
        if (ret != sizeof(adc_channel)) {
            log::error("set adc channel failed\r\n");
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
        if (_fd <= 2) {
            log::error("is the adc device on?");
            return -1;
        }
        int value = -1;
        char buf[8]{0x00};

        if (::lseek(_fd, 0, SEEK_SET) == -1) {
            log::error("read adc failed");
            return value;
        }

        int ret = ::read(_fd, buf, sizeof(buf));
        if (ret < 0) {
            log::error("read adc failed");
            return value;
        }

        // log::info("ret = %d", ret);
        // for (int i = 0; i < ret; ++i)
        //     log::info("adc buf[%d] = %d", i, buf[i]);

        value = std::atoi(buf);
        return value;
    }

    float ADC::read_vol()
    {
        int value = this->read();

        return (value * _vref / _resolution);
    }
}