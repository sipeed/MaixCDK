/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_touchscreen.hpp"
#include "global_config.h"

#ifdef PLATFORM_LINUX
#include "maix_touchscreen_sdl.hpp"
#elif PLATFORM_MAIXCAM
#include "maix_touchscreen_maixcam.hpp"
#endif

namespace maix::touchscreen
{
    TouchScreen::TouchScreen(const std::string &device, bool open)
    {
#ifdef PLATFORM_LINUX
        _impl = new TouchScreen_SDL(device);
#elif PLATFORM_MAIXCAM
        _impl = new TouchScreen_MaixCam();
#else
    #warning "This platform not support touchscreen yet"
#endif
        if (open) {
            _impl->open();
        }
    }

    TouchScreen::~TouchScreen()
    {
#ifdef PLATFORM_LINUX
        delete (TouchScreen_SDL *)_impl;
#elif PLATFORM_MAIXCAM
        delete (TouchScreen_MaixCam *)_impl;
#else
    #warning "This platform not support touchscreen yet"
#endif
    }

    err::Err TouchScreen::open()
    {
        return _impl->open();
        // return err::ERR_NONE;
    }

    err::Err TouchScreen::close()
    {
        return _impl->close();
        // return err::ERR_NONE;
    }

    err::Err TouchScreen::read(int &x, int &y, bool &pressed)
    {
        return _impl->read(x, y, pressed);
        // return err::ERR_NONE;
    }

    std::vector<int> TouchScreen::read()
    {
        return _impl->read();
        // return {0, 0, 0};
    }

    err::Err TouchScreen::read0(int &x, int &y, bool &pressed)
    {
        return _impl->read0(x, y, pressed);
        // return err::ERR_NONE;
    }

    std::vector<int> TouchScreen::read0()
    {
        return _impl->read0();
        // return {0, 0, 0};
    }

    bool TouchScreen::available(int timeout)
    {
        return _impl->available(timeout);
    }

    bool TouchScreen::is_opened() {
        return _impl->is_opened();
    }
} // namespace maix::touchscreen

