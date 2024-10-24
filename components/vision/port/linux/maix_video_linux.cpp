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
    maix::Bytes *Encoder::NoneBytes = new maix::Bytes();
#else
    maix::image::Image *Video::NoneImage = NULL;
    maix::image::Image *Encoder::NoneImage = NULL;
    maix::Bytes *Encoder::NoneBytes = NULL;
#endif

    double timebase_to_us(std::vector<int> timebase, uint64_t value) {
        return value * 1000000 / ((double)timebase[1] / timebase[0]);
    }

    double timebase_to_ms(std::vector<int> timebase, uint64_t value) {
        return value * 1000 / ((double)timebase[1] / timebase[0]);
    }

    Encoder::Encoder(std::string path, int width, int height, image::Format format, VideoType type, int framerate, int gop, int bitrate, int time_base, bool capture, bool block) {
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

    video::Frame *Encoder::encode(image::Image *img, Bytes *pcm) {
        (void)img;
        (void)pcm;
        throw err::Exception(err::ERR_NOT_IMPL);
        return nullptr;
    }

    Decoder::Decoder(std::string path, image::Format format) {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    Decoder::~Decoder() {
        throw err::Exception(err::ERR_NOT_IMPL);
    }

    video::Context *Decoder::decode_video(bool block) {
        return NULL;
    }

    video::Context *Decoder::decode_audio() {
        return NULL;
    }

    video::Context *Decoder::decode(bool block) {
        (void)block;
        return NULL;
    }

    double Decoder::seek(double time) {
        throw err::Exception(err::ERR_NOT_IMPL);
        return 0;
    }

    double Decoder::duration() {
        throw err::Exception(err::ERR_NOT_IMPL);
        return 0;
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

    VideoRecorder::VideoRecorder(bool open)
    {
        (void)open;
    }

    VideoRecorder::~VideoRecorder()
    {
    }

    err::Err VideoRecorder::open()
    {
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::close()
    {
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::lock(int64_t timeout)
    {
        (void)timeout;
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::unlock()
    {
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::bind_display(display::Display *display, image::Fit fit)
    {
        (void)display;
        (void)fit;
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::bind_camera(camera::Camera *camera)
    {
        (void)camera;
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::bind_audio(audio::Recorder *audio)
    {
        (void)audio;
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::bind_imu(void *imu)
    {
        (void)imu;
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::reset()
    {
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::config_path(std::string path)
    {
        (void)path;
        return err::ERR_NOT_IMPL;
    }

    std::string VideoRecorder::get_path()
    {
        return "";
    }

    err::Err VideoRecorder::config_snapshot(bool enable, std::vector<int> resolution, image::Format format)
    {
        (void)enable;
        (void)resolution;
        (void)format;
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::config_resolution(std::vector<int> resolution)
    {
        (void)resolution;
        return err::ERR_NOT_IMPL;
    }

    std::vector<int> VideoRecorder::get_resolution()
    {
        return {};
    }

    err::Err VideoRecorder::config_fps(int fps)
    {
        return err::ERR_NOT_IMPL;
    }

    int VideoRecorder::get_fps()
    {
        return 0;
    }

    err::Err VideoRecorder::config_bitrate(int bitrate)
    {
        (void)bitrate;
        return err::ERR_NOT_IMPL;
    }

    int VideoRecorder::get_bitrate()
    {
        return 0;
    }

    int VideoRecorder::mute(int data)
    {
        (void)data;
        return 0;
    }

    int VideoRecorder::volume(int data)
    {
        (void)data;
        return 0;
    }

    int64_t VideoRecorder::seek()
    {
        return 0;
    }

    err::Err VideoRecorder::record_start()
    {
        return err::ERR_NOT_IMPL;
    }

    image::Image *VideoRecorder::snapshot()
    {
        return NULL;
    }

    err::Err VideoRecorder::record_finish()
    {
        return err::ERR_NOT_IMPL;
    }

    err::Err VideoRecorder::draw_rect(int id, int x, int y, int w, int h, image::Color color, int thickness, bool hidden)
    {
        (void)id;
        (void)x;
        (void)y;
        (void)w;
        (void)h;
        (void)color;
        (void)thickness;
        (void)hidden;
        return err::ERR_NOT_IMPL;
    }
} // namespace maix::video
