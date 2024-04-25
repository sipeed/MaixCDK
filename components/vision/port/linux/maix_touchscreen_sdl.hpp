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
#include "SDL.h"


namespace maix::touchscreen
{

    class TouchScreen_SDL final : public TouchScreen_Base
    {
    public:
        static int _x;
        static int _y;
        static bool _pressed;
        static int _width;
        static int _height;

        TouchScreen_SDL(const std::string &device = "")
        {
            this->opened = false;
        }

        err::Err open()
        {
            this->opened = true;
            return err::ERR_NONE;
        }

        err::Err close()
        {
            this->opened = false;
            return err::ERR_NONE;
        }

        bool is_opened() {
            return this->opened;
        }

        err::Err read(int &x, int &y, bool &pressed)
        {
            x = TouchScreen_SDL::_x;
            y = TouchScreen_SDL::_y;
            pressed = TouchScreen_SDL::_pressed;
            return err::ERR_NONE;
        }

        std::vector<int> read()
        {
            return {_x, _y, _pressed ? 1 : 0};
        }

        err::Err read0(int &x, int &y, bool &pressed)
        {
            x = TouchScreen_SDL::_x;
            y = TouchScreen_SDL::_y;
            pressed = TouchScreen_SDL::_pressed;
            return err::ERR_NONE;
        }

        std::vector<int> read0()
        {
            return {_x, _y, _pressed ? 1 : 0};
        }

        bool available(int timeout)
        {
            return false;
        }

        static void touch_event_handle(SDL_Event *event)
        {
            // print pid of current thread
            switch (event->type)
            {
            case SDL_MOUSEBUTTONDOWN:
                _pressed = true;
                _x = event->button.x;
                _y = event->button.y;
                break;
            case SDL_MOUSEBUTTONUP:
                _pressed = false;
                _x = event->button.x;
                _y = event->button.y;
                break;
            case SDL_MOUSEMOTION:
                _x = event->motion.x;
                _y = event->motion.y;
                break;
            case SDL_FINGERDOWN:
                _pressed = true;
                _x = event->tfinger.x * _width;
                _y = event->tfinger.y * _height;
                break;
            case SDL_FINGERUP:
                _pressed = false;
                _x = event->tfinger.x * _width;
                _y = event->tfinger.y * _height;
                break;
            case SDL_FINGERMOTION:
                _x = event->tfinger.x * _width;
                _y = event->tfinger.y * _height;
                break;
            default:
                break;
            }
        }

    public:
        bool opened;
    };
} // namespace maix::touchscreen



