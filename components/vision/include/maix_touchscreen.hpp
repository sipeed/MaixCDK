/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>
#include <vector>
#include "maix_err.hpp"
#include "maix_touchscreen_base.hpp"

namespace maix::touchscreen
{
    /**
     * TouchScreen class
     * @maixpy maix.touchscreen.TouchScreen
     */
    class TouchScreen
    {
    public:
        /**
         * @brief Construct a new TouchScreen object
         * @param device touchscreen device path, you can get devices by list_devices method, by default(value is NULL(None in MaixPy)) means the first device
         * @param open If true, touchscreen will automatically call open() after creation. default is true.
         * @maixpy maix.touchscreen.TouchScreen.__init__
         * @maixcdk maix.touchscreen.TouchScreen.TouchScreen
         */
        TouchScreen(const std::string &device = "", bool open = true);
        ~TouchScreen();

        /**
         * @brief open touchscreen device
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.touchscreen.TouchScreen.open
         */
        err::Err open();

        /**
         * @brief close touchscreen device
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.touchscreen.TouchScreen.close
         */
        err::Err close();

        /**
         * @brief read touchscreen device.
         * @attention This method will discard same event in buffer, that is:
         *        if too many move event in buffer when call this method, it will only return the last one,
         *        and if read pressed or released event, it will return immediately.
         * @param x x coordinate
         * @param y y coordinate
         * @param pressed pressed state
         * @return error code, err::ERR_NONE means success, others means failed, if no event return err::ERR_NOT_READY
         * @maixcdk maix.touchscreen.TouchScreen.read
         */
        err::Err read(int &x, int &y, bool &pressed);

        /**
         * @brief read touchscreen device
         * @attention This method will discard same event in buffer, that is:
         *        if too many move event in buffer when call this method, it will only return the last one,
         *        and if read pressed or released event, it will return immediately.
         * @return Returns a list include x, y, pressed state
         * @maixpy maix.touchscreen.TouchScreen.read
         */
        std::vector<int> read();

        /**
         * @brief read touchscreen device
         * @attention This method will return immediately if have event, so it's better to use available() to check if have more event in buffer,
         *            or too much event in buffer when your program call this read() interval is too long will make your program slow.
         * @param x x coordinate
         * @param y y coordinate
         * @param pressed pressed state
         * @return error code, err::ERR_NONE means success, others means failed, if no event return err::ERR_NOT_READY
         * @maixcdk maix.touchscreen.TouchScreen.read0
         */
        err::Err read0(int &x, int &y, bool &pressed);

        /**
         * @brief read touchscreen device
         * @attention This method will return immediately if have event, so it's better to use available() to check if have more event in buffer,
         *            or too much event in buffer when your program call this read() interval is too long will make your program slow.
         * @return Returns a list include x, y, pressed state
         * @maixpy maix.touchscreen.TouchScreen.read0
         */
        std::vector<int> read0();

        /**
         * @brief If we need to read from touchscreen, for event driven touchscreen means have event or not
         * @param timeout -1 means block, 0 means no block, >0 means timeout, default is 0, unit is ms.
         * @return true if need to read(have event), false if not
         * @maixpy maix.touchscreen.TouchScreen.available
        */
        bool available(int timeout = 0);

        /**
         * Check if touchscreen is opened
         * @return true if touchscreen is opened, false if not
         * @maixpy maix.touchscreen.TouchScreen.is_opened
        */
        bool is_opened();
    private:
        TouchScreen_Base *_impl; // pointer for implementation
        bool _is_opened;
    };
} // namespace maix::touchscreen

