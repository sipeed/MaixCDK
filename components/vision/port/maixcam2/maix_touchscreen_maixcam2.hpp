/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>
#include <vector>
#include "maix_basic.hpp"
#include "maix_touchscreen_base.hpp"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

namespace maix::touchscreen
{

    class TouchScreen_MaixCam final : public TouchScreen_Base
    {
    public:
        TouchScreen_MaixCam(const std::string &device = "/dev/input/event1")
        {
            _opened = false;
            _fd = -1;
            _device = device;
        }

        err::Err open()
        {
            if(_fd > 0)
            {
                return err::ERR_NONE;
            }
            _opened = false;
            _fd = ::open(_device.c_str(), O_RDONLY);
            if(_fd < 0)
            {
                log::error("open touch screen failed: %s", _device.c_str());
                return err::ERR_IO;
            }

            _x = 0;
            _y = 0;
            _pressed = false;

            // set unblock read
            int non_blocking = 1;
            ioctl(_fd, FIONBIO, &non_blocking);

            // char * absval[6] = { "Value", "Min", "Max", "Fuzz", "Flat", "Resolution" };
            int absX[6] = {};
            int absY[6] = {};

            ioctl(_fd, EVIOCGABS(ABS_MT_POSITION_X), absX);
            ioctl(_fd, EVIOCGABS(ABS_MT_POSITION_Y), absY);

            _x_max = absX[2];
            _y_max = absY[2];
            if(_x_max <= 0 || _y_max <= 0)
            {
                log::error("get touchscreen resolution failed");
                _x_max = 368;
                _y_max = 552;
            }

            _init_epoll(_fd);

            _opened = true;
            return err::ERR_NONE;
        }

        err::Err close()
        {
            if(_fd > 0)
            {
                ::close(_fd);
                _fd = -1;
            }
            _opened = false;
            return err::ERR_NONE;
        }

        bool is_opened() {
            return _opened;
        }

        err::Err read(int &x, int &y, bool &pressed)
        {
            err::Err e = _read(true);
            if(e != err::ERR_NONE)
                return e;
            x = _x;
            y = _y;
            pressed = _pressed;
            return err::ERR_NONE;
        }

        std::vector<int> read()
        {
            _read(true);
            return {_x, _y, _pressed ? 1 : 0};
        }

        err::Err read0(int &x, int &y, bool &pressed)
        {
            err::Err e = _read(false);
            if(e != err::ERR_NONE)
                return e;
            x = _x;
            y = _y;
            pressed = _pressed;
            return err::ERR_NONE;
        }

        std::vector<int> read0()
        {
            _read(false);
            return {_x, _y, _pressed ? 1 : 0};
        }

        bool available(int timeout)
        {
            return _available(timeout);
        }


    private:
        int _x;
        int _y;
        bool _pressed;
        int _width;
        int _height;
        bool _opened;
        int _fd;
        std::string _device;
        int _x_max;
        int _y_max;
        int _epoll_fd;

        void _init_epoll(int fd)
        {
            _epoll_fd = epoll_create(1);
            if(_epoll_fd < 0)
            {
                log::error("create epoll failed: %s", strerror(errno));
                return;
            }
            struct epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.fd = fd;
            if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
            {
                log::error("epoll_ctl add failed: %s", strerror(errno));
                return;
            }
        }

        bool _available(int timeout)
        {
            struct epoll_event events[1];
            int nfds = epoll_wait(_epoll_fd, events, 1, timeout);
            return nfds > 0;
        }

        err::Err _read(bool empty_buffer)
        {
            struct input_event event;
            // struct epoll_event events[1];
            bool event_press = false;
            bool event_move = false;
            while(1)
            {
                // int nfds = epoll_wait(_epoll_fd, events, 1, 0);
                // if(nfds <= 0)
                // {
                //     return err::ERR_NOT_READY;
                // }
                int ret = ::read(_fd, &event, sizeof(event));
                if(ret != sizeof(event))
                {
                    if(event_move)
                        return err::ERR_NONE;
                    if(errno == EAGAIN || errno == EWOULDBLOCK)
                        return err::ERR_NOT_READY;
                    log::error("read touch screen failed: %s, %d", strerror(errno), ret);
                    return err::ERR_IO;
                }
                // printf("event %d %d\n", event.type, event.code);
                if(event.type == EV_ABS)
                {
                    if(event.code == ABS_MT_POSITION_X || event.code == ABS_X)
                    {
                        // _x = event.value;
                        // anti-clockwise 90 degree
                        _y = event.value;
                        event_move = true;
                    }
                    else if(event.code == ABS_MT_POSITION_Y || event.code == ABS_Y)
                    {
                        // _y = event.value;
                        // anti-clockwise 90 degree
                        _x = _y_max - event.value - 1;
                        event_move = true;
                    }
                }
                else if(event.type == EV_KEY)
                {
                    if(event.code == BTN_TOUCH)
                    {
                        _pressed = event.value == 1;
                        event_press = true;
                    }
                }
                else if(event.type == EV_SYN)
                {
                    if(event_press)
                    {
                        break;
                    }
                    if(event_move && !empty_buffer)
                    {
                        break;
                    }
                }
            }
            return err::ERR_NONE;
        }
    };
} // namespace maix::touchscreen



