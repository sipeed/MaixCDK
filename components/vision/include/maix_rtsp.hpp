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
#include "maix_image.hpp"
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
     * Region class
     * @maixpy maix.rtsp.Region
     */
    class Region
    {
    public:
        /**
         * @brief Construct a new Region object
         * @param x region coordinate x
         * @param y region coordinate y
         * @param width region width
         * @param height region height
         * @param format region format
         * @param camera bind region to camera
         * @maixpy maix.rtsp.Region.__init__
         * @maixcdk maix.rtsp.Region.Region
         */
        Region(int x, int y, int width, int height, image::Format format, camera::Camera *camera);
        ~Region();

        /**
         * @brief Return an image object from region
         * @return image object
         * @maixpy maix.rtsp.Region.get_canvas
        */
        image::Image *get_canvas();

        /**
         * @brief Update canvas
         * @return error code
         * * @maixpy maix.rtsp.Region.update_canvas
        */
        err::Err update_canvas();
    private:
        int _id;
        int _x;
        int _y;
        int _width;
        int _height;
        bool _flip;
        bool _mirror;
        image::Format _format;
        image::Image *_image;
        camera::Camera *_camera;
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

        /**
         * @brief return a region object, you can draw image on the region.
         * @param x region coordinate x
         * @param y region coordinate y
         * @param width region width
         * @param height region height
         * @param format region format, support Format::FMT_BGRA8888 only
         * @return the reigon object
         * @maixpy maix.rtsp.Rtsp.add_region
        */
        rtsp::Region *add_region(int x, int y, int width, int height, image::Format format = image::Format::FMT_BGRA8888);

        /**
         * @brief update and show region
         * @return error code
         * @maixpy maix.rtsp.Rtsp.update_region
        */
        err::Err update_region(rtsp::Region &region);

        /**
         * @brief return region list
         * @attention DO NOT ADD THIS FUNC TO MAIXPY
         * @return return a list of region
        */
        std::vector<rtsp::Region *> get_regions() {
            return this->_region_list;
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
        std::vector<rtsp::Region *> _region_list;
        std::vector<bool> _region_update_flag;
    };
} // namespace maix::rtsp

#endif // __MAIX_RTSP_HPP