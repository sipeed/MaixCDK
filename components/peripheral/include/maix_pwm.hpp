/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_basic.hpp"

namespace maix::peripheral::pwm
{
    /**
     * Peripheral pwm class
     * @maixpy maix.peripheral.pwm.PWM
     */
    class PWM
    {
    public:
        /**
         * @brief PWM constructor
         *
         * @param[in] pin pwm id, int type, like 0, 1, 2 etc.
         * @param[in] freq pwm frequency, unit: Hz. int type. default is 1000
         * @param[in] duty pwm duty. double type. range is [0, 100], default is 0.
         * @param[in] enable enable pwm output right now. bool type. default is true, if false, you need to call enable() to enable pwm output.
         * @param[in] duty_val pwm duty value, int type. default -1 means not set and auto calculate by freq and duty.
         *                     This arg directly set pwm duty value, if set, will ignore duty arg.
         *                     duty_val = duty / 100 * T_ns, T_ns = 1 / freq * 1000000000.
         * @throw If args error or init pwm failed, will throw err::Exception
         * @maixpy maix.peripheral.pwm.PWM.__init__
         */
        PWM(int id, int freq = 1000, double duty = 0, bool enable = true, int duty_val = -1);
        ~PWM();

        /**
         * @brief get or set pwm duty
         *
         * @param[in] duty pwm duty, double type, value in [0, 100], default -1 means only read.
         * @return current duty, float type, if set and set failed will return -err::Err
         * @maixpy maix.peripheral.pwm.PWM.duty
         */
        double duty(double duty = -1);

        /**
         * @brief set pwm duty value
         *
         * @param[in] duty_val pwm duty value. int type. default is -1
         * duty_val > 0 means set duty_val
         * duty_val == -1 or not set, return current duty_val
         *
         * @return int type
         * when get duty_val, return current duty_val, else return -err::Err code.
         * @maixpy maix.peripheral.pwm.PWM.duty_val
         */
        int duty_val(int duty_val = -1);

        /**
         * @brief get or set pwm frequency
         *
         * @param[in] freq pwm frequency. int type. default is -1
         * freq >= 0, set freq
         * freq == -1 or not set, return current freq
         *
         * @return int type, current freq, if set and set failed will return -err::Err
         * @maixpy maix.peripheral.pwm.PWM.freq
         */
        int freq(int freq = -1);

        /**
         * @brief set pwm enable
         * @return err::Err type, err.Err.ERR_NONE means success
         * @maixpy maix.peripheral.pwm.PWM.enable
         */
        err::Err enable();

        /**
         * @brief set pwm disable
         * @return err::Err type, err.Err.ERR_NONE means success
         * @maixpy maix.peripheral.pwm.PWM.disable
         */
        err::Err disable();

        /**
         * @brief get pwm enable status
         * @return bool type, true means enable, false means disable
         * @maixpy maix.peripheral.pwm.PWM.is_enabled
         */
        bool is_enabled();

    private:
        int _pwm_id;
        int _chip_id;
        int _id_offset;

        int _freq;       // unit: hz
        int _period_ns;  // unit: ns = 1 / freq * 1000000000
        int _duty;       // unit: %
        int _duty_val;   // unit: percent of period_ns
        int _enable;
    };
}; // namespace maix::peripheral::pwm
