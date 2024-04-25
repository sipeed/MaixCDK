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
    Rtsp::Rtsp(std::string ip, int port, int fps, rtsp::RtspStreamType stream_type) {
        (void)ip;
        (void)port;
        (void)stream_type;
    }

    Rtsp::~Rtsp() {
    }

    err::Err Rtsp::start() {
        err::Err err = err::ERR_NONE;
        return err;
    }

    err::Err Rtsp::stop() {
        err::Err err = err::ERR_NONE;
        return err;
    }

    err::Err Rtsp::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NONE;
        (void)camera;
        return err;
    }

    err::Err Rtsp::write(video::VideoStream &stream) {
        err::Err err = err::ERR_NONE;
        (void)stream;
        return err;
    }

    std::string Rtsp::get_url() {
        return "not supported";
    }
}
