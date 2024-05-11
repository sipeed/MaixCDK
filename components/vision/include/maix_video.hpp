/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_display.hpp"
#include "maix_camera.hpp"
#include <memory>

/**
 * @brief maix.video module
 * @maixpy maix.video
*/
namespace maix::video
{
    extern maix::image::Image NoneImage;

    /**
     * Video type
     * @maixpy maix.video.VideoType
     */
    enum VideoType
    {
        VIDEO_NONE = 0,  // format invalid
        VIDEO_ENC_H265_CBR,
        VIDEO_ENC_MP4_CBR,
        VIDEO_DEC_H265_CBR,
        VIDEO_DEC_MP4_CBR,
    };

    /**
     * Frame class
     * @maixpy maix.video.Frame
     */
    class Frame
    {
    public:
        VideoType type;
        std::unique_ptr<uint8_t> frame;
    };

    /**
     * Packet class
     * @maixpy maix.video.Packet
    */
    class Packet
    {
        uint64_t _pts;                        // unit: time_base
        uint64_t _dts;                        // unit: time_base
        uint64_t _duration;                   // equals next_pts - this_pts in presentation order. unit: time_base.
        uint8_t *_data;
        size_t _data_size;
    public:
        /**
         * @brief Packet number (pair of numerator and denominator).
         * @param data src data pointer, use pointers directly without copying.
         * Note: this object will try to free this memory
         * @param len data len
         * @param pts presentation time stamp. unit: time_base
         * @param dts decoding time stamp. unit: time_base
         * @param duration packet display time. unit: time_base
         * @maixpy maix.video.Packet.__init__
         * @maixcdk maix.video.Packet.Packet
         */
        Packet(uint8_t *data, int len, uint64_t pts = -1, uint64_t dts = -1, int64_t duration = 0) {
            _data = data;
            _data_size = _data ? len : 0;
            _pts = pts;
            _dts = dts;
            _duration = duration;
        }

        /**
         * @brief Packet number (pair of numerator and denominator).
         * @maixcdk maix.video.Packet.Packet
         */
        Packet() {
            _data = NULL;
            _data_size = 0;
        }

        ~Packet() {
            if (_data) {
                free(_data);
                _data = NULL;
            }
        }

        /**
         * @brief Get raw data of packet
         * @return raw data
         * @maixpy maix.video.Packet.get
         */
        std::vector<uint8_t> get() {
            std::vector<uint8_t> vec(_data, _data + _data_size);
            return vec;
        }

        /**
         * @brief Get raw data of packet
         * @return raw data
         * @maixpy maix.video.Packet.data
         */
        uint8_t *data() {
            return _data;
        }

        /**
         * @brief Get raw data size of packet
         * @return size of raw data
         * @maixpy maix.video.Packet.data_size
         */
        size_t data_size() {
            return _data_size;
        }

        /**
         * @brief Check packet is valid
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.is_valid
         */
        bool is_valid() {
            return (_data && _data_size != 0) ? true : false;
        }

        /**
         * @brief Set pts
         * @param pts presentation time stamp. unit: time_base
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.set_pts
         */
        void set_pts(uint64_t pts) {
            _pts = pts;
        }

        /**
         * @brief Set dts
         * @param dts decoding time stamp.  unit: time_base
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.set_dts
         */
        void set_dts(uint64_t dts) {
            _dts = dts;
        }

        /**
         * @brief Set duration
         * @param duration packet display time. unit: time_base
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.set_duration
         */
        void set_duration(uint64_t duration) {
            _duration = duration;
        }
    };

