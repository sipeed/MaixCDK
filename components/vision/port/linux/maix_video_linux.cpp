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
        err::Err err = err::ERR_NONE;

        return err;
    }

    void Encode::close() {
        if (!this->_is_opened) {
            return;
        }
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

    Video::Video(std::string path, bool record, int interval, int width, int height, bool audio, int sample_rate, int channel, bool open)
    {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    Video::~Video() {
        if (this->_is_opened) {
            this->close();
        }
    }

    err::Err Video::open(std::string path, bool record, int interval, int width, int height, bool audio, int sample_rate, int channel)
    {
        throw err::Exception(err::ERR_NOT_IMPL);
        return err::ERR_NONE;
    }

    void Video::close()
    {
       throw err::Exception(err::ERR_NOT_IMPL);
    }

    err::Err Video::bind_camera(camera::Camera *camera) {
        err::Err err = err::ERR_NONE;
        throw err::Exception(err::ERR_NOT_IMPL);
        return err;
    }


    err::Err Video::record_start(uint64_t record_time) {
       throw err::Exception(err::ERR_NOT_IMPL);
    }

    err::Err Video::record_finish() {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    image::Image *Video::capture() {
        return NULL;
    }

    video::VideoStream Video::encode(image::Image &img)
    {
        video::VideoStream stream;
        return stream;
    }

    image::Image *Video::decode(video::VideoStream &stream)
    {
        return NULL;
    }
} // namespace maix::video
