/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#ifndef __MAIX_RTSP_HPP
#define __MAIX_RTSP_HPP

#include "maix_err.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "maix_video.hpp"
#include "maix_basic.hpp"

namespace maix::rtsp
{
    /**
     * The stream type of rtsp
     * @maixpy maix.rtsp.RtspStreamType
     */
    enum RtspStreamType
    {
        RTSP_STREAM_NONE = 0,  // format invalid
        RTSP_STREAM_H265,
    };

    /**
     * Rtsp class
     * @maixpy maix.rtsp.Rtsp
     */
    class Rtsp
    {
    public:
        /**
         * @brief Construct a new Video object
         * @param ip rtsp ip
         * @param port rtsp port
         * @param fps rtsp fps
         * @param stream_type rtsp stream type
         * @maixpy maix.rtsp.Rtsp.__init__
         * @maixcdk maix.rtsp.Rtsp.Rtsp
         */
        Rtsp(std::string ip = std::string(), int port = 8554, int fps = 30, rtsp::RtspStreamType stream_type = rtsp::RtspStreamType::RTSP_STREAM_H265);
        ~Rtsp();

        /**
         * @brief start rtsp
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtsp.Rtsp.start
        */
        err::Err start();

        /**
         * @brief stop rtsp
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtsp.Rtsp.start
        */
        err::Err stop();

        /**
         * @brief Bind camera
         * @param camera camera object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtsp.Rtsp.bind_camera
        */
        err::Err bind_camera(camera::Camera *camera);

        /**
         * @brief Write data to rtsp
         * @param type rtsp stream type
         * @param data rtsp stream data
         * @param fps rtsp stream data size
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.rtsp.Rtsp.write
        */
        err::Err write(video::VideoStream &stream);

        /**
         * @brief Get url of rtsp
         * @return url of rtsp
         * @maixpy maix.rtsp.Rtsp.get_url
        */
        std::string get_url();

        /**
         * @brief Get camera object from rtsp
         * @return camera object
         * @maixpy maix.rtsp.Rtsp.to_camera
        */
        camera::Camera *to_camera() {
            return this->_camera;
        }

        /**
         * @brief return rtsp start status
         * @return true means rtsp is start, false means rtsp is stop.
         * @maixpy maix.rtsp.Rtsp.rtsp_is_start
        */
        bool rtsp_is_start()
        {
            return this->_is_start;
        }
    private:
        std::string _ip;
        int _port;
        int _fps;
        rtsp::RtspStreamType _stream_type;
        bool _is_start;
        bool _bind_camera;
        camera::Camera *_camera;
        thread::Thread *_thread;
    };
} // namespace maix::rtsp

#endif // __MAIX_RTSP_HPP