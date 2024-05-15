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
#include <string>
#include <stdexcept>
#include <pthread.h>

namespace maix::rtmp
{
    /**
     * Rtmp class
     * @maixpy maix.rtmp.Rtmp
     */
    class Rtmp {
        std::string _host;
        std::string _app;
        std::string _stream;
        int _port;

        int _socket;
        void *_handler;

        camera::Camera *_cam;
        thread::Thread *_thread;
        pthread_mutex_t _lock;
        bool _start;
        std::string _path;
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
         * @maixpy maix.rtmp.Rtmp.__init__
         * @maixcdk maix.rtmp.Rtmp.Rtmp
         */
        Rtmp(std::string host, int port = 1935, std::string app = std::string(), std::string stream = std::string());
        ~Rtmp();

        /**
         * @brief Push rtmp video data
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixcdk maix.rtmp.Rtmp.push_video
        */
        int push_video(void *data, size_t data_size, uint32_t timestamp);

        /**
         * @brief Push rtmp audio data
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixcdk maix.rtmp.Rtmp.push_audio
        */
        int push_audio(void *data, size_t data_size, uint32_t timestamp);

        /**
         * @brief Push rtmp script data
         * @return error code, err::ERR_NONE means success, others means failed
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
            _cam = cam;
            return err::ERR_NONE;
        }

        /**
         * @brief Start push stream
         * @note only support flv file now
         * @param path File path, if you passed file path, cyclic push the file, else if you bound camera, push the camera image.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtmp.Rtmp.start
        */
        err::Err start(std::string path = std::string());

        /**
         * @brief Stop push
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
         * @maixpy maix.rtmp.Rtmp.get_path
        */
        bool is_started() {
            return _start ? true : false;
        }
    };
}

#endif // __MAIX_RTMP_HPP
