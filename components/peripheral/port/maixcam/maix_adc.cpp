/**
 * @author lxowalle@sipeed iawak9lkm@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.6.6: Add framework, create this file.
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
#define DEFAULT_VREF 5.0062
#define ERR_NEAR_ZERO 0.01

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

        if (vref < 0) {
            _vref = DEFAULT_VREF;
            log::info("ADC default vref:%lf", DEFAULT_VREF);
        }

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
            ::close(_fd);
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
        value = std::atoi(buf);
        return value;
    }

    float ADC::read_vol()
    {
        int value = this->read();

        auto vol = value * _vref / _resolution;
        if (vol < ERR_NEAR_ZERO)
            vol = 0;

        return vol;
    }
}