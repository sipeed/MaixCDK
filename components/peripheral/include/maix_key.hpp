/**
 * Gey device key input
 * @author neucrack@sipeed.com
 * @date 2024.3.19: Add file by Neucrack
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
*/
#pragma once

#include "maix_err.hpp"
#include <functional>
#include <string>

namespace maix::peripheral::key
{


    /**
     * Keys enum, id the same as linux input.h(input-event-codes.h)
     * @maixpy maix.peripheral.key.Keys
    */
    enum Keys{
        KEY_NONE   = 0x000,
        KEY_ESC    = 0x001,
        KEY_SPACE  = 0x039,
        KEY_LEFT   = 0x069,
        KEY_RIGHT  = 0x06a,
        KEY_POWER  = 0x074,
        KEY_OK     = 0x160,
        KEY_OPTION = 0x165,
        KEY_NEXT   = 0x197,
        KEY_PREV   = 0x19c,
    };

    /**
     * Key state enum
     * @maixpy maix.peripheral.key.State
    */
    enum State{
        KEY_RELEASED     = 0,
        KEY_PRESSED      = 1,
        KEY_LONG_PRESSED = 2,
    };

    /**
     * Key input class
     * @maixpy maix.peripheral.key.Key
    */
    class Key
    {
    public:
        /**
         * @brief Key Device constructor
         * @param callback When key triggered and callback is not empty(empty In MaixPy is None, in C++ is nullptr), 
         *                 callback will be called with args key(key.Keys) and value(key.State).
         *                 If set to null, you can get key value by read() function.
         *                 This callback called in a standalone thread, so you can block a while in callback, and you should be carefully when operate shared data.
         * @param open auto open device in constructor, if false, you need call open() to open device.
         * @param device Specifies the input device to use. The default initializes all keys,
         *               for a specific device, provide the path (e.g., "/dev/input/device").
         * @param long_press_time The duration (in milliseconds) from pressing the key to triggering the long press event. Default is 2000ms.
         * @maixpy maix.peripheral.key.Key.__init__
         * @maixcdk maix.peripheral.key.Key.Key
        */
        Key(std::function<void(int, int)> callback = nullptr, bool open = true, const string &device = "", int long_press_time = 2000);

        ~Key();

        /**
         * @brief Open(Initialize) key device, if already opened, will close first and then open.
         * @return err::Err type, err.Err.ERR_NONE means success
         * @maixpy maix.peripheral.key.Key.open
        */
        err::Err open();

        /**
         * @brief Close key device
         * @return err::Err type, err.Err.ERR_NONE means success
         * @maixpy maix.peripheral.key.Key.close
        */
        err::Err close();

        /**
         * @brief Check key device is opened
         * @return bool type, true means opened, false means closed
         * @maixpy maix.peripheral.key.Key.is_opened
        */
        bool is_opened();

        /**
         * @brief Read key input, if callback is set, DO NOT call this function manually.
         * @param key maix.key.Keys type, indicate which key is triggered, e.g. maix.key.Keys.KEY_OK mean the OK key is triggered.
         *            If read failed, will not change the value of key.
         * @param value maix.key.State type, indicate the key state, e.g. maix.key.State.KEY_PRESSED mean the key is pressed
         *            If read failed, will not change the value of value.
         * @return err::Err type, err.Err.ERR_NONE means success, err.Err.ERR_NOT_READY means no key input, other means error.
         * @maixcdk maix.peripheral.key.Key.read
        */
        err::Err read(int &key, int &value);

        /**
         * @brief Read key input, and return key and value, if callback is set, DO NOT call this function manually.
         * @return list type, first is key(maix.key.Keys), second is value(maix.key.State), if no key input, return [0, 0]
         * @throw If read failed, will throw maix.err.Exception.
         * @maixpy maix.peripheral.key.Key.read
        */
        std::pair<int, int> read();

        /**
         * @brief Sets and retrieves the key's long press time.
         * @param press_time The long press time to set for the key.
         *        Setting it to 0 will disable the long press event.
         * @return int type, the current long press time for the key (in milliseconds).
         * @maixpy maix.peripheral.key.Key.long_press_time
        */
        int long_press_time(int press_time = -1);

    private:
        std::vector<int> _fds;
        std::string _device;
        std::vector<std::string> _device_list;
        std::function<void(int, int)> _callback;
        void *_data;

        err::Err get_key_devices();
    };

    /**
     * @brief Add default listener, if you want to exit app when press ok button, you can just call this function.
     *        This function is auto called in MaixPy' startup code, so you don't need to call it in MaixPy.
     *        Create Key object will auto call rm_default_listener() to cancel the default ok button function.
     *        When ok button pressed, a SIGINT signal will be raise and call app.set_exit_flag(True).
     * @maixpy maix.peripheral.key.add_default_listener
    */
    void add_default_listener();

    /**
     * @brief Remove default listener, if you want to cancel the default ok button function(exit app), you can just call this function.
     * @maixpy maix.peripheral.key.rm_default_listener
    */
    void rm_default_listener();

} // namespace maix::peipheral::key




