/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include <stdint.h>
#include "maix_basic.hpp"
#include "maix_err.hpp"
#include "maix_audio.hpp"

using namespace maix;

namespace maix::audio
{
    Recorder::Recorder(std::string path, int sample_rate, audio::Format format, int channel, bool block) {

        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
    }

    Recorder::~Recorder() {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
    }

    int Recorder::volume(int value) {
        (void)value;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    void Recorder::reset(bool start) {
        (void)start;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
    }

    maix::Bytes *Recorder::record(int record_ms) {
        (void)record_ms;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return NULL;
    }

    int Recorder::frame_size(int frame_count) {
        (void)frame_count;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    int Recorder::get_remaining_frames() {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    int Recorder::period_size(int period_size) {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    int Recorder::period_count(int period_count) {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    maix::Bytes *Recorder::record_bytes(int record_size) {
        (void)record_size;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return NULL;
    }

    bool Recorder::mute(int data) {
        (void)data;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return false;
    }

    err::Err Recorder::finish() {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return err::ERR_NOT_IMPL;
    }

    int Recorder::sample_rate() {
        return 0;
    }

    audio::Format Recorder::format() {
        return audio::Format::FMT_NONE;
    }

    int Recorder::channel() {
        return 0;
    }

    maix::Bytes *Player::NoneBytes = new maix::Bytes();

    Player::Player(std::string path, int sample_rate, audio::Format format, int channel, bool block) {
        (void)path;
        (void)sample_rate;
        (void)format;
        (void)channel;
        (void)block;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
    }

    Player::~Player() {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
    }

    int Player::volume(int value) {
        (void)value;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    err::Err Player::play(maix::Bytes *data) {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return err::ERR_NOT_IMPL;
    }

    int Player::frame_size(int frame_count) {
        (void)frame_count;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    int Player::get_remaining_frames() {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    int Player::period_size(int period_size) {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    int Player::period_count(int period_count) {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return 0;
    }

    void Player::reset(bool start) {
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
    }

    int Player::sample_rate() {
        return 0;
    }

    audio::Format Player::format() {
        return audio::Format::FMT_NONE;
    }

    int Player::channel() {
        return 0;
    }
} // namespace maix::audio