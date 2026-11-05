/**
 * @author Sipeed
 * @copyright Sipeed Ltd 2026-
 * @license Apache 2.0
 * @update 2026.11.5: create this file.
 */

#pragma once

#include <string>
#include "maix_basic.hpp"

namespace maix::peripheral::light
{
    /**
     * @brief Fill light(补光灯) control class.
     *
     * The light pin is read from the board config file(/boot/board or /boot/boards/board.<id>)
     * item "cam_light_io", default is "B25" on MaixCAM2 and "B3" on MaixCAM.
     *
     * On MaixCAM2 the light is driven by PWM so its brightness can be adjusted,
     * the PWM frequency and duty are read from the system config file(/boot/configs):
     *   maix_light_freq: PWM frequency, unit Hz, default 10000
     *   maix_light_duty: PWM duty, unit %, range [0, 100], default 50
     * On MaixCAM the light is driven by GPIO, only on/off is supported.
     *
     * @maixpy maix.peripheral.light.Light
     */
    class Light
    {
    public:
        /**
         * @brief Fill light constructor, will open the light hardware and turn it off by default.
         * @throw err::Exception if open the light hardware failed.
         * @maixpy maix.peripheral.light.Light.__init__
         */
        Light();
        ~Light();

        /**
         * @brief Turn on the fill light.
         * On MaixCAM2 the PWM duty will be set to the configured duty(maix_light_duty),
         * on MaixCAM the GPIO will be set to high level.
         * @return error code, err::ERR_NONE means success.
         * @maixpy maix.peripheral.light.Light.on
         */
        err::Err on();

        /**
         * @brief Turn off the fill light.
         * @return error code, err::ERR_NONE means success.
         * @maixpy maix.peripheral.light.Light.off
         */
        err::Err off();

        /**
         * @brief Get the fill light state.
         * @return true if the light is on, else false.
         * @maixpy maix.peripheral.light.Light.is_on
         */
        bool is_on();

        /**
         * @brief Set the fill light brightness(PWM duty), only take effect at runtime,
         * will not modify the config file.
         *
         * On MaixCAM only 0(off) and non-zero(on) are supported.
         *
         * @note If duty > 70, a warning will be printed, because long time high power
         * running may cause the LED to age quickly.
         *
         * @param[in] duty PWM duty, unit %, range [0, 100]. 0 means turn off the light.
         * @return error code, err::ERR_NONE means success.
         * @maixpy maix.peripheral.light.Light.set_duty
         */
        err::Err set_duty(double duty);

        /**
         * @brief Get the current fill light brightness(PWM duty).
         * @return PWM duty, unit %, range [0, 100].
         * @maixpy maix.peripheral.light.Light.duty
         */
        double duty();

        /**
         * @brief Get the fill light pin name.
         * @return pin name, e.g. "B25".
         * @maixpy maix.peripheral.light.Light.pin
         */
        std::string pin();

        /**
         * @brief Get the fill light PWM frequency.
         * @return PWM frequency, unit Hz. Return 0 if the light is driven by GPIO.
         * @maixpy maix.peripheral.light.Light.freq
         */
        int freq();

    private:
        void *_priv;
    };
} // namespace maix::peripheral::light
