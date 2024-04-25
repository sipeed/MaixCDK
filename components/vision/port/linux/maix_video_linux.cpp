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
    Encode::Encode(int width, int height, video::VideoType type) {
        this->_pre_width = width;
        this->_pre_height = height;
        this->_pre_video_type = type;
        this->_is_opened = false;
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    Encode::~Encode() {
        if (this->_is_opened) {
            this->close();
        }
    }

    err::Err Encode::open(int width, int height, video::VideoType type) {
        return err::ERR_NOT_IMPL;
    }

    void Encode::close() {
    }

    video::VideoStream Encode::encode(image::Image &img) {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    Decode::Decode(int width, int height, video::VideoType type) {

    };

    Decode::~Decode() {

    }

    err::Err Decode::open(int width, int height, video::VideoType type) {
        return err::ERR_NOT_IMPL;
    }

    void Decode::close() {

    }

    image::Image *Decode::decode(video::VideoStream &stream) {
        return NULL;
    }

    Video::Video(int width, int height, video::VideoType type)
    {

    }

    Video::~Video() {

    }

    err::Err Video::open(int width, int height, video::VideoType type)
    {
        return err::ERR_NOT_IMPL;
    }

    void Video::close()
    {

    }

    video::VideoStream Video::encode(image::Image &img)
    {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    image::Image *Video::decode(video::VideoStream &stream)
    {
        return NULL;
    }
} // namespace maix::video
