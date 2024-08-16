/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include <stdint.h>
#include "maix_err.hpp"
#include "maix_log.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_video.hpp"

namespace maix::video
{
#if CONFIG_BUILD_WITH_MAIXPY
    maix::image::Image *Video::NoneImage = new maix::image::Image();
    maix::image::Image *Encoder::NoneImage = new maix::image::Image();
#else
    maix::image::Image *Video::NoneImage = NULL;
    maix::image::Image *Encoder::NoneImage = NULL;
#endif

    Encoder::Encoder(int width, int height, image::Format format, VideoType type, int framerate, int gop, int bitrate, int time_base, bool capture) {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    Encoder::~Encoder() {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    err::Err Encoder::bind_camera(camera::Camera *camera) {
        (void)camera;
        throw err::Exception(err::ERR_NOT_IMPL);
        return err::ERR_NOT_IMPL;
    }

    video::Frame *Encoder::encode(image::Image *img) {
        (void)img;
        throw err::Exception(err::ERR_NOT_IMPL);
        return nullptr;
    }

    Decoder::Decoder(std::string path, image::Format format) {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    Decoder::~Decoder() {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    image::Image *Decoder::decode_video() {
        return NULL;
    }

    err::Err Decoder::seek(uint64_t timestamp) {
        throw err::Exception(err::ERR_NOT_IMPL);
        return err::ERR_NOT_IMPL;
    }

    Video::Video(std::string path, int width, int height, image::Format format, int time_base, int framerate, bool capture, bool open)
    {
        (void)path;
        (void)width;
        (void)height;
        (void)format;
        (void)time_base;
        (void)framerate;
        (void)capture;
        (void)open;
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    Video::~Video() {
        if (this->_is_opened) {
            this->close();
        }
    }

    err::Err Video::open(std::string path, double fps)
    {
        (void)path;
        (void)fps;
        throw err::Exception(err::ERR_NOT_IMPL);
        return err::ERR_NONE;
    }

    void Video::close()
    {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    err::Err Video::bind_camera(camera::Camera *camera) {
        (void)camera;
        err::Err err = err::ERR_NONE;
        throw err::Exception(err::ERR_NOT_IMPL);
        return err;
    }

    video::Packet *Video::encode(image::Image *img) {
        (void)img;
        return nullptr;
    }

    image::Image *Video::decode(video::Frame *frame) {
        (void)frame;
        return NULL;
    }

    err::Err Video::finish() {
        throw err::Exception(err::ERR_NOT_IMPL);
        return err::ERR_NOT_IMPL;
    }
} // namespace maix::video
