/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.5.15: Add framework, create this file.
 */

#ifndef __MAIX_RTMP_HPP
#define __MAIX_RTMP_HPP

#include "maix_basic.hpp"
#include "maix_camera.hpp"
#include "maix_video.hpp"
#include <string>
#include <stdexcept>
#include <pthread.h>

/**
 * @brief maix.rtmp module
 * @maixpy maix.rtmp
*/
namespace maix::rtmp
{
    /**
     * Video type
     * @maixpy maix.rtmp.TagType
     */
    enum TagType
    {
        TAG_NONE,
        TAG_VIDEO,
        TAG_AUDIO,
        TAG_SCRIPT,
    };

    /**
     * Rtmp class
     * @maixpy maix.rtmp.Rtmp
     */
    class Rtmp {
        std::string _host;
        std::string _app;
        std::string _stream;
        int _port;
        int _bitrate;

        int _socket;
        void *_handler;

        camera::Camera *_camera = nullptr;
        video::Encoder *_video_encoder = nullptr;
        audio::Recorder *_audio_recorder = nullptr;
        display::Display *_display = nullptr;
        thread::Thread *_thread = nullptr;
        thread::Thread *_push_thread = nullptr;
        thread::Thread *_app_thread = nullptr;
        pthread_mutex_t _lock;
        bool _start;
        std::string _path;
        image::Image *_capture_image;
        bool _need_capture;

        void *_param;
    public:
        /**
         * @brief Construct a new Video object
         * @note Rtmp url : rtmp://host:prot/app/stream
         * example:
         *  r = Rtmp("localhost", 1935, "live", "stream")
         *  means rtmp url is rtmp://localhost:1935/live/stream
         * @param host rtmp ip
         * @param port rtmp port, default is 1935.
         * @param app rtmp app name
         * @param stream rtmp stream name
         * @param bitrate rtmp bitrate, default is 1000 * 1000
         * @maixpy maix.rtmp.Rtmp.__init__
         * @maixcdk maix.rtmp.Rtmp.Rtmp
         */
        Rtmp(std::string host = "localhost", int port = 1935, std::string app = std::string(), std::string stream = std::string(), int bitrate = 1000 * 1000);
        ~Rtmp();

        /**
         * @brief Get bitrate
         * @return bitrate
         * @maixpy maix.rtmp.Rtmp.bitrate
        */
        int bitrate() {
            return _bitrate;
        }

        /**
         * @brief Push rtmp video data
         * @return return 0 ok, other error
         * @maixcdk maix.rtmp.Rtmp.push_video
        */
        int push_video(void *data, size_t data_size, uint32_t timestamp);

        /**
         * @brief Push rtmp audio data
         * @return return 0 ok, other error
         * @maixcdk maix.rtmp.Rtmp.push_audio
        */
        int push_audio(void *data, size_t data_size, uint32_t timestamp);

        /**
         * @brief Push rtmp script data
         * @return return 0 ok, other error
         * @maixcdk maix.rtmp.Rtmp.push_script
        */
        int push_script(void *data, size_t data_size, uint32_t timestamp);

        /**
         * @brief Bind camera
         * @note If the cam object is bound, the cam object cannot be used elsewhere.
         * @param cam camera object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtmp.Rtmp.bind_camera
        */
        err::Err bind_camera(camera::Camera *cam) {
            _camera = cam;
            return err::ERR_NONE;
        }

        /**
         * @brief Bind audio recorder
         * @note If the audio_recorder object is bound, the audio_recorder object cannot be used elsewhere.
         * @param recorder audio_recorder object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtmp.Rtmp.bind_audio_recorder
        */
        err::Err bind_audio_recorder(audio::Recorder *recorder) {
            _audio_recorder = recorder;
            return err::ERR_NONE;
        }

        /**
         * @brief Bind display
         * @note If the display object is bound, the display object cannot be used elsewhere.
         * @param disaply display object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtmp.Rtmp.bind_display
        */
        err::Err bind_display(display::Display *display) {
            _display = display;
            return err::ERR_NONE;
        }

        /**
         * @brief If you bind a camera, return the camera object.
         * @return Camera object
         * @maixpy maix.rtmp.Rtmp.get_camera
        */
        camera::Camera *get_camera() {
            return _camera;
        }

        /**
         * @brief If you bind a camera, capture the image of the camera
         * @note The return value may be null, you must check whether the return value is null
         * @return Image object
         * @maixcdk maix.rtmp.Rtmp.capture
        */
        image::Image *capture() {
            err::check_raise(err::ERR_NOT_IMPL, "not support now!");
            // lock(300);
            // if (!_capture_image || !_capture_image->data()) {
            //     unlock();
            //     return NULL;
            // }
            // image::Image *new_image = new image::Image(_capture_image->width(), _capture_image->height(), _capture_image->format(),
            //     (uint8_t *)_capture_image->data(), _capture_image->data_size(), false);
            // unlock();
            // return new_image;
            return NULL;
        }

        /**
         * @brief Get pointer of capture image
         * @return Image object
        */
        image::Image *get_capture_image() {
            return _capture_image;
        }

        /**
         * @brief Set pointer of capture image
         * @return Image object
        */
        image::Image *set_capture_image(void *img) {
            _capture_image = (image::Image *)img;
            return _capture_image;
        }

        /**
         * @brief Start push stream
         * @note only support flv file now
         * @param path File path, if you passed file path, cyclic push the file, else if you bound camera, push the camera image.(This parameter has been deprecated)
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtmp.Rtmp.start
        */
        err::Err start(std::string path = std::string());

        /**
         * @brief Stop push stream
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtmp.Rtmp.stop
        */
        err::Err stop();

        /**
         * @brief Lock
         * @param time lock time, unit:ms
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixcdk maix.rtmp.Rtmp.lock
        */
        err::Err lock(uint32_t time);

        /**
         * @brief Unlock
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixcdk maix.rtmp.Rtmp.unlock
        */
        err::Err unlock();

        /**
         * @brief Get the file path of the push stream
         * @return file path
         * @maixpy maix.rtmp.Rtmp.get_path
        */
        std::string get_path() {
            return _path;
        }

        /**
         * @brief Check whether push streaming has started
         * @return If rtmp thread is running, returns true
         * @maixpy maix.rtmp.Rtmp.is_started
        */
        bool is_started() {
            return _start ? true : false;
        }

        /**
         * @brief get handler
         * @note DO NOT ADD TO MAIXPY
         * @return rtmp handler
        */
        void *get_handler() {
            return _handler;
        }
    };
}

#endif // __MAIX_RTMP_HPP
