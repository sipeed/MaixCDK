/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.9.04: create this file.
 */

#pragma once

#include "maix_basic.hpp"

namespace maix::peripheral::hid
{
    /**
     * Device enum of hid
     * @maixpy maix.peripheral.hid.DeviceType
    */
    enum DeviceType {
        DEVICE_MOUSE = 0,
        DEVICE_KEYBOARD,
        DEVICE_TOUCHPAD
    };

    /**
     * Hid class
     * @maixpy maix.peripheral.hid.Hid
    */
    class Hid
    {
    public:
        /**
         * @brief Hid Device constructor
         * @param device_type Device type, used to select mouse, keyboard, or touchpad.
         * @param open auto open device in constructor, if false, you need call open() to open device
         * @maixpy maix.peripheral.hid.Hid.__init__
        */
        Hid(hid::DeviceType device_type, bool open = true);
        ~Hid();

        /**
         * @brief Open hid device
         * @return err::Err
         * @maixpy maix.peripheral.hid.Hid.open
        */
        err::Err open();

        /**
         * @brief Close hid device
         * @return err::Err
         * @maixpy maix.peripheral.hid.Hid.close
        */
        err::Err close();

        /**
         * @brief Write data to hid device
         * @param data data to write
         * For the keyboard, 8 bytes of data need to be written, with the format as follows:
         * data =      [0x00,   #
         *              0x00,   #
         *              0x00,   # Key value. Refer to the "Universal Serial Bus HID Usage Tables" section of the official documentation(https://www.usb.org).
         *              0x00,   #
         *              0x00,   #
         *              0x00,   #
         *              0x00,   #
         *              0x00]   #
         * For the mouse, 4 bytes of data need to be written, with the format as follows:
         * data =       [0x00,  # Button state
         *                      0x00: no button pressed
         *                      0x01: press left button
         *                      0x02: press right button
         *                      0x04: press middle button
         *              x,      # X-axis relative coordinates. Signed number, positive values for x indicate movement to the right
         *              y,      # Y-axis relative coordinates. Signed number, positive values for y indicate movement downward
         *              0x00]   # Wheel movement. Signed number, positive values indicate downward movement.
         * For the touchpad, 6 bytes of data need to be written, with the format as follows:
         * data =      [0x00,   # Button state (0: no button pressed, 0x01: press left button, 0x10, press right button.)
         *              x & 0xFF, (x >> 8) & 0xFF,  # X-axis absolute coordinate, 0 means unused.
         *                                          Note: You must map the target position to the range [0x1, 0x7FFF]. This means x value = <position_to_move> * 0x7FFF / <actual_screen_width>
         *              y & 0xFF, (y >> 8) & 0xFF,  # Y-axis absolute coordinate, 0 means unused.
         *                                          Note: You must map the target position to the range [0x1, 0x7FFF]. This means y value = <position_to_move> * 0x7FFF / <actual_screen_height>
         *              0x00,   # Wheel movement. Signed number, positive values indicate downward movement.
         * @return err::Err
         * @maixpy maix.peripheral.hid.Hid.write
        */
        err::Err write(std::vector<int> &data);

        /**
         * @brief Check if hid device is opened
         * @return bool
         * @maixpy maix.peripheral.hid.Hid.is_opened
        */
        bool is_opened() {
            return _is_opened;
        }
    private:
        bool _is_opened;
        int _fd;
        DeviceType _device_type;
    };
}; // namespace maix::peripheral::hid



