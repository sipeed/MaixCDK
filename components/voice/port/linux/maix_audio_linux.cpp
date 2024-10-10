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
    Recorder::Recorder(std::string path, int sample_rate, audio::Format format, int channel) {

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

    maix::Bytes *Recorder::record(int record_ms) {
        (void)record_ms;
        err::check_raise(err::ERR_NOT_IMPL, "not support this function");
        return NULL;
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

    maix::Bytes *Player::NoneBytes = new maix::Bytes();

    Player::Player(std::string path, int sample_rate, audio::Format format, int channel) {
        (void)path;
        (void)sample_rate;
        (void)format;
        (void)channel;
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
} // namespace maix::audio