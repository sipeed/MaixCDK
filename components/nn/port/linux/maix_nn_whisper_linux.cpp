/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.6.7: Add yolov8 support.
 */

#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_nn_whisper.hpp"

namespace maix::nn
{
    Whisper::Whisper(const string &model, std::string language) {
        err::check_raise(err::ERR_NOT_IMPL, "Whisper not implemented");
    }

    Whisper::~Whisper() {
        unload();
    }

    std::string Whisper::type()
    {
        return "";
    }

    err::Err Whisper::unload() {
        return err::ERR_NOT_IMPL;
    }

    err::Err Whisper::load(const string &model) {
        return err::ERR_NOT_IMPL;
    }

    std::string Whisper::transcribe(std::string &file) {
        return "";
    }

    std::string Whisper::transcribe_raw(Bytes *pcm, int sample_rate, int channels, int bits_per_frame) {
        return "";
    }
} // namespace maix::nn