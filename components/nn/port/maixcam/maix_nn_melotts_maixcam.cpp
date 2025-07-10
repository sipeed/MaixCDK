/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2025.5.20: Add melotts support.
 */

#include "maix_basic.hpp"
#include "maix_nn_melotts.hpp"

namespace maix::nn
{
    MeloTTS::MeloTTS(const string &model, std::string language, double speed, double noise_scale, double noise_scale_w, double sdp_ratio) {
        err::check_raise(err::ERR_NOT_IMPL, "MeloTTS not implemented");
    }

    MeloTTS::~MeloTTS() {
    }

    std::string MeloTTS::type()
    {
        return "";
    }

    err::Err MeloTTS::load(const string &model) {
        return err::ERR_NOT_IMPL;
    }

    err::Err MeloTTS::unload() {
        return err::ERR_NOT_IMPL;
    }

    Bytes *MeloTTS::forward(std::string text, std::string path, bool output_pcm) {
        return nullptr;
    }
} // namespace maix::nn