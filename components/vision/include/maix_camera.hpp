/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_tensor.hpp"
#include "maix_log.hpp"
#include "maix_pipeline.hpp"
#include "maix_image.hpp"
#include "maix_err.hpp"
#include <stdlib.h>
#include <map>
#include <stdexcept>
#include <vector>
#include <opencv2/opencv.hpp>

/**
 * @brief maix.camera module, access camera device and get image from it
 * @maixpy maix.camera
*/
namespace maix::camera
{
    /**
     * List all supported camera devices.
     * @return Returns the path to the camera device.
     * @maixpy maix.camera.list_devices
    */
    std::vector<std::string> list_devices();

    /**
     * Enable set camera registers, default is false, if set to true, will not set camera registers, you can manually set registers by write_reg API.
     * @param enable enable/disable set camera registers
     * @maixpy maix.camera.set_regs_enable
    */
    void set_regs_enable(bool enable = true);

    /**
     * Get device name. Most of the time, the returned name is the name of the sensor.
     * @maixpy maix.camera.get_device_name
    */
    std::string get_device_name();

    /**
     * Get sensor size
     * @return Return a list of sensor sizes, the format is [w, h].
     * @maixpy maix.camera.get_sensor_size
    */
    std::vector<int> get_sensor_size();

    /**
     * Auto exposure mode
     * @maixpy maix.camera.AeMode
    */
    enum class AeMode {
        Invalid = -1,
        Auto = 0,
        Manual = 1
    };

    /**
     * Auto white balance mode
     * @maixpy maix.camera.AwbMode
    */
    enum class AwbMode {
        Invalid = -1,
        Auto = 0,
        Manual = 1
    };

    /**
     * Camera class
     * @maixpy maix.camera.Camera
     */
    class Camera
    {
    public:
        /**
         * @brief Construct a new Camera object.
         * Maximum resolution support 2560x1440.
         * @param width camera width, default is -1, means auto, mostly means max width of camera support
         * @param height camera height, default is -1, means auto, mostly means max height of camera support
         * @param format camera output format, default is image.Format.FMT_RGB888
         * @param device camera device path, you can get devices by list_devices method, by default(value is NULL(None in MaixPy)) means the first device
         * @param fps camera fps, default is -1, means auto, mostly means max fps of camera support
         * @param buff_num camera buffer number, default is 3, means 3 buffer, one used by user, one used for cache the next frame,
         *                 more than one buffer will accelerate image read speed, but will cost more memory.
         * @param open If true, camera will automatically call open() after creation. default is true.
         * @param raw If true, you can use read_raw() to capture the raw image output from the sensor.
         * @maixpy maix.camera.Camera.__init__
         * @maixcdk maix.camera.Camera.Camera
         */
        Camera(int width = -1, int height = -1, image::Format format = image::FMT_RGB888, const char *device = nullptr, double fps = -1, int buff_num = 3, bool open = true, bool raw = false);
        ~Camera();

        /**
         * Get the number of channels supported by the camera.
         * @return Returns the maximum number of channels.
         * @maixpy maix.camera.Camera.get_ch_nums
         */
        int get_ch_nums();

        /**
         * Open camera and run
         * @param width camera width, default is -1, means auto, mostly means max width of camera support
         * @param height camera height, default is -1, means auto, mostly means max height of camera support
         * @param format camera output format, default same as the constructor's format argument
         * @param fps camera fps, default is -1, means auto, mostly means max fps of camera support
         * @param buff_num camera buffer number, default is 3, means 3 buffer, one used by user, one used for cache the next frame,
         *                 more than one buffer will accelerate image read speed, but will cost more memory.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.camera.Camera.open
         */
        err::Err open(int width = -1, int height = -1, image::Format format = image::FMT_INVALID, double fps = -1, int buff_num = -1);

        /**
         * Get one frame image from camera buffer, must call open method before read.
         * If open method not called, will call it automatically, if open failed, will throw exception!
         * So call open method before read is recommended.
         * @param buff buffer to store image data, if buff is nullptr, will alloc memory automatically.
         *             In MaixPy, default to None, you can create a image.Image object, then pass img.data() to buff.
         * @param block block read, default is true, means block util read image successfully,
         *              if set to false, will return nullptr if no image in buffer
         * @param block_ms block read timeout
         * @return image::Image object, if failed, return nullptr, you should delete if manually in C++
         * @maixpy maix.camera.Camera.read
        */
        image::Image *read(void *buff = nullptr, size_t buff_size = 0, bool block = true, int block_ms = -1);

        /**
         * Read the raw image and obtain the width, height, and format of the raw image through the returned Image object.
         * @note The raw image is in a Bayer format, and its width and height are affected by the driver. Modifying the size and format is generally not allowed.
         * @return image::Image object, if failed, return nullptr, you should delete if manually in C++
         * @maixpy maix.camera.Camera.read_raw
        */
        image::Image *read_raw();

