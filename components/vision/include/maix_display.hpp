/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_tensor.hpp"
#include "maix_log.hpp"
#include "maix_image.hpp"
#include "maix_err.hpp"
#include "maix_display_base.hpp"
#include <stdlib.h>
#include <map>
#include <stdexcept>
#include <vector>
#include <opencv2/opencv.hpp>

/**
 * @brief maix.display module, control display device and show image on it
 * @maixpy maix.display
*/
namespace maix::display
{
    std::vector<std::string> list_devices();

    /**
     * Display class
     * @maixpy maix.display.Display
     */
    class Display
    {
    public:
        /**
         * @brief Construct a new Display object
         * @param width display width, by default(value is -1) means auto detect,
         *              if width > max device supported width, will auto set to max device supported width
         * @param height display height, by default(value is -1) means auto detect,
         *              if height > max device supported height, will auto set to max device supported height
         * @param device display device name, you can get devices by list_devices method, by default(value is NULL(None in MaixPy)) means the first device
         * @param open If true, display will automatically call open() after creation. default is true.
         * @maixpy maix.display.Display.__init__
         * @maixcdk maix.display.Display.Display
         */
        Display(int width = -1, int height = -1, image::Format format = image::FMT_RGB888, const char *device = nullptr, bool open = true);

        /**
         * @brief Construct a new Display object.
         * @attention DisplayBase * parameter need to be set manually, otherwise the operation of this object will be invalid.
         * @param device display device path, you can get devices by list_devices method, by default(value is NULL(None in MaixPy)) means the first device
         * @param base basic operation objects.
         * @param width display width, default is -1, means auto, mostly means max width of display support
         * @param height display height, default is -1, means auto, mostly means max height of display support
         * @param format display output format
         * @param open If true, display will automatically call open() after creation. default is true.
         * @maixcdk maix.display.Display.Display
         */
        Display(const char *device, DisplayBase *base, int width = -1, int height = -1, image::Format format = image::FMT_INVALID, bool open = true);

        ~Display();

        /**
         * Get display width
         * @return width
         * @maixpy maix.display.Display.width
        */
        int width();

        /**
         * Get display height
         * @param ch channel to get, by default(value is 0) means the first channel
         * @return height
         * @maixpy maix.display.Display.height
        */
        int height();

        /**
         * Get display size
         * @param ch channel to get, by default(value is 0) means the first channel
         * @return size A list type in MaixPy, [width, height]
         * @maixpy maix.display.Display.size
        */
        std::vector<int> size();

        /**
         * Get display format
         * @return format
         * @maixpy maix.display.Display.format
        */
        image::Format format();

        /**
         * @brief open display device, if already opened, will return err.ERR_NONE.
         * @param width display width, default is -1, means auto, mostly means max width of display support
         * @param height display height, default is -1, means auto, mostly means max height of display support
         * @param format display output format, default is RGB888
         * @return error code
         * @maixpy maix.display.Display.open
        */
        err::Err open(int width = -1, int height = -1, image::Format format = image::FMT_INVALID);

        /**
         * @brief close display device
         * @return error code
         * @maixpy maix.display.Display.close
        */
        err::Err close();

        /**
         * Add a new channel and return a new Display object, you can use close() to close this channel.
         * @param width display width, default is -1, means auto, mostly means max width of display support
         * @param height display height, default is -1, means auto, mostly means max height of display support
         * @param format display output format, default is RGB888
         * @param open If true, display will automatically call open() after creation. default is true.
         * @return new Display object
         * @maixpy maix.display.Display.add_channel
        */
        display::Display *add_channel(int width = -1, int height = -1, image::Format format = image::FMT_RGB888, bool open = true);

        /**
         * @brief check display device is opened or not
         * @return opened or not, bool type
         * @maixpy maix.display.Display.is_opened
        */
        bool is_opened();

        /**
         * @brief check display device is closed or not
         * @return closed or not, bool type
         * @maixpy maix.display.Display.is_closed
        */
        bool is_closed() { return !is_opened();}

        /**
         * @brief show image on display device, and will also send to MaixVision work station if connected.
         * @param img image to show, image.Image object,
         *            if the size of image smaller than display size, will show in the center of display;
         *            if the size of image bigger than display size, will auto resize to display size and keep ratio, fill blank with black color.
         * @param fit image in screen fit mode, by default(value is image.FIT_CONTAIN), @see image.Fit for more details
         *            e.g. image.FIT_CONTAIN means resize image to fit display size and keep ratio, fill blank with black color.
         * @return error code
         * @maixpy maix.display.Display.show
        */
        err::Err show(image::Image &img, image::Fit fit = image::FIT_CONTAIN);

        /**
         * Get display device path
         * @return display device path
         * @maixpy maix.display.Display.device
        */
        std::string device()
        {
            return _device;
        }

        /**
         * Set display backlight
         * @param value backlight value, float type, range is [0, 100]
         * @maixpy maix.display.Display.set_backlight
        */
        void set_backlight(float value);

        /**
         * Get display backlight
         * @return value backlight value, float type, range is [0, 100]
         * @maixpy maix.display.Display.get_backlight
        */
        float get_backlight();

        /**
         * Get display supported channels(layers)
         */
        int get_ch_nums();

        /**
         * Set display mirror
         * @param en enable/disable mirror
         * @maixpy maix.display.Display.set_hmirror
        */
        err::Err set_hmirror(bool en);

        /**
         * Set display flip
         * @param en enable/disable flip
         * @maixpy maix.display.Display.set_vflip
        */
        err::Err set_vflip(bool en);
    private:
        std::string _device;
        DisplayBase *_impl; // pointer for implement usage
    };


    /**
     * Send image to MaixVision work station if connected.
     * If you want to debug your program an don't want to initialize display, use this method.
     * @param img image to send, image.Image object
     * @maixpy maix.display.send_to_maixvision
    */
    void send_to_maixvision(image::Image &img);
}
