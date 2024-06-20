/**
 * @author neucrack@sipeed, lxowalle@sipeed, iawak9lkm@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>
#include "maix_basic.hpp"

namespace maix::peripheral::gpio
{
    /**
     * @brief GPIO mode
     * @maixpy maix.peripheral.gpio.Mode
     */
    enum Mode
    {
        IN     = 0x01,     // input mode
        OUT    = 0x02,     // output mode
        OUT_OD = 0x03,     // output open drain mode
        MODE_MAX
    };

    /**
     * @brief GPIO pull mode
     * @maixpy maix.peripheral.gpio.Pull
     */
    enum Pull
    {
        PULL_NONE = 0x00,  // pull none mode
        PULL_UP   = 0x01,  // pull up mode
        PULL_DOWN = 0x02,  // pull down mode
        PULL_MAX
    };

    /**
     * Peripheral gpio class
     * @maixpy maix.peripheral.gpio.GPIO
     */
    class GPIO
    {
    public:
        /**
         * @brief GPIO constructor
         *
         * @param[in] pin gpio pin name, string type the same as board's pin name, e.g. "B14" or "GPIOB14", or number string like "10" if board no gpiochipe name.
         * @param[in] mode gpio mode. gpio.Mode type, default is gpio.Mode.IN (input) mode.
         * @param[in] pull gpio pull. gpio.Pull type, default is gpio.Pull.PULL_NONE (pull none) mode.
         * For input mode, this will set gpio default status(value), if set to gpio.Pull.PULL_NONE, gpio value will be floating.
         * For output mode, this will set gpio default status(value), if set to gpio.Pull.PULL_UP, gpio value will be 1, else 0.
         * @throw err::Exception if open gpio device failed.
         * @maixpy maix.peripheral.gpio.GPIO.__init__
         */
        GPIO(std::string pin, gpio::Mode mode = gpio::Mode::IN, gpio::Pull pull = gpio::Pull::PULL_NONE);
        ~GPIO();

        /**
         * @brief set and get gpio value
         *
         * @param[in] value gpio value. int type.
         *                   0, means write gpio to low level
         *                   1, means write gpio to high level
         *                  -1, means read gpio value, not set
         * @return int type, return gpio value, can be 0 or 1
         * @maixpy maix.peripheral.gpio.GPIO.value
         */
        int value(int value = -1);

        /**
         * @brief set gpio high (value to 1)
         * @maixpy maix.peripheral.gpio.GPIO.high
         */
        void high();

        /**
         * @brief set gpio low (value to 0)
         * @maixpy maix.peripheral.gpio.GPIO.low
         */
        void low();

        /**
         * @brief gpio toggle
         * @maixpy maix.peripheral.gpio.GPIO.toggle
         */
        void toggle();

        /**
         * @brief gpio get mode
         * @maixpy maix.peripheral.gpio.GPIO.get_mode
         */
        gpio::Mode get_mode();

        /**
         * @brief get gpio pull
         * @return gpio::Pull type
         * @maixpy maix.peripheral.gpio.GPIO.get_pull
         */
        gpio::Pull get_pull();

        /**
         * @brief reset gpio
         * @param[in] mode gpio mode. gpio.Mode type
         * @param[in] pull gpio pull. gpio.Pull type
         * For input mode, this will set gpio default status(value), if set to gpio.Pull.PULL_NONE, gpio value will be floating.
         * For output mode, this will set gpio default status(value), if set to gpio.Pull.PULL_UP, gpio value will be 1, else 0.
         * @return err::Err type
         * @maixpy maix.peripheral.gpio.GPIO.reset
         */
        err::Err reset(gpio::Mode mode, gpio::Pull pull);

    private:
        std::string _pin;
        gpio::Mode  _mode;
        gpio::Pull  _pull;
        int         _fd;
        int         _offset;
        int         _line;
        bool        _special;
    };
}; // namespace maix::peripheral::gpio
