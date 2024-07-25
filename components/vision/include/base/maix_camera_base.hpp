/**
 * @file maix_camera_base.hpp
 * @brief Maix camera SDL implementation
 * @author neucrack@sipeed.com
 * @license Apache 2.0 Sipeed Ltd
 * @update date 2023-10-23 Create by neucrack
*/

#pragma once

#include <vector>
#include "maix_image.hpp"
#include "maix_err.hpp"

namespace maix::camera
{
    class CameraBase
    {
    public:
        /**
         * @brief Construct a new Camera object
         * @param device camera device name, you can get devices by list_devices method, by default(value is NULL(None in MaixPy)) means the first device
         * @param width camera width, by default(value is -1) means auto detect,
         *              if width > max device supported width, will auto set to max device supported width
         * @param height camera height, by default(value is -1) means auto detect,
         *              if height > max device supported height, will auto set to max device supported height
         * @param format camera format, by default(value is FMT_RGB888)
         * @param buff_num camera buffer number, by default(value is 3)
         */
        CameraBase(const char *device = nullptr, int width = -1, int height = -1, image::Format format = image::Format::FMT_RGB888, int buff_num = 3){};

        /**
         * @brief Judge if the given format is supported by the camera
        */
        virtual bool is_support_format(image::Format format) = 0;

        /**
         * @brief open camera device
         * @param width camera width, by default(value is -1) means auto detect,
         *              if width > max device supported width, will auto set to max device supported width
         * @param height camera height, by default(value is -1) means auto detect,
         *              if height > max device supported height, will auto set to max device supported height
         * @param format camera format, by default(value is FMT_RGB888)
         * @param buff_num camera buffer number, by default(value is 3)
         * @return error code
        */
        virtual err::Err open(int width = -1, int height = -1, image::Format format = image::Format::FMT_RGB888, int buff_num = 3) = 0;

        /**
         * @brief read a frame from camera
         * @param buff buffer to store image data, if NULL, will alloc a new buffer
         * @return image data
        */
        virtual image::Image *read(void *buff = NULL, size_t buff_size = 0) = 0;

        /**
         * @brief close camera device
         * @return none
        */
        virtual void close() = 0;

        /**
         * Add a new channel and return a new Camera object, you can use close() to close this channel.
         * @param width camera width, default is -1, means auto, mostly means max width of camera support
         * @param height camera height, default is -1, means auto, mostly means max height of camera support
         * @param format camera output format, default is RGB888
         * @param buff_num camera buffer number, default is 3, means 3 buffer, one used by user, one used for cache the next frame,
         *                 more than one buffer will accelerate image read speed, but will cost more memory.
         * @return new Camera object
        */
        virtual camera::CameraBase *add_channel(int width = -1, int height = -1, image::Format format = image::FMT_RGB888, int buff_num = 3) = 0;

        /**
         * @brief clear all buffer
         * @return none
        */
        virtual void clear_buff() = 0;

        /**
         * @brief check camera device is opened or not
         * @return opened or not, bool type
        */
        virtual bool is_opened() = 0;

        /**
         * Get camera supported channels(layers)
         */
        virtual int get_ch_nums() = 0;

        /**
         * Get channel number of camera.
         */
        virtual int get_channel() = 0;

        /**
         * Set/Get camera mirror
         * @param en enable/disable mirror
        */
        virtual int hmirror(int en) = 0;

        /**
         * Set/Get camera flip
         * @param en enable/disable flip
        */
        virtual int vflip(int en) = 0;

        /**
         * Set/Get camera luma
         * @param int luma value
        */
        virtual int luma(int value) = 0;

        /**
         * Set/Get camera constrast
         * @param int constrast value
        */
        virtual int constrast(int value) = 0;

        /**
         * Set/Get camera saturation
         * @param int saturation value
        */
        virtual int saturation(int value) = 0;

        /**
         * Set/Get camera exposure
         * @param int constrast value
        */
        virtual int exposure(int value) = 0;

        /**
         * Set/Get camera gain
         * @param int saturation value
        */
        virtual int gain(int gain) = 0;

        /**
         * Set/Get awb mode
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value value = 0, means set awb to auto mode, value = 1, means set awb to manual mode, default is auto mode.
         * @return returns awb mode
        */
        virtual int awb_mode(int value = -1) = 0;

        /**
         * Set/Get exp mode
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value value = 0, means set exposure to auto mode, value = 1, means set exposure to manual mode, default is auto mode.
         * @return returns exposure mode
        */
        virtual int exp_mode(int value = -1) = 0;

        /**
         * Set window size of camera
         * @param roi Support two input formats, [x,y,w,h] set the coordinates and size of the window;
         * [w,h] set the size of the window, when the window is centred.
         * @return error code
        */
        virtual int set_windowing(std::vector<int> roi) = 0;
    };

}

