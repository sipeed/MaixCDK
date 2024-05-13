/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_pwm.hpp"
#include "maix_log.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define EXPORT_PATH "/sys/class/pwm/pwmchip%d/export"
#define UNEXPORT_PATH "/sys/class/pwm/pwmchip%d/unexport"
#define DUTYCYCLE_PATH "/sys/class/pwm/pwmchip%d/pwm%d/duty_cycle"
#define PERIOD_PATH "/sys/class/pwm/pwmchip%d/pwm%d/period"
#define ENABLE_PATH "/sys/class/pwm/pwmchip%d/pwm%d/enable"
#define freq_to_period_ns(freq) (1000 * 1000 * 1000 / (freq))

namespace maix::peripheral::pwm
{
    static int _pwm_id_to_chipid(int pwm_id, int &chip_id, int &offset)
    {
        chip_id = 0;
        offset = 0;
        if (pwm_id >= 0 && pwm_id <= 3)
        {
            chip_id = 0;
            offset = pwm_id;
        }
        else if (pwm_id >= 4 && pwm_id <= 7)
        {
            chip_id = 4;
            offset = pwm_id - 4;
        }
        else if (pwm_id >= 8 && pwm_id <= 11)
        {
            chip_id = 8;
            offset = pwm_id - 8;
        }
        else if (pwm_id >= 12 && pwm_id <= 15)
        {
            chip_id = 12;
            offset = pwm_id - 12;
        }
        else
        {
            log::error("pwm_id %d is not support\r\n", pwm_id);
            return -1;
        }
        return 0;
    }

    static err::Err _pwm_unexport(int chip_id, int offset)
    {
        int fd;
        char buf[100];
        snprintf(buf, sizeof(buf), UNEXPORT_PATH, chip_id);
        fd = ::open(buf, O_WRONLY);
        if (fd < 0)
        {
            log::error("open %s failed\r\n", buf);
            return err::ERR_IO;
        }
        snprintf(buf, sizeof(buf), "%d", offset);
        if ((int)strlen(buf) != ::write(fd, buf, strlen(buf)))
        {
            log::error("write %s > %s failed\r\n", buf, UNEXPORT_PATH);
            close(fd);
            return err::ERR_IO;
        }
        fsync(fd);
        close(fd);
        return err::ERR_NONE;
    }

    static err::Err _pwm_export(int chip_id, int offset)
    {
        int fd;
        char buf[100];
        // check if ENABLE_PATH exists, return ERR_NONE
        snprintf(buf, sizeof(buf), ENABLE_PATH, chip_id, offset);
        if (access(buf, F_OK) == 0)
        {
            log::warn("pwm %d already exported, unexport first automatically", chip_id + offset);
            _pwm_unexport(chip_id, offset);
        }
        snprintf(buf, sizeof(buf), EXPORT_PATH, chip_id);
        fd = ::open(buf, O_WRONLY);
        if (fd < 0)
        {
            log::error("open %s failed\r\n", buf);
            return err::ERR_IO;
        }
        snprintf(buf, sizeof(buf), "%d", offset);
        if ((int)strlen(buf) != ::write(fd, buf, strlen(buf)))
        {
            log::error("write %s > %s failed\r\n", buf, EXPORT_PATH);
            close(fd);
            return err::ERR_IO;
        }
        fsync(fd);
        close(fd);
        return err::ERR_NONE;
    }

    static err::Err _pwm_set_period(int chip_id, int offset, int peroid_ns)
    {
        int fd;
        char buf[100];
        snprintf(buf, sizeof(buf), PERIOD_PATH, chip_id, offset);
        fd = ::open(buf, O_RDWR);
        if (fd < 0)
        {
            log::error("open %s failed\r\n", buf);
            return err::ERR_IO;
        }
        snprintf(buf, sizeof(buf), "%d", peroid_ns);
        if ((int)strlen(buf) != ::write(fd, buf, strlen(buf)))
        {
            log::error("write peroid_ns = %s failed\r\n", buf);
            close(fd);
            return err::ERR_IO;
        }
        fsync(fd);
        close(fd);
        return err::ERR_NONE;
    }

    static err::Err _pwm_set_duty_cycle(int chip_id, int offset, int duty_cycle)
    {
        int fd;
        char buf[100];
        snprintf(buf, sizeof(buf), DUTYCYCLE_PATH, chip_id, offset);
        fd = ::open(buf, O_RDWR);
        if (fd < 0)
        {
            log::error("open %s failed\r\n", buf);
            return err::ERR_IO;
        }
        snprintf(buf, sizeof(buf), "%d", duty_cycle);
        if ((int)strlen(buf) != ::write(fd, buf, strlen(buf)))
        {
            log::error("write duty_cycle = %s failed\r\n", buf);
            close(fd);
            return err::ERR_IO;
        }
        fsync(fd);
        close(fd);
        return err::ERR_NONE;
    }

