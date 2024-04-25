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
     * VideoStream class
     * @maixpy maix.video.VideoStream
     */
    class VideoStream
    {
    public:
        VideoType type;
        std::unique_ptr<uint8_t> frame;
    };

    /**
     * Encode class
     * @maixpy maix.video.Encode
     */
    class Encode
    {
    public:
        /**
         * @brief Construct a new encode object
         * @param width encode image width, default is -1, means auto, mostly means max width of encode support
         * @param height encode image height, default is -1, means auto, mostly means max height of encode support
         * @param type video type, default is VideoType::VIDEO_ENC_H265_CBR
         */
        Encode(int width = -1, int height = -1, video::VideoType type = video::VideoType::VIDEO_ENC_H265_CBR);
        ~Encode();

        /**
         * @brief Open encode
         * @param width encode image width, default is -1, means auto, mostly means max width of encode support
         * @param height encode image height, default is -1, means auto, mostly means max height of encode support
         * @param type video type, default is VideoType::VIDEO_ENC_H265_CBR
         * @return error code, err::ERR_NONE means success, others means failed
        */
        err::Err open(int width = -1, int height = -1, video::VideoType type = video::VideoType::VIDEO_ENC_H265_CBR);

        /**
         * @brief Close encode
         * @return none
        */
        void close();

        /**
         * @brief Encode image
         * @param img image. @see image::Image
         * @return the result of encode. @see video::VideoStream
        */
        video::VideoStream encode(image::Image &img);

        /**
         * @brief check video device is opened or not
         * @return opened or not, bool type
        */
        bool is_opened() {
            return _is_opened;
        }
    private:
        bool _is_opened;

        int _pre_width;
        int _pre_height;
        video::VideoType _pre_video_type;

        int _width;
        int _height;
        video::VideoType _video_type;
    };

    /**
     * Decode class
     * @maixpy maix.video.Decode
     */
    class Decode
    {
    public:
        /**
         * @brief Construct a new decode object
         * @param width decode image width, default is -1, means auto, mostly means max width of decode support
         * @param height decode image height, default is -1, means auto, mostly means max height of decode support
         * @param type decode image type, default is VideoType::VIDEO_DEC_H265_CBR
         */
        Decode(int width = -1, int height = -1, video::VideoType type = video::VideoType::VIDEO_DEC_H265_CBR);
        ~Decode();

        /**
         * @brief Construct a new decode object
         * @param width decode image width, default is -1, means auto, mostly means max width of decode support
         * @param height decode image height, default is -1, means auto, mostly means max height of decode support
         * @param type decode image type, default is VideoType::VIDEO_DEC_H265_CBR
         */
        err::Err open(int width = -1, int height = -1, video::VideoType type = video::VideoType::VIDEO_ENC_H265_CBR);

        /**
         * @brief Close decode
         * @return none
        */
        void close();

        /**
         * @brief Decode image
         * @param stream the video stream. @see video::VideoStream
         * @return the result of encode. @see image::Image
        */
        image::Image *decode(video::VideoStream &stream);

        /**
         * @brief check decode is opened or not
         * @return opened or not, bool type
        */
        bool is_opened() {
            return _is_opened;
        }
    private:
        bool _is_opened;
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
         * @param path video path. if record is true, xxx.h265 means video format is H265, xxx.mp4 means video format is MP4
         * @param record If record is true, means record vide. if record is false means play video, default is false.
         * @param interval record interval. unit: us
         * @param width video width, default is -1, means auto, mostly means max width of video support
         * @param height video height, default is -1, means auto, mostly means max height of video support
         * @param audio If audio is true, means record with audio. default is false.
         * @param sample_rate audio sample rate, default is 44100.
         * @param channel audio channel, default is 1.
         * @param open If true, vido will automatically call open() after creation. default is true.
         * @maixpy maix.video.Video.__init__
         * @maixcdk maix.video.Video.Video
         */
        Video(std::string path = std::string(), bool record = false, int interval = 33333, int width = -1, int height = -1, bool audio = false, int sample_rate = 44100, int channel = 1, bool open = true);
        ~Video();

        /**
         * Open video and run
         * @param path video path. if record is true, xxx.h265 means video format is H265, xxx.mp4 means video format is MP4
         * @param record If record is true, means record vide. if record is false means play video, default is false.
         * @param interval record interval. unit: us
         * @param width video width, default is -1, means auto, mostly means max width of video support
         * @param height video height, default is -1, means auto, mostly means max height of video support
         * @param audio If audio is true, means record with audio. default is false.
         * @param sample_rate audio sample rate, default is 44100.
         * @param channel audio channel, default is 1.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.open
         */
        err::Err open(std::string path = std::string(), bool record = false, int interval = 33333, int width = -1, int height = -1, bool audio = false, int sample_rate = 44100, int channel = 1);

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
         * @brief start record video
         * @param record_time record video time, unit: ms.
         * If record_time = -1, mean record will not auto stop until record_finish() is called.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.record_start
        */
        err::Err record_start(uint64_t record_time = -1);

        /**
         * @brief stop record video
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.record_finish
        */
        err::Err record_finish();

        /**
         * @brief stop record video
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.capture
        */
        image::Image *capture();

        /**
         * Encode image
         * @param img the image will be encode
         * @return encode result
         * @maixpy maix.video.Video.encode
        */
        video::VideoStream encode(image::Image &img);

        /**
         * Decode image
         * @param img the image will be decode
         * @return decode result
         * @maixpy maix.video.Video.decode
        */
        image::Image *decode(video::VideoStream &stream);

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

        /**
         * Camera thread
         * @attention DO NOT USE THIS FUNCTION DO ANYTHING!
         * @return none
        */
        void camera_thread();
    private:
        std::string _pre_path;
        bool _pre_record;
        int _pre_interval;
        int _pre_width;
        int _pre_height;
        bool _pre_audio;
        int _pre_sample_rate;
        int _pre_channel;

        std::string _path;
        std::string _tmp_path;
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
    };
}

