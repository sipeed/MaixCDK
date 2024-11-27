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

    Rtsp::Rtsp(std::string ip, int port, int fps, rtsp::RtspStreamType stream_type, int bitrate) {
        (void)ip;
        (void)port;
        (void)fps;
        (void)stream_type;
        (void)bitrate;
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

    err::Err Rtsp::bind_audio_recorder(audio::Recorder *recorder) {
        err::Err err = err::ERR_NOT_IMPL;
        (void)recorder;
        return err;
    }

    err::Err Rtsp::write(video::Frame &stream) {
        err::Err err = err::ERR_NOT_IMPL;
        (void)stream;
        return err;
    }

    camera::Camera *Rtsp::to_camera() {
        err::check_null_raise(NULL, "The camera object is NULL");
        return NULL;
    }

    std::string Rtsp::get_url() {
        return "not supported";
    }

    std::vector<std::string> Rtsp::get_urls() {
        return std::vector<std::string>();
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

    err::Err Rtsp::del_region(rtsp::Region *region) {
        err::check_null_raise(region, "The region object is NULL");

        return err::ERR_NOT_IMPL;
    }

    err::Err Rtsp::draw_rect(int id, int x, int y, int width, int height, image::Color color, int thickness) {
        return err::ERR_NOT_IMPL;
    }

    err::Err Rtsp::draw_string(int id, int x, int y, const char *str, image::Color color, int size, int thickness)
    {
        return err::ERR_NOT_IMPL;
    }
}