        /**
         * Clear buff to ensure the next read image is the latest image
         * @maixpy maix.camera.Camera.clear_buff
        */
        void clear_buff();

        /**
         * Read some frames and drop, this is usually used avoid read not stable image when camera just opened.
         * @param num number of frames to read and drop
         * @maixpy maix.camera.Camera.skip_frames
        */
        void skip_frames(int num);

        /**
         * Close camera
         * @maixpy maix.camera.Camera.close
        */
        void close();

        /**
         * Add a new channel and return a new Camera object, you can use close() to close this channel.
         * @param width camera width, default is -1, means auto, mostly means max width of camera support
         * @param height camera height, default is -1, means auto, mostly means max height of camera support
         * @param format camera output format, default is RGB888
         * @param fps camera fps, default is -1, means auto, mostly means max fps of camera support
         * @param buff_num camera buffer number, default is 3, means 3 buffer, one used by user, one used for cache the next frame,
         *                 more than one buffer will accelerate image read speed, but will cost more memory.
         * @param open If true, camera will automatically call open() after creation. default is true.
         * @return new Camera object
         * @maixpy maix.camera.Camera.add_channel
        */
        camera::Camera *add_channel(int width = -1, int height = -1, image::Format format = image::FMT_RGB888, double fps = -1, int buff_num = 3, bool open = true);

        /**
         * Check if camera is opened
         * @return true if camera is opened, false if not
         * @maixpy maix.camera.Camera.is_opened
        */
        bool is_opened();

        /**
         * @brief check camera device is closed or not
         * @return closed or not, bool type
         * @maixpy maix.camera.Camera.is_closed
        */
        bool is_closed() { return !is_opened();}

        /**
         * Get camera width
         * @return camera width
         * @maixpy maix.camera.Camera.width
        */
        int width()
        {
            return _width;
        }

        /**
         * Get camera height
         * @return camera height
         * @maixpy maix.camera.Camera.height
        */
        int height()
        {
            return _height;
        }

        /**
         * Get camera fps
         * @return camera fps
         * @maixpy maix.camera.Camera.fps
        */
        double fps()
        {
            return _fps;
        }

        /**
         * Get camera output format
         * @return camera output format, image::Format object
         * @maixpy maix.camera.Camera.format
        */
        image::Format format()
        {
            return _format;
        }

        /**
         * Get camera buffer number
         * @return camera buffer number
         * @maixpy maix.camera.Camera.buff_num
        */
        int buff_num()
        {
            return _buff_num;
        }

        /**
         * Set/Get camera horizontal mirror
         * @return camera horizontal mirror
         * @maixpy maix.camera.Camera.hmirror
         */
        int hmirror(int value = -1);

        /**
         * Set/Get camera vertical flip
         * @return camera vertical flip
         * @maixpy maix.camera.Camera.vflip
        */
        int vflip(int value = -1);

        /**
         * Get camera device path
         * @return camera device path
         * @maixpy maix.camera.Camera.device
        */
        std::string device()
        {
            return _device;
        }

        /**
         * Write camera register
         * @param addr register address
         * @param data register data
         * @param bit_width register data bit width, default is 8
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.camera.Camera.write_reg
         */
        err::Err write_reg(int addr, int data, int bit_width = 8);

        /**
         * Read camera register
         * @param addr register address
         * @return register data, -1 means failed
         * @param bit_width register data bit width, default is 8
         * @maixpy maix.camera.Camera.read_reg
         */
        int read_reg(int addr, int bit_width = 8);

        /**
         * Camera output color bar image for test
         * @param enable enable/disable color bar
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.camera.Camera.show_colorbar
         */
        err::Err show_colorbar(bool enable);

        /**
         * Get channel of camera
         * @return channel number
         * @maixpy maix.camera.Camera.get_channel
         */
        int get_channel();

        /**
         * Set camera resolution
         * @param width new width
         * @param height new height
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.camera.Camera.set_resolution
        */
        err::Err set_resolution(int width, int height);

        /**
         * Set camera fps
         * @param fps new fps
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.camera.Camera.set_fps
        */
        err::Err set_fps(double fps);

        /**
         * Set/Get camera exposure
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value exposure time. unit: us
         * If value == -1, return exposure time.
         * If value != 0, set and return exposure time.
         * @return camera exposure time
         * @maixpy maix.camera.Camera.exposure
        */
        int exposure(int value = -1);

        /**
         * Set/Get camera gain
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value camera gain.
         * If value == -1, returns camera gain.
         * If value != 0, set and return camera gain.
         * @return camera gain
         * @maixpy maix.camera.Camera.gain
        */
        int gain(int value = -1);

