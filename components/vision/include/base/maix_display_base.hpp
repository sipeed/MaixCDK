/**
 * @file maix_display_base.hpp
 * @brief Maix display SDL implementation
 * @author neucrack@sipeed.com
 * @license Apache 2.0 Sipeed Ltd
 * @update date 2023-10-23 Create by neucrack
*/

#pragma once

#include <vector>
#include "maix_image.hpp"
#include "maix_err.hpp"

namespace maix::display
{
    class DisplayBase
    {
    public:
        /**
         * @brief Construct a new Display object
         * @param device display device name, you can get devices by list_devices method, by default(value is NULL(None in MaixPy)) means the first device
         * @param width display width, by default(value is -1) means auto detect,
         *              if width > max device supported width, will auto set to max device supported width
         * @param height display height, by default(value is -1) means auto detect,
         *              if height > max device supported height, will auto set to max device supported height
         * @param format display format, by default(value is FMT_RGB888)
         */
        DisplayBase(const char *device = nullptr, int width = -1, int height = -1, image::Format format = image::FMT_RGB888){};

        /**
         * Get display width
         * @return width
        */
        virtual int width() = 0;

        /**
         * Get display height
         * @return height
        */
        virtual int height() = 0;

        /**
         * Get display size
         * @return size A list type in MaixPy, [width, height]
        */
        virtual std::vector<int> size() = 0;

        /**
         * Get display format
         * @return format
        */
        virtual image::Format format() = 0;

        /**
         * @brief open display device
         * @return error code
        */
        virtual err::Err open(int width = -1, int height = -1, image::Format format = image::FMT_RGB888) = 0;

        /**
         * @brief close display device
         * @return error code
        */
        virtual err::Err close() = 0;

        /**
         * Add a new channel and return a new Display object, you can use close() to close this channel.
         * @param width display width, default is -1, means auto, mostly means max width of display support
         * @param height display height, default is -1, means auto, mostly means max height of display support
         * @param format display output format, default is RGB888
         * @return new Display object
        */
        virtual display::DisplayBase *add_channel(int width = -1, int height = -1, image::Format format = image::FMT_RGB888) = 0;

        /**
         * @brief check display device is opened or not
         * @return opened or not, bool type
        */
        virtual bool is_opened() = 0;

        /**
         * @brief show image on display device, and will also send to MaixVision work station if connected.
         * @param img image to show, image.Image object,
         *            if the size of image smaller than display size, will show in the center of display;
         *            if the size of image bigger than display size, will auto resize to display size and keep ratio, fill blank with black color.
         * @param fit image in screen fit mode, by default(value is image.FIT_CONTAIN), @see image.Fit for more details
         *            e.g. image.FIT_CONTAIN means resize image to fit display size and keep ratio, fill blank with black color.
         * @return error code
        */
        virtual err::Err show(image::Image &img, image::Fit fit = image::FIT_CONTAIN) = 0;

        /**
         * Set display backlight
         * @param value backlight value, float type, range is [0, 100]
        */
        virtual void set_backlight(float value) = 0;

        /**
         * Get display backlight
         * @return value backlight value, float type, range is [0, 100]
        */
        virtual float get_backlight() = 0;

        /**
         * Get display supported channels(layers)
         */
        virtual int get_ch_nums() = 0;

        /**
         * Set display mirror
         * @param en enable/disable mirror
        */
        virtual err::Err set_hmirror(bool en) = 0;

        /**
         * Set display flip
         * @param en enable/disable flip
        */
        virtual err::Err set_vflip(bool en) = 0;

    };
}

