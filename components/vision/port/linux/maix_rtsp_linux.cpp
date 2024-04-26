/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_rtsp.hpp"
#include "maix_err.hpp"
#include <dirent.h>
#include <pthread.h>

namespace maix::rtsp
{
    Region::Region(int x, int y, int width, int height, image::Format format, camera::Camera *camera)
    {
        (void)x;
        (void)y;
        (void)width;
        (void)height;
        (void)format;
        (void)camera;
    }

    Region::~Region() {

    }

    image::Image *Region::get_canvas() {
        return NULL;
    }

    err::Err Region::update_canvas() {
        return err::ERR_NOT_IMPL;
    }

    Rtsp::Rtsp(std::string ip, int port, int fps, rtsp::RtspStreamType stream_type) {
        (void)ip;
        (void)port;
        (void)fps;
        (void)stream_type;
    }

    Rtsp::~Rtsp() {
    }

    err::Err Rtsp::start() {
        err::Err err = err::ERR_NOT_IMPL;
        return err;
    }

    err::Err Rtsp::stop() {
        err::Err err = err::ERR_NOT_IMPL;
        return err;
    }

    err::Err Rtsp::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NOT_IMPL;
        (void)camera;
        return err;
    }

    err::Err Rtsp::write(video::VideoStream &stream) {
        err::Err err = err::ERR_NOT_IMPL;
        (void)stream;
        return err;
    }

    std::string Rtsp::get_url() {
        return "not supported";
    }

    rtsp::Region *Rtsp::add_region(int x, int y, int width, int height, image::Format format) {
        (void)x;
        (void)y;
        (void)width;
        (void)height;
        (void)format;
        return NULL;
    }

    err::Err Rtsp::update_region(rtsp::Region &region) {
        return region.update_canvas();
    }
}