    /**
     * Video class
     * @maixpy maix.video.Video
     */
    class Video
    {
    public:
        /**
         * @brief Construct a new Video object
         * @param path video path. the path determines the location where you load or save the file, if path is none, the video module will not save or load file.
         * xxx.h265 means video format is H265, xxx.mp4 means video format is MP4
         * @param width picture width. this value may be set automatically. default is 2560.
         * @param height picture height. this value may be set automatically. default is 1440.
         * @param format picture pixel format. this value may be set automatically. default is FMT_YVU420SP.
         * @param time_base frame time base. time_base default is 30, means 1/30 ms
         * @param framerate frame rate. framerate default is 30, means 30 frames per second
         * for video. 1/time_base is not the average frame rate if the frame rate is not constant.
         * @param capture enable capture, if true, you can use capture() function to get an image object
         * @param open If true, video will automatically call open() after creation. default is true.
         * @maixpy maix.video.Video.__init__
         * @maixcdk maix.video.Video.Video
         */
        Video(std::string path = std::string(), int width = 2560, int height = 1440, image::Format format = image::Format::FMT_YVU420SP, int time_base = 30, int framerate = 30, bool capture = false, bool open = true);
        ~Video();

        /**
         * Open video and run
         * @param path video path. the path determines the location where you load or save the file, if path is none, the video module will not save or load file.
         * xxx.h265 means video format is H265, xxx.mp4 means video format is MP4
         * @param fps video fps
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.open
         */
        err::Err open(std::string path = std::string(), double fps = 30.0);

        /**
         * Close video
         * @maixpy maix.video.Video.close
        */
        void close();

        /**
         * @brief Bind camera
         * @param camera camera object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.bind_camera
        */
        err::Err bind_camera(camera::Camera *camera);

        /**
         * Encode image.
         * @param img the image will be encode.
         * if the img is NULL, this function will try to get image from camera, you must use bind_camera() function to bind the camera.
         * @return encode result
         * @maixpy maix.video.Video.encode
        */
        video::Packet *encode(image::Image *img = &maix::video::NoneImage);

        /**
         * Decode frame
         * @param frame the frame will be decode
         * @return decode result
         * @maixpy maix.video.Video.decode
        */
        image::Image *decode(video::Frame *frame = nullptr);

        /**
         * Encode or decode finish
         * @return error code
         * @maixpy maix.video.Video.finish
        */
        err::Err finish();

        /**
         * Capture image
         * @attention Each time encode is called, the last captured image will be released.
         * @return error code
         * @maixpy maix.video.Video.capture
        */
        image::Image *capture() {
            err::check_null_raise(_capture_image, "Can't capture image, please make sure the capture flag is set, and run this api after encode().");
            image::Image *new_image = new image::Image(_capture_image->width(), _capture_image->height(), _capture_image->format(),
                (uint8_t *)_capture_image->data(), _capture_image->data_size(), false);
            return new_image;
        }

        /**
         * Check if video is recording
         * @return true if video is recording, false if not
         * @maixpy maix.video.Video.is_recording
        */
        bool is_recording() {
            return this->_is_recording;
        }

        /**
         * Check if video is opened
         * @return true if video is opened, false if not
         * @maixpy maix.video.Video.is_opened
        */
        bool is_opened() {
            return this->_is_opened;
        }

        /**
         * @brief check video device is closed or not
         * @return closed or not, bool type
         * @maixpy maix.video.Video.is_closed
        */
        bool is_closed() { return !is_opened();}

        /**
         * Get video width
         * @return video width
         * @maixpy maix.video.Video.width
        */
        int width()
        {
            return _width;
        }

        /**
         * Get video height
         * @return video height
         * @maixpy maix.video.Video.height
        */
        int height()
        {
            return _height;
        }
    private:
        std::string _pre_path;
        double _pre_fps;
        bool _pre_record;
        int _pre_interval;
        int _pre_width;
        int _pre_height;
        bool _pre_audio;
        int _pre_sample_rate;
        int _pre_channel;

        std::string _path;
        std::string _tmp_path;
        double _fps;
        bool _record;
        int _interval;
        int _width;
        int _height;
        bool _audio;
        int _sample_rate;
        int _channel;

        bool _bind_camera;
        bool _is_recording;
        camera::Camera *_camera;
        thread::Thread *_thread;
        video::VideoType _video_type;
        bool _is_opened;
        uint64_t _record_ms;
        uint64_t _record_start_ms;
        int _fd;
        bool _need_capture;
        image::Image *_capture_image;

        bool _need_auto_config;
        int _time_base;
        int _framerate;
        fs::File file;
        uint64_t _last_pts;
    };
}