    static err::Err _pwm_set_enable(int chip_id, int offset, bool en)
    {
        int fd;
        char buf[100];
        snprintf(buf, sizeof(buf), ENABLE_PATH, chip_id, offset);
        fd = ::open(buf, O_RDWR);
        if (fd < 0)
        {
            log::error("open %s failed\r\n", buf);
            return err::ERR_IO;
        }
        snprintf(buf, sizeof(buf), "%d", en ? 1 : 0);
        if ((int)strlen(buf) != ::write(fd, buf, strlen(buf)))
        {
            log::error("write enable = %s failed\r\n", buf);
            close(fd);
            return err::ERR_IO;
        }
        fsync(fd);
        close(fd);
        return err::ERR_NONE;
    }

    PWM::PWM(int id, int freq, double duty, bool enable, int duty_val)
    {
        _pwm_id = id;
        _freq = freq;
        _enable = enable;

        if (duty < 0 && duty_val < 0)
        {
            throw err::Exception(err::Err::ERR_ARGS, "one of duty and duty_val must be set");
        }
        if (freq <= 0)
        {
            throw err::Exception(err::Err::ERR_ARGS, "freq must be > 0");
        }

        _period_ns = freq_to_period_ns(freq);

        if (duty_val >= 0)
        {
            _duty_val = duty_val;
            _duty = duty_val * 100 / _period_ns;
        }
        else
        {
            _duty = duty;
            _duty_val = _period_ns * duty / 100;
        }

        // get chip_id and offset
        if (0 != _pwm_id_to_chipid(_pwm_id, _chip_id, _id_offset))
        {
            throw err::Exception(err::Err::ERR_ARGS, "pwm_id is not support");
        }

        // export
        if (_pwm_export(_chip_id, _id_offset) != err::ERR_NONE)
        {
            throw err::Exception(err::Err::ERR_IO, "export pwm failed");
        }

        if (_pwm_set_period(_chip_id, _id_offset, _period_ns) != err::ERR_NONE)
        {
            throw err::Exception(err::Err::ERR_IO, "set pwm period failed");
        }

        if (_pwm_set_duty_cycle(_chip_id, _id_offset, _duty_val) != err::ERR_NONE)
        {
            throw err::Exception(err::Err::ERR_IO, "set pwm duty_cycle failed");
        }

        // set enable
        if (_pwm_set_enable(_chip_id, _id_offset, _enable) != err::ERR_NONE)
        {
            throw err::Exception(err::Err::ERR_IO, "set pwm enable failed");
        }
    }

    PWM::~PWM()
    {
        disable();
        _pwm_unexport(_chip_id, _id_offset);
    }

    double PWM::duty(double duty)
    {
        if (duty < 0)
        {
            return _duty;
        }
        if (duty > 100)
            duty = 100;
        _duty = duty;
        _duty_val = _period_ns * duty / 100;
        err::Err ret = _pwm_set_duty_cycle(_chip_id, _id_offset, _duty_val);
        if (ret != err::ERR_NONE)
        {
            log::error("set pwm duty_cycle failed");
            return (int)-ret;
        }
        return duty;
    }

    int PWM::duty_val(int duty_val)
    {
        if (duty_val < 0)
        {
            return _duty_val;
        }
        _duty_val = duty_val;
        _duty = duty_val * 100 / _period_ns;
        err::Err ret = _pwm_set_duty_cycle(_chip_id, _id_offset, duty_val);
        if (ret != err::ERR_NONE)
        {
            log::error("set pwm duty_cycle failed");
            return (int)-ret;
        }
        return duty_val;
    }

    int PWM::freq(int freq)
    {
        if (freq < 0)
        {
            return _freq;
        }
        _freq = freq;
        _period_ns = freq_to_period_ns(freq);
        err::Err ret = _pwm_set_period(_chip_id, _id_offset, _period_ns);
        if (ret != err::ERR_NONE)
        {
            log::error("set pwm period failed");
            return (int)-ret;
        }
        return freq;
    }

    err::Err PWM::enable()
    {
        return _pwm_set_enable(_chip_id, _id_offset, true);
    }

    err::Err PWM::disable()
    {
        return _pwm_set_enable(_chip_id, _id_offset, false);
    }

    bool PWM::is_enabled()
    {
        int fd;
        char buf[100];
        snprintf(buf, sizeof(buf), ENABLE_PATH, _chip_id, _id_offset);
        fd = ::open(buf, O_RDONLY);
        if (fd < 0)
        {
            log::error("open %s failed\r\n", buf);
            return false;
        }
        if (1 != ::read(fd, buf, 1))
        {
            log::error("read %s failed\r\n", buf);
            close(fd);
            return false;
        }
        close(fd);
        return buf[0] == '1';
    }
}; // namespace maix