        /**
         * Set/Get camera iso
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value camera iso.
         * If value == -1, returns camera iso.
         * If value != 0, set and return camera iso.
         * @return camera iso
         * @maixpy maix.camera.Camera.iso
        */
        int iso(int value = -1);

        /**
         * Set/Get camera luma
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value luma value, range is [0, 100]
         * If value == -1, returns luma value.
         * If value != 0, set and return luma value.
         * @return returns luma value
         * @maixpy maix.camera.Camera.luma
        */
        int luma(int value = -1);

        /**
         * Set/Get camera constrast
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value constrast value, range is [0, 100]
         * If value == -1, returns constrast value.
         * If value != 0, set and return constrast value.
         * @return returns constrast value
         * @maixpy maix.camera.Camera.constrast
        */
        int constrast(int value = -1);

        /**
         * Set/Get camera saturation
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value saturation value, range is [0, 100]
         * If value == -1, returns saturation value.
         * If value != 0, set and return saturation value.
         * @return returns saturation value
         * @maixpy maix.camera.Camera.saturation
        */
        int saturation(int value = -1);

        /**
         * Set/Get white balance mode (deprecated interface)
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * This interface may be deprecated in the future, and there may be incompatibilities in the definition of the parameters of the new interface
         * @param value value = 0, means set white balance to auto mode, value = 1, means set white balance to manual mode, default is auto mode.
         * @return returns awb mode
         * @maixpy maix.camera.Camera.awb_mode
        */
        camera::AwbMode awb_mode(camera::AwbMode value = camera::AwbMode::Invalid);

        /**
         * Set/Get white balance mode (deprecated interface)
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * @param value value = 0, means set white balance to manual mode, value = 1, means set white balance to auto mode, default is auto mode.
         * @return returns awb mode
         * @maixpy maix.camera.Camera.set_awb
        */
        int set_awb(int mode = -1);

        /**
         * @brief This interface is used to manually set the white balance gains and disable auto white balance, you can re-enable auto white balance using awb_mode().
         * @note MaixCam can only manually set white balance by adjusting the gain.
         * @param gains This is a float array representing the gains for r, gr, gb, and b respectively, with a value range of 0 to 1.0.
         * For MaixCam, the recommended initial values are [0.134, 0.0625, 0.0625, 0.1239]
         * For MaixCam2, the recommended initial values are[0.0682, 0, 0, 0.04897]
         * If no parameters are passed, the current gain values will be returned.
         * @return Returns the current gain values.
         * @maixpy maix.camera.Camera.set_wb_gain
        */
        std::vector<float> set_wb_gain(std::vector<float> gains = std::vector<float>());

        /**
         * Set/Get exposure mode (deprecated interface)
         * @attention This method will affect the isp and thus the image, so please be careful with it.
         * This interface may be deprecated in the future, and there may be incompatibilities in the definition of the parameters of the new interface
         * @param value value = 0, means set exposure to auto mode, value = 1, means set exposure to manual mode, default is auto mode.
         * @return returns exposure mode
         * @maixpy maix.camera.Camera.exp_mode
        */
        camera::AeMode exp_mode(camera::AeMode value = camera::AeMode::Invalid);

        /**
         * Set window size of camera
         * @param roi Support two input formats, [x,y,w,h] set the coordinates and size of the window;
         * [w,h] set the size of the window, when the window is centred.
         * @return error code
         * @maixpy maix.camera.Camera.set_windowing
        */
        err::Err set_windowing(std::vector<int> roi);

        /**
         * Get sensor size
         * @return Return a list of sensor sizes, the format is [w, h].
        */
        std::vector<int> get_sensor_size();

        /**
         * Get get the driver of camera
         * @return camera driver
        */
        void *get_driver();

        /**
         * Retrieve camera data from the internal buffer without copying.
         * @note You must use this interface very carefully. If it returns null, it means no image was captured.
         * If it returns a frame, you must explicitly release the frame after use; otherwise, it may block the next image capture.
         * For Python users, call `del frame` to release it; for C++ users, call `delete frame`.
         * @param block_ms block read, if block_ms = -1, block indefinitely until an image is read; if block_ms = 0, do not block and return immediately
         * @return Return image frame
         * @maixcdk maix.camera.Camera.pop
        */
        pipeline::Frame *pop(int block_ms = 1000);
    private:
        std::string _device;
        int _ch;
        int _width;
        int _height;
        double _fps;
        int _buff_num;
        image::Format _format;
        image::Format _format_impl; // used by implement code and need convert to _format
        int _hmirror;
        int _vflip;
        float _exposure;
        float _gain;
        bool _show_colorbar;
        bool _open_set_regs;
        bool _check_format(image::Format format);
        uint64_t _last_read_us;
        bool _invert_flip;
        bool _invert_mirror;
        bool _is_opened;
        void *_param;
    };
}
