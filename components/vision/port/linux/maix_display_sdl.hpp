/**
 * @file maix_display_sdl.hpp
 * @brief Maix display SDL implementation
 * @author neucrack@sipeed.com
 * @license Apache 2.0 Sipeed Ltd
 * @update date 2023-10-23 Create by neucrack
 */

#pragma once

#include "maix_display_base.hpp"
#include "maix_thread.hpp"
#include "maix_image.hpp"
#include "SDL.h"
#include "maix_touchscreen_sdl.hpp"

namespace maix::display
{
    class SDL_Display final : public DisplayBase
    {
    public:
        SDL_Display(const std::string &device, int width, int height, image::Format format)
        {
            this->_width = width;
            this->_height = height;
            this->exit = false;
            this->_event_exit_done = false;
            this->opened = false;
            this->_th = nullptr;
        }

        ~SDL_Display()
        {
            if (opened)
                close();
        }

        int width()
        {
            return _width;
        }

        int height()
        {
            return _height;
        }

        std::vector<int> size()
        {
            return {_width, _height};
        }

        image::Format format()
        {
            return image::FMT_RGBA8888;
        }

        err::Err open(int width, int height, image::Format format)
        {
            if (opened)
            {
                if (exit) // already closed by disp
                    close();
                else
                    return err::ERR_NONE;
            }
            int ret = SDL_Init(SDL_INIT_VIDEO);
            if (ret != 0)
            {
                log::error("SDL_Init failed: %d, %s\n", ret, SDL_GetError());
                return err::ERR_RUNTIME;
            }
            // get actually screen max supported size
            SDL_DisplayMode mode;
            ret = SDL_GetCurrentDisplayMode(0, &mode);
            if (ret != 0)
            {
                log::error("SDL_GetCurrentDisplayMode failed: %d, %s\n", ret, SDL_GetError());
                return err::ERR_RUNTIME;
            }
            if (_width > mode.w)
            {
                log::warn("screen max supported width: %d, but set %d\n", mode.w, _width);
                _width = mode.w;
            }else if(_width == -1){ //fix bug of displaying with width and height of -1 in linux SDL mode
                _width = mode.w;
            }
            if (_height > mode.h)
            {
                log::warn("screen max supported height: %d, but set %d\n", mode.h, _height);
                _height = mode.h;
            }else if(_height == -1){ //fix bug of displaying with width and height of -1 in linux SDL mode
                _height = mode.h;
            }

            _screen = SDL_CreateWindow("Maix", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, SDL_WINDOW_SHOWN);
            if (!_screen)
            {
                log::error("SDL_CreateWindow failed: %s\n", SDL_GetError());
                return err::ERR_RUNTIME;
            }
            _surface = SDL_GetWindowSurface(_screen);
            this->exit = false;
            this->_event_exit_done = false;
            // create thread to listen event
            if (_th)
                delete _th;
            _th = new thread::Thread(listen_event, this);
            _th->detach();
            opened = true;
            return err::ERR_NONE;
        }

        static void listen_event(void *args_in)
        {
            SDL_Display *disp = (SDL_Display *)args_in;
            SDL_Event event;
            while (!disp->exit)
            {
                int ret = SDL_WaitEventTimeout(&event,100);
                if(ret == 0){
                    continue;
                }
                if (event.type == SDL_QUIT)
                {
                    log::debug("SDL_QUIT\n");
                    disp->exit = true;
                    break;
                }
                touchscreen::TouchScreen_SDL::touch_event_handle(&event);
                if (disp->exit)
                    break;
            }
            disp->opened = false;
            SDL_DestroyWindow(disp->_screen);
            SDL_Quit();
            log::debug("SDL_Quit done\n");
            disp->_event_exit_done = true;
        }

        err::Err close()
        {
            this->exit = true;
            while (!this->_event_exit_done)
            {
                SDL_Delay(10);
            }
            if (_th)
            {
                delete _th;
                _th = nullptr;
            }
            opened = false;
            return err::ERR_NONE;
        }

        display::SDL_Display *add_channel(int width, int height, image::Format format)
        {
            return NULL;
        }

        bool is_opened()
        {
            return opened;
        }

        err::Err show(image::Image &img, image::Fit fit)
        {
            SDL_Rect rect;
            image::Image *img_ptr = &img;
            (void)fit;
            // center of screen
            rect.x = (_width - img_ptr->width()) / 2;
            rect.y = (_height - img_ptr->height()) / 2;
            rect.w = img_ptr->width();
            rect.h = img_ptr->height();
            image::Format format = img_ptr->format();
            int pix_size = 0;
            int line_size = 0;
            SDL_Surface *surface = NULL;
            if (format == image::FMT_RGBA8888)
            {
                pix_size = 4 * 8;
                line_size = img_ptr->width() * 4;
                surface = SDL_CreateRGBSurfaceFrom(img_ptr->data(), img_ptr->width(), img_ptr->height(), pix_size, line_size, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            }
            else if (format == image::FMT_BGRA8888)
            {
                pix_size = 4 * 8;
                line_size = img_ptr->width() * 4;
                surface = SDL_CreateRGBSurfaceFrom(img_ptr->data(), img_ptr->width(), img_ptr->height(), pix_size, line_size, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
            }
            else if (format == image::FMT_RGB888)
            {
                pix_size = 3 * 8;
                line_size = img_ptr->width() * 3;
                surface = SDL_CreateRGBSurfaceFrom(img_ptr->data(), img_ptr->width(), img_ptr->height(), pix_size, line_size, 0x0000FF, 0x00FF00, 0xFF0000, 0);
            }
            else if (format == image::FMT_BGR888)
            {
                pix_size = 3 * 8;
                line_size = img_ptr->width() * 3;
                surface = SDL_CreateRGBSurfaceFrom(img_ptr->data(), img_ptr->width(), img_ptr->height(), pix_size, line_size, 0xFF0000, 0x00FF00, 0x0000FF, 0);
            }
            else if (format == image::FMT_GRAYSCALE)
            {
                pix_size = 1 * 8;
                line_size = img_ptr->width() * 1;
                surface = SDL_CreateRGBSurfaceFrom(img_ptr->data(), img_ptr->width(), img_ptr->height(), pix_size, line_size, 0xFF, 0, 0, 0);
            }
            else
            {
                log::error("not support format: %d\n", format);
                return err::ERR_ARGS;
            }
            if (format == image::FMT_BGRA8888 || format == image::FMT_RGBA8888)
            {
                // clear image first
                SDL_FillRect(_surface, NULL, 0);
            }
            SDL_BlitSurface(surface, NULL, _surface, &rect);
            SDL_UpdateWindowSurface(_screen);
            SDL_FreeSurface(surface);
            return err::ERR_NONE;
        }

        void set_backlight(float value)
        {
            return;
        }

        float get_backlight()
        {
            return 0.0;
        }

        int get_ch_nums()
        {
            return 1;
        }

        /**
         * Set display mirror
         * @param en enable/disable mirror
         */
        virtual err::Err set_hmirror(bool en)
        {
            return err::Err::ERR_NONE;
        }

        /**
         * Set display flip
         * @param en enable/disable flip
         */
        virtual err::Err set_vflip(bool en)
        {
            return err::Err::ERR_NONE;
        }

    public:
        bool exit;
        bool opened;

    private:
        int _width;
        int _height;
        SDL_Window *_screen;
        SDL_Surface *_surface;
        thread::Thread *_th;
        bool _event_exit_done;
    };
}
